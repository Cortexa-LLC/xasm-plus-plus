/**
 * @file test_macro_processor.cpp
 * @brief Unit tests for MacroProcessor class
 * 
 * Tests the standalone macro processor functionality including:
 * - Macro definition storage
 * - Macro lookup
 * - Parameter substitution
 * - Local label uniquification
 * - Macro expansion
 */

#include "xasm++/syntax/macro_processor.h"
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Macro Definition Tests
// ============================================================================

/**
 * Test: DefineMacro - Store simple macro without parameters
 * 
 * Given: A macro with no parameters
 * When: DefineMacro is called
 * Then: Macro is stored and can be retrieved
 */
TEST(MacroProcessorTest, DefineMacroSimple) {
  MacroProcessor processor;

  // Define a simple macro
  MacroDefinition macro;
  macro.name = "DELAY";
  macro.parameters = {};
  macro.body = {"        NOP", "        NOP"};
  macro.definition_line = 1;

  processor.DefineMacro(macro.name, macro.parameters, macro.body);

  // Verify macro is defined
  EXPECT_TRUE(processor.IsMacro("DELAY"));
}

/**
 * Test: DefineMacro - Store macro with parameters
 * 
 * Given: A macro with 2 parameters
 * When: DefineMacro is called
 * Then: Macro is stored with correct parameter list
 */
TEST(MacroProcessorTest, DefineMacroWithParameters) {
  MacroProcessor processor;

  std::vector<std::string> params = {"SRC", "DEST"};
  std::vector<std::string> body = {"        LDA     SRC", "        STA     DEST"};

  processor.DefineMacro("MOVB", params, body);

  // Verify macro is defined
  EXPECT_TRUE(processor.IsMacro("MOVB"));
  
  // Verify can get the macro back
  const MacroDefinition* macro = processor.GetMacro("MOVB");
  ASSERT_NE(macro, nullptr);
  EXPECT_EQ(macro->name, "MOVB");
  EXPECT_EQ(macro->parameters.size(), 2);
  EXPECT_EQ(macro->parameters[0], "SRC");
  EXPECT_EQ(macro->parameters[1], "DEST");
  EXPECT_EQ(macro->body.size(), 2);
}

/**
 * Test: IsMacro - Check if macro exists
 * 
 * Given: Some macros defined
 * When: IsMacro is called
 * Then: Returns true for defined macros, false otherwise
 */
TEST(MacroProcessorTest, IsMacro) {
  MacroProcessor processor;

  processor.DefineMacro("MACRO1", {}, {"NOP"});
  processor.DefineMacro("MACRO2", {}, {"NOP"});

  // Defined macros return true
  EXPECT_TRUE(processor.IsMacro("MACRO1"));
  EXPECT_TRUE(processor.IsMacro("MACRO2"));

  // Undefined macro returns false
  EXPECT_FALSE(processor.IsMacro("UNDEFINED"));
  EXPECT_FALSE(processor.IsMacro("MACRO3"));
}

/**
 * Test: GetMacro - Retrieve macro definition
 * 
 * Given: A defined macro
 * When: GetMacro is called
 * Then: Returns pointer to macro definition
 */
TEST(MacroProcessorTest, GetMacro) {
  MacroProcessor processor;

  std::vector<std::string> params = {"X", "Y"};
  std::vector<std::string> body = {"        LDA     X", "        STA     Y"};
  processor.DefineMacro("TEST", params, body);

  // Get existing macro
  const MacroDefinition* macro = processor.GetMacro("TEST");
  ASSERT_NE(macro, nullptr);
  EXPECT_EQ(macro->name, "TEST");
  EXPECT_EQ(macro->parameters.size(), 2);
  EXPECT_EQ(macro->body.size(), 2);

  // Get non-existing macro
  const MacroDefinition* undefined = processor.GetMacro("NOTFOUND");
  EXPECT_EQ(undefined, nullptr);
}

/**
 * Test: Clear - Remove all macros
 * 
 * Given: Multiple macros defined
 * When: Clear is called
 * Then: All macros are removed
 */
TEST(MacroProcessorTest, Clear) {
  MacroProcessor processor;

  processor.DefineMacro("MACRO1", {}, {"NOP"});
  processor.DefineMacro("MACRO2", {}, {"NOP"});
  processor.DefineMacro("MACRO3", {}, {"NOP"});

  // Verify macros are defined
  EXPECT_TRUE(processor.IsMacro("MACRO1"));
  EXPECT_TRUE(processor.IsMacro("MACRO2"));
  EXPECT_TRUE(processor.IsMacro("MACRO3"));

  // Clear all macros
  processor.Clear();

  // Verify all macros are removed
  EXPECT_FALSE(processor.IsMacro("MACRO1"));
  EXPECT_FALSE(processor.IsMacro("MACRO2"));
  EXPECT_FALSE(processor.IsMacro("MACRO3"));
}

