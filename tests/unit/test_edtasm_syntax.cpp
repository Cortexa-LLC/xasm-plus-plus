// EdtasmSyntaxParser tests
// TRS-80 Color Computer EDTASM+ Assembler Syntax

#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/edtasm_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Comment Parsing
// ============================================================================

TEST(EdtasmSyntaxTest, CommentWithAsterisk) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("* This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST(EdtasmSyntaxTest, CommentWithSemicolon) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        LDA     #10     ; Inline comment", section, symbols);

  // Should create instruction atom, ignoring comment
  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Instruction);
}

TEST(EdtasmSyntaxTest, BlankLines) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("\n\n   \n\t\n", section, symbols);

  // Blank lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

// ============================================================================
// Phase 1: Foundation - Label Parsing
// ============================================================================

TEST(EdtasmSyntaxTest, GlobalLabel) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("START", section, symbols);

  // Should create label symbol and LabelAtom
  EXPECT_TRUE(symbols.IsDefined("START"));

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);

  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(atom);
  ASSERT_NE(label_atom, nullptr);
  EXPECT_EQ(label_atom->name, "START");
}

TEST(EdtasmSyntaxTest, LabelWithInstruction) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("LOOP    LDA     #$00", section, symbols);

  // Should create label and instruction atoms
  EXPECT_TRUE(symbols.IsDefined("LOOP"));
  ASSERT_EQ(section.atoms.size(), 2UL);

  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}

// ============================================================================
// Phase 1: Core Directives - ORG
// ============================================================================

TEST(EdtasmSyntaxTest, OrgDirective) {
  EdtasmSyntaxParser parser;
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

TEST(EdtasmSyntaxTest, OrgDirectiveDecimal) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        ORG     1024", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 1024U);
}

// ============================================================================
// Phase 1: Core Directives - END
// ============================================================================

TEST(EdtasmSyntaxTest, EndDirective) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        END", section, symbols);

  // END should produce no atoms (signals end of assembly)
  EXPECT_EQ(section.atoms.size(), 0UL);
}

TEST(EdtasmSyntaxTest, EndDirectiveWithEntryPoint) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // First define START label
  parser.Parse("START   NOP\n        END     START", section, symbols);

  // Should have NOP instruction, no END atom
  ASSERT_EQ(section.atoms.size(), 2UL); // Label + Instruction
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}

// ============================================================================
// Phase 1: Core Directives - EQU (Equate)
// ============================================================================

TEST(EdtasmSyntaxTest, EquDirective) {
  EdtasmSyntaxParser parser;
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

TEST(EdtasmSyntaxTest, EquDirectiveHex) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("SCREEN  EQU     $0400", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("SCREEN"));
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("SCREEN", value));
  EXPECT_EQ(value, 0x0400);
}

// ============================================================================
// Phase 1: Core Directives - SET (Variable)
// ============================================================================

TEST(EdtasmSyntaxTest, SetDirective) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("COUNT   SET     0", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("COUNT"));
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("COUNT", value));
  EXPECT_EQ(value, 0);
}

TEST(EdtasmSyntaxTest, SetDirectiveRedefine) {
  EdtasmSyntaxParser parser;
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
// Phase 1: Core Directives - FCB (Form Constant Byte)
// ============================================================================

TEST(EdtasmSyntaxTest, FcbDirectiveSingle) {
  EdtasmSyntaxParser parser;
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

TEST(EdtasmSyntaxTest, FcbDirectiveMultiple) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCB     1,2,3,4,5", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5UL);
  EXPECT_EQ(data_atom->data[0], 1U);
  EXPECT_EQ(data_atom->data[1], 2U);
  EXPECT_EQ(data_atom->data[4], 5U);
}

TEST(EdtasmSyntaxTest, FcbDirectiveCharacter) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCB     'A'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1UL);
  EXPECT_EQ(data_atom->data[0], 'A');
}

// ============================================================================
// Phase 1: Core Directives - FDB (Form Double Byte)
// ============================================================================

TEST(EdtasmSyntaxTest, FdbDirectiveSingle) {
  EdtasmSyntaxParser parser;
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

TEST(EdtasmSyntaxTest, FdbDirectiveMultiple) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FDB     $1000,$2000", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4UL);
  EXPECT_EQ(data_atom->data[0], 0x10);
  EXPECT_EQ(data_atom->data[1], 0x00);
  EXPECT_EQ(data_atom->data[2], 0x20);
  EXPECT_EQ(data_atom->data[3], 0x00);
}

// ============================================================================
// Phase 1: Core Directives - FCC (Form Constant Characters)
// ============================================================================

TEST(EdtasmSyntaxTest, FccDirectiveSlashDelimiter) {
  EdtasmSyntaxParser parser;
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

TEST(EdtasmSyntaxTest, FccDirectiveQuoteDelimiter) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        FCC     \"World\"", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5UL);
  EXPECT_EQ(data_atom->data[0], 'W');
  EXPECT_EQ(data_atom->data[4], 'd');
}

// ============================================================================
// Phase 1: Core Directives - RMB (Reserve Memory Bytes)
// ============================================================================

TEST(EdtasmSyntaxTest, RmbDirective) {
  EdtasmSyntaxParser parser;
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
// Phase 1: Core Directives - SETDP (Set Direct Page)
// ============================================================================

TEST(EdtasmSyntaxTest, SetdpDirective) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("        SETDP   $10", section, symbols);

  // SETDP should produce no atoms (assembler directive only)
  EXPECT_EQ(section.atoms.size(), 0UL);
}

// ============================================================================
// Phase 1: Integration - Complete Program
// ============================================================================

TEST(EdtasmSyntaxTest, CompleteProgram) {
  EdtasmSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = R"(
* Example EDTASM program
        ORG     $0400

SCREEN  EQU     $0400

START   LDA     #$00
        STA     SCREEN
        RTS

        END     START
)";

  parser.Parse(source, section, symbols);

  // Should have: ORG, Label, LDA, STA, RTS atoms
  ASSERT_GE(section.atoms.size(), 5UL);

  // Verify symbols
  EXPECT_TRUE(symbols.IsDefined("SCREEN"));
  EXPECT_TRUE(symbols.IsDefined("START"));
}
