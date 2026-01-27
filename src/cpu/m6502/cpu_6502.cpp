// 6502 CPU implementation - Core 6502 opcodes

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/opcodes_6502.h"

namespace xasm {

// LDA - Load Accumulator
// Note: Changed to uint32_t for 65816 24-bit addressing support
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::LDA_IMM);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::LDA_ZP);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::LDA_ZPX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::LDA_ABS);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::LDA_ABX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::LDA_ABY);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::LDA_INX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::LDA_INY);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Modes
        case AddressingMode::IndirectZeroPage:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(Opcodes::LDA_IZP);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        // Phase 2.5 - Group 9: 65816 Long Addressing Modes
        case AddressingMode::AbsoluteLong:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(Opcodes::LDA_ALG);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));         // Low byte
                bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));  // Middle byte
                bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF)); // High byte (bank)
            }
            break;

        case AddressingMode::IndirectLong:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(Opcodes::LDA_ILG);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::IndirectLongIndexedY:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(Opcodes::LDA_ILY);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        // Phase 2.5 - Group 10: 65816 Stack Relative Addressing
        case AddressingMode::StackRelative:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(Opcodes::LDA_SR);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::StackRelativeIndirectIndexedY:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(Opcodes::LDA_SRY);
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        default:
            break;
    }

    return bytes;
}

// STA - Store Accumulator
std::vector<uint8_t> Cpu6502::EncodeSTA(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::STA_ZP);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::STA_ZPX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::STA_ABS);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::STA_ABX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::STA_ABY);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::STA_INX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::STA_INY);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// JMP - Jump
std::vector<uint8_t> Cpu6502::EncodeJMP(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Absolute) {
        bytes.push_back(Opcodes::JMP_ABS);
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    } else if (mode == AddressingMode::Indirect) {
        bytes.push_back(Opcodes::JMP_IND);
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    } else if (mode == AddressingMode::AbsoluteIndexedIndirect) {
        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Mode
        // Only available in 65C02 and later
        if (cpu_mode_ != CpuMode::Cpu6502) {
            bytes.push_back(Opcodes::JMP_AIX);
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
        }
    }

    return bytes;
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
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::ADC_IMM);  // ADC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::ADC_ZP);  // ADC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::ADC_ZPX);  // ADC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::ADC_ABS);  // ADC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::ADC_ABX);  // ADC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::ADC_ABY);  // ADC abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::ADC_INX);  // ADC (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::ADC_INY);  // ADC (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// SBC - Subtract with Carry
std::vector<uint8_t> Cpu6502::EncodeSBC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::SBC_IMM);  // SBC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::SBC_ZP);  // SBC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::SBC_ZPX);  // SBC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::SBC_ABS);  // SBC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::SBC_ABX);  // SBC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::SBC_ABY);  // SBC abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::SBC_INX);  // SBC (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::SBC_INY);  // SBC (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Logic Instructions

// AND - Logical AND
std::vector<uint8_t> Cpu6502::EncodeAND(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::AND_IMM);  // AND #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::AND_ZP);  // AND zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::AND_ZPX);  // AND zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::AND_ABS);  // AND abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::AND_ABX);  // AND abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::AND_ABY);  // AND abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::AND_INX);  // AND (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::AND_INY);  // AND (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// ORA - Logical OR
std::vector<uint8_t> Cpu6502::EncodeORA(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::ORA_IMM);  // ORA #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::ORA_ZP);  // ORA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::ORA_ZPX);  // ORA zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::ORA_ABS);  // ORA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::ORA_ABX);  // ORA abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::ORA_ABY);  // ORA abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::ORA_INX);  // ORA (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::ORA_INY);  // ORA (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// EOR - Exclusive OR
std::vector<uint8_t> Cpu6502::EncodeEOR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::EOR_IMM);  // EOR #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::EOR_ZP);  // EOR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::EOR_ZPX);  // EOR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::EOR_ABS);  // EOR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::EOR_ABX);  // EOR abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::EOR_ABY);  // EOR abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::EOR_INX);  // EOR (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::EOR_INY);  // EOR (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Additional Loads/Stores

