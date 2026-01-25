// 6502 CPU implementation - Core 6502 opcodes

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// LDA - Load Accumulator
// Note: Changed to uint32_t for 65816 24-bit addressing support
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0xA9);  // LDA #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA5);  // LDA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xB5);  // LDA zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAD);  // LDA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xBD);  // LDA abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0xB9);  // LDA abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0xA1);  // LDA (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0xB1);  // LDA (zp),Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Modes
        case AddressingMode::IndirectZeroPage:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(0xB2);  // LDA (zp) - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        // Phase 2.5 - Group 9: 65816 Long Addressing Modes
        case AddressingMode::AbsoluteLong:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(0xAF);  // LDA long
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));         // Low byte
                bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));  // Middle byte
                bytes.push_back(static_cast<uint8_t>((operand >> 16) & 0xFF)); // High byte (bank)
            }
            break;

        case AddressingMode::IndirectLong:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(0xA7);  // LDA [dp]
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::IndirectLongIndexedY:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(0xB7);  // LDA [dp],Y
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        // Phase 2.5 - Group 10: 65816 Stack Relative Addressing
        case AddressingMode::StackRelative:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(0xA3);  // LDA sr,S
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::StackRelativeIndirectIndexedY:
            // Only available in 65816
            if (cpu_mode_ == CpuMode::Cpu65816) {
                bytes.push_back(0xB3);  // LDA (sr,S),Y
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
            bytes.push_back(0x85);  // STA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x95);  // STA zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8D);  // STA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x9D);  // STA abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0x99);  // STA abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0x81);  // STA (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0x91);  // STA (zp),Y
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
        bytes.push_back(0x4C);  // JMP abs
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    } else if (mode == AddressingMode::Indirect) {
        bytes.push_back(0x6C);  // JMP (ind)
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
        bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
    } else if (mode == AddressingMode::AbsoluteIndexedIndirect) {
        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Mode
        // Only available in 65C02 and later
        if (cpu_mode_ != CpuMode::Cpu6502) {
            bytes.push_back(0x7C);  // JMP (abs,X) - 65C02+
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
        }
    }

    return bytes;
}

// NOP - No Operation
std::vector<uint8_t> Cpu6502::EncodeNOP() const {
    return {0xEA};
}

// RTS - Return from Subroutine
std::vector<uint8_t> Cpu6502::EncodeRTS() const {
    return {0x60};
}

// Phase 2.2: Arithmetic Instructions

// ADC - Add with Carry
std::vector<uint8_t> Cpu6502::EncodeADC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::Immediate:
            bytes.push_back(0x69);  // ADC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x65);  // ADC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x75);  // ADC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x6D);  // ADC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x7D);  // ADC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0x79);  // ADC abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0x61);  // ADC (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0x71);  // ADC (zp),Y
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
            bytes.push_back(0xE9);  // SBC #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xE5);  // SBC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xF5);  // SBC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xED);  // SBC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xFD);  // SBC abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0xF9);  // SBC abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0xE1);  // SBC (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0xF1);  // SBC (zp),Y
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
            bytes.push_back(0x29);  // AND #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x25);  // AND zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x35);  // AND zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x2D);  // AND abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x3D);  // AND abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0x39);  // AND abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0x21);  // AND (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0x31);  // AND (zp),Y
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
            bytes.push_back(0x09);  // ORA #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x05);  // ORA zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x15);  // ORA zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x0D);  // ORA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x1D);  // ORA abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0x19);  // ORA abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0x01);  // ORA (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0x11);  // ORA (zp),Y
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
            bytes.push_back(0x49);  // EOR #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x45);  // EOR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x55);  // EOR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x4D);  // EOR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x5D);  // EOR abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0x59);  // EOR abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0x41);  // EOR (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0x51);  // EOR (zp),Y
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
            bytes.push_back(0xA2);  // LDX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA6);  // LDX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageY:
            bytes.push_back(0xB6);  // LDX zp,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAE);  // LDX abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0xBE);  // LDX abs,Y
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
            bytes.push_back(0xA0);  // LDY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xA4);  // LDY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xB4);  // LDY zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xAC);  // LDY abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xBC);  // LDY abs,X
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
            bytes.push_back(0x86);  // STX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageY:
            bytes.push_back(0x96);  // STX zp,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8E);  // STX abs
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
            bytes.push_back(0x84);  // STY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x94);  // STY zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x8C);  // STY abs
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
            bytes.push_back(0xC9);  // CMP #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xC5);  // CMP zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xD5);  // CMP zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCD);  // CMP abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xDD);  // CMP abs,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteY:
            bytes.push_back(0xD9);  // CMP abs,Y
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::IndirectX:
            bytes.push_back(0xC1);  // CMP (zp,X)
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::IndirectY:
            bytes.push_back(0xD1);  // CMP (zp),Y
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
            bytes.push_back(0xE0);  // CPX #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xE4);  // CPX zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xEC);  // CPX abs
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
            bytes.push_back(0xC0);  // CPY #imm
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0xC4);  // CPY zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCC);  // CPY abs
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
        bytes.push_back(0xF0);  // BEQ opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BNE - Branch if Not Equal
std::vector<uint8_t> Cpu6502::EncodeBNE(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0xD0);  // BNE opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCC - Branch if Carry Clear
std::vector<uint8_t> Cpu6502::EncodeBCC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x90);  // BCC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BCS - Branch if Carry Set
std::vector<uint8_t> Cpu6502::EncodeBCS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0xB0);  // BCS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BMI - Branch if Minus
std::vector<uint8_t> Cpu6502::EncodeBMI(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x30);  // BMI opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BPL - Branch if Plus
std::vector<uint8_t> Cpu6502::EncodeBPL(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x10);  // BPL opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVC - Branch if Overflow Clear
std::vector<uint8_t> Cpu6502::EncodeBVC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x50);  // BVC opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// BVS - Branch if Overflow Set
std::vector<uint8_t> Cpu6502::EncodeBVS(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;
    if (mode == AddressingMode::Relative) {
        bytes.push_back(0x70);  // BVS opcode
        bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
    }
    return bytes;
}

