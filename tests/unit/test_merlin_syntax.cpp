// MerlinSyntax Parser tests
// Phases 1-3: Foundation, Local Labels, DUM Blocks
// Phases 4-6: PUT, DFB, DO/FIN

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/merlin_syntax.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Comment Parsing
// ============================================================================

TEST(MerlinSyntaxTest, CommentWithAsterisk) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("* This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, CommentWithSemicolon) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("; This is a comment", section, symbols);

  // Comment lines should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, InlineComment) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG $8000  ; Set origin", section, symbols);

  // Should create ORG atom, ignoring comment
  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);
}

// ============================================================================
// Phase 1: Foundation - Global Label Parsing
// ============================================================================

TEST(MerlinSyntaxTest, GlobalLabel) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("START", section, symbols);

  // Should create label symbol and LabelAtom
  EXPECT_TRUE(symbols.IsDefined("START"));

  ASSERT_EQ(section.atoms.size(), 1);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);

  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(atom);
  ASSERT_NE(label_atom, nullptr);
  EXPECT_EQ(label_atom->name, "START");
}

TEST(MerlinSyntaxTest, GlobalLabelWithInstruction) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("LOOP     LDA #$00", section, symbols);

  // Should create label and instruction atoms
  EXPECT_TRUE(symbols.IsDefined("LOOP"));
  ASSERT_EQ(section.atoms.size(), 2);

  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}

// ============================================================================
// Phase 1: Foundation - ORG Directive
// ============================================================================

TEST(MerlinSyntaxTest, OrgDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ORG $8000", section, symbols);

  // Should create OrgAtom
  ASSERT_EQ(section.atoms.size(), 1);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000);
}

TEST(MerlinSyntaxTest, OrgWithDecimal) {
  MerlinSyntaxParser parser;
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

TEST(MerlinSyntaxTest, HexNumber) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $FF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(MerlinSyntaxTest, BinaryNumber) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB %11110000", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xF0);
}

TEST(MerlinSyntaxTest, DecimalNumber) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB 42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 42);
}

// ============================================================================
// Phase 1: Foundation - EQU Directive
// ============================================================================

TEST(MerlinSyntaxTest, EquDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CONST    EQU $FF", section, symbols);

  // Should define symbol, but create no atoms
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, EquWithExpression) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("VALUE    EQU $100+$20", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("VALUE"));
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("VALUE", value));
}

TEST(MerlinSyntaxTest, EqualsDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CONST = $FF", section, symbols);

  // Should define symbol using = syntax, but create no atoms
  EXPECT_TRUE(symbols.IsDefined("CONST"));
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, EqualsWithHex) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("rw18 = $d000", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("rw18"));
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("rw18", value));
  EXPECT_EQ(value, 0xd000);
}

TEST(MerlinSyntaxTest, DumWithSymbol) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "master = $f880\n"
                       " dum master\n"
                       "_firstboot ds 3\n"
                       " dend\n";

  parser.Parse(source, section, symbols);

  // Should define master
  EXPECT_TRUE(symbols.IsDefined("master"));
  // Should define _firstboot in DUM block
  EXPECT_TRUE(symbols.IsDefined("_firstboot"));
}

TEST(MerlinSyntaxTest, OrgWithSymbol) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "start = $8000\n"
                       " org start\n"
                       " nop\n";

  parser.Parse(source, section, symbols);

  // Should define start
  EXPECT_TRUE(symbols.IsDefined("start"));
  // Should have ORG atom with correct address
  ASSERT_GE(section.atoms.size(), 1);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000);
}

// ============================================================================
// Phase 1: Foundation - DB Directive
// ============================================================================

TEST(MerlinSyntaxTest, DbSingleByte) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(MerlinSyntaxTest, DbMultipleBytes) {
  MerlinSyntaxParser parser;
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
// Phase 1: Foundation - DW Directive
// ============================================================================

TEST(MerlinSyntaxTest, DwSingleWord) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DW $1234", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // DW stores expressions for multi-pass evaluation (supports forward refs)
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "$1234");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

TEST(MerlinSyntaxTest, DwMultipleWords) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DW $ABCD,$EF01", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // DW stores expressions for multi-pass evaluation (supports forward refs)
  ASSERT_EQ(data_atom->expressions.size(), 2);
  EXPECT_EQ(data_atom->expressions[0], "$ABCD");
  EXPECT_EQ(data_atom->expressions[1], "$EF01");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

