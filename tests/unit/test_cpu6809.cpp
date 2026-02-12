/**
 * @file test_cpu6809.cpp
 * @brief Unit tests for Motorola 6809 CPU plugin
 *
 * Tests instruction encoding for the 6809 processor following TDD:
 * RED -> GREEN -> REFACTOR cycle.
 */

#include "xasm++/cpu/cpu_6809.h"
#include <gtest/gtest.h>

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

TEST_F(Cpu6809Test, GetName_Returns6809) { EXPECT_EQ("6809", cpu.GetName()); }

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
  EXPECT_EQ(0x86, bytes[0]); // LDA immediate opcode
  EXPECT_EQ(0x42, bytes[1]); // Operand
}

TEST_F(Cpu6809Test, LDB_Immediate8Bit) {
  auto bytes = cpu.EncodeLDB(0x55, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC6, bytes[0]); // LDB immediate opcode
  EXPECT_EQ(0x55, bytes[1]); // Operand
}

TEST_F(Cpu6809Test, LDD_Immediate16Bit_BigEndian) {
  // LDD #$1234 should encode as: 0xCC, 0x12, 0x34 (big-endian!)
  auto bytes = cpu.EncodeLDD(0x1234, AddressingMode6809::Immediate16);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xCC, bytes[0]); // LDD immediate opcode
  EXPECT_EQ(0x12, bytes[1]); // High byte (MSB first)
  EXPECT_EQ(0x34, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LDD_Immediate16Bit_VerifyBigEndian) {
  // Verify big-endian with different value
  auto bytes = cpu.EncodeLDD(0xABCD, AddressingMode6809::Immediate16);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xCC, bytes[0]);
  EXPECT_EQ(0xAB, bytes[1]); // High byte first
  EXPECT_EQ(0xCD, bytes[2]); // Low byte second
}

// ============================================================================
// Phase 4: Direct Addressing Mode
// ============================================================================

TEST_F(Cpu6809Test, LDA_Direct) {
  auto bytes = cpu.EncodeLDA(0x80, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x96, bytes[0]); // LDA direct opcode
  EXPECT_EQ(0x80, bytes[1]); // Direct page offset
}

TEST_F(Cpu6809Test, LDB_Direct) {
  auto bytes = cpu.EncodeLDB(0x40, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xD6, bytes[0]); // LDB direct opcode
  EXPECT_EQ(0x40, bytes[1]);
}

TEST_F(Cpu6809Test, LDD_Direct) {
  auto bytes = cpu.EncodeLDD(0xFF, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xDC, bytes[0]); // LDD direct opcode
  EXPECT_EQ(0xFF, bytes[1]);
}

TEST_F(Cpu6809Test, STA_Direct) {
  auto bytes = cpu.EncodeSTA(0x20, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x97, bytes[0]); // STA direct opcode
  EXPECT_EQ(0x20, bytes[1]);
}

TEST_F(Cpu6809Test, STB_Direct) {
  auto bytes = cpu.EncodeSTB(0x30, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xD7, bytes[0]); // STB direct opcode
  EXPECT_EQ(0x30, bytes[1]);
}

TEST_F(Cpu6809Test, STD_Direct) {
  auto bytes = cpu.EncodeSTD(0x40, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xDD, bytes[0]); // STD direct opcode
  EXPECT_EQ(0x40, bytes[1]);
}

// ============================================================================
// Phase 4: Extended Addressing Mode (16-bit absolute)
// ============================================================================

