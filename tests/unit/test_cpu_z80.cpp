/**
 * @file test_cpu_z80.cpp
 * @brief Unit tests for Zilog Z80 CPU plugin
 *
 * Tests instruction encoding for the Z80 processor following TDD:
 * RED -> GREEN -> REFACTOR cycle.
 *
 * Z80 Features:
 * - ~150 opcodes (8080 base + Z80 extensions)
 * - IX/IY index registers
 * - Bit manipulation instructions (BIT, SET, RES)
 * - Block instructions (LDIR, CPIR, etc.)
 * - Prefix bytes: 0xCB (bit ops), 0xDD (IX), 0xED (extended), 0xFD (IY)
 */

#include "xasm++/cpu/cpu_z80.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Test Fixture
// ============================================================================

class CpuZ80Test : public ::testing::Test {
protected:
  CpuZ80 cpu;
};

// ============================================================================
// Phase 1: Foundation Tests
// ============================================================================

TEST_F(CpuZ80Test, GetName_ReturnsZ80) { EXPECT_EQ("Z80", cpu.GetName()); }

TEST_F(CpuZ80Test, GetCpuFamily_ReturnsZ80) {
  EXPECT_EQ("Z80", cpu.GetCpuFamily());
}

// ============================================================================
// Phase 2: Basic Instructions - 8-bit Loads
// ============================================================================

TEST_F(CpuZ80Test, LD_A_n_ImmediateMode) {
  // LD A, n -> 0x3E nn
  auto bytes = cpu.EncodeLD_A_n(0x42);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x3E, bytes[0]); // LD A, n opcode
  EXPECT_EQ(0x42, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_B_n_ImmediateMode) {
  // LD B, n -> 0x06 nn
  auto bytes = cpu.EncodeLD_B_n(0x55);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x06, bytes[0]); // LD B, n opcode
  EXPECT_EQ(0x55, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_C_n_ImmediateMode) {
  // LD C, n -> 0x0E nn
  auto bytes = cpu.EncodeLD_C_n(0xAA);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x0E, bytes[0]); // LD C, n opcode
  EXPECT_EQ(0xAA, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_D_n_ImmediateMode) {
  // LD D, n -> 0x16 nn
  auto bytes = cpu.EncodeLD_D_n(0x77);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x16, bytes[0]); // LD D, n opcode
  EXPECT_EQ(0x77, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_E_n_ImmediateMode) {
  // LD E, n -> 0x1E nn
  auto bytes = cpu.EncodeLD_E_n(0x88);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x1E, bytes[0]); // LD E, n opcode
  EXPECT_EQ(0x88, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_H_n_ImmediateMode) {
  // LD H, n -> 0x26 nn
  auto bytes = cpu.EncodeLD_H_n(0x99);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x26, bytes[0]); // LD H, n opcode
  EXPECT_EQ(0x99, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_L_n_ImmediateMode) {
  // LD L, n -> 0x2E nn
  auto bytes = cpu.EncodeLD_L_n(0xBB);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x2E, bytes[0]); // LD L, n opcode
  EXPECT_EQ(0xBB, bytes[1]); // Immediate value
}

// ============================================================================
// Phase 3: 16-bit Loads
// ============================================================================

TEST_F(CpuZ80Test, LD_BC_nn_Immediate16Bit) {
  // LD BC, nn -> 0x01 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_BC_nn(0x1234);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x01, bytes[0]); // LD BC, nn opcode
  EXPECT_EQ(0x34, bytes[1]); // Low byte (little-endian)
  EXPECT_EQ(0x12, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_DE_nn_Immediate16Bit) {
  // LD DE, nn -> 0x11 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_DE_nn(0xABCD);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x11, bytes[0]); // LD DE, nn opcode
  EXPECT_EQ(0xCD, bytes[1]); // Low byte
  EXPECT_EQ(0xAB, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_HL_nn_Immediate16Bit) {
  // LD HL, nn -> 0x21 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_HL_nn(0x8000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x21, bytes[0]); // LD HL, nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x80, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_SP_nn_Immediate16Bit) {
  // LD SP, nn -> 0x31 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_SP_nn(0xFFFF);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x31, bytes[0]); // LD SP, nn opcode
  EXPECT_EQ(0xFF, bytes[1]); // Low byte
  EXPECT_EQ(0xFF, bytes[2]); // High byte
}

// ============================================================================
// Phase 4: Basic Arithmetic
// ============================================================================

TEST_F(CpuZ80Test, ADD_A_n_ImmediateMode) {
  // ADD A, n -> 0xC6 nn
  auto bytes = cpu.EncodeADD_A_n(0x10);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xC6, bytes[0]); // ADD A, n opcode
  EXPECT_EQ(0x10, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, SUB_n_ImmediateMode) {
  // SUB n -> 0xD6 nn
  auto bytes = cpu.EncodeSUB_n(0x05);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xD6, bytes[0]); // SUB n opcode
  EXPECT_EQ(0x05, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, INC_A_Implied) {
  // INC A -> 0x3C
  auto bytes = cpu.EncodeINC_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x3C, bytes[0]); // INC A opcode
}

TEST_F(CpuZ80Test, DEC_A_Implied) {
  // DEC A -> 0x3D
  auto bytes = cpu.EncodeDEC_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x3D, bytes[0]); // DEC A opcode
}

