// 6502 CPU implementation - Core 6502 opcodes

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/opcodes_6502.h"

namespace xasm {

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
std::vector<uint8_t> Cpu6502::EncodeWithTable(const OpcodeTable& table, 
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
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) const {
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
        .indirect_zero_page = Opcodes::LDA_IZP,           // 65C02+
        .absolute_indexed_indirect = std::nullopt,
        .absolute_long = Opcodes::LDA_ALG,                // 65816
        .indirect_long = Opcodes::LDA_ILG,                // 65816
        .indirect_long_indexed_y = Opcodes::LDA_ILY,      // 65816
        .stack_relative = Opcodes::LDA_SR,                // 65816
        .stack_relative_indirect_indexed_y = Opcodes::LDA_SRY  // 65816
    };

    return EncodeWithTable(LDA_TABLE, operand, mode);
}

// STA - Store Accumulator
std::vector<uint8_t> Cpu6502::EncodeSTA(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(STA_TABLE, operand, mode);
}

// JMP - Jump
std::vector<uint8_t> Cpu6502::EncodeJMP(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable JMP_TABLE = {
        .absolute = Opcodes::JMP_ABS,
        .indirect = Opcodes::JMP_IND,
        .absolute_indexed_indirect = Opcodes::JMP_AIX  // 65C02+
    };
    return EncodeWithTable(JMP_TABLE, operand, mode);
}

// NOP - No Operation
std::vector<uint8_t> Cpu6502::EncodeNOP() const {
    return {Opcodes::NOP};
}

// RTS - Return from Subroutine
std::vector<uint8_t> Cpu6502::EncodeRTS() const {
    return {Opcodes::RTS};
}

// Phase 2.2: Arithmetic Instructions

// ADC - Add with Carry
std::vector<uint8_t> Cpu6502::EncodeADC(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(ADC_TABLE, operand, mode);
}

// SBC - Subtract with Carry
std::vector<uint8_t> Cpu6502::EncodeSBC(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(SBC_TABLE, operand, mode);
}

// Phase 2.2: Logic Instructions

// AND - Logical AND
std::vector<uint8_t> Cpu6502::EncodeAND(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(AND_TABLE, operand, mode);
}

// ORA - Logical OR
std::vector<uint8_t> Cpu6502::EncodeORA(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(ORA_TABLE, operand, mode);
}

// EOR - Exclusive OR
std::vector<uint8_t> Cpu6502::EncodeEOR(uint16_t operand, AddressingMode mode) const {
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
        .stack_relative_indirect_indexed_y = std::nullopt
    };

    return EncodeWithTable(EOR_TABLE, operand, mode);
}

// Phase 2.2: Additional Loads/Stores

// LDX - Load X Register
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable LDX_TABLE = {
        .immediate = Opcodes::LDX_IMM,
        .zero_page = Opcodes::LDX_ZP,
        .zero_page_x = std::nullopt,
        .zero_page_y = Opcodes::LDX_ZPY,
        .absolute = Opcodes::LDX_ABS,
        .absolute_x = std::nullopt,
        .absolute_y = Opcodes::LDX_ABY
    };
    return EncodeWithTable(LDX_TABLE, operand, mode);
}

// LDY - Load Y Register
std::vector<uint8_t> Cpu6502::EncodeLDY(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable LDY_TABLE = {
        .immediate = Opcodes::LDY_IMM,
        .zero_page = Opcodes::LDY_ZP,
        .zero_page_x = Opcodes::LDY_ZPX,
        .zero_page_y = std::nullopt,
        .absolute = Opcodes::LDY_ABS,
        .absolute_x = Opcodes::LDY_ABX,
        .absolute_y = std::nullopt
    };
    return EncodeWithTable(LDY_TABLE, operand, mode);
}

// STX - Store X Register
std::vector<uint8_t> Cpu6502::EncodeSTX(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable STX_TABLE = {
        .zero_page = Opcodes::STX_ZP,
        .zero_page_x = std::nullopt,
        .zero_page_y = Opcodes::STX_ZPY,
        .absolute = Opcodes::STX_ABS
    };
    return EncodeWithTable(STX_TABLE, operand, mode);
}

// STY - Store Y Register
std::vector<uint8_t> Cpu6502::EncodeSTY(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable STY_TABLE = {
        .zero_page = Opcodes::STY_ZP,
        .zero_page_x = Opcodes::STY_ZPX,
        .zero_page_y = std::nullopt,
        .absolute = Opcodes::STY_ABS
    };
    return EncodeWithTable(STY_TABLE, operand, mode);
}

