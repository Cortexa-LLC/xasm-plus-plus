/**
 * @file test_opcodes_z80.cpp
 * @brief Unit tests for Z80 opcode constants
 *
 * Tests verify that all Z80 opcode constants are defined with correct values
 * and properly organized in namespaces.
 */

#include "xasm++/cpu/opcodes_z80.h"
#include <gtest/gtest.h>

using namespace xasm::Z80Opcodes;

// ============================================================================
// Basic 8-bit Load Instructions
// ============================================================================

TEST(Z80OpcodesTest, LD_A_n) { EXPECT_EQ(0x3E, LD_A_n); }

TEST(Z80OpcodesTest, LD_B_n) { EXPECT_EQ(0x06, LD_B_n); }

TEST(Z80OpcodesTest, LD_C_n) { EXPECT_EQ(0x0E, LD_C_n); }

TEST(Z80OpcodesTest, LD_D_n) { EXPECT_EQ(0x16, LD_D_n); }

TEST(Z80OpcodesTest, LD_E_n) { EXPECT_EQ(0x1E, LD_E_n); }

TEST(Z80OpcodesTest, LD_H_n) { EXPECT_EQ(0x26, LD_H_n); }

TEST(Z80OpcodesTest, LD_L_n) { EXPECT_EQ(0x2E, LD_L_n); }

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

TEST(Z80OpcodesTest, LD_BC_nn) { EXPECT_EQ(0x01, LD_BC_nn); }

TEST(Z80OpcodesTest, LD_DE_nn) { EXPECT_EQ(0x11, LD_DE_nn); }

TEST(Z80OpcodesTest, LD_HL_nn) { EXPECT_EQ(0x21, LD_HL_nn); }

TEST(Z80OpcodesTest, LD_SP_nn) { EXPECT_EQ(0x31, LD_SP_nn); }

// ============================================================================
// Register-to-Register Load Instructions
// ============================================================================

TEST(Z80OpcodesTest, LD_A_B) { EXPECT_EQ(0x78, LD_A_B); }

TEST(Z80OpcodesTest, LD_A_C) { EXPECT_EQ(0x79, LD_A_C); }

TEST(Z80OpcodesTest, LD_B_A) { EXPECT_EQ(0x47, LD_B_A); }

TEST(Z80OpcodesTest, LD_C_A) { EXPECT_EQ(0x4F, LD_C_A); }

// ============================================================================
// Arithmetic Instructions
// ============================================================================

TEST(Z80OpcodesTest, ADD_A_n) { EXPECT_EQ(0xC6, ADD_A_n); }

TEST(Z80OpcodesTest, ADC_A_n) { EXPECT_EQ(0xCE, ADC_A_n); }

TEST(Z80OpcodesTest, SUB_n) { EXPECT_EQ(0xD6, SUB_n); }

TEST(Z80OpcodesTest, SBC_A_n) { EXPECT_EQ(0xDE, SBC_A_n); }

TEST(Z80OpcodesTest, INC_A) { EXPECT_EQ(0x3C, INC_A); }

TEST(Z80OpcodesTest, DEC_A) { EXPECT_EQ(0x3D, DEC_A); }

// ============================================================================
// Logical Instructions
// ============================================================================

TEST(Z80OpcodesTest, AND_n) { EXPECT_EQ(0xE6, AND_n); }

TEST(Z80OpcodesTest, OR_n) { EXPECT_EQ(0xF6, OR_n); }

TEST(Z80OpcodesTest, XOR_n) { EXPECT_EQ(0xEE, XOR_n); }

TEST(Z80OpcodesTest, XOR_A) { EXPECT_EQ(0xAF, XOR_A); }

TEST(Z80OpcodesTest, CP_n) { EXPECT_EQ(0xFE, CP_n); }

// ============================================================================
// Control Flow Instructions
// ============================================================================

TEST(Z80OpcodesTest, NOP) { EXPECT_EQ(0x00, NOP); }

TEST(Z80OpcodesTest, JP_nn) { EXPECT_EQ(0xC3, JP_nn); }