// ============================================================================
// Phase 5: Control Flow
// ============================================================================

TEST_F(CpuZ80Test, NOP_Implied) {
  // NOP -> 0x00
  auto bytes = cpu.EncodeNOP();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x00, bytes[0]); // NOP opcode
}

TEST_F(CpuZ80Test, JP_nn_Absolute) {
  // JP nn -> 0xC3 nn nn (little-endian)
  auto bytes = cpu.EncodeJP_nn(0x8000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xC3, bytes[0]); // JP nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x80, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, RET_Implied) {
  // RET -> 0xC9
  auto bytes = cpu.EncodeRET();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC9, bytes[0]); // RET opcode
}

// ============================================================================
// Phase 6: Stack Operations
// ============================================================================

TEST_F(CpuZ80Test, PUSH_BC) {
  // PUSH BC -> 0xC5
  auto bytes = cpu.EncodePUSH_BC();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC5, bytes[0]); // PUSH BC opcode
}

TEST_F(CpuZ80Test, POP_BC) {
  // POP BC -> 0xC1
  auto bytes = cpu.EncodePOP_BC();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC1, bytes[0]); // POP BC opcode
}

// ============================================================================
// Phase 7: Bit Operations (CB prefix)
// ============================================================================

TEST_F(CpuZ80Test, BIT_0_A) {
  // BIT 0, A -> 0xCB 0x47
  auto bytes = cpu.EncodeBIT(0, 0x07); // Register A = 0x07
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xCB, bytes[0]); // CB prefix
  EXPECT_EQ(0x47, bytes[1]); // BIT 0, A opcode
}

TEST_F(CpuZ80Test, SET_7_A) {
  // SET 7, A -> 0xCB 0xFF
  auto bytes = cpu.EncodeSET(7, 0x07); // Register A = 0x07
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xCB, bytes[0]); // CB prefix
  EXPECT_EQ(0xFF, bytes[1]); // SET 7, A opcode
}

TEST_F(CpuZ80Test, RES_3_B) {
  // RES 3, B -> 0xCB 0x98
  auto bytes = cpu.EncodeRES(3, 0x00); // Register B = 0x00
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xCB, bytes[0]); // CB prefix
  EXPECT_EQ(0x98, bytes[1]); // RES 3, B opcode
}

// ============================================================================
// Phase 8: IX Register Operations (DD prefix)
// ============================================================================

TEST_F(CpuZ80Test, LD_IX_nn) {
  // LD IX, nn -> 0xDD 0x21 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_IX_nn(0x4000);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xDD, bytes[0]); // IX prefix
  EXPECT_EQ(0x21, bytes[1]); // LD HL, nn opcode (repurposed for IX)
  EXPECT_EQ(0x00, bytes[2]); // Low byte
  EXPECT_EQ(0x40, bytes[3]); // High byte
}

TEST_F(CpuZ80Test, LD_A_IX_d) {
  // LD A, (IX+d) -> 0xDD 0x7E dd
  auto bytes = cpu.EncodeLD_A_IX_d(0x10);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xDD, bytes[0]); // IX prefix
  EXPECT_EQ(0x7E, bytes[1]); // LD A, (HL) opcode (repurposed for IX)
  EXPECT_EQ(0x10, bytes[2]); // Displacement
}

// ============================================================================
// Phase 9: IY Register Operations (FD prefix)
// ============================================================================