// ============================================================================
// Case Insensitivity Tests
// ============================================================================

/**
 * Test: Case insensitive macro lookup
 * 
 * Given: Macro defined with mixed case
 * When: Looked up with different case
 * Then: Should be found (case insensitive)
 */
TEST(MacroProcessorTest, CaseInsensitiveLookup) {
  MacroProcessor processor;

  processor.DefineMacro("MyMacro", {}, {"NOP"});

  // Should find macro regardless of case
  EXPECT_TRUE(processor.IsMacro("MyMacro"));
  EXPECT_TRUE(processor.IsMacro("MYMACRO"));
  EXPECT_TRUE(processor.IsMacro("mymacro"));
  EXPECT_TRUE(processor.IsMacro("myMACRO"));
}

// ============================================================================
// Parameter Count Tests
// ============================================================================

/**
 * Test: Maximum 8 parameters
 * 
 * Given: Macro with 8 parameters (FLEX spec limit)
 * When: DefineMacro is called
 * Then: Should succeed without error
 */
TEST(MacroProcessorTest, MaximumParameters) {
  MacroProcessor processor;

  std::vector<std::string> params = {"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"};
  std::vector<std::string> body = {"        NOP"};

  // Should succeed (8 parameters is max)
  EXPECT_NO_THROW(processor.DefineMacro("MAXPARAM", params, body));
  
  const MacroDefinition* macro = processor.GetMacro("MAXPARAM");
  ASSERT_NE(macro, nullptr);
  EXPECT_EQ(macro->parameters.size(), 8);
}

// ============================================================================
// Macro Expansion Tests
// ============================================================================

/**
 * Test: ExpandMacro - Simple macro without parameters
 * 
 * Given: A macro with no parameters
 * When: ExpandMacro is called
 * Then: Returns macro body unchanged
 */
TEST(MacroProcessorTest, ExpandMacroSimple) {
  MacroProcessor processor;

  std::vector<std::string> body = {"        NOP", "        NOP", "        RTS"};
  processor.DefineMacro("DELAY", {}, body);

  // Expand macro
  std::vector<std::string> expanded = processor.ExpandMacro("DELAY", {});

  // Verify expansion
  ASSERT_EQ(expanded.size(), 3);
  EXPECT_EQ(expanded[0], "        NOP");
  EXPECT_EQ(expanded[1], "        NOP");
  EXPECT_EQ(expanded[2], "        RTS");
}

/**
 * Test: ExpandMacro - Macro with parameter substitution
 * 
 * Given: A macro with 2 parameters
 * When: ExpandMacro is called with arguments
 * Then: Parameters are substituted with arguments
 */
TEST(MacroProcessorTest, ExpandMacroWithParameters) {
  MacroProcessor processor;

  std::vector<std::string> params = {"SRC", "DEST"};
  std::vector<std::string> body = {
    "        LDA     SRC",
    "        STA     DEST"
  };
  processor.DefineMacro("MOVB", params, body);

  // Expand with arguments
  std::vector<std::string> expanded = processor.ExpandMacro("MOVB", {"$80", "$90"});

  // Verify parameter substitution
  ASSERT_EQ(expanded.size(), 2);
  EXPECT_EQ(expanded[0], "        LDA     $80");
  EXPECT_EQ(expanded[1], "        STA     $90");
}

/**
 * Test: ExpandMacro - Missing arguments
 * 
 * Given: Macro with 2 parameters
 * When: Expanded with only 1 argument
 * Then: Missing parameter substituted with empty string
 */
TEST(MacroProcessorTest, ExpandMacroMissingArguments) {
  MacroProcessor processor;

  std::vector<std::string> params = {"X", "Y"};
  std::vector<std::string> body = {"        LDA     X", "        STA     Y"};
  processor.DefineMacro("TEST", params, body);

  // Expand with only one argument
  std::vector<std::string> expanded = processor.ExpandMacro("TEST", {"$FF"});

  // Second parameter should be empty
  ASSERT_EQ(expanded.size(), 2);
  EXPECT_EQ(expanded[0], "        LDA     $FF");
  EXPECT_EQ(expanded[1], "        STA     "); // Y replaced with empty string
}

/**
 * Test: ExpandMacro - Undefined macro
 * 
 * Given: No macro defined
 * When: ExpandMacro is called
 * Then: Returns empty vector
 */
