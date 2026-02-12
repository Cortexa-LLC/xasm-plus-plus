// EDTASM-M80++ Syntax Parser tests
// Universal superset supporting M80, ZMAC, and Z80ASM

#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Comment Parsing
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, CommentWithSemicolon) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("; This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, InlineComment) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, GlobalLabelPrivate) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, GlobalLabelPublic) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("START::", section, symbols);

  // Should create public label (M80 style)
  EXPECT_TRUE(symbols.IsDefined("START"));

  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);
}

TEST(EdtasmM80PlusPlusSyntaxTest, LocalLabelZ80ASM) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, LocalLabelZMAC) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, OrgDirectiveHexH) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, OrgDirectiveDollar) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG $8000", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000);
}

TEST(EdtasmM80PlusPlusSyntaxTest, OrgDirectiveDecimal) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, HexNumberDollarPrefix) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $FF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, HexNumberHSuffix) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 0FFH", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, HexNumber0xPrefix) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 0xFF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, BinaryNumberBSuffix) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 11110000B", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xF0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, OctalNumberOSuffix) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 377O", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF); // 377 octal = FF hex
}

TEST(EdtasmM80PlusPlusSyntaxTest, DecimalNumber) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, EquDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CONST    EQU $FF", section, symbols);

  // Should define symbol, but create no atoms
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, EqualsDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, DbSingleByte) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, DefbSingleByte) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbMultipleBytes) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, DwSingleWord) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DW $1234", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  
  // DW now stores as expressions for consistent forward/backward ref handling
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "$1234");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

TEST(EdtasmM80PlusPlusSyntaxTest, DefwSingleWord) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFW $ABCD", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  
  // DEFW now stores as expressions for consistent forward/backward ref handling
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "$ABCD");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

// ============================================================================
// Phase 1: Foundation - DS/DEFS Directives
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, DsReserveSpace) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DS 10", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 10);
}

TEST(EdtasmM80PlusPlusSyntaxTest, DefsReserveSpace) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, EndDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         END", section, symbols);

  // END directive should be a no-op - no atoms generated
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, EndDirectiveIgnoresAfter) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, PublicSingleSymbol) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, PublicMultipleSymbols) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, GlobalSynonymForPublic) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, EntrySynonymForPublic) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, ExternSingleSymbol) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXTERN PRINTF\n", section, symbols);

  // Symbol should exist and be marked as imported
  EXPECT_TRUE(symbols.IsDefined("PRINTF"));
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}

TEST(EdtasmM80PlusPlusSyntaxTest, ExternMultipleSymbols) {
  EdtasmM80PlusPlusSyntaxParser parser;
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

TEST(EdtasmM80PlusPlusSyntaxTest, ExtrnSynonymForExtern) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXTRN PRINTF\n", section, symbols);

  // EXTRN should work identically to EXTERN
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}

TEST(EdtasmM80PlusPlusSyntaxTest, ExtSynonymForExtern) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         EXT PRINTF\n", section, symbols);

  // EXT should work identically to EXTERN
  auto symbol = symbols.GetSymbol("PRINTF");
  ASSERT_NE(symbol, nullptr);
  EXPECT_TRUE(symbol->is_imported);
}

// ============================================================================
// Phase 7: Conditional Assembly - IF/ELSE/ENDIF
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IfEndifTrueCondition) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include the DB since condition is true
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfEndifFalseCondition) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 0\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude the DB since condition is false
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfElseEndifTrueBranch) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         DB $42\n"
                       "         ELSE\n"
                       "         DB $99\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include first DB (true branch)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfElseEndifFalseBranch) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 0\n"
                       "         DB $42\n"
                       "         ELSE\n"
                       "         DB $99\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include second DB (else branch)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x99);
}

