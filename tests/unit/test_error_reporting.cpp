// Error Reporting Tests
// Tests for M8 Refactoring: Error messages with file, line, and context

#include "xasm++/symbol.h"
#include "xasm++/syntax/merlin_syntax.h"
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

using namespace xasm;

// ============================================================================
// Cross-Platform Environment Variable Helpers
// ============================================================================

#ifdef _WIN32
#include <stdlib.h>
inline int setenv_portable(const char *name, const char *value, int) {
  return _putenv_s(name, value);
}
inline int unsetenv_portable(const char *name) { return _putenv_s(name, ""); }
#else
inline int setenv_portable(const char *name, const char *value, int overwrite) {
  return setenv(name, value, overwrite);
}
inline int unsetenv_portable(const char *name) { return unsetenv(name); }
#endif

// ============================================================================
// Platform-Aware Temp Directory Helper
// ============================================================================

static std::string get_temp_dir() {
#ifdef _WIN32
  const char *temp = std::getenv("TEMP");
  if (!temp)
    temp = std::getenv("TMP");
  if (!temp)
    temp = "C:\\Windows\\Temp";
  std::string temp_str(temp);
  // Normalize to forward slashes for consistency
  for (char &c : temp_str) {
    if (c == '\\')
      c = '/';
  }
  return temp_str;
#else
  return "/tmp";
#endif
}

// ============================================================================
// Helper function to check if error contains file:line format
// ============================================================================

bool HasFileLineFormat(const std::string &error_msg) {
  // Match pattern: filename:line: error: message
  // Or simpler: something:number:
  std::regex pattern(R"([^:]+:\d+:)");
  return std::regex_search(error_msg, pattern);
}

// ============================================================================
// Test: Invalid Hex Number Errors
// ============================================================================

TEST(ErrorReportingTest, InvalidHexNumberIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DB $", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
    EXPECT_TRUE(error_msg.find("Invalid hex number") != std::string::npos)
        << "Error message should describe the problem: '" << error_msg << "'";
  }
}