TEST(MerlinSyntaxTest, DwWithSymbolReference) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "startrun NOP\n"
                       ":1 dw startrun\n";

  parser.Parse(source, section, symbols);

  // Should define both startrun and :1 labels
  EXPECT_TRUE(symbols.IsDefined("startrun"));
  EXPECT_TRUE(symbols.IsDefined(":1"));

  // Should have atoms: label, NOP, label, DW
  ASSERT_GE(section.atoms.size(), 3);

  // Find the DataAtom from DW
  std::shared_ptr<DataAtom> data_atom = nullptr;
  for (const auto &atom : section.atoms) {
    if (atom->type == AtomType::Data) {
      data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
      if (data_atom && !data_atom->expressions.empty()) {
        break; // Found the DW data with expressions
      }
    }
  }
  ASSERT_NE(data_atom, nullptr);
  // DW stores expressions for multi-pass evaluation (supports forward refs)
  ASSERT_EQ(data_atom->expressions.size(), 1);
  EXPECT_EQ(data_atom->expressions[0], "startrun");
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
}

// ============================================================================
// Phase 1: Foundation - HEX Directive
// ============================================================================

TEST(MerlinSyntaxTest, HexDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         HEX 0102030405", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
  EXPECT_EQ(data_atom->data[3], 0x04);
  EXPECT_EQ(data_atom->data[4], 0x05);
}

TEST(MerlinSyntaxTest, HexWithSpaces) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         HEX AB CD EF", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3);
  EXPECT_EQ(data_atom->data[0], 0xAB);
  EXPECT_EQ(data_atom->data[1], 0xCD);
  EXPECT_EQ(data_atom->data[2], 0xEF);
}

TEST(MerlinSyntaxTest, HexWithCommas) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         HEX 01,02,03", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
}

TEST(MerlinSyntaxTest, HexWithCommasAndSpaces) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         HEX 00, 0d, 0b, 09", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 0x00);
  EXPECT_EQ(data_atom->data[1], 0x0D);
  EXPECT_EQ(data_atom->data[2], 0x0B);
  EXPECT_EQ(data_atom->data[3], 0x09);
}

// ============================================================================
// Phase 4: PUT Directive (File Inclusion)
// ============================================================================

TEST(MerlinSyntaxTest, PutDirectiveBasic) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a temporary test file
  std::string test_file = "/tmp/xasm_test_include.asm";
  std::ofstream out(test_file);
  out << "INCLUDED_LABEL\n";
  out << "         DB $42\n";
  out.close();

  // Parse PUT directive
  parser.Parse("         PUT /tmp/xasm_test_include.asm", section, symbols);

  // Should have included the label and DB directive
  EXPECT_TRUE(symbols.IsDefined("INCLUDED_LABEL"));
  ASSERT_GE(section.atoms.size(), 1);

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutCircularIncludeSelf) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a file that includes itself
  std::string test_file = "/tmp/xasm_test_circular.asm";
  std::ofstream out(test_file);
  out << " PUT /tmp/xasm_test_circular.asm\n";
  out.close();

  // Should throw exception for circular include
  EXPECT_THROW(
      parser.Parse(" PUT /tmp/xasm_test_circular.asm", section, symbols),
      std::runtime_error);

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutCircularIncludeCycle) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create file A that includes B
  std::string file_a = "/tmp/xasm_test_a.asm";
  std::ofstream out_a(file_a);
  out_a << " PUT /tmp/xasm_test_b.asm\n";
  out_a.close();

  // Create file B that includes A (creates cycle)
  std::string file_b = "/tmp/xasm_test_b.asm";
  std::ofstream out_b(file_b);
  out_b << " PUT /tmp/xasm_test_a.asm\n";
  out_b.close();

  // Should throw exception for circular include
  EXPECT_THROW(parser.Parse(" PUT /tmp/xasm_test_a.asm", section, symbols),
               std::runtime_error);

  // Clean up
  std::remove(file_a.c_str());
  std::remove(file_b.c_str());
}

TEST(MerlinSyntaxTest, PutNestedIncludes3Levels) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create level 3 file (innermost)
  std::string file_3 = "/tmp/xasm_test_level3.asm";
  std::ofstream out_3(file_3);
  out_3 << "LEVEL3_LABEL\n";
  out_3 << " DB $03\n";
  out_3.close();

  // Create level 2 file that includes level 3
  std::string file_2 = "/tmp/xasm_test_level2.asm";
  std::ofstream out_2(file_2);
  out_2 << "LEVEL2_LABEL\n";
  out_2 << " DB $02\n";
  out_2 << " PUT /tmp/xasm_test_level3.asm\n";
  out_2.close();

  // Create level 1 file that includes level 2
  std::string file_1 = "/tmp/xasm_test_level1.asm";
  std::ofstream out_1(file_1);
  out_1 << "LEVEL1_LABEL\n";
  out_1 << " DB $01\n";
  out_1 << " PUT /tmp/xasm_test_level2.asm\n";
  out_1.close();

  // Parse main file that includes level 1 (total 3 levels of nesting)
  parser.Parse(" PUT /tmp/xasm_test_level1.asm", section, symbols);

  // Should have all three labels defined
  EXPECT_TRUE(symbols.IsDefined("LEVEL1_LABEL"));
  EXPECT_TRUE(symbols.IsDefined("LEVEL2_LABEL"));
  EXPECT_TRUE(symbols.IsDefined("LEVEL3_LABEL"));

  // Clean up
  std::remove(file_1.c_str());
  std::remove(file_2.c_str());
  std::remove(file_3.c_str());
}