// LDX - Load X Register
std::vector<uint8_t> Cpu6502::EncodeLDX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::LDX_IMM);  // LDX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::LDX_ZP);  // LDX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageY:
            bytes.push_back(Opcodes::LDX_ZPY);  // LDX zp,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::LDX_ABS);  // LDX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::LDX_ABY);  // LDX abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// LDY - Load Y Register
std::vector<uint8_t> Cpu6502::EncodeLDY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::LDY_IMM);  // LDY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::LDY_ZP);  // LDY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::LDY_ZPX);  // LDY zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::LDY_ABS);  // LDY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::LDY_ABX);  // LDY abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// STX - Store X Register
std::vector<uint8_t> Cpu6502::EncodeSTX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::STX_ZP);  // STX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageY:
            bytes.push_back(Opcodes::STX_ZPY);  // STX zp,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::STX_ABS);  // STX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// STY - Store Y Register
std::vector<uint8_t> Cpu6502::EncodeSTY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::STY_ZP);  // STY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::STY_ZPX);  // STY zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::STY_ABS);  // STY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Comparisons

// CMP - Compare Accumulator
std::vector<uint8_t> Cpu6502::EncodeCMP(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::CMP_IMM);  // CMP #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::CMP_ZP);  // CMP zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::CMP_ZPX);  // CMP zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::CMP_ABS);  // CMP abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::CMP_ABX);  // CMP abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(Opcodes::CMP_ABY);  // CMP abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(Opcodes::CMP_INX);  // CMP (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(Opcodes::CMP_INY);  // CMP (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        default:
            break;
    }

    return bytes;
}

// CPX - Compare X Register
std::vector<uint8_t> Cpu6502::EncodeCPX(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::CPX_IMM);  // CPX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::CPX_ZP);  // CPX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::CPX_ABS);  // CPX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// CPY - Compare Y Register
std::vector<uint8_t> Cpu6502::EncodeCPY(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(Opcodes::CPY_IMM);  // CPY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::CPY_ZP);  // CPY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::CPY_ABS);  // CPY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Phase 2.2: Branch Instructions

// BEQ - Branch if Equal
std::vector<uint8_t> Cpu6502::EncodeBEQ(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BEQ);  // BEQ opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BNE);  // BNE opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BCC);  // BCC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BCS);  // BCS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BMI);  // BMI opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BPL);  // BPL opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BVC);  // BVC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(Opcodes::BVS);  // BVS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
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
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::INC_ZP);  // INC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::INC_ZPX);  // INC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::INC_ABS);  // INC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::INC_ABX);  // INC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// DEC - Decrement Memory
std::vector<uint8_t> Cpu6502::EncodeDEC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::DEC_ZP);  // DEC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::DEC_ZPX);  // DEC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::DEC_ABS);  // DEC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::DEC_ABX);  // DEC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
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
    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Absolute) {
        bytes.push_back(Opcodes::JSR);  // JSR abs
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
    }

    return bytes;
}

// ============================================================================
// Phase 2.3: Complete 6502 Instruction Set
// ============================================================================

// Group 1: BIT - Test Bits
std::vector<uint8_t> Cpu6502::EncodeBIT(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::BIT_ZP);  // BIT zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::BIT_ABS);  // BIT abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Modes for BIT
        case AddressingMode::Immediate:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(Opcodes::BIT_IMM);  // BIT #imm - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::ZeroPageX:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(Opcodes::BIT_ZPX);  // BIT zp,X - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::AbsoluteX:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(Opcodes::BIT_ABX);  // BIT abs,X - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
                bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
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
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Accumulator:
            bytes.push_back(Opcodes::ASL_ACC);  // ASL A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::ASL_ZP);  // ASL zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::ASL_ZPX);  // ASL zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::ASL_ABS);  // ASL abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::ASL_ABX);  // ASL abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// LSR - Logical Shift Right
std::vector<uint8_t> Cpu6502::EncodeLSR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Accumulator:
            bytes.push_back(Opcodes::LSR_ACC);  // LSR A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::LSR_ZP);  // LSR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::LSR_ZPX);  // LSR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::LSR_ABS);  // LSR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::LSR_ABX);  // LSR abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// Group 3: Rotate Instructions

// ROL - Rotate Left
std::vector<uint8_t> Cpu6502::EncodeROL(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Accumulator:
            bytes.push_back(Opcodes::ROL_ACC);  // ROL A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::ROL_ZP);  // ROL zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::ROL_ZPX);  // ROL zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::ROL_ABS);  // ROL abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::ROL_ABX);  // ROL abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
}

