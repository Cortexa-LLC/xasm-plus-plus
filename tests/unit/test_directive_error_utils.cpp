/**
 * @file test_directive_error_utils.cpp
 * @brief Tests for directive error handling utility functions
 *
 * Tests extraction of common error handling patterns from directive handlers.
 * Part of P2.5 refactoring to eliminate duplicate error handling code.
 */

#include "xasm++/directives/directive_error_utils.h"
#include "xasm++/syntax/directive_registry.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace xasm;
using namespace xasm::directive_utils;

// ============================================================================
// FormatError Tests
// ============================================================================

TEST(DirectiveErrorUtilsTest, FormatErrorWithFileAndLine) {
  DirectiveContext context;
  context.current_file = "test.asm";
  context.current_line = 42;

  std::string result = FormatError("Invalid operand", context);

  EXPECT_EQ(result, "test.asm:42: error: Invalid operand");
}

TEST(DirectiveErrorUtilsTest, FormatErrorWithoutFile) {
  DirectiveContext context;
  context.current_file = "";
  context.current_line = 0;

  std::string result = FormatError("Invalid operand", context);

  EXPECT_EQ(result, "Invalid operand");
}

TEST(DirectiveErrorUtilsTest, FormatErrorWithFileButNoLine) {
  DirectiveContext context;
  context.current_file = "test.asm";
  context.current_line = 0;

  std::string result = FormatError("Invalid operand", context);

  EXPECT_EQ(result, "Invalid operand");
}

TEST(DirectiveErrorUtilsTest, FormatErrorWithLineButNoFile) {
  DirectiveContext context;
  context.current_file = "";
  context.current_line = 42;

  std::string result = FormatError("Invalid operand", context);

  EXPECT_EQ(result, "Invalid operand");
}

// ============================================================================
// RequireOperand Tests
// ============================================================================

TEST(DirectiveErrorUtilsTest, RequireOperandThrowsOnEmpty) {
  DirectiveContext context;
  context.current_file = "test.asm";
  context.current_line = 10;

  EXPECT_THROW(
      {
        try {
          RequireOperand("", "ORG", context);
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:10: error: ORG directive requires an operand",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(DirectiveErrorUtilsTest, RequireOperandSucceedsOnNonEmpty) {
  DirectiveContext context;

  // Should not throw
  EXPECT_NO_THROW(RequireOperand("$1000", "ORG", context));
  EXPECT_NO_THROW(RequireOperand("  value  ", "EQU", context));
}

TEST(DirectiveErrorUtilsTest, RequireOperandTrimsBeforeCheck) {
  DirectiveContext context;

  // Whitespace-only should throw
  EXPECT_THROW(RequireOperand("   ", "DB", context), std::runtime_error);
  EXPECT_THROW(RequireOperand("\t\t", "DW", context), std::runtime_error);
}

// ============================================================================
// ValidateParser Tests
// ============================================================================

TEST(DirectiveErrorUtilsTest, ValidateParserThrowsOnNull) {
  EXPECT_THROW(
      {
        try {
          ValidateParser(nullptr);
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("Internal error: parser_state is null", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(DirectiveErrorUtilsTest, ValidateParserSucceedsOnNonNull) {
  int dummy = 42;
  void *parser = &dummy;

  // Should not throw
  EXPECT_NO_THROW(ValidateParser(parser));
}

// ============================================================================
// RequireNonEmpty Tests
// ============================================================================

TEST(DirectiveErrorUtilsTest, RequireNonEmptyThrowsOnEmpty) {
  DirectiveContext context;
  context.current_file = "test.asm";
  context.current_line = 20;

  EXPECT_THROW(
      {
        try {
          RequireNonEmpty("", "String literal", context);
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:20: error: String literal cannot be empty",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(DirectiveErrorUtilsTest, RequireNonEmptySucceedsOnNonEmpty) {
  DirectiveContext context;

  // Should not throw
  EXPECT_NO_THROW(RequireNonEmpty("text", "String", context));
  EXPECT_NO_THROW(RequireNonEmpty("x", "Value", context));
}

// ============================================================================
// ThrowFormattedError Tests
// ============================================================================

TEST(DirectiveErrorUtilsTest, ThrowFormattedErrorWithContext) {
  DirectiveContext context;
  context.current_file = "test.asm";
  context.current_line = 15;

  EXPECT_THROW(
      {
        try {
          ThrowFormattedError("Invalid hex digit 'Z'", context);
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:15: error: Invalid hex digit 'Z'", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(DirectiveErrorUtilsTest, ThrowFormattedErrorWithoutContext) {
  DirectiveContext context;
  context.current_file = "";
  context.current_line = 0;

  EXPECT_THROW(
      {
        try {
          ThrowFormattedError("Parse error", context);
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("Parse error", e.what());
          throw;
        }
      },
      std::runtime_error);
}
