/**
 * @file cpu_z80.cpp
 * @brief Implementation of Zilog Z80 CPU Plugin
 *
 * This file implements instruction encoding for the Zilog Z80 processor.
 *
 * @note Implementation follows TDD approach: RED -> GREEN -> REFACTOR
 */

#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/cpu/opcodes_z80.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <unordered_set>

namespace xasm {

using namespace Z80Mnemonics;
namespace Opcodes = Z80Opcodes;

// ============================================================================
// CpuPlugin Interface Implementation
// ============================================================================

std::vector<uint8_t>
CpuZ80::EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                          const std::string &operand_str) const {
  // Helper to trim whitespace
  std::string trimmed = util::Trim(operand_str);

  // NOP instruction
  if (mnemonic == NOP) {
    return EncodeNOP();
  }

  // RET instruction
  if (mnemonic == RET) {
    return EncodeRET();
  }

  // LD instruction - various addressing modes
  if (mnemonic == LD) {
    // Check for "BC, #$xxxx" pattern (16-bit immediate to BC)
    if (trimmed.starts_with("BC,") || trimmed.starts_with("BC ,")) {
      size_t comma_pos = trimmed.find(',');
      if (comma_pos != std::string::npos) {
        std::string value_part = util::Trim(trimmed.substr(comma_pos + 1));
        if (!value_part.empty() && value_part[0] == '#') {
          return EncodeLD_BC_nn(static_cast<uint16_t>(operand));
        }
      }
    }
    // Check for "A, #$xx" pattern (immediate load to A)
    if (trimmed.starts_with("A,") || trimmed.starts_with("A ,")) {
      size_t comma_pos = trimmed.find(',');
      if (comma_pos != std::string::npos) {
        std::string value_part = util::Trim(trimmed.substr(comma_pos + 1));
        // Check for immediate mode indicator (#)
        if (!value_part.empty() && value_part[0] == '#') {
          return EncodeLD_A_n(static_cast<uint8_t>(operand));
        }
      }
    }
  }

  // ADD instruction
  if (mnemonic == ADD) {
    // Check for "A, #$xx" pattern (add immediate to A)
    if (trimmed.starts_with("A,") || trimmed.starts_with("A ,")) {
      return EncodeADD_A_n(static_cast<uint8_t>(operand));
    }
  }

  // SUB instruction
  if (mnemonic == SUB) {
    // SUB n (immediate subtract from A)
    return EncodeSUB_n(static_cast<uint8_t>(operand));
  }

  // INC instruction
  if (mnemonic == INC) {
    if (trimmed == "A") {
      return EncodeINC_A();
    }
  }

  // DEC instruction
  if (mnemonic == DEC) {
    if (trimmed == "A") {
      return EncodeDEC_A();
    }
  }

  // XOR instruction
  if (mnemonic == XOR) {
    if (trimmed == "A") {
      return EncodeXOR_A();
    }
  }

  // JP instruction - absolute jump
  if (mnemonic == JP) {
    // Simple absolute address (no condition)
    return EncodeJP_nn(static_cast<uint16_t>(operand));
  }

  // JR instruction - relative jump
  if (mnemonic == JR) {
    // Simple relative jump (no condition)
    return EncodeJR_e(static_cast<int8_t>(operand));
  }

  // Return empty vector if instruction not recognized
  return {};
}

// ============================================================================
// Helper Methods
// ============================================================================

std::vector<uint8_t> CpuZ80::ToLittleEndian(uint16_t value) {
  return {static_cast<uint8_t>(value & 0xFF),         // Low byte
          static_cast<uint8_t>((value >> 8) & 0xFF)}; // High byte
}