TEST_F(CpuZ80Test, LD_IY_nn) {
  // LD IY, nn -> 0xFD 0x21 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_IY_nn(0x5000);
  ASSERT_EQ(bytes.size(), 4UL);
  EXPECT_EQ(0xFD, bytes[0]); // IY prefix
  EXPECT_EQ(0x21, bytes[1]); // LD HL, nn opcode (repurposed for IY)
  EXPECT_EQ(0x00, bytes[2]); // Low byte
  EXPECT_EQ(0x50, bytes[3]); // High byte
}

TEST_F(CpuZ80Test, LD_A_IY_d) {
  // LD A, (IY+d) -> 0xFD 0x7E dd
  auto bytes = cpu.EncodeLD_A_IY_d(0x20);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xFD, bytes[0]); // IY prefix
  EXPECT_EQ(0x7E, bytes[1]); // LD A, (HL) opcode (repurposed for IY)
  EXPECT_EQ(0x20, bytes[2]); // Displacement
}

// ============================================================================
// Phase 10: Register-to-Register Loads
// ============================================================================

TEST_F(CpuZ80Test, LD_A_B) {
  // LD A, B -> 0x78
  auto bytes = cpu.EncodeLD_A_B();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x78, bytes[0]); // LD A, B opcode
}

TEST_F(CpuZ80Test, LD_A_C) {
  // LD A, C -> 0x79
  auto bytes = cpu.EncodeLD_A_C();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x79, bytes[0]); // LD A, C opcode
}

TEST_F(CpuZ80Test, LD_B_A) {
  // LD B, A -> 0x47
  auto bytes = cpu.EncodeLD_B_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x47, bytes[0]); // LD B, A opcode
}

TEST_F(CpuZ80Test, LD_C_A) {
  // LD C, A -> 0x4F
  auto bytes = cpu.EncodeLD_C_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x4F, bytes[0]); // LD C, A opcode
}

// ============================================================================
// Phase 11: More Arithmetic Operations
// ============================================================================

TEST_F(CpuZ80Test, ADC_A_n) {
  // ADC A, n -> 0xCE nn
  auto bytes = cpu.EncodeADC_A_n(0x15);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xCE, bytes[0]); // ADC A, n opcode
  EXPECT_EQ(0x15, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, SBC_A_n) {
  // SBC A, n -> 0xDE nn
  auto bytes = cpu.EncodeSBC_A_n(0x08);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xDE, bytes[0]); // SBC A, n opcode
  EXPECT_EQ(0x08, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, CP_n) {
  // CP n -> 0xFE nn (compare immediate with A)
  auto bytes = cpu.EncodeCP_n(0x42);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xFE, bytes[0]); // CP n opcode
  EXPECT_EQ(0x42, bytes[1]); // Immediate value
}

// ============================================================================
// Phase 12: Logical Operations
// ============================================================================

TEST_F(CpuZ80Test, AND_n) {
  // AND n -> 0xE6 nn
  auto bytes = cpu.EncodeAND_n(0x0F);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xE6, bytes[0]); // AND n opcode
  EXPECT_EQ(0x0F, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, OR_n) {
  // OR n -> 0xF6 nn
  auto bytes = cpu.EncodeOR_n(0xF0);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xF6, bytes[0]); // OR n opcode
  EXPECT_EQ(0xF0, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, XOR_n) {
  // XOR n -> 0xEE nn
  auto bytes = cpu.EncodeXOR_n(0xFF);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xEE, bytes[0]); // XOR n opcode
  EXPECT_EQ(0xFF, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, XOR_A) {
  // XOR A -> 0xAF (common idiom to clear A)
  auto bytes = cpu.EncodeXOR_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xAF, bytes[0]); // XOR A opcode
}

// ============================================================================
// Phase 13: Branch Instructions
// ============================================================================

TEST_F(CpuZ80Test, JR_e) {
  // JR e -> 0x18 ee (relative jump)
  auto bytes = cpu.EncodeJR_e(0x10);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x18, bytes[0]); // JR e opcode
  EXPECT_EQ(0x10, bytes[1]); // Signed displacement
}

TEST_F(CpuZ80Test, JR_NZ_e) {
  // JR NZ, e -> 0x20 ee
  auto bytes = cpu.EncodeJR_NZ_e(0x05);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x20, bytes[0]); // JR NZ, e opcode
  EXPECT_EQ(0x05, bytes[1]); // Signed displacement
}

TEST_F(CpuZ80Test, JR_Z_e) {
  // JR Z, e -> 0x28 ee
  auto bytes = cpu.EncodeJR_Z_e(0x08);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x28, bytes[0]); // JR Z, e opcode
  EXPECT_EQ(0x08, bytes[1]); // Signed displacement
}