// Phase 2.2: Comparisons

// CMP - Compare Accumulator
std::vector<uint8_t> Cpu6502::EncodeCMP(uint16_t operand, AddressingMode mode) const {
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
        .indirect_y = Opcodes::CMP_INY
    };
    return EncodeWithTable(CMP_TABLE, operand, mode);
}

// CPX - Compare X Register
std::vector<uint8_t> Cpu6502::EncodeCPX(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable CPX_TABLE = {
        .immediate = Opcodes::CPX_IMM,
        .zero_page = Opcodes::CPX_ZP,
        .absolute = Opcodes::CPX_ABS
    };
    return EncodeWithTable(CPX_TABLE, operand, mode);
}

// CPY - Compare Y Register
std::vector<uint8_t> Cpu6502::EncodeCPY(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable CPY_TABLE = {
        .immediate = Opcodes::CPY_IMM,
        .zero_page = Opcodes::CPY_ZP,
        .absolute = Opcodes::CPY_ABS
    };
    return EncodeWithTable(CPY_TABLE, operand, mode);
}

// Phase 2.2: Branch Instructions

// BEQ - Branch if Equal
std::vector<uint8_t> Cpu6502::EncodeBEQ(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BEQ_TABLE = {
        .relative = Opcodes::BEQ
    };
    return EncodeWithTable(BEQ_TABLE, operand, mode);
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BNE_TABLE = {
        .relative = Opcodes::BNE
    };
    return EncodeWithTable(BNE_TABLE, operand, mode);
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BCC_TABLE = {
        .relative = Opcodes::BCC
    };
    return EncodeWithTable(BCC_TABLE, operand, mode);
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BCS_TABLE = {
        .relative = Opcodes::BCS
    };
    return EncodeWithTable(BCS_TABLE, operand, mode);
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BMI_TABLE = {
        .relative = Opcodes::BMI
    };
    return EncodeWithTable(BMI_TABLE, operand, mode);
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BPL_TABLE = {
        .relative = Opcodes::BPL
    };
    return EncodeWithTable(BPL_TABLE, operand, mode);
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BVC_TABLE = {
        .relative = Opcodes::BVC
    };
    return EncodeWithTable(BVC_TABLE, operand, mode);
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable BVS_TABLE = {
        .relative = Opcodes::BVS
    };
    return EncodeWithTable(BVS_TABLE, operand, mode);
}

// Phase 2.2: Inc/Dec Instructions

// INX - Increment X Register
std::vector<uint8_t> Cpu6502::EncodeINX() const {
    return {Opcodes::INX};
}

// INY - Increment Y Register
std::vector<uint8_t> Cpu6502::EncodeINY() const {
    return {Opcodes::INY};
}

// DEX - Decrement X Register
std::vector<uint8_t> Cpu6502::EncodeDEX() const {
    return {Opcodes::DEX};
}

// DEY - Decrement Y Register
std::vector<uint8_t> Cpu6502::EncodeDEY() const {
    return {Opcodes::DEY};
}

// INC - Increment Memory
std::vector<uint8_t> Cpu6502::EncodeINC(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable INC_TABLE = {
        .zero_page = Opcodes::INC_ZP,
        .zero_page_x = Opcodes::INC_ZPX,
        .absolute = Opcodes::INC_ABS,
        .absolute_x = Opcodes::INC_ABX
    };
    return EncodeWithTable(INC_TABLE, operand, mode);
}

// DEC - Decrement Memory
std::vector<uint8_t> Cpu6502::EncodeDEC(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable DEC_TABLE = {
        .zero_page = Opcodes::DEC_ZP,
        .zero_page_x = Opcodes::DEC_ZPX,
        .absolute = Opcodes::DEC_ABS,
        .absolute_x = Opcodes::DEC_ABX
    };
    return EncodeWithTable(DEC_TABLE, operand, mode);
}

// Phase 2.2: Stack Operations

// PHA - Push Accumulator
std::vector<uint8_t> Cpu6502::EncodePHA() const {
    return {Opcodes::PHA};
}

// PLA - Pull Accumulator
std::vector<uint8_t> Cpu6502::EncodePLA() const {
    return {Opcodes::PLA};
}

