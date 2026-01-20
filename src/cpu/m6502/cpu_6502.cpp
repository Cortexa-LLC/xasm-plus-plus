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