// ============================================================================
// Phase 7: Conditional Assembly - IFDEF/IFNDEF
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IfdefSymbolDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "SYMBOL   EQU 1\n"
                       "         IFDEF SYMBOL\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since SYMBOL is defined
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfdefSymbolUndefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFDEF SYMBOL\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since SYMBOL is not defined
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfndefSymbolUndefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFNDEF SYMBOL\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since SYMBOL is not defined
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfndefSymbolDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "SYMBOL   EQU 1\n"
                       "         IFNDEF SYMBOL\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since SYMBOL is defined
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Comparison Conditional Tests
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IfeqZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFEQ 0\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since 0 == 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfeqNonZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFEQ 5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since 5 != 0
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfneZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFNE 0\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since 0 == 0 (not != 0)
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfneNonZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFNE 5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since 5 != 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfltNegative) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFLT -5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since -5 < 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfltPositive) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFLT 5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since 5 >= 0
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfgtPositive) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFGT 5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since 5 > 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfgtNegative) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFGT -5\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since -5 <= 0
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfleZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFLE 0\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since 0 <= 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfgeZero) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFGE 0\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since 0 >= 0
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Pass-Dependent Conditional Tests
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, If1AlwaysFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF1\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since this is a single-pass assembler
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, If2AlwaysFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF2\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since this is a single-pass assembler
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Blank/Non-Blank Conditional Tests
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IfbEmpty) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFB\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since operand is blank
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfbNonEmpty) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFB SOMETHING\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since operand is not blank
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfnbNonEmpty) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFNB SOMETHING\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since operand is not blank
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfnbEmpty) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFNB\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since operand is blank
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// String Comparison Conditional Tests
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IfidnIdentical) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFIDN ABC,ABC\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since strings are identical
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfidnCaseInsensitive) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFIDN ABC,abc\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since comparison is case-insensitive
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfidnDifferent) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFIDN ABC,XYZ\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since strings are different
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfdifDifferent) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFDIF ABC,XYZ\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since strings are different
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IfdifIdentical) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IFDIF ABC,ABC\n"
                       "         DB $42\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since strings are identical
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Nested Conditional Tests
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, NestedIfBothTrue) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         IF 1\n"
                       "         DB $42\n"
                       "         ENDIF\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since both conditions are true
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, NestedIfOuterFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 0\n"
                       "         IF 1\n"
                       "         DB $42\n"
                       "         ENDIF\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since outer condition is false
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, NestedIfInnerFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         IF 0\n"
                       "         DB $42\n"
                       "         ENDIF\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should exclude DB since inner condition is false
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, NestedIfElse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         IF 0\n"
                       "         DB $11\n"
                       "         ELSE\n"
                       "         DB $22\n"
                       "         ENDIF\n"
                       "         DB $33\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB $22 and DB $33 (outer true, inner false -> else, outer continues)
  ASSERT_EQ(section.atoms.size(), 2);
  auto data_atom1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom1, nullptr);
  EXPECT_EQ(data_atom1->data[0], 0x22);
  
  auto data_atom2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom2, nullptr);
  EXPECT_EQ(data_atom2->data[0], 0x33);
}

TEST(EdtasmM80PlusPlusSyntaxTest, TripleNested) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IF 1\n"
                       "         IF 1\n"
                       "         IF 1\n"
                       "         DB $42\n"
                       "         ENDIF\n"
                       "         ENDIF\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // Should include DB since all conditions are true
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, MixedConditionals) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "SYMBOL   EQU 1\n"
                       "         IFDEF SYMBOL\n"
                       "         IFGT SYMBOL\n"
                       "         DB $42\n"
                       "         ENDIF\n"
                       "         ENDIF\n";

  parser.Parse(source, section, symbols);

  // SYMBOL is defined and equals 1, so IFDEF true and IFGT (1 > 0) true
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 8: Listing Control Directives
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, ListXlistDirectives) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // .LIST/.XLIST control listing output
  // These directives affect listing but don't generate code
  std::string source = "         .LIST\n"
                       "         DB $42\n"
                       "         .XLIST\n"
                       "         DB $43\n"
                       "         .LIST\n"
                       "         DB $44\n";

  parser.Parse(source, section, symbols);

  // Should generate: LIST atom, DB atom, XLIST atom, DB atom, LIST atom, DB atom = 6 atoms
  ASSERT_EQ(section.atoms.size(), 6);
  
  // Check that listing control atoms are present
  auto list1 = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[0]);
  ASSERT_NE(list1, nullptr);
  EXPECT_EQ(list1->control_type, ListingControlType::List);
  
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 0x42);
  
  auto xlist = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[2]);
  ASSERT_NE(xlist, nullptr);
  EXPECT_EQ(xlist->control_type, ListingControlType::Nolist);
  
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data2, nullptr);
  EXPECT_EQ(data2->data[0], 0x43);
  
  auto list2 = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[4]);
  ASSERT_NE(list2, nullptr);
  EXPECT_EQ(list2->control_type, ListingControlType::List);
  
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[5]);
  ASSERT_NE(data3, nullptr);
  EXPECT_EQ(data3->data[0], 0x44);
}