// ROR - Rotate Right
std::vector<uint8_t> Cpu6502::EncodeROR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Accumulator:
            bytes.push_back(Opcodes::ROR_ACC);  // ROR A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(Opcodes::ROR_ZP);  // ROR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(Opcodes::ROR_ZPX);  // ROR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(Opcodes::ROR_ABS);  // ROR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(Opcodes::ROR_ABX);  // ROR abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
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
    // Calculate offset: target - (PC + 2)
    // PC + 2 because branch instruction is 2 bytes (opcode + offset)
    int16_t offset = static_cast<int16_t>(target_addr) - static_cast<int16_t>(current_addr + 2);
    
    // Check if offset fits in 8-bit signed range (-128 to +127)
    return (offset < -128 || offset > 127);
}

/**
 * @brief Get complementary (inverted) branch opcode
 * 
 * All 6502 branch opcodes can be inverted by XORing with Opcodes::JSR.
 * This is used for branch relaxation: BEQ far → BNE *+5; JMP far
 * 
 * Examples:
 *   BEQ (Opcodes::BEQ) → BNE (Opcodes::BNE)  [Opcodes::BEQ XOR Opcodes::JSR = Opcodes::BNE]
 *   BCC (Opcodes::BCC) → BCS (Opcodes::BCS)  [Opcodes::BCC XOR Opcodes::JSR = Opcodes::BCS]
 * 
 * @param branch_opcode Original branch opcode
 * @return Complementary branch opcode
 */
uint8_t Cpu6502::GetComplementaryBranchOpcode(uint8_t branch_opcode) const {
    return branch_opcode ^ Opcodes::JSR;
}

/**
 * @brief Encode branch instruction with automatic relaxation if needed
 * 
 * If branch target is in range (-128 to +127 bytes):
 *   Emits normal 2-byte branch: [opcode] [offset]
 * 
 * If branch target is out of range:
 *   Emits relaxed 5-byte sequence: [B!cc] [Opcodes::BRANCH_RELAXATION_OFFSET] [JMP] [target_lo] [target_hi]
 * 
 * Example: BEQ $1200 from $1000 (offset = +510, out of range)
 *   Normal:  F0 7E (fails - offset too large)
 *   Relaxed: D0 03 4C 00 12 (BNE *+5; JMP $1200)
 * 
 * @param branch_opcode Branch opcode (BEQ, BNE, BCC, BCS, etc.)
 * @param current_addr Address where branch instruction will be located
 * @param target_addr Target address to branch to
 * @return Encoded bytes (2 bytes if in range, 5 bytes if relaxed)
 */
std::vector<uint8_t> Cpu6502::EncodeBranchWithRelaxation(uint8_t branch_opcode,
                                                           uint16_t current_addr,
                                                           uint16_t target_addr) const {
    std::vector<uint8_t> bytes;
    
    if (!NeedsBranchRelaxation(current_addr, target_addr)) {
        // Branch is in range - emit normal 2-byte branch
        int16_t offset = static_cast<int16_t>(target_addr) - static_cast<int16_t>(current_addr + 2);
        bytes.push_back(branch_opcode);
        bytes.push_back(static_cast<uint8_t>(offset & 0xFF));
    } else {
        // Branch is out of range - emit relaxed 5-byte sequence
        // Format: [B!cc] [Opcodes::BRANCH_RELAXATION_OFFSET] [JMP] [target_lo] [target_hi]
        
        // 1. Emit complementary branch (inverted condition)
        uint8_t complement = GetComplementaryBranchOpcode(branch_opcode);
        bytes.push_back(complement);
        
        // 2. Emit offset of +3 (skip over the 3-byte JMP instruction)
        bytes.push_back(Opcodes::BRANCH_RELAXATION_OFFSET);
        
        // 3. Emit JMP absolute to target
        bytes.push_back(Opcodes::JMP_ABS);  // JMP opcode
        bytes.push_back(static_cast<uint8_t>(target_addr & 0xFF));        // Low byte
        bytes.push_back(static_cast<uint8_t>((target_addr >> 8) & 0xFF)); // High byte
    }
    
    return bytes;
}

} // namespace xasm
