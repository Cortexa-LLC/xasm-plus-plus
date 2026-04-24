/**
 * @file test_mnemonic_constants.cpp
 * @brief Unit tests for opcode mnemonic enums and parse functions
 *
 * Tests that mnemonics can be parsed from strings to enum values, and
 * that the Z80 string constants (kept for syntax-parser backward compat)
 * are defined and accessible.
 */

#include "xasm++/cpu/mnemonics_6809.h"
#include "xasm++/cpu/opcodes_6502.h"
#include "xasm++/cpu/opcodes_6809.h"
#include "xasm++/cpu/opcodes_z80.h"
#include <cstring>
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Z80 Mnemonic String Constants Tests (backward-compat constants in Z80Mnemonics)
// ============================================================================

TEST(MnemonicConstantsTest, Z80_LoadInstructions) {
  EXPECT_STREQ("LD", Z80Mnemonics::kLD);
  EXPECT_STREQ("PUSH", Z80Mnemonics::kPUSH);
  EXPECT_STREQ("POP", Z80Mnemonics::kPOP);
}

TEST(MnemonicConstantsTest, Z80_ArithmeticInstructions) {
  EXPECT_STREQ("ADD", Z80Mnemonics::kADD);
  EXPECT_STREQ("ADC", Z80Mnemonics::kADC);
  EXPECT_STREQ("SUB", Z80Mnemonics::kSUB);
  EXPECT_STREQ("SBC", Z80Mnemonics::kSBC);
  EXPECT_STREQ("INC", Z80Mnemonics::kINC);
  EXPECT_STREQ("DEC", Z80Mnemonics::kDEC);
}

TEST(MnemonicConstantsTest, Z80_LogicalInstructions) {
  EXPECT_STREQ("AND", Z80Mnemonics::kAND);
  EXPECT_STREQ("OR", Z80Mnemonics::kOR);
  EXPECT_STREQ("XOR", Z80Mnemonics::kXOR);
  EXPECT_STREQ("CP", Z80Mnemonics::kCP);
}

TEST(MnemonicConstantsTest, Z80_BranchInstructions) {
  EXPECT_STREQ("JP", Z80Mnemonics::kJP);
  EXPECT_STREQ("JR", Z80Mnemonics::kJR);
  EXPECT_STREQ("CALL", Z80Mnemonics::kCALL);
  EXPECT_STREQ("RET", Z80Mnemonics::kRET);
  EXPECT_STREQ("RST", Z80Mnemonics::kRST);
}

TEST(MnemonicConstantsTest, Z80_RotateInstructions) {
  EXPECT_STREQ("RLCA", Z80Mnemonics::kRLCA);
  EXPECT_STREQ("RRCA", Z80Mnemonics::kRRCA);
  EXPECT_STREQ("RLA", Z80Mnemonics::kRLA);
  EXPECT_STREQ("RRA", Z80Mnemonics::kRRA);
  EXPECT_STREQ("RLC", Z80Mnemonics::kRLC);
  EXPECT_STREQ("RRC", Z80Mnemonics::kRRC);
  EXPECT_STREQ("RL", Z80Mnemonics::kRL);
  EXPECT_STREQ("RR", Z80Mnemonics::kRR);
  EXPECT_STREQ("SLA", Z80Mnemonics::kSLA);
  EXPECT_STREQ("SRA", Z80Mnemonics::kSRA);
  EXPECT_STREQ("SRL", Z80Mnemonics::kSRL);
}

TEST(MnemonicConstantsTest, Z80_BitInstructions) {
  EXPECT_STREQ("BIT", Z80Mnemonics::kBIT);
  EXPECT_STREQ("SET", Z80Mnemonics::kSET);
  EXPECT_STREQ("RES", Z80Mnemonics::kRES);
}

TEST(MnemonicConstantsTest, Z80_MiscInstructions) {
  EXPECT_STREQ("NOP", Z80Mnemonics::kNOP);
  EXPECT_STREQ("HALT", Z80Mnemonics::kHALT);
  EXPECT_STREQ("DI", Z80Mnemonics::kDI);
  EXPECT_STREQ("EI", Z80Mnemonics::kEI);
  EXPECT_STREQ("NEG", Z80Mnemonics::kNEG);
  EXPECT_STREQ("CPL", Z80Mnemonics::kCPL);
  EXPECT_STREQ("CCF", Z80Mnemonics::kCCF);
  EXPECT_STREQ("SCF", Z80Mnemonics::kSCF);
  EXPECT_STREQ("DAA", Z80Mnemonics::kDAA);
}

