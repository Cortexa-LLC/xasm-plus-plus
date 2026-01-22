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
    Accumulator,    // ASL A, LSR A, ROL A, ROR A
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

    // Phase 2.2: Additional loads/stores
    std::vector<uint8_t> EncodeLDX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeLDY(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSTX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSTY(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Comparisons
    std::vector<uint8_t> EncodeCMP(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeCPX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeCPY(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Branch instructions
    std::vector<uint8_t> EncodeBEQ(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBNE(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBCC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBCS(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBMI(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBPL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBVC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBVS(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Inc/Dec instructions
    std::vector<uint8_t> EncodeINX() const;
    std::vector<uint8_t> EncodeINY() const;
    std::vector<uint8_t> EncodeDEX() const;
    std::vector<uint8_t> EncodeDEY() const;
    std::vector<uint8_t> EncodeINC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeDEC(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Stack operations
    std::vector<uint8_t> EncodePHA() const;
    std::vector<uint8_t> EncodePLA() const;
    std::vector<uint8_t> EncodePHP() const;
    std::vector<uint8_t> EncodePLP() const;

    // Phase 2.2: Subroutine
    std::vector<uint8_t> EncodeJSR(uint16_t operand, AddressingMode mode) const;

    // Phase 2.3: Complete 6502 Instruction Set
    // Group 1: BIT - Test Bits
    std::vector<uint8_t> EncodeBIT(uint16_t operand, AddressingMode mode) const;

    // Group 2: Shift Instructions
    std::vector<uint8_t> EncodeASL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeLSR(uint16_t operand, AddressingMode mode) const;

    // Calculate instruction size
    size_t CalculateInstructionSize(AddressingMode mode) const;
};

} // namespace xasm