TEST(EdtasmM80PlusPlusSyntaxTest, StarListDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // *LIST ON/OFF is Z80ASM style listing control
  std::string source = "         *LIST ON\n"
                       "         DB $42\n"
                       "         *LIST OFF\n"
                       "         DB $43\n"
                       "         *LIST ON\n"
                       "         DB $44\n";

  parser.Parse(source, section, symbols);

  // Should generate: *LIST atom, DB atom, *LIST atom, DB atom, *LIST atom, DB atom = 6 atoms
  ASSERT_EQ(section.atoms.size(), 6);
  
  // Check that listing control atoms are present
  auto list1 = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[0]);
  ASSERT_NE(list1, nullptr);
  EXPECT_EQ(list1->control_type, ListingControlType::List);
  
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 0x42);
  
  auto list_off = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[2]);
  ASSERT_NE(list_off, nullptr);
  EXPECT_EQ(list_off->control_type, ListingControlType::Nolist);
  
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data2, nullptr);
  EXPECT_EQ(data2->data[0], 0x43);
  
  auto list2 = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[4]);
  ASSERT_NE(list2, nullptr);
  EXPECT_EQ(list2->control_type, ListingControlType::List);
  
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[5]);
  ASSERT_NE(data3, nullptr);
  EXPECT_EQ(data3->data[0], 0x44);
}

TEST(EdtasmM80PlusPlusSyntaxTest, TitleDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // .TITLE sets listing title
  std::string source = "         .TITLE \"My Program\"\n"
                       "         DB $42\n";

  parser.Parse(source, section, symbols);

  // Should create title atom and data atom
  ASSERT_EQ(section.atoms.size(), 2);
  auto title_atom = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[0]);
  ASSERT_NE(title_atom, nullptr);
  EXPECT_EQ(title_atom->control_type, ListingControlType::Title);
  
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, SubttlDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // .SUBTTL sets subtitle
  std::string source = "         .SUBTTL \"Initialization\"\n"
                       "         DB $42\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 2);
  auto subtitle_atom = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[0]);
  ASSERT_NE(subtitle_atom, nullptr);
  EXPECT_EQ(subtitle_atom->control_type, ListingControlType::Subtitle);
  
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, EjectDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // EJECT forces page break in listing
  std::string source = "         DB $42\n"
                       "         EJECT\n"
                       "         DB $43\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 0x42);
  
  auto eject_atom = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[1]);
  ASSERT_NE(eject_atom, nullptr);
  EXPECT_EQ(eject_atom->control_type, ListingControlType::Page);
  
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  ASSERT_NE(data2, nullptr);
  EXPECT_EQ(data2->data[0], 0x43);
}

TEST(EdtasmM80PlusPlusSyntaxTest, SpaceDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // SPACE inserts blank lines in listing
  std::string source = "         DB $42\n"
                       "         SPACE 3\n"
                       "         DB $43\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 0x42);
  
  auto space_atom = std::dynamic_pointer_cast<ListingControlAtom>(section.atoms[1]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->control_type, ListingControlType::Space);
  
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  ASSERT_NE(data2, nullptr);
  EXPECT_EQ(data2->data[0], 0x43);
}

TEST(EdtasmM80PlusPlusSyntaxTest, NameDirective) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // NAME sets module name
  std::string source = "         NAME MYMODULE\n"
                       "         DB $42\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 9: Special Features - RADIX Control
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, RadixBinary) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Set radix to binary (base 2)
  std::string source = "         *RADIX 2\n"
                       "         DB 11110000\n"; // Binary without suffix

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xF0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, RadixOctal) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Set radix to octal (base 8)
  std::string source = "         .RADIX 8\n"
                       "         DB 377\n"; // Octal without suffix

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, RadixDecimal) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Set radix to decimal (base 10) - default
  std::string source = "         *RADIX 10\n"
                       "         DB 255\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 255);
}

