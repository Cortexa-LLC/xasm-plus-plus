/**
 * @file test_cpu6809_indexed.cpp
 * @brief Unit tests for Motorola 6809 Indexed Addressing Modes (16 sub-modes)
 *
 * Tests the complex indexed addressing post-byte encoding following TDD:
 * RED -> GREEN -> REFACTOR cycle.
 *
 * The 6809 has 16 indexed addressing sub-modes encoded in a post-byte.
 * This is the most complex feature of the 6809 CPU.
 */

#include "xasm++/cpu/cpu_6809.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Test Fixture
// ============================================================================

class Cpu6809IndexedTest : public ::testing::Test {
protected:
  Cpu6809 cpu;
};

// ============================================================================
// Phase 1: Zero Offset Indexed (,X ,Y ,U ,S)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedZeroOffset_X) {
  // LDA ,X -> opcode A6, post-byte 84
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedZeroOffset_Y) {
  // LDA ,Y -> opcode A6, post-byte A4
  // Y register bits: 01 (bits 6-5)
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]);
  // TODO: Need to pass register selection separately
  // EXPECT_EQ(0xA4, bytes[1]);  // Post-byte: 10100100 (,Y)
}

// ============================================================================
// Phase 2: 5-bit Offset Indexed (-16 to +15 offset)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Zero) {
  // LDA 0,X -> post-byte 00000000 (5-bit zero offset)
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x00, bytes[1]); // Post-byte: 00000000 (0,X)
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Positive) {
  // LDA 10,X -> post-byte 00001010
  auto bytes = cpu.EncodeLDA(10, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x0A, bytes[1]); // Post-byte: 00001010 (10,X)
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Max_Positive) {
  // LDA 15,X -> post-byte 00001111
  auto bytes = cpu.EncodeLDA(15, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x0F, bytes[1]); // Post-byte: 00001111 (15,X)
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Negative) {
  // LDA -1,X -> post-byte 00011111 (5-bit two's complement)
  auto bytes = cpu.EncodeLDA(-1, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x1F, bytes[1]); // Post-byte: 00011111 (-1,X)
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Max_Negative) {
  // LDA -16,X -> post-byte 00010000 (5-bit two's complement)
  auto bytes = cpu.EncodeLDA(-16, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x10, bytes[1]); // Post-byte: 00010000 (-16,X)
}

// ============================================================================
// Phase 3: 8-bit Offset Indexed (-128 to +127 offset)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_Indexed8BitOffset_Positive) {
  // LDA 100,X -> opcode A6, post-byte 88, offset 64
  auto bytes = cpu.EncodeLDA(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed8BitOffset_Negative) {
  // LDA -50,X -> opcode A6, post-byte 88, offset CE
  auto bytes = cpu.EncodeLDA(-50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000
  EXPECT_EQ(0xCE, bytes[2]); // Offset: -50 as two's complement
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed8BitOffset_Max_Positive) {
  // LDA 127,X
  auto bytes = cpu.EncodeLDA(127, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x88, bytes[1]);
  EXPECT_EQ(0x7F, bytes[2]); // Offset: 127
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed8BitOffset_Max_Negative) {
  // LDA -128,X
  auto bytes = cpu.EncodeLDA(-128, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x88, bytes[1]);
  EXPECT_EQ(0x80, bytes[2]); // Offset: -128 as two's complement
}

// ============================================================================
// Phase 4: 16-bit Offset Indexed (-32768 to +32767 offset)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_Indexed16BitOffset_Positive) {
  // LDA 1000,X -> opcode A6, post-byte 89, offset 03E8 (big-endian)
  auto bytes = cpu.EncodeLDA(1000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: 10001001 (16-bit offset, X)
  EXPECT_EQ(0x03, bytes[2]); // Offset high byte
  EXPECT_EQ(0xE8, bytes[3]); // Offset low byte
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed16BitOffset_Negative) {
  // LDA -1000,X -> opcode A6, post-byte 89, offset FC18
  auto bytes = cpu.EncodeLDA(-1000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: 10001001
  EXPECT_EQ(0xFC, bytes[2]); // Offset high byte (two's complement)
  EXPECT_EQ(0x18, bytes[3]); // Offset low byte
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed16BitOffset_Max_Positive) {
  // LDA 32767,X
  auto bytes = cpu.EncodeLDA(32767, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x89, bytes[1]);
  EXPECT_EQ(0x7F, bytes[2]); // High byte: 0x7F
  EXPECT_EQ(0xFF, bytes[3]); // Low byte: 0xFF
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed16BitOffset_Max_Negative) {
  // LDA -32768,X
  auto bytes = cpu.EncodeLDA(-32768, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x89, bytes[1]);
  EXPECT_EQ(0x80, bytes[2]); // High byte: 0x80
  EXPECT_EQ(0x00, bytes[3]); // Low byte: 0x00
}

// ============================================================================
// Phase 5: Accumulator Offset Indexed (A,X  B,Y  D,U)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedAccumA) {
  // LDA A,X -> opcode A6, post-byte 86
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAccumA);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x86, bytes[1]); // Post-byte: 10000110 (A,X)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedAccumB) {
  // LDA B,X -> opcode A6, post-byte 85
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAccumB);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x85, bytes[1]); // Post-byte: 10000101 (B,X)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedAccumD) {
  // LDA D,X -> opcode A6, post-byte 8B
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAccumD);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x8B, bytes[1]); // Post-byte: 10001011 (D,X)
}

