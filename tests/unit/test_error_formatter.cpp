// Error Formatter Tests
// Tests for enhanced error diagnostics with Rust-style formatting

#include "xasm++/assembler.h"
#include "xasm++/core/error_formatter.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

using namespace xasm;

// ============================================================================
// Phase 1: Basic Error Formatting Tests
// ============================================================================

TEST(ErrorFormatterTest, FormatBasicError) {
  // RED: Test for basic error formatting without colors
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = "test.s";
  error.location.line = 42;
  error.location.column = 10;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should contain error type
  EXPECT_NE(formatted.find("error:"), std::string::npos);

  // Should contain message
  EXPECT_NE(formatted.find("undefined symbol 'PLAYER_X'"), std::string::npos);

  // Should contain location in format: --> file:line:column
  EXPECT_NE(formatted.find("test.s:42:10"), std::string::npos);
}

TEST(ErrorFormatterTest, FormatWithSourceContext) {
  // RED: Test that source context is displayed
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  // Create a test source file
  std::string test_file = "/tmp/test_error_formatter_source.s";
  std::ofstream out(test_file);
  out << "         ORG $8000\n";
  out << "         LDA PLAYER_X\n";
  out << "         STA $0400\n";
  out.close();

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'";
  error.location.filename = test_file;
  error.location.line = 2;
  error.location.column = 14;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should contain the source line
  EXPECT_NE(formatted.find("LDA PLAYER_X"), std::string::npos);

  // Should have line number prefix
  EXPECT_NE(formatted.find("2 |"), std::string::npos);

  // Cleanup
  std::remove(test_file.c_str());
}

// ============================================================================
// Phase 2: Color Support Tests
// ============================================================================

TEST(ErrorFormatterTest, FormatWithColors) {
  // RED: Test that colors are applied when enabled
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Enabled);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should contain ANSI color codes
  EXPECT_NE(formatted.find("\033["), std::string::npos);
}

TEST(ErrorFormatterTest, FormatNoColors) {
  // RED: Test that colors are not applied when disabled
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should NOT contain ANSI color codes
  EXPECT_EQ(formatted.find("\033["), std::string::npos);
}

TEST(ErrorFormatterTest, RespectNoColorEnv) {
  // RED: Test that NO_COLOR environment variable is respected
  setenv("NO_COLOR", "1", 1);

  ErrorFormatter formatter(ErrorFormatter::ColorMode::Auto);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should NOT contain ANSI color codes when NO_COLOR is set
  EXPECT_EQ(formatted.find("\033["), std::string::npos);

  unsetenv("NO_COLOR");
}

// ============================================================================
// Phase 3: Column Marker Tests
// ============================================================================

TEST(ErrorFormatterTest, GenerateColumnMarker) {
  // RED: Test column marker generation
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  // Create test file with simple line
  std::string test_file = "/tmp/test_marker.s";
  std::ofstream out(test_file);
  out << "         LDA PLAYER_X\n";
  out.close();

  AssemblerError error;
  error.message = "undefined symbol";
  error.location.filename = test_file;
  error.location.line = 1;
  error.location.column = 14; // Points to 'P' in PLAYER_X

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should contain marker pointing to column
  // Marker should be ^^^^^ under PLAYER_X
  EXPECT_NE(formatted.find("^"), std::string::npos);

  std::remove(test_file.c_str());
}

TEST(ErrorFormatterTest, MarkerWithTabsAndSpaces) {
  // RED: Test that tabs are handled correctly in markers
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  std::string test_file = "/tmp/test_tabs.s";
  std::ofstream out(test_file);
  out << "\t\tLDA PLAYER_X\n"; // Tabs before instruction
  out.close();

  AssemblerError error;
  error.message = "undefined symbol";
  error.location.filename = test_file;
  error.location.line = 1;
  error.location.column = 18; // After tabs

  std::string formatted = formatter.FormatError(error, nullptr);

  // Should still contain marker
  EXPECT_NE(formatted.find("^"), std::string::npos);

  std::remove(test_file.c_str());
}

// ============================================================================
// Phase 4: Symbol Suggestion Tests
// ============================================================================

TEST(ErrorFormatterTest, LevenshteinDistance) {
  // Test the Levenshtein distance algorithm
  // This is a unit test for internal functionality

  // Note: We'll implement this as a static helper in ErrorFormatter
  EXPECT_EQ(ErrorFormatter::CalculateEditDistance("", ""), 0);
  EXPECT_EQ(ErrorFormatter::CalculateEditDistance("abc", "abc"), 0);
  EXPECT_EQ(ErrorFormatter::CalculateEditDistance("abc", "abd"), 1);
  EXPECT_EQ(ErrorFormatter::CalculateEditDistance("abc", "def"), 3);
  EXPECT_EQ(ErrorFormatter::CalculateEditDistance("kitten", "sitting"), 3);
}

