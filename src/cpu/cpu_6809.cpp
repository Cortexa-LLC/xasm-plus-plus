/**
 * @file cpu_6809.cpp
 * @brief Motorola 6809 CPU Plugin implementation
 *
 * Implements instruction encoding for the Motorola 6809 processor.
 * The 6809 is one of the most sophisticated 8-bit CPUs, featuring
 * advanced indexed addressing and 16-bit operations.
 *
 * Encoding is table-driven: kOpcodeTable maps each mnemonic to its
 * per-addressing-mode opcode bytes.  EncodeMemInstr() handles the
 * shared Immediate8 / Immediate16 / Direct / Extended / Indexed logic
 * in one place, eliminating ~25 near-identical Encode* method bodies.
 */

#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/cpu/cpu_6809_constants.h"
#include "xasm++/cpu/cpu_error_utils.h"
#include "xasm++/cpu/encoding_utils.h"
#include "xasm++/cpu/opcodes_6809.h"
#include "xasm++/util/string_utils.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace xasm {

// ============================================================================
// Direct page register accessors
// ============================================================================

void Cpu6809::SetDirectPage(uint8_t dp) { direct_page_ = dp; }

uint8_t Cpu6809::GetDirectPage() const { return direct_page_; }

// ============================================================================
// Opcode table types
// ============================================================================

/// Sentinel value: "this addressing mode is not valid for this instruction"
static constexpr uint8_t NO_OP = 0xFF;

/**
 * @brief Opcode entries for a single mnemonic across all memory-operand
 *        addressing modes.
 *
 * Fields:
 *  prefix  — optional page-2 ($10) or page-3 ($11) prefix byte, or 0x00
 *  imm     — Immediate8 or Immediate16 opcode (NO_OP if not supported)
 *  dir     — Direct-page opcode               (NO_OP if not supported)
 *  ext     — Extended opcode                  (NO_OP if not supported)
 *  idx     — Indexed opcode                   (NO_OP if not supported)
 *  imm16   — true when the immediate form takes a 16-bit operand
 */
struct InstrOpcodes {
  uint8_t prefix; ///< 0x00=none, 0x10=page2, 0x11=page3
  uint8_t imm;    ///< immediate opcode (8-bit unless imm16==true)
  uint8_t dir;    ///< direct-page opcode
  uint8_t ext;    ///< extended opcode
  uint8_t idx;    ///< indexed opcode
  bool imm16;     ///< true → 16-bit immediate operand
};

// ============================================================================
// Static opcode lookup table  (mnemonic → InstrOpcodes)
// ============================================================================

