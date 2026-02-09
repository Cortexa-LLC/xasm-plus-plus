// Z80 Universal Syntax Parser tests
// Universal superset supporting M80, ZMAC, and Z80ASM

#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/z80_universal_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Comment Parsing
// ============================================================================

TEST(Z80UniversalSyntaxTest, CommentWithSemicolon) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("; This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80UniversalSyntaxTest, InlineComment) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG 8000H  ; Set origin", section, symbols);

  // Should create ORG atom, ignoring comment
  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);
}

// ============================================================================
// Phase 1: Foundation - Global Label Parsing
// ============================================================================

TEST(Z80UniversalSyntaxTest, GlobalLabelPrivate) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("START:", section, symbols);

  // Should create label symbol and LabelAtom
  EXPECT_TRUE(symbols.IsDefined("START"));

  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);

  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(atom);
  ASSERT_NE(label_atom, nullptr);
  EXPECT_EQ(label_atom->name, "START");
}

TEST(Z80UniversalSyntaxTest, GlobalLabelPublic) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("START::", section, symbols);

  // Should create public label (M80 style)
  EXPECT_TRUE(symbols.IsDefined("START"));

  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);
}

TEST(Z80UniversalSyntaxTest, LocalLabelZ80ASM) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "MAIN:\n"
                       "$LOOP:\n"
                       "  LD A,1\n"
                       "  JP $LOOP\n";

  parser.Parse(source, section, symbols);

  // Should create MAIN and local label $LOOP
  EXPECT_TRUE(symbols.IsDefined("MAIN"));
  // Local labels are scoped to parent
}

TEST(Z80UniversalSyntaxTest, LocalLabelZMAC) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "MAIN:\n"
                       ".loop:\n"
                       "  LD A,1\n"
                       "  JP .loop\n";

  parser.Parse(source, section, symbols);

  // Should create MAIN and local label .loop (ZMAC style)
  EXPECT_TRUE(symbols.IsDefined("MAIN"));
}

// ============================================================================
// Phase 1: Foundation - ORG Directive
// ============================================================================

TEST(Z80UniversalSyntaxTest, OrgDirectiveHexH) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG 8000H", section, symbols);

  // Should create OrgAtom
  ASSERT_EQ(section.atoms.size(), 1);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000);
}

TEST(Z80UniversalSyntaxTest, OrgDirectiveDollar) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG $8000", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000);
}

TEST(Z80UniversalSyntaxTest, OrgDirectiveDecimal) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG 32768", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 32768);
}

// ============================================================================
// Phase 1: Foundation - Number Formats
// ============================================================================

TEST(Z80UniversalSyntaxTest, HexNumberDollarPrefix) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $FF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(Z80UniversalSyntaxTest, HexNumberHSuffix) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 0FFH", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(Z80UniversalSyntaxTest, HexNumber0xPrefix) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 0xFF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(Z80UniversalSyntaxTest, BinaryNumberBSuffix) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 11110000B", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xF0);
}

TEST(Z80UniversalSyntaxTest, OctalNumberOSuffix) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 377O", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF); // 377 octal = FF hex
}

TEST(Z80UniversalSyntaxTest, DecimalNumber) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 255", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 255);
}

// ============================================================================
// Phase 1: Foundation - EQU Directive
// ============================================================================

TEST(Z80UniversalSyntaxTest, EquDirective) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CONST    EQU $FF", section, symbols);

  // Should define symbol, but create no atoms
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80UniversalSyntaxTest, EqualsDirective) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CONST = $FF", section, symbols);

  // Should define symbol using = syntax
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 1: Foundation - DB/DEFB/BYTE Directives
// ============================================================================

TEST(Z80UniversalSyntaxTest, DbSingleByte) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80UniversalSyntaxTest, DefbSingleByte) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80UniversalSyntaxTest, DbMultipleBytes) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $01,$02,$03", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
}

// ============================================================================
// Phase 1: Foundation - DW/DEFW Directives
// ============================================================================

TEST(Z80UniversalSyntaxTest, DwSingleWord) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DW $1234", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2);
  EXPECT_EQ(data_atom->data[0], 0x34); // Low byte (little-endian)
  EXPECT_EQ(data_atom->data[1], 0x12); // High byte
}