// ============================================================================
// Phase 6: Auto-Increment Indexed (,X+  ,X++)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedAutoInc1) {
  // LDA ,X+ -> opcode A6, post-byte 80
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAutoInc1);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x80, bytes[1]); // Post-byte: 10000000 (,X+)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedAutoInc2) {
  // LDA ,X++ -> opcode A6, post-byte 81
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAutoInc2);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x81, bytes[1]); // Post-byte: 10000001 (,X++)
}

// ============================================================================
// Phase 7: Auto-Decrement Indexed (,-X  ,--X)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedAutoDec1) {
  // LDA ,-X -> opcode A6, post-byte 82
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAutoDec1);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x82, bytes[1]); // Post-byte: 10000010 (,-X)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedAutoDec2) {
  // LDA ,--X -> opcode A6, post-byte 83
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedAutoDec2);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x83, bytes[1]); // Post-byte: 10000011 (,--X)
}

// ============================================================================
// Phase 8: PC-Relative Indexed (label,PCR)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedPCRelative8) {
  // LDA 50,PCR -> opcode A6, post-byte 8C, offset 32
  auto bytes = cpu.EncodeLDA(50, AddressingMode6809::IndexedPCRelative8);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x8C, bytes[1]); // Post-byte: 10001100 (8-bit PC-relative)
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedPCRelative8_Negative) {
  // LDA -50,PCR
  auto bytes = cpu.EncodeLDA(-50, AddressingMode6809::IndexedPCRelative8);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x8C, bytes[1]); // Post-byte: 10001100
  EXPECT_EQ(0xCE, bytes[2]); // Offset: -50 as two's complement
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedPCRelative16) {
  // LDA 1000,PCR -> opcode A6, post-byte 8D, offset 03E8
  auto bytes = cpu.EncodeLDA(1000, AddressingMode6809::IndexedPCRelative16);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x8D, bytes[1]); // Post-byte: 10001101 (16-bit PC-relative)
  EXPECT_EQ(0x03, bytes[2]); // Offset high byte
  EXPECT_EQ(0xE8, bytes[3]); // Offset low byte
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedPCRelative16_Negative) {
  // LDA -1000,PCR
  auto bytes = cpu.EncodeLDA(-1000, AddressingMode6809::IndexedPCRelative16);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x8D, bytes[1]); // Post-byte: 10001101
  EXPECT_EQ(0xFC, bytes[2]); // Offset high byte (two's complement)
  EXPECT_EQ(0x18, bytes[3]); // Offset low byte
}

