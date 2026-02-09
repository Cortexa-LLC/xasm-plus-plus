/**
 * @file cpu_z80.cpp
 * @brief Implementation of Zilog Z80 CPU Plugin
 *
 * This file implements instruction encoding for the Zilog Z80 processor.
 *
 * @note Implementation follows TDD approach: RED -> GREEN -> REFACTOR
 */

#include "xasm++/cpu/cpu_z80.h"

namespace xasm {

// ============================================================================
// CpuPlugin Interface Implementation
// ============================================================================

std::vector<uint8_t>
CpuZ80::EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                          const std::string &operand_str) const {
  // TODO: Implement generic instruction encoding
  (void)mnemonic;     // Suppress unused parameter warning
  (void)operand;      // Suppress unused parameter warning
  (void)operand_str;  // Suppress unused parameter warning
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
  return {0x3E, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_B_n(uint8_t value) const {
  return {0x06, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_C_n(uint8_t value) const {
  return {0x0E, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_D_n(uint8_t value) const {
  return {0x16, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_E_n(uint8_t value) const {
  return {0x1E, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_H_n(uint8_t value) const {
  return {0x26, value};
}

std::vector<uint8_t> CpuZ80::EncodeLD_L_n(uint8_t value) const {
  return {0x2E, value};
}

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_BC_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0x01};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_DE_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0x11};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_HL_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0x21};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_SP_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0x31};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

// ============================================================================
// Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADD_A_n(uint8_t value) const {
  return {0xC6, value};
}

std::vector<uint8_t> CpuZ80::EncodeSUB_n(uint8_t value) const {
  return {0xD6, value};
}

std::vector<uint8_t> CpuZ80::EncodeINC_A() const { return {0x3C}; }

std::vector<uint8_t> CpuZ80::EncodeDEC_A() const { return {0x3D}; }

// ============================================================================
// Control Flow Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeNOP() const { return {0x00}; }

std::vector<uint8_t> CpuZ80::EncodeJP_nn(uint16_t address) const {
  std::vector<uint8_t> result = {0xC3};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET() const { return {0xC9}; }

// ============================================================================
// Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_BC() const { return {0xC5}; }

std::vector<uint8_t> CpuZ80::EncodePOP_BC() const { return {0xC1}; }

// ============================================================================
// Bit Operations (CB Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0x40 + (bit << 3) + reg;
  return {0xCB, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeSET(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0xC0 + (bit << 3) + reg;
  return {0xCB, opcode};
}

std::vector<uint8_t> CpuZ80::EncodeRES(uint8_t bit, uint8_t reg) const {
  uint8_t opcode = 0x80 + (bit << 3) + reg;
  return {0xCB, opcode};
}

// ============================================================================
// IX Register Operations (DD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IX_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0xDD, 0x21};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IX_d(int8_t displacement) const {
  return {0xDD, 0x7E, static_cast<uint8_t>(displacement)};
}

// ============================================================================
// IY Register Operations (FD Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_IY_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0xFD, 0x21};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_IY_d(int8_t displacement) const {
  return {0xFD, 0x7E, static_cast<uint8_t>(displacement)};
}

// ============================================================================
// Register-to-Register Load Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_B() const { return {0x78}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_C() const { return {0x79}; }

std::vector<uint8_t> CpuZ80::EncodeLD_B_A() const { return {0x47}; }

std::vector<uint8_t> CpuZ80::EncodeLD_C_A() const { return {0x4F}; }

// ============================================================================
// Additional Arithmetic Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeADC_A_n(uint8_t value) const {
  return {0xCE, value};
}

std::vector<uint8_t> CpuZ80::EncodeSBC_A_n(uint8_t value) const {
  return {0xDE, value};
}

std::vector<uint8_t> CpuZ80::EncodeCP_n(uint8_t value) const {
  return {0xFE, value};
}

// ============================================================================
// Logical Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeAND_n(uint8_t value) const {
  return {0xE6, value};
}

std::vector<uint8_t> CpuZ80::EncodeOR_n(uint8_t value) const {
  return {0xF6, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_n(uint8_t value) const {
  return {0xEE, value};
}

std::vector<uint8_t> CpuZ80::EncodeXOR_A() const { return {0xAF}; }

// ============================================================================
// Branch Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeJR_e(int8_t offset) const {
  return {0x18, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NZ_e(int8_t offset) const {
  return {0x20, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_Z_e(int8_t offset) const {
  return {0x28, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_NC_e(int8_t offset) const {
  return {0x30, static_cast<uint8_t>(offset)};
}

std::vector<uint8_t> CpuZ80::EncodeJR_C_e(int8_t offset) const {
  return {0x38, static_cast<uint8_t>(offset)};
}

// ============================================================================
// Memory Access Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_A_addr(uint16_t address) const {
  std::vector<uint8_t> result = {0x3A};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_addr_A(uint16_t address) const {
  std::vector<uint8_t> result = {0x32};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeLD_A_HL() const { return {0x7E}; }

std::vector<uint8_t> CpuZ80::EncodeLD_HL_A() const { return {0x77}; }

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeRLCA() const { return {0x07}; }

std::vector<uint8_t> CpuZ80::EncodeRRCA() const { return {0x0F}; }

std::vector<uint8_t> CpuZ80::EncodeRLA() const { return {0x17}; }

std::vector<uint8_t> CpuZ80::EncodeRRA() const { return {0x1F}; }

// ============================================================================
// Extended Instructions (ED Prefix)
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeLD_I_A() const { return {0xED, 0x47}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_I() const { return {0xED, 0x57}; }

std::vector<uint8_t> CpuZ80::EncodeLD_R_A() const { return {0xED, 0x4F}; }

std::vector<uint8_t> CpuZ80::EncodeLD_A_R() const { return {0xED, 0x5F}; }

std::vector<uint8_t> CpuZ80::EncodeLDIR() const { return {0xED, 0xB0}; }

std::vector<uint8_t> CpuZ80::EncodeCPIR() const { return {0xED, 0xB1}; }

std::vector<uint8_t> CpuZ80::EncodeNEG() const { return {0xED, 0x44}; }

// ============================================================================
// Additional Stack Operations
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodePUSH_DE() const { return {0xD5}; }

std::vector<uint8_t> CpuZ80::EncodePUSH_HL() const { return {0xE5}; }

std::vector<uint8_t> CpuZ80::EncodePUSH_AF() const { return {0xF5}; }

std::vector<uint8_t> CpuZ80::EncodePOP_DE() const { return {0xD1}; }

std::vector<uint8_t> CpuZ80::EncodePOP_HL() const { return {0xE1}; }

std::vector<uint8_t> CpuZ80::EncodePOP_AF() const { return {0xF1}; }

// ============================================================================
// Conditional Call and Return Instructions
// ============================================================================

std::vector<uint8_t> CpuZ80::EncodeCALL_nn(uint16_t address) const {
  std::vector<uint8_t> result = {0xCD};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeCALL_NZ_nn(uint16_t address) const {
  std::vector<uint8_t> result = {0xC4};
  auto bytes = ToLittleEndian(address);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

std::vector<uint8_t> CpuZ80::EncodeRET_Z() const { return {0xC8}; }

std::vector<uint8_t> CpuZ80::EncodeRET_NZ() const { return {0xC0}; }

} // namespace xasm
