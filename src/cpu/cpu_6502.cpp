// 6502 CPU implementation - Core 6502 opcodes

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/cpu_error_utils.h"
#include "xasm++/cpu/opcodes_6502.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <unordered_map>
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
                                              AddressingMode mode,
                                              ImmWidth imm_width) const {
  std::vector<uint8_t> bytes;
  std::optional<uint8_t> opcode;

  // Map addressing mode to opcode from table
  switch (mode) {
  case AddressingMode::Immediate: // NOLINT(bugprone-branch-clone)
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

  case AddressingMode::IndirectZeroPage: // NOLINT(bugprone-branch-clone)
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

  case AddressingMode::AbsoluteLong: // NOLINT(bugprone-branch-clone)
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

  // ZeroPage → Absolute fallback: some instructions (JSR, JMP) have no ZP
  // mode.  When an address fits in ZP range ($00-$FF) but the instruction only
  // supports Absolute, fall back so that e.g. `jsr $005c` emits $20 $5c $00
  // rather than being silently dropped.
  if (!opcode.has_value() && mode == AddressingMode::ZeroPage &&
      table.absolute.has_value()) {
    opcode = table.absolute;
    mode = AddressingMode::Absolute;
  }

  // ZeroPageX → AbsoluteX fallback: same reasoning for indexed X mode.
  if (!opcode.has_value() && mode == AddressingMode::ZeroPageX &&
      table.absolute_x.has_value()) {
    opcode = table.absolute_x;
    mode = AddressingMode::AbsoluteX;
  }

  // ZeroPageY → AbsoluteY fallback: some instructions (STA, LDA) have no ZP,Y
  // mode on the 6502/65C02 but do have ABS,Y.  When the caller requested ZP,Y
  // but the table has no opcode for it, try AbsoluteY instead so that e.g.
  // `sta 0,y` emits $99 $00 $00 (ABS,Y) rather than being silently dropped.
  if (!opcode.has_value() && mode == AddressingMode::ZeroPageY &&
      table.absolute_y.has_value()) {
    opcode = table.absolute_y;
    mode = AddressingMode::AbsoluteY;
  }

  // If opcode found, encode instruction
  if (opcode.has_value()) {
    bytes.push_back(opcode.value());

    // Add operand bytes based on addressing mode
    switch (mode) {
    case AddressingMode::Immediate: {
      // In 65816 native mode, accumulator/index instructions may use
      // 16-bit immediates depending on the M and X status flags.
      bool use_16bit = false;
      if (cpu_mode_ == CpuMode::Cpu65816) {
        if (imm_width == ImmWidth::UseM && !m_flag_) {
          use_16bit = true;
        }
        if (imm_width == ImmWidth::UseX && !x_flag_) {
          use_16bit = true;
        }
      }
      bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
      if (use_16bit) {
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
      }
      break;
    }
    case AddressingMode::ZeroPage: // NOLINT(bugprone-branch-clone)
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
    default:
      // No operand bytes
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
  static const OpcodeTable kLdaTable = {
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

  return EncodeWithTable(kLdaTable, operand, mode, ImmWidth::UseM);
}

// STA - Store Accumulator
std::vector<uint8_t> Cpu6502::EncodeSTA(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kStaTable = {
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
      .indirect_zero_page = Opcodes::STA_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kStaTable, operand, mode);
}

// JMP - Jump
std::vector<uint8_t> Cpu6502::EncodeJMP(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kJmpTable = {
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
  return EncodeWithTable(kJmpTable, operand, mode);
}

// NOP - No Operation
std::vector<uint8_t> Cpu6502::EncodeNOP() { return {Opcodes::NOP}; }

// RTS - Return from Subroutine
std::vector<uint8_t> Cpu6502::EncodeRTS() { return {Opcodes::RTS}; }

// Phase 2.2: Arithmetic Instructions

// ADC - Add with Carry
std::vector<uint8_t> Cpu6502::EncodeADC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kAdcTable = {
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
      .indirect_zero_page = Opcodes::ADC_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kAdcTable, operand, mode, ImmWidth::UseM);
}

// SBC - Subtract with Carry
std::vector<uint8_t> Cpu6502::EncodeSBC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kSbcTable = {
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
      .indirect_zero_page = Opcodes::SBC_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kSbcTable, operand, mode, ImmWidth::UseM);
}

// Phase 2.2: Logic Instructions

// AND - Logical AND
std::vector<uint8_t> Cpu6502::EncodeAND(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kAndTable = {
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
      .indirect_zero_page = Opcodes::AND_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kAndTable, operand, mode, ImmWidth::UseM);
}

// ORA - Logical OR
std::vector<uint8_t> Cpu6502::EncodeORA(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kOraTable = {
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
      .indirect_zero_page = Opcodes::ORA_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kOraTable, operand, mode, ImmWidth::UseM);
}

// EOR - Exclusive OR
std::vector<uint8_t> Cpu6502::EncodeEOR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kEorTable = {
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
      .indirect_zero_page = Opcodes::EOR_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};

  return EncodeWithTable(kEorTable, operand, mode, ImmWidth::UseM);
}

// Phase 2.2: Additional Loads/Stores

// LDX - Load X Register
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kLdxTable = {
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
  return EncodeWithTable(kLdxTable, operand, mode, ImmWidth::UseX);
}

// LDY - Load Y Register
std::vector<uint8_t> Cpu6502::EncodeLDY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kLdyTable = {
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
  return EncodeWithTable(kLdyTable, operand, mode, ImmWidth::UseX);
}

// STX - Store X Register
std::vector<uint8_t> Cpu6502::EncodeSTX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kStxTable = {
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
  return EncodeWithTable(kStxTable, operand, mode);
}

// STY - Store Y Register
std::vector<uint8_t> Cpu6502::EncodeSTY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kStyTable = {
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
  return EncodeWithTable(kStyTable, operand, mode);
}

// Phase 2.2: Comparisons

// CMP - Compare Accumulator
std::vector<uint8_t> Cpu6502::EncodeCMP(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kCmpTable = {
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
      .indirect_zero_page = Opcodes::CMP_IZP, // 65C02+
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(kCmpTable, operand, mode, ImmWidth::UseM);
}

// CPX - Compare X Register
std::vector<uint8_t> Cpu6502::EncodeCPX(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kCpxTable = {
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
  return EncodeWithTable(kCpxTable, operand, mode, ImmWidth::UseX);
}

// CPY - Compare Y Register
std::vector<uint8_t> Cpu6502::EncodeCPY(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kCpyTable = {
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
  return EncodeWithTable(kCpyTable, operand, mode, ImmWidth::UseX);
}

// Phase 2.2: Branch Instructions

// BEQ - Branch if Equal
std::vector<uint8_t> Cpu6502::EncodeBEQ(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBeqTable = {
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
  return EncodeWithTable(kBeqTable, operand, mode);
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBneTable = {
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
  return EncodeWithTable(kBneTable, operand, mode);
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBccTable = {
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
  return EncodeWithTable(kBccTable, operand, mode);
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBcsTable = {
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
  return EncodeWithTable(kBcsTable, operand, mode);
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBmiTable = {
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
  return EncodeWithTable(kBmiTable, operand, mode);
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBplTable = {
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
  return EncodeWithTable(kBplTable, operand, mode);
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBvcTable = {
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
  return EncodeWithTable(kBvcTable, operand, mode);
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kBvsTable = {
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
  return EncodeWithTable(kBvsTable, operand, mode);
}

// Phase 2.2: Inc/Dec Instructions

// INX - Increment X Register
std::vector<uint8_t> Cpu6502::EncodeINX() { return {Opcodes::INX}; }

// INY - Increment Y Register
std::vector<uint8_t> Cpu6502::EncodeINY() { return {Opcodes::INY}; }

// DEX - Decrement X Register
std::vector<uint8_t> Cpu6502::EncodeDEX() { return {Opcodes::DEX}; }

// DEY - Decrement Y Register
std::vector<uint8_t> Cpu6502::EncodeDEY() { return {Opcodes::DEY}; }

// INC - Increment Memory
std::vector<uint8_t> Cpu6502::EncodeINC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kIncTable = {
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
      .accumulator = Opcodes::INC_ACC, // 65C02+ INA
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(kIncTable, operand, mode);
}

// DEC - Decrement Memory
std::vector<uint8_t> Cpu6502::EncodeDEC(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kDecTable = {
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
      .accumulator = Opcodes::DEC_ACC, // 65C02+ DEA
      .relative = std::nullopt,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(kDecTable, operand, mode);
}

// Phase 2.2: Stack Operations

// PHA - Push Accumulator
std::vector<uint8_t> Cpu6502::EncodePHA() { return {Opcodes::PHA}; }

// PLA - Pull Accumulator
std::vector<uint8_t> Cpu6502::EncodePLA() { return {Opcodes::PLA}; }

// PHP - Push Processor Status
std::vector<uint8_t> Cpu6502::EncodePHP() { return {Opcodes::PHP}; }

// PLP - Pull Processor Status
std::vector<uint8_t> Cpu6502::EncodePLP() { return {Opcodes::PLP}; }

// Phase 2.2: Subroutine

// JSR - Jump to Subroutine
std::vector<uint8_t> Cpu6502::EncodeJSR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kJsrTable = {
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
  return EncodeWithTable(kJsrTable, operand, mode);
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
  case AddressingMode::ZeroPage: // NOLINT(bugprone-branch-clone)
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
      if (mode == AddressingMode::Immediate) { // NOLINT(bugprone-branch-clone)
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
  static const OpcodeTable kAslTable = {
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
  return EncodeWithTable(kAslTable, operand, mode);
}

// LSR - Logical Shift Right
std::vector<uint8_t> Cpu6502::EncodeLSR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kLsrTable = {
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
  return EncodeWithTable(kLsrTable, operand, mode);
}

// Group 3: Rotate Instructions

// ROL - Rotate Left
std::vector<uint8_t> Cpu6502::EncodeROL(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kRolTable = {
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
  return EncodeWithTable(kRolTable, operand, mode);
}

// ROR - Rotate Right
std::vector<uint8_t> Cpu6502::EncodeROR(uint16_t operand,
                                        AddressingMode mode) const {
  static const OpcodeTable kRorTable = {
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
  return EncodeWithTable(kRorTable, operand, mode);
}

// Group 4: Interrupt Instructions

// RTI - Return from Interrupt
std::vector<uint8_t> Cpu6502::EncodeRTI() { return {Opcodes::RTI}; }

// BRK - Break
std::vector<uint8_t> Cpu6502::EncodeBRK() { return {Opcodes::BRK}; }

// Group 5: Flag Operations

// CLC - Clear Carry
std::vector<uint8_t> Cpu6502::EncodeCLC() { return {Opcodes::CLC}; }

// SEC - Set Carry
std::vector<uint8_t> Cpu6502::EncodeSEC() { return {Opcodes::SEC}; }

// CLD - Clear Decimal
std::vector<uint8_t> Cpu6502::EncodeCLD() { return {Opcodes::CLD}; }

// SED - Set Decimal
std::vector<uint8_t> Cpu6502::EncodeSED() { return {Opcodes::SED}; }

// CLI - Clear Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeCLI() { return {Opcodes::CLI}; }

// SEI - Set Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeSEI() { return {Opcodes::SEI}; }

// CLV - Clear Overflow
std::vector<uint8_t> Cpu6502::EncodeCLV() { return {Opcodes::CLV}; }

// Group 6: Transfer Instructions

// TSX - Transfer SP to X
std::vector<uint8_t> Cpu6502::EncodeTSX() { return {Opcodes::TSX}; }

// TXS - Transfer X to SP
std::vector<uint8_t> Cpu6502::EncodeTXS() { return {Opcodes::TXS}; }

// TAX - Transfer A to X
std::vector<uint8_t> Cpu6502::EncodeTAX() { return {Opcodes::TAX}; }

// TAY - Transfer A to Y
std::vector<uint8_t> Cpu6502::EncodeTAY() { return {Opcodes::TAY}; }

// TXA - Transfer X to A
std::vector<uint8_t> Cpu6502::EncodeTXA() { return {Opcodes::TXA}; }

// TYA - Transfer Y to A
std::vector<uint8_t> Cpu6502::EncodeTYA() { return {Opcodes::TYA}; }

// Calculate instruction size based on addressing mode
size_t Cpu6502::CalculateInstructionSize(AddressingMode mode) {
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
// GetInstructionSize — first-pass address estimation
// ============================================================================

/**
 * @brief Estimate instruction size from mnemonic and operand string
 *
 * Determines instruction byte count using operand-string syntax heuristics,
 * without requiring symbol resolution.  Used so that `.BS TARGET-*` and
 * similar current-address expressions compute correctly during the parse pass.
 *
 * Rules (6502/65C02/65816):
 *  - Empty operand or "A"     → 1 byte  (implied / accumulator)
 *  - "#..."                   → 2 bytes (immediate)
 *  - Branch mnemonics         → 2 bytes (relative)
 *  - "(expr,X)"               → 3 bytes (absolute indexed indirect)
 *  - "(expr),Y" or "(expr)"   → 2 bytes (ZP indirect / indexed)
 *  - "[...]"                  → 2 bytes (65816 indirect long)
 *  - ">..."                   → 4 bytes (65816 absolute long)
 *  - "$xx"  (1–2 hex digits)  → 2 bytes (explicit zero-page)
 *  - "$xxxx" or symbol        → 3 bytes (absolute)
 */
size_t Cpu6502::GetInstructionSize(const std::string &mnemonic, // NOLINT(bugprone-easily-swappable-parameters)
                                   const std::string &operand_str) const {
  // Strip trailing '!' from mnemonic before processing
  std::string clean_mnemonic = mnemonic;
  if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
    clean_mnemonic.pop_back();
  }

  // Local trim helper (avoids dependency on util header here)
  auto to_upper = [](std::string s) -> std::string {
    for (char &c : s)
      c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
  };

  const std::string kOp = util::Trim(operand_str);
  const std::string kMn = to_upper(clean_mnemonic);

  // --- Implied / Accumulator: no operand or bare "A" ---
  if (kOp.empty() || kOp == "A") {
    return 1;
  }

  // --- Immediate: #... ---
  if (kOp[0] == '#') {
    return 2;
  }

  // --- SCMASM high byte immediate: /expr ---
  // In SCMASM syntax /expr is equivalent to #>expr (immediate high byte).
  if (kOp[0] == '/') {
    return 2;
  }

  // --- Branch instructions: always relative (2 bytes) ---
  static const std::unordered_set<std::string> BRANCHES = {
      M6502Mnemonics::BEQ, M6502Mnemonics::BNE, M6502Mnemonics::BCC,
      M6502Mnemonics::BCS, M6502Mnemonics::BMI, M6502Mnemonics::BPL,
      M6502Mnemonics::BVC, M6502Mnemonics::BVS, M6502Mnemonics::BRA,
      M6502Mnemonics::BLT, // alias for BCC
  };
  if (BRANCHES.contains(kMn)) {
    return 2;
  }

  // --- Indirect modes: (...) ---
  if (kOp[0] == '(') {
    size_t close = kOp.find(')');
    if (close != std::string::npos) {
      const std::string kInsideUpper = to_upper(kOp.substr(1, close - 1));
      // (abs,X) → AbsoluteIndexedIndirect = 3 bytes
      if (kInsideUpper.find(",X") != std::string::npos) {
        return 3;
      }
      // (zp),Y → IndirectY = 2 bytes
      if (close + 1 < kOp.length()) {
        const std::string kAfterUpper =
            to_upper(util::Trim(kOp.substr(close + 1)));
        if (kAfterUpper == ",Y") {
          return 2;
        }
      }
      // JMP (abs) → absolute indirect = 3 bytes
      // (JMP is the only 6502/65C02 instruction that uses (abs) indirect;
      // all other instructions with (sym) use ZP indirect = 2 bytes)
      if (kMn == M6502Mnemonics::JMP || kMn == M6502Mnemonics::JSR) {
        return 3;
      }
      // (zp) → ZP indirect (65C02) = 2 bytes
      return 2;
    }
    return 2; // unmatched paren — assume 2
  }

  // --- 65816 indirect long: [...] ---
  if (kOp[0] == '[') {
    return 2;
  }

  // --- 65816 absolute long: >... ---
  if (kOp[0] == '>') {
    return 4;
  }

  // --- Indexed modes: has ,X or ,Y suffix ---
  const std::string kOpUpper = to_upper(kOp);
  const size_t kCommaX = kOpUpper.find(",X");
  const size_t kCommaY = kOpUpper.find(",Y");
  const size_t kCommaPos =
      (kCommaX != std::string::npos) ? kCommaX : kCommaY;
  if (kCommaPos != std::string::npos) {
    const std::string kAddrPart = util::Trim(kOp.substr(0, kCommaPos));
    if (!kAddrPart.empty() && kAddrPart[0] == '$') {
      // $xx (≤2 hex digits) → ZP indexed = 2 bytes
      const size_t kHexDigits = kAddrPart.length() - 1;
      return (kHexDigits <= 2) ? 2 : 3;
    }
    return 3; // symbol indexed → absolute
  }

  // --- Explicit hex literal ---
  if (kOp[0] == '$') {
    // $xx (1–2 hex digits) → ZP = 2 bytes; $xxxx = absolute = 3 bytes
    const size_t kHexDigits = kOp.length() - 1;
    return (kHexDigits <= 2) ? 2 : 3;
  }

  // --- Everything else (symbol reference) → assume absolute = 3 bytes ---
  return 3;
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
 * @param target Branch source and destination addresses
 * @return true if branch needs relaxation (out of range), false otherwise
 */
bool Cpu6502::NeedsBranchRelaxation(BranchTarget target) const {
  return Cpu6502BranchHandler::NeedsBranchRelaxation(target);
}

uint8_t Cpu6502::GetComplementaryBranchOpcode(uint8_t branch_opcode) const {
  return Cpu6502BranchHandler::GetComplementaryBranchOpcode(branch_opcode);
}

std::vector<uint8_t> Cpu6502::EncodeBranchWithRelaxation(
    uint8_t branch_opcode, BranchTarget target) const {
  return Cpu6502BranchHandler::EncodeBranchWithRelaxation(branch_opcode, target);
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

  return {Opcodes::WAI};
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

  return {Opcodes::STP};
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

std::vector<uint8_t> Cpu6502::EncodeBBR0(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR0, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR1(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR1, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR2(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR2, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR3(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR3, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR4(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR4, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR5(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR5, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR6(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBR6, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR7(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
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

std::vector<uint8_t> Cpu6502::EncodeBBS0(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS0, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS1(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS1, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS2(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS2, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS3(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS3, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS4(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS4, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS5(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS5, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS6(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
                                         uint8_t offset) const {
  if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
    return {};
  }
  return {RockwellOpcodes::BBS6, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS7(uint8_t zp_addr, // NOLINT(bugprone-easily-swappable-parameters)
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
  return {Opcodes::PHX};
}

std::vector<uint8_t> Cpu6502::EncodePLX() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {Opcodes::PLX};
}

std::vector<uint8_t> Cpu6502::EncodePHY() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {Opcodes::PHY};
}

std::vector<uint8_t> Cpu6502::EncodePLY() const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }
  return {Opcodes::PLY};
}

// ============================================================================
// Phase 2.5 - Group 3: 65C02 Store Zero
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeSTZ(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {}; // Not available in base 6502
  }

  static const OpcodeTable kStzTable = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::STZ_ZP,
      .zero_page_x = Opcodes::STZ_ZPX,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::STZ_ABS,
      .absolute_x = Opcodes::STZ_ABX,
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
  return EncodeWithTable(kStzTable, operand, mode);
}

// ============================================================================
// Phase 2.5 - Group 4: 65C02 Bit Test
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeTRB(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable kTrbTable = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::TRB_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::TRB_ABS,
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
  return EncodeWithTable(kTrbTable, operand, mode);
}

std::vector<uint8_t> Cpu6502::EncodeTSB(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable kTsbTable = {
      .immediate = std::nullopt,
      .zero_page = Opcodes::TSB_ZP,
      .zero_page_x = std::nullopt,
      .zero_page_y = std::nullopt,
      .absolute = Opcodes::TSB_ABS,
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
  return EncodeWithTable(kTsbTable, operand, mode);
}

// ============================================================================
// Phase 2.5 - Group 5: 65C02 Branch Always
// ============================================================================

std::vector<uint8_t> Cpu6502::EncodeBRA(uint16_t operand,
                                        AddressingMode mode) const {
  if (cpu_mode_ == CpuMode::Cpu6502) {
    return {};
  }

  static const OpcodeTable kBraTable = {
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
      .relative = Opcodes::BRA,
      .indirect_zero_page = std::nullopt,
      .absolute_indexed_indirect = std::nullopt,
      .absolute_long = std::nullopt,
      .indirect_long = std::nullopt,
      .indirect_long_indexed_y = std::nullopt,
      .stack_relative = std::nullopt,
      .stack_relative_indirect_indexed_y = std::nullopt};
  return EncodeWithTable(kBraTable, operand, mode);
}

// ============================================================================
// Phase 2.5 - 65816 Transfer Instructions (no CPU state access)
// ============================================================================

// TXY - Transfer X to Y (65816 implied, opcode $9B)
std::vector<uint8_t> Cpu6502::EncodeTXY() {
  return {Opcodes::TXY};
}

// TYX - Transfer Y to X (65816 implied, opcode $BB)
std::vector<uint8_t> Cpu6502::EncodeTYX() {
  return {Opcodes::TYX};
}

std::vector<uint8_t> Cpu6502::EncodeXCE() const {
  // XCE exchanges the carry flag with the 65816 emulation bit (e).
  // The assembler cannot know the carry value at assembly time, so we apply a
  // conservative rule: always reset M and X to 8-bit (true) after XCE.
  //
  // This is correct for BOTH transition directions:
  //   clc / xce  → switch to native mode: M=1,X=1 reset here is immediately
  //                 overridden by the rep #$30 / rep #$20 that convention
  //                 requires after entering native mode.
  //   sec / xce  → switch to emulation mode: M=1,X=1 is exactly the CPU's
  //                 state in emulation mode, so subsequent immediates are
  //                 assembled as 8-bit (2-byte) as required.
  m_flag_ = true;
  x_flag_ = true;
  return {Opcodes::XCE};
}

std::vector<uint8_t> Cpu6502::EncodeSEP(uint16_t value,
                                        AddressingMode mode) const {
  // SEP only has Immediate mode; accept any mode for vasm compatibility
  // (source may use 'sep $30' without '#' prefix)
  (void)mode;
  // Update M/X flags: SEP sets bits (1 = 8-bit)
  // Bit 5 (0x20) = M flag, Bit 4 (0x10) = X flag
  if (value & 0x20) {
    m_flag_ = true;  // set M → 8-bit accumulator
  }
  if (value & 0x10) {
    x_flag_ = true;  // set X → 8-bit index
  }
  return {Opcodes::SEP, static_cast<uint8_t>(value & 0xFF)};
}

std::vector<uint8_t> Cpu6502::EncodeREP(uint16_t value,
                                        AddressingMode mode) const {
  // REP only has Immediate mode; accept any mode for vasm compatibility
  // (source may use 'rep $30' without '#' prefix)
  (void)mode;
  // Update M/X flags: REP clears bits (0 = 16-bit)
  // Bit 5 (0x20) = M flag, Bit 4 (0x10) = X flag
  if (value & 0x20) {
    m_flag_ = false;  // clear M → 16-bit accumulator
  }
  if (value & 0x10) {
    x_flag_ = false;  // clear X → 16-bit index
  }
  return {Opcodes::REP, static_cast<uint8_t>(value & 0xFF)};
}

// ============================================================================
// CPU Mode Configuration
// ============================================================================

void Cpu6502::SetCpuMode(CpuMode mode) { cpu_mode_ = mode; }

CpuMode Cpu6502::GetCpuMode() const { return cpu_mode_; }

void Cpu6502::SetCpuModeFromAtom(int mode) {
  switch (mode) {
    case 0:
      SetCpuMode(CpuMode::Cpu6502);
      break;
    case 1:
      SetCpuMode(CpuMode::Cpu65C02);
      break;
    case 2:
      SetCpuMode(CpuMode::Cpu65816);
      break;
    default:
      break;
  }
  // Reset MX flags to 8-bit default whenever the CPU mode changes.
  // REP/SEP instructions in the atom stream will update them as they
  // are encoded in-order, so each pass sees consistent state.
  m_flag_ = true;
  x_flag_ = true;
}

void Cpu6502::SetMX(bool m_flag, bool x_flag) {
  m_flag_ = m_flag;
  x_flag_ = x_flag;
}

bool Cpu6502::IsAccumulator8Bit() const { return m_flag_; }

bool Cpu6502::IsIndex8Bit() const { return x_flag_; }

void Cpu6502::SetRelaxBranches(bool relax) { relax_branches_ = relax; }

// ============================================================================
// CpuPlugin Interface Implementation - EncodeInstruction()
// ============================================================================

/**
 * @brief Static dispatch table: mnemonic -> encoder wrapper
 *
 * Each entry holds a callable that forwards (cpu_ptr, operand, mode) to the
 * appropriate Encode* member function.  The table is built once on first call
 * and then reused — O(1) dispatch replaces the original 89-branch if-chain.
 */
// File-local alias for the private EncFn type (mirrors Cpu6502::EncFn).
using EncFnAlias =
    std::function<std::vector<uint8_t>(const Cpu6502 *, uint32_t, AddressingMode)>;

// Helpers that populate subsections of the encoder table.
// Each covers a logical group of ~15 instructions to keep per-function CC low.
static void PopulateLoadStoreArith(std::unordered_map<std::string, EncFnAlias> &t) {
  // clang-format off
  t[LDA] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeLDA(op, m); };
  t[LDX] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeLDX(op, m); };
  t[LDY] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeLDY(op, m); };
  t[STA] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSTA(op, m); };
  t[STX] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSTX(op, m); };
  t[STY] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSTY(op, m); };
  t[STZ] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSTZ(op, m); };
  t[ADC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeADC(op, m); };
  t[SBC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSBC(op, m); };
  t[INC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeINC(op, m); };
  t[DEC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeDEC(op, m); };
  t[INX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeINX(); };
  t[INY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeINY(); };
  t[DEX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeDEX(); };
  t[DEY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeDEY(); };
  // clang-format on
}

static void PopulateLogicCompareBranch(std::unordered_map<std::string, EncFnAlias> &t) {
  // clang-format off
  t[AND] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeAND(op, m); };
  t[ORA] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeORA(op, m); };
  t[EOR] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeEOR(op, m); };
  t[BIT] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBIT(op, m); };
  t[CMP] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeCMP(op, m); };
  t[CPX] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeCPX(op, m); };
  t[CPY] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeCPY(op, m); };
  // Branch (short; relaxed branches go via EncodeInstructionSpecial)
  t[BEQ] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBEQ(op, m); };
  t[BNE] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBNE(op, m); };
  t[BCC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBCC(op, m); };
  t[BCS] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBCS(op, m); };
  t[BMI] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBMI(op, m); };
  t[BPL] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBPL(op, m); };
  t[BVC] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBVC(op, m); };
  t[BVS] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBVS(op, m); };
  t[BRA] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeBRA(op, m); };
  // clang-format on
}

static void PopulateJumpStackShift(std::unordered_map<std::string, EncFnAlias> &t) {
  // clang-format off
  t[JMP] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeJMP(op, m); };
  t[JSR] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeJSR(op, m); };
  t[RTS] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeRTS(); };
  t[RTI] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeRTI(); };
  t[PHA] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHA(); };
  t[PLA] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLA(); };
  t[PHP] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHP(); };
  t[PLP] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLP(); };
  t[PHX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHX(); };
  t[PLX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLX(); };
  t[PHY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHY(); };
  t[PLY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLY(); };
  t[ASL] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeASL(op, m); };
  t[LSR] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeLSR(op, m); };
  t[ROL] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeROL(op, m); };
  t[ROR] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeROR(op, m); };
  // clang-format on
}

