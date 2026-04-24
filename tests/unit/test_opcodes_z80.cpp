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

TEST(Z80OpcodesTest, kLD_A_n) { EXPECT_EQ(0x3E, kLD_A_n); }

TEST(Z80OpcodesTest, kLD_B_n) { EXPECT_EQ(0x06, kLD_B_n); }

TEST(Z80OpcodesTest, kLD_C_n) { EXPECT_EQ(0x0E, kLD_C_n); }

TEST(Z80OpcodesTest, kLD_D_n) { EXPECT_EQ(0x16, kLD_D_n); }

TEST(Z80OpcodesTest, kLD_E_n) { EXPECT_EQ(0x1E, kLD_E_n); }

TEST(Z80OpcodesTest, kLD_H_n) { EXPECT_EQ(0x26, kLD_H_n); }

TEST(Z80OpcodesTest, kLD_L_n) { EXPECT_EQ(0x2E, kLD_L_n); }

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

TEST(Z80OpcodesTest, kLD_BC_nn) { EXPECT_EQ(0x01, kLD_BC_nn); }

TEST(Z80OpcodesTest, kLD_DE_nn) { EXPECT_EQ(0x11, kLD_DE_nn); }

TEST(Z80OpcodesTest, kLD_HL_nn) { EXPECT_EQ(0x21, kLD_HL_nn); }

TEST(Z80OpcodesTest, kLD_SP_nn) { EXPECT_EQ(0x31, kLD_SP_nn); }

// ============================================================================
// Register-to-Register Load Instructions
// ============================================================================

TEST(Z80OpcodesTest, kLD_A_B) { EXPECT_EQ(0x78, kLD_A_B); }

TEST(Z80OpcodesTest, kLD_A_C) { EXPECT_EQ(0x79, kLD_A_C); }

TEST(Z80OpcodesTest, kLD_B_A) { EXPECT_EQ(0x47, kLD_B_A); }

TEST(Z80OpcodesTest, kLD_C_A) { EXPECT_EQ(0x4F, kLD_C_A); }

// ============================================================================
// Arithmetic Instructions
// ============================================================================

TEST(Z80OpcodesTest, kADD_A_n) { EXPECT_EQ(0xC6, kADD_A_n); }

TEST(Z80OpcodesTest, kADC_A_n) { EXPECT_EQ(0xCE, kADC_A_n); }

TEST(Z80OpcodesTest, kSUB_n) { EXPECT_EQ(0xD6, kSUB_n); }

TEST(Z80OpcodesTest, kSBC_A_n) { EXPECT_EQ(0xDE, kSBC_A_n); }

TEST(Z80OpcodesTest, kINC_A) { EXPECT_EQ(0x3C, kINC_A); }

TEST(Z80OpcodesTest, kDEC_A) { EXPECT_EQ(0x3D, kDEC_A); }

// ============================================================================
// Logical Instructions
// ============================================================================

TEST(Z80OpcodesTest, kAND_n) { EXPECT_EQ(0xE6, kAND_n); }

TEST(Z80OpcodesTest, kOR_n) { EXPECT_EQ(0xF6, kOR_n); }

TEST(Z80OpcodesTest, kXOR_n) { EXPECT_EQ(0xEE, kXOR_n); }

TEST(Z80OpcodesTest, kXOR_A) { EXPECT_EQ(0xAF, kXOR_A); }

TEST(Z80OpcodesTest, kCP_n) { EXPECT_EQ(0xFE, kCP_n); }

// ============================================================================
// Control Flow Instructions
// ============================================================================

TEST(Z80OpcodesTest, kNOP) { EXPECT_EQ(0x00, kNOP); }

TEST(Z80OpcodesTest, kJP_nn) { EXPECT_EQ(0xC3, kJP_nn); }

TEST(Z80OpcodesTest, kJR_e) { EXPECT_EQ(0x18, kJR_e); }

TEST(Z80OpcodesTest, kJR_NZ_e) { EXPECT_EQ(0x20, kJR_NZ_e); }