TEST(MnemonicConstantsTest, Z80_BlockInstructions) {
  EXPECT_STREQ("LDI", Z80Mnemonics::kLDI);
  EXPECT_STREQ("LDIR", Z80Mnemonics::kLDIR);
  EXPECT_STREQ("LDD", Z80Mnemonics::kLDD);
  EXPECT_STREQ("LDDR", Z80Mnemonics::kLDDR);
  EXPECT_STREQ("CPI", Z80Mnemonics::kCPI);
  EXPECT_STREQ("CPIR", Z80Mnemonics::kCPIR);
  EXPECT_STREQ("CPD", Z80Mnemonics::kCPD);
  EXPECT_STREQ("CPDR", Z80Mnemonics::kCPDR);
}

// ============================================================================
// Z80Mnemonic enum + ParseZ80Mnemonic tests
// ============================================================================

TEST(MnemonicEnumTest, Z80_ParseKnown) {
  EXPECT_EQ(Z80Mnemonic::LD,   ParseZ80Mnemonic("LD"));
  EXPECT_EQ(Z80Mnemonic::ADD,  ParseZ80Mnemonic("ADD"));
  EXPECT_EQ(Z80Mnemonic::JP,   ParseZ80Mnemonic("JP"));
  EXPECT_EQ(Z80Mnemonic::NOP,  ParseZ80Mnemonic("NOP"));
  EXPECT_EQ(Z80Mnemonic::DJNZ, ParseZ80Mnemonic("DJNZ"));
  EXPECT_EQ(Z80Mnemonic::LDIR, ParseZ80Mnemonic("LDIR"));
}

TEST(MnemonicEnumTest, Z80_ParseUnknown) {
  EXPECT_EQ(Z80Mnemonic::Unknown, ParseZ80Mnemonic(""));
  EXPECT_EQ(Z80Mnemonic::Unknown, ParseZ80Mnemonic("FOOBAR"));
  EXPECT_EQ(Z80Mnemonic::Unknown, ParseZ80Mnemonic("lda")); // lowercase
}

// ============================================================================
// 6502 Mnemonic Constants Tests (unchanged)
// ============================================================================

TEST(MnemonicConstantsTest, M6502_LoadStoreInstructions) {
  EXPECT_STREQ("LDA", M6502Mnemonics::LDA);
  EXPECT_STREQ("LDX", M6502Mnemonics::LDX);
  EXPECT_STREQ("LDY", M6502Mnemonics::LDY);
  EXPECT_STREQ("STA", M6502Mnemonics::STA);
  EXPECT_STREQ("STX", M6502Mnemonics::STX);
  EXPECT_STREQ("STY", M6502Mnemonics::STY);
}

TEST(MnemonicConstantsTest, M6502_ArithmeticInstructions) {
  EXPECT_STREQ("ADC", M6502Mnemonics::ADC);
  EXPECT_STREQ("SBC", M6502Mnemonics::SBC);
  EXPECT_STREQ("INC", M6502Mnemonics::INC);
  EXPECT_STREQ("DEC", M6502Mnemonics::DEC);
  EXPECT_STREQ("INX", M6502Mnemonics::kINX);
  EXPECT_STREQ("DEX", M6502Mnemonics::kDEX);
  EXPECT_STREQ("INY", M6502Mnemonics::kINY);
  EXPECT_STREQ("DEY", M6502Mnemonics::kDEY);
}

TEST(MnemonicConstantsTest, M6502_LogicalInstructions) {
  EXPECT_STREQ("AND", M6502Mnemonics::AND);
  EXPECT_STREQ("ORA", M6502Mnemonics::ORA);
  EXPECT_STREQ("EOR", M6502Mnemonics::EOR);
  EXPECT_STREQ("BIT", M6502Mnemonics::BIT);
}

TEST(MnemonicConstantsTest, M6502_BranchInstructions) {
  EXPECT_STREQ("BCC", M6502Mnemonics::kBCC);
  EXPECT_STREQ("BCS", M6502Mnemonics::kBCS);
  EXPECT_STREQ("BEQ", M6502Mnemonics::kBEQ);
  EXPECT_STREQ("BNE", M6502Mnemonics::kBNE);
  EXPECT_STREQ("BMI", M6502Mnemonics::kBMI);
  EXPECT_STREQ("BPL", M6502Mnemonics::kBPL);
  EXPECT_STREQ("BVC", M6502Mnemonics::kBVC);
  EXPECT_STREQ("BVS", M6502Mnemonics::kBVS);
}

TEST(MnemonicConstantsTest, M6502_JumpInstructions) {
  EXPECT_STREQ("JMP", M6502Mnemonics::JMP);
  EXPECT_STREQ("JSR", M6502Mnemonics::kJSR);
  EXPECT_STREQ("RTS", M6502Mnemonics::kRTS);
  EXPECT_STREQ("RTI", M6502Mnemonics::kRTI);
}

