/**
 * @file test_parser_error_utils.cpp
 * @brief Unit tests for parser error utility functions
 */

#include "xasm++/syntax/parser_error_utils.h"
#include "xasm++/syntax/directive_registry.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using namespace xasm;

// ============================================================================
// Test Fixtures
// ============================================================================

class ParserErrorUtilsTest : public ::testing::Test {
protected:
  DirectiveContext context_;

  void SetUp() override {
    context_.current_file = "test.asm";
    context_.current_line = 42;
    context_.source_line = "    EQU 100";
  }
};

// ============================================================================
// FormatError Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, FormatError_WithFileAndLine) {
  std::string error = FormatError(context_, "Test error message");
  EXPECT_EQ(error, "test.asm:42: Test error message");
}

TEST_F(ParserErrorUtilsTest, FormatError_WithoutFile) {
  context_.current_file = "";
  std::string error = FormatError(context_, "Test error message");
  EXPECT_EQ(error, "Test error message");
}

TEST_F(ParserErrorUtilsTest, FormatError_WithoutLine) {
  context_.current_line = 0;
  std::string error = FormatError(context_, "Test error message");
  EXPECT_EQ(error, "Test error message");
}

TEST_F(ParserErrorUtilsTest, FormatError_EmptyMessage) {
  std::string error = FormatError(context_, "");
  EXPECT_EQ(error, "test.asm:42: ");
}

// ============================================================================
// ThrowFormattedError Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowFormattedError_WithContext) {
  EXPECT_THROW(
      {
        try {
          ThrowFormattedError(context_, "Test error");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: Test error", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(ParserErrorUtilsTest, ThrowFormattedError_WithoutContext) {
  context_.current_file = "";
  context_.current_line = 0;

  EXPECT_THROW(
      {
        try {
          ThrowFormattedError(context_, "Test error");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("Test error", e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// ThrowRequiresLabel Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowRequiresLabel_BasicUsage) {
  EXPECT_THROW(
      {
        try {
          ThrowRequiresLabel(context_, "EQU");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: EQU requires a label", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(ParserErrorUtilsTest, ThrowRequiresLabel_DifferentDirectives) {
  EXPECT_THROW(
      {
        try {
          ThrowRequiresLabel(context_, "SET");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: SET requires a label", e.what());
          throw;
        }
      },
      std::runtime_error);

  EXPECT_THROW(
      {
        try {
          ThrowRequiresLabel(context_, "MACRO");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: MACRO requires a label", e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// ThrowRequiresOperand Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowRequiresOperand_BasicUsage) {
  EXPECT_THROW(
      {
        try {
          ThrowRequiresOperand(context_, "FCC");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: FCC requires operand", e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// ThrowMissingMatchingDirective Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowMissingMatchingDirective_BasicUsage) {
  EXPECT_THROW(
      {
        try {
          ThrowMissingMatchingDirective(context_, "ENDM", "MACRO");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: ENDM without matching MACRO", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(ParserErrorUtilsTest, ThrowMissingMatchingDirective_MultipleOptions) {
  EXPECT_THROW(
      {
        try {
          ThrowMissingMatchingDirective(context_, "ENDM", "MACRO/REPT/IRP");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: ENDM without matching MACRO/REPT/IRP",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// ThrowNotImplemented Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowNotImplemented_WithOperand) {
  EXPECT_THROW(
      {
        try {
          ThrowNotImplemented(context_, "INCLUDE", "myfile.asm");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: INCLUDE directive not yet implemented: "
                       "myfile.asm",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(ParserErrorUtilsTest, ThrowNotImplemented_WithoutOperand) {
  EXPECT_THROW(
      {
        try {
          ThrowNotImplemented(context_, "INCLUDE", "");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: INCLUDE directive not yet implemented",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// ThrowInvalidValue Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ThrowInvalidValue_WithValue) {
  EXPECT_THROW(
      {
        try {
          ThrowInvalidValue(context_, "RADIX", "20",
                            "must be between 2 and 16");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: Invalid RADIX value: 20 (must be between "
                       "2 and 16)",
                       e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST_F(ParserErrorUtilsTest, ThrowInvalidValue_WithoutReason) {
  EXPECT_THROW(
      {
        try {
          ThrowInvalidValue(context_, "address", "-100", "");
        } catch (const std::runtime_error &e) {
          EXPECT_STREQ("test.asm:42: Invalid address value: -100", e.what());
          throw;
        }
      },
      std::runtime_error);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ParserErrorUtilsTest, ErrorsContainFileContext) {
  // All error functions should include file:line: prefix when available
  std::vector<std::string> expected_prefix = {"test.asm:42: "};

  try {
    ThrowRequiresLabel(context_, "EQU");
  } catch (const std::runtime_error &e) {
    EXPECT_THAT(e.what(), ::testing::StartsWith("test.asm:42: "));
  }

  try {
    ThrowRequiresOperand(context_, "FCC");
  } catch (const std::runtime_error &e) {
    EXPECT_THAT(e.what(), ::testing::StartsWith("test.asm:42: "));
  }

  try {
    ThrowMissingMatchingDirective(context_, "ENDM", "MACRO");
  } catch (const std::runtime_error &e) {
    EXPECT_THAT(e.what(), ::testing::StartsWith("test.asm:42: "));
  }

  try {
    ThrowNotImplemented(context_, "DD", "1234");
  } catch (const std::runtime_error &e) {
    EXPECT_THAT(e.what(), ::testing::StartsWith("test.asm:42: "));
  }

  try {
    ThrowInvalidValue(context_, "count", "-1", "must be >= 0");
  } catch (const std::runtime_error &e) {
    EXPECT_THAT(e.what(), ::testing::StartsWith("test.asm:42: "));
  }
}