TEST(ErrorReportingTest, InvalidHexNumberMalformedIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DB $XYZ", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: Invalid Binary Number Errors
// ============================================================================

TEST(ErrorReportingTest, InvalidBinaryNumberIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DB %", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
    EXPECT_TRUE(error_msg.find("Invalid binary number") != std::string::npos)
        << "Error message should describe the problem: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: Undefined Symbol Errors
// ============================================================================

TEST(ErrorReportingTest, UndefinedSymbolIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DS UNDEFINED_SYMBOL", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
    EXPECT_TRUE(error_msg.find("Undefined symbol") != std::string::npos)
        << "Error message should describe the problem: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: ORG Directive Errors
// ============================================================================

TEST(ErrorReportingTest, OrgMissingOperandIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         ORG", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: DUM Directive Errors
// ============================================================================

TEST(ErrorReportingTest, DumMissingOperandIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DUM", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: Conditional Assembly Errors
// ============================================================================

TEST(ErrorReportingTest, ElseWithoutDoIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         ELSE", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
    EXPECT_TRUE(error_msg.find("ELSE without matching DO") != std::string::npos)
        << "Error message should describe the problem: '" << error_msg << "'";
  }
}

TEST(ErrorReportingTest, FinWithoutDoIncludesLocation) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         FIN", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: Multi-line Error Location Accuracy
// ============================================================================

TEST(ErrorReportingTest, ErrorOnLine3ShowsCorrectLine) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  std::string source = "         DB $01\n"
                       "         DB $02\n"
                       "         DB $\n" // Error on line 3
                       "         DB $04\n";

  try {
    parser.Parse(source, section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error message should include file:line: '" << error_msg << "'";
    // Check that line 3 is mentioned
    EXPECT_TRUE(error_msg.find(":3:") != std::string::npos ||
                error_msg.find("line 3") != std::string::npos)
        << "Error should indicate line 3: '" << error_msg << "'";
  }
}

// ============================================================================
// Test: Error Format Consistency
// ============================================================================

TEST(ErrorReportingTest, AllErrorsHaveConsistentFormat) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;

  // Test multiple error types to ensure consistency
  std::vector<std::string> error_cases = {
      "         DB $",    // Invalid hex
      "         DB %",    // Invalid binary
      "         ORG",     // Missing operand
      "         ELSE",    // ELSE without DO
      "         DS UNDEF" // Undefined symbol
  };

  for (const auto &error_case : error_cases) {
    Section section("test", 0);
    try {
      parser.Parse(error_case, section, symbols);
      FAIL() << "Expected error for: " << error_case;
    } catch (const std::runtime_error &e) {
      std::string error_msg = e.what();
      EXPECT_TRUE(HasFileLineFormat(error_msg))
          << "All errors should have file:line format. Case: " << error_case
          << ", Error: " << error_msg;
    }
  }
}

// ============================================================================
// Test: Filename in Error Messages
// ============================================================================

TEST(ErrorReportingTest, ErrorIncludesFilename) {
  MerlinSyntaxParser parser;
  ConcreteSymbolTable symbols;
  Section section("test", 0);

  try {
    parser.Parse("         DB $", section, symbols);
    FAIL() << "Expected runtime_error to be thrown";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    // Should have SOME filename (even if it's <stdin> or similar)
    EXPECT_TRUE(HasFileLineFormat(error_msg))
        << "Error should include filename component: '" << error_msg << "'";
  }
}

// ============================================================================
// New Error Formatter Tests - ErrorFormatter Class
// ============================================================================

#include "xasm++/core/error_formatter.h"
#include <fstream>
#include <sstream>

TEST(ErrorFormatterTest, BasicFormatWithoutColors) {
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 42;
  error.location.column = 10;

  std::string formatted = formatter.FormatError(error, nullptr);

  EXPECT_NE(formatted.find("error: undefined symbol 'PLAYER_X'"),
            std::string::npos);
  EXPECT_NE(formatted.find("test.s:42:10"), std::string::npos);
  // Should NOT contain ANSI escape codes
  EXPECT_EQ(formatted.find("\033["), std::string::npos);
}

TEST(ErrorFormatterTest, FormatWithColors) {
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Enabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 42;
  error.location.column = 10;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should contain ANSI escape codes for colors
  EXPECT_NE(formatted.find("\033["), std::string::npos);
}

TEST(ErrorFormatterTest, FormatWithSourceContext) {
  // Create a test file
  std::ofstream test_file(get_temp_dir() + "/test_error_context.s");
  test_file << "; Test file\n";
  test_file << "         ORG $6000\n";
  test_file << "         LDA PLAYER_X  ; Undefined symbol\n";
  test_file << "         RTS\n";
  test_file.close();

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = get_temp_dir() + "/test_error_context.s";
  error.location.line = 3;
  error.location.column = 14;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should include source line
  EXPECT_NE(formatted.find("LDA PLAYER_X"), std::string::npos);
  // Should include line number in margin
  EXPECT_NE(formatted.find("3 |"), std::string::npos);
}

TEST(ErrorFormatterTest, FormatWithColumnMarker) {
  std::ofstream test_file(get_temp_dir() + "/test_column_marker.s");
  test_file << "         LDA PLAYER_X\n";
  test_file.close();

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = get_temp_dir() + "/test_column_marker.s";
  error.location.line = 1;
  error.location.column = 14;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should include column marker (^^^)
  EXPECT_NE(formatted.find("^"), std::string::npos);
}

TEST(ErrorFormatterTest, FormatWithSymbolSuggestions) {
  ConcreteSymbolTable symbols;
  symbols.DefineLabel("PLAYER_Y", 0x1000);
  symbols.DefineLabel("PLAYER_Z", 0x2000);
  symbols.DefineLabel("ENEMY_X", 0x3000);

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should suggest PLAYER_Y (edit distance 1)
  EXPECT_NE(formatted.find("did you mean"), std::string::npos);
  EXPECT_NE(formatted.find("PLAYER_Y"), std::string::npos);
}

TEST(ErrorFormatterTest, MultipleSuggestions) {
  ConcreteSymbolTable symbols;
  symbols.DefineLabel("PLAYER_Y", 0x1000);
  symbols.DefineLabel("PLAYER_Z", 0x2000);
  symbols.DefineLabel("PLAYER_A", 0x3000);

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should suggest multiple symbols
  EXPECT_NE(formatted.find("PLAYER_Y"), std::string::npos);
  EXPECT_NE(formatted.find("PLAYER_Z"), std::string::npos);
  EXPECT_NE(formatted.find("PLAYER_A"), std::string::npos);
}

TEST(ErrorFormatterTest, NoColorEnvironmentVariable) {
  // Set NO_COLOR environment variable
  setenv_portable("NO_COLOR", "1", 1);

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Auto);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should NOT contain ANSI escape codes
  EXPECT_EQ(formatted.find("\033["), std::string::npos);

  // Clean up
  unsetenv_portable("NO_COLOR");
}

TEST(ErrorFormatterTest, AutoColorModeWithTTY) {
  // Note: This test behavior depends on whether stdout is a TTY
  // We just verify it doesn't crash
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Auto);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Just verify it produces output
  EXPECT_FALSE(formatted.empty());
}

TEST(ErrorFormatterTest, EditDistanceCalculation) {
  // Test the edit distance algorithm directly
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  // Create an error to get access to the formatter
  ConcreteSymbolTable symbols;
  symbols.DefineLabel("PLAYER_Y", 0x1000); // Distance 1 from PLAYER_X
  symbols.DefineLabel("ZLAYER_X", 0x2000); // Distance 1 from PLAYER_X
  symbols.DefineLabel("ENEMY_X", 0x3000);  // Distance 3 from PLAYER_X

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should suggest PLAYER_Y and ZLAYER_X (distance 1)
  // Should NOT suggest ENEMY_X (distance 3 > 2)
  EXPECT_NE(formatted.find("PLAYER_Y"), std::string::npos);
  EXPECT_NE(formatted.find("ZLAYER_X"), std::string::npos);
  EXPECT_EQ(formatted.find("ENEMY_X"), std::string::npos);
}
