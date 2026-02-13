/**
 * @file test_mnemonic_constants.cpp
 * @brief Unit tests for opcode mnemonic string constants
 *
 * Tests that mnemonic constants are defined and accessible.
 * Eliminates magic strings code smell by providing named constants.
 */

#include "xasm++/cpu/opcodes_6502.h"
#include "xasm++/cpu/opcodes_6809.h"
#include "xasm++/cpu/opcodes_z80.h"
#include <cstring>
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Z80 Mnemonic Constants Tests
// ============================================================================

TEST(MnemonicConstantsTest, Z80_LoadInstructions) {
  // Test that Z80 load mnemonics are defined and correct
  EXPECT_STREQ("LD", Z80Mnemonics::LD);
  EXPECT_STREQ("PUSH", Z80Mnemonics::PUSH);
  EXPECT_STREQ("POP", Z80Mnemonics::POP);
}

TEST(MnemonicConstantsTest, Z80_ArithmeticInstructions) {
  // Test that Z80 arithmetic mnemonics are defined and correct
  EXPECT_STREQ("ADD", Z80Mnemonics::ADD);
  EXPECT_STREQ("ADC", Z80Mnemonics::ADC);
  EXPECT_STREQ("SUB", Z80Mnemonics::SUB);
  EXPECT_STREQ("SBC", Z80Mnemonics::SBC);
  EXPECT_STREQ("INC", Z80Mnemonics::INC);
  EXPECT_STREQ("DEC", Z80Mnemonics::DEC);
}

TEST(MnemonicConstantsTest, Z80_LogicalInstructions) {
  // Test that Z80 logical mnemonics are defined and correct
  EXPECT_STREQ("AND", Z80Mnemonics::AND);
  EXPECT_STREQ("OR", Z80Mnemonics::OR);
  EXPECT_STREQ("XOR", Z80Mnemonics::XOR);
  EXPECT_STREQ("CP", Z80Mnemonics::CP);
}

TEST(MnemonicConstantsTest, Z80_BranchInstructions) {
  // Test that Z80 branch/jump mnemonics are defined and correct
  EXPECT_STREQ("JP", Z80Mnemonics::JP);
  EXPECT_STREQ("JR", Z80Mnemonics::JR);
  EXPECT_STREQ("CALL", Z80Mnemonics::CALL);
  EXPECT_STREQ("RET", Z80Mnemonics::RET);
  EXPECT_STREQ("RST", Z80Mnemonics::RST);
}

TEST(MnemonicConstantsTest, Z80_RotateInstructions) {
  // Test that Z80 rotate/shift mnemonics are defined and correct
  EXPECT_STREQ("RLCA", Z80Mnemonics::RLCA);
  EXPECT_STREQ("RRCA", Z80Mnemonics::RRCA);
  EXPECT_STREQ("RLA", Z80Mnemonics::RLA);
  EXPECT_STREQ("RRA", Z80Mnemonics::RRA);
  EXPECT_STREQ("RLC", Z80Mnemonics::RLC);
  EXPECT_STREQ("RRC", Z80Mnemonics::RRC);
  EXPECT_STREQ("RL", Z80Mnemonics::RL);
  EXPECT_STREQ("RR", Z80Mnemonics::RR);
  EXPECT_STREQ("SLA", Z80Mnemonics::SLA);
  EXPECT_STREQ("SRA", Z80Mnemonics::SRA);
  EXPECT_STREQ("SRL", Z80Mnemonics::SRL);
}

TEST(MnemonicConstantsTest, Z80_BitInstructions) {
  // Test that Z80 bit manipulation mnemonics are defined and correct
  EXPECT_STREQ("BIT", Z80Mnemonics::BIT);
  EXPECT_STREQ("SET", Z80Mnemonics::SET);
  EXPECT_STREQ("RES", Z80Mnemonics::RES);
}

TEST(MnemonicConstantsTest, Z80_MiscInstructions) {
  // Test that Z80 miscellaneous mnemonics are defined and correct
  EXPECT_STREQ("NOP", Z80Mnemonics::NOP);
  EXPECT_STREQ("HALT", Z80Mnemonics::HALT);
  EXPECT_STREQ("DI", Z80Mnemonics::DI);
  EXPECT_STREQ("EI", Z80Mnemonics::EI);
  EXPECT_STREQ("NEG", Z80Mnemonics::NEG);
  EXPECT_STREQ("CPL", Z80Mnemonics::CPL);
  EXPECT_STREQ("CCF", Z80Mnemonics::CCF);
  EXPECT_STREQ("SCF", Z80Mnemonics::SCF);
  EXPECT_STREQ("DAA", Z80Mnemonics::DAA);
}

TEST(MnemonicConstantsTest, Z80_BlockInstructions) {
  // Test that Z80 block operation mnemonics are defined and correct
  EXPECT_STREQ("LDI", Z80Mnemonics::LDI);
  EXPECT_STREQ("LDIR", Z80Mnemonics::LDIR);
  EXPECT_STREQ("LDD", Z80Mnemonics::LDD);
  EXPECT_STREQ("LDDR", Z80Mnemonics::LDDR);
  EXPECT_STREQ("CPI", Z80Mnemonics::CPI);
  EXPECT_STREQ("CPIR", Z80Mnemonics::CPIR);
  EXPECT_STREQ("CPD", Z80Mnemonics::CPD);
  EXPECT_STREQ("CPDR", Z80Mnemonics::CPDR);
}