TEST_F(CpuZ80Test, JR_NC_e) {
  // JR NC, e -> 0x30 ee
  auto bytes = cpu.EncodeJR_NC_e(0x0C);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x30, bytes[0]); // JR NC, e opcode
  EXPECT_EQ(0x0C, bytes[1]); // Signed displacement
}

TEST_F(CpuZ80Test, JR_C_e) {
  // JR C, e -> 0x38 ee
  auto bytes = cpu.EncodeJR_C_e(static_cast<int8_t>(0xFC)); // Negative displacement (-4)
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x38, bytes[0]); // JR C, e opcode
  EXPECT_EQ(0xFC, bytes[1]); // Signed displacement (-4)
}

// ============================================================================
// Phase 14: Memory Access Instructions
// ============================================================================

TEST_F(CpuZ80Test, LD_A_addr) {
  // LD A, (nn) -> 0x3A nn nn (little-endian)
  auto bytes = cpu.EncodeLD_A_addr(0x8000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x3A, bytes[0]); // LD A, (nn) opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x80, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_addr_A) {
  // LD (nn), A -> 0x32 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_addr_A(0x9000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x32, bytes[0]); // LD (nn), A opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x90, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_A_HL) {
  // LD A, (HL) -> 0x7E
  auto bytes = cpu.EncodeLD_A_HL();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x7E, bytes[0]); // LD A, (HL) opcode
}

TEST_F(CpuZ80Test, LD_HL_A) {
  // LD (HL), A -> 0x77
  auto bytes = cpu.EncodeLD_HL_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x77, bytes[0]); // LD (HL), A opcode
}

// ============================================================================
// Phase 15: Rotate and Shift Instructions
// ============================================================================

TEST_F(CpuZ80Test, RLCA) {
  // RLCA -> 0x07 (rotate left circular accumulator)
  auto bytes = cpu.EncodeRLCA();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x07, bytes[0]); // RLCA opcode
}

TEST_F(CpuZ80Test, RRCA) {
  // RRCA -> 0x0F (rotate right circular accumulator)
  auto bytes = cpu.EncodeRRCA();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x0F, bytes[0]); // RRCA opcode
}

TEST_F(CpuZ80Test, RLA) {
  // RLA -> 0x17 (rotate left through carry)
  auto bytes = cpu.EncodeRLA();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x17, bytes[0]); // RLA opcode
}

TEST_F(CpuZ80Test, RRA) {
  // RRA -> 0x1F (rotate right through carry)
  auto bytes = cpu.EncodeRRA();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x1F, bytes[0]); // RRA opcode
}

// ============================================================================
// Phase 16: Extended Instructions (ED Prefix)
// ============================================================================

TEST_F(CpuZ80Test, LD_I_A) {
  // LD I, A -> 0xED 0x47 (load interrupt vector)
  auto bytes = cpu.EncodeLD_I_A();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0x47, bytes[1]); // LD I, A opcode
}

TEST_F(CpuZ80Test, LD_A_I) {
  // LD A, I -> 0xED 0x57
  auto bytes = cpu.EncodeLD_A_I();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0x57, bytes[1]); // LD A, I opcode
}

TEST_F(CpuZ80Test, LD_R_A) {
  // LD R, A -> 0xED 0x4F (load refresh register)
  auto bytes = cpu.EncodeLD_R_A();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0x4F, bytes[1]); // LD R, A opcode
}

TEST_F(CpuZ80Test, LD_A_R) {
  // LD A, R -> 0xED 0x5F
  auto bytes = cpu.EncodeLD_A_R();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0x5F, bytes[1]); // LD A, R opcode
}

TEST_F(CpuZ80Test, LDIR) {
  // LDIR -> 0xED 0xB0 (load, increment, repeat)
  auto bytes = cpu.EncodeLDIR();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0xB0, bytes[1]); // LDIR opcode
}

TEST_F(CpuZ80Test, CPIR) {
  // CPIR -> 0xED 0xB1 (compare, increment, repeat)
  auto bytes = cpu.EncodeCPIR();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0xB1, bytes[1]); // CPIR opcode
}

TEST_F(CpuZ80Test, NEG) {
  // NEG -> 0xED 0x44 (negate accumulator)
  auto bytes = cpu.EncodeNEG();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // ED prefix
  EXPECT_EQ(0x44, bytes[1]); // NEG opcode
}

