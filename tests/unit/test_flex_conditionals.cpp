/**
 * @file test_flex_conditionals.cpp
 * @brief FLEX ASM09 Conditional Assembly Tests
 * 
 * Tests for IFC/ENDC conditional assembly directives.
 * Phase 3 of FLEX ASM09 implementation.
 */

#include <gtest/gtest.h>
#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/flex_syntax.h"

using namespace xasm;

// ============================================================================
// Phase 3: Conditional Assembly - Basic IFC/ENDC Tests
// ============================================================================

/**
 * Test: Simple IFC directive with defined symbol
 * 
 * Given: Symbol "DEBUG" is defined
 * When: IFC DEBUG ... ENDC block is encountered
 * Then: Code inside block is assembled
 */
TEST(FlexConditionalTest, IfcDefinedSymbolIncludesCode) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define DEBUG symbol first
  symbols.DefineLabel("DEBUG", 1);

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$42
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have 2 instructions: LDA (inside IFC) and NOP (outside)
  EXPECT_GE(section.atoms.size(), 2);
}

/**
 * Test: Simple IFC directive with undefined symbol
 * 
 * Given: Symbol "DEBUG" is NOT defined
 * When: IFC DEBUG ... ENDC block is encountered
 * Then: Code inside block is NOT assembled
 */
TEST(FlexConditionalTest, IfcUndefinedSymbolExcludesCode) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DEBUG symbol is NOT defined

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$42
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have only 1 instruction: NOP (IFC block skipped)
  EXPECT_EQ(section.atoms.size(), 1);
}

/**
 * Test: IFC with expression evaluation (non-zero = true)
 * 
 * Given: Expression "5+5" evaluates to 10 (non-zero)
 * When: IFC 5+5 ... ENDC block is encountered
 * Then: Code inside block is assembled
 */
TEST(FlexConditionalTest, IfcExpressionNonZeroIncludesCode) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
        ORG     $1000

        IFC     5+5
        LDA     #$42
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have 2 instructions: LDA (5+5=10, true) and NOP
  EXPECT_GE(section.atoms.size(), 2);
}

/**
 * Test: IFC with expression evaluation (zero = false)
 * 
 * Given: Expression "5-5" evaluates to 0 (zero)
 * When: IFC 5-5 ... ENDC block is encountered
 * Then: Code inside block is NOT assembled
 */
TEST(FlexConditionalTest, IfcExpressionZeroExcludesCode) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
        ORG     $1000

        IFC     5-5
        LDA     #$42
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have only 1 instruction: NOP (0 is false)
  EXPECT_EQ(section.atoms.size(), 1);
}

// ============================================================================
// Phase 3: Nested Conditional Tests
// ============================================================================

/**
 * Test: Nested IFC blocks (both true)
 * 
 * Given: Both outer and inner conditions are true
 * When: Nested IFC blocks are encountered
 * Then: All code is assembled
 */
TEST(FlexConditionalTest, NestedIfcBothTrue) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  symbols.DefineLabel("DEBUG", 1);
  symbols.DefineLabel("TRACE", 1);

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$10
        IFC     TRACE
        LDB     #$20
        ENDC
        LDX     #$30
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have: LDA, LDB, LDX, NOP = 4 instructions
  EXPECT_GE(section.atoms.size(), 4);
}

/**
 * Test: Nested IFC blocks (outer true, inner false)
 * 
 * Given: Outer condition true, inner condition false
 * When: Nested IFC blocks are encountered
 * Then: Only outer code assembled, inner block skipped
 */
TEST(FlexConditionalTest, NestedIfcOuterTrueInnerFalse) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  symbols.DefineLabel("DEBUG", 1);
  // TRACE is NOT defined

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$10
        IFC     TRACE
        LDB     #$20
        ENDC
        LDX     #$30
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have: LDA, LDX, NOP = 3 instructions (LDB skipped)
  EXPECT_EQ(section.atoms.size(), 3);
}

