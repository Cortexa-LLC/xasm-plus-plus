// 6502 CPU implementation - Core 6502 opcodes

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/opcodes_6502.h"
#include <algorithm>
#include <unordered_set>

namespace xasm {

// Use mnemonic constants to eliminate magic strings
using namespace M6502Mnemonics;

// ============================================================================
// Generic Encoding Helper - Reduces Duplication Across Encode Methods
// ============================================================================

/**
 * @brief Generic encoding function using opcode table
 *
 * This function eliminates duplication across 50+ Encode methods by
 * implementing the common switch-statement logic once.
 *
 * @param table Opcode table mapping addressing modes to opcodes
 * @param operand The operand value (address or immediate value)
 * @param mode The addressing mode
 * @return Encoded bytes (opcode + operand bytes)
 */
std::vector<uint8_t> Cpu6502::EncodeWithTable(const OpcodeTable &table,
                                              uint32_t operand,
                                              AddressingMode mode) const {
  std::vector<uint8_t> bytes;
  std::optional<uint8_t> opcode;

  // Map addressing mode to opcode from table
  switch (mode) {
  case AddressingMode::Immediate:
    opcode = table.immediate;
    break;

  case AddressingMode::ZeroPage:
    opcode = table.zero_page;
    break;

  case AddressingMode::ZeroPageX:
    opcode = table.zero_page_x;
    break;

  case AddressingMode::ZeroPageY:
    opcode = table.zero_page_y;
    break;

  case AddressingMode::Absolute:
    opcode = table.absolute;
    break;

  case AddressingMode::AbsoluteX:
    opcode = table.absolute_x;
    break;

  case AddressingMode::AbsoluteY:
    opcode = table.absolute_y;
    break;

  case AddressingMode::Indirect:
    opcode = table.indirect;
    break;

  case AddressingMode::IndirectX:
    opcode = table.indirect_x;
    break;

  case AddressingMode::IndirectY:
    opcode = table.indirect_y;
    break;

  case AddressingMode::Accumulator:
    opcode = table.accumulator;
    break;

  case AddressingMode::Relative:
    opcode = table.relative;
    break;

  case AddressingMode::IndirectZeroPage:
    // Only available in 65C02 and later
    if (cpu_mode_ != CpuMode::Cpu6502) {
      opcode = table.indirect_zero_page;
    }
    break;

  case AddressingMode::AbsoluteIndexedIndirect:
    // Only available in 65C02 and later
    if (cpu_mode_ != CpuMode::Cpu6502) {
      opcode = table.absolute_indexed_indirect;
    }
    break;

  case AddressingMode::AbsoluteLong:
    // Only available in 65816
    if (cpu_mode_ == CpuMode::Cpu65816) {
      opcode = table.absolute_long;
    }
    break;

  case AddressingMode::IndirectLong:
    // Only available in 65816
    if (cpu_mode_ == CpuMode::Cpu65816) {
      opcode = table.indirect_long;
    }
    break;

  case AddressingMode::IndirectLongIndexedY:
    // Only available in 65816
    if (cpu_mode_ == CpuMode::Cpu65816) {
      opcode = table.indirect_long_indexed_y;
    }
    break;

  case AddressingMode::StackRelative:
    // Only available in 65816
    if (cpu_mode_ == CpuMode::Cpu65816) {
      opcode = table.stack_relative;
    }
    break;

  case AddressingMode::StackRelativeIndirectIndexedY:
    // Only available in 65816
    if (cpu_mode_ == CpuMode::Cpu65816) {
      opcode = table.stack_relative_indirect_indexed_y;
    }
    break;

  default:
    break;
  }

  // If opcode found, encode instruction
  if (opcode.has_value()) {
    bytes.push_back(opcode.value());

    // Add operand bytes based on addressing mode
    switch (mode) {
    case AddressingMode::Immediate:
    case AddressingMode::ZeroPage:
    case AddressingMode::ZeroPageX:
    case AddressingMode::ZeroPageY:
    case AddressingMode::IndirectX:
    case AddressingMode::IndirectY:
    case AddressingMode::Relative:
    case AddressingMode::IndirectZeroPage:
    case AddressingMode::IndirectLong:
    case AddressingMode::IndirectLongIndexedY:
    case AddressingMode::StackRelative:
    case AddressingMode::StackRelativeIndirectIndexedY:
      // 1-byte operand
      bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      break;

    case AddressingMode::Absolute:
    case AddressingMode::AbsoluteX:
    case AddressingMode::AbsoluteY:
    case AddressingMode::Indirect:
    case AddressingMode::AbsoluteIndexedIndirect:
      // 2-byte operand (little-endian)
      bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
      break;

    case AddressingMode::AbsoluteLong:
      // 3-byte operand (24-bit address, little-endian)
      bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
      bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF));
      break;

    case AddressingMode::Accumulator:
    case AddressingMode::Implied:
      // No operand bytes
      break;

    default:
      break;
    }
  }

  return bytes;
}

// ============================================================================
// Encode Methods - Now Using Opcode Tables (Reduced Duplication)
// ============================================================================

// LDA - Load Accumulator
// Note: Changed to uint32_t for 65816 24-bit addressing support
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable LDA_TABLE = {
      .immediate = Opcodes::LDA_IMM,
      .zero_page = Opcodes::LDA_ZP,
      .zero_page_x = Opcodes::LDA_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::LDA_ABS,
      .absolute_x = Opcodes::LDA_ABX,
      .absolute_y = Opcodes::LDA_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::LDA_INX,
      .indirect_y = Opcodes::LDA_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = Opcodes::LDA_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = Opcodes::LDA_ALG,                    // 65816
      .indirect_long = Opcodes::LDA_ILG,                    // 65816
      .indirect_long_indexed_y = Opcodes::LDA_ILY,          // 65816
      .stack_relative = Opcodes::LDA_SR,                    // 65816
      .stack_relative_indirect_indexed_y = Opcodes::LDA_SRY // 65816
  };

  return EncodeWithTable(LDA_TABLE, operand, mode);
}

// STA - Store Accumulator
std::vector<uint8_t> Cpu6502::EncodeSTA(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable STA_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::STA_ZP,
      .zero_page_x = Opcodes::STA_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::STA_ABS,
      .absolute_x = Opcodes::STA_ABX,
      .absolute_y = Opcodes::STA_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::STA_INX,
      .indirect_y = Opcodes::STA_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(STA_TABLE, operand, mode);
}

// JMP - Jump
std::vector<uint8_t> Cpu6502::EncodeJMP(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable JMP_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::JMP_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = Opcodes::JMP_IND,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = Opcodes::JMP_AIX, // 65C02+
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(JMP_TABLE, operand, mode);
}

// NOP - No Operation
std::vector<uint8_t> Cpu6502::EncodeNOP() const { return {Opcodes::NOP}; }