TEST_F(Cpu6809Test, LDA_Extended_BigEndian) {
  auto bytes = cpu.EncodeLDA(0x1234, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xB6, bytes[0]); // LDA extended opcode
  EXPECT_EQ(0x12, bytes[1]); // High byte (big-endian)
  EXPECT_EQ(0x34, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LDB_Extended_BigEndian) {
  auto bytes = cpu.EncodeLDB(0x5678, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xF6, bytes[0]); // LDB extended opcode
  EXPECT_EQ(0x56, bytes[1]); // High byte
  EXPECT_EQ(0x78, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LDD_Extended_BigEndian) {
  auto bytes = cpu.EncodeLDD(0x9ABC, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xFC, bytes[0]); // LDD extended opcode
  EXPECT_EQ(0x9A, bytes[1]); // High byte
  EXPECT_EQ(0xBC, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, STA_Extended_BigEndian) {
  auto bytes = cpu.EncodeSTA(0x8000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xB7, bytes[0]); // STA extended opcode
  EXPECT_EQ(0x80, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, STB_Extended_BigEndian) {
  auto bytes = cpu.EncodeSTB(0xFFFE, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xF7, bytes[0]); // STB extended opcode
  EXPECT_EQ(0xFF, bytes[1]); // High byte
  EXPECT_EQ(0xFE, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, STD_Extended_BigEndian) {
  auto bytes = cpu.EncodeSTD(0xDEAD, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xFD, bytes[0]); // STD extended opcode
  EXPECT_EQ(0xDE, bytes[1]); // High byte
  EXPECT_EQ(0xAD, bytes[2]); // Low byte
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
  EXPECT_EQ(0x00, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0xFFFF) {
  auto bytes = cpu.EncodeLDD(0xFFFF, AddressingMode6809::Immediate16);
  EXPECT_EQ(0xFF, bytes[1]); // High byte
  EXPECT_EQ(0xFF, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0x1200) {
  auto bytes = cpu.EncodeLDD(0x1200, AddressingMode6809::Immediate16);
  EXPECT_EQ(0x12, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, BigEndian_ByteOrder_0x0034) {
  auto bytes = cpu.EncodeLDD(0x0034, AddressingMode6809::Immediate16);
  EXPECT_EQ(0x00, bytes[1]); // High byte
  EXPECT_EQ(0x34, bytes[2]); // Low byte
}

// ============================================================================
// Phase 5: 16-bit Register Load/Store (LDX, LDY, LDU, LDS, STX, STY, STU, STS)
// ============================================================================

TEST_F(Cpu6809Test, LDX_Immediate16Bit_BigEndian) {
  auto bytes = cpu.EncodeLDX(0x1000, AddressingMode6809::Immediate16);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x8E, bytes[0]); // LDX immediate opcode
  EXPECT_EQ(0x10, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LDX_Direct) {
  auto bytes = cpu.EncodeLDX(0x80, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x9E, bytes[0]); // LDX direct opcode
  EXPECT_EQ(0x80, bytes[1]);
}

TEST_F(Cpu6809Test, LDX_Extended_BigEndian) {
  auto bytes = cpu.EncodeLDX(0x2000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xBE, bytes[0]); // LDX extended opcode
  EXPECT_EQ(0x20, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LDY_Immediate16Bit_BigEndian) {
  // Page 2 opcode (prefix $10)
  auto bytes = cpu.EncodeLDY(0x3000, AddressingMode6809::Immediate16);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x8E, bytes[1]); // LDY immediate opcode
  EXPECT_EQ(0x30, bytes[2]); // High byte
  EXPECT_EQ(0x00, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LDY_Direct) {
  auto bytes = cpu.EncodeLDY(0x40, AddressingMode6809::Direct);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x9E, bytes[1]); // LDY direct opcode
  EXPECT_EQ(0x40, bytes[2]);
}

TEST_F(Cpu6809Test, LDY_Extended_BigEndian) {
  auto bytes = cpu.EncodeLDY(0x4000, AddressingMode6809::Extended);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xBE, bytes[1]); // LDY extended opcode
  EXPECT_EQ(0x40, bytes[2]); // High byte
  EXPECT_EQ(0x00, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, STX_Direct) {
  auto bytes = cpu.EncodeSTX(0x50, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x9F, bytes[0]); // STX direct opcode
  EXPECT_EQ(0x50, bytes[1]);
}

TEST_F(Cpu6809Test, STX_Extended_BigEndian) {
  auto bytes = cpu.EncodeSTX(0x5000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xBF, bytes[0]); // STX extended opcode
  EXPECT_EQ(0x50, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, STY_Direct) {
  auto bytes = cpu.EncodeSTY(0x60, AddressingMode6809::Direct);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x9F, bytes[1]); // STY direct opcode
  EXPECT_EQ(0x60, bytes[2]);
}

TEST_F(Cpu6809Test, STY_Extended_BigEndian) {
  auto bytes = cpu.EncodeSTY(0x6000, AddressingMode6809::Extended);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0xBF, bytes[1]); // STY extended opcode
  EXPECT_EQ(0x60, bytes[2]); // High byte
  EXPECT_EQ(0x00, bytes[3]); // Low byte
}

// ============================================================================
// Phase 6: Arithmetic Instructions (ADDA, ADDB, SUBA, SUBB)
// ============================================================================

TEST_F(Cpu6809Test, ADDA_Immediate8) {
  auto bytes = cpu.EncodeADDA(0x42, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x8B, bytes[0]); // ADDA immediate opcode
  EXPECT_EQ(0x42, bytes[1]);
}

TEST_F(Cpu6809Test, ADDA_Direct) {
  auto bytes = cpu.EncodeADDA(0x80, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x9B, bytes[0]); // ADDA direct opcode
  EXPECT_EQ(0x80, bytes[1]);
}

TEST_F(Cpu6809Test, ADDA_Extended) {
  auto bytes = cpu.EncodeADDA(0x1234, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xBB, bytes[0]); // ADDA extended opcode
  EXPECT_EQ(0x12, bytes[1]); // High byte
  EXPECT_EQ(0x34, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, ADDB_Immediate8) {
  auto bytes = cpu.EncodeADDB(0x55, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xCB, bytes[0]); // ADDB immediate opcode
  EXPECT_EQ(0x55, bytes[1]);
}

TEST_F(Cpu6809Test, ADDB_Direct) {
  auto bytes = cpu.EncodeADDB(0x90, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xDB, bytes[0]); // ADDB direct opcode
  EXPECT_EQ(0x90, bytes[1]);
}

TEST_F(Cpu6809Test, SUBA_Immediate8) {
  auto bytes = cpu.EncodeSUBA(0x10, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x80, bytes[0]); // SUBA immediate opcode
  EXPECT_EQ(0x10, bytes[1]);
}

TEST_F(Cpu6809Test, SUBA_Direct) {
  auto bytes = cpu.EncodeSUBA(0x20, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x90, bytes[0]); // SUBA direct opcode
  EXPECT_EQ(0x20, bytes[1]);
}

TEST_F(Cpu6809Test, SUBB_Immediate8) {
  auto bytes = cpu.EncodeSUBB(0x30, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC0, bytes[0]); // SUBB immediate opcode
  EXPECT_EQ(0x30, bytes[1]);
}

TEST_F(Cpu6809Test, SUBB_Direct) {
  auto bytes = cpu.EncodeSUBB(0x40, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xD0, bytes[0]); // SUBB direct opcode
  EXPECT_EQ(0x40, bytes[1]);
}

// ============================================================================
// Phase 7: Branch Instructions (Relative Addressing)
// ============================================================================

TEST_F(Cpu6809Test, BRA_Relative8_Forward) {
  // BRA with 8-bit offset (forward branch)
  auto bytes = cpu.EncodeBRA(10, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x20, bytes[0]); // BRA opcode
  EXPECT_EQ(10, bytes[1]);   // Offset
}

TEST_F(Cpu6809Test, BRA_Relative8_Backward) {
  // BRA with negative offset (backward branch)
  auto bytes = cpu.EncodeBRA(-10, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x20, bytes[0]); // BRA opcode
  EXPECT_EQ(246, bytes[1]);  // -10 as unsigned byte (two's complement)
}

TEST_F(Cpu6809Test, BEQ_Relative8) {
  auto bytes = cpu.EncodeBEQ(5, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x27, bytes[0]); // BEQ opcode
  EXPECT_EQ(5, bytes[1]);
}

TEST_F(Cpu6809Test, BNE_Relative8) {
  auto bytes = cpu.EncodeBNE(-5, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x26, bytes[0]); // BNE opcode
  EXPECT_EQ(251, bytes[1]);  // -5 as unsigned byte
}

TEST_F(Cpu6809Test, BCC_Relative8) {
  auto bytes = cpu.EncodeBCC(20, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x24, bytes[0]); // BCC opcode
  EXPECT_EQ(20, bytes[1]);
}

TEST_F(Cpu6809Test, BCS_Relative8) {
  auto bytes = cpu.EncodeBCS(15, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x25, bytes[0]); // BCS opcode
  EXPECT_EQ(15, bytes[1]);
}

TEST_F(Cpu6809Test, BMI_Relative8) {
  auto bytes = cpu.EncodeBMI(-20, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2B, bytes[0]); // BMI opcode
  EXPECT_EQ(236, bytes[1]);  // -20 as unsigned byte
}

TEST_F(Cpu6809Test, BPL_Relative8) {
  auto bytes = cpu.EncodeBPL(8, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2A, bytes[0]); // BPL opcode
  EXPECT_EQ(8, bytes[1]);
}

TEST_F(Cpu6809Test, BVS_Relative8) {
  auto bytes = cpu.EncodeBVS(12, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x29, bytes[0]); // BVS opcode
  EXPECT_EQ(12, bytes[1]);
}

TEST_F(Cpu6809Test, BVC_Relative8) {
  auto bytes = cpu.EncodeBVC(25, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x28, bytes[0]); // BVC opcode
  EXPECT_EQ(25, bytes[1]);
}

TEST_F(Cpu6809Test, BGE_Relative8) {
  auto bytes = cpu.EncodeBGE(7, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2C, bytes[0]); // BGE opcode
  EXPECT_EQ(7, bytes[1]);
}

TEST_F(Cpu6809Test, BLT_Relative8) {
  auto bytes = cpu.EncodeBLT(-8, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2D, bytes[0]); // BLT opcode
  EXPECT_EQ(248, bytes[1]);  // -8 as unsigned byte
}

TEST_F(Cpu6809Test, BGT_Relative8) {
  auto bytes = cpu.EncodeBGT(10, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2E, bytes[0]); // BGT opcode
  EXPECT_EQ(10, bytes[1]);
}

TEST_F(Cpu6809Test, BLE_Relative8) {
  auto bytes = cpu.EncodeBLE(-12, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x2F, bytes[0]); // BLE opcode
  EXPECT_EQ(244, bytes[1]);  // -12 as unsigned byte
}

TEST_F(Cpu6809Test, BHI_Relative8) {
  auto bytes = cpu.EncodeBHI(6, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x22, bytes[0]); // BHI opcode
  EXPECT_EQ(6, bytes[1]);
}

TEST_F(Cpu6809Test, BLS_Relative8) {
  auto bytes = cpu.EncodeBLS(-7, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x23, bytes[0]); // BLS opcode
  EXPECT_EQ(249, bytes[1]);  // -7 as unsigned byte
}

TEST_F(Cpu6809Test, BSR_Relative8) {
  // Branch to Subroutine
  auto bytes = cpu.EncodeBSR(30, AddressingMode6809::Relative8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x8D, bytes[0]); // BSR opcode
  EXPECT_EQ(30, bytes[1]);
}

// ============================================================================
// Phase 8: Stack Operations (PSHS, PULS, PSHU, PULU)
// ============================================================================

TEST_F(Cpu6809Test, PSHS_SingleRegister_A) {
  // Push A to system stack
  uint8_t mask = 0x02; // Bit 1 = A register
  auto bytes = cpu.EncodePSHS(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x34, bytes[0]); // PSHS opcode
  EXPECT_EQ(0x02, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PSHS_MultipleRegisters) {
  // Push A, B, X, PC (0x02 | 0x04 | 0x10 | 0x80 = 0x96)
  uint8_t mask = 0x96;
  auto bytes = cpu.EncodePSHS(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x34, bytes[0]); // PSHS opcode
  EXPECT_EQ(0x96, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PULS_SingleRegister_B) {
  // Pull B from system stack
  uint8_t mask = 0x04; // Bit 2 = B register
  auto bytes = cpu.EncodePULS(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x35, bytes[0]); // PULS opcode
  EXPECT_EQ(0x04, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PULS_MultipleRegisters) {
  // Pull A, B, X, PC
  uint8_t mask = 0x96;
  auto bytes = cpu.EncodePULS(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x35, bytes[0]); // PULS opcode
  EXPECT_EQ(0x96, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PSHU_SingleRegister_X) {
  // Push X to user stack
  uint8_t mask = 0x10; // Bit 4 = X register
  auto bytes = cpu.EncodePSHU(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x36, bytes[0]); // PSHU opcode
  EXPECT_EQ(0x10, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PSHU_MultipleRegisters) {
  // Push D, Y (0x06 | 0x20 = 0x26)
  uint8_t mask = 0x26;
  auto bytes = cpu.EncodePSHU(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x36, bytes[0]); // PSHU opcode
  EXPECT_EQ(0x26, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PULU_SingleRegister_Y) {
  // Pull Y from user stack
  uint8_t mask = 0x20; // Bit 5 = Y register
  auto bytes = cpu.EncodePULU(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x37, bytes[0]); // PULU opcode
  EXPECT_EQ(0x20, bytes[1]); // Post-byte mask
}

TEST_F(Cpu6809Test, PULU_MultipleRegisters) {
  // Pull D, Y
  uint8_t mask = 0x26;
  auto bytes = cpu.EncodePULU(mask);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x37, bytes[0]); // PULU opcode
  EXPECT_EQ(0x26, bytes[1]); // Post-byte mask
}

// ============================================================================
// Phase 9: Register Transfer and Exchange (TFR, EXG)
// ============================================================================

TEST_F(Cpu6809Test, TFR_A_to_B) {
  // Transfer A to B (src=8, dst=9 -> post-byte=0x89)
  auto bytes = cpu.EncodeTFR(8, 9);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1F, bytes[0]); // TFR opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: A(8) -> B(9)
}

TEST_F(Cpu6809Test, TFR_X_to_Y) {
  // Transfer X to Y (src=1, dst=2 -> post-byte=0x12)
  auto bytes = cpu.EncodeTFR(1, 2);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1F, bytes[0]); // TFR opcode
  EXPECT_EQ(0x12, bytes[1]); // Post-byte: X(1) -> Y(2)
}

TEST_F(Cpu6809Test, TFR_D_to_X) {
  // Transfer D to X (src=0, dst=1 -> post-byte=0x01)
  auto bytes = cpu.EncodeTFR(0, 1);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1F, bytes[0]); // TFR opcode
  EXPECT_EQ(0x01, bytes[1]); // Post-byte: D(0) -> X(1)
}

TEST_F(Cpu6809Test, EXG_A_and_B) {
  // Exchange A and B (reg1=8, reg2=9 -> post-byte=0x89)
  auto bytes = cpu.EncodeEXG(8, 9);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1E, bytes[0]); // EXG opcode
  EXPECT_EQ(0x89, bytes[1]); // Post-byte: A(8) <-> B(9)
}

TEST_F(Cpu6809Test, EXG_X_and_Y) {
  // Exchange X and Y (reg1=1, reg2=2 -> post-byte=0x12)
  auto bytes = cpu.EncodeEXG(1, 2);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1E, bytes[0]); // EXG opcode
  EXPECT_EQ(0x12, bytes[1]); // Post-byte: X(1) <-> Y(2)
}

TEST_F(Cpu6809Test, EXG_D_and_U) {
  // Exchange D and U (reg1=0, reg2=3 -> post-byte=0x03)
  auto bytes = cpu.EncodeEXG(0, 3);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x1E, bytes[0]); // EXG opcode
  EXPECT_EQ(0x03, bytes[1]); // Post-byte: D(0) <-> U(3)
}

// ============================================================================
// Phase 10: Comparison Instructions (CMPA, CMPB, CMPX, CMPY)
// ============================================================================

TEST_F(Cpu6809Test, CMPA_Immediate8) {
  auto bytes = cpu.EncodeCMPA(0x42, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x81, bytes[0]); // CMPA immediate opcode
  EXPECT_EQ(0x42, bytes[1]);
}

TEST_F(Cpu6809Test, CMPA_Direct) {
  auto bytes = cpu.EncodeCMPA(0x80, AddressingMode6809::Direct);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x91, bytes[0]); // CMPA direct opcode
  EXPECT_EQ(0x80, bytes[1]);
}

TEST_F(Cpu6809Test, CMPB_Immediate8) {
  auto bytes = cpu.EncodeCMPB(0x55, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC1, bytes[0]); // CMPB immediate opcode
  EXPECT_EQ(0x55, bytes[1]);
}

TEST_F(Cpu6809Test, CMPX_Immediate16) {
  auto bytes = cpu.EncodeCMPX(0x1234, AddressingMode6809::Immediate16);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x8C, bytes[0]); // CMPX immediate opcode
  EXPECT_EQ(0x12, bytes[1]); // High byte
  EXPECT_EQ(0x34, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, CMPY_Immediate16) {
  // CMPY uses page 2 prefix
  auto bytes = cpu.EncodeCMPY(0x5678, AddressingMode6809::Immediate16);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x8C, bytes[1]); // CMPY immediate opcode
  EXPECT_EQ(0x56, bytes[2]); // High byte
  EXPECT_EQ(0x78, bytes[3]); // Low byte
}

// ============================================================================
// Phase 11: Logical Operations (ANDA, ANDB, ORA, ORB, EORA, EORB)
// ============================================================================

TEST_F(Cpu6809Test, ANDA_Immediate8) {
  auto bytes = cpu.EncodeANDA(0xFF, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x84, bytes[0]); // ANDA immediate opcode
  EXPECT_EQ(0xFF, bytes[1]);
}

TEST_F(Cpu6809Test, ANDB_Immediate8) {
  auto bytes = cpu.EncodeANDB(0x0F, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC4, bytes[0]); // ANDB immediate opcode
  EXPECT_EQ(0x0F, bytes[1]);
}

TEST_F(Cpu6809Test, ORA_Immediate8) {
  auto bytes = cpu.EncodeORA(0x80, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x8A, bytes[0]); // ORA immediate opcode
  EXPECT_EQ(0x80, bytes[1]);
}

TEST_F(Cpu6809Test, ORB_Immediate8) {
  auto bytes = cpu.EncodeORB(0x01, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xCA, bytes[0]); // ORB immediate opcode
  EXPECT_EQ(0x01, bytes[1]);
}

TEST_F(Cpu6809Test, EORA_Immediate8) {
  auto bytes = cpu.EncodeEORA(0xAA, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x88, bytes[0]); // EORA immediate opcode
  EXPECT_EQ(0xAA, bytes[1]);
}

TEST_F(Cpu6809Test, EORB_Immediate8) {
  auto bytes = cpu.EncodeEORB(0x55, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC8, bytes[0]); // EORB immediate opcode
  EXPECT_EQ(0x55, bytes[1]);
}

// ============================================================================
// Phase 12: Additional Instructions (JSR, JMP, LEA)
// ============================================================================

TEST_F(Cpu6809Test, JSR_Extended) {
  auto bytes = cpu.EncodeJSR(0x8000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0xBD, bytes[0]); // JSR extended opcode
  EXPECT_EQ(0x80, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, JMP_Extended) {
  auto bytes = cpu.EncodeJMP(0x9000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x7E, bytes[0]); // JMP extended opcode
  EXPECT_EQ(0x90, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LEAX_Extended) {
  auto bytes = cpu.EncodeLEAX(0x1000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x30, bytes[0]); // LEAX opcode
  EXPECT_EQ(0x10, bytes[1]); // High byte (note: LEA uses indexed mode encoding)
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

TEST_F(Cpu6809Test, LEAY_Extended) {
  auto bytes = cpu.EncodeLEAY(0x2000, AddressingMode6809::Extended);
  ASSERT_EQ(3, bytes.size());
  EXPECT_EQ(0x31, bytes[0]); // LEAY opcode
  EXPECT_EQ(0x20, bytes[1]); // High byte
  EXPECT_EQ(0x00, bytes[2]); // Low byte
}

// ============================================================================
// Phase 13: Bit Test and Shift/Rotate Instructions
// ============================================================================

TEST_F(Cpu6809Test, BITA_Immediate8) {
  auto bytes = cpu.EncodeBITA(0xF0, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0x85, bytes[0]); // BITA immediate opcode
  EXPECT_EQ(0xF0, bytes[1]);
}

TEST_F(Cpu6809Test, BITB_Immediate8) {
  auto bytes = cpu.EncodeBITB(0x0F, AddressingMode6809::Immediate8);
  ASSERT_EQ(2, bytes.size());
  EXPECT_EQ(0xC5, bytes[0]); // BITB immediate opcode
  EXPECT_EQ(0x0F, bytes[1]);
}

TEST_F(Cpu6809Test, ASLA_Inherent) {
  auto bytes = cpu.EncodeASLA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x48, bytes[0]); // ASLA opcode
}

TEST_F(Cpu6809Test, ASLB_Inherent) {
  auto bytes = cpu.EncodeASLB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x58, bytes[0]); // ASLB opcode
}

TEST_F(Cpu6809Test, ASRA_Inherent) {
  auto bytes = cpu.EncodeASRA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x47, bytes[0]); // ASRA opcode
}

TEST_F(Cpu6809Test, ASRB_Inherent) {
  auto bytes = cpu.EncodeASRB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x57, bytes[0]); // ASRB opcode
}

TEST_F(Cpu6809Test, LSRA_Inherent) {
  auto bytes = cpu.EncodeLSRA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x44, bytes[0]); // LSRA opcode
}

TEST_F(Cpu6809Test, LSRB_Inherent) {
  auto bytes = cpu.EncodeLSRB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x54, bytes[0]); // LSRB opcode
}

TEST_F(Cpu6809Test, ROLA_Inherent) {
  auto bytes = cpu.EncodeROLA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x49, bytes[0]); // ROLA opcode
}

TEST_F(Cpu6809Test, ROLB_Inherent) {
  auto bytes = cpu.EncodeROLB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x59, bytes[0]); // ROLB opcode
}