TEST(EdtasmM80PlusPlusSyntaxTest, RadixHexadecimal) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Set radix to hexadecimal (base 16)
  std::string source = "         .RADIX 16\n"
                       "         DB FF\n"; // Hex without prefix

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, RadixChangeMidAssembly) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Change radix mid-assembly
  std::string source = "         *RADIX 16\n"
                       "         DB FF\n"      // Hex
                       "         *RADIX 10\n"
                       "         DB 42\n"      // Decimal
                       "         *RADIX 2\n"
                       "         DB 101010\n"; // Binary

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  ASSERT_NE(data3, nullptr);
  
  EXPECT_EQ(data1->data[0], 0xFF);
  EXPECT_EQ(data2->data[0], 42);
  EXPECT_EQ(data3->data[0], 0x2A); // 101010 binary = 42 decimal
}

// ============================================================================
// Phase 9: Special Features - DATE/TIME Pseudo-ops
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, DatePseudoOp) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DATE should define a symbol with current date (YYYYMMDD format)
  std::string source = "BUILDDATE EQU DATE\n";

  parser.Parse(source, section, symbols);

  // Symbol should be defined
  EXPECT_TRUE(symbols.IsDefined("BUILDDATE"));
}

TEST(EdtasmM80PlusPlusSyntaxTest, TimePseudoOp) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // TIME should define a symbol with current time (HHMMSS format)
  std::string source = "BUILDTIME EQU TIME\n";

  parser.Parse(source, section, symbols);

  // Symbol should be defined
  EXPECT_TRUE(symbols.IsDefined("BUILDTIME"));
}

// ============================================================================
// Phase 9: Special Features - Undocumented Z-80 Instructions
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, UndocumentedIxhRegister) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // LD A,ixh - Load A from high byte of IX
  std::string source = "         LD A,ixh\n";

  parser.Parse(source, section, symbols);

  // Should create instruction atom (encoding verified by CPU plugin tests)
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LD");
  EXPECT_EQ(inst_atom->operand, "A,ixh");
}

TEST(EdtasmM80PlusPlusSyntaxTest, UndocumentedIxlRegister) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // LD B,ixl - Load B from low byte of IX
  std::string source = "         LD B,ixl\n";

  parser.Parse(source, section, symbols);

  // Should create instruction atom (encoding verified by CPU plugin tests)
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LD");
  EXPECT_EQ(inst_atom->operand, "B,ixl");
}

TEST(EdtasmM80PlusPlusSyntaxTest, UndocumentedIyhRegister) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // LD C,iyh - Load C from high byte of IY
  std::string source = "         LD C,iyh\n";

  parser.Parse(source, section, symbols);

  // Should create instruction atom (encoding verified by CPU plugin tests)
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LD");
  EXPECT_EQ(inst_atom->operand, "C,iyh");
}

TEST(EdtasmM80PlusPlusSyntaxTest, UndocumentedIylRegister) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // LD D,iyl - Load D from low byte of IY
  std::string source = "         LD D,iyl\n";

  parser.Parse(source, section, symbols);

  // Should create instruction atom (encoding verified by CPU plugin tests)
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LD");
  EXPECT_EQ(inst_atom->operand, "D,iyl");
}

TEST(EdtasmM80PlusPlusSyntaxTest, UndocumentedSl1Instruction) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // SL1 B - Shift left, fill with 1 (undocumented)
  std::string source = "         SL1 B\n";

  parser.Parse(source, section, symbols);

  // Should create instruction atom (encoding verified by CPU plugin tests)
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "SL1");
  EXPECT_EQ(inst_atom->operand, "B");
}