TEST(Z80OpcodesTest, JR_e) { EXPECT_EQ(0x18, JR_e); }

TEST(Z80OpcodesTest, JR_NZ_e) { EXPECT_EQ(0x20, JR_NZ_e); }

TEST(Z80OpcodesTest, JR_Z_e) { EXPECT_EQ(0x28, JR_Z_e); }

TEST(Z80OpcodesTest, JR_NC_e) { EXPECT_EQ(0x30, JR_NC_e); }

TEST(Z80OpcodesTest, JR_C_e) { EXPECT_EQ(0x38, JR_C_e); }

TEST(Z80OpcodesTest, CALL_nn) { EXPECT_EQ(0xCD, CALL_nn); }

TEST(Z80OpcodesTest, RET) { EXPECT_EQ(0xC9, RET); }

TEST(Z80OpcodesTest, RET_Z) { EXPECT_EQ(0xC8, RET_Z); }

TEST(Z80OpcodesTest, RET_NZ) { EXPECT_EQ(0xC0, RET_NZ); }

// ============================================================================
// Stack Instructions
// ============================================================================

TEST(Z80OpcodesTest, PUSH_BC) { EXPECT_EQ(0xC5, PUSH_BC); }

TEST(Z80OpcodesTest, PUSH_DE) { EXPECT_EQ(0xD5, PUSH_DE); }

TEST(Z80OpcodesTest, PUSH_HL) { EXPECT_EQ(0xE5, PUSH_HL); }

TEST(Z80OpcodesTest, PUSH_AF) { EXPECT_EQ(0xF5, PUSH_AF); }

TEST(Z80OpcodesTest, POP_BC) { EXPECT_EQ(0xC1, POP_BC); }

TEST(Z80OpcodesTest, POP_DE) { EXPECT_EQ(0xD1, POP_DE); }

TEST(Z80OpcodesTest, POP_HL) { EXPECT_EQ(0xE1, POP_HL); }

TEST(Z80OpcodesTest, POP_AF) { EXPECT_EQ(0xF1, POP_AF); }

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

TEST(Z80OpcodesTest, RLCA) { EXPECT_EQ(0x07, RLCA); }

TEST(Z80OpcodesTest, RRCA) { EXPECT_EQ(0x0F, RRCA); }

TEST(Z80OpcodesTest, RLA) { EXPECT_EQ(0x17, RLA); }

TEST(Z80OpcodesTest, RRA) { EXPECT_EQ(0x1F, RRA); }

// ============================================================================
// Memory Access Instructions
// ============================================================================

TEST(Z80OpcodesTest, LD_A_addr) { EXPECT_EQ(0x3A, LD_A_addr); }

TEST(Z80OpcodesTest, LD_addr_A) { EXPECT_EQ(0x32, LD_addr_A); }

TEST(Z80OpcodesTest, LD_A_HL) { EXPECT_EQ(0x7E, LD_A_HL); }

TEST(Z80OpcodesTest, LD_HL_A) { EXPECT_EQ(0x77, LD_HL_A); }

// ============================================================================
// CB-Prefixed Bit Operations (namespace CB)
// ============================================================================

TEST(Z80OpcodesTest, CB_PREFIX) { EXPECT_EQ(0xCB, CB_PREFIX); }

TEST(Z80OpcodesTest, CB_BIT_0_A) { EXPECT_EQ(0x47, CB::BIT_0_A); }

TEST(Z80OpcodesTest, CB_SET_7_A) { EXPECT_EQ(0xFF, CB::SET_7_A); }

TEST(Z80OpcodesTest, CB_RES_3_B) { EXPECT_EQ(0x98, CB::RES_3_B); }

TEST(Z80OpcodesTest, CB_RLC_A) { EXPECT_EQ(0x07, CB::RLC_A); }

TEST(Z80OpcodesTest, CB_RRC_A) { EXPECT_EQ(0x0F, CB::RRC_A); }

TEST(Z80OpcodesTest, CB_RL_A) { EXPECT_EQ(0x17, CB::RL_A); }

