// Z80 Universal Syntax Conditional Directives Tests
// Tests for Phase 7: IF/ELSE/ENDIF, IFDEF/IFNDEF,
// IFEQ/IFNE/IFLT/IFGT/IFLE/IFGE, IF1/IF2, IFB/IFNB, IFIDN/IFDIF, and nested
// conditionals

#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// Helper to get symbol value
[[maybe_unused]] static int64_t GetSymbolValue(ConcreteSymbolTable &symbols,
                                               const std::string &name) {
  auto symbol = symbols.GetSymbol(name);
  if (!symbol || !symbol->value) {
    return -1;
  }
  return symbol->value->Evaluate(symbols);
}

// ============================================================================
// Phase 7: Basic Conditionals - IF/ELSE/ENDIF
// ============================================================================

TEST(Z80ConditionalsTest, IfTrueCondition) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         DB 42H
         ENDIF
)",
               section, symbols);

  // IF 1 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfFalseCondition) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 0
         DB 42H
         ENDIF
)",
               section, symbols);

  // IF 0 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfElseTrue) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         DB 11H
         ELSE
         DB 22H
         ENDIF
)",
               section, symbols);

  // IF 1 (true) - first DB should be assembled, second skipped
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x11);
}

TEST(Z80ConditionalsTest, IfElseFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 0
         DB 11H
         ELSE
         DB 22H
         ENDIF
)",
               section, symbols);

  // IF 0 (false) - second DB should be assembled, first skipped
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x22);
}

// ============================================================================
// Phase 7: Symbol Existence - IFDEF/IFNDEF
// ============================================================================

TEST(Z80ConditionalsTest, IfdefSymbolDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
SYM      EQU 100
         IFDEF SYM
         DB 42H
         ENDIF
)",
               section, symbols);

  // SYM is defined - DB should be assembled
  // EQU creates a label atom, so expect 2 atoms (label + data)
  ASSERT_EQ(section.atoms.size(), 2);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfdefSymbolNotDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFDEF UNDEFINED_SYM
         DB 42H
         ENDIF
)",
               section, symbols);

  // UNDEFINED_SYM not defined - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfndefSymbolNotDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFNDEF UNDEFINED_SYM
         DB 42H
         ENDIF
)",
               section, symbols);

  // UNDEFINED_SYM not defined - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfndefSymbolDefined) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
SYM      EQU 100
         IFNDEF SYM
         DB 42H
         ENDIF
)",
               section, symbols);

  // SYM is defined - DB should be skipped
  // Only the EQU label atom should be present
  EXPECT_EQ(section.atoms.size(), 1);
  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(section.atoms[0]);
  EXPECT_NE(label_atom, nullptr);
}

// ============================================================================
// Phase 7: Comparison Conditionals - IFEQ/IFNE
// ============================================================================

TEST(Z80ConditionalsTest, IfeqEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFEQ 5, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 == 5 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfeqNotEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFEQ 5, 3
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 == 3 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfneNotEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFNE 5, 3
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 != 3 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfneEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFNE 5, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 != 5 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 7: Relational Conditionals - IFLT/IFGT/IFLE/IFGE
// ============================================================================

TEST(Z80ConditionalsTest, IfltLessThan) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFLT 3, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 3 < 5 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfltNotLessThan) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFLT 5, 3
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 < 3 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfgtGreaterThan) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFGT 5, 3
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 > 3 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfgtNotGreaterThan) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFGT 3, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 3 > 5 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfleLessOrEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFLE 5, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 <= 5 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfleNotLessOrEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFLE 5, 3
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 <= 3 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfgeGreaterOrEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFGE 5, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 5 >= 5 (true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfgeNotGreaterOrEqual) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFGE 3, 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // 3 >= 5 (false) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 7: Pass-based Conditionals - IF1/IF2
// ============================================================================

TEST(Z80ConditionalsTest, If1FirstPass) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF1
         DB 42H
         ENDIF
)",
               section, symbols);

  // IF1 - true on first pass (we're always first pass in single-pass)
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, If2SecondPass) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF2
         DB 42H
         ENDIF
)",
               section, symbols);

  // IF2 - false on first pass (we don't do second pass by default)
  // Single-pass assembler treats IF2 as false
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 7: Blank/Non-blank Conditionals - IFB/IFNB
// ============================================================================

TEST(Z80ConditionalsTest, IfbBlankArgument) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFB <>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFB <> (blank) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfbNonBlankArgument) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFB <NOTBLANK>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFB <NOTBLANK> (not blank) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfnbNonBlankArgument) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFNB <NOTBLANK>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFNB <NOTBLANK> (not blank) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfnbBlankArgument) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFNB <>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFNB <> (blank) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 7: String Comparison Conditionals - IFIDN/IFDIF
// ============================================================================