static void PopulateFlagsTransferMisc(std::unordered_map<std::string, EncFnAlias> &t) {
  // clang-format off
  t[CLC] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeCLC(); };
  t[SEC] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeSEC(); };
  t[CLD] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeCLD(); };
  t[SED] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeSED(); };
  t[CLI] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeCLI(); };
  t[SEI] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeSEI(); };
  t[CLV] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeCLV(); };
  t[TAX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTAX(); };
  t[TAY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTAY(); };
  t[TXA] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTXA(); };
  t[TYA] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTYA(); };
  t[TSX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTSX(); };
  t[TXS] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTXS(); };
  t[NOP] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeNOP(); };
  t[BRK] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeBRK(); };
  t[TRB] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeTRB(op, m); };
  t[TSB] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeTSB(op, m); };
  // clang-format on
}

static void Populate65816(std::unordered_map<std::string, EncFnAlias> &t) {
  // clang-format off
  t[PHB] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHB(); };
  t[PLB] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLB(); };
  t[PHK] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHK(); };
  t[PHD] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePHD(); };
  t[PLD] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodePLD(); };
  t[TCD] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTCD(); };
  t[TDC] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTDC(); };
  t[TCS] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTCS(); };
  t[TSC] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTSC(); };
  t[TXY] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTXY(); };
  t[TYX] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeTYX(); };
  t[JML] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeJML(op, m); };
  t[JSL] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeJSL(op, m); };
  t[RTL] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeRTL(); };
  t[PEA] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodePEA(op, m); };
  t[PEI] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodePEI(op, m); };
  t[PER] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodePER(op, m); };
  t[XBA] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeXBA(); };
  t[XCE] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeXCE(); };
  t[SEP] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeSEP(op, m); };
  t[REP] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeREP(op, m); };
  t[COP] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeCOP(op, m); };
  t[WDM] = [](const Cpu6502 *c, uint32_t op, AddressingMode m) { return c->EncodeWDM(op, m); };
  t[WAI] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeWAI(); };
  t[STP] = [](const Cpu6502 *c, uint32_t,    AddressingMode)   { return c->EncodeSTP(); };
  // clang-format on
}