// clang-format off
static const std::unordered_map<std::string, InstrOpcodes> kOpcodeTable = {
  // -------------------------------------------------------------------------
  // 8-bit Load / Store — accumulator A
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LDA,  { 0x00, Opcodes6809::LDA_IMM,  Opcodes6809::LDA_DIR,  Opcodes6809::LDA_EXT,  Opcodes6809::LDA_IDX,  false } },
  { M6809Mnemonics::STA,  { 0x00, NO_OP,                  Opcodes6809::STA_DIR,  Opcodes6809::STA_EXT,  Opcodes6809::STA_IDX,  false } },

  // -------------------------------------------------------------------------
  // 8-bit Load / Store — accumulator B
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LDB,  { 0x00, Opcodes6809::LDB_IMM,  Opcodes6809::LDB_DIR,  Opcodes6809::LDB_EXT,  Opcodes6809::LDB_IDX,  false } },
  { M6809Mnemonics::STB,  { 0x00, NO_OP,                  Opcodes6809::STB_DIR,  Opcodes6809::STB_EXT,  Opcodes6809::STB_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — accumulator D
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LDD,  { 0x00, Opcodes6809::LDD_IMM,  Opcodes6809::LDD_DIR,  Opcodes6809::LDD_EXT,  Opcodes6809::LDD_IDX,  true  } },
  { M6809Mnemonics::STD,  { 0x00, NO_OP,                  Opcodes6809::STD_DIR,  Opcodes6809::STD_EXT,  Opcodes6809::STD_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — index register X
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LDX,  { 0x00, Opcodes6809::LDX_IMM,  Opcodes6809::LDX_DIR,  Opcodes6809::LDX_EXT,  Opcodes6809::LDX_IDX,  true  } },
  { M6809Mnemonics::STX,  { 0x00, NO_OP,                  Opcodes6809::STX_DIR,  Opcodes6809::STX_EXT,  Opcodes6809::STX_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — index register Y  (page-2 prefix)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LDY,  { 0x10, Opcodes6809::Page2::LDY_IMM, Opcodes6809::Page2::LDY_DIR, Opcodes6809::Page2::LDY_EXT, Opcodes6809::Page2::LDY_IDX, true  } },
  { M6809Mnemonics::STY,  { 0x10, NO_OP,                        Opcodes6809::Page2::STY_DIR, Opcodes6809::Page2::STY_EXT, Opcodes6809::Page2::STY_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — ADD
  // -------------------------------------------------------------------------
  { M6809Mnemonics::ADDA, { 0x00, Opcodes6809::ADDA_IMM, Opcodes6809::ADDA_DIR, Opcodes6809::ADDA_EXT, Opcodes6809::ADDA_IDX, false } },
  { M6809Mnemonics::ADDB, { 0x00, Opcodes6809::ADDB_IMM, Opcodes6809::ADDB_DIR, Opcodes6809::ADDB_EXT, Opcodes6809::ADDB_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — SUB
  // -------------------------------------------------------------------------
  { M6809Mnemonics::SUBA, { 0x00, Opcodes6809::SUBA_IMM, Opcodes6809::SUBA_DIR, Opcodes6809::SUBA_EXT, Opcodes6809::SUBA_IDX, false } },
  { M6809Mnemonics::SUBB, { 0x00, Opcodes6809::SUBB_IMM, Opcodes6809::SUBB_DIR, Opcodes6809::SUBB_EXT, Opcodes6809::SUBB_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — CMP (compare without storing result)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::CMPA, { 0x00, Opcodes6809::CMPA_IMM, Opcodes6809::CMPA_DIR, Opcodes6809::CMPA_EXT, Opcodes6809::CMPA_IDX, false } },
  { M6809Mnemonics::CMPB, { 0x00, Opcodes6809::CMPB_IMM, Opcodes6809::CMPB_DIR, Opcodes6809::CMPB_EXT, Opcodes6809::CMPB_IDX, false } },
  { M6809Mnemonics::CMPX, { 0x00, Opcodes6809::CMPX_IMM, Opcodes6809::CMPX_DIR, Opcodes6809::CMPX_EXT, Opcodes6809::CMPX_IDX, true  } },

  // CMPY uses page-2 prefix; no indexed mode defined
  { M6809Mnemonics::CMPY, { 0x10, Opcodes6809::Page2::CMPY_IMM, Opcodes6809::Page2::CMPY_DIR, Opcodes6809::Page2::CMPY_EXT, NO_OP, true } },

  // -------------------------------------------------------------------------
  // Logical — AND
  // -------------------------------------------------------------------------
  { M6809Mnemonics::ANDA, { 0x00, Opcodes6809::ANDA_IMM, Opcodes6809::ANDA_DIR, Opcodes6809::ANDA_EXT, Opcodes6809::ANDA_IDX, false } },
  { M6809Mnemonics::ANDB, { 0x00, Opcodes6809::ANDB_IMM, Opcodes6809::ANDB_DIR, Opcodes6809::ANDB_EXT, Opcodes6809::ANDB_IDX, false } },

  // -------------------------------------------------------------------------
  // Logical — OR
  // -------------------------------------------------------------------------
  { M6809Mnemonics::ORA,  { 0x00, Opcodes6809::ORA_IMM,  Opcodes6809::ORA_DIR,  Opcodes6809::ORA_EXT,  Opcodes6809::ORA_IDX,  false } },
  { M6809Mnemonics::ORB,  { 0x00, Opcodes6809::ORB_IMM,  Opcodes6809::ORB_DIR,  Opcodes6809::ORB_EXT,  Opcodes6809::ORB_IDX,  false } },

  // -------------------------------------------------------------------------
  // Logical — XOR (EOR)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::EORA, { 0x00, Opcodes6809::EORA_IMM, Opcodes6809::EORA_DIR, Opcodes6809::EORA_EXT, Opcodes6809::EORA_IDX, false } },
  { M6809Mnemonics::EORB, { 0x00, Opcodes6809::EORB_IMM, Opcodes6809::EORB_DIR, Opcodes6809::EORB_EXT, Opcodes6809::EORB_IDX, false } },

  // -------------------------------------------------------------------------
  // Logical — BIT test (AND without storing result)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::BITA, { 0x00, Opcodes6809::BITA_IMM, Opcodes6809::BITA_DIR, Opcodes6809::BITA_EXT, Opcodes6809::BITA_IDX, false } },
  { M6809Mnemonics::BITB, { 0x00, Opcodes6809::BITB_IMM, Opcodes6809::BITB_DIR, Opcodes6809::BITB_EXT, Opcodes6809::BITB_IDX, false } },

  // -------------------------------------------------------------------------
  // Jump / Subroutine  (no immediate mode)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::JMP,  { 0x00, NO_OP, Opcodes6809::JMP_DIR, Opcodes6809::JMP_EXT, Opcodes6809::JMP_IDX, false } },
  { M6809Mnemonics::JSR,  { 0x00, NO_OP, Opcodes6809::JSR_DIR, Opcodes6809::JSR_EXT, Opcodes6809::JSR_IDX, false } },

  // -------------------------------------------------------------------------
  // Load Effective Address  (indexed mode only; treated via Extended for addr)
  // -------------------------------------------------------------------------
  { M6809Mnemonics::LEAX, { 0x00, NO_OP, NO_OP, Opcodes6809::LEAX, Opcodes6809::LEAX, false } },
  { M6809Mnemonics::LEAY, { 0x00, NO_OP, NO_OP, Opcodes6809::LEAY, Opcodes6809::LEAY, false } },
};
// clang-format on

// ============================================================================
// Free-function indexed post-byte builder (no class access needed)
// ============================================================================

/**
 * @brief Build the indexed post-byte(s) for a given mode and offset.
 *
 * This mirrors Cpu6809::EncodeIndexedPostByte but is accessible from
 * the static EncodeMemInstr helper without requiring class member access.
 *
 * @param mode      Indexed sub-mode
 * @param offset    Signed offset / address
 * @param base_reg  Base register index (0=X, 1=Y, 2=U, 3=S)
 * @return          Post-byte and any trailing offset bytes
 */
static std::vector<uint8_t>
BuildIndexedPostByte(AddressingMode6809 mode, int32_t offset,
                     uint8_t base_reg) {
  std::vector<uint8_t> result;
  uint8_t reg_bits = (base_reg & 0x03) << 5;

  switch (mode) {
  case AddressingMode6809::IndexedZeroOffset:
    result.push_back(0x84 | reg_bits);
    break;
  case AddressingMode6809::Indexed5BitOffset: {
    uint8_t offset_5bit = static_cast<uint8_t>(offset & 0x1F);
    result.push_back(offset_5bit | reg_bits);
    break;
  }
  case AddressingMode6809::Indexed8BitOffset:
    result.push_back(0x88 | reg_bits);
    result.push_back(static_cast<uint8_t>(offset & 0xFF));
    break;
  case AddressingMode6809::Indexed16BitOffset: {
    result.push_back(0x89 | reg_bits);
    auto ob = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
    result.insert(result.end(), ob.begin(), ob.end());
    break;
  }
  case AddressingMode6809::IndexedAccumA:
    result.push_back(0x86 | reg_bits);
    break;
  case AddressingMode6809::IndexedAccumB:
    result.push_back(0x85 | reg_bits);
    break;
  case AddressingMode6809::IndexedAccumD:
    result.push_back(0x8B | reg_bits);
    break;
  case AddressingMode6809::IndexedAutoInc1:
    result.push_back(0x80 | reg_bits);
    break;
  case AddressingMode6809::IndexedAutoInc2:
    result.push_back(0x81 | reg_bits);
    break;
  case AddressingMode6809::IndexedAutoDec1:
    result.push_back(0x82 | reg_bits);
    break;
  case AddressingMode6809::IndexedAutoDec2:
    result.push_back(0x83 | reg_bits);
    break;
  case AddressingMode6809::IndexedPCRelative8:
    result.push_back(0x8C | reg_bits);
    result.push_back(static_cast<uint8_t>(offset & 0xFF));
    break;
  case AddressingMode6809::IndexedPCRelative16: {
    result.push_back(0x8D | reg_bits);
    auto ob = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
    result.insert(result.end(), ob.begin(), ob.end());
    break;
  }
  case AddressingMode6809::IndexedIndirect: {
    if (offset == 0) {
      result.push_back(0x94 | reg_bits);
    } else if (offset >= -128 && offset <= 127) {
      result.push_back(0x98 | reg_bits);
      result.push_back(static_cast<uint8_t>(offset & 0xFF));
    } else {
      result.push_back(0x99 | reg_bits);
      auto ob = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
      result.insert(result.end(), ob.begin(), ob.end());
    }
    break;
  }
  case AddressingMode6809::IndexedExtendedIndirect: {
    result.push_back(0x9F);
    auto ab = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
    result.insert(result.end(), ab.begin(), ab.end());
    break;
  }
  default:
    break;
  }

  return result;
}

// ============================================================================
// Generic memory-operand encoder (replaces ~25 near-identical Encode* methods)
// ============================================================================

/**
 * @brief Encode any instruction that uses standard memory addressing modes.
 *
 * Given an entry from kOpcodeTable, builds the correct byte sequence for the
 * supplied (operand, mode) pair.  All page-prefix handling is done here.
 *
 * @param entry    Opcode entry from kOpcodeTable
 * @param operand  Address / immediate value (32-bit, upper bits ignored)
 * @param mode     Addressing mode for the instruction
 * @return         Encoded bytes, or empty vector on unsupported combination
 */
static std::vector<uint8_t>
EncodeMemInstr(const InstrOpcodes &entry, uint32_t operand,
               AddressingMode6809 mode) {
  // Helper: build {[prefix,] opcode, imm8}
  auto make_imm8 = [&](uint8_t opcode) -> std::vector<uint8_t> {
    if (opcode == NO_OP)
      return {};
    std::vector<uint8_t> r;
    if (entry.prefix)
      r.push_back(entry.prefix);
    r.push_back(opcode);
    r.push_back(static_cast<uint8_t>(operand & 0xFF));
    return r;
  };

  // Helper: build {[prefix,] opcode, hi, lo}  (big-endian 16-bit operand)
  auto make_imm16 = [&](uint8_t opcode) -> std::vector<uint8_t> {
    if (opcode == NO_OP)
      return {};
    std::vector<uint8_t> r;
    if (entry.prefix)
      r.push_back(entry.prefix);
    r.push_back(opcode);
    auto v = encoding::ToBigEndian16(static_cast<uint16_t>(operand));
    r.insert(r.end(), v.begin(), v.end());
    return r;
  };

  switch (mode) {
  // ---------------------------------------------------------
  case AddressingMode6809::Immediate8:
    return make_imm8(entry.imm);

  case AddressingMode6809::Immediate16:
    return make_imm16(entry.imm);

  // ---------------------------------------------------------
  case AddressingMode6809::Direct:
    return make_imm8(entry.dir);

  // ---------------------------------------------------------
  case AddressingMode6809::Extended:
    return make_imm16(entry.ext);

  // ---------------------------------------------------------
  // All indexed sub-modes share the same base opcode; the post-byte
  // distinguishes between them.
  case AddressingMode6809::IndexedZeroOffset:
  case AddressingMode6809::Indexed5BitOffset:
  case AddressingMode6809::Indexed8BitOffset:
  case AddressingMode6809::Indexed16BitOffset:
  case AddressingMode6809::IndexedAccumA:
  case AddressingMode6809::IndexedAccumB:
  case AddressingMode6809::IndexedAccumD:
  case AddressingMode6809::IndexedAutoInc1:
  case AddressingMode6809::IndexedAutoInc2:
  case AddressingMode6809::IndexedAutoDec1:
  case AddressingMode6809::IndexedAutoDec2:
  case AddressingMode6809::IndexedPCRelative8:
  case AddressingMode6809::IndexedPCRelative16:
  case AddressingMode6809::IndexedIndirect:
  case AddressingMode6809::IndexedExtendedIndirect: {
    if (entry.idx == NO_OP)
      return {};
    std::vector<uint8_t> result;
    if (entry.prefix)
      result.push_back(entry.prefix);
    result.push_back(entry.idx);
    auto postbyte = BuildIndexedPostByte(mode, static_cast<int32_t>(operand), 0);
    result.insert(result.end(), postbyte.begin(), postbyte.end());
    return result;
  }

  default:
    return {};
  }
}

// ============================================================================
// Inherent-mode (no operand) instructions
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {Opcodes6809::NOP}; // NOP opcode
}

std::vector<uint8_t> Cpu6809::EncodeRTS() const {
  return {Opcodes6809::RTS}; // RTS opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRA() const {
  return {Opcodes6809::CLRA}; // CLRA opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRB() const {
  return {Opcodes6809::CLRB}; // CLRB opcode
}

// Shift and Rotate Instructions
std::vector<uint8_t> Cpu6809::EncodeASLA() const { return {Opcodes6809::ASLA}; }

std::vector<uint8_t> Cpu6809::EncodeASLB() const { return {Opcodes6809::ASLB}; }

std::vector<uint8_t> Cpu6809::EncodeASRA() const { return {Opcodes6809::ASRA}; }

std::vector<uint8_t> Cpu6809::EncodeASRB() const { return {Opcodes6809::ASRB}; }

std::vector<uint8_t> Cpu6809::EncodeLSRA() const { return {Opcodes6809::LSRA}; }

std::vector<uint8_t> Cpu6809::EncodeLSRB() const { return {Opcodes6809::LSRB}; }

std::vector<uint8_t> Cpu6809::EncodeROLA() const { return {Opcodes6809::ROLA}; }

std::vector<uint8_t> Cpu6809::EncodeROLB() const { return {Opcodes6809::ROLB}; }

std::vector<uint8_t> Cpu6809::EncodeRORA() const { return {Opcodes6809::RORA}; }

std::vector<uint8_t> Cpu6809::EncodeRORB() const { return {Opcodes6809::RORB}; }

// Increment / Decrement
std::vector<uint8_t> Cpu6809::EncodeINCA() const { return {Opcodes6809::INCA}; }

std::vector<uint8_t> Cpu6809::EncodeINCB() const { return {Opcodes6809::INCB}; }

std::vector<uint8_t> Cpu6809::EncodeDECA() const { return {Opcodes6809::DECA}; }

std::vector<uint8_t> Cpu6809::EncodeDECB() const { return {Opcodes6809::DECB}; }

// Test and Complement/Negate
std::vector<uint8_t> Cpu6809::EncodeTSTA() const { return {Opcodes6809::TSTA}; }

std::vector<uint8_t> Cpu6809::EncodeTSTB() const { return {Opcodes6809::TSTB}; }

std::vector<uint8_t> Cpu6809::EncodeCOMA() const { return {Opcodes6809::COMA}; }

std::vector<uint8_t> Cpu6809::EncodeCOMB() const { return {Opcodes6809::COMB}; }

std::vector<uint8_t> Cpu6809::EncodeNEGA() const { return {Opcodes6809::NEGA}; }

std::vector<uint8_t> Cpu6809::EncodeNEGB() const { return {Opcodes6809::NEGB}; }

// ============================================================================
// Memory-operand instructions  (delegates to kOpcodeTable via EncodeMemInstr)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDB(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDD(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LDD), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDX(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LDX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDY(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LDY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTA(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::STA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTB(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::STB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTD(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::STD), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTX(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::STX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTY(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::STY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeADDA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ADDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeADDB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ADDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSUBA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::SUBA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSUBB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::SUBB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::CMPA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::CMPB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPX(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::CMPX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPY(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::CMPY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeANDA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ANDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeANDB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ANDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeORA(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ORA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeORB(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::ORB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeEORA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::EORA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeEORB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::EORB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeBITA(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::BITA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeBITB(uint32_t operand,
                                         AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::BITB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeJMP(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::JMP), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeJSR(uint32_t operand,
                                        AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::JSR), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLEAX(uint32_t operand,
                                          AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LEAX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLEAY(uint32_t operand,
                                          AddressingMode6809 mode) const {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonics::LEAY), operand, mode);
}

// ============================================================================
// Short Branch Instructions (8-bit relative addressing)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeBRA(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BRA, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBEQ(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BEQ, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBNE(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BNE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCC(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BCC, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCS(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BCS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBMI(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BMI, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBPL(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BPL, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVS(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BVS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVC(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BVC, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGE(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BGE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGT(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BGT, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLE(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLT(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLT, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBHI(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BHI, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLS(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBSR(int32_t offset,
                                        AddressingMode6809 mode) const {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BSR, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

// ============================================================================
// Long Branch Instructions (16-bit relative addressing)
// ============================================================================

/// Helper: build a 4-byte long branch: page-2 prefix + opcode + 16-bit offset
static std::vector<uint8_t> MakeLongBranch(uint8_t opcode, int16_t offset) {
  std::vector<uint8_t> result = {Opcodes6809::PAGE2_PREFIX, opcode};
  auto offset_bytes = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
  result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
  return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBRA(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBRA, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBRN(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBRN, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBEQ(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBEQ, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBNE(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBNE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBCC(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBCC, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBCS(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBCS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBMI(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBMI, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBPL(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBPL, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBVS(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBVS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBVC(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBVC, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBGE(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBGE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLT(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBLT, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBGT(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBGT, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLE(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBLE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBHI(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBHI, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLS(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBLS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBSR(int16_t offset) const {
  return MakeLongBranch(Opcodes6809::Page2::LBSR, offset);
}

// ============================================================================
// Register-transfer and Exchange
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeTFR(uint8_t src, uint8_t dst) const {
  // Post-byte format: high nibble = source, low nibble = destination
  uint8_t postbyte = (src << 4) | dst;
  return {Opcodes6809::TFR, postbyte};
}

std::vector<uint8_t> Cpu6809::EncodeEXG(uint8_t reg1, uint8_t reg2) const {
  // Post-byte format: high nibble = reg1, low nibble = reg2
  uint8_t postbyte = (reg1 << 4) | reg2;
  return {Opcodes6809::EXG, postbyte};
}

// ============================================================================
// Stack push / pull
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodePSHS(uint8_t mask) const {
  return {Opcodes6809::PSHS, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULS(uint8_t mask) const {
  return {Opcodes6809::PULS, mask};
}

std::vector<uint8_t> Cpu6809::EncodePSHU(uint8_t mask) const {
  return {Opcodes6809::PSHU, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULU(uint8_t mask) const {
  return {Opcodes6809::PULU, mask};
}

// ============================================================================
// Indexed post-byte encoder
// ============================================================================

std::vector<uint8_t>
Cpu6809::EncodeIndexedPostByte(AddressingMode6809 mode, int32_t offset,
                               uint8_t base_reg) const {
  // Delegate to the free-function implementation (shared with EncodeMemInstr).
  return BuildIndexedPostByte(mode, offset, base_reg);
}

// ============================================================================
// Instruction Size Calculation
// ============================================================================

size_t Cpu6809::CalculateInstructionSize(AddressingMode6809 mode) const {
  switch (mode) {
  case AddressingMode6809::Inherent:
    return 1;

  case AddressingMode6809::Immediate8:
  case AddressingMode6809::Direct:
    return 2;

  case AddressingMode6809::Immediate16:
  case AddressingMode6809::Extended:
    return 3;

  case AddressingMode6809::Relative8:
    return 2;

  case AddressingMode6809::Relative16:
    return 3;

  // Indexed modes vary by sub-mode (2-5 bytes)
  case AddressingMode6809::IndexedZeroOffset:
  case AddressingMode6809::IndexedAutoInc1:
  case AddressingMode6809::IndexedAutoDec1:
    return 2; // Opcode + post-byte

  case AddressingMode6809::Indexed5BitOffset:
  case AddressingMode6809::Indexed8BitOffset:
  case AddressingMode6809::IndexedAccumA:
  case AddressingMode6809::IndexedAccumB:
  case AddressingMode6809::IndexedAccumD:
  case AddressingMode6809::IndexedAutoInc2:
  case AddressingMode6809::IndexedAutoDec2:
  case AddressingMode6809::IndexedPCRelative8:
    return 3; // Opcode + post-byte + 8-bit offset

  case AddressingMode6809::Indexed16BitOffset:
  case AddressingMode6809::IndexedPCRelative16:
    return 4; // Opcode + post-byte + 16-bit offset

  case AddressingMode6809::IndexedIndirect:
  case AddressingMode6809::IndexedExtendedIndirect:
    return 5; // Opcode + post-byte + 16-bit address

  default:
    return 0;
  }
}

// ============================================================================
// HasOpcode
// ============================================================================

bool Cpu6809::HasOpcode(const std::string &mnemonic) const {
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Create static set of all 6809 mnemonics for O(1) lookup
  static const std::unordered_set<std::string> valid_opcodes = {
      // Load/Store
      M6809Mnemonics::LDA, M6809Mnemonics::LDB, M6809Mnemonics::LDD,
      M6809Mnemonics::LDX, M6809Mnemonics::LDY, M6809Mnemonics::LDU,
      M6809Mnemonics::LDS, M6809Mnemonics::STA, M6809Mnemonics::STB,
      M6809Mnemonics::STD, M6809Mnemonics::STX, M6809Mnemonics::STY,
      M6809Mnemonics::STU, M6809Mnemonics::STS,

      // Arithmetic
      M6809Mnemonics::ADDA, M6809Mnemonics::ADDB, M6809Mnemonics::ADDD,
      M6809Mnemonics::SUBA, M6809Mnemonics::SUBB, M6809Mnemonics::SUBD,
      M6809Mnemonics::INCA, M6809Mnemonics::INCB, M6809Mnemonics::INC,
      M6809Mnemonics::DECA, M6809Mnemonics::DECB, M6809Mnemonics::DEC,
      M6809Mnemonics::NEGA, M6809Mnemonics::NEGB, M6809Mnemonics::NEG,

      // Logical
      M6809Mnemonics::ANDA, M6809Mnemonics::ANDB,
      M6809Mnemonics::ORA,  M6809Mnemonics::ORB,
      M6809Mnemonics::EORA, M6809Mnemonics::EORB,
      M6809Mnemonics::BITA, M6809Mnemonics::BITB,
      M6809Mnemonics::COMA, M6809Mnemonics::COMB,
      M6809Mnemonics::CLRA, M6809Mnemonics::CLRB, M6809Mnemonics::CLR,

      // Compare / Test
      M6809Mnemonics::CMPA, M6809Mnemonics::CMPB,
      M6809Mnemonics::CMPX, M6809Mnemonics::CMPY,
      M6809Mnemonics::CMPU, M6809Mnemonics::CMPS,
      M6809Mnemonics::TSTA, M6809Mnemonics::TSTB, M6809Mnemonics::TST,

      // Shift / Rotate
      M6809Mnemonics::ASLA, M6809Mnemonics::ASLB, M6809Mnemonics::ASL,
      M6809Mnemonics::ASRA, M6809Mnemonics::ASRB, M6809Mnemonics::ASR,
      M6809Mnemonics::LSRA, M6809Mnemonics::LSRB, M6809Mnemonics::LSR,
      M6809Mnemonics::ROLA, M6809Mnemonics::ROLB, M6809Mnemonics::ROL,
      M6809Mnemonics::RORA, M6809Mnemonics::RORB, M6809Mnemonics::ROR,

      // Short Branch (8-bit)
      M6809Mnemonics::BRA, M6809Mnemonics::BRN,
      M6809Mnemonics::BHI, M6809Mnemonics::BLS,
      M6809Mnemonics::BCC, M6809Mnemonics::BHS,   // BHS is alias for BCC
      M6809Mnemonics::BCS, M6809Mnemonics::BLO,   // BLO is alias for BCS
      M6809Mnemonics::BNE, M6809Mnemonics::BEQ,
      M6809Mnemonics::BVC, M6809Mnemonics::BVS,
      M6809Mnemonics::BPL, M6809Mnemonics::BMI,
      M6809Mnemonics::BGE, M6809Mnemonics::BLT,
      M6809Mnemonics::BGT, M6809Mnemonics::BLE,
      M6809Mnemonics::BSR,

      // Long Branch (16-bit)
      M6809Mnemonics::LBRA, M6809Mnemonics::LBRN,
      M6809Mnemonics::LBHI, M6809Mnemonics::LBLS,
      M6809Mnemonics::LBCC, M6809Mnemonics::LBHS, // LBHS alias for LBCC
      M6809Mnemonics::LBCS, M6809Mnemonics::LBLO, // LBLO alias for LBCS
      M6809Mnemonics::LBNE, M6809Mnemonics::LBEQ,
      M6809Mnemonics::LBVC, M6809Mnemonics::LBVS,
      M6809Mnemonics::LBPL, M6809Mnemonics::LBMI,
      M6809Mnemonics::LBGE, M6809Mnemonics::LBLT,
      M6809Mnemonics::LBGT, M6809Mnemonics::LBLE,
      M6809Mnemonics::LBSR,

      // Jump / Subroutine
      M6809Mnemonics::JMP, M6809Mnemonics::JSR, M6809Mnemonics::RTS,
      M6809Mnemonics::LEAX, M6809Mnemonics::LEAY,

      // Stack
      M6809Mnemonics::PSHS, M6809Mnemonics::PULS,
      M6809Mnemonics::PSHU, M6809Mnemonics::PULU,

      // Register Transfer / Exchange
      M6809Mnemonics::TFR, M6809Mnemonics::EXG,

      // Inherent
      M6809Mnemonics::NOP,
  };

  return valid_opcodes.count(upper) > 0;
}

// ============================================================================
// EncodeInstruction — main dispatch
// ============================================================================

std::vector<uint8_t>
Cpu6809::EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                           const std::string &operand_str) const {
  // Determine addressing mode from operand_str
  std::string trimmed = util::Trim(operand_str);
  AddressingMode6809 mode = AddressingMode6809::Inherent;

  if (!trimmed.empty()) {
    // Immediate mode: #value
    if (trimmed[0] == '#') {
      // 16-bit immediate for instructions that operate on 16-bit registers
      bool is_16bit =
          (mnemonic == M6809Mnemonics::LDD  || mnemonic == M6809Mnemonics::LDX  ||
           mnemonic == M6809Mnemonics::LDY  || mnemonic == M6809Mnemonics::LDU  ||
           mnemonic == M6809Mnemonics::LDS  ||
           mnemonic == M6809Mnemonics::CMPX || mnemonic == M6809Mnemonics::CMPY ||
           mnemonic == M6809Mnemonics::CMPU || mnemonic == M6809Mnemonics::CMPS);
      mode = is_16bit ? AddressingMode6809::Immediate16
                      : AddressingMode6809::Immediate8;
    }
    // Direct page mode: <address or address < $100
    else if (trimmed[0] == '<' || operand < cpu6809::DIRECT_PAGE_MAX) {
      mode = AddressingMode6809::Direct;
    }
    // Indexed modes: ,X or ,Y or offset,X etc.
    else if (trimmed.find(',') != std::string::npos) {
      if (trimmed.find(",X") != std::string::npos ||
          trimmed.find(",Y") != std::string::npos ||
          trimmed.find(",U") != std::string::npos ||
          trimmed.find(",S") != std::string::npos) {
        size_t comma = trimmed.find(',');
        if (comma == 0) {
          mode = AddressingMode6809::IndexedZeroOffset;
        } else {
          mode = AddressingMode6809::Indexed8BitOffset;
        }
      } else {
        mode = AddressingMode6809::IndexedZeroOffset;
      }
    }
    // Extended mode: address >= $100
    else {
      mode = AddressingMode6809::Extended;
    }
  }

  // ── Inherent (no-operand) instructions ───────────────────────────────────
  if (mnemonic == M6809Mnemonics::NOP)  return EncodeNOP();
  if (mnemonic == M6809Mnemonics::RTS)  return EncodeRTS();
  if (mnemonic == M6809Mnemonics::CLRA) return EncodeCLRA();
  if (mnemonic == M6809Mnemonics::CLRB) return EncodeCLRB();
  if (mnemonic == M6809Mnemonics::ASLA) return EncodeASLA();
  if (mnemonic == M6809Mnemonics::ASLB) return EncodeASLB();
  if (mnemonic == M6809Mnemonics::ASRA) return EncodeASRA();
  if (mnemonic == M6809Mnemonics::ASRB) return EncodeASRB();
  if (mnemonic == M6809Mnemonics::LSRA) return EncodeLSRA();
  if (mnemonic == M6809Mnemonics::LSRB) return EncodeLSRB();
  if (mnemonic == M6809Mnemonics::ROLA) return EncodeROLA();
  if (mnemonic == M6809Mnemonics::ROLB) return EncodeROLB();
  if (mnemonic == M6809Mnemonics::RORA) return EncodeRORA();
  if (mnemonic == M6809Mnemonics::RORB) return EncodeRORB();
  if (mnemonic == M6809Mnemonics::INCA) return EncodeINCA();
  if (mnemonic == M6809Mnemonics::INCB) return EncodeINCB();
  if (mnemonic == M6809Mnemonics::DECA) return EncodeDECA();
  if (mnemonic == M6809Mnemonics::DECB) return EncodeDECB();
  if (mnemonic == M6809Mnemonics::TSTA) return EncodeTSTA();
  if (mnemonic == M6809Mnemonics::TSTB) return EncodeTSTB();
  if (mnemonic == M6809Mnemonics::COMA) return EncodeCOMA();
  if (mnemonic == M6809Mnemonics::COMB) return EncodeCOMB();
  if (mnemonic == M6809Mnemonics::NEGA) return EncodeNEGA();
  if (mnemonic == M6809Mnemonics::NEGB) return EncodeNEGB();

  // ── Short branches ────────────────────────────────────────────────────────
  {
    int32_t off = static_cast<int32_t>(operand);
    if (mnemonic == M6809Mnemonics::BRA) return EncodeBRA(off, mode);
    if (mnemonic == M6809Mnemonics::BEQ) return EncodeBEQ(off, mode);
    if (mnemonic == M6809Mnemonics::BNE) return EncodeBNE(off, mode);
    if (mnemonic == M6809Mnemonics::BCC || mnemonic == M6809Mnemonics::BHS)
      return EncodeBCC(off, mode);
    if (mnemonic == M6809Mnemonics::BCS || mnemonic == M6809Mnemonics::BLO)
      return EncodeBCS(off, mode);
    if (mnemonic == M6809Mnemonics::BMI) return EncodeBMI(off, mode);
    if (mnemonic == M6809Mnemonics::BPL) return EncodeBPL(off, mode);
    if (mnemonic == M6809Mnemonics::BVS) return EncodeBVS(off, mode);
    if (mnemonic == M6809Mnemonics::BVC) return EncodeBVC(off, mode);
    if (mnemonic == M6809Mnemonics::BGE) return EncodeBGE(off, mode);
    if (mnemonic == M6809Mnemonics::BGT) return EncodeBGT(off, mode);
    if (mnemonic == M6809Mnemonics::BLE) return EncodeBLE(off, mode);
    if (mnemonic == M6809Mnemonics::BLT) return EncodeBLT(off, mode);
    if (mnemonic == M6809Mnemonics::BHI) return EncodeBHI(off, mode);
    if (mnemonic == M6809Mnemonics::BLS) return EncodeBLS(off, mode);
    if (mnemonic == M6809Mnemonics::BSR) return EncodeBSR(off, mode);
  }

  // ── Long branches ─────────────────────────────────────────────────────────
  {
    int16_t off16 = static_cast<int16_t>(operand);
    if (mnemonic == M6809Mnemonics::LBRA) return EncodeLBRA(off16);
    if (mnemonic == M6809Mnemonics::LBSR) return EncodeLBSR(off16);
    if (mnemonic == M6809Mnemonics::LBRN) return EncodeLBRN(off16);
    if (mnemonic == M6809Mnemonics::LBHI) return EncodeLBHI(off16);
    if (mnemonic == M6809Mnemonics::LBLS) return EncodeLBLS(off16);
    if (mnemonic == M6809Mnemonics::LBCC || mnemonic == M6809Mnemonics::LBHS)
      return EncodeLBCC(off16);
    if (mnemonic == M6809Mnemonics::LBCS || mnemonic == M6809Mnemonics::LBLO)
      return EncodeLBCS(off16);
    if (mnemonic == M6809Mnemonics::LBNE) return EncodeLBNE(off16);
    if (mnemonic == M6809Mnemonics::LBEQ) return EncodeLBEQ(off16);
    if (mnemonic == M6809Mnemonics::LBVC) return EncodeLBVC(off16);
    if (mnemonic == M6809Mnemonics::LBVS) return EncodeLBVS(off16);
    if (mnemonic == M6809Mnemonics::LBPL) return EncodeLBPL(off16);
    if (mnemonic == M6809Mnemonics::LBMI) return EncodeLBMI(off16);
    if (mnemonic == M6809Mnemonics::LBGE) return EncodeLBGE(off16);
    if (mnemonic == M6809Mnemonics::LBLT) return EncodeLBLT(off16);
    if (mnemonic == M6809Mnemonics::LBGT) return EncodeLBGT(off16);
    if (mnemonic == M6809Mnemonics::LBLE) return EncodeLBLE(off16);
  }

  // ── Register-pair instructions ────────────────────────────────────────────
  if (mnemonic == M6809Mnemonics::TFR) {
    uint8_t src = static_cast<uint8_t>((operand >> 4) & 0x0F);
    uint8_t dst = static_cast<uint8_t>(operand & 0x0F);
    return EncodeTFR(src, dst);
  }
  if (mnemonic == M6809Mnemonics::EXG) {
    uint8_t r1 = static_cast<uint8_t>((operand >> 4) & 0x0F);
    uint8_t r2 = static_cast<uint8_t>(operand & 0x0F);
    return EncodeEXG(r1, r2);
  }

  // ── Stack operations ──────────────────────────────────────────────────────
  if (mnemonic == M6809Mnemonics::PSHS)
    return EncodePSHS(static_cast<uint8_t>(operand & 0xFF));
  if (mnemonic == M6809Mnemonics::PULS)
    return EncodePULS(static_cast<uint8_t>(operand & 0xFF));
  if (mnemonic == M6809Mnemonics::PSHU)
    return EncodePSHU(static_cast<uint8_t>(operand & 0xFF));
  if (mnemonic == M6809Mnemonics::PULU)
    return EncodePULU(static_cast<uint8_t>(operand & 0xFF));

  // ── Table-driven memory-operand instructions ──────────────────────────────
  auto it = kOpcodeTable.find(mnemonic);
  if (it != kOpcodeTable.end()) {
    return EncodeMemInstr(it->second, operand, mode);
  }

  // Unknown mnemonic
  return {};
}

} // namespace xasm