TEST(Z80OpcodesTest, CB_RR_A) { EXPECT_EQ(0x1F, CB::RR_A); }

TEST(Z80OpcodesTest, CB_SLA_A) { EXPECT_EQ(0x27, CB::SLA_A); }

TEST(Z80OpcodesTest, CB_SRA_A) { EXPECT_EQ(0x2F, CB::SRA_A); }

TEST(Z80OpcodesTest, CB_SRL_A) { EXPECT_EQ(0x3F, CB::SRL_A); }

// ============================================================================
// ED-Prefixed Extended Instructions (namespace ED)
// ============================================================================

TEST(Z80OpcodesTest, ED_PREFIX) { EXPECT_EQ(0xED, ED_PREFIX); }

TEST(Z80OpcodesTest, ED_LD_I_A) { EXPECT_EQ(0x47, ED::LD_I_A); }

TEST(Z80OpcodesTest, ED_LD_A_I) { EXPECT_EQ(0x57, ED::LD_A_I); }

TEST(Z80OpcodesTest, ED_LD_R_A) { EXPECT_EQ(0x4F, ED::LD_R_A); }

TEST(Z80OpcodesTest, ED_LD_A_R) { EXPECT_EQ(0x5F, ED::LD_A_R); }

TEST(Z80OpcodesTest, ED_LDIR) { EXPECT_EQ(0xB0, ED::LDIR); }

TEST(Z80OpcodesTest, ED_CPIR) { EXPECT_EQ(0xB1, ED::CPIR); }

TEST(Z80OpcodesTest, ED_NEG) { EXPECT_EQ(0x44, ED::NEG); }

TEST(Z80OpcodesTest, ED_RETI) { EXPECT_EQ(0x4D, ED::RETI); }

TEST(Z80OpcodesTest, ED_IM_0) { EXPECT_EQ(0x46, ED::IM_0); }

TEST(Z80OpcodesTest, ED_IM_1) { EXPECT_EQ(0x56, ED::IM_1); }

TEST(Z80OpcodesTest, ED_IM_2) { EXPECT_EQ(0x5E, ED::IM_2); }

// ============================================================================
// DD-Prefixed IX Register Instructions (namespace DD)
// ============================================================================

TEST(Z80OpcodesTest, DD_PREFIX) { EXPECT_EQ(0xDD, DD_PREFIX); }

TEST(Z80OpcodesTest, DD_LD_IX_nn) { EXPECT_EQ(0x21, DD::LD_IX_nn); }

TEST(Z80OpcodesTest, DD_LD_A_IX_d) { EXPECT_EQ(0x7E, DD::LD_A_IX_d); }

TEST(Z80OpcodesTest, DD_ADD_IX_BC) { EXPECT_EQ(0x09, DD::ADD_IX_BC); }

TEST(Z80OpcodesTest, DD_INC_IX) { EXPECT_EQ(0x23, DD::INC_IX); }

TEST(Z80OpcodesTest, DD_DEC_IX) { EXPECT_EQ(0x2B, DD::DEC_IX); }

// ============================================================================
// FD-Prefixed IY Register Instructions (namespace FD)
// ============================================================================

TEST(Z80OpcodesTest, FD_PREFIX) { EXPECT_EQ(0xFD, FD_PREFIX); }

TEST(Z80OpcodesTest, FD_LD_IY_nn) { EXPECT_EQ(0x21, FD::LD_IY_nn); }

TEST(Z80OpcodesTest, FD_LD_A_IY_d) { EXPECT_EQ(0x7E, FD::LD_A_IY_d); }

TEST(Z80OpcodesTest, FD_ADD_IY_BC) { EXPECT_EQ(0x09, FD::ADD_IY_BC); }

TEST(Z80OpcodesTest, FD_INC_IY) { EXPECT_EQ(0x23, FD::INC_IY); }

TEST(Z80OpcodesTest, FD_DEC_IY) { EXPECT_EQ(0x2B, FD::DEC_IY); }
