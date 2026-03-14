// Expression error handling tests
// Tests that the expression evaluator properly handles malformed expressions,
// invalid hex input, and unresolved symbols.

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/syntax/merlin_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// Helper to create test assembler with CPU
static Assembler CreateTestAssembler() {
  Assembler assembler;
  static Cpu6502 cpu; // Static so pointer stays valid
  assembler.SetCpuPlugin(&cpu);
  return assembler;
}

// Test that ParseNumber validates hex input properly
TEST(ExpressionErrorHandlingTest, ParseNumberValidatesHexInput) {
  MerlinSyntaxParser parser;

  // Should throw runtime_error, not logic_error
  EXPECT_THROW(
      {
        // Access ParseNumber through ParseExpression
        Section section;
        ConcreteSymbolTable symbols;
        std::string source = R"(
TEST equ $G0
        )";
        parser.Parse(source, section, symbols);
      },
      std::runtime_error);
}

// Test that bare $ assembles as current-location operator
// After migrating to shared ExpressionParser, $ alone is a valid
// "current location" (PC) expression — identical to *.  lda #$ is
// syntactically well-formed and assembles successfully.
TEST(ExpressionErrorHandlingTest, EmptyHexAfterDollarSign) {
  std::string source = R"(
         org $0800
         lda #$      ; $ = current PC (current-location operator)
    )";

  // Create assembler with CPU
  Assembler assembler = CreateTestAssembler();

  Section section;
  ConcreteSymbolTable symbols;
  MerlinSyntaxParser parser;

  // Parse to create atoms
  parser.Parse(source, section, symbols);
  assembler.AddSection(section);

  // Assembly should succeed — $ is current-location, not a malformed hex literal.
  AssemblerResult result = assembler.Assemble();
  EXPECT_TRUE(result.success);
}

// Test that parser handles malformed binary expression
TEST(ExpressionErrorHandlingTest, MalformedBinaryExpression) {
  std::string source = R"(
         org $0800
         lda #%1012  ; Invalid binary digit '2'
    )";

  // Create assembler with CPU
  Assembler assembler = CreateTestAssembler();

  Section section;
  ConcreteSymbolTable symbols;
  MerlinSyntaxParser parser;

  // Parse to create atoms
  parser.Parse(source, section, symbols);
  assembler.AddSection(section);

  // Should throw during assembly when encoding instructions
  bool threw_proper_error = false;
  try {
    AssemblerResult result = assembler.Assemble();
  } catch (const std::runtime_error &e) {
    std::string msg(e.what());
    threw_proper_error = (msg.find("Invalid binary") != std::string::npos);
  }

  EXPECT_TRUE(threw_proper_error);
}

// Test unresolved symbol reference (forward reference issue)
TEST(ExpressionErrorHandlingTest, UnresolvedSymbolInExpression) {
  std::string source = R"(
         org $0800
start    lda #UNDEFINED_SYMBOL
    )";

  Section section;
  ConcreteSymbolTable symbols;
  MerlinSyntaxParser parser;

  // Parser should not crash - it should either:
  // 1. Return 0 for undefined symbol (current behavior)
  // 2. Throw proper error about undefined symbol
  EXPECT_NO_THROW({ parser.Parse(source, section, symbols); });
}

// Test complex expression with forward reference
// This mimics patterns found in POP source code
TEST(ExpressionErrorHandlingTest, ComplexForwardReferenceExpression) {
  std::string source = R"(
         org $0800
start    lda #>FORWARD_LABEL  ; High byte of forward reference
         lda #<FORWARD_LABEL  ; Low byte of forward reference
         rts
FORWARD_LABEL equ $2000
    )";

  Section section;
  ConcreteSymbolTable symbols;
  MerlinSyntaxParser parser;

  // Should not crash - multi-pass assembly should resolve this
  EXPECT_NO_THROW({ parser.Parse(source, section, symbols); });
}