const std::unordered_map<std::string, Cpu6502::EncFn> &Cpu6502::EncoderTable() {
  static const std::unordered_map<std::string, Cpu6502::EncFn> kTable = []() {
    std::unordered_map<std::string, EncFnAlias> t;
    t.reserve(96);
    PopulateLoadStoreArith(t);
    PopulateLogicCompareBranch(t);
    PopulateJumpStackShift(t);
    PopulateFlagsTransferMisc(t);
    Populate65816(t);
    return t;
  }();
  return kTable;
}

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
Cpu6502::EncodeInstruction(const std::string &mnemonic, uint32_t operand, // NOLINT(bugprone-easily-swappable-parameters)
                           const std::string &operand_str) const {
  // Strip trailing '!' from mnemonic before processing
  // The '!' suffix is used in some assembly dialects to force
  // a specific instruction encoding but should not affect the
  // mnemonic lookup in the opcode table.
  std::string clean_mnemonic = mnemonic;
  if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
    clean_mnemonic.pop_back();
  }

  // Helper to parse hex value
  auto parse_hex = [](const std::string &s) -> uint32_t {
    if (s.empty() || s[0] != '$')
      return 0;
    return std::stoul(s.substr(1), nullptr, Opcodes::RADIX_HEXADECIMAL);
  };

  // Determine addressing mode from operand_str
  std::string trimmed = util::Trim(operand_str);
  // Normalize to uppercase for addressing mode detection.
  // SCMASM source may use lowercase register suffixes (,x / ,y) while
  // Merlin uses uppercase.  Symbol values are resolved into the numeric
  // `operand` parameter before EncodeInstruction is called, so uppercasing
  // the string here only affects mode-detection string comparisons, not
  // symbol lookups.
  std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  AddressingMode mode = AddressingMode::Implied;

  // Shift/rotate/inc/dec instructions with no operand mean accumulator mode.
  // SCMASM uses "asl" / "inc" (no operand) instead of the explicit "asl A" form.
  if (trimmed.empty() &&
      (clean_mnemonic == ASL || clean_mnemonic == LSR || clean_mnemonic == ROL ||
       clean_mnemonic == ROR || clean_mnemonic == INC || clean_mnemonic == DEC)) {
    mode = AddressingMode::Accumulator;
  }

  if (!trimmed.empty()) {
    // Accumulator mode
    if (trimmed == "A") {
      mode = AddressingMode::Accumulator;
    }
    // Immediate mode (# prefix or SCMASM / high-byte prefix)
    else if (trimmed[0] == '#' || trimmed[0] == '/') {
      mode = AddressingMode::Immediate;
    }
    // 65816 indirect long: [$zp] or [$zp],Y
    else if (trimmed[0] == '[') {
      if (trimmed.find(",Y") != std::string::npos) {
        mode = AddressingMode::IndirectLongIndexedY;
      } else {
        mode = AddressingMode::IndirectLong;
      }
    }
    // Indirect modes
    else if (trimmed[0] == '(') {
      size_t close_paren = trimmed.find(')');
      if (close_paren != std::string::npos) {
        std::string inside = trimmed.substr(1, close_paren - 1);
        inside = util::Trim(inside);

        // Check for indexed indirect: ($80,X) or absolute indexed indirect: ($1234,X)
        // 65C02 JMP (abs,X) uses AbsoluteIndexedIndirect ($7C); zero-page ($80,X)
        // uses IndirectX.  Distinguish by the resolved operand value.
        if (inside.find(",X") != std::string::npos ||
            inside.find(", X") != std::string::npos) {
          mode = (operand > 0xFF) ? AddressingMode::AbsoluteIndexedIndirect
                                  : AddressingMode::IndirectX;
        }
        // Check for indirect indexed: ($80),Y
        else if (close_paren < trimmed.length() - 1) {
          std::string after = util::Trim(trimmed.substr(close_paren + 1));
          if (after == ",Y" || after == ", Y") {
            mode = AddressingMode::IndirectY;
          }
        }
        // JMP (abs): absolute indirect = Indirect
        // All other instructions (LDA, STA, ADC, etc.): (zp) = IndirectZeroPage (65C02+)
        else if (clean_mnemonic == M6502Mnemonics::JMP) {
          mode = AddressingMode::Indirect;
        } else {
          mode = AddressingMode::IndirectZeroPage;
        }
      }
    }
    // Indexed modes: ,X or ,Y
    else if (trimmed.find(",X") != std::string::npos ||
             trimmed.find(", X") != std::string::npos) {
      size_t comma_pos = trimmed.find(",X");
      if (comma_pos == std::string::npos) {
        comma_pos = trimmed.find(", X");
      }
      std::string addr_part = util::Trim(trimmed.substr(0, comma_pos));

      if (!addr_part.empty() && addr_part[0] == '$') {
        // Explicit hex value - use value to determine mode
        uint32_t val = parse_hex(addr_part);
        mode = (val <= 0xFF) ? AddressingMode::ZeroPageX
                             : AddressingMode::AbsoluteX;
      } else {
        // Symbol reference - default to Absolute for compatibility
        // BUG-001 FIX: Symbol - use resolved operand value to determine mode
        // BUT: Only use ZeroPage if value is clearly in ZeroPage range (1-255)
        mode = (operand <= 0xFF) ? AddressingMode::ZeroPageX
                                 : AddressingMode::AbsoluteX;
      }
    } else if (trimmed.find(",Y") != std::string::npos ||
               trimmed.find(", Y") != std::string::npos) {
      size_t comma_pos = trimmed.find(",Y");
      if (comma_pos == std::string::npos) {
        comma_pos = trimmed.find(", Y");
      }
      std::string addr_part = util::Trim(trimmed.substr(0, comma_pos));

      if (!addr_part.empty() && addr_part[0] == '$') {
        // Explicit hex value - use value to determine mode
        uint32_t val = parse_hex(addr_part);
        mode = (val <= 0xFF) ? AddressingMode::ZeroPageY
                             : AddressingMode::AbsoluteY;
      } else {
        // Symbol reference - default to Absolute for compatibility
        // BUG-001 FIX: Symbol - use resolved operand value to determine mode
        // BUT: Only use ZeroPage if value is clearly in ZeroPage range (1-255)
        mode = (operand <= 0xFF) ? AddressingMode::ZeroPageY
                                 : AddressingMode::AbsoluteY;
      }
    }
    // Absolute or ZeroPage
    else {
      if (trimmed[0] == '$') {
        // Explicit hex value - use value to determine mode
        uint32_t val = parse_hex(trimmed);
        mode =
            (val <= 0xFF) ? AddressingMode::ZeroPage : AddressingMode::Absolute;
      } else {
        // Symbol reference - use resolved operand value to determine mode.
        // ZeroPage is used for values 0–$FF, EXCEPT for JMP and JSR which have
        // no ZeroPage variant and must always use Absolute addressing.
        const bool kNoZpForm = (clean_mnemonic == JMP || clean_mnemonic == JSR);
        mode = (!kNoZpForm && operand <= 0xFF)
                   ? AddressingMode::ZeroPage
                   : AddressingMode::Absolute;
      }
    }
  }

  // Dispatch via static encoder table (replaces the former if-chain).
  const auto &table = EncoderTable();
  auto it = table.find(clean_mnemonic);
  if (it == table.end()) {
    cpu::ThrowUnsupportedInstruction(clean_mnemonic);
  }
  return it->second(this, operand, mode);
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
  // Strip trailing '!' from mnemonic before checking
  std::string clean_mnemonic = mnemonic;
  if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
    clean_mnemonic.pop_back();
  }

  // Branch instructions require special encoding (branch relaxation)
  // Use mnemonic constants to avoid magic strings
  if (clean_mnemonic == M6502Mnemonics::BEQ || clean_mnemonic == M6502Mnemonics::BNE ||
      clean_mnemonic == M6502Mnemonics::BCC || clean_mnemonic == M6502Mnemonics::BCS ||
      clean_mnemonic == M6502Mnemonics::BMI || clean_mnemonic == M6502Mnemonics::BPL ||
      clean_mnemonic == M6502Mnemonics::BVC || clean_mnemonic == M6502Mnemonics::BVS ||
      clean_mnemonic == M6502Mnemonics::BLT || // BLT is an alias for BCC
      clean_mnemonic == M6502Mnemonics::BRA) { // BRA (65C02+)
    return true;
  }

  // MVN/MVP (65816 block move instructions) require special encoding
  if (clean_mnemonic == M6502Mnemonics::MVN || clean_mnemonic == M6502Mnemonics::MVP) {
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
Cpu6502::EncodeInstructionSpecial(const std::string &mnemonic, // NOLINT(bugprone-easily-swappable-parameters)
                                  const std::string &operand,
                                  uint16_t current_address) const {
  // Strip trailing '!' from mnemonic before processing
  std::string clean_mnemonic = mnemonic;
  if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
    clean_mnemonic.pop_back();
  }

  // Helper to parse hex value
  auto parse_hex = [](const std::string &s) -> uint32_t {
    if (s.empty() || s[0] != '$')
      cpu::ThrowExpectedHexValue();
    return std::stoul(s.substr(1), nullptr, Opcodes::RADIX_HEXADECIMAL);
  };

  // Branch instructions with relaxation
  if (clean_mnemonic == M6502Mnemonics::BEQ || clean_mnemonic == M6502Mnemonics::BNE ||
      clean_mnemonic == M6502Mnemonics::BCC || clean_mnemonic == M6502Mnemonics::BCS ||
      clean_mnemonic == M6502Mnemonics::BMI || clean_mnemonic == M6502Mnemonics::BPL ||
      clean_mnemonic == M6502Mnemonics::BVC || clean_mnemonic == M6502Mnemonics::BVS ||
      clean_mnemonic == M6502Mnemonics::BLT || clean_mnemonic == M6502Mnemonics::BRA) {

    // Parse target address from operand string
    std::string trimmed = util::Trim(operand);
    uint16_t target_addr = 0;

    if (!trimmed.empty() && trimmed[0] == '$') {
      target_addr = static_cast<uint16_t>(parse_hex(trimmed));
    } else {
      // Should not reach here - label resolution happens in assembler
      cpu::ThrowBranchTargetMustBeResolved();
    }

    // Get branch opcode for this mnemonic.
    // BLT is an alias for BCC (Branch if Less Than).
    static const std::unordered_map<std::string, uint8_t> kBranchOpcodes = {
        {M6502Mnemonics::BEQ, Opcodes::BEQ},
        {M6502Mnemonics::BNE, Opcodes::BNE},
        {M6502Mnemonics::BCC, Opcodes::BCC},
        {M6502Mnemonics::BLT, Opcodes::BCC}, // alias for BCC
        {M6502Mnemonics::BCS, Opcodes::BCS},
        {M6502Mnemonics::BMI, Opcodes::BMI},
        {M6502Mnemonics::BPL, Opcodes::BPL},
        {M6502Mnemonics::BVC, Opcodes::BVC},
        {M6502Mnemonics::BVS, Opcodes::BVS},
        {M6502Mnemonics::BRA, Opcodes::BRA},
    };
    auto it = kBranchOpcodes.find(clean_mnemonic);
    if (it == kBranchOpcodes.end()) {
      cpu::ThrowSpecialEncodingNotSupported(clean_mnemonic);
    }
    uint8_t branch_opcode = it->second;

    // Check if branch is out of range
    if (Cpu6502BranchHandler::NeedsBranchRelaxation({current_address, target_addr})) {
      if (!relax_branches_) {
        // Error by default — matches original assembler behavior (Merlin 8, etc.)
        // Programmer is responsible for keeping branches in range.
        int16_t offset = static_cast<int16_t>(target_addr) -
                         static_cast<int16_t>(current_address + 2);
        throw std::runtime_error(
            "Branch out of range: offset " + std::to_string(offset) +
            " (must be -128 to +127); use --relax-branches to expand");
      }
    }
    // Use branch relaxation (handles both short and long branches)
    return EncodeBranchWithRelaxation(branch_opcode, {current_address, target_addr});
  }

  // MVN/MVP (Block Move with two operands)
  if (clean_mnemonic == M6502Mnemonics::MVN || mnemonic == M6502Mnemonics::MVP) {
    // Parse operands: "srcbank,destbank" or "$E1,$01"
    std::string trimmed_operand = util::Trim(operand);
    size_t comma_pos = trimmed_operand.find(',');

    if (comma_pos == std::string::npos) {
      cpu::ThrowRequiresTwoOperands(clean_mnemonic, "srcbank,destbank");
    }

    // Extract source and dest banks
    std::string src_str = util::Trim(trimmed_operand.substr(0, comma_pos));
    std::string dst_str = util::Trim(trimmed_operand.substr(comma_pos + 1));

    // Helper lambda to parse bank value
    auto parse_bank = [&parse_hex](const std::string &str) -> uint8_t {
      if (!str.empty() && str[0] == '$') {
        return static_cast<uint8_t>(parse_hex(str) & 0xFF);
      }
      return static_cast<uint8_t>(
          std::stoul(str, nullptr, Opcodes::RADIX_DECIMAL) & 0xFF);
    };

    try {
      uint8_t srcbank = parse_bank(src_str);
      uint8_t destbank = parse_bank(dst_str);

      // Encode the instruction (MVN/MVP are 65816-specific)
      return (clean_mnemonic == M6502Mnemonics::MVN) ? EncodeMVN(srcbank, destbank)
                                               : EncodeMVP(srcbank, destbank);

    } catch (const std::exception &e) {
      cpu::ThrowInvalidValues(clean_mnemonic, e.what());
    }
  }

  // If we get here, instruction doesn't support special encoding
  cpu::ThrowSpecialEncodingNotSupported(clean_mnemonic);
}

