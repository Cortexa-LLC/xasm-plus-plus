// 65816 Extended Instructions tests
// Tests for SEP, REP, and other missing 65816 instructions

#include "xasm++/cpu/cpu_6502.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Group 1: SEP/REP - Set/Reset Processor Status Bits
// ============================================================================

// Test 1: SEP (Set Processor Status Bits) - 65816
TEST(Cpu65816ExtendedTest, SEP_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // SEP #$30 -> E2 30 (set M and X to 8-bit mode)
    auto bytes = cpu.EncodeSEP(0x30, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xE2);  // SEP opcode
    EXPECT_EQ(bytes[1], 0x30);  // Status bits to set
}

// Test 2: REP (Reset Processor Status Bits) - 65816
TEST(Cpu65816ExtendedTest, REP_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // REP #$30 -> C2 30 (clear M and X for 16-bit mode)
    auto bytes = cpu.EncodeREP(0x30, AddressingMode::Immediate);
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xC2);  // REP opcode
    EXPECT_EQ(bytes[1], 0x30);  // Status bits to reset
}

// Test 3: SEP not available in 6502 mode
TEST(Cpu65816ExtendedTest, SEP_NotAvailableIn6502Mode) {
    Cpu6502 cpu;
    // Default mode is 6502

    auto bytes = cpu.EncodeSEP(0x30, AddressingMode::Immediate);
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported
}

// Test 4: REP not available in 65C02 mode
TEST(Cpu65816ExtendedTest, REP_NotAvailableIn65C02Mode) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65C02);

    auto bytes = cpu.EncodeREP(0x30, AddressingMode::Immediate);
    EXPECT_EQ(bytes.size(), 0);  // Empty = not supported
}

// Test 5: XCE (Exchange Carry and Emulation) already implemented
TEST(Cpu65816ExtendedTest, XCE_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // XCE -> FB
    auto bytes = cpu.EncodeXCE();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xFB);  // XCE opcode
}

// Test 6: PHB (Push Data Bank) already implemented
TEST(Cpu65816ExtendedTest, PHB_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PHB -> 8B
    auto bytes = cpu.EncodePHB();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x8B);  // PHB opcode
}

// Test 7: PLB (Pull Data Bank) already implemented
TEST(Cpu65816ExtendedTest, PLB_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // PLB -> AB
    auto bytes = cpu.EncodePLB();
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0xAB);  // PLB opcode
}

// Test 8: MVN (Block Move Negative) already implemented
TEST(Cpu65816ExtendedTest, MVN_65816) {
    Cpu6502 cpu;
    cpu.SetCpuMode(CpuMode::Cpu65816);

    // MVN $12,$34 -> 54 12 34
    auto bytes = cpu.EncodeMVN(0x12, 0x34);
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x54);  // MVN opcode
    EXPECT_EQ(bytes[1], 0x12);  // Source bank
    EXPECT_EQ(bytes[2], 0x34);  // Dest bank
}
