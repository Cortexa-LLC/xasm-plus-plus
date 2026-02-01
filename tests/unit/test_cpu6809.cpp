/**
 * @file test_cpu6809.cpp
 * @brief Unit tests for Motorola 6809 CPU plugin
 * 
 * Tests instruction encoding for the 6809 processor following TDD:
 * RED -> GREEN -> REFACTOR cycle.
 */

#include <gtest/gtest.h>
#include "xasm++/cpu/cpu_6809.h"

using namespace xasm;

// ============================================================================
// Test Fixture
// ============================================================================

class Cpu6809Test : public ::testing::Test {
protected:
    Cpu6809 cpu;
};

// ============================================================================
// Phase 1: Foundation Tests
// ============================================================================

TEST_F(Cpu6809Test, GetName_Returns6809) {
    EXPECT_EQ("6809", cpu.GetName());
}

TEST_F(Cpu6809Test, DirectPage_DefaultsToZero) {
    EXPECT_EQ(0x00, cpu.GetDirectPage());
}

TEST_F(Cpu6809Test, DirectPage_CanBeSet) {
    cpu.SetDirectPage(0x20);
    EXPECT_EQ(0x20, cpu.GetDirectPage());
}

// ============================================================================
// Phase 2: Inherent Mode Instructions (1 byte, no operand)
// ============================================================================

TEST_F(Cpu6809Test, NOP_InherentMode) {
    auto bytes = cpu.EncodeNOP();
    ASSERT_EQ(1, bytes.size());
    EXPECT_EQ(0x12, bytes[0]);
}

TEST_F(Cpu6809Test, RTS_InherentMode) {
    auto bytes = cpu.EncodeRTS();
    ASSERT_EQ(1, bytes.size());
    EXPECT_EQ(0x39, bytes[0]);
}

TEST_F(Cpu6809Test, CLRA_InherentMode) {
    auto bytes = cpu.EncodeCLRA();
    ASSERT_EQ(1, bytes.size());
    EXPECT_EQ(0x4F, bytes[0]);
}

TEST_F(Cpu6809Test, CLRB_InherentMode) {
    auto bytes = cpu.EncodeCLRB();
    ASSERT_EQ(1, bytes.size());
    EXPECT_EQ(0x5F, bytes[0]);
}

// ============================================================================
// Phase 3: Immediate Addressing Mode (8-bit and 16-bit)
// ============================================================================

TEST_F(Cpu6809Test, LDA_Immediate8Bit) {
    auto bytes = cpu.EncodeLDA(0x42, AddressingMode6809::Immediate8);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0x86, bytes[0]);  // LDA immediate opcode
    EXPECT_EQ(0x42, bytes[1]);  // Operand
}

TEST_F(Cpu6809Test, LDB_Immediate8Bit) {
    auto bytes = cpu.EncodeLDB(0x55, AddressingMode6809::Immediate8);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0xC6, bytes[0]);  // LDB immediate opcode
    EXPECT_EQ(0x55, bytes[1]);  // Operand
}

TEST_F(Cpu6809Test, LDD_Immediate16Bit_BigEndian) {
    // LDD #$1234 should encode as: 0xCC, 0x12, 0x34 (big-endian!)
    auto bytes = cpu.EncodeLDD(0x1234, AddressingMode6809::Immediate16);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xCC, bytes[0]);  // LDD immediate opcode
    EXPECT_EQ(0x12, bytes[1]);  // High byte (MSB first)
    EXPECT_EQ(0x34, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, LDD_Immediate16Bit_VerifyBigEndian) {
    // Verify big-endian with different value
    auto bytes = cpu.EncodeLDD(0xABCD, AddressingMode6809::Immediate16);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xCC, bytes[0]);
    EXPECT_EQ(0xAB, bytes[1]);  // High byte first
    EXPECT_EQ(0xCD, bytes[2]);  // Low byte second
}

// ============================================================================
// Phase 4: Direct Addressing Mode
// ============================================================================

TEST_F(Cpu6809Test, LDA_Direct) {
    auto bytes = cpu.EncodeLDA(0x80, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0x96, bytes[0]);  // LDA direct opcode
    EXPECT_EQ(0x80, bytes[1]);  // Direct page offset
}

TEST_F(Cpu6809Test, LDB_Direct) {
    auto bytes = cpu.EncodeLDB(0x40, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0xD6, bytes[0]);  // LDB direct opcode
    EXPECT_EQ(0x40, bytes[1]);
}

TEST_F(Cpu6809Test, LDD_Direct) {
    auto bytes = cpu.EncodeLDD(0xFF, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0xDC, bytes[0]);  // LDD direct opcode
    EXPECT_EQ(0xFF, bytes[1]);
}

TEST_F(Cpu6809Test, STA_Direct) {
    auto bytes = cpu.EncodeSTA(0x20, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0x97, bytes[0]);  // STA direct opcode
    EXPECT_EQ(0x20, bytes[1]);
}

