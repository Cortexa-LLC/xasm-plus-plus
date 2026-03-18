/**
 * @file test_scmasm_conditionals.cpp
 * @brief Unit tests for SCMASM conditional assembly directives (.DO/.ELSE/.FIN)
 */

#include "xasm++/assembler.h"
#include "xasm++/atom.h"
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
// Label Handling on .DO Directive Line (Bug B)
// ============================================================================

/**
 * @brief Test that a label on a .DO directive line gets the address at the start of the .DO line
 *
 * Bug B off-by-1 fix: The label on a .DO directive line should get the address
 * at the START of the .DO line (before entering the block), which is the address
 * of the first instruction following the .DO directive.
 *
 * Example: DIB .DO SSCIRQ=1
 *          DIB should get the address of the first instruction after the .DO line,
 *          which is the current PC at the point the .DO is encountered.
 */
TEST_F(ScmasmConditionalTest, LabelOnDoLineGetsAddressAtStart) {
  // Set up: condition is true, so block content will be assembled
  std::string source = "    .OR $2000\n"
                       "START .DO 1\n"
                       "    .DA $1234,$5678,$ABCD,$EF00\n" // 8 bytes
                       ".FIN\n"
                       "NEXT .DA $FF\n";  // Use .DA instead of lda for clearer test

  EXPECT_TRUE(ParseSucceeds(source));

  // Bug B off-by-1 fix: START should be at $2000 (the address at the START of .DO, before entering the block)
  // NOT at $2008 (the address after the block)
  int64_t start_value;
  EXPECT_TRUE(symbols.Lookup("START", start_value));
  EXPECT_EQ(start_value, 0x2000) << "START label should be at address at START of .DO line";

  // NEXT should be at $2008 (the address after .FIN, where the next directive begins)
  int64_t next_value;
  EXPECT_TRUE(symbols.Lookup("NEXT", next_value));
  EXPECT_EQ(next_value, 0x2008) << "NEXT should be at address after .FIN (after 8-byte block)";
}

/**
 * @brief Test that a label on a .DO directive line with false condition gets current address
 *
 * When the .DO condition is false, no content is assembled, so the label
 * should get the current address (no change).
 */
TEST_F(ScmasmConditionalTest, LabelOnDoLineWithFalseCondition) {
  std::string source = "    .OR $2000\n"
                       "START .DO 0\n"
                       "    .DA $1234,$5678,$ABCD,$EF00\n" // 8 bytes (not assembled)
                       ".FIN\n"
                       "NEXT .DA $AA\n";  // Use .DA for clarity

  EXPECT_TRUE(ParseSucceeds(source));

  // START should be at $2000 (current address, nothing assembled in block)
  int64_t start_value;
  EXPECT_TRUE(symbols.Lookup("START", start_value));
  EXPECT_EQ(start_value, 0x2000) << "START label should be at current address when block skipped";

  // NEXT should also be at $2000 (address after .FIN when block was skipped)
  int64_t next_value;
  EXPECT_TRUE(symbols.Lookup("NEXT", next_value));
  EXPECT_EQ(next_value, 0x2000) << "NEXT should be at same address as START when block skipped";
}

/**
 * @brief Test label on .DO line with .ELSE clause (true branch)
 */
TEST_F(ScmasmConditionalTest, LabelOnDoLineWithElseTrue) {
  std::string source = "    .OR $3000\n"
                       "DIB .DO 1\n"
                       "    .DA $11,$22\n"  // 4 bytes in true branch
                       ".ELSE\n"
                       "    .DA $33\n"      // 2 bytes in false branch (skipped)
                       ".FIN\n"
                       "AFTER lda #1\n";

  EXPECT_TRUE(ParseSucceeds(source));

  // Bug B off-by-1 fix: DIB should be at $3000 (address at START of .DO, before entering block)
  int64_t dib_value;
  EXPECT_TRUE(symbols.Lookup("DIB", dib_value));
  EXPECT_EQ(dib_value, 0x3000);
}

/**
 * @brief Test label on .DO line with .ELSE clause (false branch)
 */
TEST_F(ScmasmConditionalTest, LabelOnDoLineWithElseFalse) {
  std::string source = "    .OR $3000\n"
                       "DIB .DO 0\n"
                       "    .DA $11,$22\n"  // 4 bytes in true branch (skipped)
                       ".ELSE\n"
                       "    .DA $33\n"      // 2 bytes in false branch
                       ".FIN\n"
                       "AFTER lda #1\n";

  EXPECT_TRUE(ParseSucceeds(source));

  // Bug B off-by-1 fix: DIB should be at $3000 (address at START of .DO, before entering block)
  int64_t dib_value;
  EXPECT_TRUE(symbols.Lookup("DIB", dib_value));
  EXPECT_EQ(dib_value, 0x3000);
}

// ============================================================================
// Regression Tests
// ============================================================================