// RTS - Return from Subroutine
std::vector<uint8_t> Cpu6502::EncodeRTS() const { return {Opcodes::RTS}; }

// Phase 2.2: Arithmetic Instructions

// ADC - Add with Carry
std::vector<uint8_t> Cpu6502::EncodeADC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable ADC_TABLE = {
      .immediate = Opcodes::ADC_IMM,
      .zero_page = Opcodes::ADC_ZP,
      .zero_page_x = Opcodes::ADC_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::ADC_ABS,
      .absolute_x = Opcodes::ADC_ABX,
      .absolute_y = Opcodes::ADC_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::ADC_INX,
      .indirect_y = Opcodes::ADC_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(ADC_TABLE, operand, mode);
}

// SBC - Subtract with Carry
std::vector<uint8_t> Cpu6502::EncodeSBC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable SBC_TABLE = {
      .immediate = Opcodes::SBC_IMM,
      .zero_page = Opcodes::SBC_ZP,
      .zero_page_x = Opcodes::SBC_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::SBC_ABS,
      .absolute_x = Opcodes::SBC_ABX,
      .absolute_y = Opcodes::SBC_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::SBC_INX,
      .indirect_y = Opcodes::SBC_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(SBC_TABLE, operand, mode);
}

// Phase 2.2: Logic Instructions

// AND - Logical AND
std::vector<uint8_t> Cpu6502::EncodeAND(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable AND_TABLE = {
      .immediate = Opcodes::AND_IMM,
      .zero_page = Opcodes::AND_ZP,
      .zero_page_x = Opcodes::AND_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::AND_ABS,
      .absolute_x = Opcodes::AND_ABX,
      .absolute_y = Opcodes::AND_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::AND_INX,
      .indirect_y = Opcodes::AND_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(AND_TABLE, operand, mode);
}

// ORA - Logical OR
std::vector<uint8_t> Cpu6502::EncodeORA(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable ORA_TABLE = {
      .immediate = Opcodes::ORA_IMM,
      .zero_page = Opcodes::ORA_ZP,
      .zero_page_x = Opcodes::ORA_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::ORA_ABS,
      .absolute_x = Opcodes::ORA_ABX,
      .absolute_y = Opcodes::ORA_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::ORA_INX,
      .indirect_y = Opcodes::ORA_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(ORA_TABLE, operand, mode);
}

// EOR - Exclusive OR
std::vector<uint8_t> Cpu6502::EncodeEOR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable EOR_TABLE = {
      .immediate = Opcodes::EOR_IMM,
      .zero_page = Opcodes::EOR_ZP,
      .zero_page_x = Opcodes::EOR_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::EOR_ABS,
      .absolute_x = Opcodes::EOR_ABX,
      .absolute_y = Opcodes::EOR_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::EOR_INX,
      .indirect_y = Opcodes::EOR_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(EOR_TABLE, operand, mode);
}

// Phase 2.2: Additional Loads/Stores

// LDX - Load X Register
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable LDX_TABLE = {
      .immediate = Opcodes::LDX_IMM,
      .zero_page = Opcodes::LDX_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = Opcodes::LDX_ZPY,
      .absolute = Opcodes::LDX_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = Opcodes::LDX_ABY,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(LDX_TABLE, operand, mode);
}

// LDY - Load Y Register
std::vector<uint8_t> Cpu6502::EncodeLDY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable LDY_TABLE = {
      .immediate = Opcodes::LDY_IMM,
      .zero_page = Opcodes::LDY_ZP,
      .zero_page_x = Opcodes::LDY_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::LDY_ABS,
      .absolute_x = Opcodes::LDY_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(LDY_TABLE, operand, mode);
}

// STX - Store X Register
std::vector<uint8_t> Cpu6502::EncodeSTX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable STX_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::STX_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = Opcodes::STX_ZPY,
      .absolute = Opcodes::STX_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(STX_TABLE, operand, mode);
}

// STY - Store Y Register
std::vector<uint8_t> Cpu6502::EncodeSTY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable STY_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::STY_ZP,
      .zero_page_x = Opcodes::STY_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::STY_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(STY_TABLE, operand, mode);
}

// Phase 2.2: Comparisons

// CMP - Compare Accumulator
std::vector<uint8_t> Cpu6502::EncodeCMP(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable CMP_TABLE = {
      .immediate = Opcodes::CMP_IMM,
      .zero_page = Opcodes::CMP_ZP,
      .zero_page_x = Opcodes::CMP_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::CMP_ABS,
      .absolute_x = Opcodes::CMP_ABX,
      .absolute_y = Opcodes::CMP_ABY,
      .indirect = std::nullopt,
      .indirect_x = Opcodes::CMP_INX,
      .indirect_y = Opcodes::CMP_INY,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(CMP_TABLE, operand, mode);
}

// CPX - Compare X Register
std::vector<uint8_t> Cpu6502::EncodeCPX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable CPX_TABLE = {
      .immediate = Opcodes::CPX_IMM,
      .zero_page = Opcodes::CPX_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::CPX_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(CPX_TABLE, operand, mode);
}

// CPY - Compare Y Register
std::vector<uint8_t> Cpu6502::EncodeCPY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable CPY_TABLE = {
      .immediate = Opcodes::CPY_IMM,
      .zero_page = Opcodes::CPY_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::CPY_ABS,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(CPY_TABLE, operand, mode);
}

// Phase 2.2: Branch Instructions

// BEQ - Branch if Equal
std::vector<uint8_t> Cpu6502::EncodeBEQ(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BEQ_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BEQ,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BEQ_TABLE, operand, mode);
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BNE_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BNE,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BNE_TABLE, operand, mode);
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BCC_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BCC,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BCC_TABLE, operand, mode);
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BCS_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BCS,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BCS_TABLE, operand, mode);
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BMI_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BMI,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BMI_TABLE, operand, mode);
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BPL_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BPL,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BPL_TABLE, operand, mode);
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BVC_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BVC,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BVC_TABLE, operand, mode);
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable BVS_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = Opcodes::BVS,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BVS_TABLE, operand, mode);
}

// Phase 2.2: Inc/Dec Instructions

// INX - Increment X Register
std::vector<uint8_t> Cpu6502::EncodeINX() const { return {Opcodes::INX}; }

// INY - Increment Y Register
std::vector<uint8_t> Cpu6502::EncodeINY() const { return {Opcodes::INY}; }

// DEX - Decrement X Register
std::vector<uint8_t> Cpu6502::EncodeDEX() const { return {Opcodes::DEX}; }

// DEY - Decrement Y Register
std::vector<uint8_t> Cpu6502::EncodeDEY() const { return {Opcodes::DEY}; }

// INC - Increment Memory
std::vector<uint8_t> Cpu6502::EncodeINC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable INC_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::INC_ZP,
      .zero_page_x = Opcodes::INC_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::INC_ABS,
      .absolute_x = Opcodes::INC_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(INC_TABLE, operand, mode);
}

