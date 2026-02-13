// FlexAsmSyntax Macro Processor Tests
// FLEX ASM09 Phase 2: MACRO/ENDM Implementation

#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/flex_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Phase 2: Macro Processor - Parameter Substitution
// ============================================================================

/**
 * Test: SubstituteParameters - Simple case with one parameter
 *
 * Given: Macro line "LDA SRC" with parameter SRC
 * When: Called with argument "$80"
 * Then: Returns "LDA $80"
 */
TEST(FlexMacroTest, SubstituteParametersSimple) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::string
    TestSubstituteParameters(const std::string &line,
                             const MacroDefinition &macro,
                             const std::vector<std::string> &arguments) {
      return SubstituteParameters(line, macro, arguments);
    }
  };

  TestableFlexAsm parser;

  // Create a simple macro definition
  MacroDefinition macro;
  macro.name = "MOVB";
  macro.parameters = {"SRC", "DEST"};
  macro.body = {"        LDA     SRC", "        STA     DEST"};
  macro.definition_line = 1;

  // Test substitution
  std::vector<std::string> arguments = {"$80", "$90"};

  // Test first line: "        LDA     SRC" -> "        LDA     $80"
  std::string result =
      parser.TestSubstituteParameters("        LDA     SRC", macro, arguments);
  EXPECT_EQ(result, "        LDA     $80");

  // Test second line: "        STA     DEST" -> "        STA     $90"
  result =
      parser.TestSubstituteParameters("        STA     DEST", macro, arguments);
  EXPECT_EQ(result, "        STA     $90");

  // Test both parameters in one line
  result = parser.TestSubstituteParameters("        LDA     SRC,DEST", macro,
                                           arguments);
  EXPECT_EQ(result, "        LDA     $80,$90");
}

/**
 * Test: SubstituteParameters - Parameter as substring should not be replaced
 *
 * Given: Macro line "LDA SOURCE" with parameter SRC
 * When: Called with argument "$80"
 * Then: Returns "LDA SOURCE" (SRC not replaced because it's part of SOURCE)
 */
TEST(FlexMacroTest, SubstituteParametersWordBoundary) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::string
    TestSubstituteParameters(const std::string &line,
                             const MacroDefinition &macro,
                             const std::vector<std::string> &arguments) {
      return SubstituteParameters(line, macro, arguments);
    }
  };

  TestableFlexAsm parser;

  MacroDefinition macro;
  macro.name = "TEST";
  macro.parameters = {"SRC"};
  macro.body = {};
  macro.definition_line = 1;

  std::vector<std::string> arguments = {"$80"};

  // "SOURCE" contains "SRC" but should not be replaced (word boundary)
  std::string result = parser.TestSubstituteParameters("        LDA     SOURCE",
                                                       macro, arguments);
  EXPECT_EQ(result, "        LDA     SOURCE");

  // But standalone "SRC" should be replaced
  result =
      parser.TestSubstituteParameters("        LDA     SRC", macro, arguments);
  EXPECT_EQ(result, "        LDA     $80");

  // SRC at end of line should be replaced
  result =
      parser.TestSubstituteParameters("        LDA     SRC", macro, arguments);
  EXPECT_EQ(result, "        LDA     $80");
}

/**
 * Test: SubstituteParameters - Missing argument
 *
 * Given: Macro with 2 parameters but only 1 argument provided
 * When: Substituting second parameter
 * Then: Second parameter replaced with empty string
 */
TEST(FlexMacroTest, SubstituteParametersMissing) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::string
    TestSubstituteParameters(const std::string &line,
                             const MacroDefinition &macro,
                             const std::vector<std::string> &arguments) {
      return SubstituteParameters(line, macro, arguments);
    }
  };

  TestableFlexAsm parser;

  MacroDefinition macro;
  macro.name = "MOVB";
  macro.parameters = {"SRC", "DEST"};
  macro.body = {};
  macro.definition_line = 1;

  // Only provide one argument, second is missing
  std::vector<std::string> arguments = {"$80"};

  // First parameter should be replaced
  std::string result =
      parser.TestSubstituteParameters("        LDA     SRC", macro, arguments);
  EXPECT_EQ(result, "        LDA     $80");

  // Second parameter (missing argument) should be replaced with empty string
  result =
      parser.TestSubstituteParameters("        STA     DEST", macro, arguments);
  EXPECT_EQ(result, "        STA     ");
}

// ============================================================================
// Phase 2: Macro Processor - Local Label Uniquification
// ============================================================================

/**
 * Test: MakeLocalLabelUnique - Simple case
 *
 * Given: Local label ".LOOP" and expansion_id 1
 * When: MakeLocalLabelUnique called
 * Then: Returns ".LOOP_001"
 */