TEST(MerlinSyntaxTest, PutFileNotFound) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Try to include a non-existent file
  EXPECT_THROW(
      parser.Parse(" PUT /tmp/nonexistent_file_xasm.asm", section, symbols),
      std::runtime_error);
}

TEST(MerlinSyntaxTest, PutEmptyFile) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create an empty file
  std::string test_file = "/tmp/xasm_test_empty.asm";
  std::ofstream out(test_file);
  out.close();

  // Should handle empty file gracefully
  parser.Parse(" PUT /tmp/xasm_test_empty.asm", section, symbols);

  // Should have no atoms added
  EXPECT_EQ(section.atoms.size(), 0);

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutAutoAppendSExtension) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a test file with .S extension
  std::string test_file = "/tmp/xasm_test_include.S";
  std::ofstream out(test_file);
  out << "AUTOAPPEND_LABEL\n";
  out << " DB $55\n";
  out.close();

  // Parse PUT directive WITHOUT .S extension - should auto-append
  parser.Parse(" PUT /tmp/xasm_test_include", section, symbols);

  // Should have found the file with auto-appended .S extension
  EXPECT_TRUE(symbols.IsDefined("AUTOAPPEND_LABEL"));

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutKeepsExistingExtension) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a test file with .HEX extension
  std::string test_file = "/tmp/xasm_test_data.HEX";
  std::ofstream out(test_file);
  out << "DATA_LABEL\n";
  out << " DB $AA\n";
  out.close();

  // Parse PUT directive WITH .HEX extension - should NOT append .S
  parser.Parse(" PUT /tmp/xasm_test_data.HEX", section, symbols);

  // Should have found the file with original extension
  EXPECT_TRUE(symbols.IsDefined("DATA_LABEL"));

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutAutoAppendRelativePath) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a test file with .S extension in /tmp
  std::string test_file = "/tmp/eq.S";
  std::ofstream out(test_file);
  out << "EQ_LABEL\n";
  out << " DB $EE\n";
  out.close();

  // Parse PUT directive with just "eq" (no extension, no path)
  // Should auto-append .S and find in /tmp
  parser.Parse(" PUT eq", section, symbols);

  // Should have found eq.S
  EXPECT_TRUE(symbols.IsDefined("EQ_LABEL"));

  // Clean up
  std::remove(test_file.c_str());
}

TEST(MerlinSyntaxTest, PutExplicitSExtension) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Create a test file with .S extension
  std::string test_file = "/tmp/xasm_explicit.S";
  std::ofstream out(test_file);
  out << "EXPLICIT_LABEL\n";
  out << " DB $EE\n";
  out.close();

  // Parse PUT directive WITH explicit .S extension
  parser.Parse(" PUT /tmp/xasm_explicit.S", section, symbols);

  // Should work as before (backward compatibility)
  EXPECT_TRUE(symbols.IsDefined("EXPLICIT_LABEL"));

  // Clean up
  std::remove(test_file.c_str());
}

// ============================================================================
// Phase 5: DFB Directive (Define Font Byte)
// ============================================================================

TEST(MerlinSyntaxTest, DfbSingleByte) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DFB $42", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(MerlinSyntaxTest, DfbMultipleBytes) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         DFB $01,$02,$03,$04", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
  EXPECT_EQ(data_atom->data[3], 0x04);
}

TEST(MerlinSyntaxTest, DfbSignedNegativeByte) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // -1 should be stored as $FF
  parser.Parse("         DFB -1", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xFF);
}

TEST(MerlinSyntaxTest, DfbHighBit) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // $80 + value sets high bit
  parser.Parse("         DFB $80+$41", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0xC1); // $80 | $41
}

TEST(MerlinSyntaxTest, DfbWithExpression) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("VALUE    EQU $10", section, symbols);
  parser.Parse("         DFB VALUE+5", section, symbols);

  ASSERT_GE(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(
      section.atoms[section.atoms.size() - 1]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 0x15); // $10 + 5
}

