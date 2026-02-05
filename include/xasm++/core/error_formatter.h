/**
 * @file error_formatter.h
 * @brief Enhanced error formatting with Rust-style diagnostics
 *
 * Provides rich error messages with:
 * - Source context with line numbers
 * - Column markers pointing to error location
 * - ANSI color support (with auto-detection)
 * - Symbol suggestions for typos
 *
 * Example output:
 * @code
 * error: undefined symbol 'PLAYER_X'
 *   --> test.s:42:10
 *    |
 * 42 |     LDA PLAYER_X
 *    |         ^^^^^^^^ not defined
 *    |
 * help: did you mean 'PLAYER_Y'?
 * @endcode
 */

#pragma once

#include "xasm++/assembler.h"
#include "xasm++/symbol.h"
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Enhanced error formatter with Rust-style diagnostics
 *
 * Formats AssemblerError messages with rich context including:
 * - Source code lines with line numbers
 * - Column markers (^^^ under error location)
 * - Optional ANSI color codes
 * - Symbol suggestions for undefined symbols
 *
 * Usage:
 * @code
 * ErrorFormatter formatter(ErrorFormatter::ColorMode::Auto);
 * for (const auto& error : result.errors) {
 *     std::cerr << formatter.FormatError(error, &symbols);
 * }
 * @endcode
 */
class ErrorFormatter {
public:
  /**
   * @brief Color output mode
   */
  enum class ColorMode {
    Auto,    ///< Auto-detect based on terminal and NO_COLOR env var
    Enabled, ///< Always use colors
    Disabled ///< Never use colors
  };

  /**
   * @brief Construct formatter with specified color mode
   *
   * @param mode Color mode (default: Auto)
   */
  explicit ErrorFormatter(ColorMode mode = ColorMode::Auto);

  /**
   * @brief Format an assembler error with rich context
   *
   * Creates a multi-line formatted error message including:
   * - Error type and message
   * - File location (file:line:column)
   * - Source context (if file readable)
   * - Column marker pointing to error
   * - Symbol suggestions (if applicable)
   *
   * @param error The error to format
   * @param symbols Symbol table for generating suggestions (nullable)
   * @return Formatted error message string
   */
  std::string FormatError(const AssemblerError &error,
                          const ConcreteSymbolTable *symbols) const;

  /**
   * @brief Calculate Levenshtein edit distance between two strings
   *
   * Used to find similar symbol names for suggestions.
   *
   * @param s1 First string
   * @param s2 Second string
   * @return Minimum number of edits (insert, delete, substitute) needed
   */
  static size_t CalculateEditDistance(const std::string &s1,
                                      const std::string &s2);

private:
  ColorMode color_mode_; ///< Color output mode

  /**
   * @brief Check if color output should be used
   *
   * Takes into account:
   * - Configured color mode
   * - NO_COLOR environment variable
   * - Terminal capabilities (if Auto mode)
   *
   * @return true if colors should be used
   */
  bool ShouldUseColors() const;

  /**
   * @brief Apply ANSI color code to text
   *
   * @param text Text to colorize
   * @param color ANSI color code (e.g., "31" for red)
   * @return Colorized text with reset at end
   */
  std::string Colorize(const std::string &text, const std::string &color) const;

  /**
   * @brief Read a specific line from a file
   *
   * @param filename Path to file
   * @param line_number Line number (1-based)
   * @return Line content (without newline) or empty string if not found
   */
  std::string ReadSourceLine(const std::string &filename,
                             size_t line_number) const;

  /**
   * @brief Generate column marker line
   *
   * Creates a line like:
   * @code
   *    |         ^^^^^^^^ not defined
   * @endcode
   *
   * @param column Column position (1-based)
   * @param length Length of marker (default: 8)
   * @param message Message to show after marker (optional)
   * @return Formatted marker line
   */
  std::string GenerateColumnMarker(size_t column, size_t length,
                                   const std::string &message) const;

  /**
   * @brief Find similar symbols for suggestions
   *
   * Searches symbol table for symbols with small edit distance.
   * Returns up to 3 best matches with edit distance <= 2.
   *
   * @param typo The misspelled symbol name
   * @param symbols Symbol table to search
   * @return Vector of suggested symbol names (may be empty)
   */
  std::vector<std::string>
  FindSimilarSymbols(const std::string &typo,
                     const ConcreteSymbolTable *symbols) const;

  /**
   * @brief Extract symbol name from error message
   *
   * Looks for patterns like:
   * - "undefined symbol 'FOO'"
   * - "symbol 'FOO' not found"
   *
   * @param message Error message
   * @return Symbol name if found, empty string otherwise
   */
  std::string ExtractSymbolName(const std::string &message) const;

  /**
   * @brief Format symbol suggestions as help text
   *
   * Creates help text like:
   * @code
   * help: did you mean 'PLAYER_Y'?
   * @endcode
   * or:
   * @code
   * help: did you mean one of these?
   *       - PLAYER_A
   *       - PLAYER_B
   * @endcode
   *
   * @param suggestions Vector of symbol names
   * @return Formatted help text (empty if no suggestions)
   */
  std::string
  FormatSuggestions(const std::vector<std::string> &suggestions) const;
};

} // namespace xasm