TEST(FlexMacroTest, MakeLocalLabelUniqueSimple) {
  // Create a test class that exposes MakeLocalLabelUnique for testing
  // We'll use a simple wrapper approach
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::string TestMakeLocalLabelUnique(const std::string &label, int id) {
      return MakeLocalLabelUnique(label, id);
    }
  };

  TestableFlexAsm parser;

  // Test: .LOOP with expansion_id 1 -> .LOOP_001
  std::string result = parser.TestMakeLocalLabelUnique(".LOOP", 1);
  EXPECT_EQ(result, ".LOOP_001");

  // Test: .LOOP with expansion_id 2 -> .LOOP_002
  result = parser.TestMakeLocalLabelUnique(".LOOP", 2);
  EXPECT_EQ(result, ".LOOP_002");

  // Test: .DONE with expansion_id 123 -> .DONE_123
  result = parser.TestMakeLocalLabelUnique(".DONE", 123);
  EXPECT_EQ(result, ".DONE_123");
}

/**
 * Test: MakeLocalLabelUnique - Non-local label
 *
 * Given: Regular label "START" (not starting with .)
 * When: MakeLocalLabelUnique called
 * Then: Returns "START" unchanged
 */
TEST(FlexMacroTest, MakeLocalLabelUniqueNonLocal) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::string TestMakeLocalLabelUnique(const std::string &label, int id) {
      return MakeLocalLabelUnique(label, id);
    }
  };

  TestableFlexAsm parser;

  // Non-local labels should pass through unchanged
  std::string result = parser.TestMakeLocalLabelUnique("START", 1);
  EXPECT_EQ(result, "START");

  result = parser.TestMakeLocalLabelUnique("LOOP", 999);
  EXPECT_EQ(result, "LOOP");

  result = parser.TestMakeLocalLabelUnique("MYLABEL", 42);
  EXPECT_EQ(result, "MYLABEL");
}

// ============================================================================
// Phase 2: Macro Processor - Macro Expansion
// ============================================================================

/**
 * Test: ExpandMacro - Simple macro without parameters
 *
 * Given: DELAY macro with no parameters
 * When: ExpandMacro called
 * Then: Returns macro body as-is
 */
TEST(FlexMacroTest, ExpandMacroSimple) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    // Need to access macros_ map (it's private)
    // For testing, we'll use a workaround - create through parsing
    std::vector<std::string>
    TestExpandMacro(const std::string &name,
                    const std::vector<std::string> &arguments) {
      // First add the macro manually to the macros_ map
      MacroDefinition macro;
      macro.name = name;
      macro.parameters = {}; // No parameters for simple test
      macro.body = {"        CLRA", "        CLRB"};
      macro.definition_line = 1;
      macros_[name] = macro;

      return ExpandMacro(name, arguments);
    }
  };

  TestableFlexAsm parser;

  // Test expansion of simple macro with no parameters
  std::vector<std::string> arguments = {};
  std::vector<std::string> result = parser.TestExpandMacro("CLEAR", arguments);

  // Expected: Body returned with substitutions applied and local labels made
  // unique
  ASSERT_GE(result.size(), 2UL); // At least the original 2 lines
  EXPECT_EQ(result[0], "        CLRA");
  EXPECT_EQ(result[1], "        CLRB");
}

/**
 * Test: ExpandMacro - Macro with parameters
 *
 * Given: MOVB macro with SRC,DEST parameters
 * When: ExpandMacro called with $80,$90
 * Then: Returns body with parameters substituted
 */
TEST(FlexMacroTest, ExpandMacroWithParameters) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::vector<std::string>
    TestExpandMacro(const std::string &name,
                    const std::vector<std::string> &arguments) {
      MacroDefinition macro;
      macro.name = name;
      macro.parameters = {"SRC", "DEST"};
      macro.body = {"        LDA     SRC", "        STA     DEST"};
      macro.definition_line = 1;
      macros_[name] = macro;

      return ExpandMacro(name, arguments);
    }
  };

  TestableFlexAsm parser;

  // Test expansion with parameter substitution
  std::vector<std::string> arguments = {"$80", "$90"};
  std::vector<std::string> result = parser.TestExpandMacro("MOVB", arguments);

  // Expected: Parameters substituted
  ASSERT_GE(result.size(), 2UL);
  EXPECT_EQ(result[0], "        LDA     $80");
  EXPECT_EQ(result[1], "        STA     $90");
}

/**
 * Test: ExpandMacro - Macro with local labels
 *
 * Given: COPY macro with .LOOP local label
 * When: ExpandMacro called twice
 * Then: Each expansion has unique label (.LOOP_001, .LOOP_002)
 */