// DEC - Decrement Memory
std::vector<uint8_t> Cpu6502::EncodeDEC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable DEC_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::DEC_ZP,
      .zero_page_x = Opcodes::DEC_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::DEC_ABS,
      .absolute_x = Opcodes::DEC_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(DEC_TABLE, operand, mode);
}

// Phase 2.2: Stack Operations

// PHA - Push Accumulator
std::vector<uint8_t> Cpu6502::EncodePHA() const { return {Opcodes::PHA}; }

// PLA - Pull Accumulator
std::vector<uint8_t> Cpu6502::EncodePLA() const { return {Opcodes::PLA}; }

// PHP - Push Processor Status
std::vector<uint8_t> Cpu6502::EncodePHP() const { return {Opcodes::PHP}; }

// PLP - Pull Processor Status
std::vector<uint8_t> Cpu6502::EncodePLP() const { return {Opcodes::PLP}; }

// Phase 2.2: Subroutine

// JSR - Jump to Subroutine
std::vector<uint8_t> Cpu6502::EncodeJSR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable JSR_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::JSR,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(JSR_TABLE, operand, mode);
}

// ============================================================================
// Phase 2.3: Complete 6502 Instruction Set
// ============================================================================

// Group 1: BIT - Test Bits
// Note: Special case - some standard addressing modes only available in 65C02+
std::vector<uint8_t> Cpu6502::EncodeBIT(uint16_t operand,
                                        AddressingMode mode) const {
  std::vector<uint8_t> bytes;

  switch (mode) {
  case AddressingMode::ZeroPage:
    bytes.push_back(Opcodes::BIT_ZP);
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    break;

  case AddressingMode::Absolute:
    bytes.push_back(Opcodes::BIT_ABS);
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    break;

  case AddressingMode::Immediate:
  case AddressingMode::ZeroPageX:
  case AddressingMode::AbsoluteX:
    // These modes only available in 65C02+
    if (cpu_mode_ != CpuMode::Cpu6502) {
      if (mode == AddressingMode::Immediate) {
        bytes.push_back(Opcodes::BIT_IMM);
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      } else if (mode == AddressingMode::ZeroPageX) {
        bytes.push_back(Opcodes::BIT_ZPX);
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      } else if (mode == AddressingMode::AbsoluteX) {
        bytes.push_back(Opcodes::BIT_ABX);
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
      }
    }
    break;

  default:
    break;
  }

  return bytes;
}

// Group 2: Shift Instructions

// ASL - Arithmetic Shift Left
std::vector<uint8_t> Cpu6502::EncodeASL(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable ASL_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::ASL_ZP,
      .zero_page_x = Opcodes::ASL_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::ASL_ABS,
      .absolute_x = Opcodes::ASL_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = Opcodes::ASL_ACC,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(ASL_TABLE, operand, mode);
}

// LSR - Logical Shift Right
std::vector<uint8_t> Cpu6502::EncodeLSR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable LSR_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::LSR_ZP,
      .zero_page_x = Opcodes::LSR_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::LSR_ABS,
      .absolute_x = Opcodes::LSR_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = Opcodes::LSR_ACC,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(LSR_TABLE, operand, mode);
}

// Group 3: Rotate Instructions

// ROL - Rotate Left
std::vector<uint8_t> Cpu6502::EncodeROL(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable ROL_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::ROL_ZP,
      .zero_page_x = Opcodes::ROL_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::ROL_ABS,
      .absolute_x = Opcodes::ROL_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = Opcodes::ROL_ACC,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(ROL_TABLE, operand, mode);
}

// ROR - Rotate Right
std::vector<uint8_t> Cpu6502::EncodeROR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable ROR_TABLE = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::ROR_ZP,
      .zero_page_x = Opcodes::ROR_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::ROR_ABS,
      .absolute_x = Opcodes::ROR_ABX,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = Opcodes::ROR_ACC,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(ROR_TABLE, operand, mode);
}

// Group 4: Interrupt Instructions

// RTI - Return from Interrupt
std::vector<uint8_t> Cpu6502::EncodeRTI() const { return {Opcodes::RTI}; }

// BRK - Break
std::vector<uint8_t> Cpu6502::EncodeBRK() const { return {Opcodes::BRK}; }

// Group 5: Flag Operations

// CLC - Clear Carry
std::vector<uint8_t> Cpu6502::EncodeCLC() const { return {Opcodes::CLC}; }

// SEC - Set Carry
std::vector<uint8_t> Cpu6502::EncodeSEC() const { return {Opcodes::SEC}; }

// CLD - Clear Decimal
std::vector<uint8_t> Cpu6502::EncodeCLD() const { return {Opcodes::CLD}; }

// SED - Set Decimal
std::vector<uint8_t> Cpu6502::EncodeSED() const { return {Opcodes::SED}; }

// CLI - Clear Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeCLI() const { return {Opcodes::CLI}; }

// SEI - Set Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeSEI() const { return {Opcodes::SEI}; }

// CLV - Clear Overflow
std::vector<uint8_t> Cpu6502::EncodeCLV() const { return {Opcodes::CLV}; }

// Group 6: Transfer Instructions

// TSX - Transfer SP to X
std::vector<uint8_t> Cpu6502::EncodeTSX() const { return {Opcodes::TSX}; }

// TXS - Transfer X to SP
std::vector<uint8_t> Cpu6502::EncodeTXS() const { return {Opcodes::TXS}; }

// TAX - Transfer A to X
std::vector<uint8_t> Cpu6502::EncodeTAX() const { return {Opcodes::TAX}; }

// TAY - Transfer A to Y
std::vector<uint8_t> Cpu6502::EncodeTAY() const { return {Opcodes::TAY}; }

// TXA - Transfer X to A
std::vector<uint8_t> Cpu6502::EncodeTXA() const { return {Opcodes::TXA}; }

// TYA - Transfer Y to A
std::vector<uint8_t> Cpu6502::EncodeTYA() const { return {Opcodes::TYA}; }

// Calculate instruction size based on addressing mode
size_t Cpu6502::CalculateInstructionSize(AddressingMode mode) const {
  switch (mode) {
  case AddressingMode::Implied:
    return 1;

  case AddressingMode::Immediate:
  case AddressingMode::ZeroPage:
  case AddressingMode::ZeroPageX:
  case AddressingMode::ZeroPageY:
  case AddressingMode::IndirectX:
  case AddressingMode::IndirectY:
  case AddressingMode::Relative:
  case AddressingMode::IndirectZeroPage:     // Phase 2.5 - Group 6: 65C02
  case AddressingMode::IndirectLong:         // Phase 2.5 - Group 9: 65816
  case AddressingMode::IndirectLongIndexedY: // Phase 2.5 - Group 9: 65816
  case AddressingMode::StackRelative:        // Phase 2.5 - Group 10: 65816
  case AddressingMode::StackRelativeIndirectIndexedY: // Phase 2.5 - Group 10:
                                                      // 65816
    return 2;

  case AddressingMode::Absolute:
  case AddressingMode::AbsoluteX:
  case AddressingMode::AbsoluteY:
  case AddressingMode::Indirect:
  case AddressingMode::AbsoluteIndexedIndirect: // Phase 2.5 - Group 6: 65C02
    return 3;

  case AddressingMode::AbsoluteLong: // Phase 2.5 - Group 9: 65816 (24-bit
                                     // address)
    return 4;

  default:
    return 0;
  }
}