TEST(MerlinSyntaxTest, DfbWithLabel) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("DATA     DFB $01,$02", section, symbols);

  EXPECT_TRUE(symbols.IsDefined("DATA"));
  ASSERT_GE(section.atoms.size(), 1);
}

// ============================================================================
// Phase 6: DO/ELSE/FIN Conditional Assembly
// ============================================================================

TEST(MerlinSyntaxTest, DoFinTrue) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " DO 1\n"
                       " DB $42\n"
                       " FIN\n";

  parser.Parse(source, section, symbols);

  // Code inside DO 1...FIN should be included
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(MerlinSyntaxTest, DoFinFalse) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " DO 0\n"
                       " DB $42\n"
                       " FIN\n";

  parser.Parse(source, section, symbols);

  // Code inside DO 0...FIN should be excluded
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, DoElseFinTrue) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " DO 1\n"
                       " DB $42\n"
                       " ELSE\n"
                       " DB $99\n"
                       " FIN\n";

  parser.Parse(source, section, symbols);

  // DO branch included, ELSE branch excluded
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(MerlinSyntaxTest, DoElseFinFalse) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " DO 0\n"
                       " DB $42\n"
                       " ELSE\n"
                       " DB $99\n"
                       " FIN\n";

  parser.Parse(source, section, symbols);

  // DO branch excluded, ELSE branch included
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x99);
}

TEST(MerlinSyntaxTest, DoFinNested) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " DO 1\n"
                       "  DO 1\n"
                       "   DB $42\n"
                       "  FIN\n"
                       " FIN\n";

  parser.Parse(source, section, symbols);

  // Nested DO blocks both true - code should be included
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 7: Listing Control Directives (LST, LSTDO, TR)
// ============================================================================

TEST(MerlinSyntaxTest, LstDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         LST", section, symbols);

  // LST is a listing control directive - doesn't generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, LstOffDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         LST OFF", section, symbols);

  // LST OFF is a listing control directive - doesn't generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, LstdoDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         LSTDO", section, symbols);

  // LSTDO is a listing control directive - doesn't generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, TrDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         TR", section, symbols);

  // TR is a listing control directive - doesn't generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, TrWithParameters) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         TR ADR", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0);

  Section section2("test", 0);
  parser.Parse("         TR ON", section2, symbols);
  EXPECT_EQ(section2.atoms.size(), 0);

  Section section3("test", 0);
  parser.Parse("         TR OFF", section3, symbols);
  EXPECT_EQ(section3.atoms.size(), 0);
}

// ============================================================================
// Phase 7: ASC Directive (ASCII String)
// ============================================================================

TEST(MerlinSyntaxTest, AscSimpleString) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ASC 'HELLO'", section, symbols);

  // ASC should set high bit on ALL characters (Apple II standard)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5);
  EXPECT_EQ(data_atom->data[0], 'H' | 0x80); // 0xC8
  EXPECT_EQ(data_atom->data[1], 'E' | 0x80); // 0xC5
  EXPECT_EQ(data_atom->data[2], 'L' | 0x80); // 0xCC
  EXPECT_EQ(data_atom->data[3], 'L' | 0x80); // 0xCC
  EXPECT_EQ(data_atom->data[4], 'O' | 0x80); // 0xCF
}

TEST(MerlinSyntaxTest, AscDoubleQuotes) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ASC \"TEST\"", section, symbols);

  // ASC should set high bit on ALL characters (Apple II standard)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4);
  EXPECT_EQ(data_atom->data[0], 'T' | 0x80); // 0xD4
  EXPECT_EQ(data_atom->data[1], 'E' | 0x80); // 0xC5
  EXPECT_EQ(data_atom->data[2], 'S' | 0x80); // 0xD3
  EXPECT_EQ(data_atom->data[3], 'T' | 0x80); // 0xD4
}

TEST(MerlinSyntaxTest, AscHighBit) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // ASC sets high bit on ALL characters (Apple II standard)
  // The '80' suffix is legacy and ignored (high bit already set)
  parser.Parse("         ASC 'HI'80", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2);
  EXPECT_EQ(data_atom->data[0], 'H' | 0x80); // High bit set on all chars
  EXPECT_EQ(data_atom->data[1], 'I' | 0x80); // High bit set on all chars
}

TEST(MerlinSyntaxTest, AscEmptyString) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         ASC ''", section, symbols);

  // Empty string should create DataAtom with no bytes
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(), 0);
}

// ============================================================================
// DS Directive with Expressions
// ============================================================================