// ============================================================================
// Phase 17: More Stack Operations
// ============================================================================

TEST_F(CpuZ80Test, PUSH_DE) {
  // PUSH DE -> 0xD5
  auto bytes = cpu.EncodePUSH_DE();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xD5, bytes[0]); // PUSH DE opcode
}

TEST_F(CpuZ80Test, PUSH_HL) {
  // PUSH HL -> 0xE5
  auto bytes = cpu.EncodePUSH_HL();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xE5, bytes[0]); // PUSH HL opcode
}

TEST_F(CpuZ80Test, PUSH_AF) {
  // PUSH AF -> 0xF5
  auto bytes = cpu.EncodePUSH_AF();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xF5, bytes[0]); // PUSH AF opcode
}

TEST_F(CpuZ80Test, POP_DE) {
  // POP DE -> 0xD1
  auto bytes = cpu.EncodePOP_DE();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xD1, bytes[0]); // POP DE opcode
}

TEST_F(CpuZ80Test, POP_HL) {
  // POP HL -> 0xE1
  auto bytes = cpu.EncodePOP_HL();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xE1, bytes[0]); // POP HL opcode
}

TEST_F(CpuZ80Test, POP_AF) {
  // POP AF -> 0xF1
  auto bytes = cpu.EncodePOP_AF();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xF1, bytes[0]); // POP AF opcode
}

// ============================================================================
// Phase 18: Conditional Instructions
// ============================================================================

TEST_F(CpuZ80Test, CALL_nn) {
  // CALL nn -> 0xCD nn nn (little-endian)
  auto bytes = cpu.EncodeCALL_nn(0x4000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xCD, bytes[0]); // CALL nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x40, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, CALL_NZ_nn) {
  // CALL NZ, nn -> 0xC4 nn nn
  auto bytes = cpu.EncodeCALL_NZ_nn(0x5000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xC4, bytes[0]); // CALL NZ, nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x50, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, RET_Z) {
  // RET Z -> 0xC8
  auto bytes = cpu.EncodeRET_Z();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC8, bytes[0]); // RET Z opcode
}

TEST_F(CpuZ80Test, RET_NZ) {
  // RET NZ -> 0xC0
  auto bytes = cpu.EncodeRET_NZ();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC0, bytes[0]); // RET NZ opcode
}

// ============================================================================
// Phase 19: Generic EncodeInstruction with Mnemonic Constants
// ============================================================================

TEST_F(CpuZ80Test, EncodeInstruction_LD_A_Immediate) {
  // Test LD A, n using mnemonic string "LD"
  auto bytes = cpu.EncodeInstruction("LD", 0x42, "A, #$42");
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x3E, bytes[0]); // LD A, n opcode
  EXPECT_EQ(0x42, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, EncodeInstruction_ADD_A_Immediate) {
  // Test ADD A, n using mnemonic string "ADD"
  auto bytes = cpu.EncodeInstruction("ADD", 0x10, "A, #$10");
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xC6, bytes[0]); // ADD A, n opcode
  EXPECT_EQ(0x10, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, EncodeInstruction_JP_Absolute) {
  // Test JP nn using mnemonic string "JP"
  auto bytes = cpu.EncodeInstruction("JP", 0x8000, "$8000");
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xC3, bytes[0]); // JP nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x80, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, EncodeInstruction_NOP) {
  // Test NOP using mnemonic string "NOP"
  auto bytes = cpu.EncodeInstruction("NOP", 0, "");
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x00, bytes[0]); // NOP opcode
}

TEST_F(CpuZ80Test, EncodeInstruction_SUB_Immediate) {
  // Test SUB n using mnemonic string "SUB"
  auto bytes = cpu.EncodeInstruction("SUB", 0x05, "#$05");
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xD6, bytes[0]); // SUB n opcode
  EXPECT_EQ(0x05, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, EncodeInstruction_INC_A) {
  // Test INC A using mnemonic string "INC"
  auto bytes = cpu.EncodeInstruction("INC", 0, "A");
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x3C, bytes[0]); // INC A opcode
}

TEST_F(CpuZ80Test, EncodeInstruction_DEC_A) {
  // Test DEC A using mnemonic string "DEC"
  auto bytes = cpu.EncodeInstruction("DEC", 0, "A");
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x3D, bytes[0]); // DEC A opcode
}