TEST_F(Cpu6809Test, RORA_Inherent) {
  auto bytes = cpu.EncodeRORA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x46, bytes[0]); // RORA opcode
}

TEST_F(Cpu6809Test, RORB_Inherent) {
  auto bytes = cpu.EncodeRORB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x56, bytes[0]); // RORB opcode
}

// ============================================================================
// Phase 14: More Inherent Mode Instructions
// ============================================================================

TEST_F(Cpu6809Test, INCA_Inherent) {
  auto bytes = cpu.EncodeINCA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x4C, bytes[0]); // INCA opcode
}

TEST_F(Cpu6809Test, INCB_Inherent) {
  auto bytes = cpu.EncodeINCB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x5C, bytes[0]); // INCB opcode
}

TEST_F(Cpu6809Test, DECA_Inherent) {
  auto bytes = cpu.EncodeDECA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x4A, bytes[0]); // DECA opcode
}

TEST_F(Cpu6809Test, DECB_Inherent) {
  auto bytes = cpu.EncodeDECB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x5A, bytes[0]); // DECB opcode
}

TEST_F(Cpu6809Test, TSTB_Inherent) {
  auto bytes = cpu.EncodeTSTB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x5D, bytes[0]); // TSTB opcode
}

TEST_F(Cpu6809Test, TSTA_Inherent) {
  auto bytes = cpu.EncodeTSTA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x4D, bytes[0]); // TSTA opcode
}