// ============================================================================
// Branch Relaxation Support (Long Branch Handling)
// ============================================================================

/**
 * @brief Check if branch needs relaxation (target is out of 8-bit signed range)
 *
 * 6502 branch instructions use 8-bit signed relative offsets (-128 to +127
 * bytes). If target is outside this range, branch must be "relaxed" into B!cc +
 * JMP sequence.
 *
 * @param current_addr Address of the branch instruction
 * @param target_addr Target address to branch to
 * @return true if branch needs relaxation (out of range), false otherwise
 */
bool Cpu6502::NeedsBranchRelaxation(uint16_t current_addr,
                                    uint16_t target_addr) const {
  return branch_handler_.NeedsBranchRelaxation(current_addr, target_addr);
}

uint8_t Cpu6502::GetComplementaryBranchOpcode(uint8_t branch_opcode) const {
  return branch_handler_.GetComplementaryBranchOpcode(branch_opcode);
}

std::vector<uint8_t> Cpu6502::EncodeBranchWithRelaxation(
    uint8_t branch_opcode, uint16_t current_addr, uint16_t target_addr) const {
  return branch_handler_.EncodeBranchWithRelaxation(branch_opcode, current_addr,
                                                    target_addr);
}

// ============================================================================
// Phase 2.6: 65C02 Rockwell Extensions
// ============================================================================

/**
 * @brief Encode WAI (Wait for Interrupt) - Rockwell 65C02 extension
 *
 * WAI halts the processor until an interrupt (IRQ or NMI) occurs.
 * Power-saving instruction for Apple IIc, IIgs, and WDC 65C02S.
 *
 * Opcode: CB (Implied addressing)
 * Only available in Cpu65C02Rock mode.
 */
std::vector<uint8_t> Cpu6502::EncodeWAI() const {
  // Rockwell/WDC 65C02 extension only
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {}; // Not available in this CPU mode
  }

  return {0xCB}; // WAI opcode
}

/**
 * @brief Encode STP (Stop Processor) - Rockwell 65C02 extension
 *
 * STP completely stops the processor until hardware reset.
 * Halt instruction for power-down or error conditions.
 *
 * Opcode: DB (Implied addressing)
 * Only available in Cpu65C02Rock mode.
 */
std::vector<uint8_t> Cpu6502::EncodeSTP() const {
  // Rockwell/WDC 65C02 extension only
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {}; // Not available in this CPU mode
  }

  return {0xDB}; // STP opcode
}

// ============================================================================
// RMB0-RMB7: Reset Memory Bit (Rockwell 65C02 Extensions)
//
// These instructions reset (clear to 0) a specific bit in a zero page location.
// Each instruction operates on one of the 8 bits (0-7).
//
// Addressing: Zero Page only
// Format: RMBn $zp (2 bytes: opcode + zero page address)
// Opcodes: 07, 17, 27, 37, 47, 57, 67, 77
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeRMB0(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB0, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB1(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB1, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB2(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB2, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB3(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB3, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB4(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB4, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB5(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB5, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB6(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB6, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB7(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::RMB7, operand};
}

// ============================================================================
// SMB0-SMB7: Set Memory Bit (Rockwell 65C02 Extensions)
//
// These instructions set (to 1) a specific bit in a zero page location.
// Each instruction operates on one of the 8 bits (0-7).
//
// Addressing: Zero Page only
// Format: SMBn $zp (2 bytes: opcode + zero page address)
// Opcodes: 87, 97, A7, B7, C7, D7, E7, F7
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeSMB0(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB0, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB1(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB1, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB2(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB2, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB3(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB3, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB4(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB4, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB5(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB5, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB6(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB6, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB7(uint8_t operand,
                                         AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
    return {};
  }
  return {RockwellOpcodes::SMB7, operand};
}

// ============================================================================
// BBR0-BBR7: Branch if Bit Reset (Rockwell 65C02 Extensions)
//
// These instructions test a specific bit in a zero page location and branch
// if that bit is reset (0). Each instruction tests one of the 8 bits (0-7).
//
// Addressing: Zero Page + Relative (unique to these instructions)
// Format: BBRn $zp, label (3 bytes: opcode + zp address + relative offset)
// Opcodes: 0F, 1F, 2F, 3F, 4F, 5F, 6F, 7F
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeBBR0(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR0, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR1(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR1, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR2(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR2, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR3(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR3, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR4(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR4, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR5(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR5, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR6(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR6, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR7(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR7, zp_addr, offset};
}

// ============================================================================
// BBS0-BBS7: Branch if Bit Set (Rockwell 65C02 Extensions)
//
// These instructions test a specific bit in a zero page location and branch
// if that bit is set (1). Each instruction tests one of the 8 bits (0-7).
//
// Addressing: Zero Page + Relative (unique to these instructions)
// Format: BBSn $zp, label (3 bytes: opcode + zp address + relative offset)
// Opcodes: 8F, 9F, AF, BF, CF, DF, EF, FF
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeBBS0(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS0, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS1(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS1, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS2(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS2, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS3(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS3, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS4(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS4, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS5(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS5, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS6(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS6, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS7(uint8_t zp_addr,
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS7, zp_addr, offset};
}

// ============================================================================
// Phase 2.5 - Group 2: 65C02 Stack Operations
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodePHX() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {}; // Not available in base 6502
  }
  return {0xDA}; // PHX opcode
}

std::vector<uint8_t> Cpu6502::EncodePLX() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {0xFA}; // PLX opcode
}

std::vector<uint8_t> Cpu6502::EncodePHY() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {0x5A}; // PHY opcode
}

std::vector<uint8_t> Cpu6502::EncodePLY() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {0x7A}; // PLY opcode
}

// ============================================================================
// Phase 2.5 - Group 3: 65C02 Store Zero
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeSTZ(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {}; // Not available in base 6502
  }

  static const OpcodeTable STZ_TABLE = {
      .immediate = std::nullopt,
      .zero_page = 0x64,
      .zero_page_x = 0x74,
      .zero_page_y = std::nullopt,
      .absolute = 0x9C,
      .absolute_x = 0x9E,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(STZ_TABLE, operand, mode);
}

// ============================================================================
// Phase 2.5 - Group 4: 65C02 Bit Test
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeTRB(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable TRB_TABLE = {
      .immediate = std::nullopt,
      .zero_page = 0x14,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = 0x1C,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(TRB_TABLE, operand, mode);
}

std::vector<uint8_t> Cpu6502::EncodeTSB(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable TSB_TABLE = {
      .immediate = std::nullopt,
      .zero_page = 0x04,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = 0x0C,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(TSB_TABLE, operand, mode);
}

// ============================================================================
// Phase 2.5 - Group 5: 65C02 Branch Always
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeBRA(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable BRA_TABLE = {
      .immediate = std::nullopt,
      .zero_page = std::nullopt,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = std::nullopt,
      .absolute_x = std::nullopt,
      .absolute_y = std::nullopt,
      .indirect = std::nullopt,
      .indirect_x = std::nullopt,
      .indirect_y = std::nullopt,
      .accumulator = std::nullopt,
      .relative = 0x80,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(BRA_TABLE, operand, mode);
}

// ============================================================================
// Phase 2.5 - Groups 11-14: 65816 Instructions
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodePHB() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x8B};
}

std::vector<uint8_t> Cpu6502::EncodePLB() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0xAB};
}

std::vector<uint8_t> Cpu6502::EncodePHK() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x4B};
}

std::vector<uint8_t> Cpu6502::EncodePHD() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x0B};
}

std::vector<uint8_t> Cpu6502::EncodePLD() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x2B};
}

std::vector<uint8_t> Cpu6502::EncodeTCD() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x5B};
}

std::vector<uint8_t> Cpu6502::EncodeTDC() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x7B};
}

std::vector<uint8_t> Cpu6502::EncodeTCS() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x1B};
}

std::vector<uint8_t> Cpu6502::EncodeTSC() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x3B};
}

std::vector<uint8_t> Cpu6502::EncodeJML(uint32_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::AbsoluteLong) {
    std::vector<uint8_t> bytes = {0x5C};
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF));
    return bytes;
  } else if (mode == AddressingMode::Indirect) {
    std::vector<uint8_t> bytes = {0xDC};
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    return bytes;
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeJSL(uint32_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::AbsoluteLong) {
    std::vector<uint8_t> bytes = {0x22};
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF));
    return bytes;
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeRTL() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x6B};
}