/**
 * @brief Regression test: local label on .FIN line must be defined
 *
 * Bug: In define_boundary_label(), the check `blabel[0] == '.'` incorrectly
 * skipped defining local labels (like `.28` or `.8`) on .FIN lines,
 * treating them as directives. This caused branches targeting those labels
 * to encode as `$FE` (branch-to-self), producing an infinite loop at runtime.
 *
 * The fix: only skip if the token starts with '.' AND is NOT a local label
 * (i.e., IsLocalLabel returns false — not `.N` where N is all digits).
 *
 * This test verifies that a branch to a local label defined on a .FIN line
 * resolves to the correct address, not $FE (branch to self).
 */
TEST_F(ScmasmConditionalTest, LocalLabelOnFinLineResolvesCorrectly) {
  // Source: branch `beq .28` inside a .DO block where `.28` is the label on
  // the .FIN line. The assembled BEQ must NOT produce $FE (branch to self).
  std::string source =
      "    .OR $0800\n"
      "COND    .EQ 1\n"
      "OUTER   lda #0\n"
      "        .DO COND=1\n"
      "        lda #1\n"  // 2 bytes
      "        beq .28\n" // 2 bytes — must branch to .28 (NOT $FE)
      "        lda #2\n"  // 2 bytes (target of successful beq .28 skip)
      "        lda #3\n"  // 2 bytes
      "        lda #4\n"  // 2 bytes
      "        lda #5\n"  // 2 bytes
      "        lda #6\n"  // 2 bytes
      "        lda #7\n"  // 2 bytes
      ".28     .FIN\n"    // .28 defined HERE — at end of block
      "AFTER   nop\n";

  parser->Parse(source, section, symbols);

  Assembler assembler;
  assembler.SetCpuPlugin(cpu.get());
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  // Collect all bytes from instruction atoms
  std::vector<uint8_t> bytes;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (inst) {
        bytes.insert(bytes.end(), inst->encoded_bytes.begin(),
                     inst->encoded_bytes.end());
      }
    } else if (atom && atom->type == AtomType::Data) {
      auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
      if (data_atom) {
        bytes.insert(bytes.end(), data_atom->data.begin(),
                     data_atom->data.end());
      }
    }
  }

  // Locate the BEQ instruction (F0) in the assembled output
  // Sequence: LDA #0 (2), LDA #1 (2), BEQ .28 (2), LDA #2..#7 (12), NOP (1)
  // BEQ at offset 4 (after 2 LDAs)
  ASSERT_GE(bytes.size(), 7u) << "Must have at least 7 bytes";
  EXPECT_EQ(bytes[4], 0xF0) << "BEQ opcode expected at offset 4";

  // The BEQ offset must NOT be $FE (branch to self bug)
  EXPECT_NE(bytes[5], 0xFE)
      << "BEQ offset $FE means branch-to-self bug: .28 label on .FIN line was "
         "not defined";

  // The BEQ should branch forward past the remaining LDAs to .28 = 12 bytes
  // BEQ opcode is at address $0804, after 2-byte instruction PC = $0806
  // .28 is at $0804 + 2 + 12 = $0812 (after all the LDA #2..#7 instructions)
  // offset = $0812 - $0806 = 0x0C = 12
  EXPECT_EQ(bytes[5], 0x0C)
      << "BEQ should branch 12 bytes forward to .28 label on .FIN line";
}

/**
 * @brief Regression test: local label `.8` on .FIN line inside .DO block
 *
 * Same bug as above but with single-digit `.8` label (ProDOS FX ILDR pattern:
 *   .8   .FIN  -- end of .DO M.PM=1 block)
 */
TEST_F(ScmasmConditionalTest, SingleDigitLocalLabelOnFinLineResolvesCorrectly) {
  std::string source =
      "    .OR $0800\n"
      "COND    .EQ 1\n"
      "OUTER   lda #0\n"
      "        .DO COND=1\n"
      "        bcs .8\n"  // should branch to .8 at .FIN, NOT $FE
      "        lda #1\n"
      "        lda #2\n"
      "        lda #3\n"
      "        lda #4\n"
      "        lda #5\n"
      ".8      .FIN\n"  // .8 defined on .FIN line
      "AFTER   nop\n";

  parser->Parse(source, section, symbols);

  Assembler assembler;
  assembler.SetCpuPlugin(cpu.get());
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  std::vector<uint8_t> bytes;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (inst) {
        bytes.insert(bytes.end(), inst->encoded_bytes.begin(),
                     inst->encoded_bytes.end());
      }
    } else if (atom && atom->type == AtomType::Data) {
      auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
      if (data_atom) {
        bytes.insert(bytes.end(), data_atom->data.begin(),
                     data_atom->data.end());
      }
    }
  }

  ASSERT_GE(bytes.size(), 4u);

  // BCS at offset 2 (after LDA #0 = 2 bytes)
  EXPECT_EQ(bytes[2], 0xB0) << "BCS opcode at offset 2";
  EXPECT_NE(bytes[3], 0xFE)
      << "BCS offset $FE means branch-to-self bug: .8 label on .FIN was not "
         "defined";
  // 5 LDA instructions = 10 bytes after BCS; BCS offset = 10
  EXPECT_EQ(bytes[3], 0x0A)
      << "BCS should branch 10 bytes forward to .8 label on .FIN line";
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