// ============================================================================
// Phase 9: Indirect Indexed ([,X]  [10,Y])
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedIndirect_ZeroOffset) {
  // LDA [,X] -> opcode A6, post-byte 94 (indirect flag set)
  auto bytes = cpu.EncodeLDA(0, AddressingMode6809::IndexedIndirect);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x94, bytes[1]); // Post-byte: 10010100 ([,X])
                             // Bit 7 = 1 (indirect)
                             // Bits 6-5 = 00 (X register)
                             // Bits 4-0 = 10100 (zero offset mode)
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedIndirect_8BitOffset) {
  // LDA [10,X] -> opcode A6, post-byte 98, offset 0A
  auto bytes = cpu.EncodeLDA(10, AddressingMode6809::IndexedIndirect);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x98, bytes[1]); // Post-byte: 10011000 ([8-bit offset, X])
                             // Bit 7 = 1 (indirect)
  EXPECT_EQ(0x0A, bytes[2]); // Offset: 10
}

TEST_F(Cpu6809IndexedTest, LDA_IndexedIndirect_16BitOffset) {
  // LDA [1000,X] -> opcode A6, post-byte 99, offset 03E8
  auto bytes = cpu.EncodeLDA(1000, AddressingMode6809::IndexedIndirect);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x99, bytes[1]); // Post-byte: 10011001 ([16-bit offset, X])
  EXPECT_EQ(0x03, bytes[2]); // Offset high byte
  EXPECT_EQ(0xE8, bytes[3]); // Offset low byte
}

// ============================================================================
// Phase 10: Extended Indirect ([$1234])
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_IndexedExtendedIndirect) {
  // LDA [$1234] -> opcode A6, post-byte 9F, address 1234
  auto bytes =
      cpu.EncodeLDA(0x1234, AddressingMode6809::IndexedExtendedIndirect);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]); // LDA indexed opcode
  EXPECT_EQ(0x9F, bytes[1]); // Post-byte: 10011111 (extended indirect)
  EXPECT_EQ(0x12, bytes[2]); // Address high byte
  EXPECT_EQ(0x34, bytes[3]); // Address low byte
}

// ============================================================================
// Phase 11: Register Variants (Y, U, S registers)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_Y_Register) {
  // LDA 5,Y -> post-byte 00100101 (Y register, 5-bit offset)
  // Bits 6-5 = 01 (Y register)
  // TODO: Need API to specify register
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_U_Register) {
  // LDA 5,U -> post-byte 01000101 (U register, 5-bit offset)
  // Bits 6-5 = 10 (U register)
  // TODO: Need API to specify register
}

TEST_F(Cpu6809IndexedTest, LDA_Indexed5BitOffset_S_Register) {
  // LDA 5,S -> post-byte 01100101 (S register, 5-bit offset)
  // Bits 6-5 = 11 (S register)
  // TODO: Need API to specify register
}

// ============================================================================
// Phase 12: Comprehensive Examples with Different Instructions
// ============================================================================

TEST_F(Cpu6809IndexedTest, STB_IndexedZeroOffset) {
  // STB ,X -> different opcode, same post-byte pattern
  auto bytes = cpu.EncodeSTB(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xE7, bytes[0]); // STB indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, LDD_Indexed8BitOffset) {
  // LDD 50,X
  auto bytes = cpu.EncodeLDD(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xEC, bytes[0]); // LDD indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, LEAX_Indexed16BitOffset) {
  // LEAX 2000,X
  auto bytes = cpu.EncodeLEAX(2000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0x30, bytes[0]); // LEAX opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: 10001001 (16-bit offset, X)
  EXPECT_EQ(0x07, bytes[2]); // Offset high byte (2000 = 0x07D0)
  EXPECT_EQ(0xD0, bytes[3]); // Offset low byte
}

// ============================================================================
// Phase 13: Edge Cases and Boundary Conditions
// ============================================================================

TEST_F(Cpu6809IndexedTest, Indexed_ChoosesCorrectMode_ForOffset_16) {
  // Offset 16 is just outside 5-bit range, should use 8-bit
  auto bytes = cpu.EncodeLDA(16, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x88, bytes[1]); // 8-bit offset mode
  EXPECT_EQ(0x10, bytes[2]); // Offset: 16
}