std::vector<uint8_t> Cpu6502::EncodePEA(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Absolute) {
    std::vector<uint8_t> bytes = {0xF4};
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    return bytes;
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodePEI(uint8_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::ZeroPage) {
    return {0xD4, operand};
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodePER(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Relative) {
    std::vector<uint8_t> bytes = {0x62};
    bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    return bytes;
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeMVN(uint8_t srcbank,
                                        uint8_t destbank) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x54, srcbank, destbank};
}

std::vector<uint8_t> Cpu6502::EncodeMVP(uint8_t srcbank,
                                        uint8_t destbank) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0x44, srcbank, destbank};
}

std::vector<uint8_t> Cpu6502::EncodeCOP(uint8_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Immediate) {
    return {0x02, operand};
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeWDM(uint8_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Immediate) {
    return {0x42, operand};
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeXBA() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0xEB};
}

std::vector<uint8_t> Cpu6502::EncodeXCE() const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};
  return {0xFB};
}

std::vector<uint8_t> Cpu6502::EncodeSEP(uint16_t value,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Immediate) {
    return {0xE2, static_cast<uint8_t>(value & 0xFF)};
  }
  return {};
}

std::vector<uint8_t> Cpu6502::EncodeREP(uint16_t value,
                                        AddressingMode mode) const {
  if (cpu_mode_ != CpuMode::Cpu65816)
    return {};

  if (mode == AddressingMode::Immediate) {
    return {0xC2, static_cast<uint8_t>(value & 0xFF)};
  }
  return {};
}

// ============================================================================
// CPU Mode Configuration
// ============================================================================

void Cpu6502::SetCpuMode(CpuMode mode) { cpu_mode_ = mode; }

CpuMode Cpu6502::GetCpuMode() const { return cpu_mode_; }

void Cpu6502::SetMX(bool m_flag, bool x_flag) {
  m_flag_ = m_flag;
  x_flag_ = x_flag;
}

bool Cpu6502::IsAccumulator8Bit() const { return m_flag_; }

bool Cpu6502::IsIndex8Bit() const { return x_flag_; }

// ============================================================================
// CpuPlugin Interface Implementation - EncodeInstruction()
// ============================================================================

/**
 * @brief Polymorphic instruction encoder - dispatches to specific Encode*
 * methods
 *
 * This method implements the CpuPlugin interface, allowing the assembler to
 * encode instructions without knowing the specific CPU type.
 *
 * @param mnemonic Instruction mnemonic (e.g., "LDA", "STA", "JMP")
 * @param operand Operand value (immediate value or address)
 * @param operand_str Original operand string for parsing addressing modes
 * @return Vector of encoded bytes
 *
 * @throws std::invalid_argument if instruction/addressing mode not supported
 * @throws std::out_of_range if operand value out of range
 */
