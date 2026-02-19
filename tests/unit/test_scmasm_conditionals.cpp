/**
 * @file test_scmasm_conditionals.cpp
 * @brief Unit tests for SCMASM conditional assembly directives (.DO/.ELSE/.FIN)
 */

#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

/**
 * Test fixture for SCMASM conditional assembly tests
 */
class ScmasmConditionalTest : public ::testing::Test {
protected:
  std::unique_ptr<ScmasmSyntaxParser> parser;
  std::unique_ptr<Cpu6502> cpu;
  Section section;
  ConcreteSymbolTable symbols;

  void SetUp() override {
    parser = std::make_unique<ScmasmSyntaxParser>();
    cpu = std::make_unique<Cpu6502>();
    parser->SetCpu(cpu.get());
  }

  /**
   * Helper: Parse source and return atom count
   */
  size_t ParseAndGetAtomCount(const std::string &source) {
    section = Section();
    symbols = ConcreteSymbolTable();

    try {
      parser->Parse(source, section, symbols);
    } catch (const std::exception &e) {
      ADD_FAILURE() << "Parse failed: " << e.what();
    }

    return section.atoms.size();
  }

  /**
   * Helper: Parse source and return success
   */
  bool ParseSucceeds(const std::string &source) {
    section = Section();
    symbols = ConcreteSymbolTable();

    try {
      parser->Parse(source, section, symbols);
      return true;
    } catch (const std::exception &e) {
      return false;
    }
  }

  /**
   * Helper: Expect parse failure
   */
  void ExpectParseError(const std::string &source) {
    section = Section();
    symbols = ConcreteSymbolTable();

    EXPECT_THROW(
        { parser->Parse(source, section, symbols); }, std::runtime_error);
  }
};

// ============================================================================
// Basic .DO/.FIN Tests
// ============================================================================

TEST_F(ScmasmConditionalTest, DoTrueIncludesCode) {
  std::string source = "VAL .EQ 1\n"
                       ".DO VAL=1\n"
                       "    lda #42\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  // Should have atoms: EQU directive + instruction
  EXPECT_GT(atom_count, 0u); // Code included
}

TEST_F(ScmasmConditionalTest, DoFalseExcludesCode) {
  std::string source = "VAL .EQ 1\n"
                       ".DO VAL=0\n"
                       "    lda #42\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  // Should only have EQU directive, no instruction
  EXPECT_EQ(atom_count, 0u); // No code emitted (EQU doesn't create atoms)
}

TEST_F(ScmasmConditionalTest, DoWithExpression) {
  std::string source = "A .EQ 5\n"
                       "B .EQ 3\n"
                       ".DO A>B\n"
                       "    lda #1\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  EXPECT_GT(atom_count, 0u); // Expression true, code included
}

TEST_F(ScmasmConditionalTest, DoWithFalseExpression) {
  std::string source = "A .EQ 3\n"
                       "B .EQ 5\n"
                       ".DO A>B\n"
                       "    lda #1\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  EXPECT_EQ(atom_count, 0u); // Expression false, no code emitted
}

// ============================================================================
// .ELSE Tests
// ============================================================================

TEST_F(ScmasmConditionalTest, ElseTakesFalseBranch) {
  std::string source = "VAL .EQ 0\n"
                       ".DO VAL=1\n"
                       "    lda #1\n"
                       ".ELSE\n"
                       "    lda #2\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  EXPECT_GT(atom_count, 0u); // ELSE branch taken, code included
}

TEST_F(ScmasmConditionalTest, ElseSkipsTrueBranch) {
  std::string source = "VAL .EQ 1\n"
                       ".DO VAL=1\n"
                       "    lda #1\n"
                       ".ELSE\n"
                       "    lda #2\n"
                       ".FIN\n";

  auto atom_count = ParseAndGetAtomCount(source);

  EXPECT_GT(atom_count, 0u); // DO branch taken, code included
}

TEST_F(ScmasmConditionalTest, ElseWithoutDoFails) {
  std::string source = ".ELSE\n"
                       "    lda #1\n"
                       ".FIN\n";

  ExpectParseError(source);
}

// ============================================================================
// Nested Conditionals
// ============================================================================

TEST_F(ScmasmConditionalTest, NestedConditionalsAllTrue) {
  std::string source = "OUTER .EQ 1\n"
                       "INNER .EQ 1\n"
                       ".DO OUTER=1\n"
                       "    lda #1\n"
                       "    .DO INNER=1\n"
                       "        lda #2\n"
                       "    .FIN\n"
                       "    lda #3\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // All instructions included
}