TEST(FlexMacroTest, ExpandMacroWithLocalLabels) {
  class TestableFlexAsm : public FlexAsmSyntax {
  public:
    std::vector<std::string>
    TestExpandMacro(const std::string &name,
                    const std::vector<std::string> &arguments) {
      MacroDefinition macro;
      macro.name = name;
      macro.parameters = {};
      macro.body = {".LOOP   LDA     $80", "        BNE     .LOOP"};
      macro.definition_line = 1;
      macros_[name] = macro;

      return ExpandMacro(name, arguments);
    }
  };

  TestableFlexAsm parser;

  // First expansion - should get .LOOP_001
  std::vector<std::string> result1 = parser.TestExpandMacro("COPY", {});
  ASSERT_GE(result1.size(), 2UL);
  EXPECT_TRUE(result1[0].find(".LOOP_001") != std::string::npos);
  EXPECT_TRUE(result1[1].find(".LOOP_001") != std::string::npos);

  // Second expansion - should get .LOOP_002 (different expansion ID)
  std::vector<std::string> result2 = parser.TestExpandMacro("COPY", {});
  ASSERT_GE(result2.size(), 2UL);
  EXPECT_TRUE(result2[0].find(".LOOP_002") != std::string::npos);
  EXPECT_TRUE(result2[1].find(".LOOP_002") != std::string::npos);
}

// ============================================================================
// Integration Tests: MACRO/ENDM Directive Parsing
// ============================================================================

// Testable wrapper for integration tests
class TestableFlexIntegration : public FlexAsmSyntax {
public:
  using FlexAsmSyntax::IsMacroDefined;
};

TEST(FlexMacroIntegrationTest, DefineMacroSimple) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define a simple macro (no parameters)
  std::string program = R"(