TEST(Z80UniversalSyntaxTest, DefwSingleWord) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFW $ABCD", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2);
  EXPECT_EQ(data_atom->data[0], 0xCD); // Low byte
  EXPECT_EQ(data_atom->data[1], 0xAB); // High byte
}

// ============================================================================
// Phase 1: Foundation - DS/DEFS Directives
// ============================================================================

TEST(Z80UniversalSyntaxTest, DsReserveSpace) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DS 10", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 10);
}

TEST(Z80UniversalSyntaxTest, DefsReserveSpace) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFS 20", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 20);
}

// ============================================================================
// Phase 1: Foundation - END Directive
// ============================================================================

TEST(Z80UniversalSyntaxTest, EndDirective) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         END", section, symbols);

  // END directive should be a no-op - no atoms generated
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80UniversalSyntaxTest, EndDirectiveIgnoresAfter) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         DB $01\n"
                       "         END\n"
                       "         DB $02\n";

  parser.Parse(source, section, symbols);

  // Should only have first DB - code after END should be ignored
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x01);
}

// ============================================================================
// Phase 2: PUBLIC/GLOBAL/ENTRY Directives (Symbol Export)
// ============================================================================

TEST(Z80UniversalSyntaxTest, PublicSingleSymbol) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "START:   LD A,1\n"
                       "         PUBLIC START\n";

  parser.Parse(source, section, symbols);

  // Symbol should exist and be marked as exported
  EXPECT_TRUE(symbols.IsDefined("START"));
  auto symbol = symbols.GetSymbol("START");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_exported);
}

TEST(Z80UniversalSyntaxTest, PublicMultipleSymbols) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "SYM1:    NOP\n"
                       "SYM2:    NOP\n"
                       "SYM3:    NOP\n"
                       "         PUBLIC SYM1, SYM2, SYM3\n";

  parser.Parse(source, section, symbols);

  // All three symbols should be marked as exported
  auto sym1 = symbols.GetSymbol("SYM1");
  auto sym2 = symbols.GetSymbol("SYM2");
  auto sym3 = symbols.GetSymbol("SYM3");

  ASSERT_NE(sym1, nullptr);
  ASSERT_NE(sym2, nullptr);
  ASSERT_NE(sym3, nullptr);

  EXPECT_TRUE(sym1->is_exported);
  EXPECT_TRUE(sym2->is_exported);
  EXPECT_TRUE(sym3->is_exported);
}

TEST(Z80UniversalSyntaxTest, GlobalSynonymForPublic) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "START:   NOP\n"
                       "         GLOBAL START\n";

  parser.Parse(source, section, symbols);

  // GLOBAL should work identically to PUBLIC
  auto symbol = symbols.GetSymbol("START");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_exported);
}

TEST(Z80UniversalSyntaxTest, EntrySynonymForPublic) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "START:   NOP\n"
                       "         ENTRY START\n";

  parser.Parse(source, section, symbols);

  // ENTRY should work identically to PUBLIC
  auto symbol = symbols.GetSymbol("START");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_exported);
}

// ============================================================================
// Phase 2: EXTERN/EXTRN/EXT Directives (Symbol Import)
// ============================================================================

TEST(Z80UniversalSyntaxTest, ExternSingleSymbol) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXTERN PRINTF\n", section, symbols);

  // Symbol should exist and be marked as imported
  EXPECT_TRUE(symbols.IsDefined("PRINTF"));
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}

TEST(Z80UniversalSyntaxTest, ExternMultipleSymbols) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXTERN FUNC1, FUNC2, FUNC3\n", section, symbols);

  // All three symbols should be marked as imported
  auto func1 = symbols.GetSymbol("FUNC1");
  auto func2 = symbols.GetSymbol("FUNC2");
  auto func3 = symbols.GetSymbol("FUNC3");

  ASSERT_NE(func1, nullptr);
  ASSERT_NE(func2, nullptr);
  ASSERT_NE(func3, nullptr);

  EXPECT_TRUE(func1->is_imported);
  EXPECT_TRUE(func2->is_imported);
  EXPECT_TRUE(func3->is_imported);
}

TEST(Z80UniversalSyntaxTest, ExtrnSynonymForExtern) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXTRN PRINTF\n", section, symbols);

  // EXTRN should work identically to EXTERN
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}

TEST(Z80UniversalSyntaxTest, ExtSynonymForExtern) {
  Z80UniversalSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXT PRINTF\n", section, symbols);

  // EXT should work identically to EXTERN
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}