TEST_F(Cpu6809IndexedTest, Indexed_ChoosesCorrectMode_ForOffset_Minus17) {
  // Offset -17 is just outside 5-bit range, should use 8-bit
  auto bytes = cpu.EncodeLDA(-17, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x88, bytes[1]); // 8-bit offset mode
  EXPECT_EQ(0xEF, bytes[2]); // Offset: -17 as two's complement
}

TEST_F(Cpu6809IndexedTest, Indexed_ChoosesCorrectMode_ForOffset_128) {
  // Offset 128 is just outside 8-bit signed range, should use 16-bit
  auto bytes = cpu.EncodeLDA(128, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x89, bytes[1]); // 16-bit offset mode
  EXPECT_EQ(0x00, bytes[2]); // Offset high byte
  EXPECT_EQ(0x80, bytes[3]); // Offset low byte
}

TEST_F(Cpu6809IndexedTest, Indexed_ChoosesCorrectMode_ForOffset_Minus129) {
  // Offset -129 is just outside 8-bit signed range, should use 16-bit
  auto bytes = cpu.EncodeLDA(-129, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA6, bytes[0]);
  EXPECT_EQ(0x89, bytes[1]); // 16-bit offset mode
  EXPECT_EQ(0xFF, bytes[2]); // Offset high byte (two's complement)
  EXPECT_EQ(0x7F, bytes[3]); // Offset low byte
}

// ============================================================================
// Phase 14: STA Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, STA_IndexedZeroOffset) {
  // STA ,X -> opcode A7, post-byte 84
  auto bytes = cpu.EncodeSTA(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA7, bytes[0]); // STA indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, STA_Indexed5BitOffset) {
  // STA 10,X
  auto bytes = cpu.EncodeSTA(10, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA7, bytes[0]); // STA indexed opcode
  EXPECT_EQ(0x0A, bytes[1]); // Post-byte: 00001010 (10,X)
}

TEST_F(Cpu6809IndexedTest, STA_Indexed8BitOffset) {
  // STA 100,X
  auto bytes = cpu.EncodeSTA(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA7, bytes[0]); // STA indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, STA_Indexed16BitOffset) {
  // STA 1000,X
  auto bytes = cpu.EncodeSTA(1000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xA7, bytes[0]); // STA indexed opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: 10001001 (16-bit offset, X)
  EXPECT_EQ(0x03, bytes[2]); // Offset high byte
  EXPECT_EQ(0xE8, bytes[3]); // Offset low byte
}

// ============================================================================
// Phase 15: STD Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, STD_IndexedZeroOffset) {
  // STD ,X -> opcode ED, post-byte 84
  auto bytes = cpu.EncodeSTD(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // STD indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, STD_Indexed8BitOffset) {
  // STD 50,X
  auto bytes = cpu.EncodeSTD(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xED, bytes[0]); // STD indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, STD_IndexedAutoInc2) {
  // STD ,X++
  auto bytes = cpu.EncodeSTD(0, AddressingMode6809::IndexedAutoInc2);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // STD indexed opcode
  EXPECT_EQ(0x81, bytes[1]); // Post-byte: 10000001 (,X++)
}

// ============================================================================
// Phase 16: LDX Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDX_IndexedZeroOffset) {
  // LDX ,X -> opcode AE, post-byte 84
  auto bytes = cpu.EncodeLDX(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAE, bytes[0]); // LDX indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, LDX_Indexed8BitOffset) {
  // LDX 100,X
  auto bytes = cpu.EncodeLDX(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xAE, bytes[0]); // LDX indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, LDX_IndexedAccumD) {
  // LDX D,X
  auto bytes = cpu.EncodeLDX(0, AddressingMode6809::IndexedAccumD);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAE, bytes[0]); // LDX indexed opcode
  EXPECT_EQ(0x8B, bytes[1]); // Post-byte: 10001011 (D,X)
}

