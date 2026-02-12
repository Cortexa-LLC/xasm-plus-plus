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
  auto trim = [](const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
      return std::string("");
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
  };

  std::string trimmed = trim(operand_str);

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
    if (trimmed.find("BC,") == 0 || trimmed.find("BC ,") == 0) {
      size_t comma_pos = trimmed.find(',');
      if (comma_pos != std::string::npos) {
        std::string value_part = trim(trimmed.substr(comma_pos + 1));
        if (!value_part.empty() && value_part[0] == '#') {
          return EncodeLD_BC_nn(static_cast<uint16_t>(operand));
        }
      }
    }
    // Check for "A, #$xx" pattern (immediate load to A)
    if (trimmed.find("A,") == 0 || trimmed.find("A ,") == 0) {
      size_t comma_pos = trimmed.find(',');
      if (comma_pos != std::string::npos) {
        std::string value_part = trim(trimmed.substr(comma_pos + 1));
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
    if (trimmed.find("A,") == 0 || trimmed.find("A ,") == 0) {
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

std::vector<uint8_t> CpuZ80::ToLittleEndian(uint16_t value) const {
  return {static_cast<uint8_t>(value & 0xFF),        // Low byte
          static_cast<uint8_t>((value >> 8) & 0xFF)}; // High byte
}

// ============================================================================
// 8-bit Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {Opcodes::LD_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_B_n(uint8_t value) const {
  return {Opcodes::LD_B_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_C_n(uint8_t value) const {
  return {Opcodes::LD_C_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_D_n(uint8_t value) const {
  return {Opcodes::LD_D_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_E_n(uint8_t value) const {
  return {Opcodes::LD_E_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_H_n(uint8_t value) const {
  return {Opcodes::LD_H_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_L_n(uint8_t value) const {
  return {Opcodes::LD_L_n, value};
}

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_BC_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::LD_BC_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_DE_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::LD_DE_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::LD_HL_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_SP_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::LD_SP_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_BC() const {
  return {Opcodes::LD_A_BC};
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_DE() const {
  return {Opcodes::LD_A_DE};
}

std::vector<uint8_t> CpuZ80::EncodeLD_BC_A() const {
  return {Opcodes::LD_BC_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_DE_A() const {
  return {Opcodes::LD_DE_A};
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_n(uint8_t value) const {
  return {Opcodes::LD_HL_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_addr(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::LD_HL_addr};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_addr_HL(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::LD_addr_HL};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_SP_HL() const {
  return {Opcodes::LD_SP_HL};
}

// ============================================================================
// Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADD_A_n(uint8_t value) const {
  return {Opcodes::ADD_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeSUB_n(uint8_t value) const {
  return {Opcodes::SUB_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeINC_A() const { return {Opcodes::INC_A}; }

std::vector<uint8_t> CpuZ80::EncodeDEC_A() const { return {Opcodes::DEC_A}; }

// ============================================================================
// Control Flow Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeNOP() const { return {Opcodes::NOP}; }

std::vector<uint8_t> CpuZ80::EncodeJP_nn(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::JP_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET() const { return {Opcodes::RET}; }

// ============================================================================
// Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_BC() const { return {Opcodes::PUSH_BC}; }

std::vector<uint8_t> CpuZ80::EncodePOP_BC() const { return {Opcodes::POP_BC}; }

// ============================================================================
// Bit Operations (CB Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0x40 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeSET(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0xC0 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeRES(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0x80 + (bit << 3) + reg;
  return {Opcodes::CB_PREFIX, opcode};
}

// ============================================================================
// IX Register Operations (DD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IX_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::DD_PREFIX, Opcodes::DD::LD_IX_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IX_d(int8_t displacement) const {
  return {Opcodes::DD_PREFIX, Opcodes::DD::LD_A_IX_d, static_cast<uint8_t>(displacement)};
}

// ============================================================================
// IY Register Operations (FD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IY_nn(uint16_t value) const {
  std::vector<uint8_t> result = {Opcodes::FD_PREFIX, Opcodes::FD::LD_IY_nn};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IY_d(int8_t displacement) const {
  return {Opcodes::FD_PREFIX, Opcodes::FD::LD_A_IY_d, static_cast<uint8_t>(displacement)};
}

// ============================================================================
// Register-to-Register Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_B() const { return {Opcodes::LD_A_B}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_C() const { return {Opcodes::LD_A_C}; }

std::vector<uint8_t> CpuZ80::EncodeLD_B_A() const { return {Opcodes::LD_B_A}; }

std::vector<uint8_t> CpuZ80::EncodeLD_C_A() const { return {Opcodes::LD_C_A}; }

// ============================================================================
// Additional Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADC_A_n(uint8_t value) const {
  return {Opcodes::ADC_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeSBC_A_n(uint8_t value) const {
  return {Opcodes::SBC_A_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeCP_n(uint8_t value) const {
  return {Opcodes::CP_n, value};
}

// ============================================================================
// Logical Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeAND_n(uint8_t value) const {
  return {Opcodes::AND_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeOR_n(uint8_t value) const {
  return {Opcodes::OR_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_n(uint8_t value) const {
  return {Opcodes::XOR_n, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_A() const { return {Opcodes::XOR_A}; }

// ============================================================================
// Branch Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeJR_e(int8_t offset) const {
  return {Opcodes::JR_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NZ_e(int8_t offset) const {
  return {Opcodes::JR_NZ_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_Z_e(int8_t offset) const {
  return {Opcodes::JR_Z_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NC_e(int8_t offset) const {
  return {Opcodes::JR_NC_e, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_C_e(int8_t offset) const {
  return {Opcodes::JR_C_e, static_cast<uint8_t>(offset)};
}

// ============================================================================
// Memory Access Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_addr(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::LD_A_addr};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_addr_A(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::LD_addr_A};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_HL() const { return {Opcodes::LD_A_HL}; }

std::vector<uint8_t> CpuZ80::EncodeLD_HL_A() const { return {Opcodes::LD_HL_A}; }

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeRLCA() const { return {Opcodes::RLCA}; }

std::vector<uint8_t> CpuZ80::EncodeRRCA() const { return {Opcodes::RRCA}; }

std::vector<uint8_t> CpuZ80::EncodeRLA() const { return {Opcodes::RLA}; }

std::vector<uint8_t> CpuZ80::EncodeRRA() const { return {Opcodes::RRA}; }

std::vector<uint8_t> CpuZ80::EncodeRLD() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::RLD};
}

std::vector<uint8_t> CpuZ80::EncodeRRD() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::RRD};
}

// ============================================================================
// Input/Output Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeIN_A_n(uint8_t port) const {
  return {Opcodes::IN_A_n, port};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_n_A(uint8_t port) const {
  return {Opcodes::OUT_n_A, port};
}

std::vector<uint8_t> CpuZ80::EncodeIN_B_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_B_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_C_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_C_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_D_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_D_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_E_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_E_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_H_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_H_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_L_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_L_C};
}

std::vector<uint8_t> CpuZ80::EncodeIN_A_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::IN_A_C};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_B() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_B};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_C() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_C};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_D() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_D};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_E() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_E};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_H() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_H};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_L() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_L};
}

std::vector<uint8_t> CpuZ80::EncodeOUT_C_A() const {
  return {Opcodes::ED_PREFIX, Opcodes::ED::OUT_C_A};
}

// ============================================================================
// Extended Instructions (ED Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_I_A() const { return {Opcodes::ED_PREFIX, Opcodes::ED::LD_I_A}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_I() const { return {Opcodes::ED_PREFIX, Opcodes::ED::LD_A_I}; }

std::vector<uint8_t> CpuZ80::EncodeLD_R_A() const { return {Opcodes::ED_PREFIX, Opcodes::ED::LD_R_A}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_R() const { return {Opcodes::ED_PREFIX, Opcodes::ED::LD_A_R}; }

std::vector<uint8_t> CpuZ80::EncodeLDIR() const { return {Opcodes::ED_PREFIX, Opcodes::ED::LDIR}; }

std::vector<uint8_t> CpuZ80::EncodeCPIR() const { return {Opcodes::ED_PREFIX, Opcodes::ED::CPIR}; }

std::vector<uint8_t> CpuZ80::EncodeNEG() const { return {Opcodes::ED_PREFIX, Opcodes::ED::NEG}; }

// ============================================================================
// Additional Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_DE() const { return {Opcodes::PUSH_DE}; }

std::vector<uint8_t> CpuZ80::EncodePUSH_HL() const { return {Opcodes::PUSH_HL}; }

std::vector<uint8_t> CpuZ80::EncodePUSH_AF() const { return {Opcodes::PUSH_AF}; }

std::vector<uint8_t> CpuZ80::EncodePOP_DE() const { return {Opcodes::POP_DE}; }

std::vector<uint8_t> CpuZ80::EncodePOP_HL() const { return {Opcodes::POP_HL}; }

std::vector<uint8_t> CpuZ80::EncodePOP_AF() const { return {Opcodes::POP_AF}; }

// ============================================================================
// Conditional Call and Return Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeCALL_nn(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::CALL_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeCALL_NZ_nn(uint16_t address) const {
  std::vector<uint8_t> result = {Opcodes::CALL_NZ_nn};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET_Z() const { return {Opcodes::RET_Z}; }

std::vector<uint8_t> CpuZ80::EncodeRET_NZ() const { return {Opcodes::RET_NZ}; }

} // namespace xasm