// ============================================================================
// 8-bit Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) {
  return {Opcodes::LD_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_B_n(uint8_t value) {
  return {Opcodes::LD_B_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_C_n(uint8_t value) {
  return {Opcodes::LD_C_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_D_n(uint8_t value) {
  return {Opcodes::LD_D_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_E_n(uint8_t value) {
  return {Opcodes::LD_E_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_H_n(uint8_t value) {
  return {Opcodes::LD_H_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_L_n(uint8_t value) {
  return {Opcodes::LD_L_n, value};
}

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_BC_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::LD_BC_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_DE_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::LD_DE_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::LD_HL_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_SP_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::LD_SP_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_BC() {
  return {Opcodes::LD_A_BC};
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_DE() {
  return {Opcodes::LD_A_DE};
}

std::vector<uint8_t> CpuZ80::EncodeLD_BC_A() {
  return {Opcodes::LD_BC_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_DE_A() {
  return {Opcodes::LD_DE_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_n(uint8_t value) {
  return {Opcodes::LD_HL_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_addr(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::LD_HL_addr};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_addr_HL(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::LD_addr_HL};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_SP_HL() {
  return {Opcodes::LD_SP_HL};
}

// ============================================================================
// Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADD_A_n(uint8_t value) {
  return {Opcodes::ADD_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeSUB_n(uint8_t value) {
  return {Opcodes::SUB_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeINC_A() { return {Opcodes::INC_A}; }

std::vector<uint8_t> CpuZ80::EncodeDEC_A() { return {Opcodes::DEC_A}; }

// ============================================================================
// Control Flow Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeNOP() { return {Opcodes::NOP}; }

std::vector<uint8_t> CpuZ80::EncodeJP_nn(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::JP_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET() { return {Opcodes::RET}; }

// ============================================================================
// Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_BC() {
  return {Opcodes::PUSH_BC};
}

std::vector<uint8_t> CpuZ80::EncodePOP_BC() { return {Opcodes::POP_BC}; }

// ============================================================================
// Bit Operations (CB Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) {
  uint8_t opcode = 0x40 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeSET(uint8_t bit, uint8_t reg) {
  uint8_t opcode = 0xC0 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeRES(uint8_t bit, uint8_t reg) {
  uint8_t opcode = 0x80 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

// ============================================================================
// IX Register Operations (DD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IX_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::DD_PREFIX, Opcodes::DD::LD_IX_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IX_d(int8_t displacement) {
  return {Opcodes::DD_PREFIX, Opcodes::DD::LD_A_IX_d,
          static_cast<uint8_t>(displacement)};
}

// ============================================================================
// IY Register Operations (FD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IY_nn(uint16_t value) {
  std::vector<uint8_t> result = {Opcodes::FD_PREFIX, Opcodes::FD::LD_IY_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IY_d(int8_t displacement) {
  return {Opcodes::FD_PREFIX, Opcodes::FD::LD_A_IY_d,
          static_cast<uint8_t>(displacement)};
}

// ============================================================================
// Register-to-Register Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_B() { return {Opcodes::LD_A_B}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_C() { return {Opcodes::LD_A_C}; }

std::vector<uint8_t> CpuZ80::EncodeLD_B_A() { return {Opcodes::LD_B_A}; }

std::vector<uint8_t> CpuZ80::EncodeLD_C_A() { return {Opcodes::LD_C_A}; }

// ============================================================================
// Additional Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADC_A_n(uint8_t value) {
  return {Opcodes::ADC_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeSBC_A_n(uint8_t value) {
  return {Opcodes::SBC_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeCP_n(uint8_t value) {
  return {Opcodes::CP_n, value};
}

// ============================================================================
// Logical Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeAND_n(uint8_t value) {
  return {Opcodes::AND_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeOR_n(uint8_t value) {
  return {Opcodes::OR_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_n(uint8_t value) {
  return {Opcodes::XOR_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_A() { return {Opcodes::XOR_A}; }

// ============================================================================
// Branch Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeJR_e(int8_t offset) {
  return {Opcodes::JR_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NZ_e(int8_t offset) {
  return {Opcodes::JR_NZ_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_Z_e(int8_t offset) {
  return {Opcodes::JR_Z_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NC_e(int8_t offset) {
  return {Opcodes::JR_NC_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_C_e(int8_t offset) {
  return {Opcodes::JR_C_e, static_cast<uint8_t>(offset)};
}

// ============================================================================
// Memory Access Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_addr(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::LD_A_addr};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_addr_A(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::LD_addr_A};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_HL() {
  return {Opcodes::LD_A_HL};
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_A() {
  return {Opcodes::LD_HL_A};
}

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeRLCA() { return {Opcodes::RLCA}; }

std::vector<uint8_t> CpuZ80::EncodeRRCA() { return {Opcodes::RRCA}; }

std::vector<uint8_t> CpuZ80::EncodeRLA() { return {Opcodes::RLA}; }

std::vector<uint8_t> CpuZ80::EncodeRRA() { return {Opcodes::RRA}; }

std::vector<uint8_t> CpuZ80::EncodeRLD() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::RLD};
}

std::vector<uint8_t> CpuZ80::EncodeRRD() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::RRD};
}

// ============================================================================
// Input/Output Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeIN_A_n(uint8_t port) {
  return {Opcodes::IN_A_n, port};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_n_A(uint8_t port) {
  return {Opcodes::OUT_n_A, port};
}

std::vector<uint8_t> CpuZ80::EncodeIN_B_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_B_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_C_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_C_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_D_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_D_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_E_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_E_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_H_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_H_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_L_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_L_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_A_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_A_C};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_B() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_B};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_C() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_C};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_D() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_D};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_E() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_E};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_H() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_H};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_L() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_L};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_A() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_A};
}

// ============================================================================
// Extended Instructions (ED Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_I_A() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::LD_I_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_I() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::LD_A_I};
}

std::vector<uint8_t> CpuZ80::EncodeLD_R_A() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::LD_R_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_R() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::LD_A_R};
}

std::vector<uint8_t> CpuZ80::EncodeLDIR() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::LDIR};
}

std::vector<uint8_t> CpuZ80::EncodeCPIR() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::CPIR};
}

std::vector<uint8_t> CpuZ80::EncodeNEG() {
  return {Opcodes::ED_PREFIX, Opcodes::ED::NEG};
}

// ============================================================================
// Additional Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_DE() {
  return {Opcodes::PUSH_DE};
}

std::vector<uint8_t> CpuZ80::EncodePUSH_HL() {
  return {Opcodes::PUSH_HL};
}

std::vector<uint8_t> CpuZ80::EncodePUSH_AF() {
  return {Opcodes::PUSH_AF};
}

std::vector<uint8_t> CpuZ80::EncodePOP_DE() { return {Opcodes::POP_DE}; }

std::vector<uint8_t> CpuZ80::EncodePOP_HL() { return {Opcodes::POP_HL}; }

std::vector<uint8_t> CpuZ80::EncodePOP_AF() { return {Opcodes::POP_AF}; }

// ============================================================================
// Conditional Call and Return Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeCALL_nn(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::CALL_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeCALL_NZ_nn(uint16_t address) {
  std::vector<uint8_t> result = {Opcodes::CALL_NZ_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET_Z() { return {Opcodes::RET_Z}; }

std::vector<uint8_t> CpuZ80::EncodeRET_NZ() { return {Opcodes::RET_NZ}; }

// ============================================================================
// CpuPlugin Interface Implementation - HasOpcode()
// ============================================================================

/**
 * @brief Check if a mnemonic is a valid opcode for the Z80
 *
 * @param mnemonic Instruction mnemonic (e.g., "LD", "ADD", "JP")
 * @return true if mnemonic is a valid opcode, false otherwise
 */
bool CpuZ80::HasOpcode(const std::string &mnemonic) const {
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Create static set of all Z80 mnemonics for O(1) lookup
  static const std::unordered_set<std::string> valid_opcodes = {
      // Load/Store
      Z80Mnemonics::LD, Z80Mnemonics::PUSH, Z80Mnemonics::POP,

      // Arithmetic
      Z80Mnemonics::ADD, Z80Mnemonics::ADC, Z80Mnemonics::SUB,
      Z80Mnemonics::SBC, Z80Mnemonics::INC, Z80Mnemonics::DEC,

      // Logical
      Z80Mnemonics::AND, Z80Mnemonics::OR, Z80Mnemonics::XOR, Z80Mnemonics::CP,

      // Branch/Jump
      Z80Mnemonics::JP, Z80Mnemonics::JR, Z80Mnemonics::CALL, Z80Mnemonics::RET,
      Z80Mnemonics::RST, Z80Mnemonics::DJNZ,

      // Rotate/Shift
      Z80Mnemonics::RLCA, Z80Mnemonics::RRCA, Z80Mnemonics::RLA,
      Z80Mnemonics::RRA, Z80Mnemonics::RLC, Z80Mnemonics::RRC, Z80Mnemonics::RL,
      Z80Mnemonics::RR, Z80Mnemonics::SLA, Z80Mnemonics::SRA, Z80Mnemonics::SRL,

      // Bit Manipulation
      Z80Mnemonics::BIT, Z80Mnemonics::SET, Z80Mnemonics::RES,

      // Miscellaneous
      Z80Mnemonics::NOP, Z80Mnemonics::HALT, Z80Mnemonics::DI, Z80Mnemonics::EI,
      Z80Mnemonics::NEG, Z80Mnemonics::CPL, Z80Mnemonics::CCF,
      Z80Mnemonics::SCF, Z80Mnemonics::DAA,

      // Block Transfer/Search
      Z80Mnemonics::LDI, Z80Mnemonics::LDIR, Z80Mnemonics::LDD,
      Z80Mnemonics::LDDR, Z80Mnemonics::CPI, Z80Mnemonics::CPIR,
      Z80Mnemonics::CPD, Z80Mnemonics::CPDR,

      // Input/Output
      Z80Mnemonics::IN, Z80Mnemonics::OUT, Z80Mnemonics::INI,
      Z80Mnemonics::INIR, Z80Mnemonics::IND, Z80Mnemonics::INDR,
      Z80Mnemonics::OUTI, Z80Mnemonics::OTIR, Z80Mnemonics::OUTD,
      Z80Mnemonics::OTDR,

      // Exchange
      Z80Mnemonics::EX, Z80Mnemonics::EXX,

      // Interrupt
      Z80Mnemonics::RETI, Z80Mnemonics::RETN, Z80Mnemonics::IM};

  return valid_opcodes.contains(upper);
}

} // namespace xasm