TEST_F(ScmasmConditionalTest, NestedConditionalsOuterFalse) {
  std::string source = "OUTER .EQ 0\n"
                       "INNER .EQ 1\n"
                       ".DO OUTER=1\n"
                       "    lda #1\n"
                       "    .DO INNER=1\n"
                       "        lda #2\n"
                       "    .FIN\n"
                       "    lda #3\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_EQ(section.atoms.size(), 0u); // All code excluded
}

TEST_F(ScmasmConditionalTest, NestedConditionalsInnerFalse) {
  std::string source = "OUTER .EQ 1\n"
                       "INNER .EQ 0\n"
                       ".DO OUTER=1\n"
                       "    lda #1\n"
                       "    .DO INNER=1\n"
                       "        lda #2\n"
                       "    .FIN\n"
                       "    lda #3\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // Outer instructions included
}

TEST_F(ScmasmConditionalTest, NestedWithElse) {
  std::string source = "OUTER .EQ 1\n"
                       "INNER .EQ 0\n"
                       ".DO OUTER=1\n"
                       "    .DO INNER=1\n"
                       "        lda #1\n"
                       "    .ELSE\n"
                       "        lda #2\n"
                       "    .FIN\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // ELSE branch taken
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_F(ScmasmConditionalTest, UnbalancedDoMissingFin) {
  std::string source = ".DO 1\n"
                       "    lda #1\n";

  ExpectParseError(source);
}

TEST_F(ScmasmConditionalTest, UnbalancedFinWithoutDo) {
  std::string source = "lda #1\n"
                       ".FIN\n";

  ExpectParseError(source);
}

TEST_F(ScmasmConditionalTest, MultipleElseInSameBlock) {
  std::string source = ".DO 1\n"
                       "    lda #1\n"
                       ".ELSE\n"
                       "    lda #2\n"
                       ".ELSE\n"
                       "    lda #3\n"
                       ".FIN\n";

  ExpectParseError(source);
}

TEST_F(ScmasmConditionalTest, FinWithoutDo) {
  std::string source = ".FIN\n";

  ExpectParseError(source);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ScmasmConditionalTest, EmptyConditionalBlock) {
  std::string source = ".DO 1\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_EQ(section.atoms.size(), 0u); // Empty block is valid
}

TEST_F(ScmasmConditionalTest, ConditionalWithNoExpression) {
  std::string source = ".DO\n"
                       "    lda #1\n"
                       ".FIN\n";

  // Should fail - .DO requires an expression
  ExpectParseError(source);
}

TEST_F(ScmasmConditionalTest, ConditionalWithZeroExpression) {
  std::string source = ".DO 0\n"
                       "    lda #1\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_EQ(section.atoms.size(), 0u); // 0 is false, no code
}

TEST_F(ScmasmConditionalTest, ConditionalWithNonZeroExpression) {
  std::string source = ".DO 42\n"
                       "    lda #1\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // Non-zero is true, code included
}

TEST_F(ScmasmConditionalTest, MultipleSequentialConditionals) {
  std::string source = ".DO 1\n"
                       "    lda #1\n"
                       ".FIN\n"
                       ".DO 1\n"
                       "    lda #2\n"
                       ".FIN\n"
                       ".DO 0\n"
                       "    lda #3\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // First two blocks emitted
}

// ============================================================================
// Integration with Other Directives
// ============================================================================

TEST_F(ScmasmConditionalTest, ConditionalWithEquDirective) {
  std::string source = "VAL .EQ 1\n"
                       ".DO VAL\n"
                       "INNER .EQ 42\n"
                       "    lda #INNER\n"
                       ".FIN\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // Code included

  // Verify INNER symbol was defined
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("INNER", value));
  EXPECT_EQ(value, 42);
}

TEST_F(ScmasmConditionalTest, ConditionalWithOrgDirective) {
  std::string source = ".DO 1\n"
                       "    .OR $1000\n"
                       "    lda #1\n"
                       ".FIN\n";

  // Should parse without errors - .OR is processed conditionally
  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // Code included
}

TEST_F(ScmasmConditionalTest, ConditionalExcludesOrgDirective) {
  std::string source = ".DO 0\n"
                       "    .OR $1000\n"
                       "    lda #1\n"
                       ".FIN\n"
                       "    lda #2\n";

  EXPECT_TRUE(ParseSucceeds(source));
  EXPECT_GT(section.atoms.size(), 0u); // Only lda #2 after .FIN
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