TEST_F(Cpu6809Test, STB_Direct) {
    auto bytes = cpu.EncodeSTB(0x30, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0xD7, bytes[0]);  // STB direct opcode
    EXPECT_EQ(0x30, bytes[1]);
}

TEST_F(Cpu6809Test, STD_Direct) {
    auto bytes = cpu.EncodeSTD(0x40, AddressingMode6809::Direct);
    ASSERT_EQ(2, bytes.size());
    EXPECT_EQ(0xDD, bytes[0]);  // STD direct opcode
    EXPECT_EQ(0x40, bytes[1]);
}

// ============================================================================
// Phase 4: Extended Addressing Mode (16-bit absolute)
// ============================================================================

TEST_F(Cpu6809Test, LDA_Extended_BigEndian) {
    auto bytes = cpu.EncodeLDA(0x1234, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xB6, bytes[0]);  // LDA extended opcode
    EXPECT_EQ(0x12, bytes[1]);  // High byte (big-endian)
    EXPECT_EQ(0x34, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, LDB_Extended_BigEndian) {
    auto bytes = cpu.EncodeLDB(0x5678, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xF6, bytes[0]);  // LDB extended opcode
    EXPECT_EQ(0x56, bytes[1]);  // High byte
    EXPECT_EQ(0x78, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, LDD_Extended_BigEndian) {
    auto bytes = cpu.EncodeLDD(0x9ABC, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xFC, bytes[0]);  // LDD extended opcode
    EXPECT_EQ(0x9A, bytes[1]);  // High byte
    EXPECT_EQ(0xBC, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, STA_Extended_BigEndian) {
    auto bytes = cpu.EncodeSTA(0x8000, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xB7, bytes[0]);  // STA extended opcode
    EXPECT_EQ(0x80, bytes[1]);  // High byte
    EXPECT_EQ(0x00, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, STB_Extended_BigEndian) {
    auto bytes = cpu.EncodeSTB(0xFFFE, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xF7, bytes[0]);  // STB extended opcode
    EXPECT_EQ(0xFF, bytes[1]);  // High byte
    EXPECT_EQ(0xFE, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, STD_Extended_BigEndian) {
    auto bytes = cpu.EncodeSTD(0xDEAD, AddressingMode6809::Extended);
    ASSERT_EQ(3, bytes.size());
    EXPECT_EQ(0xFD, bytes[0]);  // STD extended opcode
    EXPECT_EQ(0xDE, bytes[1]);  // High byte
    EXPECT_EQ(0xAD, bytes[2]);  // Low byte
}

// ============================================================================
// Instruction Size Calculation Tests
// ============================================================================

TEST_F(Cpu6809Test, CalculateInstructionSize_Inherent) {
    EXPECT_EQ(1, cpu.CalculateInstructionSize(AddressingMode6809::Inherent));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Immediate8) {
    EXPECT_EQ(2, cpu.CalculateInstructionSize(AddressingMode6809::Immediate8));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Immediate16) {
    EXPECT_EQ(3, cpu.CalculateInstructionSize(AddressingMode6809::Immediate16));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Direct) {
    EXPECT_EQ(2, cpu.CalculateInstructionSize(AddressingMode6809::Direct));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Extended) {
    EXPECT_EQ(3, cpu.CalculateInstructionSize(AddressingMode6809::Extended));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Relative8) {
    EXPECT_EQ(2, cpu.CalculateInstructionSize(AddressingMode6809::Relative8));
}

TEST_F(Cpu6809Test, CalculateInstructionSize_Relative16) {
    EXPECT_EQ(3, cpu.CalculateInstructionSize(AddressingMode6809::Relative16));
}

// ============================================================================
// Big-Endian Byte Order Verification
// ============================================================================

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0x0000) {
    auto bytes = cpu.EncodeLDD(0x0000, AddressingMode6809::Immediate16);
    EXPECT_EQ(0x00, bytes[1]);  // High byte
    EXPECT_EQ(0x00, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0xFFFF) {
    auto bytes = cpu.EncodeLDD(0xFFFF, AddressingMode6809::Immediate16);
    EXPECT_EQ(0xFF, bytes[1]);  // High byte
    EXPECT_EQ(0xFF, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0x1200) {
    auto bytes = cpu.EncodeLDD(0x1200, AddressingMode6809::Immediate16);
    EXPECT_EQ(0x12, bytes[1]);  // High byte
    EXPECT_EQ(0x00, bytes[2]);  // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0x0034) {
    auto bytes = cpu.EncodeLDD(0x0034, AddressingMode6809::Immediate16);
    EXPECT_EQ(0x00, bytes[1]);  // High byte
    EXPECT_EQ(0x34, bytes[2]);  // Low byte
}