/**
 * Test: Nested IFC blocks (outer false)
 * 
 * Given: Outer condition is false
 * When: Nested IFC blocks are encountered
 * Then: Entire outer block skipped (inner condition not evaluated)
 */
TEST(FlexConditionalTest, NestedIfcOuterFalse) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // DEBUG is NOT defined
  symbols.DefineLabel("TRACE", 1);

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$10
        IFC     TRACE
        LDB     #$20
        ENDC
        LDX     #$30
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Should have only NOP (entire outer block skipped)
  EXPECT_EQ(section.atoms.size(), 1);
}

// ============================================================================
// Phase 3: Error Handling Tests
// ============================================================================

/**
 * Test: ENDC without matching IFC
 * 
 * Given: ENDC directive without preceding IFC
 * When: Parser encounters orphan ENDC
 * Then: Parser throws error
 */
TEST(FlexConditionalTest, EndcWithoutIfc) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
        ORG     $1000

        ENDC

        NOP
)";

  EXPECT_THROW(parser.Parse(program, section, symbols), std::runtime_error);
}

/**
 * Test: IFC without matching ENDC
 * 
 * Given: IFC directive without closing ENDC
 * When: Parser reaches end of file
 * Then: Parser throws error about unclosed conditional
 */
TEST(FlexConditionalTest, IfcWithoutEndc) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
        ORG     $1000

        IFC     DEBUG
        LDA     #$42

        NOP
)";

  EXPECT_THROW(parser.Parse(program, section, symbols), std::runtime_error);
}

// ============================================================================
// Phase 3: Integration Tests
// ============================================================================

/**
 * Test: Conditional with macro integration
 * 
 * Given: Macro defined inside conditional block
 * When: Condition is true
 * Then: Macro is defined and can be invoked
 */
TEST(FlexConditionalTest, ConditionalMacroDefinition) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  symbols.DefineLabel("ENABLE_MACROS", 1);

  std::string program = R"(
        ORG     $1000

        IFC     ENABLE_MACROS
CLEAR   MACRO
        CLRA
        CLRB
        ENDM
        ENDC

START   CLEAR
)";

  parser.Parse(program, section, symbols);

  // Should have CLRA, CLRB from macro expansion
  EXPECT_GE(section.atoms.size(), 2);
}

/**
 * Test: Conditional block with labels
 * 
 * Given: Labels defined inside conditional block
 * When: Condition is true
 * Then: Labels are defined in symbol table
 */
TEST(FlexConditionalTest, ConditionalLabels) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  symbols.DefineLabel("DEBUG", 1);

  std::string program = R"(
        ORG     $1000

START   NOP

        IFC     DEBUG
TRACE   LDA     #$42
        ENDC

        NOP
)";

  parser.Parse(program, section, symbols);

  // Verify TRACE label exists
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("TRACE", value));
}

/**
 * Test: Complete program with conditionals
 * 
 * Given: Complete FLEX ASM program with multiple conditionals
 * When: Parsed with specific symbols defined
 * Then: Correct code is assembled based on conditions
 */
TEST(FlexConditionalTest, CompleteProgram) {
  FlexAsmSyntax parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  symbols.DefineLabel("DEBUG", 1);
  // RELEASE is NOT defined

  std::string program = R"(
        NAM     ConditionalTest
        TTL     Complete Conditional Program

        ORG     $1000

START   NOP

        IFC     DEBUG
        JSR     DEBUG_INIT
        ENDC

        IFC     RELEASE
        JSR     RELEASE_INIT
        ENDC

        LDA     #$42

        IFC     DEBUG
DEBUG_INIT
        CLRA
        RTS
        ENDC

        END     START
)";

  parser.Parse(program, section, symbols);

  // Should have: NOP, JSR DEBUG_INIT, LDA, CLRA, RTS
  // (RELEASE block skipped)
  EXPECT_GE(section.atoms.size(), 5);

  // Verify DEBUG_INIT label exists
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("DEBUG_INIT", value));
}