TEST(MacroProcessorTest, ExpandMacroUndefined) {
  MacroProcessor processor;

  // Try to expand undefined macro
  std::vector<std::string> expanded = processor.ExpandMacro("NOTFOUND", {});

  // Should return empty vector
  EXPECT_TRUE(expanded.empty());
}

/**
 * Test: ExpandMacro - Local label uniquification
 * 
 * Given: Macro with local labels (.LOOP)
 * When: Expanded multiple times
 * Then: Each expansion has unique local labels
 */
TEST(MacroProcessorTest, ExpandMacroLocalLabels) {
  MacroProcessor processor;

  std::vector<std::string> body = {
    ".LOOP   LDA     ,X+",
    "        STA     ,Y+",
    "        LEAX    -1,X",
    "        BNE     .LOOP"
  };
  processor.DefineMacro("COPY", {}, body);

  // First expansion
  std::vector<std::string> expanded1 = processor.ExpandMacro("COPY", {});
  ASSERT_EQ(expanded1.size(), 4);
  EXPECT_EQ(expanded1[0], ".LOOP_001   LDA     ,X+");
  EXPECT_EQ(expanded1[3], "        BNE     .LOOP_001");

  // Second expansion - should have different label
  std::vector<std::string> expanded2 = processor.ExpandMacro("COPY", {});
  ASSERT_EQ(expanded2.size(), 4);
  EXPECT_EQ(expanded2[0], ".LOOP_002   LDA     ,X+");
  EXPECT_EQ(expanded2[3], "        BNE     .LOOP_002");
}

/**
 * Test: ExpandMacro - Multiple local labels
 * 
 * Given: Macro with multiple different local labels
 * When: Expanded once
 * Then: All local labels get unique suffixes
 */
TEST(MacroProcessorTest, ExpandMacroMultipleLocalLabels) {
  MacroProcessor processor;

  std::vector<std::string> body = {
    ".START  LDA     #0",
    "        BEQ     .END",
    ".LOOP   NOP",
    "        BRA     .LOOP",
    ".END    RTS"
  };
  processor.DefineMacro("COMPLEX", {}, body);

  // Expand once
  std::vector<std::string> expanded = processor.ExpandMacro("COMPLEX", {});

  ASSERT_EQ(expanded.size(), 5);
  EXPECT_EQ(expanded[0], ".START_001  LDA     #0");
  EXPECT_EQ(expanded[1], "        BEQ     .END_001");
  EXPECT_EQ(expanded[2], ".LOOP_001   NOP");
  EXPECT_EQ(expanded[3], "        BRA     .LOOP_001");
  EXPECT_EQ(expanded[4], ".END_001    RTS");
}

/**
 * Test: ExpandMacro - Parameter substitution with word boundaries
 * 
 * Given: Macro with parameter that's substring of another word
 * When: Expanded with argument
 * Then: Only whole-word matches are substituted
 */
TEST(MacroProcessorTest, ExpandMacroParameterWordBoundaries) {
  MacroProcessor processor;

  std::vector<std::string> params = {"A"};
  std::vector<std::string> body = {
    "        LDA     A",      // Should substitute
    "        STA     DATA",   // Should NOT substitute (A is part of DATA)
    "        BRA     LABEL"   // Should NOT substitute (A is part of LABEL)
  };
  processor.DefineMacro("TEST", params, body);

  // Expand with argument
  std::vector<std::string> expanded = processor.ExpandMacro("TEST", {"$FF"});

  ASSERT_EQ(expanded.size(), 3);
  EXPECT_EQ(expanded[0], "        LDA     $FF");    // A substituted
  EXPECT_EQ(expanded[1], "        STA     DATA");   // DATA unchanged
  EXPECT_EQ(expanded[2], "        BRA     LABEL");  // LABEL unchanged
}

/**
 * Test: ExpandMacro - Clear resets expansion counter
 * 
 * Given: Macros expanded multiple times
 * When: Clear is called and new macro defined
 * Then: Expansion counter restarts at 1
 */
TEST(MacroProcessorTest, ClearResetsExpansionCounter) {
  MacroProcessor processor;

  std::vector<std::string> body = {".LOOP NOP"};
  processor.DefineMacro("M1", {}, body);

  // Expand twice
  processor.ExpandMacro("M1", {});
  processor.ExpandMacro("M1", {});

  // Clear and define new macro
  processor.Clear();
  processor.DefineMacro("M2", {}, body);

  // Expansion should start at 001 again
  std::vector<std::string> expanded = processor.ExpandMacro("M2", {});
  ASSERT_EQ(expanded.size(), 1);
  EXPECT_EQ(expanded[0], ".LOOP_001 NOP");
}