// Phase 2.2: Inc/Dec Instructions

// INX - Increment X Register
std::vector<uint8_t> Cpu6502::EncodeINX() const {
    return {0xE8};
}

// INY - Increment Y Register
std::vector<uint8_t> Cpu6502::EncodeINY() const {
    return {0xC8};
}

// DEX - Decrement X Register
std::vector<uint8_t> Cpu6502::EncodeDEX() const {
    return {0xCA};
}

// DEY - Decrement Y Register
std::vector<uint8_t> Cpu6502::EncodeDEY() const {
    return {0x88};
}

// INC - Increment Memory
std::vector<uint8_t> Cpu6502::EncodeINC(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    switch (mode) {
        case AddressingMode::ZeroPage:
            bytes.push_back(0xE6);  // INC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xF6);  // INC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xEE);  // INC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xFE);  // INC abs,X
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
            bytes.push_back(0xC6);  // DEC zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0xD6);  // DEC zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0xCE);  // DEC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0xDE);  // DEC abs,X
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
    return {0x48};
}

// PLA - Pull Accumulator
std::vector<uint8_t> Cpu6502::EncodePLA() const {
    return {0x68};
}

// PHP - Push Processor Status
std::vector<uint8_t> Cpu6502::EncodePHP() const {
    return {0x08};
}

// PLP - Pull Processor Status
std::vector<uint8_t> Cpu6502::EncodePLP() const {
    return {0x28};
}

// Phase 2.2: Subroutine

// JSR - Jump to Subroutine
std::vector<uint8_t> Cpu6502::EncodeJSR(uint16_t operand, AddressingMode mode) const {
    std::vector<uint8_t> bytes;

    if (mode == AddressingMode::Absolute) {
        bytes.push_back(0x20);  // JSR abs
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
            bytes.push_back(0x24);  // BIT zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x2C);  // BIT abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        // Phase 2.5 - Group 6: 65C02 Enhanced Addressing Modes for BIT
        case AddressingMode::Immediate:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(0x89);  // BIT #imm - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::ZeroPageX:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(0x34);  // BIT zp,X - 65C02+
                bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            }
            break;

        case AddressingMode::AbsoluteX:
            // Only available in 65C02 and later
            if (cpu_mode_ != CpuMode::Cpu6502) {
                bytes.push_back(0x3C);  // BIT abs,X - 65C02+
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
            bytes.push_back(0x0A);  // ASL A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x06);  // ASL zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x16);  // ASL zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x0E);  // ASL abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x1E);  // ASL abs,X
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
            bytes.push_back(0x4A);  // LSR A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x46);  // LSR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x56);  // LSR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x4E);  // LSR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x5E);  // LSR abs,X
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
            bytes.push_back(0x2A);  // ROL A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x26);  // ROL zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x36);  // ROL zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x2E);  // ROL abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x3E);  // ROL abs,X
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
            bytes.push_back(0x6A);  // ROR A
            break;

        case AddressingMode::ZeroPage:
            bytes.push_back(0x66);  // ROR zp
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::ZeroPageX:
            bytes.push_back(0x76);  // ROR zp,X
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            break;

        case AddressingMode::Absolute:
            bytes.push_back(0x6E);  // ROR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        case AddressingMode::AbsoluteX:
            bytes.push_back(0x7E);  // ROR abs,X
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
    return {0x40};
}

// BRK - Break
std::vector<uint8_t> Cpu6502::EncodeBRK() const {
    return {0x00};
}

// Group 5: Flag Operations

// CLC - Clear Carry
std::vector<uint8_t> Cpu6502::EncodeCLC() const {
    return {0x18};
}

// SEC - Set Carry
std::vector<uint8_t> Cpu6502::EncodeSEC() const {
    return {0x38};
}

// CLD - Clear Decimal
std::vector<uint8_t> Cpu6502::EncodeCLD() const {
    return {0xD8};
}

// SED - Set Decimal
std::vector<uint8_t> Cpu6502::EncodeSED() const {
    return {0xF8};
}

// CLI - Clear Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeCLI() const {
    return {0x58};
}

// SEI - Set Interrupt Disable
std::vector<uint8_t> Cpu6502::EncodeSEI() const {
    return {0x78};
}

// CLV - Clear Overflow
std::vector<uint8_t> Cpu6502::EncodeCLV() const {
    return {0xB8};
}

// Group 6: Transfer Instructions

// TSX - Transfer SP to X
std::vector<uint8_t> Cpu6502::EncodeTSX() const {
    return {0xBA};
}

// TXS - Transfer X to SP
std::vector<uint8_t> Cpu6502::EncodeTXS() const {
    return {0x9A};
}

// TAX - Transfer A to X
std::vector<uint8_t> Cpu6502::EncodeTAX() const {
    return {0xAA};
}

// TAY - Transfer A to Y
std::vector<uint8_t> Cpu6502::EncodeTAY() const {
    return {0xA8};
}

// TXA - Transfer X to A
std::vector<uint8_t> Cpu6502::EncodeTXA() const {
    return {0x8A};
}

// TYA - Transfer Y to A
std::vector<uint8_t> Cpu6502::EncodeTYA() const {
    return {0x98};
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

} // namespace xasm