std::vector<uint8_t>
Cpu6502::EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                           const std::string &operand_str) const {
  // Helper to trim whitespace
  auto trim = [](const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
      return std::string("");
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
  };

  // Helper to parse hex value
  auto parse_hex = [](const std::string &s) -> uint32_t {
    if (s.empty() || s[0] != '$')
      return 0;
    return std::stoul(s.substr(1), nullptr, 16);
  };

  // Determine addressing mode from operand_str
  std::string trimmed = trim(operand_str);
  AddressingMode mode = AddressingMode::Implied;

  if (!trimmed.empty()) {
    // Accumulator mode
    if (trimmed == "A") {
      mode = AddressingMode::Accumulator;
    }
    // Immediate mode
    else if (trimmed[0] == '#') {
      mode = AddressingMode::Immediate;
    }
    // Indirect modes
    else if (trimmed[0] == '(') {
      size_t close_paren = trimmed.find(')');
      if (close_paren != std::string::npos) {
        std::string inside = trimmed.substr(1, close_paren - 1);
        inside = trim(inside);

        // Check for indexed indirect: ($80,X)
        if (inside.find(",X") != std::string::npos ||
            inside.find(", X") != std::string::npos) {
          mode = AddressingMode::IndirectX;
        }
        // Check for indirect indexed: ($80),Y
        else if (close_paren < trimmed.length() - 1) {
          std::string after = trim(trimmed.substr(close_paren + 1));
          if (after == ",Y" || after == ", Y") {
            mode = AddressingMode::IndirectY;
          }
        }
        // Simple indirect: ($1234) - only for JMP
        else {
          mode = AddressingMode::Indirect;
        }
      }
    }
    // Indexed modes: ,X or ,Y
    else if (trimmed.find(",X") != std::string::npos ||
             trimmed.find(", X") != std::string::npos) {
      size_t comma_pos = trimmed.find(",X");
      if (comma_pos == std::string::npos)
        comma_pos = trimmed.find(", X");
      std::string addr_part = trim(trimmed.substr(0, comma_pos));

      if (!addr_part.empty() && addr_part[0] == '$') {
        uint32_t val = parse_hex(addr_part);
        mode = (val <= 0xFF) ? AddressingMode::ZeroPageX
                             : AddressingMode::AbsoluteX;
      } else {
        // BUG-001 FIX: Symbol - use resolved operand value to determine mode
        mode = (operand <= 0xFF) ? AddressingMode::ZeroPageX
                                 : AddressingMode::AbsoluteX;
      }
    } else if (trimmed.find(",Y") != std::string::npos ||
               trimmed.find(", Y") != std::string::npos) {
      size_t comma_pos = trimmed.find(",Y");
      if (comma_pos == std::string::npos)
        comma_pos = trimmed.find(", Y");
      std::string addr_part = trim(trimmed.substr(0, comma_pos));

      if (!addr_part.empty() && addr_part[0] == '$') {
        uint32_t val = parse_hex(addr_part);
        mode = (val <= 0xFF) ? AddressingMode::ZeroPageY
                             : AddressingMode::AbsoluteY;
      } else {
        // BUG-001 FIX: Symbol - use resolved operand value to determine mode
        mode = (operand <= 0xFF) ? AddressingMode::ZeroPageY
                                 : AddressingMode::AbsoluteY;
      }
    }
    // Absolute or ZeroPage
    else {
      if (trimmed[0] == '$') {
        uint32_t val = parse_hex(trimmed);
        mode =
            (val <= 0xFF) ? AddressingMode::ZeroPage : AddressingMode::Absolute;
      } else {
        // BUG-001 FIX: Symbol - use resolved operand value to determine mode
        mode = (operand <= 0xFF) ? AddressingMode::ZeroPage
                                 : AddressingMode::Absolute;
      }
    }
  }

  // Dispatch to appropriate Encode* method based on mnemonic
  // Load instructions
  if (mnemonic == LDA)
    return EncodeLDA(operand, mode);
  if (mnemonic == LDX)
    return EncodeLDX(operand, mode);
  if (mnemonic == LDY)
    return EncodeLDY(operand, mode);

  // Store instructions
  if (mnemonic == STA)
    return EncodeSTA(operand, mode);
  if (mnemonic == STX)
    return EncodeSTX(operand, mode);
  if (mnemonic == STY)
    return EncodeSTY(operand, mode);
  if (mnemonic == STZ)
    return EncodeSTZ(operand, mode);

  // Arithmetic
  if (mnemonic == ADC)
    return EncodeADC(operand, mode);
  if (mnemonic == SBC)
    return EncodeSBC(operand, mode);
  if (mnemonic == INC)
    return EncodeINC(operand, mode);
  if (mnemonic == DEC)
    return EncodeDEC(operand, mode);
  if (mnemonic == INX)
    return EncodeINX();
  if (mnemonic == INY)
    return EncodeINY();
  if (mnemonic == DEX)
    return EncodeDEX();
  if (mnemonic == DEY)
    return EncodeDEY();

  // Logical
  if (mnemonic == AND)
    return EncodeAND(operand, mode);
  if (mnemonic == ORA)
    return EncodeORA(operand, mode);
  if (mnemonic == EOR)
    return EncodeEOR(operand, mode);
  if (mnemonic == BIT)
    return EncodeBIT(operand, mode);

  // Compare
  if (mnemonic == CMP)
    return EncodeCMP(operand, mode);
  if (mnemonic == CPX)
    return EncodeCPX(operand, mode);
  if (mnemonic == CPY)
    return EncodeCPY(operand, mode);

  // Branches
  if (mnemonic == BEQ)
    return EncodeBEQ(operand, mode);
  if (mnemonic == BNE)
    return EncodeBNE(operand, mode);
  if (mnemonic == BCC)
    return EncodeBCC(operand, mode);
  if (mnemonic == BCS)
    return EncodeBCS(operand, mode);
  if (mnemonic == BMI)
    return EncodeBMI(operand, mode);
  if (mnemonic == BPL)
    return EncodeBPL(operand, mode);
  if (mnemonic == BVC)
    return EncodeBVC(operand, mode);
  if (mnemonic == BVS)
    return EncodeBVS(operand, mode);
  if (mnemonic == BRA)
    return EncodeBRA(operand, mode);

  // Jumps/Subroutines
  if (mnemonic == JMP)
    return EncodeJMP(operand, mode);
  if (mnemonic == JSR)
    return EncodeJSR(operand, mode);
  if (mnemonic == RTS)
    return EncodeRTS();
  if (mnemonic == RTI)
    return EncodeRTI();

  // Stack
  if (mnemonic == PHA)
    return EncodePHA();
  if (mnemonic == PLA)
    return EncodePLA();
  if (mnemonic == PHP)
    return EncodePHP();
  if (mnemonic == PLP)
    return EncodePLP();
  if (mnemonic == PHX)
    return EncodePHX();
  if (mnemonic == PLX)
    return EncodePLX();
  if (mnemonic == PHY)
    return EncodePHY();
  if (mnemonic == PLY)
    return EncodePLY();

  // Shifts/Rotates
  if (mnemonic == ASL)
    return EncodeASL(operand, mode);
  if (mnemonic == LSR)
    return EncodeLSR(operand, mode);
  if (mnemonic == ROL)
    return EncodeROL(operand, mode);
  if (mnemonic == ROR)
    return EncodeROR(operand, mode);

  // Flags
  if (mnemonic == CLC)
    return EncodeCLC();
  if (mnemonic == SEC)
    return EncodeSEC();
  if (mnemonic == CLD)
    return EncodeCLD();
  if (mnemonic == SED)
    return EncodeSED();
  if (mnemonic == CLI)
    return EncodeCLI();
  if (mnemonic == SEI)
    return EncodeSEI();
  if (mnemonic == CLV)
    return EncodeCLV();

  // Transfers
  if (mnemonic == TAX)
    return EncodeTAX();
  if (mnemonic == TAY)
    return EncodeTAY();
  if (mnemonic == TXA)
    return EncodeTXA();
  if (mnemonic == TYA)
    return EncodeTYA();
  if (mnemonic == TSX)
    return EncodeTSX();
  if (mnemonic == TXS)
    return EncodeTXS();

  // Misc
  if (mnemonic == NOP)
    return EncodeNOP();
  if (mnemonic == BRK)
    return EncodeBRK();

  // 65C02 Bit test
  if (mnemonic == TRB)
    return EncodeTRB(operand, mode);
  if (mnemonic == TSB)
    return EncodeTSB(operand, mode);

  // 65816 instructions
  if (mnemonic == PHB)
    return EncodePHB();
  if (mnemonic == PLB)
    return EncodePLB();
  if (mnemonic == PHK)
    return EncodePHK();
  if (mnemonic == PHD)
    return EncodePHD();
  if (mnemonic == PLD)
    return EncodePLD();
  if (mnemonic == TCD)
    return EncodeTCD();
  if (mnemonic == TDC)
    return EncodeTDC();
  if (mnemonic == TCS)
    return EncodeTCS();
  if (mnemonic == TSC)
    return EncodeTSC();
  if (mnemonic == JML)
    return EncodeJML(operand, mode);
  if (mnemonic == JSL)
    return EncodeJSL(operand, mode);
  if (mnemonic == RTL)
    return EncodeRTL();
  if (mnemonic == PEA)
    return EncodePEA(operand, mode);
  if (mnemonic == PEI)
    return EncodePEI(operand, mode);
  if (mnemonic == PER)
    return EncodePER(operand, mode);
  if (mnemonic == XBA)
    return EncodeXBA();
  if (mnemonic == XCE)
    return EncodeXCE();
  if (mnemonic == SEP)
    return EncodeSEP(operand, mode);
  if (mnemonic == REP)
    return EncodeREP(operand, mode);
  if (mnemonic == COP)
    return EncodeCOP(operand, mode);
  if (mnemonic == WDM)
    return EncodeWDM(operand, mode);

  // Rockwell 65C02 extensions
  if (mnemonic == WAI)
    return EncodeWAI();
  if (mnemonic == STP)
    return EncodeSTP();

  // Unsupported instruction
  throw std::invalid_argument("Unsupported instruction: " + mnemonic);
}

