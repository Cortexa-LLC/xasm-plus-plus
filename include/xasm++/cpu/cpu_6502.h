// 6502 CPU Plugin
// Phase 1: Minimal Viable Assembler - CPU Plugin

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace xasm {

// Addressing modes for 6502
enum class AddressingMode {
    Implied,        // RTS, NOP
    Immediate,      // LDA #$42
    ZeroPage,       // LDA $80
    ZeroPageX,      // LDA $80,X
    ZeroPageY,      // LDX $80,Y
    Absolute,       // LDA $1234
    AbsoluteX,      // LDA $1234,X
    AbsoluteY,      // LDA $1234,Y
    Indirect,       // JMP ($1234)
    IndirectX,      // LDA ($80,X)
    IndirectY,      // LDA ($80),Y
    Relative,       // BEQ label
};

// 6502 CPU Plugin
class Cpu6502 {
public:
    Cpu6502() = default;

    // Metadata
    std::string GetName() const { return "6502"; }

    // Instruction encoding methods (Phase 1 subset)
    std::vector<uint8_t> EncodeLDA(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSTA(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeJMP(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeNOP() const;
    std::vector<uint8_t> EncodeRTS() const;

    // Phase 2.2: Arithmetic instructions
    std::vector<uint8_t> EncodeADC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSBC(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Logic instructions
    std::vector<uint8_t> EncodeAND(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeORA(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeEOR(uint16_t operand, AddressingMode mode) const;

    // Calculate instruction size
    size_t CalculateInstructionSize(AddressingMode mode) const;
};

} // namespace xasm