TEST(MerlinSyntaxTest, DsWithSymbolReference) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "maxback = 200\n"
                       "bgX ds maxback\n";

  parser.Parse(source, section, symbols);

  // Should define maxback
  EXPECT_TRUE(symbols.IsDefined("maxback"));
  int64_t maxback_value;
  EXPECT_TRUE(symbols.Lookup("maxback", maxback_value));
  EXPECT_EQ(maxback_value, 200);

  // Should define bgX and create SpaceAtom with 200 bytes
  EXPECT_TRUE(symbols.IsDefined("bgX"));

  // Find the SpaceAtom
  std::shared_ptr<SpaceAtom> space_atom = nullptr;
  for (const auto &atom : section.atoms) {
    if (atom->type == AtomType::Space) {
      space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
      break;
    }
  }
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 200);
}

TEST(MerlinSyntaxTest, DsWithExpression) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "BLUETYPE ds 24*30\n";

  parser.Parse(source, section, symbols);

  // Should define BLUETYPE and create SpaceAtom with 720 bytes
  EXPECT_TRUE(symbols.IsDefined("BLUETYPE"));

  // Find the SpaceAtom
  std::shared_ptr<SpaceAtom> space_atom = nullptr;
  for (const auto &atom : section.atoms) {
    if (atom->type == AtomType::Space) {
      space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
      break;
    }
  }
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 720);
}

// ============================================================================
// Phase 8: USR Directive (External Subroutine Call)
// ============================================================================

TEST(MerlinSyntaxTest, UsrWithHexAddress) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         USR $C000", section, symbols);

  // USR is a no-op - should not generate any atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, UsrWithDecimalAddress) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         USR 49152", section, symbols);

  // USR is a no-op - should not generate any atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, UsrWithLabel) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "DRAW     NOP\n"
                       "         USR DRAW\n";

  parser.Parse(source, section, symbols);

  // Should define DRAW label
  EXPECT_TRUE(symbols.IsDefined("DRAW"));

  // Should have only label and NOP - USR generates no atoms
  ASSERT_EQ(section.atoms.size(), 2); // Label, NOP (no JSR)
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);

  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "NOP");
}

TEST(MerlinSyntaxTest, UsrWithLabelOnLine) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("CALLDRAW USR $C000", section, symbols);

  // Should define CALLDRAW label
  EXPECT_TRUE(symbols.IsDefined("CALLDRAW"));

  // Should have only label - USR generates no atoms
  ASSERT_EQ(section.atoms.size(), 1);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
}

// ============================================================================
// Bounds Checking and Input Validation Tests (M4 Refactoring)
// ============================================================================

TEST(MerlinSyntaxTest, StripCommentsEmptyString) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Empty string should not crash
  parser.Parse("", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, StripCommentsSingleAsterisk) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Single * should be treated as comment
  parser.Parse("*", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, ParseNumberEmptyString) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Empty string in number context should return 0 or handle gracefully
  parser.Parse("VALUE EQU ", section, symbols);
  // Should not crash
}

TEST(MerlinSyntaxTest, ParseNumberDollarOnly) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // $ with no digits should throw error
  EXPECT_THROW(parser.Parse("VALUE EQU $", section, symbols),
               std::runtime_error);
}

TEST(MerlinSyntaxTest, ParseNumberPercentOnly) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // % with no digits should throw error
  EXPECT_THROW(parser.Parse("VALUE EQU %", section, symbols),
               std::runtime_error);
}

TEST(MerlinSyntaxTest, ParseExpressionSingleQuote) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Single quote character (from SPECIALK.S edge case)
  parser.Parse("VALUE EQU '", section, symbols);
  // Should not crash, handle gracefully
}

TEST(MerlinSyntaxTest, ParseExpressionLowByteEmpty) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // < with nothing after it
  EXPECT_THROW(parser.Parse("VALUE EQU <", section, symbols),
               std::exception // Should throw some exception
  );
}

TEST(MerlinSyntaxTest, ParseExpressionHighByteEmpty) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // > with nothing after it
  EXPECT_THROW(parser.Parse("VALUE EQU >", section, symbols),
               std::exception // Should throw some exception
  );
}

TEST(MerlinSyntaxTest, HandleHexEmptyString) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // HEX with no operand
  parser.Parse("         HEX", section, symbols);

  // Should create empty DataAtom
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(), 0);
}

TEST(MerlinSyntaxTest, HandleHexSingleDigit) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // HEX with odd number of digits should handle gracefully
  parser.Parse("         HEX A", section, symbols);

  // Should not crash (might skip last digit or pad with 0)
  ASSERT_EQ(section.atoms.size(), 1);
}

TEST(MerlinSyntaxTest, HandleAscEmptyOperand) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // ASC with no operand
  parser.Parse("         ASC", section, symbols);

  // Should create empty DataAtom
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(), 0);
}