// ============================================================================
// Phase 11: Data Directives - String Literals and Escape Sequences
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, DbStringSingleQuote) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'Hello'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 'H');
  EXPECT_EQ(data_atom->data[1], 'e');
  EXPECT_EQ(data_atom->data[2], 'l');
  EXPECT_EQ(data_atom->data[3], 'l');
  EXPECT_EQ(data_atom->data[4], 'o');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbStringDoubleQuote) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB \"World\"", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 'W');
  EXPECT_EQ(data_atom->data[1], 'o');
  EXPECT_EQ(data_atom->data[2], 'r');
  EXPECT_EQ(data_atom->data[3], 'l');
  EXPECT_EQ(data_atom->data[4], 'd');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbMixedStringAndNumbers) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'Hi',13,10,0", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 'H');
  EXPECT_EQ(data_atom->data[1], 'i');
  EXPECT_EQ(data_atom->data[2], 13);  // CR
  EXPECT_EQ(data_atom->data[3], 10);  // LF
  EXPECT_EQ(data_atom->data[4], 0);   // NULL
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeNewline) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'Line1\\nLine2'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 11);
  EXPECT_EQ(data_atom->data[0], 'L');
  EXPECT_EQ(data_atom->data[1], 'i');
  EXPECT_EQ(data_atom->data[2], 'n');
  EXPECT_EQ(data_atom->data[3], 'e');
  EXPECT_EQ(data_atom->data[4], '1');
  EXPECT_EQ(data_atom->data[5], '\n');  // Escape sequence
  EXPECT_EQ(data_atom->data[6], 'L');
  EXPECT_EQ(data_atom->data[7], 'i');
  EXPECT_EQ(data_atom->data[8], 'n');
  EXPECT_EQ(data_atom->data[9], 'e');
  EXPECT_EQ(data_atom->data[10], '2');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeCarriageReturn) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'CR:\\r'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 'C');
  EXPECT_EQ(data_atom->data[1], 'R');
  EXPECT_EQ(data_atom->data[2], ':');
  EXPECT_EQ(data_atom->data[3], '\r');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeTab) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'A\\tB'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3);
  EXPECT_EQ(data_atom->data[0], 'A');
  EXPECT_EQ(data_atom->data[1], '\t');
  EXPECT_EQ(data_atom->data[2], 'B');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeBackslash) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'C:\\\\path'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 7);
  EXPECT_EQ(data_atom->data[0], 'C');
  EXPECT_EQ(data_atom->data[1], ':');
  EXPECT_EQ(data_atom->data[2], '\\');
  EXPECT_EQ(data_atom->data[3], 'p');
  EXPECT_EQ(data_atom->data[4], 'a');
  EXPECT_EQ(data_atom->data[5], 't');
  EXPECT_EQ(data_atom->data[6], 'h');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeSingleQuote) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'It\\'s'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 'I');
  EXPECT_EQ(data_atom->data[1], 't');
  EXPECT_EQ(data_atom->data[2], '\'');
  EXPECT_EQ(data_atom->data[3], 's');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEscapeDoubleQuote) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB \"Say \\\"Hi\\\"\"", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 8);
  EXPECT_EQ(data_atom->data[0], 'S');
  EXPECT_EQ(data_atom->data[1], 'a');
  EXPECT_EQ(data_atom->data[2], 'y');
  EXPECT_EQ(data_atom->data[3], ' ');
  EXPECT_EQ(data_atom->data[4], '"');
  EXPECT_EQ(data_atom->data[5], 'H');
  EXPECT_EQ(data_atom->data[6], 'i');
  EXPECT_EQ(data_atom->data[7], '"');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbEmptyString) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB ''", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(), 0);  // Empty string produces no bytes
}

TEST(EdtasmM80PlusPlusSyntaxTest, DbMultipleStringsWithEscapes) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 'Hello\\n','World\\n',0", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 13);
  // 'Hello\n' = 6 bytes
  EXPECT_EQ(data_atom->data[0], 'H');
  EXPECT_EQ(data_atom->data[5], '\n');
  // 'World\n' = 6 bytes
  EXPECT_EQ(data_atom->data[6], 'W');
  EXPECT_EQ(data_atom->data[11], '\n');
  // NULL terminator
  EXPECT_EQ(data_atom->data[12], 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, DefmAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DEFM 'Message'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 7);
  EXPECT_EQ(data_atom->data[0], 'M');
  EXPECT_EQ(data_atom->data[1], 'e');
  EXPECT_EQ(data_atom->data[2], 's');
  EXPECT_EQ(data_atom->data[3], 's');
  EXPECT_EQ(data_atom->data[4], 'a');
  EXPECT_EQ(data_atom->data[5], 'g');
  EXPECT_EQ(data_atom->data[6], 'e');
}

TEST(EdtasmM80PlusPlusSyntaxTest, TextAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         TEXT 'ASCII'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 'A');
  EXPECT_EQ(data_atom->data[4], 'I');
}