TEST(ErrorFormatterTest, FindSimilarSymbols) {
  // RED: Test finding similar symbols for suggestions
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  ConcreteSymbolTable symbols;
  symbols.DefineLabel("PLAYER_Y", 0x1000);
  symbols.DefineLabel("PLAYER_X", 0x1001);
  symbols.DefineLabel("ENEMY_X", 0x1002);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_Z'"; // Typo: Z instead of Y or X
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should suggest PLAYER_X or PLAYER_Y (both are 1 edit away)
  bool has_suggestion = (formatted.find("PLAYER_X") != std::string::npos) ||
                        (formatted.find("PLAYER_Y") != std::string::npos);
  EXPECT_TRUE(has_suggestion);

  // Should have "help:" or "did you mean?" text
  bool has_help = (formatted.find("help:") != std::string::npos) ||
                  (formatted.find("did you mean") != std::string::npos);
  EXPECT_TRUE(has_help);
}

TEST(ErrorFormatterTest, NoSuggestionIfTooDifferent) {
  // RED: Test that no suggestion is given if symbols are too different
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  ConcreteSymbolTable symbols;
  symbols.DefineLabel("FOOBAR", 0x1000);
  symbols.DefineLabel("BAZQUX", 0x1001);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_X'"; // Completely different
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should NOT suggest symbols that are too different (>2 edits)
  EXPECT_EQ(formatted.find("FOOBAR"), std::string::npos);
  EXPECT_EQ(formatted.find("BAZQUX"), std::string::npos);
  EXPECT_EQ(formatted.find("help:"), std::string::npos);
}

TEST(ErrorFormatterTest, MultipleSuggestions) {
  // RED: Test multiple suggestions when edit distance is equal
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  ConcreteSymbolTable symbols;
  symbols.DefineLabel("PLAYER_A", 0x1000);
  symbols.DefineLabel("PLAYER_B", 0x1001);
  symbols.DefineLabel("PLAYER_C", 0x1002);

  AssemblerError error;
  error.message = "undefined symbol 'PLAYER_D'"; // All are 1 edit away
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  std::string formatted = formatter.FormatError(error, &symbols);

  // Should show multiple suggestions or at least one
  int suggestion_count = 0;
  if (formatted.find("PLAYER_A") != std::string::npos)
    suggestion_count++;
  if (formatted.find("PLAYER_B") != std::string::npos)
    suggestion_count++;
  if (formatted.find("PLAYER_C") != std::string::npos)
    suggestion_count++;

  EXPECT_GE(suggestion_count, 1); // At least one suggestion
}

// ============================================================================
// Edge Cases and Robustness Tests
// ============================================================================

TEST(ErrorFormatterTest, HandleMissingSourceFile) {
  // Test that missing source file doesn't crash
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "test error";
  error.location.filename = "/nonexistent/file.s";
  error.location.line = 1;
  error.location.column = 1;

  // Should not throw exception
  EXPECT_NO_THROW({
    std::string formatted = formatter.FormatError(error, nullptr);
    // Should still have basic error info
    EXPECT_NE(formatted.find("test error"), std::string::npos);
  });
}

TEST(ErrorFormatterTest, HandleInvalidLineNumber) {
  // Test that invalid line number doesn't crash
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  std::string test_file = "/tmp/test_invalid_line.s";
  std::ofstream out(test_file);
  out << "Line 1\n";
  out << "Line 2\n";
  out.close();

  AssemblerError error;
  error.message = "test error";
  error.location.filename = test_file;
  error.location.line = 999; // Beyond file length
  error.location.column = 1;

  EXPECT_NO_THROW(
      { std::string formatted = formatter.FormatError(error, nullptr); });

  std::remove(test_file.c_str());
}

TEST(ErrorFormatterTest, HandleEmptySymbolTable) {
  // Test with no symbols (nullptr)
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  AssemblerError error;
  error.message = "undefined symbol 'FOO'";
  error.location.filename = "test.s";
  error.location.line = 1;
  error.location.column = 1;

  EXPECT_NO_THROW({
    std::string formatted = formatter.FormatError(error, nullptr);
    // Should not crash, just no suggestions
    EXPECT_EQ(formatted.find("help:"), std::string::npos);
  });
}

TEST(ErrorFormatterTest, HandleColumnZero) {
  // Test column 0 (start of line)
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  std::string test_file = "/tmp/test_col_zero.s";
  std::ofstream out(test_file);
  out << "LDA #$00\n";
  out.close();

  AssemblerError error;
  error.message = "test error";
  error.location.filename = test_file;
  error.location.line = 1;
  error.location.column = 0;

  EXPECT_NO_THROW(
      { std::string formatted = formatter.FormatError(error, nullptr); });

  std::remove(test_file.c_str());
}

TEST(ErrorFormatterTest, HandleColumnBeyondLine) {
  // Test column beyond line length
  ErrorFormatter formatter(ErrorFormatter::ColorMode::Disabled);

  std::string test_file = "/tmp/test_col_beyond.s";
  std::ofstream out(test_file);
  out << "Short\n";
  out.close();

  AssemblerError error;
  error.message = "test error";
  error.location.filename = test_file;
  error.location.line = 1;
  error.location.column = 100; // Way beyond line

  EXPECT_NO_THROW(
      { std::string formatted = formatter.FormatError(error, nullptr); });

  std::remove(test_file.c_str());
}