// ============================================================================
// CpuPlugin Interface Implementation - Special Encoding Support
// ============================================================================

/**
 * @brief Check if an instruction requires special encoding
 *
 * Branch instructions with relaxation and MVN/MVP multi-byte instructions
 * need special handling that requires context beyond standard operand values.
 *
 * @param mnemonic Instruction mnemonic to check
 * @return true if the instruction requires special encoding
 */
bool Cpu6502::RequiresSpecialEncoding(const std::string &mnemonic) const {
  // Branch instructions require special encoding (branch relaxation)
  // Use mnemonic constants to avoid magic strings
  if (mnemonic == M6502Mnemonics::BEQ || mnemonic == M6502Mnemonics::BNE ||
      mnemonic == M6502Mnemonics::BCC || mnemonic == M6502Mnemonics::BCS ||
      mnemonic == M6502Mnemonics::BMI || mnemonic == M6502Mnemonics::BPL ||
      mnemonic == M6502Mnemonics::BVC || mnemonic == M6502Mnemonics::BVS ||
      mnemonic == M6502Mnemonics::BLT || // BLT is an alias for BCC
      mnemonic == M6502Mnemonics::BRA) { // BRA (65C02+)
    return true;
  }

  // MVN/MVP (65816 block move instructions) require special encoding
  if (mnemonic == M6502Mnemonics::MVN || mnemonic == M6502Mnemonics::MVP) {
    return true;
  }

  return false;
}

/**
 * @brief Encode an instruction with special handling
 *
 * Handles:
 * - Branch instructions with relaxation (needs current address and target)
 * - MVN/MVP instructions (needs two bank operands parsed from string)
 *
 * @param mnemonic Instruction mnemonic
 * @param operand Operand string (unparsed, for special parsing)
 * @param current_address Current instruction address (for relative branches)
 * @return Vector of encoded bytes
 *
 * @throws std::invalid_argument if instruction not supported for special
 * encoding
 * @throws std::runtime_error if encoding fails
 */
std::vector<uint8_t>
Cpu6502::EncodeInstructionSpecial(const std::string &mnemonic,
                                  const std::string &operand,
                                  uint16_t current_address) const {
  // Helper to trim whitespace
  auto trim = [](const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
      return std::string("");
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
  };

  // Helper to parse hex value
  auto parse_hex = [](const std::string &s) -> uint32_t {
    if (s.empty() || s[0] != '$')
      throw std::runtime_error("Expected hex value starting with $");
    return std::stoul(s.substr(1), nullptr, 16);
  };

  // Branch instructions with relaxation
  if (mnemonic == M6502Mnemonics::BEQ || mnemonic == M6502Mnemonics::BNE ||
      mnemonic == M6502Mnemonics::BCC || mnemonic == M6502Mnemonics::BCS ||
      mnemonic == M6502Mnemonics::BMI || mnemonic == M6502Mnemonics::BPL ||
      mnemonic == M6502Mnemonics::BVC || mnemonic == M6502Mnemonics::BVS ||
      mnemonic == M6502Mnemonics::BLT || mnemonic == M6502Mnemonics::BRA) {

    // Parse target address from operand string
    std::string trimmed = trim(operand);
    uint16_t target_addr = 0;

    if (!trimmed.empty() && trimmed[0] == '$') {
      target_addr = static_cast<uint16_t>(parse_hex(trimmed));
    } else {
      // Should not reach here - label resolution happens in assembler
      throw std::runtime_error("Branch target must be resolved address");
    }

    // Get branch opcode for this mnemonic
    uint8_t branch_opcode = 0;
    if (mnemonic == M6502Mnemonics::BEQ)
      branch_opcode = Opcodes::BEQ;
    else if (mnemonic == M6502Mnemonics::BNE)
      branch_opcode = Opcodes::BNE;
    else if (mnemonic == M6502Mnemonics::BCC || mnemonic == M6502Mnemonics::BLT)
      branch_opcode = Opcodes::BCC;
    else if (mnemonic == M6502Mnemonics::BCS)
      branch_opcode = Opcodes::BCS;
    else if (mnemonic == M6502Mnemonics::BMI)
      branch_opcode = Opcodes::BMI;
    else if (mnemonic == M6502Mnemonics::BPL)
      branch_opcode = Opcodes::BPL;
    else if (mnemonic == M6502Mnemonics::BVC)
      branch_opcode = Opcodes::BVC;
    else if (mnemonic == M6502Mnemonics::BVS)
      branch_opcode = Opcodes::BVS;
    else if (mnemonic == M6502Mnemonics::BRA)
      branch_opcode = Opcodes::BRA;

    // Use branch relaxation (handles both short and long branches)
    return EncodeBranchWithRelaxation(branch_opcode, current_address,
                                      target_addr);
  }

  // MVN/MVP (Block Move with two operands)
  if (mnemonic == M6502Mnemonics::MVN || mnemonic == M6502Mnemonics::MVP) {
    // Parse operands: "srcbank,destbank" or "$E1,$01"
    std::string trimmed_operand = trim(operand);
    size_t comma_pos = trimmed_operand.find(',');

    if (comma_pos == std::string::npos) {
      throw std::runtime_error(mnemonic +
                               " requires two operands: srcbank,destbank");
    }

    // Extract source and dest banks
    std::string src_str = trim(trimmed_operand.substr(0, comma_pos));
    std::string dst_str = trim(trimmed_operand.substr(comma_pos + 1));

    // Helper lambda to parse bank value
    auto parse_bank = [&parse_hex](const std::string &str) -> uint8_t {
      if (!str.empty() && str[0] == '$') {
        return static_cast<uint8_t>(parse_hex(str) & 0xFF);
      }
      return static_cast<uint8_t>(std::stoul(str, nullptr, 10) & 0xFF);
    };

    try {
      uint8_t srcbank = parse_bank(src_str);
      uint8_t destbank = parse_bank(dst_str);

      // Encode the instruction (MVN/MVP are 65816-specific)
      return (mnemonic == M6502Mnemonics::MVN) ? EncodeMVN(srcbank, destbank)
                                               : EncodeMVP(srcbank, destbank);

    } catch (const std::exception &e) {
      throw std::runtime_error("Invalid bank values for " + mnemonic + ": " +
                               e.what());
    }
  }

  // If we get here, instruction doesn't support special encoding
  throw std::invalid_argument(
      "Special encoding not supported for instruction: " + mnemonic);
}