TEST(MerlinSyntaxTest, HandleAscSingleQuoteOnly) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // ASC with just opening quote
  parser.Parse("         ASC '", section, symbols);

  // Should handle gracefully
  ASSERT_EQ(section.atoms.size(), 1);
}

TEST(MerlinSyntaxTest, HandleDsEmptyOperand) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DS with no operand
  parser.Parse("         DS", section, symbols);

  // Should create SpaceAtom with 0 size
  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 0);
}

TEST(MerlinSyntaxTest, HandleDsWithProgramCounter) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DS with * (program counter) - should reserve 0 bytes (current_addr - current_addr)
  parser.Parse("         DS *", section, symbols);

  // Should create SpaceAtom with 0 size (current address is 0, so DS 0-0 = 0)
  ASSERT_EQ(section.atoms.size(), 1);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section.atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->size, 0);
}

TEST(MerlinSyntaxTest, TrimEmptyString) {
  MerlinSyntaxParser parser;

  // Access Trim through a parse that uses it
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Parse line that's all whitespace
  parser.Parse("         ", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, ParseLineOnlyWhitespace) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("              ", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, ParseExpressionShortStringAddition) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Single character before +
  parser.Parse("X EQU 5", section, symbols);
  parser.Parse("Y EQU X+", section, symbols);
  // Should handle gracefully, not crash
}

// ============================================================================
// Phase 10: END Directive
// ============================================================================

TEST(MerlinSyntaxTest, EndDirective) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("         END", section, symbols);

  // END directive should be a no-op - no atoms generated
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, EndDirectiveWithCode) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         ORG $8000\n"
                       "START    LDA #$00\n"
                       "         STA $C000\n"
                       "         END\n";

  parser.Parse(source, section, symbols);

  // Should process code before END
  EXPECT_TRUE(symbols.IsDefined("START"));
  ASSERT_GE(section.atoms.size(), 3); // ORG, Label, LDA, STA
}

TEST(MerlinSyntaxTest, EndDirectiveIgnoresAfter) {
  MerlinSyntaxParser parser;
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
// Phase 9: Macro System (PMC/MAC/EOM)
// ============================================================================

TEST(MerlinSyntaxTest, MacroDefinitionEmpty) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC EmptyMacro\n"
                       " EOM\n";

  parser.Parse(source, section, symbols);

  // Empty macro should be defined but produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MacroDefinitionSimple) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC SimpleMacro\n"
                       " NOP\n"
                       " NOP\n"
                       " EOM\n";

  parser.Parse(source, section, symbols);

  // Macro definition should not generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MacroExpansionSimple) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC SimpleMacro\n"
                       " NOP\n"
                       " NOP\n"
                       " EOM\n"
                       "\n"
                       " MAC SimpleMacro\n";

  parser.Parse(source, section, symbols);

  // MAC should expand to 2 NOP instructions
  ASSERT_EQ(section.atoms.size(), 2);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}

TEST(MerlinSyntaxTest, MacroWithParametersOneParam) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC LoadValue\n"
                       " LDA ]1\n"
                       " EOM\n"
                       "\n"
                       " MAC LoadValue;#$42\n";

  parser.Parse(source, section, symbols);

  // Should expand to LDA #$42
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "LDA");
}

TEST(MerlinSyntaxTest, MacroWithParametersTwoParams) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC CopyByte\n"
                       " LDA ]1\n"
                       " STA ]2\n"
                       " EOM\n"
                       "\n"
                       " MAC CopyByte;$C000;$C001\n";

  parser.Parse(source, section, symbols);

  // Should expand to LDA $C000 / STA $C001
  ASSERT_EQ(section.atoms.size(), 2);
  auto inst1 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst1, nullptr);
  EXPECT_EQ(inst1->mnemonic, "LDA");

  auto inst2 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst2, nullptr);
  EXPECT_EQ(inst2->mnemonic, "STA");
}

TEST(MerlinSyntaxTest, MacroUndefinedError) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " MAC UndefinedMacro\n";

  // Should throw error for undefined macro
  EXPECT_THROW(parser.Parse(source, section, symbols), std::runtime_error);
}

TEST(MerlinSyntaxTest, MacroNestedExpansion) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC InnerMacro\n"
                       " NOP\n"
                       " EOM\n"
                       "\n"
                       " PMC OuterMacro\n"
                       " MAC InnerMacro\n"
                       " EOM\n"
                       "\n"
                       " MAC OuterMacro\n";

  parser.Parse(source, section, symbols);

  // OuterMacro should expand InnerMacro, which expands to NOP
  ASSERT_EQ(section.atoms.size(), 1);
  auto inst = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst, nullptr);
  EXPECT_EQ(inst->mnemonic, "NOP");
}

