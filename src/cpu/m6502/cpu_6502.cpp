// 6502 CPU implementation

#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// LDA - Load Accumulator
std::vector<uint8_t> Cpu6502::EncodeLDA(uint16_t operand, AddressingMode mode) const {
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

        case AddressingMode::Absolute:
            bytes.push_back(0xAD);  // LDA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            // For Phase 1, only support these modes
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

        case AddressingMode::Absolute:
            bytes.push_back(0x8D);  // STA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF));
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

        case AddressingMode::Absolute:
            bytes.push_back(0x6D);  // ADC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
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

        case AddressingMode::Absolute:
            bytes.push_back(0xED);  // SBC abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
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

        case AddressingMode::Absolute:
            bytes.push_back(0x2D);  // AND abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
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

        case AddressingMode::Absolute:
            bytes.push_back(0x0D);  // ORA abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
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

        case AddressingMode::Absolute:
            bytes.push_back(0x4D);  // EOR abs
            bytes.push_back(static_cast<uint8_t>(operand & 0xFF));        // Low byte
            bytes.push_back(static_cast<uint8_t>((operand >> 8) & 0xFF)); // High byte
            break;

        default:
            break;
    }

    return bytes;
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
            return 2;

        case AddressingMode::Absolute:
        case AddressingMode::AbsoluteX:
        case AddressingMode::AbsoluteY:
        case AddressingMode::Indirect:
            return 3;

        default:
            return 0;
    }
}

} // namespace xasm