// ============================================================================
// Phase 17: LDY Indexed Addressing Modes (Page 2)
// ============================================================================

TEST_F(Cpu6809IndexedTest, LDY_IndexedZeroOffset) {
  // LDY ,X -> page 2 prefix 10, opcode AE, post-byte 84
  auto bytes = cpu.EncodeLDY(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAE, bytes[1]); // LDY indexed opcode
  EXPECT_EQ(0x84, bytes[2]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, LDY_Indexed8BitOffset) {
  // LDY 50,X
  auto bytes = cpu.EncodeLDY(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAE, bytes[1]); // LDY indexed opcode
  EXPECT_EQ(0x88, bytes[2]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x32, bytes[3]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, LDY_Indexed16BitOffset) {
  // LDY 2000,X
  auto bytes = cpu.EncodeLDY(2000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 5UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAE, bytes[1]); // LDY indexed opcode
  EXPECT_EQ(0x89, bytes[2]); // Post-byte: 10001001 (16-bit offset, X)
  EXPECT_EQ(0x07, bytes[3]); // Offset high byte (2000 = 0x07D0)
  EXPECT_EQ(0xD0, bytes[4]); // Offset low byte
}

// ============================================================================
// Phase 18: STX Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, STX_IndexedZeroOffset) {
  // STX ,X -> opcode AF, post-byte 84
  auto bytes = cpu.EncodeSTX(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAF, bytes[0]); // STX indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, STX_Indexed8BitOffset) {
  // STX 100,X
  auto bytes = cpu.EncodeSTX(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xAF, bytes[0]); // STX indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, STX_IndexedAutoDec2) {
  // STX ,--X
  auto bytes = cpu.EncodeSTX(0, AddressingMode6809::IndexedAutoDec2);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAF, bytes[0]); // STX indexed opcode
  EXPECT_EQ(0x83, bytes[1]); // Post-byte: 10000011 (,--X)
}

// ============================================================================
// Phase 19: STY Indexed Addressing Modes (Page 2)
// ============================================================================

TEST_F(Cpu6809IndexedTest, STY_IndexedZeroOffset) {
  // STY ,X -> page 2 prefix 10, opcode AF, post-byte 84
  auto bytes = cpu.EncodeSTY(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAF, bytes[1]); // STY indexed opcode
  EXPECT_EQ(0x84, bytes[2]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, STY_Indexed8BitOffset) {
  // STY 50,X
  auto bytes = cpu.EncodeSTY(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAF, bytes[1]); // STY indexed opcode
  EXPECT_EQ(0x88, bytes[2]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x32, bytes[3]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, STY_IndexedAutoInc2) {
  // STY ,X++
  auto bytes = cpu.EncodeSTY(0, AddressingMode6809::IndexedAutoInc2);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xAF, bytes[1]); // STY indexed opcode
  EXPECT_EQ(0x81, bytes[2]); // Post-byte: 10000001 (,X++)
}

// ============================================================================
// Phase 20: ADDA Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, ADDA_IndexedZeroOffset) {
  // ADDA ,X -> opcode AB, post-byte 84
  auto bytes = cpu.EncodeADDA(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAB, bytes[0]); // ADDA indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, ADDA_Indexed5BitOffset) {
  // ADDA 10,X
  auto bytes = cpu.EncodeADDA(10, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAB, bytes[0]); // ADDA indexed opcode
  EXPECT_EQ(0x0A, bytes[1]); // Post-byte: 00001010 (10,X)
}

TEST_F(Cpu6809IndexedTest, ADDA_Indexed8BitOffset) {
  // ADDA 100,X
  auto bytes = cpu.EncodeADDA(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xAB, bytes[0]); // ADDA indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, ADDA_IndexedAccumB) {
  // ADDA B,X
  auto bytes = cpu.EncodeADDA(0, AddressingMode6809::IndexedAccumB);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xAB, bytes[0]); // ADDA indexed opcode
  EXPECT_EQ(0x85, bytes[1]); // Post-byte: 10000101 (B,X)
}