// ============================================================================
// M6809Mnemonic enum + ParseM6809Mnemonic tests (replacing old string consts)
// ============================================================================

TEST(MnemonicEnumTest, M6809_ParseLoadStore) {
  EXPECT_EQ(M6809Mnemonic::LDA, ParseM6809Mnemonic("LDA"));
  EXPECT_EQ(M6809Mnemonic::LDB, ParseM6809Mnemonic("LDB"));
  EXPECT_EQ(M6809Mnemonic::LDD, ParseM6809Mnemonic("LDD"));
  EXPECT_EQ(M6809Mnemonic::LDX, ParseM6809Mnemonic("LDX"));
  EXPECT_EQ(M6809Mnemonic::LDY, ParseM6809Mnemonic("LDY"));
  EXPECT_EQ(M6809Mnemonic::STA, ParseM6809Mnemonic("STA"));
  EXPECT_EQ(M6809Mnemonic::STB, ParseM6809Mnemonic("STB"));
  EXPECT_EQ(M6809Mnemonic::STD, ParseM6809Mnemonic("STD"));
  EXPECT_EQ(M6809Mnemonic::STX, ParseM6809Mnemonic("STX"));
  EXPECT_EQ(M6809Mnemonic::STY, ParseM6809Mnemonic("STY"));
}

TEST(MnemonicEnumTest, M6809_ParseArithmetic) {
  EXPECT_EQ(M6809Mnemonic::ADDA, ParseM6809Mnemonic("ADDA"));
  EXPECT_EQ(M6809Mnemonic::ADDB, ParseM6809Mnemonic("ADDB"));
  EXPECT_EQ(M6809Mnemonic::ADDD, ParseM6809Mnemonic("ADDD"));
  EXPECT_EQ(M6809Mnemonic::SUBA, ParseM6809Mnemonic("SUBA"));
  EXPECT_EQ(M6809Mnemonic::SUBB, ParseM6809Mnemonic("SUBB"));
  EXPECT_EQ(M6809Mnemonic::SUBD, ParseM6809Mnemonic("SUBD"));
}

TEST(MnemonicEnumTest, M6809_ParseBranch) {
  EXPECT_EQ(M6809Mnemonic::BRA,  ParseM6809Mnemonic("BRA"));
  EXPECT_EQ(M6809Mnemonic::BCC,  ParseM6809Mnemonic("BCC"));
  EXPECT_EQ(M6809Mnemonic::BCS,  ParseM6809Mnemonic("BCS"));
  EXPECT_EQ(M6809Mnemonic::BEQ,  ParseM6809Mnemonic("BEQ"));
  EXPECT_EQ(M6809Mnemonic::BNE,  ParseM6809Mnemonic("BNE"));
  EXPECT_EQ(M6809Mnemonic::BMI,  ParseM6809Mnemonic("BMI"));
  EXPECT_EQ(M6809Mnemonic::BPL,  ParseM6809Mnemonic("BPL"));
}

TEST(MnemonicEnumTest, M6809_ParseLongBranch) {
  EXPECT_EQ(M6809Mnemonic::LBRA, ParseM6809Mnemonic("LBRA"));
  EXPECT_EQ(M6809Mnemonic::LBEQ, ParseM6809Mnemonic("LBEQ"));
  EXPECT_EQ(M6809Mnemonic::LBLT, ParseM6809Mnemonic("LBLT"));
}

TEST(MnemonicEnumTest, M6809_ParseInherent) {
  EXPECT_EQ(M6809Mnemonic::NOP,  ParseM6809Mnemonic("NOP"));
  EXPECT_EQ(M6809Mnemonic::RTS,  ParseM6809Mnemonic("RTS"));
  EXPECT_EQ(M6809Mnemonic::CLRA, ParseM6809Mnemonic("CLRA"));
  EXPECT_EQ(M6809Mnemonic::DAA,  ParseM6809Mnemonic("DAA"));
  EXPECT_EQ(M6809Mnemonic::MUL,  ParseM6809Mnemonic("MUL"));
}

TEST(MnemonicEnumTest, M6809_ParseUnknown) {
  EXPECT_EQ(M6809Mnemonic::Unknown, ParseM6809Mnemonic(""));
  EXPECT_EQ(M6809Mnemonic::Unknown, ParseM6809Mnemonic("FOOBAR"));
  EXPECT_EQ(M6809Mnemonic::Unknown, ParseM6809Mnemonic("lda")); // lowercase
}