DELAY   MACRO
        NOP
        NOP
        ENDM
)";

  try {
    parser.Parse(program, section, symbols);

    // Verify macro is defined
    EXPECT_TRUE(parser.IsMacroDefined("DELAY"));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

TEST(FlexMacroIntegrationTest, DefineMacroWithParameters) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define macro with parameters
  std::string program = R"(
MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM
)";

  try {
    parser.Parse(program, section, symbols);

    // Verify macro is defined
    EXPECT_TRUE(parser.IsMacroDefined("MOVB"));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

TEST(FlexMacroIntegrationTest, InvokeMacroSimple) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define and invoke a macro
  std::string program = R"(
DELAY   MACRO
        NOP
        ENDM

        DELAY
)";

  try {
    parser.Parse(program, section, symbols);

    // Should successfully expand (not throw exception)
    // Should have one NOP instruction
    EXPECT_GE(section.atoms.size(), 1UL);
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

// ============================================================================
// Additional Integration Tests: Comprehensive Macro Scenarios
// ============================================================================

/**
 * Test: Simple macro with multiple invocations
 *
 * This test validates:
 * - Basic macro definition with no parameters
 * - Multiple macro invocations
 * - Correct code generation for each invocation
 */
TEST(FlexMacroIntegrationTest, SimpleMacroMultipleInvocations) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define CLEAR macro and invoke it twice
  std::string program = R"(
        ORG     $1000

CLEAR   MACRO
        CLRA
        CLRB
        ENDM

START   LDX     #$2000
        CLEAR
        STX     ,Y
        CLEAR
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have: LDX, CLRA, CLRB, STX, CLRA, CLRB
    // At minimum 6 instructions (or atoms)
    EXPECT_GE(section.atoms.size(), 6UL);

    // Verify START label exists and points to $1000
    int64_t value;
    EXPECT_TRUE(symbols.Lookup("START", value));
    EXPECT_EQ(value, 0x1000);
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Macro with parameter substitution
 *
 * This test validates:
 * - Macro parameters in definition
 * - Argument passing during invocation
 * - Correct parameter substitution in macro body
 */
TEST(FlexMacroIntegrationTest, MacroWithParameters) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define MOVB macro with two parameters
  std::string program = R"(
        ORG     $1000

MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM

START   MOVB    $80,$90
        MOVB    $A0,$B0
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have: 2 invocations * 2 instructions = 4 instructions
    EXPECT_GE(section.atoms.size(), 4UL);

    // Verify START label
    int64_t value;
    EXPECT_TRUE(symbols.Lookup("START", value));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Macro with three parameters
 *
 * This test validates:
 * - Multiple parameters (more than 2)
 * - Correct substitution of all parameters
 */
TEST(FlexMacroIntegrationTest, MacroWithThreeParameters) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define ADDM macro with three parameters
  std::string program = R"(
        ORG     $1000

ADDM    MACRO   VAL1,VAL2,RESULT
        LDA     VAL1
        ADDA    VAL2
        STA     RESULT
        ENDM

START   ADDM    $C0,$C1,$C2
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have: 3 instructions (LDA, ADDA, STA)
    EXPECT_GE(section.atoms.size(), 3UL);

    // Verify START label
    int64_t value;
    EXPECT_TRUE(symbols.Lookup("START", value));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Macro with local labels - single invocation
 *
 * This test validates:
 * - Local label definition (.LOOP)
 * - Local label uniquification on first invocation
 * - Branch targets using local labels
 */
TEST(FlexMacroIntegrationTest, MacroWithLocalLabelsSingle) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define COPY macro with local label
  std::string program = R"(
        ORG     $1000

COPY    MACRO   COUNT
        LDX     #COUNT
.LOOP   LDA     ,X+
        STA     ,Y+
        DECB
        BNE     .LOOP
        ENDM

START   LDB     #10
        LDY     #$2000
        COPY    10
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have: LDB, LDY, and expanded COPY (5 instructions)
    EXPECT_GE(section.atoms.size(), 7UL);

    // Verify .LOOP_001 label exists (uniquified)
    int64_t value;
    EXPECT_TRUE(symbols.Lookup(".LOOP_001", value));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Macro with local labels - multiple invocations
 *
 * This test validates:
 * - Each invocation gets unique local labels
 * - .LOOP_001 for first invocation
 * - .LOOP_002 for second invocation
 */
TEST(FlexMacroIntegrationTest, MacroWithLocalLabelsMultiple) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define COPY macro and invoke twice
  std::string program = R"(
        ORG     $1000

COPY    MACRO   COUNT
        LDX     #COUNT
.LOOP   LDA     ,X+
        STA     ,Y+
        DECB
        BNE     .LOOP
        ENDM

START   LDB     #10
        LDY     #$2000
        COPY    10
        LDB     #20
        COPY    20
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have both macro invocations expanded
    EXPECT_GE(section.atoms.size(), 12UL);

    // Verify both unique labels exist
    int64_t loop1_value, loop2_value;
    EXPECT_TRUE(symbols.Lookup(".LOOP_001", loop1_value))
        << ".LOOP_001 should exist for first invocation";
    EXPECT_TRUE(symbols.Lookup(".LOOP_002", loop2_value))
        << ".LOOP_002 should exist for second invocation";

    // They should have different addresses
    EXPECT_NE(loop1_value, loop2_value);
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Macro with multiple local labels
 *
 * This test validates:
 * - Multiple local labels in one macro body
 * - All local labels uniquified with same expansion ID
 */
TEST(FlexMacroIntegrationTest, MacroWithMultipleLocalLabels) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  // Define DELAY macro with two local labels
  std::string program = R"(
        ORG     $1000

DELAY   MACRO   OUTER,INNER
        LDX     #OUTER
.OUTER  LDY     #INNER
.INNER  LEAY    -1,Y
        BNE     .INNER
        LEAX    -1,X
        BNE     .OUTER
        ENDM

START   DELAY   5,100
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have expanded DELAY macro
    EXPECT_GE(section.atoms.size(), 6UL);

    // Verify both local labels exist with same expansion ID
    int64_t outer_value, inner_value;
    EXPECT_TRUE(symbols.Lookup(".OUTER_001", outer_value))
        << ".OUTER_001 should exist";
    EXPECT_TRUE(symbols.Lookup(".INNER_001", inner_value))
        << ".INNER_001 should exist";
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}

/**
 * Test: Complex program with macros, directives, and labels
 *
 * This test validates:
 * - Macros work alongside normal directives (ORG, END)
 * - Macros work with regular labels
 * - Complete program flow
 */
TEST(FlexMacroIntegrationTest, CompleteProgram) {
  TestableFlexIntegration parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string program = R"(
        NAM     TestProg
        TTL     Complete Program Test

MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM

CLEAR   MACRO
        CLRA
        CLRB
        ENDM

        ORG     $1000

START   CLEAR
        MOVB    $80,$90
        MOVB    $A0,$B0
DONE    NOP
        
        END     START
)";

  try {
    parser.Parse(program, section, symbols);

    // Should have all instructions
    EXPECT_GE(section.atoms.size(), 7UL);

    // Verify labels
    int64_t start_value, done_value;
    EXPECT_TRUE(symbols.Lookup("START", start_value));
    EXPECT_EQ(start_value, 0x1000);

    EXPECT_TRUE(symbols.Lookup("DONE", done_value));
  } catch (const std::exception &e) {
    FAIL() << "Exception thrown: " << e.what();
  }
}