TEST_F(CpuZ80Test, EncodeInstruction_RET) {
  // Test RET using mnemonic string "RET"
  auto bytes = cpu.EncodeInstruction("RET", 0, "");
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xC9, bytes[0]); // RET opcode
}

TEST_F(CpuZ80Test, EncodeInstruction_JR_Relative) {
  // Test JR e using mnemonic string "JR"
  auto bytes = cpu.EncodeInstruction("JR", 0x10, "$10");
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x18, bytes[0]); // JR e opcode
  EXPECT_EQ(0x10, bytes[1]); // Signed displacement
}

TEST_F(CpuZ80Test, EncodeInstruction_LD_BC_Immediate) {
  // Test LD BC, nn using mnemonic string "LD"
  auto bytes = cpu.EncodeInstruction("LD", 0x1234, "BC, #$1234");
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x01, bytes[0]); // LD BC, nn opcode
  EXPECT_EQ(0x34, bytes[1]); // Low byte
  EXPECT_EQ(0x12, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, EncodeInstruction_XOR_A) {
  // Test XOR A using mnemonic string "XOR"
  auto bytes = cpu.EncodeInstruction("XOR", 0, "A");
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xAF, bytes[0]); // XOR A opcode
}

// ============================================================================
// Phase 20: Missing LD Variants
// ============================================================================

TEST_F(CpuZ80Test, LD_A_BC_IndirectLoad) {
  // LD A, (BC) -> 0x0A
  auto bytes = cpu.EncodeLD_A_BC();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x0A, bytes[0]); // LD A, (BC) opcode
}

TEST_F(CpuZ80Test, LD_A_DE_IndirectLoad) {
  // LD A, (DE) -> 0x1A
  auto bytes = cpu.EncodeLD_A_DE();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x1A, bytes[0]); // LD A, (DE) opcode
}

TEST_F(CpuZ80Test, LD_BC_A_IndirectStore) {
  // LD (BC), A -> 0x02
  auto bytes = cpu.EncodeLD_BC_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x02, bytes[0]); // LD (BC), A opcode
}

TEST_F(CpuZ80Test, LD_DE_A_IndirectStore) {
  // LD (DE), A -> 0x12
  auto bytes = cpu.EncodeLD_DE_A();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0x12, bytes[0]); // LD (DE), A opcode
}

TEST_F(CpuZ80Test, LD_HL_n_ImmediateToMemory) {
  // LD (HL), n -> 0x36 nn
  auto bytes = cpu.EncodeLD_HL_n(0x42);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x36, bytes[0]); // LD (HL), n opcode
  EXPECT_EQ(0x42, bytes[1]); // Immediate value
}

TEST_F(CpuZ80Test, LD_HL_addr_LoadFromMemory) {
  // LD HL, (nn) -> 0x2A nn nn (little-endian)
  auto bytes = cpu.EncodeLD_HL_addr(0x1234);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x2A, bytes[0]); // LD HL, (nn) opcode
  EXPECT_EQ(0x34, bytes[1]); // Low byte (little-endian)
  EXPECT_EQ(0x12, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_addr_HL_StoreToMemory) {
  // LD (nn), HL -> 0x22 nn nn (little-endian)
  auto bytes = cpu.EncodeLD_addr_HL(0xABCD);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x22, bytes[0]); // LD (nn), HL opcode
  EXPECT_EQ(0xCD, bytes[1]); // Low byte (little-endian)
  EXPECT_EQ(0xAB, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_SP_HL_LoadStackPointer) {
  // LD SP, HL -> 0xF9
  auto bytes = cpu.EncodeLD_SP_HL();
  ASSERT_EQ(bytes.size(), 1UL);
  EXPECT_EQ(0xF9, bytes[0]); // LD SP, HL opcode
}

// ============================================================================
// Phase 21: RLD/RRD Digit Rotate Instructions
// ============================================================================

TEST_F(CpuZ80Test, RLD_DigitRotateLeft) {
  // RLD -> 0xED 0x6F (rotate left digit)
  auto bytes = cpu.EncodeRLD();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended opcode prefix
  EXPECT_EQ(0x6F, bytes[1]); // RLD opcode
}

TEST_F(CpuZ80Test, RRD_DigitRotateRight) {
  // RRD -> 0xED 0x67 (rotate right digit)
  auto bytes = cpu.EncodeRRD();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended opcode prefix
  EXPECT_EQ(0x67, bytes[1]); // RRD opcode
}

