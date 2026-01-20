// 6502 CPU Plugin tests
// Phase 1: Minimal Viable Assembler - 6502 CPU Plugin

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: CPU creation
TEST(Cpu6502Test, Creation) {
    Cpu6502 cpu;

    EXPECT_EQ(cpu.GetName(), "6502");
}

// Test 2: LDA Immediate encoding (#$42)
TEST(Cpu6502Test, LDA_Immediate) {
    Cpu6502 cpu;
    ConcreteSymbolTable symbols;

    // LDA #$42 -> A9 42
    auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA9);  // LDA immediate opcode
    EXPECT_EQ(bytes[1], 0x42);  // Operand
}

// Test 3: LDA Zero Page encoding ($80)
TEST(Cpu6502Test, LDA_ZeroPage) {
    Cpu6502 cpu;

    // LDA $80 -> A5 80
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode::ZeroPage);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xA5);  // LDA zero page opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 4: LDA Absolute encoding ($1234)
TEST(Cpu6502Test, LDA_Absolute) {
    Cpu6502 cpu;

    // LDA $1234 -> AD 34 12 (little-endian)
    auto bytes = cpu.EncodeLDA(0x1234, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0xAD);  // LDA absolute opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// Test 5: STA Zero Page encoding
TEST(Cpu6502Test, STA_ZeroPage) {
    Cpu6502 cpu;

    // STA $80 -> 85 80
    auto bytes = cpu.EncodeSTA(0x80, AddressingMode::ZeroPage);

    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x85);  // STA zero page opcode
    EXPECT_EQ(bytes[1], 0x80);
}

// Test 6: STA Absolute encoding
TEST(Cpu6502Test, STA_Absolute) {
    Cpu6502 cpu;

    // STA $1234 -> 8D 34 12
    auto bytes = cpu.EncodeSTA(0x1234, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x8D);  // STA absolute opcode
    EXPECT_EQ(bytes[1], 0x34);
    EXPECT_EQ(bytes[2], 0x12);
}

// Test 7: JMP Absolute encoding
TEST(Cpu6502Test, JMP_Absolute) {
    Cpu6502 cpu;

    // JMP $8000 -> 4C 00 80
    auto bytes = cpu.EncodeJMP(0x8000, AddressingMode::Absolute);

    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x4C);  // JMP absolute opcode
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x80);
}

// Test 8: NOP encoding
TEST(Cpu6502Test, NOP) {
    Cpu6502 cpu;

    // NOP -> EA
    auto bytes = cpu.EncodeNOP();

    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xEA);  // NOP opcode
}

// Test 9: RTS encoding
TEST(Cpu6502Test, RTS) {
    Cpu6502 cpu;

    // RTS -> 60
    auto bytes = cpu.EncodeRTS();

    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x60);  // RTS opcode
}

// Test 10: AddressingMode enum
TEST(Cpu6502Test, AddressingModes) {
    AddressingMode imm = AddressingMode::Immediate;
    AddressingMode zp = AddressingMode::ZeroPage;
    AddressingMode abs = AddressingMode::Absolute;

    EXPECT_NE(imm, zp);
    EXPECT_NE(zp, abs);
    EXPECT_NE(imm, abs);
}

// Test 11: Instruction size calculation
TEST(Cpu6502Test, CalculateSize) {
    Cpu6502 cpu;

    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Immediate), 2);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::ZeroPage), 2);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Absolute), 3);
    EXPECT_EQ(cpu.CalculateInstructionSize(AddressingMode::Implied), 1);
}

// Test 12: Multiple instructions
TEST(Cpu6502Test, MultipleInstructions) {
    Cpu6502 cpu;

    auto lda = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
    auto sta = cpu.EncodeSTA(0x80, AddressingMode::ZeroPage);
    auto nop = cpu.EncodeNOP();
    auto rts = cpu.EncodeRTS();

    EXPECT_EQ(lda[0], 0xA9);
    EXPECT_EQ(sta[0], 0x85);
    EXPECT_EQ(nop[0], 0xEA);
    EXPECT_EQ(rts[0], 0x60);
}