TEST(EdtasmM80PlusPlusSyntaxTest, AsciiAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ASCII 'TEST'", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 'T');
  EXPECT_EQ(data_atom->data[3], 'T');
}

TEST(EdtasmM80PlusPlusSyntaxTest, DspaceAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DSPACE 50", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 50);
}

TEST(EdtasmM80PlusPlusSyntaxTest, BlockAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         BLOCK 100", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 100);
}

TEST(EdtasmM80PlusPlusSyntaxTest, WordAlias) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         WORD $BEEF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  
  // WORD now stores as expressions for consistent forward/backward ref handling
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "$BEEF");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

// ============================================================================
// Phase 12: Macro System - REPT Block
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, ReptSimple) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         REPT 3\n"
                       "         DB $42\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 3 DB atoms
  ASSERT_EQ(section.atoms.size(), 3);
  for (int i = 0; i < 3; ++i) {
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[i]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0x42);
  }
}

TEST(EdtasmM80PlusPlusSyntaxTest, ReptZeroTimes) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         REPT 0\n"
                       "         DB $42\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(EdtasmM80PlusPlusSyntaxTest, ReptMultipleInstructions) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         REPT 2\n"
                       "         NOP\n"
                       "         DB $FF\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 4 atoms: NOP, DB, NOP, DB
  ASSERT_EQ(section.atoms.size(), 4);
  
  auto inst1 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst1, nullptr);
  EXPECT_EQ(inst1->mnemonic, "NOP");
  
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data1, nullptr);
  EXPECT_EQ(data1->data[0], 0xFF);
  
  auto inst2 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[2]);
  ASSERT_NE(inst2, nullptr);
  EXPECT_EQ(inst2->mnemonic, "NOP");
  
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[3]);
  ASSERT_NE(data2, nullptr);
  EXPECT_EQ(data2->data[0], 0xFF);
}

// ============================================================================
// Phase 12: Macro System - IRP Iteration
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IrpSimple) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IRP X,1,2,3\n"
                       "         DB &X\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 3 DB atoms with values 1, 2, 3
  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  ASSERT_NE(data3, nullptr);
  
  EXPECT_EQ(data1->data[0], 1);
  EXPECT_EQ(data2->data[0], 2);
  EXPECT_EQ(data3->data[0], 3);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IrpWithHexValues) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IRP VAL,$10,$20,$30\n"
                       "         DB &VAL\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 3 DB atoms with hex values
  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  ASSERT_NE(data3, nullptr);
  
  EXPECT_EQ(data1->data[0], 0x10);
  EXPECT_EQ(data2->data[0], 0x20);
  EXPECT_EQ(data3->data[0], 0x30);
}

TEST(EdtasmM80PlusPlusSyntaxTest, IrpEmptyList) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IRP X,\n"
                       "         DB &X\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate no atoms (empty list means zero iterations)
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 12: Macro System - IRPC Character Iteration
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, IrpcSimple) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IRPC C,ABC\n"
                       "         DB '&C'\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 3 DB atoms with 'A', 'B', 'C'
  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  ASSERT_NE(data3, nullptr);
  
  EXPECT_EQ(data1->data[0], 'A');
  EXPECT_EQ(data2->data[0], 'B');
  EXPECT_EQ(data3->data[0], 'C');
}

TEST(EdtasmM80PlusPlusSyntaxTest, IrpcEmptyString) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         IRPC C,\n"
                       "         DB '&C'\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 12: Macro System - MACRO Definition and Invocation
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, MacroDefinitionSimple) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "MYMACRO  MACRO\n"
                       "         DB $42\n"
                       "         ENDM\n"
                       "         MYMACRO\n";

  parser.Parse(source, section, symbols);

  // Should generate 1 DB atom from macro invocation
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, MacroWithParameters) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "LOADREG  MACRO REG,VAL\n"
                       "         LD &REG,&VAL\n"
                       "         ENDM\n"
                       "         LOADREG A,$FF\n";

  parser.Parse(source, section, symbols);

  // Should generate LD instruction with substituted parameters
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LD");
  EXPECT_EQ(inst_atom->operand, "A,$FF");
}