// ============================================================================
// Phase 22: IN/OUT Instructions
// ============================================================================

TEST_F(CpuZ80Test, IN_A_n_ReadFromPort) {
  // IN A, (n) -> 0xDB n
  auto bytes = cpu.EncodeIN_A_n(0x42);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xDB, bytes[0]); // IN A, (n) opcode
  EXPECT_EQ(0x42, bytes[1]); // Port address
}

TEST_F(CpuZ80Test, OUT_n_A_WriteToPort) {
  // OUT (n), A -> 0xD3 n
  auto bytes = cpu.EncodeOUT_n_A(0x80);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xD3, bytes[0]); // OUT (n), A opcode
  EXPECT_EQ(0x80, bytes[1]); // Port address
}

TEST_F(CpuZ80Test, IN_B_C_ReadFromPortC) {
  // IN B, (C) -> 0xED 0x40
  auto bytes = cpu.EncodeIN_B_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x40, bytes[1]); // IN B, (C) opcode
}

TEST_F(CpuZ80Test, IN_C_C_ReadFromPortC) {
  // IN C, (C) -> 0xED 0x48
  auto bytes = cpu.EncodeIN_C_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x48, bytes[1]); // IN C, (C) opcode
}

TEST_F(CpuZ80Test, IN_D_C_ReadFromPortC) {
  // IN D, (C) -> 0xED 0x50
  auto bytes = cpu.EncodeIN_D_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x50, bytes[1]); // IN D, (C) opcode
}

TEST_F(CpuZ80Test, IN_E_C_ReadFromPortC) {
  // IN E, (C) -> 0xED 0x58
  auto bytes = cpu.EncodeIN_E_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x58, bytes[1]); // IN E, (C) opcode
}

TEST_F(CpuZ80Test, IN_H_C_ReadFromPortC) {
  // IN H, (C) -> 0xED 0x60
  auto bytes = cpu.EncodeIN_H_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x60, bytes[1]); // IN H, (C) opcode
}

TEST_F(CpuZ80Test, IN_L_C_ReadFromPortC) {
  // IN L, (C) -> 0xED 0x68
  auto bytes = cpu.EncodeIN_L_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x68, bytes[1]); // IN L, (C) opcode
}

TEST_F(CpuZ80Test, IN_A_C_ReadFromPortC) {
  // IN A, (C) -> 0xED 0x78
  auto bytes = cpu.EncodeIN_A_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x78, bytes[1]); // IN A, (C) opcode
}

TEST_F(CpuZ80Test, OUT_C_B_WriteToPortC) {
  // OUT (C), B -> 0xED 0x41
  auto bytes = cpu.EncodeOUT_C_B();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x41, bytes[1]); // OUT (C), B opcode
}

TEST_F(CpuZ80Test, OUT_C_C_WriteToPortC) {
  // OUT (C), C -> 0xED 0x49
  auto bytes = cpu.EncodeOUT_C_C();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x49, bytes[1]); // OUT (C), C opcode
}

TEST_F(CpuZ80Test, OUT_C_D_WriteToPortC) {
  // OUT (C), D -> 0xED 0x51
  auto bytes = cpu.EncodeOUT_C_D();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x51, bytes[1]); // OUT (C), D opcode
}

TEST_F(CpuZ80Test, OUT_C_E_WriteToPortC) {
  // OUT (C), E -> 0xED 0x59
  auto bytes = cpu.EncodeOUT_C_E();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x59, bytes[1]); // OUT (C), E opcode
}

TEST_F(CpuZ80Test, OUT_C_H_WriteToPortC) {
  // OUT (C), H -> 0xED 0x61
  auto bytes = cpu.EncodeOUT_C_H();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x61, bytes[1]); // OUT (C), H opcode
}

TEST_F(CpuZ80Test, OUT_C_L_WriteToPortC) {
  // OUT (C), L -> 0xED 0x69
  auto bytes = cpu.EncodeOUT_C_L();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x69, bytes[1]); // OUT (C), L opcode
}

TEST_F(CpuZ80Test, OUT_C_A_WriteToPortC) {
  // OUT (C), A -> 0xED 0x79
  auto bytes = cpu.EncodeOUT_C_A();
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xED, bytes[0]); // Extended prefix
  EXPECT_EQ(0x79, bytes[1]); // OUT (C), A opcode
}

// ============================================================================
// Phase 23: Edge Cases - Boundary Value Testing
// ============================================================================