// PHP - Push Processor Status
std::vector<uint8_t> Cpu6502::EncodePHP() const {
    return {Opcodes::PHP};
}

// PLP - Pull Processor Status
std::vector<uint8_t> Cpu6502::EncodePLP() const {
    return {Opcodes::PLP};
}

// Phase 2.2: Subroutine

// JSR - Jump to Subroutine
std::vector<uint8_t> Cpu6502::EncodeJSR(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable JSR_TABLE = {
        .absolute = Opcodes::JSR
    };
    return EncodeWithTable(JSR_TABLE, operand, mode);
}

// ============================================================================
// Phase 2.3: Complete 6502 Instruction Set
// ============================================================================

// Group 1: BIT - Test Bits
// Note: Special case - some standard addressing modes only available in 65C02+
std::vector<uint8_t> Cpu6502::EncodeBIT(uint16_t operand, AddressingMode mode) const {
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
std::vector<uint8_t> Cpu6502::EncodeASL(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable ASL_TABLE = {
        .zero_page = Opcodes::ASL_ZP,
        .zero_page_x = Opcodes::ASL_ZPX,
        .absolute = Opcodes::ASL_ABS,
        .absolute_x = Opcodes::ASL_ABX,
        .accumulator = Opcodes::ASL_ACC
    };
    return EncodeWithTable(ASL_TABLE, operand, mode);
}

// LSR - Logical Shift Right
std::vector<uint8_t> Cpu6502::EncodeLSR(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable LSR_TABLE = {
        .zero_page = Opcodes::LSR_ZP,
        .zero_page_x = Opcodes::LSR_ZPX,
        .absolute = Opcodes::LSR_ABS,
        .absolute_x = Opcodes::LSR_ABX,
        .accumulator = Opcodes::LSR_ACC
    };
    return EncodeWithTable(LSR_TABLE, operand, mode);
}

// Group 3: Rotate Instructions

// ROL - Rotate Left
std::vector<uint8_t> Cpu6502::EncodeROL(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable ROL_TABLE = {
        .zero_page = Opcodes::ROL_ZP,
        .zero_page_x = Opcodes::ROL_ZPX,
        .absolute = Opcodes::ROL_ABS,
        .absolute_x = Opcodes::ROL_ABX,
        .accumulator = Opcodes::ROL_ACC
    };
    return EncodeWithTable(ROL_TABLE, operand, mode);
}

// ROR - Rotate Right
std::vector<uint8_t> Cpu6502::EncodeROR(uint16_t operand, AddressingMode mode) const {
    static const OpcodeTable ROR_TABLE = {
        .zero_page = Opcodes::ROR_ZP,
        .zero_page_x = Opcodes::ROR_ZPX,
        .absolute = Opcodes::ROR_ABS,
        .absolute_x = Opcodes::ROR_ABX,
        .accumulator = Opcodes::ROR_ACC
    };
    return EncodeWithTable(ROR_TABLE, operand, mode);
}

// Group 4: Interrupt Instructions

// RTI - Return from Interrupt
std::vector<uint8_t> Cpu6502::EncodeRTI() const {
    return {Opcodes::RTI};
}

// BRK - Break
std::vector<uint8_t> Cpu6502::EncodeBRK() const {
    return {Opcodes::BRK};
}

// Group 5: Flag Operations

// CLC - Clear Carry
std::vector<uint8_t> Cpu6502::EncodeCLC() const {
    return {Opcodes::CLC};
}

// SEC - Set Carry
std::vector<uint8_t> Cpu6502::EncodeSEC() const {
    return {Opcodes::SEC};
}

// CLD - Clear Decimal
std::vector<uint8_t> Cpu6502::EncodeCLD() const {
    return {Opcodes::CLD};
}

// SED - Set Decimal
std::vector<uint8_t> Cpu6502::EncodeSED() const {
    return {Opcodes::SED};
}

// CLI - Clear Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeCLI() const {
    return {Opcodes::CLI};
}

// SEI - Set Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeSEI() const {
    return {Opcodes::SEI};
}

// CLV - Clear Overflow
std::vector<uint8_t> Cpu6502::EncodeCLV() const {
    return {Opcodes::CLV};
}

// Group 6: Transfer Instructions

// TSX - Transfer SP to X
std::vector<uint8_t> Cpu6502::EncodeTSX() const {
    return {Opcodes::TSX};
}

// TXS - Transfer X to SP
std::vector<uint8_t> Cpu6502::EncodeTXS() const {
    return {Opcodes::TXS};
}