TEST_F(Cpu6809Test, COMA_Inherent) {
  auto bytes = cpu.EncodeCOMA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x43, bytes[0]); // COMA opcode
}

TEST_F(Cpu6809Test, COMB_Inherent) {
  auto bytes = cpu.EncodeCOMB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x53, bytes[0]); // COMB opcode
}

TEST_F(Cpu6809Test, NEGA_Inherent) {
  auto bytes = cpu.EncodeNEGA();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x40, bytes[0]); // NEGA opcode
}

TEST_F(Cpu6809Test, NEGB_Inherent) {
  auto bytes = cpu.EncodeNEGB();
  ASSERT_EQ(1, bytes.size());
  EXPECT_EQ(0x50, bytes[0]); // NEGB opcode
}

// ============================================================================
// Phase 2B: Long Branch Instructions (16-bit relative)
// ============================================================================

TEST_F(Cpu6809Test, LBRA_Relative16_Forward) {
  // LBRA with 16-bit offset (forward branch)
  auto bytes = cpu.EncodeLBRA(1000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x16, bytes[1]); // LBRA opcode
  EXPECT_EQ(0x03, bytes[2]); // High byte of 1000 (0x03E8)
  EXPECT_EQ(0xE8, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBRA_Relative16_Backward) {
  // LBRA with negative offset (backward branch)
  auto bytes = cpu.EncodeLBRA(-1000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x16, bytes[1]); // LBRA opcode
  EXPECT_EQ(0xFC, bytes[2]); // High byte of -1000 (0xFC18)
  EXPECT_EQ(0x18, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBRA_Relative16_MaxPositive) {
  // LBRA with maximum positive offset (+32767)
  auto bytes = cpu.EncodeLBRA(32767);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]);
  EXPECT_EQ(0x16, bytes[1]);
  EXPECT_EQ(0x7F, bytes[2]); // High byte of 32767 (0x7FFF)
  EXPECT_EQ(0xFF, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBRA_Relative16_MaxNegative) {
  // LBRA with maximum negative offset (-32768)
  auto bytes = cpu.EncodeLBRA(-32768);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]);
  EXPECT_EQ(0x16, bytes[1]);
  EXPECT_EQ(0x80, bytes[2]); // High byte of -32768 (0x8000)
  EXPECT_EQ(0x00, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBRN_Relative16) {
  // LBRN (Long Branch Never) - always takes 4 bytes but never branches
  auto bytes = cpu.EncodeLBRN(100);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x21, bytes[1]); // LBRN opcode
  EXPECT_EQ(0x00, bytes[2]); // High byte of 100
  EXPECT_EQ(0x64, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBSR_Relative16_Forward) {
  // LBSR (Long Branch to Subroutine) with 16-bit offset (forward branch)
  auto bytes = cpu.EncodeLBSR(1000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x17, bytes[1]); // LBSR opcode
  EXPECT_EQ(0x03, bytes[2]); // High byte of 1000 (0x03E8)
  EXPECT_EQ(0xE8, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBSR_Relative16_Backward) {
  // LBSR with negative offset (backward branch)
  auto bytes = cpu.EncodeLBSR(-1000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x17, bytes[1]); // LBSR opcode
  EXPECT_EQ(0xFC, bytes[2]); // High byte of -1000 (0xFC18)
  EXPECT_EQ(0x18, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBSR_Relative16_MaxPositive) {
  // LBSR with maximum positive offset (+32767)
  auto bytes = cpu.EncodeLBSR(32767);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x17, bytes[1]); // LBSR opcode
  EXPECT_EQ(0x7F, bytes[2]); // High byte of 32767 (0x7FFF)
  EXPECT_EQ(0xFF, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBSR_Relative16_MaxNegative) {
  // LBSR with maximum negative offset (-32768)
  auto bytes = cpu.EncodeLBSR(-32768);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x17, bytes[1]); // LBSR opcode
  EXPECT_EQ(0x80, bytes[2]); // High byte of -32768 (0x8000)
  EXPECT_EQ(0x00, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBHI_Relative16) {
  // LBHI (Long Branch if Higher - unsigned)
  auto bytes = cpu.EncodeLBHI(500);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x22, bytes[1]); // LBHI opcode
  EXPECT_EQ(0x01, bytes[2]); // High byte of 500 (0x01F4)
  EXPECT_EQ(0xF4, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBLS_Relative16) {
  // LBLS (Long Branch if Lower or Same - unsigned)
  auto bytes = cpu.EncodeLBLS(-250);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x23, bytes[1]); // LBLS opcode
  EXPECT_EQ(0xFF, bytes[2]); // High byte of -250 (0xFF06)
  EXPECT_EQ(0x06, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBCC_Relative16) {
  // LBCC/LBHS (Long Branch if Carry Clear)
  auto bytes = cpu.EncodeLBCC(2000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x24, bytes[1]); // LBCC opcode
  EXPECT_EQ(0x07, bytes[2]); // High byte of 2000 (0x07D0)
  EXPECT_EQ(0xD0, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBCS_Relative16) {
  // LBCS/LBLO (Long Branch if Carry Set)
  auto bytes = cpu.EncodeLBCS(-2000);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x25, bytes[1]); // LBCS opcode
  EXPECT_EQ(0xF8, bytes[2]); // High byte of -2000 (0xF830)
  EXPECT_EQ(0x30, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBNE_Relative16) {
  // LBNE (Long Branch if Not Equal)
  auto bytes = cpu.EncodeLBNE(300);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x26, bytes[1]); // LBNE opcode
  EXPECT_EQ(0x01, bytes[2]); // High byte of 300 (0x012C)
  EXPECT_EQ(0x2C, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBEQ_Relative16) {
  // LBEQ (Long Branch if Equal)
  auto bytes = cpu.EncodeLBEQ(-300);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x27, bytes[1]); // LBEQ opcode
  EXPECT_EQ(0xFE, bytes[2]); // High byte of -300 (0xFED4)
  EXPECT_EQ(0xD4, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBVC_Relative16) {
  // LBVC (Long Branch if Overflow Clear)
  auto bytes = cpu.EncodeLBVC(150);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x28, bytes[1]); // LBVC opcode
  EXPECT_EQ(0x00, bytes[2]); // High byte of 150 (0x0096)
  EXPECT_EQ(0x96, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBVS_Relative16) {
  // LBVS (Long Branch if Overflow Set)
  auto bytes = cpu.EncodeLBVS(-150);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x29, bytes[1]); // LBVS opcode
  EXPECT_EQ(0xFF, bytes[2]); // High byte of -150 (0xFF6A)
  EXPECT_EQ(0x6A, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBPL_Relative16) {
  // LBPL (Long Branch if Plus)
  auto bytes = cpu.EncodeLBPL(800);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2A, bytes[1]); // LBPL opcode
  EXPECT_EQ(0x03, bytes[2]); // High byte of 800 (0x0320)
  EXPECT_EQ(0x20, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBMI_Relative16) {
  // LBMI (Long Branch if Minus)
  auto bytes = cpu.EncodeLBMI(-800);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2B, bytes[1]); // LBMI opcode
  EXPECT_EQ(0xFC, bytes[2]); // High byte of -800 (0xFCE0)
  EXPECT_EQ(0xE0, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBGE_Relative16) {
  // LBGE (Long Branch if Greater or Equal - signed)
  auto bytes = cpu.EncodeLBGE(600);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2C, bytes[1]); // LBGE opcode
  EXPECT_EQ(0x02, bytes[2]); // High byte of 600 (0x0258)
  EXPECT_EQ(0x58, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBLT_Relative16) {
  // LBLT (Long Branch if Less Than - signed)
  auto bytes = cpu.EncodeLBLT(-600);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2D, bytes[1]); // LBLT opcode
  EXPECT_EQ(0xFD, bytes[2]); // High byte of -600 (0xFDA8)
  EXPECT_EQ(0xA8, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBGT_Relative16) {
  // LBGT (Long Branch if Greater Than - signed)
  auto bytes = cpu.EncodeLBGT(400);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2E, bytes[1]); // LBGT opcode
  EXPECT_EQ(0x01, bytes[2]); // High byte of 400 (0x0190)
  EXPECT_EQ(0x90, bytes[3]); // Low byte
}

TEST_F(Cpu6809Test, LBLE_Relative16) {
  // LBLE (Long Branch if Less or Equal - signed)
  auto bytes = cpu.EncodeLBLE(-400);
  ASSERT_EQ(4, bytes.size());
  EXPECT_EQ(0x10, bytes[0]); // Page 2 prefix
  EXPECT_EQ(0x2F, bytes[1]); // LBLE opcode
  EXPECT_EQ(0xFE, bytes[2]); // High byte of -400 (0xFE70)
  EXPECT_EQ(0x70, bytes[3]); // Low byte
}