// ============================================================================
// CpuPlugin Interface Implementation - HasOpcode()
// ============================================================================

/**
 * @brief Check if a mnemonic is a valid opcode for the current CPU mode
 *
 * Determines whether the given mnemonic represents a valid instruction
 * for the currently-active CPU mode (6502/65C02/65C02Rock/65816). Used by
 * syntax parsers to distinguish between opcodes and labels.
 *
 * Only opcodes supported by the current mode are recognised:
 *   - Cpu6502:       base 6502 opcodes only
 *   - Cpu65C02:      base + 65C02 additions
 *   - Cpu65C02Rock:  base + 65C02 additions + Rockwell extensions
 *   - Cpu65816:      base + 65C02 additions + 65816 extensions
 *
 * @param mnemonic Instruction mnemonic (e.g., "LDA", "JMP", "ADD")
 * @return true if mnemonic is a valid opcode, false otherwise
 *
 * @note Case-insensitive comparison (accepts "lda", "LDA", "Lda")
 * @note Does NOT check addressing mode validity, only mnemonic validity
 * @note Includes all 6502/65C02/65816 opcodes regardless of current cpu_mode_
 */
bool Cpu6502::HasOpcode(const std::string &mnemonic) const {
  // Strip trailing '!' from mnemonic before checking
  std::string clean_mnemonic = mnemonic;
  if (!clean_mnemonic.empty() && clean_mnemonic.back() == '!') {
    clean_mnemonic.pop_back();
  }

  // Convert to uppercase for case-insensitive comparison
  std::string upper = clean_mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Base 6502 opcodes — valid in all modes
  static const std::unordered_set<std::string> kBaseOpcodes = {
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
      M6502Mnemonics::TXS, M6502Mnemonics::NOP, M6502Mnemonics::BRK};

  // 65C02 additions — valid in Cpu65C02, Cpu65C02Rock, and Cpu65816 modes
  static const std::unordered_set<std::string> kC02Opcodes = {
      M6502Mnemonics::PHX, M6502Mnemonics::PLX, M6502Mnemonics::PHY,
      M6502Mnemonics::PLY, M6502Mnemonics::STZ, M6502Mnemonics::TRB,
      M6502Mnemonics::TSB, M6502Mnemonics::BRA, M6502Mnemonics::STP,
      M6502Mnemonics::WAI};

  // 65C02 Rockwell extensions (RMB, SMB, BBR, BBS) — Cpu65C02Rock only
  static const std::unordered_set<std::string> kRockwellOpcodes = {
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
      RockwellMnemonics::BBS6, RockwellMnemonics::BBS7};

  // 65816 additions — Cpu65816 only
  static const std::unordered_set<std::string> kW816Opcodes = {
      M6502Mnemonics::PHB, M6502Mnemonics::PLB, M6502Mnemonics::PHD,
      M6502Mnemonics::PLD, M6502Mnemonics::PHK, M6502Mnemonics::TCD,
      M6502Mnemonics::TCS, M6502Mnemonics::TDC, M6502Mnemonics::TSC,
      M6502Mnemonics::TXY, M6502Mnemonics::TYX,
      M6502Mnemonics::JML, M6502Mnemonics::JSL, M6502Mnemonics::RTL,
      M6502Mnemonics::PEA, M6502Mnemonics::PEI, M6502Mnemonics::PER,
      M6502Mnemonics::MVN, M6502Mnemonics::MVP, M6502Mnemonics::COP,
      M6502Mnemonics::WDM, M6502Mnemonics::XBA, M6502Mnemonics::XCE,
      M6502Mnemonics::REP, M6502Mnemonics::SEP};

  // All modes include the base 6502 opcode set
  if (kBaseOpcodes.contains(upper)) {
    return true;
  }

  // 65C02 additions are recognised in 65C02, 65C02 Rockwell, and 65816 modes
  if (cpu_mode_ == CpuMode::Cpu65C02 || cpu_mode_ == CpuMode::Cpu65C02Rock ||
      cpu_mode_ == CpuMode::Cpu65816) {
    if (kC02Opcodes.contains(upper)) {
      return true;
    }
  }

  // Rockwell extensions are only recognised in the Rockwell variant
  if (cpu_mode_ == CpuMode::Cpu65C02Rock) {
    if (kRockwellOpcodes.contains(upper)) {
      return true;
    }
  }

  // 65816-only opcodes are only recognised in 65816 mode
  if (cpu_mode_ == CpuMode::Cpu65816) {
    if (kW816Opcodes.contains(upper)) {
      return true;
    }
  }

  return false;
}

} // namespace xasm
