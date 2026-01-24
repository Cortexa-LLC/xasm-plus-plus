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

// CPU modes for 6502 family (Phase 2.5)
enum class CpuMode {
    Cpu6502,        // Original 6502 (default)
    Cpu65C02,       // 65C02 with enhanced opcodes
    Cpu65C02Rock,   // 65C02 with Rockwell extensions
    Cpu65816        // 65816 with 16-bit support
};

// 6502 CPU Plugin
class Cpu6502 {
public:
    Cpu6502() = default;

    // Metadata
    std::string GetName() const { return "6502"; }

    // CPU mode switching (Phase 2.5)
    void SetCpuMode(CpuMode mode);
    CpuMode GetCpuMode() const;

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

    // Phase 2.5 - Group 2: 65C02 Stack operations
    std::vector<uint8_t> EncodePHX() const;  // Push X (65C02+)
    std::vector<uint8_t> EncodePLX() const;  // Pull X (65C02+)
    std::vector<uint8_t> EncodePHY() const;  // Push Y (65C02+)
    std::vector<uint8_t> EncodePLY() const;  // Pull Y (65C02+)

    // Phase 2.5 - Group 3: 65C02 Store Zero
    std::vector<uint8_t> EncodeSTZ(uint16_t operand, AddressingMode mode) const;  // Store Zero (65C02+)

    // Phase 2.5 - Group 4: 65C02 Bit Test
    std::vector<uint8_t> EncodeTRB(uint16_t operand, AddressingMode mode) const;  // Test and Reset Bits (65C02+)
    std::vector<uint8_t> EncodeTSB(uint16_t operand, AddressingMode mode) const;  // Test and Set Bits (65C02+)

    // Phase 2.5 - Group 5: 65C02 Branch Always
    std::vector<uint8_t> EncodeBRA(uint16_t operand, AddressingMode mode) const;  // Branch Always (65C02+)

    // Phase 2.2: Subroutine
    std::vector<uint8_t> EncodeJSR(uint16_t operand, AddressingMode mode) const;

    // Phase 2.3: Complete 6502 Instruction Set
    // Group 1: BIT - Test Bits
    std::vector<uint8_t> EncodeBIT(uint16_t operand, AddressingMode mode) const;

    // Group 2: Shift Instructions
    std::vector<uint8_t> EncodeASL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeLSR(uint16_t operand, AddressingMode mode) const;

    // Group 3: Rotate Instructions
    std::vector<uint8_t> EncodeROL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeROR(uint16_t operand, AddressingMode mode) const;

    // Group 4: Interrupt Instructions
    std::vector<uint8_t> EncodeRTI() const;
    std::vector<uint8_t> EncodeBRK() const;

    // Group 5: Flag Operations
    std::vector<uint8_t> EncodeCLC() const;
    std::vector<uint8_t> EncodeSEC() const;
    std::vector<uint8_t> EncodeCLD() const;
    std::vector<uint8_t> EncodeSED() const;
    std::vector<uint8_t> EncodeCLI() const;
    std::vector<uint8_t> EncodeSEI() const;
    std::vector<uint8_t> EncodeCLV() const;

    // Group 6: Transfer Instructions
    std::vector<uint8_t> EncodeTSX() const;
    std::vector<uint8_t> EncodeTXS() const;
    std::vector<uint8_t> EncodeTAX() const;
    std::vector<uint8_t> EncodeTAY() const;
    std::vector<uint8_t> EncodeTXA() const;
    std::vector<uint8_t> EncodeTYA() const;

    // Calculate instruction size
    size_t CalculateInstructionSize(AddressingMode mode) const;

private:
    // Phase 2.5: CPU mode state
    CpuMode cpu_mode_ = CpuMode::Cpu6502;  // Default to 6502
};

} // namespace xasm