TEST(MerlinSyntaxTest, MacroLocalLabelScope) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC LoopMacro\n"
                       ":LOOP LDA #$00\n"
                       " JMP :LOOP\n"
                       " EOM\n"
                       "\n"
                       "FIRST NOP\n"
                       " MAC LoopMacro\n"
                       "SECOND NOP\n"
                       " MAC LoopMacro\n";

  parser.Parse(source, section, symbols);

  // Each macro expansion should have its own local label scope
  // Both :LOOP labels should be scoped differently
  EXPECT_TRUE(symbols.IsDefined("FIRST"));
  EXPECT_TRUE(symbols.IsDefined("SECOND"));
}

TEST(MerlinSyntaxTest, MacroMultipleExpansions) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " PMC Inc16\n"
                       " INC ]1\n"
                       " BNE :SKIP\n"
                       " INC ]1+1\n"
                       ":SKIP\n"
                       " EOM\n"
                       "\n"
                       " MAC Inc16;$20\n"
                       " MAC Inc16;$30\n"
                       " MAC Inc16;$40\n";

  parser.Parse(source, section, symbols);

  // Should expand macro 3 times
  // Each with INC, BNE, INC (and label)
  ASSERT_GE(section.atoms.size(),
            3 * 2); // At least 3 * (INC, BNE, INC) but labels add more
}

// ============================================================================
// Merlin-style Macro Definition (MAC/<<<)
// ============================================================================

TEST(MerlinSyntaxTest, MacroMerlinStyleDefinition) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         MAC   INIT\n"
                       "         LDA   #$00\n"
                       "         TAX\n"
                       "         TAY\n"
                       "         <<<\n";

  parser.Parse(source, section, symbols);

  // Macro definition should not generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MacroMerlinStyleExpansion) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         MAC   INIT\n"
                       "         LDA   #$00\n"
                       "         TAX\n"
                       "         TAY\n"
                       "         <<<\n"
                       "\n"
                       "START    INIT\n";

  parser.Parse(source, section, symbols);

  // Should expand to: Label, LDA, TAX, TAY
  ASSERT_GE(section.atoms.size(), 4);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[2]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[3]->type, AtomType::Instruction);
}

TEST(MerlinSyntaxTest, MacroMerlinStyleWithParameters) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         MAC   STORE\n"
                       "         LDA   #]1\n"
                       "         STA   ]2\n"
                       "         <<<\n"
                       "\n"
                       "         STORE $42,$80\n";

  parser.Parse(source, section, symbols);

  // Should expand to: LDA #$42, STA $80
  ASSERT_EQ(section.atoms.size(), 2);
  auto inst1 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(inst1, nullptr);
  EXPECT_EQ(inst1->mnemonic, "LDA");
  EXPECT_EQ(inst1->operand, "#$42");

  auto inst2 = std::dynamic_pointer_cast<InstructionAtom>(section.atoms[1]);
  ASSERT_NE(inst2, nullptr);
  EXPECT_EQ(inst2->mnemonic, "STA");
  EXPECT_EQ(inst2->operand, "$80");
}

// ============================================================================
// Missing Directives (xasm++-1s3)
// ============================================================================

TEST(MerlinSyntaxTest, SavDirective) {
  // SAV - Save output filename (no-op for now)
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(" sav boot", section, symbols);

  // SAV is a no-op directive - should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, XcDirective) {
  // XC - Toggle 65C02 CPU mode (no-op for now)
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(" xc off", section, symbols);

  // XC is a no-op directive - should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MxDirectiveBinary) {
  // MX - Set 65816 register widths (binary format %00-%11)
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Test all binary modes
  parser.Parse(" mx %00", section, symbols); // 16-bit A, 16-bit X/Y
  parser.Parse(" mx %01", section, symbols); // 16-bit A, 8-bit X/Y
  parser.Parse(" mx %10", section, symbols); // 8-bit A, 16-bit X/Y
  parser.Parse(" mx %11", section, symbols); // 8-bit A, 8-bit X/Y

  // MX is a state-tracking directive - should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MxDirectiveDecimal) {
  // MX - Set 65816 register widths (decimal format 0-3)
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Test all decimal modes
  parser.Parse(" mx 0", section, symbols); // Same as %00
  parser.Parse(" mx 1", section, symbols); // Same as %01
  parser.Parse(" mx 2", section, symbols); // Same as %10
  parser.Parse(" mx 3", section, symbols); // Same as %11

  // MX is a state-tracking directive - should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MxDirectiveInvalidBinary) {
  // MX with invalid binary value should throw
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(parser.Parse(" mx %12", section, symbols), std::runtime_error);
  EXPECT_THROW(parser.Parse(" mx %100", section, symbols), std::runtime_error);
}

