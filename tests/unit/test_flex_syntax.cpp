// FlexAsmSyntax tests
// FLEX ASM09 Assembler Syntax for Motorola 6809

#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/flex_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Basic Parsing
// ============================================================================

TEST(FlexSyntaxTest, CommentWithAsterisk) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("* This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST(FlexSyntaxTest, BlankLines) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("\n\n   \n\t\n", section, symbols);

  // Blank lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

// ============================================================================
// Phase 1: Basic Directives - ORG
// ============================================================================

TEST(FlexSyntaxTest, OrgDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        ORG     $0400", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x0400U);
}

// ============================================================================
// Phase 1: Basic Directives - EQU
// ============================================================================

TEST(FlexSyntaxTest, EquDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("BUFSIZE EQU     256", section, symbols);

  // EQU creates symbol but no atom
  EXPECT_TRUE(symbols.IsDefined("BUFSIZE"));
  EXPECT_EQ(section.atoms.size(), 0UL);

  // Verify value
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("BUFSIZE", value));
  EXPECT_EQ(value, 256);
}

// ============================================================================
// Phase 1: Basic Directives - SET (Variable Symbol)
// ============================================================================

TEST(FlexSyntaxTest, SetDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("COUNT   SET     0", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("COUNT"));
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("COUNT", value));
  EXPECT_EQ(value, 0);
}

TEST(FlexSyntaxTest, SetDirectiveReassignment) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("COUNT   SET     0\nCOUNT   SET     5", section, symbols);

  // SET allows redefinition (unlike EQU)
  EXPECT_TRUE(symbols.IsDefined("COUNT"));
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("COUNT", value));
  EXPECT_EQ(value, 5);
}

// ============================================================================
// Phase 1: Data Definition - FCB
// ============================================================================

TEST(FlexSyntaxTest, FcbDirectiveSingle) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCB     $FF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Data);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1UL);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(FlexSyntaxTest, FcbDirectiveMultiple) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCB     1,2,3,4,5", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5UL);
  EXPECT_EQ(data_atom->data[0], 1U);
  EXPECT_EQ(data_atom->data[4], 5U);
}

// ============================================================================
// Phase 1: Data Definition - FDB
// ============================================================================

TEST(FlexSyntaxTest, FdbDirectiveSingle) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FDB     $1234", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2UL);
  // 6809 uses big-endian (MSB first)
  EXPECT_EQ(data_atom->data[0], 0x12); // High byte
  EXPECT_EQ(data_atom->data[1], 0x34); // Low byte
}

// ============================================================================
// Phase 1: Data Definition - FCC
// ============================================================================

TEST(FlexSyntaxTest, FccDirectiveSlashDelimiter) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCC     /Hello/", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5UL);
  EXPECT_EQ(data_atom->data[0], 'H');
  EXPECT_EQ(data_atom->data[4], 'o');
}

// ============================================================================
// Phase 1: Data Definition - RMB
// ============================================================================

TEST(FlexSyntaxTest, RmbDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("BUFFER  RMB     256", section, symbols);

  // RMB creates a Space atom (reserve memory bytes)
  ASSERT_EQ(section.atoms.size(), 2UL); // Label + Space
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Space);

  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[1]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 256UL);
  EXPECT_EQ(space_atom->size, 256UL);
}

// ============================================================================
// Phase 1: Listing Control - NAM/TTL
// ============================================================================

TEST(FlexSyntaxTest, NamDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        NAM     My Program", section, symbols);

  // NAM sets title but produces no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST(FlexSyntaxTest, TtlDirective) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        TTL     My Program", section, symbols);

  // TTL is synonym for NAM
  EXPECT_EQ(section.atoms.size(), 0UL);
}
