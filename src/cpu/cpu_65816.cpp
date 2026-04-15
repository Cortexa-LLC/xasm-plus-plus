/**
 * @file cpu_65816.cpp
 * @brief 65816 CPU extensions for the 6502 implementation
 */

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// Group 11: 65816 Bank Operations
// ============================================================================

// PHB - Push Data Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHB() const {
  return {0x8B};
}

// PLB - Pull Data Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePLB() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0xAB};
}

// PHK - Push Program Bank Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHK() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x4B};
}

// PHD - Push Direct Page Register (65816)
std::vector<uint8_t> Cpu6502::EncodePHD() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x0B};
}

// PLD - Pull Direct Page Register (65816)
std::vector<uint8_t> Cpu6502::EncodePLD() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x2B};
}

// ============================================================================
// Group 12: 65816 Transfer Operations
// ============================================================================

// TCD - Transfer C to Direct Page (65816)
std::vector<uint8_t> Cpu6502::EncodeTCD() const {
  return {0x5B};
}

// TDC - Transfer Direct Page to C (65816)
std::vector<uint8_t> Cpu6502::EncodeTDC() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x7B};
}

// TCS - Transfer C to Stack Pointer (65816)
std::vector<uint8_t> Cpu6502::EncodeTCS() const {
  return {0x1B};
}

// TSC - Transfer Stack Pointer to C (65816)
std::vector<uint8_t> Cpu6502::EncodeTSC() const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x3B};
}

// ============================================================================
// Group 13: 65816 Long Jumps
// ============================================================================

// JML - Jump Long (65816)
std::vector<uint8_t> Cpu6502::EncodeJML(uint32_t operand,
                                        AddressingMode mode) const {
  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::AbsoluteLong) {
    bytes.push_back(0x5C);                                        // JML opcode
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // Middle byte
    bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF)); // Bank byte
  }

  return bytes;
}

// JSL - Jump Subroutine Long (65816)
std::vector<uint8_t> Cpu6502::EncodeJSL(uint32_t operand,
                                        AddressingMode mode) const {
  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::AbsoluteLong) {
    bytes.push_back(0x22);                                        // JSL opcode
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // Middle byte
    bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF)); // Bank byte
  }

  return bytes;
}

// RTL - Return from Subroutine Long (65816)
std::vector<uint8_t> Cpu6502::EncodeRTL() const {
  return {0x6B};
}

// ============================================================================
// Group 14: 65816 Miscellaneous Opcodes
// ============================================================================

// PEA - Push Effective Address (65816)
std::vector<uint8_t> Cpu6502::EncodePEA(uint16_t operand,
                                        AddressingMode mode) const {
  if (!IsCpu65816()) {
    return {};
  }

  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::Absolute) {
    bytes.push_back(0xF4);                                        // PEA opcode
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
  }

  return bytes;
}

// PEI - Push Effective Indirect Address (65816)
std::vector<uint8_t> Cpu6502::EncodePEI(uint8_t operand,
                                        AddressingMode mode) const {
  if (!IsCpu65816()) {
    return {};
  }

  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::ZeroPage) {
    bytes.push_back(0xD4);    // PEI opcode
    bytes.push_back(operand); // Zero page address
  }

  return bytes;
}

// PER - Push Effective PC Relative Address (65816)
std::vector<uint8_t> Cpu6502::EncodePER(uint16_t operand,
                                        AddressingMode mode) const {
  if (!IsCpu65816()) {
    return {};
  }

  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::Relative) {
    bytes.push_back(0x62);                                        // PER opcode
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
  }

  return bytes;
}

// MVN - Block Move Negative (65816)
std::vector<uint8_t> Cpu6502::EncodeMVN(uint8_t srcbank,
                                        uint8_t destbank) const {
  return {0x54, srcbank, destbank};
}

// MVP - Block Move Positive (65816)
std::vector<uint8_t> Cpu6502::EncodeMVP(uint8_t srcbank,
                                        uint8_t destbank) const {
  if (!IsCpu65816()) {
    return {};
  }
  return {0x44, srcbank, destbank};
}

// COP - Coprocessor (65816)
std::vector<uint8_t> Cpu6502::EncodeCOP(uint8_t operand,
                                        AddressingMode mode) const {
  if (!IsCpu65816()) {
    return {};
  }

  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::Immediate) {
    bytes.push_back(0x02);    // COP opcode
    bytes.push_back(operand); // Signature byte
  }

  return bytes;
}

// WDM - Reserved (65816)
std::vector<uint8_t> Cpu6502::EncodeWDM(uint8_t operand,
                                        AddressingMode mode) const {
  if (!IsCpu65816()) {
    return {};
  }

  std::vector<uint8_t> bytes;

  if (mode == AddressingMode::Immediate) {
    bytes.push_back(0x42);    // WDM opcode
    bytes.push_back(operand); // Reserved byte
  }

  return bytes;
}

// XBA - Exchange B and A (65816)
std::vector<uint8_t> Cpu6502::EncodeXBA() const {
  return {0xEB};
}

} // namespace xasm
