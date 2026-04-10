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
#include "xasm++/cpu/mnemonics_6809.h"
#include "xasm++/cpu/opcodes_6809.h"
#include "xasm++/util/string_utils.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

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
static const std::unordered_map<M6809Mnemonic, InstrOpcodes> kOpcodeTable = {
  // -------------------------------------------------------------------------
  // 8-bit Load / Store — accumulator A
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LDA,  { 0x00, Opcodes6809::LDA_IMM,  Opcodes6809::LDA_DIR,  Opcodes6809::LDA_EXT,  Opcodes6809::LDA_IDX,  false } },
  { M6809Mnemonic::STA,  { 0x00, NO_OP,                  Opcodes6809::STA_DIR,  Opcodes6809::STA_EXT,  Opcodes6809::STA_IDX,  false } },

  // -------------------------------------------------------------------------
  // 8-bit Load / Store — accumulator B
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LDB,  { 0x00, Opcodes6809::LDB_IMM,  Opcodes6809::LDB_DIR,  Opcodes6809::LDB_EXT,  Opcodes6809::LDB_IDX,  false } },
  { M6809Mnemonic::STB,  { 0x00, NO_OP,                  Opcodes6809::STB_DIR,  Opcodes6809::STB_EXT,  Opcodes6809::STB_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — accumulator D
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LDD,  { 0x00, Opcodes6809::LDD_IMM,  Opcodes6809::LDD_DIR,  Opcodes6809::LDD_EXT,  Opcodes6809::LDD_IDX,  true  } },
  { M6809Mnemonic::STD,  { 0x00, NO_OP,                  Opcodes6809::STD_DIR,  Opcodes6809::STD_EXT,  Opcodes6809::STD_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — index register X
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LDX,  { 0x00, Opcodes6809::LDX_IMM,  Opcodes6809::LDX_DIR,  Opcodes6809::LDX_EXT,  Opcodes6809::LDX_IDX,  true  } },
  { M6809Mnemonic::STX,  { 0x00, NO_OP,                  Opcodes6809::STX_DIR,  Opcodes6809::STX_EXT,  Opcodes6809::STX_IDX,  false } },

  // -------------------------------------------------------------------------
  // 16-bit Load / Store — index register Y  (page-2 prefix)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LDY,  { 0x10, Opcodes6809::Page2::LDY_IMM, Opcodes6809::Page2::LDY_DIR, Opcodes6809::Page2::LDY_EXT, Opcodes6809::Page2::LDY_IDX, true  } },
  { M6809Mnemonic::STY,  { 0x10, NO_OP,                        Opcodes6809::Page2::STY_DIR, Opcodes6809::Page2::STY_EXT, Opcodes6809::Page2::STY_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — ADD
  // -------------------------------------------------------------------------
  { M6809Mnemonic::ADDA, { 0x00, Opcodes6809::ADDA_IMM, Opcodes6809::ADDA_DIR, Opcodes6809::ADDA_EXT, Opcodes6809::ADDA_IDX, false } },
  { M6809Mnemonic::ADDB, { 0x00, Opcodes6809::ADDB_IMM, Opcodes6809::ADDB_DIR, Opcodes6809::ADDB_EXT, Opcodes6809::ADDB_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — SUB
  // -------------------------------------------------------------------------
  { M6809Mnemonic::SUBA, { 0x00, Opcodes6809::SUBA_IMM, Opcodes6809::SUBA_DIR, Opcodes6809::SUBA_EXT, Opcodes6809::SUBA_IDX, false } },
  { M6809Mnemonic::SUBB, { 0x00, Opcodes6809::SUBB_IMM, Opcodes6809::SUBB_DIR, Opcodes6809::SUBB_EXT, Opcodes6809::SUBB_IDX, false } },

  // -------------------------------------------------------------------------
  // Arithmetic — CMP (compare without storing result)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::CMPA, { 0x00, Opcodes6809::CMPA_IMM, Opcodes6809::CMPA_DIR, Opcodes6809::CMPA_EXT, Opcodes6809::CMPA_IDX, false } },
  { M6809Mnemonic::CMPB, { 0x00, Opcodes6809::CMPB_IMM, Opcodes6809::CMPB_DIR, Opcodes6809::CMPB_EXT, Opcodes6809::CMPB_IDX, false } },
  { M6809Mnemonic::CMPX, { 0x00, Opcodes6809::CMPX_IMM, Opcodes6809::CMPX_DIR, Opcodes6809::CMPX_EXT, Opcodes6809::CMPX_IDX, true  } },

  // CMPY uses page-2 prefix; no indexed mode defined
  { M6809Mnemonic::CMPY, { 0x10, Opcodes6809::Page2::CMPY_IMM, Opcodes6809::Page2::CMPY_DIR, Opcodes6809::Page2::CMPY_EXT, NO_OP, true } },

  // -------------------------------------------------------------------------
  // Logical — AND
  // -------------------------------------------------------------------------
  { M6809Mnemonic::ANDA, { 0x00, Opcodes6809::ANDA_IMM, Opcodes6809::ANDA_DIR, Opcodes6809::ANDA_EXT, Opcodes6809::ANDA_IDX, false } },
  { M6809Mnemonic::ANDB, { 0x00, Opcodes6809::ANDB_IMM, Opcodes6809::ANDB_DIR, Opcodes6809::ANDB_EXT, Opcodes6809::ANDB_IDX, false } },

  // -------------------------------------------------------------------------
  // Logical — OR
  // -------------------------------------------------------------------------
  { M6809Mnemonic::ORA,  { 0x00, Opcodes6809::ORA_IMM,  Opcodes6809::ORA_DIR,  Opcodes6809::ORA_EXT,  Opcodes6809::ORA_IDX,  false } },
  { M6809Mnemonic::ORB,  { 0x00, Opcodes6809::ORB_IMM,  Opcodes6809::ORB_DIR,  Opcodes6809::ORB_EXT,  Opcodes6809::ORB_IDX,  false } },

  // -------------------------------------------------------------------------
  // Logical — XOR (EOR)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::EORA, { 0x00, Opcodes6809::EORA_IMM, Opcodes6809::EORA_DIR, Opcodes6809::EORA_EXT, Opcodes6809::EORA_IDX, false } },
  { M6809Mnemonic::EORB, { 0x00, Opcodes6809::EORB_IMM, Opcodes6809::EORB_DIR, Opcodes6809::EORB_EXT, Opcodes6809::EORB_IDX, false } },

  // -------------------------------------------------------------------------
  // Logical — BIT test (AND without storing result)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::BITA, { 0x00, Opcodes6809::BITA_IMM, Opcodes6809::BITA_DIR, Opcodes6809::BITA_EXT, Opcodes6809::BITA_IDX, false } },
  { M6809Mnemonic::BITB, { 0x00, Opcodes6809::BITB_IMM, Opcodes6809::BITB_DIR, Opcodes6809::BITB_EXT, Opcodes6809::BITB_IDX, false } },

  // -------------------------------------------------------------------------
  // Jump / Subroutine  (no immediate mode)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::JMP,  { 0x00, NO_OP, Opcodes6809::JMP_DIR, Opcodes6809::JMP_EXT, Opcodes6809::JMP_IDX, false } },
  { M6809Mnemonic::JSR,  { 0x00, NO_OP, Opcodes6809::JSR_DIR, Opcodes6809::JSR_EXT, Opcodes6809::JSR_IDX, false } },

  // -------------------------------------------------------------------------
  // Load Effective Address  (indexed mode only; treated via Extended for addr)
  // -------------------------------------------------------------------------
  { M6809Mnemonic::LEAX, { 0x00, NO_OP, NO_OP, Opcodes6809::LEAX, Opcodes6809::LEAX, false } },
  { M6809Mnemonic::LEAY, { 0x00, NO_OP, NO_OP, Opcodes6809::LEAY, Opcodes6809::LEAY, false } },
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
BuildIndexedPostByte(AddressingMode6809 mode, int32_t offset, // NOLINT(bugprone-easily-swappable-parameters)
                     uint8_t base_reg) {
  std::vector<uint8_t> result;
  uint8_t reg_bits = (base_reg & 0x03) << 5;

  switch (mode) {
  case AddressingMode6809::IndexedZeroOffset:
    result.push_back(0x84 | reg_bits);
    break;
  case AddressingMode6809::Indexed5BitOffset: {
    auto offset_5bit = static_cast<uint8_t>(offset & 0x1F);
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
  case AddressingMode6809::IndexedAccumA: // NOLINT(bugprone-branch-clone)
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
    if (offset == 0) { // NOLINT(bugprone-branch-clone)
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
  case AddressingMode6809::Immediate8: // NOLINT(bugprone-branch-clone)
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
    if (entry.idx == NO_OP) {
      return {};
}
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

std::vector<uint8_t> Cpu6809::EncodeNOP() { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::NOP}; // NOP opcode
}

std::vector<uint8_t> Cpu6809::EncodeRTS() { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::RTS}; // RTS opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRA() { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::CLRA}; // CLRA opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRB() { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::CLRB}; // CLRB opcode
}

// Shift and Rotate Instructions
std::vector<uint8_t> Cpu6809::EncodeASLA() { return {Opcodes6809::ASLA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeASLB() { return {Opcodes6809::ASLB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeASRA() { return {Opcodes6809::ASRA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeASRB() { return {Opcodes6809::ASRB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeLSRA() { return {Opcodes6809::LSRA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeLSRB() { return {Opcodes6809::LSRB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeROLA() { return {Opcodes6809::ROLA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeROLB() { return {Opcodes6809::ROLB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeRORA() { return {Opcodes6809::RORA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeRORB() { return {Opcodes6809::RORB}; } // NOLINT(readability-convert-member-functions-to-static)

// Increment / Decrement
std::vector<uint8_t> Cpu6809::EncodeINCA() { return {Opcodes6809::INCA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeINCB() { return {Opcodes6809::INCB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeDECA() { return {Opcodes6809::DECA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeDECB() { return {Opcodes6809::DECB}; } // NOLINT(readability-convert-member-functions-to-static)

// Test and Complement/Negate
std::vector<uint8_t> Cpu6809::EncodeTSTA() { return {Opcodes6809::TSTA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeTSTB() { return {Opcodes6809::TSTB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeCOMA() { return {Opcodes6809::COMA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeCOMB() { return {Opcodes6809::COMB}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeNEGA() { return {Opcodes6809::NEGA}; } // NOLINT(readability-convert-member-functions-to-static)

std::vector<uint8_t> Cpu6809::EncodeNEGB() { return {Opcodes6809::NEGB}; } // NOLINT(readability-convert-member-functions-to-static)

// ============================================================================
// Memory-operand instructions  (delegates to kOpcodeTable via EncodeMemInstr)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDB(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDD(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LDD), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDX(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LDX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLDY(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LDY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTA(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::STA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTB(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::STB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTD(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::STD), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTX(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::STX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSTY(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::STY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeADDA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ADDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeADDB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ADDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSUBA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::SUBA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeSUBB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::SUBB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::CMPA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::CMPB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPX(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::CMPX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeCMPY(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::CMPY), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeANDA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ANDA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeANDB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ANDB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeORA(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ORA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeORB(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::ORB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeEORA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::EORA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeEORB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::EORB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeBITA(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::BITA), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeBITB(uint32_t operand,
                                         AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::BITB), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeJMP(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::JMP), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeJSR(uint32_t operand,
                                        AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::JSR), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLEAX(uint32_t operand,
                                          AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LEAX), operand, mode);
}

std::vector<uint8_t> Cpu6809::EncodeLEAY(uint32_t operand,
                                          AddressingMode6809 mode) {
  return EncodeMemInstr(kOpcodeTable.at(M6809Mnemonic::LEAY), operand, mode);
}

// ============================================================================
// Short Branch Instructions (8-bit relative addressing)
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeBRA(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BRA, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBEQ(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BEQ, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBNE(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BNE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCC(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BCC, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBCS(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BCS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBMI(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BMI, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBPL(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BPL, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVS(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BVS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBVC(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BVC, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGE(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BGE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBGT(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BGT, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLE(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLE, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLT(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLT, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBHI(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BHI, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBLS(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BLS, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6809::EncodeBSR(int32_t offset, // NOLINT(readability-convert-member-functions-to-static)
                                        AddressingMode6809 mode) {
  if (mode == AddressingMode6809::Relative8) {
    return {Opcodes6809::BSR, static_cast<uint8_t>(offset & 0xFF)};
  }
  return {};
}

// ============================================================================
// Long Branch Instructions (16-bit relative addressing)
// ============================================================================

/// Helper: build a 4-byte long branch: page-2 prefix + opcode + 16-bit offset
static std::vector<uint8_t> MakeLongBranch(uint8_t opcode, int16_t offset) { // NOLINT(bugprone-easily-swappable-parameters)
  std::vector<uint8_t> result = {Opcodes6809::PAGE2_PREFIX, opcode};
  auto offset_bytes = encoding::ToBigEndian16(static_cast<uint16_t>(offset));
  result.insert(result.end(), offset_bytes.begin(), offset_bytes.end());
  return result;
}

std::vector<uint8_t> Cpu6809::EncodeLBRA(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBRA, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBRN(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBRN, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBEQ(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBEQ, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBNE(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBNE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBCC(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBCC, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBCS(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBCS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBMI(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBMI, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBPL(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBPL, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBVS(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBVS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBVC(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBVC, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBGE(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBGE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLT(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBLT, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBGT(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBGT, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLE(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBLE, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBHI(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBHI, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBLS(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBLS, offset);
}
std::vector<uint8_t> Cpu6809::EncodeLBSR(int16_t offset) {
  return MakeLongBranch(Opcodes6809::Page2::LBSR, offset);
}

// ============================================================================
// Register-transfer and Exchange
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodeTFR(uint8_t src, uint8_t dst) { // NOLINT(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters)
  // Post-byte format: high nibble = source, low nibble = destination
  uint8_t postbyte = (src << 4) | dst;
  return {Opcodes6809::TFR, postbyte};
}

std::vector<uint8_t> Cpu6809::EncodeEXG(uint8_t reg1, uint8_t reg2) { // NOLINT(readability-convert-member-functions-to-static)
  // Post-byte format: high nibble = reg1, low nibble = reg2
  uint8_t postbyte = (reg1 << 4) | reg2;
  return {Opcodes6809::EXG, postbyte};
}

// ============================================================================
// Stack push / pull
// ============================================================================

std::vector<uint8_t> Cpu6809::EncodePSHS(uint8_t mask) { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::PSHS, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULS(uint8_t mask) { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::PULS, mask};
}

std::vector<uint8_t> Cpu6809::EncodePSHU(uint8_t mask) { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::PSHU, mask};
}

std::vector<uint8_t> Cpu6809::EncodePULU(uint8_t mask) { // NOLINT(readability-convert-member-functions-to-static)
  return {Opcodes6809::PULU, mask};
}

// ============================================================================
// Indexed post-byte encoder
// ============================================================================

std::vector<uint8_t>
Cpu6809::EncodeIndexedPostByte(AddressingMode6809 mode, int32_t offset, // NOLINT(bugprone-easily-swappable-parameters)
                               uint8_t base_reg) {
  // Delegate to the free-function implementation (shared with EncodeMemInstr).
  return BuildIndexedPostByte(mode, offset, base_reg);
}

// ============================================================================
// Instruction Size Calculation
// ============================================================================

size_t Cpu6809::CalculateInstructionSize(AddressingMode6809 mode) { // NOLINT(readability-convert-member-functions-to-static)
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

bool Cpu6809::HasOpcode(const std::string &mnemonic) const { // NOLINT(readability-convert-member-functions-to-static)
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // ParseM6809Mnemonic returns Unknown for unrecognised strings.
  return ParseM6809Mnemonic(upper) != M6809Mnemonic::Unknown;
}

// ============================================================================
// EncodeInstruction — main dispatch
// ============================================================================

std::vector<uint8_t>
Cpu6809::EncodeInstruction(const std::string &mnemonic, uint32_t operand, // NOLINT(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters)
                           const std::string &operand_str) const {
  // ── Parse mnemonic string to enum (single map lookup) ───────────────────
  const M6809Mnemonic mn = ParseM6809Mnemonic(mnemonic); // NOLINT(cppcoreguidelines-init-variables)

  // Determine addressing mode from operand_str
  std::string trimmed = util::Trim(operand_str);
  AddressingMode6809 mode = AddressingMode6809::Inherent;

  if (!trimmed.empty()) {
    // Immediate mode: #value
    if (trimmed[0] == '#') {
      // 16-bit immediate for instructions that operate on 16-bit registers
      bool is_16bit =
          (mn == M6809Mnemonic::LDD  || mn == M6809Mnemonic::LDX  ||
           mn == M6809Mnemonic::LDY  || mn == M6809Mnemonic::LDU  ||
           mn == M6809Mnemonic::LDS  ||
           mn == M6809Mnemonic::CMPX || mn == M6809Mnemonic::CMPY ||
           mn == M6809Mnemonic::CMPU || mn == M6809Mnemonic::CMPS);
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

  // ── Switch on parsed enum (O(1), compiler-exhaustion-checked) ─────────────
  const auto off   = static_cast<int32_t>(operand);
  const auto off16 = static_cast<int16_t>(operand);

  switch (mn) {
    // ── Inherent (no-operand) ───────────────────────────────────────────────
    case M6809Mnemonic::NOP:  return EncodeNOP(); // NOLINT(bugprone-branch-clone)
    case M6809Mnemonic::RTS:  return EncodeRTS();
    case M6809Mnemonic::CLRA: return EncodeCLRA();
    case M6809Mnemonic::CLRB: return EncodeCLRB();
    case M6809Mnemonic::ASLA: return EncodeASLA();
    case M6809Mnemonic::ASLB: return EncodeASLB();
    case M6809Mnemonic::ASRA: return EncodeASRA();
    case M6809Mnemonic::ASRB: return EncodeASRB();
    case M6809Mnemonic::LSRA: return EncodeLSRA();
    case M6809Mnemonic::LSRB: return EncodeLSRB();
    case M6809Mnemonic::ROLA: return EncodeROLA();
    case M6809Mnemonic::ROLB: return EncodeROLB();
    case M6809Mnemonic::RORA: return EncodeRORA();
    case M6809Mnemonic::RORB: return EncodeRORB();
    case M6809Mnemonic::INCA: return EncodeINCA();
    case M6809Mnemonic::INCB: return EncodeINCB();
    case M6809Mnemonic::DECA: return EncodeDECA();
    case M6809Mnemonic::DECB: return EncodeDECB();
    case M6809Mnemonic::TSTA: return EncodeTSTA();
    case M6809Mnemonic::TSTB: return EncodeTSTB();
    case M6809Mnemonic::COMA: return EncodeCOMA();
    case M6809Mnemonic::COMB: return EncodeCOMB();
    case M6809Mnemonic::NEGA: return EncodeNEGA();
    case M6809Mnemonic::NEGB: return EncodeNEGB();

    // ── Short branches ──────────────────────────────────────────────────────
    case M6809Mnemonic::BRA: return EncodeBRA(off, mode);
    case M6809Mnemonic::BEQ: return EncodeBEQ(off, mode);
    case M6809Mnemonic::BNE: return EncodeBNE(off, mode);
    case M6809Mnemonic::BCC: [[fallthrough]];
    case M6809Mnemonic::BHS: return EncodeBCC(off, mode);
    case M6809Mnemonic::BCS: [[fallthrough]];
    case M6809Mnemonic::BLO: return EncodeBCS(off, mode);
    case M6809Mnemonic::BMI: return EncodeBMI(off, mode);
    case M6809Mnemonic::BPL: return EncodeBPL(off, mode);
    case M6809Mnemonic::BVS: return EncodeBVS(off, mode);
    case M6809Mnemonic::BVC: return EncodeBVC(off, mode);
    case M6809Mnemonic::BGE: return EncodeBGE(off, mode);
    case M6809Mnemonic::BGT: return EncodeBGT(off, mode);
    case M6809Mnemonic::BLE: return EncodeBLE(off, mode);
    case M6809Mnemonic::BLT: return EncodeBLT(off, mode);
    case M6809Mnemonic::BHI: return EncodeBHI(off, mode);
    case M6809Mnemonic::BLS: return EncodeBLS(off, mode);
    case M6809Mnemonic::BSR: return EncodeBSR(off, mode);
    case M6809Mnemonic::BRN: return {}; // BRN (Branch Never) - no-op

    // ── Long branches ───────────────────────────────────────────────────────
    case M6809Mnemonic::LBRA: return EncodeLBRA(off16); // NOLINT(bugprone-branch-clone)
    case M6809Mnemonic::LBSR: return EncodeLBSR(off16);
    case M6809Mnemonic::LBRN: return EncodeLBRN(off16);
    case M6809Mnemonic::LBHI: return EncodeLBHI(off16);
    case M6809Mnemonic::LBLS: return EncodeLBLS(off16);
    case M6809Mnemonic::LBCC: [[fallthrough]];
    case M6809Mnemonic::LBHS: return EncodeLBCC(off16);
    case M6809Mnemonic::LBCS: [[fallthrough]];
    case M6809Mnemonic::LBLO: return EncodeLBCS(off16);
    case M6809Mnemonic::LBNE: return EncodeLBNE(off16);
    case M6809Mnemonic::LBEQ: return EncodeLBEQ(off16);
    case M6809Mnemonic::LBVC: return EncodeLBVC(off16);
    case M6809Mnemonic::LBVS: return EncodeLBVS(off16);
    case M6809Mnemonic::LBPL: return EncodeLBPL(off16);
    case M6809Mnemonic::LBMI: return EncodeLBMI(off16);
    case M6809Mnemonic::LBGE: return EncodeLBGE(off16);
    case M6809Mnemonic::LBLT: return EncodeLBLT(off16);
    case M6809Mnemonic::LBGT: return EncodeLBGT(off16);
    case M6809Mnemonic::LBLE: return EncodeLBLE(off16);

    // ── Register-pair instructions ──────────────────────────────────────────
    case M6809Mnemonic::TFR: {
      auto src = static_cast<uint8_t>((operand >> 4) & 0x0F);
      auto dst = static_cast<uint8_t>(operand & 0x0F);
      return EncodeTFR(src, dst);
    }
    case M6809Mnemonic::EXG: {
      auto r1 = static_cast<uint8_t>((operand >> 4) & 0x0F);
      auto r2 = static_cast<uint8_t>(operand & 0x0F);
      return EncodeEXG(r1, r2);
    }

    // ── Stack operations ────────────────────────────────────────────────────
    case M6809Mnemonic::PSHS: return EncodePSHS(static_cast<uint8_t>(operand & 0xFF)); // NOLINT(bugprone-branch-clone)
    case M6809Mnemonic::PULS: return EncodePULS(static_cast<uint8_t>(operand & 0xFF));
    case M6809Mnemonic::PSHU: return EncodePSHU(static_cast<uint8_t>(operand & 0xFF));
    case M6809Mnemonic::PULU: return EncodePULU(static_cast<uint8_t>(operand & 0xFF));

    // ── Table-driven memory-operand instructions ────────────────────────────
    case M6809Mnemonic::LDA:  case M6809Mnemonic::STA:
    case M6809Mnemonic::LDB:  case M6809Mnemonic::STB:
    case M6809Mnemonic::LDD:  case M6809Mnemonic::STD:
    case M6809Mnemonic::LDX:  case M6809Mnemonic::STX:
    case M6809Mnemonic::LDY:  case M6809Mnemonic::STY:
    case M6809Mnemonic::LDU:  case M6809Mnemonic::STU:
    case M6809Mnemonic::LDS:  case M6809Mnemonic::STS:
    case M6809Mnemonic::ADDA: case M6809Mnemonic::ADCA:
    case M6809Mnemonic::SUBA: case M6809Mnemonic::SBCA:
    case M6809Mnemonic::CMPA:
    case M6809Mnemonic::ADDB: case M6809Mnemonic::ADCB:
    case M6809Mnemonic::SUBB: case M6809Mnemonic::SBCB:
    case M6809Mnemonic::CMPB:
    case M6809Mnemonic::ADDD: case M6809Mnemonic::SUBD:
    case M6809Mnemonic::CMPX: case M6809Mnemonic::CMPY:
    case M6809Mnemonic::CMPU: case M6809Mnemonic::CMPS:
    case M6809Mnemonic::CMPD:
    case M6809Mnemonic::ANDA: case M6809Mnemonic::ORA:
    case M6809Mnemonic::EORA: case M6809Mnemonic::BITA:
    case M6809Mnemonic::ANDB: case M6809Mnemonic::ORB:
    case M6809Mnemonic::EORB: case M6809Mnemonic::BITB:
    case M6809Mnemonic::ANDCC: case M6809Mnemonic::ORCC:
    case M6809Mnemonic::CLR:  case M6809Mnemonic::ASL:
    case M6809Mnemonic::ASR:  case M6809Mnemonic::LSR:
    case M6809Mnemonic::ROL:  case M6809Mnemonic::ROR:
    case M6809Mnemonic::INC:  case M6809Mnemonic::DEC:
    case M6809Mnemonic::TST:  case M6809Mnemonic::NEG:
    case M6809Mnemonic::JMP:  case M6809Mnemonic::JSR:
    case M6809Mnemonic::LEAX: case M6809Mnemonic::LEAY:
    case M6809Mnemonic::LEAS: case M6809Mnemonic::LEAU:
    case M6809Mnemonic::RTI:  case M6809Mnemonic::MUL:
    case M6809Mnemonic::SEX:  case M6809Mnemonic::SWI:
    case M6809Mnemonic::SWI2: case M6809Mnemonic::SWI3:
    case M6809Mnemonic::SYNC: case M6809Mnemonic::CWAI:
    case M6809Mnemonic::ABX:  case M6809Mnemonic::DAA: {
      auto it = kOpcodeTable.find(mn);
      if (it != kOpcodeTable.end()) {
        return EncodeMemInstr(it->second, operand, mode);
      }
      return {};
    }

    // ── Unknown / unimplemented ─────────────────────────────────────────────
    case M6809Mnemonic::Unknown:
      return {};
  }

  return {};
}


} // namespace xasm
