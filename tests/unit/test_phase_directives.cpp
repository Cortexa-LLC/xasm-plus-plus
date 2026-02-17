/**
 * @file test_phase_directives.cpp
 * @brief Unit tests for .PH (phase) and .EP (end phase) directives
 */

#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace xasm;

class PhaseDirectivesTest : public ::testing::Test {
protected:
  void SetUp() override {
    parser = std::make_unique<ScmasmSyntaxParser>();
    section = std::make_unique<Section>("CODE", 0x01, 0x2000);
    symbols = std::make_unique<ConcreteSymbolTable>();
  }

  std::unique_ptr<ScmasmSyntaxParser> parser;
  std::unique_ptr<Section> section;
  std::unique_ptr<ConcreteSymbolTable> symbols;
};

/**
 * @brief Test basic .PH directive
 *
 * When code is assembled at one address but executed at another,
 * .PH sets the "phase" address (logical/execution address) while
 * the actual assembly address continues normally.
 */
TEST_F(PhaseDirectivesTest, BasicPhaseDirective) {
  std::string source = R"(
        .OR $2000
        .DA #$00       ; Assembled at $2000 (1 byte)
        .PH $8000      ; Logical address becomes $8000
LABEL   .DA #$00       ; Assembled at $2001, but LABEL = $8000
        .EP            ; End phase
        .DA #$00       ; Back to normal ($2002)
)";

  parser->Parse(source, *section, *symbols);

  // LABEL should have phase address ($8000), not assembly address ($2001)
  EXPECT_TRUE(symbols->IsDefined("LABEL"));
  auto label = symbols->GetSymbol("LABEL");
  ASSERT_NE(label, nullptr);
  EXPECT_EQ(label->value->Evaluate(*symbols), 0x8000);
}

/**
 * @brief Test .EP without .PH (should not crash, just no-op)
 */
TEST_F(PhaseDirectivesTest, EndPhaseWithoutStartPhase) {
  std::string source = R"(
        .OR $2000
        .EP           ; Should be no-op (not in phase)
        .DA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, *section, *symbols));
}

/**
 * @brief Test phase affects all labels until .EP
 */
TEST_F(PhaseDirectivesTest, MultipleLabelsInPhase) {
  std::string source = R"(
        .OR $2000
        .PH $8000
LABEL1  .DA #$00      ; LABEL1 should be $8000
LABEL2  .DA #$00      ; LABEL2 should be $8001
LABEL3  .DA #$00      ; LABEL3 should be $8002
        .EP
LABEL4  .DA #$00      ; LABEL4 should be $2003 (actual address)
)";

  parser->Parse(source, *section, *symbols);

  auto label1 = symbols->GetSymbol("LABEL1");
  auto label2 = symbols->GetSymbol("LABEL2");
  auto label3 = symbols->GetSymbol("LABEL3");
  auto label4 = symbols->GetSymbol("LABEL4");

  ASSERT_NE(label1, nullptr);
  ASSERT_NE(label2, nullptr);
  ASSERT_NE(label3, nullptr);
  ASSERT_NE(label4, nullptr);

  EXPECT_EQ(label1->value->Evaluate(*symbols), 0x8000);
  EXPECT_EQ(label2->value->Evaluate(*symbols), 0x8001);
  EXPECT_EQ(label3->value->Evaluate(*symbols), 0x8002);
  EXPECT_EQ(label4->value->Evaluate(*symbols), 0x2003);
}

/**
 * @brief Test phase with expression operand
 */
TEST_F(PhaseDirectivesTest, PhaseWithExpression) {
  std::string source = R"(
BASE    .EQ $8000
        .OR $2000
        .PH BASE+$100
LABEL   .DA #$00      ; LABEL should be $8100
        .EP
)";

  parser->Parse(source, *section, *symbols);

  auto label = symbols->GetSymbol("LABEL");
  ASSERT_NE(label, nullptr);
  EXPECT_EQ(label->value->Evaluate(*symbols), 0x8100);
}

/**
 * @brief Test .PH without operand (should error)
 */
TEST_F(PhaseDirectivesTest, PhaseWithoutOperand) {
  std::string source = R"(
        .OR $2000
        .PH           ; Missing operand
)";

  EXPECT_THROW(parser->Parse(source, *section, *symbols), std::runtime_error);
}

/**
 * @brief Test nested phase (second .PH overrides first)
 */
TEST_F(PhaseDirectivesTest, NestedPhase) {
  std::string source = R"(
        .OR $2000
        .PH $8000
LABEL1  .DA #$00      ; $8000
        .PH $9000     ; New phase address
LABEL2  .DA #$00      ; $9000
        .EP
LABEL3  .DA #$00      ; Back to $2002
)";

  parser->Parse(source, *section, *symbols);

  auto label1 = symbols->GetSymbol("LABEL1");
  auto label2 = symbols->GetSymbol("LABEL2");
  auto label3 = symbols->GetSymbol("LABEL3");

  ASSERT_NE(label1, nullptr);
  ASSERT_NE(label2, nullptr);
  ASSERT_NE(label3, nullptr);

  EXPECT_EQ(label1->value->Evaluate(*symbols), 0x8000);
  EXPECT_EQ(label2->value->Evaluate(*symbols), 0x9000);
  EXPECT_EQ(label3->value->Evaluate(*symbols), 0x2002);
}
