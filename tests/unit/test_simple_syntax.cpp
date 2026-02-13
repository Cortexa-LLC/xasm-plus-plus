// SimpleSyntax Parser tests
// Phase 1: Minimal Viable Assembler - SimpleSyntax Plugin

#include "xasm++/symbol.h"
#include "xasm++/syntax/simple_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: Empty input parsing
TEST(SimpleSyntaxTest, EmptyInput) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("", section, symbols);

  // Empty input should produce no atoms
  EXPECT_EQ(section.atoms.size(), 0UL);
}

// Test 2: .org directive parsing
TEST(SimpleSyntaxTest, OrgDirective) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("    .org $8000", section, symbols);

  // Should create one OrgAtom
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Org);

  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000U);
}

// Test 3: .db directive parsing
TEST(SimpleSyntaxTest, DbDirective) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("    .db $01, $02, $03", section, symbols);

  // Should create one DataAtom
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Data);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3UL);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
}

// Test 4: .dw directive parsing (little-endian)
TEST(SimpleSyntaxTest, DwDirective) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("    .dw $1234", section, symbols);

  // Should create one DataAtom with little-endian word
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Data);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2UL);
  EXPECT_EQ(data_atom->data[0], 0x34); // Low byte
  EXPECT_EQ(data_atom->data[1], 0x12); // High byte
}

// Test 5: Label parsing
TEST(SimpleSyntaxTest, LabelDefinition) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("start:", section, symbols);

  // Should create Symbol and LabelAtom
  EXPECT_TRUE(symbols.IsDefined("start"));

  ASSERT_EQ(section.atoms.size(), 1UL);
  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Label);

  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(atom);
  ASSERT_NE(label_atom, nullptr);
  EXPECT_EQ(label_atom->name, "start");
}

// Test 6: Instruction parsing
TEST(SimpleSyntaxTest, InstructionImmediate) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("    LDA #$42", section, symbols);

  // Should create InstructionAtom
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Instruction);

  auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(atom);
  ASSERT_NE(inst_atom, nullptr);
  EXPECT_EQ(inst_atom->mnemonic, "LDA");
  EXPECT_EQ(inst_atom->operand, "#$42");
}

// Test 7: Comment handling
TEST(SimpleSyntaxTest, Comments) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse("    ; This is a comment\n    LDA #$42 ; Load A", section,
               symbols);

  // Comment line produces nothing, instruction line produces InstructionAtom
  ASSERT_EQ(section.atoms.size(), 1UL);

  auto atom = section.atoms[0];
  EXPECT_EQ(atom->type, AtomType::Instruction);
}

// Test 8: Complete program
TEST(SimpleSyntaxTest, CompleteProgram) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
    .org $8000
start:  LDA #$42
    STA $0200
loop:   JMP loop
    .db $01, $02
    .dw $1234
  )";

  parser.Parse(program, section, symbols);

  // Should have: OrgAtom, LabelAtom, InstructionAtom, InstructionAtom,
  // LabelAtom, InstructionAtom, DataAtom, DataAtom
  ASSERT_EQ(section.atoms.size(), 8UL);

  // Verify atom types
  EXPECT_EQ(section.atoms[0]->type, AtomType::Org);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[2]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[3]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[4]->type, AtomType::Label);
  EXPECT_EQ(section.atoms[5]->type, AtomType::Instruction);
  EXPECT_EQ(section.atoms[6]->type, AtomType::Data);
  EXPECT_EQ(section.atoms[7]->type, AtomType::Data);

  // Verify symbols
  EXPECT_TRUE(symbols.IsDefined("start"));
  EXPECT_TRUE(symbols.IsDefined("loop"));
}

// Test 9: Unknown directive should throw exception
TEST(SimpleSyntaxTest, UnknownDirectiveThrows) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Unknown directive should throw
  EXPECT_THROW(parser.Parse("    .unknown $1234", section, symbols),
               std::runtime_error);
}

// Test 10: Case-insensitive directive matching
TEST(SimpleSyntaxTest, DirectiveCaseInsensitive) {
  SimpleSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Test lowercase directive
  parser.Parse("    .org $8000", section, symbols);
  ASSERT_EQ(section.atoms.size(), 1UL);
  EXPECT_EQ(section.atoms[0]->type, AtomType::Org);

  // Test uppercase directive
  parser.Parse("    .ORG $9000", section, symbols);
  ASSERT_EQ(section.atoms.size(), 2UL);
  EXPECT_EQ(section.atoms[1]->type, AtomType::Org);

  // Test mixed case directive
  parser.Parse("    .Org $A000", section, symbols);
  ASSERT_EQ(section.atoms.size(), 3UL);
  EXPECT_EQ(section.atoms[2]->type, AtomType::Org);
}