TEST(Z80OpcodesTest, kJR_Z_e) { EXPECT_EQ(0x28, kJR_Z_e); }

TEST(Z80OpcodesTest, kJR_NC_e) { EXPECT_EQ(0x30, kJR_NC_e); }

TEST(Z80OpcodesTest, kJR_C_e) { EXPECT_EQ(0x38, kJR_C_e); }

TEST(Z80OpcodesTest, kCALL_nn) { EXPECT_EQ(0xCD, kCALL_nn); }

TEST(Z80OpcodesTest, kRET) { EXPECT_EQ(0xC9, kRET); }

TEST(Z80OpcodesTest, kRET_Z) { EXPECT_EQ(0xC8, kRET_Z); }

TEST(Z80OpcodesTest, kRET_NZ) { EXPECT_EQ(0xC0, kRET_NZ); }

// ============================================================================
// Stack Instructions
// ============================================================================

TEST(Z80OpcodesTest, kPUSH_BC) { EXPECT_EQ(0xC5, kPUSH_BC); }

TEST(Z80OpcodesTest, kPUSH_DE) { EXPECT_EQ(0xD5, kPUSH_DE); }

TEST(Z80OpcodesTest, kPUSH_HL) { EXPECT_EQ(0xE5, kPUSH_HL); }

TEST(Z80OpcodesTest, kPUSH_AF) { EXPECT_EQ(0xF5, kPUSH_AF); }

TEST(Z80OpcodesTest, kPOP_BC) { EXPECT_EQ(0xC1, kPOP_BC); }

TEST(Z80OpcodesTest, kPOP_DE) { EXPECT_EQ(0xD1, kPOP_DE); }

TEST(Z80OpcodesTest, kPOP_HL) { EXPECT_EQ(0xE1, kPOP_HL); }

TEST(Z80OpcodesTest, kPOP_AF) { EXPECT_EQ(0xF1, kPOP_AF); }

// ============================================================================
// Rotate and Shift Instructions
// ============================================================================

TEST(Z80OpcodesTest, kRLCA) { EXPECT_EQ(0x07, kRLCA); }

TEST(Z80OpcodesTest, kRRCA) { EXPECT_EQ(0x0F, kRRCA); }

TEST(Z80OpcodesTest, kRLA) { EXPECT_EQ(0x17, kRLA); }

TEST(Z80OpcodesTest, kRRA) { EXPECT_EQ(0x1F, kRRA); }

// ============================================================================
// Memory Access Instructions
// ============================================================================

TEST(Z80OpcodesTest, kLD_A_addr) { EXPECT_EQ(0x3A, kLD_A_addr); }

TEST(Z80OpcodesTest, kLD_addr_A) { EXPECT_EQ(0x32, kLD_addr_A); }

TEST(Z80OpcodesTest, kLD_A_HL) { EXPECT_EQ(0x7E, kLD_A_HL); }

TEST(Z80OpcodesTest, kLD_HL_A) { EXPECT_EQ(0x77, kLD_HL_A); }

// ============================================================================
// CB-Prefixed Bit Operations (namespace CB)
// ============================================================================

TEST(Z80OpcodesTest, kCB_PREFIX) { EXPECT_EQ(0xCB, kCB_PREFIX); }

TEST(Z80OpcodesTest, CB_BIT_0_A) { EXPECT_EQ(0x47, CB::kBIT_0_A); }

TEST(Z80OpcodesTest, CB_SET_7_A) { EXPECT_EQ(0xFF, CB::kSET_7_A); }

TEST(Z80OpcodesTest, CB_RES_3_B) { EXPECT_EQ(0x98, CB::kRES_3_B); }

TEST(Z80OpcodesTest, CB_RLC_A) { EXPECT_EQ(0x07, CB::kRLC_A); }

TEST(Z80OpcodesTest, CB_RRC_A) { EXPECT_EQ(0x0F, CB::kRRC_A); }

TEST(Z80OpcodesTest, CB_RL_A) { EXPECT_EQ(0x17, CB::kRL_A); }