TEST_F(CpuZ80Test, LD_A_n_EdgeCase_Zero) {
  // LD A, 0x00 -> 0x3E 0x00 (boundary: minimum value)
  auto bytes = cpu.EncodeLD_A_n(0x00);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x3E, bytes[0]); // LD A, n opcode
  EXPECT_EQ(0x00, bytes[1]); // Zero value
}

TEST_F(CpuZ80Test, LD_A_n_EdgeCase_Max) {
  // LD A, 0xFF -> 0x3E 0xFF (boundary: maximum value)
  auto bytes = cpu.EncodeLD_A_n(0xFF);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x3E, bytes[0]); // LD A, n opcode
  EXPECT_EQ(0xFF, bytes[1]); // Max value
}

TEST_F(CpuZ80Test, LD_BC_nn_EdgeCase_Zero) {
  // LD BC, 0x0000 -> 0x01 0x00 0x00 (boundary: minimum 16-bit value)
  auto bytes = cpu.EncodeLD_BC_nn(0x0000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x01, bytes[0]); // LD BC, nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x00, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, LD_BC_nn_EdgeCase_Max) {
  // LD BC, 0xFFFF -> 0x01 0xFF 0xFF (boundary: maximum 16-bit value)
  auto bytes = cpu.EncodeLD_BC_nn(0xFFFF);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0x01, bytes[0]); // LD BC, nn opcode
  EXPECT_EQ(0xFF, bytes[1]); // Low byte
  EXPECT_EQ(0xFF, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, ADD_A_n_EdgeCase_Zero) {
  // ADD A, 0x00 -> 0xC6 0x00 (edge case: adding zero)
  auto bytes = cpu.EncodeADD_A_n(0x00);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xC6, bytes[0]); // ADD A, n opcode
  EXPECT_EQ(0x00, bytes[1]); // Zero operand
}

TEST_F(CpuZ80Test, ADD_A_n_EdgeCase_Max) {
  // ADD A, 0xFF -> 0xC6 0xFF (edge case: maximum value, tests overflow)
  auto bytes = cpu.EncodeADD_A_n(0xFF);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0xC6, bytes[0]); // ADD A, n opcode
  EXPECT_EQ(0xFF, bytes[1]); // Max operand
}

TEST_F(CpuZ80Test, JP_nn_EdgeCase_Zero) {
  // JP 0x0000 -> 0xC3 0x00 0x00 (edge case: jump to address zero)
  auto bytes = cpu.EncodeJP_nn(0x0000);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xC3, bytes[0]); // JP nn opcode
  EXPECT_EQ(0x00, bytes[1]); // Low byte
  EXPECT_EQ(0x00, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, JP_nn_EdgeCase_Max) {
  // JP 0xFFFF -> 0xC3 0xFF 0xFF (edge case: maximum address)
  auto bytes = cpu.EncodeJP_nn(0xFFFF);
  ASSERT_EQ(bytes.size(), 3UL);
  EXPECT_EQ(0xC3, bytes[0]); // JP nn opcode
  EXPECT_EQ(0xFF, bytes[1]); // Low byte
  EXPECT_EQ(0xFF, bytes[2]); // High byte
}

TEST_F(CpuZ80Test, JR_e_EdgeCase_NegativeOffset) {
  // JR -1 -> 0x18 0xFF (edge case: negative displacement, backward jump)
  auto bytes = cpu.EncodeJR_e(static_cast<int8_t>(0xFF)); // -1 in two's complement
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x18, bytes[0]); // JR e opcode
  EXPECT_EQ(0xFF, bytes[1]); // -1 signed displacement
}

TEST_F(CpuZ80Test, JR_e_EdgeCase_MaxForward) {
  // JR +127 -> 0x18 0x7F (edge case: maximum forward displacement)
  auto bytes = cpu.EncodeJR_e(0x7F);
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x18, bytes[0]); // JR e opcode
  EXPECT_EQ(0x7F, bytes[1]); // Max positive displacement
}

TEST_F(CpuZ80Test, JR_e_EdgeCase_MaxBackward) {
  // JR -128 -> 0x18 0x80 (edge case: maximum backward displacement)
  auto bytes = cpu.EncodeJR_e(static_cast<int8_t>(0x80)); // -128 in two's complement
  ASSERT_EQ(bytes.size(), 2UL);
  EXPECT_EQ(0x18, bytes[0]); // JR e opcode
  EXPECT_EQ(0x80, bytes[1]); // Max negative displacement
}