TEST(Z80ConditionalsTest, IfidnIdenticalStrings) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFIDN <ABC>, <ABC>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFIDN <ABC>, <ABC> (identical) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfidnDifferentStrings) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFIDN <ABC>, <XYZ>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFIDN <ABC>, <XYZ> (different) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, IfdifDifferentStrings) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFDIF <ABC>, <XYZ>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFDIF <ABC>, <XYZ> (different) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfdifIdenticalStrings) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IFDIF <ABC>, <ABC>
         DB 42H
         ENDIF
)",
               section, symbols);

  // IFDIF <ABC>, <ABC> (identical) - DB should be skipped
  EXPECT_EQ(section.atoms.size(), 0);
}

// ============================================================================
// Phase 7: Nested Conditionals
// ============================================================================

TEST(Z80ConditionalsTest, NestedIfBothTrue) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         IF 1
         DB 42H
         ENDIF
         ENDIF
)",
               section, symbols);

  // Both IF conditions true - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, NestedIfOuterFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 0
         IF 1
         DB 42H
         ENDIF
         ENDIF
)",
               section, symbols);

  // Outer IF false - inner block skipped entirely
  EXPECT_EQ(section.atoms.size(), 0);
}

TEST(Z80ConditionalsTest, NestedIfInnerFalse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         IF 0
         DB 42H
         ENDIF
         DB 55H
         ENDIF
)",
               section, symbols);

  // Outer IF true, inner IF false - only outer DB assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x55);
}

TEST(Z80ConditionalsTest, NestedIfElse) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         IF 0
         DB 11H
         ELSE
         DB 22H
         ENDIF
         ELSE
         DB 33H
         ENDIF
)",
               section, symbols);

  // Outer IF true, inner IF false, inner ELSE taken
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x22);
}

TEST(Z80ConditionalsTest, NestedConditionalThreeLevels) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
         IF 1
         IF 1
         IF 1
         DB 42H
         ENDIF
         ENDIF
         ENDIF
)",
               section, symbols);

  // Three levels of true conditionals
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, MixedConditionalTypes) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
SYM      EQU 5
         IFDEF SYM
         IFEQ 5, 5
         IFGT 10, 3
         DB 42H
         ENDIF
         ENDIF
         ENDIF
)",
               section, symbols);

  // IFDEF SYM (true), IFEQ 5,5 (true), IFGT 10,3 (true)
  // EQU creates a label atom, so expect 2 atoms (label + data)
  ASSERT_EQ(section.atoms.size(), 2);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 7: Conditional Error Cases
// ============================================================================

TEST(Z80ConditionalsTest, ElseWithoutIf) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(
      {
        parser.Parse(R"(
         ELSE
         DB 42H
         ENDIF
)",
                     section, symbols);
      },
      std::runtime_error);
}

TEST(Z80ConditionalsTest, EndifWithoutIf) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(
      {
        parser.Parse(R"(
         ENDIF
)",
                     section, symbols);
      },
      std::runtime_error);
}

TEST(Z80ConditionalsTest, MultipleElseBlocks) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  EXPECT_THROW(
      {
        parser.Parse(R"(
         IF 1
         DB 11H
         ELSE
         DB 22H
         ELSE
         DB 33H
         ENDIF
)",
                     section, symbols);
      },
      std::runtime_error);
}

// ============================================================================
// Phase 7: Conditionals with Expressions
// ============================================================================

TEST(Z80ConditionalsTest, IfWithExpressionTrue) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
VAL      EQU 10
         IF VAL > 5
         DB 42H
         ENDIF
)",
               section, symbols);

  // VAL > 5 (10 > 5 = true) - DB should be assembled
  ASSERT_EQ(section.atoms.size(), 1);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfeqWithSymbols) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
VAL1     EQU 5
VAL2     EQU 5
         IFEQ VAL1, VAL2
         DB 42H
         ENDIF
)",
               section, symbols);

  // VAL1 == VAL2 (5 == 5 = true) - DB should be assembled
  // Two EQU labels + data atom = 3 atoms
  ASSERT_EQ(section.atoms.size(), 3);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}

TEST(Z80ConditionalsTest, IfltWithComplexExpression) {
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  parser.Parse(R"(
BASE     EQU 100
         IFLT BASE + 50, 200
         DB 42H
         ENDIF
)",
               section, symbols);

  // (BASE + 50) < 200 (150 < 200 = true) - DB should be assembled
  // EQU label + data atom = 2 atoms
  ASSERT_EQ(section.atoms.size(), 2);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data[0], 0x42);
}