TEST(MerlinSyntaxTest, MxDirectiveInvalidDecimal) {
  // MX with invalid decimal value should throw
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(parser.Parse(" mx 4", section, symbols), std::runtime_error);
  EXPECT_THROW(parser.Parse(" mx 10", section, symbols), std::runtime_error);
}

TEST(MerlinSyntaxTest, MxDirectiveMissing) {
  // MX without operand should throw
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(parser.Parse(" mx", section, symbols), std::runtime_error);
}

TEST(MerlinSyntaxTest, RevDirective) {
  // REV - Reverse ASCII string
  // Emits reversed string as data bytes and defines label at that location
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("C_test rev \"ABC\"", section, symbols);

  // REV should define label and emit reversed string data
  EXPECT_TRUE(symbols.IsDefined("C_test"));

  // Label should point to address 0 (start of section)
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("C_test", value));
  EXPECT_EQ(value, 0);

  // Should have label and data atoms
  ASSERT_EQ(section.atoms.size(), 2);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);

  // Data should be "CBA" (reversed)
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3);
  EXPECT_EQ(data_atom->data[0], 'C'); // 0x43
  EXPECT_EQ(data_atom->data[1], 'B'); // 0x42
  EXPECT_EQ(data_atom->data[2], 'A'); // 0x41
}

TEST(MerlinSyntaxTest, RevDirectiveWithSingleChar) {
  // REV with single character
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("C_x rev \"X\"", section, symbols);

  // REV should define label and emit single byte
  EXPECT_TRUE(symbols.IsDefined("C_x"));

  // Label should point to address 0
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("C_x", value));
  EXPECT_EQ(value, 0);

  // Should have label and data atoms
  ASSERT_EQ(section.atoms.size(), 2);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Label);

  // Data should be "X"
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1);
  EXPECT_EQ(data_atom->data[0], 'X'); // 0x58
}

TEST(MerlinSyntaxTest, LupDirective) {
  // LUP - Loop directive (should error - not yet implemented)
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = " lup 36\n";

  // LUP should throw an error indicating it's not yet implemented
  EXPECT_THROW({ parser.Parse(source, section, symbols); }, std::runtime_error);
}

// ============================================================================
// 65816 Instructions and Directives (xasm++-hhy)
// ============================================================================

TEST(MerlinSyntaxTest, BltInstructionParsing) {
  // BLT - Branch if Less Than (signed comparison)
  // This is an alias for BCC (Branch if Carry Clear)
  // Used after CMP for signed comparisons: if A < operand, branch

  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Parse BLT instruction
  parser.Parse(" blt target", section, symbols);

  // Should generate an instruction atom
  ASSERT_EQ(section.atoms.size(), 1);
  auto instr_atom =
      std::dynamic_pointer_cast<InstructionAtom>(section.atoms[0]);
  ASSERT_NE(instr_atom, nullptr);

  // BLT should be recognized as a valid branch instruction
  EXPECT_EQ(instr_atom->mnemonic, "BLT");
  EXPECT_EQ(instr_atom->operand, "target");
}

// ============================================================================
// Label-based MAC Definition (UNPACK.S style)
// ============================================================================

TEST(MerlinSyntaxTest, MacroLabelBasedDefinition) {
  // Label-based MAC definition: MacroName MAC param1;param2
  // From UNPACK.S: stlx mac bank;addr
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "stlx mac bank;addr\n"
                       " hex 9f\n"
                       " da ]2\n"
                       " db ]1\n"
                       " <<<\n";

  parser.Parse(source, section, symbols);

  // Macro definition should not generate atoms
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(MerlinSyntaxTest, MacroLabelBasedExpansion) {
  // Test macro expansion after label-based definition
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "stlx mac bank;addr\n"
                       " hex 9f\n"
                       " da ]2\n"
                       " db ]1\n"
                       " <<<\n"
                       "\n"
                       " stlx $E1;$9D00\n";

  parser.Parse(source, section, symbols);

  // Should expand to: hex 9f, da $9D00, db $E1
  // That's 1 + 2 + 1 = 4 bytes
  ASSERT_EQ(section.atoms.size(), 3); // 3 data atoms

  auto data1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data1, nullptr);
  ASSERT_EQ(data1->data.size(), 1);
  EXPECT_EQ(data1->data[0], 0x9F);
}

TEST(MerlinSyntaxTest, MacroLabelBasedWithNoParams) {
  // Label-based MAC with no parameters
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "init mac\n"
                       " lda #$00\n"
                       " tax\n"
                       " <<<\n"
                       "\n"
                       " init\n";

  parser.Parse(source, section, symbols);

  // Should expand to: LDA #$00, TAX
  ASSERT_EQ(section.atoms.size(), 2);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}