// ============================================================================
// CpuPlugin Interface Implementation - HasOpcode()
// ============================================================================

/**
 * @brief Check if a mnemonic is a valid opcode for the 6502 family
 *
 * Determines whether the given mnemonic represents a valid instruction
 * for the 6502 family (6502/65C02/65816). Used by syntax parsers to
 * distinguish between opcodes and labels.
 *
 * @param mnemonic Instruction mnemonic (e.g., "LDA", "JMP", "ADD")
 * @return true if mnemonic is a valid opcode, false otherwise
 *
 * @note Case-insensitive comparison (accepts "lda", "LDA", "Lda")
 * @note Does NOT check addressing mode validity, only mnemonic validity
 * @note Includes all 6502/65C02/65816 opcodes regardless of current cpu_mode_
 */
bool Cpu6502::HasOpcode(const std::string &mnemonic) const {
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Create static set of all 6502 family mnemonics for O(1) lookup
  // Includes: 6502 base, 65C02 additions, 65C02 Rockwell extensions, 65816
  // additions
  static const std::unordered_set<std::string> valid_opcodes = {
      // Base 6502 opcodes
      M6502Mnemonics::LDA, M6502Mnemonics::LDX, M6502Mnemonics::LDY,
      M6502Mnemonics::STA, M6502Mnemonics::STX, M6502Mnemonics::STY,
      M6502Mnemonics::ADC, M6502Mnemonics::SBC, M6502Mnemonics::INC,
      M6502Mnemonics::DEC, M6502Mnemonics::INX, M6502Mnemonics::INY,
      M6502Mnemonics::DEX, M6502Mnemonics::DEY, M6502Mnemonics::AND,
      M6502Mnemonics::ORA, M6502Mnemonics::EOR, M6502Mnemonics::BIT,
      M6502Mnemonics::CMP, M6502Mnemonics::CPX, M6502Mnemonics::CPY,
      M6502Mnemonics::BEQ, M6502Mnemonics::BNE, M6502Mnemonics::BCS,
      M6502Mnemonics::BCC, M6502Mnemonics::BMI, M6502Mnemonics::BPL,
      M6502Mnemonics::BVS, M6502Mnemonics::BVC,
      M6502Mnemonics::BLT, // Alias for BCC
      M6502Mnemonics::JMP, M6502Mnemonics::JSR, M6502Mnemonics::RTS,
      M6502Mnemonics::RTI, M6502Mnemonics::PHA, M6502Mnemonics::PLA,
      M6502Mnemonics::PHP, M6502Mnemonics::PLP, M6502Mnemonics::ASL,
      M6502Mnemonics::LSR, M6502Mnemonics::ROL, M6502Mnemonics::ROR,
      M6502Mnemonics::CLC, M6502Mnemonics::SEC, M6502Mnemonics::CLD,
      M6502Mnemonics::SED, M6502Mnemonics::CLI, M6502Mnemonics::SEI,
      M6502Mnemonics::CLV, M6502Mnemonics::TAX, M6502Mnemonics::TXA,
      M6502Mnemonics::TAY, M6502Mnemonics::TYA, M6502Mnemonics::TSX,
      M6502Mnemonics::TXS, M6502Mnemonics::NOP, M6502Mnemonics::BRK,

      // 65C02 additions
      M6502Mnemonics::PHX, M6502Mnemonics::PLX, M6502Mnemonics::PHY,
      M6502Mnemonics::PLY, M6502Mnemonics::STZ, M6502Mnemonics::TRB,
      M6502Mnemonics::TSB, M6502Mnemonics::BRA, M6502Mnemonics::STP,
      M6502Mnemonics::WAI,

      // 65C02 Rockwell extensions (RMB, SMB, BBR, BBS)
      RockwellMnemonics::RMB0, RockwellMnemonics::RMB1, RockwellMnemonics::RMB2,
      RockwellMnemonics::RMB3, RockwellMnemonics::RMB4, RockwellMnemonics::RMB5,
      RockwellMnemonics::RMB6, RockwellMnemonics::RMB7, RockwellMnemonics::SMB0,
      RockwellMnemonics::SMB1, RockwellMnemonics::SMB2, RockwellMnemonics::SMB3,
      RockwellMnemonics::SMB4, RockwellMnemonics::SMB5, RockwellMnemonics::SMB6,
      RockwellMnemonics::SMB7, RockwellMnemonics::BBR0, RockwellMnemonics::BBR1,
      RockwellMnemonics::BBR2, RockwellMnemonics::BBR3, RockwellMnemonics::BBR4,
      RockwellMnemonics::BBR5, RockwellMnemonics::BBR6, RockwellMnemonics::BBR7,
      RockwellMnemonics::BBS0, RockwellMnemonics::BBS1, RockwellMnemonics::BBS2,
      RockwellMnemonics::BBS3, RockwellMnemonics::BBS4, RockwellMnemonics::BBS5,
      RockwellMnemonics::BBS6, RockwellMnemonics::BBS7,

      // 65816 additions
      M6502Mnemonics::PHB, M6502Mnemonics::PLB, M6502Mnemonics::PHD,
      M6502Mnemonics::PLD, M6502Mnemonics::PHK, M6502Mnemonics::TCD,
      M6502Mnemonics::TCS, M6502Mnemonics::TDC, M6502Mnemonics::TSC,
      M6502Mnemonics::JML, M6502Mnemonics::JSL, M6502Mnemonics::RTL,
      M6502Mnemonics::PEA, M6502Mnemonics::PEI, M6502Mnemonics::PER,
      M6502Mnemonics::MVN, M6502Mnemonics::MVP, M6502Mnemonics::COP,
      M6502Mnemonics::WDM, M6502Mnemonics::XBA, M6502Mnemonics::XCE,
      M6502Mnemonics::REP, M6502Mnemonics::SEP};

  return valid_opcodes.find(upper) != valid_opcodes.end();
}

} // namespace xasm