// ============================================================================
// Phase 21: ADDB Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, ADDB_IndexedZeroOffset) {
  // ADDB ,X -> opcode EB, post-byte 84
  auto bytes = cpu.EncodeADDB(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xEB, bytes[0]); // ADDB indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, ADDB_Indexed8BitOffset) {
  // ADDB 50,X
  auto bytes = cpu.EncodeADDB(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xEB, bytes[0]); // ADDB indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, ADDB_IndexedPCRelative8) {
  // ADDB 50,PCR
  auto bytes = cpu.EncodeADDB(50, AddressingMode6809::IndexedPCRelative8);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xEB, bytes[0]); // ADDB indexed opcode
  EXPECT_EQ(0x8C, bytes[1]); // Post-byte: 10001100 (8-bit PC-relative)
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

// ============================================================================
// Phase 22: SUBA Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, SUBA_IndexedZeroOffset) {
  // SUBA ,X -> opcode A0, post-byte 84
  auto bytes = cpu.EncodeSUBA(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA0, bytes[0]); // SUBA indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, SUBA_Indexed5BitOffset_Negative) {
  // SUBA -5,X
  auto bytes = cpu.EncodeSUBA(-5, AddressingMode6809::Indexed5BitOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xA0, bytes[0]); // SUBA indexed opcode
  EXPECT_EQ(0x1B,
            bytes[1]); // Post-byte: 00011011 (-5,X in 5-bit two's complement)
}

TEST_F(Cpu6809IndexedTest, SUBA_Indexed8BitOffset) {
  // SUBA 100,X
  auto bytes = cpu.EncodeSUBA(100, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA0, bytes[0]); // SUBA indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x64, bytes[2]); // Offset: 100
}

TEST_F(Cpu6809IndexedTest, SUBA_IndexedIndirect) {
  // SUBA [10,X]
  auto bytes = cpu.EncodeSUBA(10, AddressingMode6809::IndexedIndirect);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xA0, bytes[0]); // SUBA indexed opcode
  EXPECT_EQ(0x98, bytes[1]); // Post-byte: 10011000 ([8-bit offset, X])
  EXPECT_EQ(0x0A, bytes[2]); // Offset: 10
}

// ============================================================================
// Phase 23: SUBB Indexed Addressing Modes
// ============================================================================

TEST_F(Cpu6809IndexedTest, SUBB_IndexedZeroOffset) {
  // SUBB ,X -> opcode E0, post-byte 84
  auto bytes = cpu.EncodeSUBB(0, AddressingMode6809::IndexedZeroOffset);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xE0, bytes[0]); // SUBB indexed opcode
  EXPECT_EQ(0x84, bytes[1]); // Post-byte: 10000100 (,X)
}

TEST_F(Cpu6809IndexedTest, SUBB_Indexed8BitOffset) {
  // SUBB 50,X
  auto bytes = cpu.EncodeSUBB(50, AddressingMode6809::Indexed8BitOffset);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xE0, bytes[0]); // SUBB indexed opcode
  EXPECT_EQ(0x88, bytes[1]); // Post-byte: 10001000 (8-bit offset, X)
  EXPECT_EQ(0x32, bytes[2]); // Offset: 50
}

TEST_F(Cpu6809IndexedTest, SUBB_Indexed16BitOffset_Negative) {
  // SUBB -1000,X
  auto bytes = cpu.EncodeSUBB(-1000, AddressingMode6809::Indexed16BitOffset);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xE0, bytes[0]); // SUBB indexed opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: 10001001 (16-bit offset, X)
  EXPECT_EQ(0xFC, bytes[2]); // Offset high byte (two's complement)
  EXPECT_EQ(0x18, bytes[3]); // Offset low byte
}

TEST_F(Cpu6809IndexedTest, SUBB_IndexedAutoInc1) {
  // SUBB ,X+
  auto bytes = cpu.EncodeSUBB(0, AddressingMode6809::IndexedAutoInc1);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xE0, bytes[0]); // SUBB indexed opcode
  EXPECT_EQ(0x80, bytes[1]); // Post-byte: 10000000 (,X+)
}