// TAX - Transfer A to X
std::vector<uint8_t> Cpu6502::EncodeTAX() const {
    return {Opcodes::TAX};
}

// TAY - Transfer A to Y
std::vector<uint8_t> Cpu6502::EncodeTAY() const {
    return {Opcodes::TAY};
}

// TXA - Transfer X to A
std::vector<uint8_t> Cpu6502::EncodeTXA() const {
    return {Opcodes::TXA};
}

// TYA - Transfer Y to A
std::vector<uint8_t> Cpu6502::EncodeTYA() const {
    return {Opcodes::TYA};
}


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
        case AddressingMode::IndirectZeroPage:              // Phase 2.5 - Group 6: 65C02
        case AddressingMode::IndirectLong:                  // Phase 2.5 - Group 9: 65816
        case AddressingMode::IndirectLongIndexedY:          // Phase 2.5 - Group 9: 65816
        case AddressingMode::StackRelative:                 // Phase 2.5 - Group 10: 65816
        case AddressingMode::StackRelativeIndirectIndexedY: // Phase 2.5 - Group 10: 65816
            return 2;

        case AddressingMode::Absolute:
        case AddressingMode::AbsoluteX:
        case AddressingMode::AbsoluteY:
        case AddressingMode::Indirect:
        case AddressingMode::AbsoluteIndexedIndirect:  // Phase 2.5 - Group 6: 65C02
            return 3;

        case AddressingMode::AbsoluteLong:  // Phase 2.5 - Group 9: 65816 (24-bit address)
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
 * 6502 branch instructions use 8-bit signed relative offsets (-128 to +127 bytes).
 * If target is outside this range, branch must be "relaxed" into B!cc + JMP sequence.
 * 
 * @param current_addr Address of the branch instruction
 * @param target_addr Target address to branch to
 * @return true if branch needs relaxation (out of range), false otherwise
 */
bool Cpu6502::NeedsBranchRelaxation(uint16_t current_addr, uint16_t target_addr) const {
    return branch_handler_.NeedsBranchRelaxation(current_addr, target_addr);
}

uint8_t Cpu6502::GetComplementaryBranchOpcode(uint8_t branch_opcode) const {
    return branch_handler_.GetComplementaryBranchOpcode(branch_opcode);
}

std::vector<uint8_t> Cpu6502::EncodeBranchWithRelaxation(uint8_t branch_opcode,
                                                           uint16_t current_addr,
                                                           uint16_t target_addr) const {
    return branch_handler_.EncodeBranchWithRelaxation(branch_opcode, current_addr, target_addr);
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
        return {};  // Not available in this CPU mode
    }
    
    return {0xCB};  // WAI opcode
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
        return {};  // Not available in this CPU mode
    }
    
    return {0xDB};  // STP opcode
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

std::vector<uint8_t> Cpu6502::EncodeRMB0(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x07, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB1(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x17, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB2(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x27, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB3(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x37, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB4(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x47, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB5(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x57, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB6(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x67, operand};
}

std::vector<uint8_t> Cpu6502::EncodeRMB7(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x77, operand};
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

std::vector<uint8_t> Cpu6502::EncodeSMB0(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x87, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB1(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0x97, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB2(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xA7, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB3(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xB7, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB4(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xC7, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB5(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xD7, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB6(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xE7, operand};
}

std::vector<uint8_t> Cpu6502::EncodeSMB7(uint8_t operand, AddressingMode mode) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock || mode != AddressingMode::ZeroPage) {
        return {};
    }
    return {0xF7, operand};
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

std::vector<uint8_t> Cpu6502::EncodeBBR0(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x0F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR1(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x1F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR2(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x2F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR3(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x3F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR4(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x4F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR5(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x5F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR6(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x6F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBR7(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x7F, zp_addr, offset};
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

std::vector<uint8_t> Cpu6502::EncodeBBS0(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x8F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS1(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0x9F, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS2(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xAF, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS3(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xBF, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS4(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xCF, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS5(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xDF, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS6(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xEF, zp_addr, offset};
}

std::vector<uint8_t> Cpu6502::EncodeBBS7(uint8_t zp_addr, uint8_t offset) const {
    if (cpu_mode_ != CpuMode::Cpu65C02Rock) {
        return {};
    }
    return {0xFF, zp_addr, offset};
}

} // namespace xasm