TEST(EdtasmM80PlusPlusSyntaxTest, MacroMultipleInvocations) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "PUTBYTE  MACRO VAL\n"
                       "         DB &VAL\n"
                       "         ENDM\n"
                       "         PUTBYTE $10\n"
                       "         PUTBYTE $20\n"
                       "         PUTBYTE $30\n";

  parser.Parse(source, section, symbols);

  // Should generate 3 DB atoms
  ASSERT_EQ(section.atoms.size(), 3);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data3 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  ASSERT_NE(data3, nullptr);
  
  EXPECT_EQ(data1->data[0], 0x10);
  EXPECT_EQ(data2->data[0], 0x20);
  EXPECT_EQ(data3->data[0], 0x30);
}

// ============================================================================
// Phase 12: Macro System - LOCAL Symbol Support
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, MacroLocalSymbols) {
  EdtasmM80PlusPlusSyntaxParser parser;
  CpuZ80 cpu;
  parser.SetCpu(&cpu);
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "TESTLOOP MACRO\n"
                       "         LOCAL LOOP\n"
                       "LOOP:    NOP\n"
                       "         JP LOOP\n"
                       "         ENDM\n"
                       "         TESTLOOP\n"
                       "         TESTLOOP\n";

  parser.Parse(source, section, symbols);

  // Should generate 4 atoms: NOP, JP, NOP, JP
  // Each invocation should have its own unique LOOP label
  ASSERT_EQ(section.atoms.size(), 4);
  
  auto inst1 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  auto inst2 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  auto inst3 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[2]);
  auto inst4 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[3]);
  
  ASSERT_NE(inst1, nullptr);
  ASSERT_NE(inst2, nullptr);
  ASSERT_NE(inst3, nullptr);
  ASSERT_NE(inst4, nullptr);
  
  EXPECT_EQ(inst1->mnemonic, "NOP");
  EXPECT_EQ(inst2->mnemonic, "JP");
  EXPECT_EQ(inst3->mnemonic, "NOP");
  EXPECT_EQ(inst4->mnemonic, "JP");
}

// ============================================================================
// Phase 12: Macro System - EXITM Early Exit
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, ExitmInRept) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         REPT 5\n"
                       "         DB $42\n"
                       "         EXITM\n"
                       "         DB $99\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate only 1 DB atom (exits after first iteration)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(EdtasmM80PlusPlusSyntaxTest, ExitmInMacro) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "TESTMAC  MACRO\n"
                       "         DB $42\n"
                       "         EXITM\n"
                       "         DB $99\n"
                       "         ENDM\n"
                       "         TESTMAC\n";

  parser.Parse(source, section, symbols);

  // Should generate only 1 DB atom (exits before second DB)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 12: Macro System - Nested Macros
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, NestedMacros) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "INNER    MACRO VAL\n"
                       "         DB &VAL\n"
                       "         ENDM\n"
                       "OUTER    MACRO\n"
                       "         INNER $10\n"
                       "         INNER $20\n"
                       "         ENDM\n"
                       "         OUTER\n";

  parser.Parse(source, section, symbols);

  // Should generate 2 DB atoms
  ASSERT_EQ(section.atoms.size(), 2);
  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  auto data2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);
  
  EXPECT_EQ(data1->data[0], 0x10);
  EXPECT_EQ(data2->data[0], 0x20);
}

TEST(EdtasmM80PlusPlusSyntaxTest, NestedReptBlocks) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         REPT 2\n"
                       "         REPT 2\n"
                       "         DB $42\n"
                       "         ENDM\n"
                       "         ENDM\n";

  parser.Parse(source, section, symbols);

  // Should generate 4 DB atoms (2 * 2)
  ASSERT_EQ(section.atoms.size(), 4);
  for (int i = 0; i < 4; ++i) {
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[i]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data[0], 0x42);
  }
}

// ============================================================================
// Phase 12: Macro System - Parameter Substitution
// ============================================================================

TEST(EdtasmM80PlusPlusSyntaxTest, MacroAmpersandSubstitution) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "MAKEBYTE MACRO VAL\n"
                       "         DB &VAL\n"
                       "         ENDM\n"
                       "         MAKEBYTE $FF\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(EdtasmM80PlusPlusSyntaxTest, MacroPercentSubstitution) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "MAKEWORD MACRO VAL\n"
                       "         DW %VAL+1\n"
                       "         ENDM\n"
                       "         MAKEWORD 100\n";

  parser.Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // % substitution evaluates expression
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "101");
}