// ============================================================================
// 6502 Mnemonic Constants Tests
// ============================================================================

TEST(MnemonicConstantsTest, M6502_LoadStoreInstructions) {
  // Test that 6502 load/store mnemonics are defined and correct
  EXPECT_STREQ("LDA", M6502Mnemonics::LDA);
  EXPECT_STREQ("LDX", M6502Mnemonics::LDX);
  EXPECT_STREQ("LDY", M6502Mnemonics::LDY);
  EXPECT_STREQ("STA", M6502Mnemonics::STA);
  EXPECT_STREQ("STX", M6502Mnemonics::STX);
  EXPECT_STREQ("STY", M6502Mnemonics::STY);
}

TEST(MnemonicConstantsTest, M6502_ArithmeticInstructions) {
  // Test that 6502 arithmetic mnemonics are defined and correct
  EXPECT_STREQ("ADC", M6502Mnemonics::ADC);
  EXPECT_STREQ("SBC", M6502Mnemonics::SBC);
  EXPECT_STREQ("INC", M6502Mnemonics::INC);
  EXPECT_STREQ("DEC", M6502Mnemonics::DEC);
  EXPECT_STREQ("INX", M6502Mnemonics::INX);
  EXPECT_STREQ("DEX", M6502Mnemonics::DEX);
  EXPECT_STREQ("INY", M6502Mnemonics::INY);
  EXPECT_STREQ("DEY", M6502Mnemonics::DEY);
}

TEST(MnemonicConstantsTest, M6502_LogicalInstructions) {
  // Test that 6502 logical mnemonics are defined and correct
  EXPECT_STREQ("AND", M6502Mnemonics::AND);
  EXPECT_STREQ("ORA", M6502Mnemonics::ORA);
  EXPECT_STREQ("EOR", M6502Mnemonics::EOR);
  EXPECT_STREQ("BIT", M6502Mnemonics::BIT);
}

TEST(MnemonicConstantsTest, M6502_BranchInstructions) {
  // Test that 6502 branch mnemonics are defined and correct
  EXPECT_STREQ("BCC", M6502Mnemonics::BCC);
  EXPECT_STREQ("BCS", M6502Mnemonics::BCS);
  EXPECT_STREQ("BEQ", M6502Mnemonics::BEQ);
  EXPECT_STREQ("BNE", M6502Mnemonics::BNE);
  EXPECT_STREQ("BMI", M6502Mnemonics::BMI);
  EXPECT_STREQ("BPL", M6502Mnemonics::BPL);
  EXPECT_STREQ("BVC", M6502Mnemonics::BVC);
  EXPECT_STREQ("BVS", M6502Mnemonics::BVS);
}

TEST(MnemonicConstantsTest, M6502_JumpInstructions) {
  // Test that 6502 jump/subroutine mnemonics are defined and correct
  EXPECT_STREQ("JMP", M6502Mnemonics::JMP);
  EXPECT_STREQ("JSR", M6502Mnemonics::JSR);
  EXPECT_STREQ("RTS", M6502Mnemonics::RTS);
  EXPECT_STREQ("RTI", M6502Mnemonics::RTI);
}

// ============================================================================
// 6809 Mnemonic Constants Tests
// ============================================================================

TEST(MnemonicConstantsTest, M6809_LoadStoreInstructions) {
  // Test that 6809 load/store mnemonics are defined and correct
  EXPECT_STREQ("LDA", M6809Mnemonics::LDA);
  EXPECT_STREQ("LDB", M6809Mnemonics::LDB);
  EXPECT_STREQ("LDD", M6809Mnemonics::LDD);
  EXPECT_STREQ("LDX", M6809Mnemonics::LDX);
  EXPECT_STREQ("LDY", M6809Mnemonics::LDY);
  EXPECT_STREQ("STA", M6809Mnemonics::STA);
  EXPECT_STREQ("STB", M6809Mnemonics::STB);
  EXPECT_STREQ("STD", M6809Mnemonics::STD);
  EXPECT_STREQ("STX", M6809Mnemonics::STX);
  EXPECT_STREQ("STY", M6809Mnemonics::STY);
}

TEST(MnemonicConstantsTest, M6809_ArithmeticInstructions) {
  // Test that 6809 arithmetic mnemonics are defined and correct
  EXPECT_STREQ("ADDA", M6809Mnemonics::ADDA);
  EXPECT_STREQ("ADDB", M6809Mnemonics::ADDB);
  EXPECT_STREQ("ADDD", M6809Mnemonics::ADDD);
  EXPECT_STREQ("SUBA", M6809Mnemonics::SUBA);
  EXPECT_STREQ("SUBB", M6809Mnemonics::SUBB);
  EXPECT_STREQ("SUBD", M6809Mnemonics::SUBD);
}

TEST(MnemonicConstantsTest, M6809_BranchInstructions) {
  // Test that 6809 branch mnemonics are defined and correct
  EXPECT_STREQ("BRA", M6809Mnemonics::BRA);
  EXPECT_STREQ("BCC", M6809Mnemonics::BCC);
  EXPECT_STREQ("BCS", M6809Mnemonics::BCS);
  EXPECT_STREQ("BEQ", M6809Mnemonics::BEQ);
  EXPECT_STREQ("BNE", M6809Mnemonics::BNE);
  EXPECT_STREQ("BMI", M6809Mnemonics::BMI);
  EXPECT_STREQ("BPL", M6809Mnemonics::BPL);
}