TEST(Z80OpcodesTest, CB_RR_A) { EXPECT_EQ(0x1F, CB::kRR_A); }

TEST(Z80OpcodesTest, CB_SLA_A) { EXPECT_EQ(0x27, CB::kSLA_A); }

TEST(Z80OpcodesTest, CB_SRA_A) { EXPECT_EQ(0x2F, CB::kSRA_A); }

TEST(Z80OpcodesTest, CB_SRL_A) { EXPECT_EQ(0x3F, CB::kSRL_A); }

// ============================================================================
// ED-Prefixed Extended Instructions (namespace ED)
// ============================================================================

TEST(Z80OpcodesTest, kED_PREFIX) { EXPECT_EQ(0xED, kED_PREFIX); }

TEST(Z80OpcodesTest, ED_LD_I_A) { EXPECT_EQ(0x47, ED::kLD_I_A); }

TEST(Z80OpcodesTest, ED_LD_A_I) { EXPECT_EQ(0x57, ED::kLD_A_I); }

TEST(Z80OpcodesTest, ED_LD_R_A) { EXPECT_EQ(0x4F, ED::kLD_R_A); }

TEST(Z80OpcodesTest, ED_LD_A_R) { EXPECT_EQ(0x5F, ED::kLD_A_R); }

TEST(Z80OpcodesTest, ED_LDIR) { EXPECT_EQ(0xB0, ED::kLDIR); }

TEST(Z80OpcodesTest, ED_CPIR) { EXPECT_EQ(0xB1, ED::kCPIR); }

TEST(Z80OpcodesTest, ED_NEG) { EXPECT_EQ(0x44, ED::kNEG); }

TEST(Z80OpcodesTest, ED_RETI) { EXPECT_EQ(0x4D, ED::kRETI); }

TEST(Z80OpcodesTest, ED_IM_0) { EXPECT_EQ(0x46, ED::kIM_0); }

TEST(Z80OpcodesTest, ED_IM_1) { EXPECT_EQ(0x56, ED::kIM_1); }

TEST(Z80OpcodesTest, ED_IM_2) { EXPECT_EQ(0x5E, ED::kIM_2); }

// ============================================================================
// DD-Prefixed IX Register Instructions (namespace DD)
// ============================================================================

TEST(Z80OpcodesTest, kDD_PREFIX) { EXPECT_EQ(0xDD, kDD_PREFIX); }

TEST(Z80OpcodesTest, DD_LD_IX_nn) { EXPECT_EQ(0x21, DD::kLD_IX_nn); }

TEST(Z80OpcodesTest, DD_LD_A_IX_d) { EXPECT_EQ(0x7E, DD::kLD_A_IX_d); }

TEST(Z80OpcodesTest, DD_ADD_IX_BC) { EXPECT_EQ(0x09, DD::kADD_IX_BC); }

TEST(Z80OpcodesTest, DD_INC_IX) { EXPECT_EQ(0x23, DD::kINC_IX); }

TEST(Z80OpcodesTest, DD_DEC_IX) { EXPECT_EQ(0x2B, DD::kDEC_IX); }

// ============================================================================
// FD-Prefixed IY Register Instructions (namespace FD)
// ============================================================================

TEST(Z80OpcodesTest, kFD_PREFIX) { EXPECT_EQ(0xFD, kFD_PREFIX); }

TEST(Z80OpcodesTest, FD_LD_IY_nn) { EXPECT_EQ(0x21, FD::kLD_IY_nn); }

TEST(Z80OpcodesTest, FD_LD_A_IY_d) { EXPECT_EQ(0x7E, FD::kLD_A_IY_d); }

TEST(Z80OpcodesTest, FD_ADD_IY_BC) { EXPECT_EQ(0x09, FD::kADD_IY_BC); }

TEST(Z80OpcodesTest, FD_INC_IY) { EXPECT_EQ(0x23, FD::kINC_IY); }

TEST(Z80OpcodesTest, FD_DEC_IY) { EXPECT_EQ(0x2B, FD::kDEC_IY); }
