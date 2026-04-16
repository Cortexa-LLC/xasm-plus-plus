/**
 * @file test_cpu_65c02.cpp
 * @brief Unit tests for 65C02 CPU extensions
 *
 * Tests for 65C02-specific instruction encoding:
 * - Stack operations (PHX, PLX, PHY, PLY)
 * - Store Zero (STZ)
 * - Bit test instructions (TRB, TSB)
 * - Branch Always (BRA)
 * - Rejection of 65C02 instructions in 6502 mode
 */

#include "xasm++/cpu/cpu_6502.h"
#include <gtest/gtest.h>

using namespace xasm;

class Cpu65c02Test : public ::testing::Test {
protected:
  void SetUp() override {
    cpu_65c02_.SetCpuMode(CpuMode::Cpu65C02);
    // cpu_6502_ stays at default CpuMode::Cpu6502
  }
  Cpu6502 cpu_65c02_;
  Cpu6502 cpu_6502_;
};

// ==========================================================================
// Group 2: Stack Operations
// ==========================================================================

TEST_F(Cpu65c02Test, PHX_65C02) {
  auto bytes = cpu_65c02_.EncodePHX();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(bytes[0], 0xDA);
}

TEST_F(Cpu65c02Test, PHX_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodePHX();
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, PLX_65C02) {
  auto bytes = cpu_65c02_.EncodePLX();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(bytes[0], 0xFA);
}

TEST_F(Cpu65c02Test, PLX_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodePLX();
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, PHY_65C02) {
  auto bytes = cpu_65c02_.EncodePHY();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(bytes[0], 0x5A);
}

TEST_F(Cpu65c02Test, PHY_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodePHY();
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, PLY_65C02) {
  auto bytes = cpu_65c02_.EncodePLY();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(bytes[0], 0x7A);
}

TEST_F(Cpu65c02Test, PLY_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodePLY();
  EXPECT_TRUE(bytes.empty());
}

// ==========================================================================
// Group 3: Store Zero
// ==========================================================================

TEST_F(Cpu65c02Test, STZ_ZeroPage) {
  auto bytes = cpu_65c02_.EncodeSTZ(0x42, AddressingMode::ZeroPage);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(bytes[0], 0x64);
  EXPECT_EQ(bytes[1], 0x42);
}

TEST_F(Cpu65c02Test, STZ_ZeroPageX) {
  auto bytes = cpu_65c02_.EncodeSTZ(0x42, AddressingMode::ZeroPageX);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(bytes[0], 0x74);
  EXPECT_EQ(bytes[1], 0x42);
}

TEST_F(Cpu65c02Test, STZ_Absolute) {
  auto bytes = cpu_65c02_.EncodeSTZ(0x1234, AddressingMode::Absolute);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(bytes[0], 0x9C);
  EXPECT_EQ(bytes[1], 0x34);  // Low byte
  EXPECT_EQ(bytes[2], 0x12);  // High byte
}

TEST_F(Cpu65c02Test, STZ_AbsoluteX) {
  auto bytes = cpu_65c02_.EncodeSTZ(0x1234, AddressingMode::AbsoluteX);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(bytes[0], 0x9E);
  EXPECT_EQ(bytes[1], 0x34);
  EXPECT_EQ(bytes[2], 0x12);
}

TEST_F(Cpu65c02Test, STZ_InvalidMode_ReturnsEmpty) {
  auto bytes = cpu_65c02_.EncodeSTZ(0x42, AddressingMode::Immediate);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, STZ_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodeSTZ(0x42, AddressingMode::ZeroPage);
  EXPECT_TRUE(bytes.empty());
}

// ==========================================================================
// Group 4: Bit Test Instructions
// ==========================================================================

TEST_F(Cpu65c02Test, TRB_ZeroPage) {
  auto bytes = cpu_65c02_.EncodeTRB(0x42, AddressingMode::ZeroPage);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(bytes[0], 0x14);
  EXPECT_EQ(bytes[1], 0x42);
}

TEST_F(Cpu65c02Test, TRB_Absolute) {
  auto bytes = cpu_65c02_.EncodeTRB(0x1234, AddressingMode::Absolute);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(bytes[0], 0x1C);
  EXPECT_EQ(bytes[1], 0x34);
  EXPECT_EQ(bytes[2], 0x12);
}

TEST_F(Cpu65c02Test, TRB_InvalidMode_ReturnsEmpty) {
  auto bytes = cpu_65c02_.EncodeTRB(0x42, AddressingMode::Immediate);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, TRB_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodeTRB(0x42, AddressingMode::ZeroPage);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, TSB_ZeroPage) {
  auto bytes = cpu_65c02_.EncodeTSB(0x42, AddressingMode::ZeroPage);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(bytes[0], 0x04);
  EXPECT_EQ(bytes[1], 0x42);
}

TEST_F(Cpu65c02Test, TSB_Absolute) {
  auto bytes = cpu_65c02_.EncodeTSB(0x1234, AddressingMode::Absolute);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(bytes[0], 0x0C);
  EXPECT_EQ(bytes[1], 0x34);
  EXPECT_EQ(bytes[2], 0x12);
}

TEST_F(Cpu65c02Test, TSB_InvalidMode_ReturnsEmpty) {
  auto bytes = cpu_65c02_.EncodeTSB(0x42, AddressingMode::Immediate);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, TSB_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodeTSB(0x42, AddressingMode::ZeroPage);
  EXPECT_TRUE(bytes.empty());
}

// ==========================================================================
// Group 5: Branch Always
// ==========================================================================

TEST_F(Cpu65c02Test, BRA_Relative) {
  auto bytes = cpu_65c02_.EncodeBRA(0x10, AddressingMode::Relative);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(bytes[0], 0x80);  // BRA opcode
  EXPECT_EQ(bytes[1], 0x10);  // Relative offset
}

TEST_F(Cpu65c02Test, BRA_InvalidMode_ReturnsEmpty) {
  auto bytes = cpu_65c02_.EncodeBRA(0x10, AddressingMode::Absolute);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(Cpu65c02Test, BRA_NotAvailableIn6502) {
  auto bytes = cpu_6502_.EncodeBRA(0x10, AddressingMode::Relative);
  EXPECT_TRUE(bytes.empty());
}
