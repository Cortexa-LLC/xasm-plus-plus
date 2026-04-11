/**
 * @file expression_parser.h
 * @brief Shared expression parser for all syntax parsers
 *
 * This component extracts duplicated expression parsing logic from the
 * Z80Universal, Merlin, and SCMASM syntax parsers into a single reusable
 * implementation. It reduces ~1,200 LOC of duplication and centralizes
 * expression grammar handling.
 *
 * @note Part of god-class refactoring Phase 1
 */

#pragma once

#include "xasm++/expression.h"
#include <cstddef>
#include <memory>
#include <string>

namespace xasm {

/**
 * @brief Feature flags for syntax-specific expression parsing
 *
 * Controls which dialect-specific features are active in ExpressionParser.
 * This enables the shared parser to be used across all syntax modes while
 * keeping syntax-specific behaviour isolated.
 *
 * @note ADR-005: Each flag corresponds to a specific syntax dialect feature
 *       that must not leak into modes that don't own it.
 */
struct ParserFeatures {
  /**
   * @brief Allow `[expr]` as an alternative grouping operator (Z80/EDTASM)
   *
   * When true, square brackets may be used in place of parentheses for
   * expression grouping, e.g. `[5 + 3] * 2`.  This is a Z80/EDTASM
   * convention; it must NOT be active for Merlin, SCMASM, or 6502 modes
   * where `[` has a different meaning or is simply invalid.
   *
   * ADR-005 V7.
   */
  bool allow_bracket_grouping = false;

  /**
   * @brief Allow `]var` as an identifier prefix (Merlin DUM-block variables)
   *
   * When true, a `]` character is recognised as a valid identifier start,
   * matching Merlin's `]variable` label convention.  This must NOT be active
   * for Z80, SCMASM, or plain 6502 modes.
   *
   * ADR-005 V8.
   */
  bool allow_merlin_var_prefix = false;

  /**
   * @brief Enable Merlin-specific bitwise operators: `.` (OR) and `!` (XOR)
   *
   * When true, the period character `.` is recognised as a binary bitwise OR
   * operator, and the exclamation mark `!` is recognised as a binary bitwise
   * XOR operator, matching Merlin 8/16 assembler conventions:
   *
   *   SYM & $F0   → bitwise AND  (& is always available)
   *   SYM . $0F   → bitwise OR   (. only active when this flag is set)
   *   SYM ! $01   → bitwise XOR  (! only active as binary op when this flag is set)
   *
   * This flag also prevents `.` from being consumed as part of an identifier
   * body so that `SYMBOL.$FF` is parsed as `SYMBOL OR $FF` rather than the
   * single undefined symbol `SYMBOL.$FF`.
   *
   * Must NOT be active for SCMASM, Z80, or plain 6502 modes where `.` is
   * part of local label names (e.g., `X.BasePath..1`).
   *
   * ADR-005 V9.
   */
  bool allow_merlin_bitwise_ops = false;

  /**
   * @brief Make `>` and `<` (high/low byte) capture the full additive expression
   *
   * In Merlin syntax, `>base+offset` means `>(base+offset)` — the `>`
   * operator applies to the entire following additive expression, not just the
   * next primary.  So `>$b700+$900 = >$C000 = $C0`, NOT `$B7 + $900`.
   *
   * When false (default), `>` and `<` have standard unary precedence (higher
   * than `+`/`-`).
   */
  bool merlin_byte_ops_greedy = false;

  /** @brief Convenience factory: features for Z80/EDTASM syntax */
  static ParserFeatures ForZ80() {
    ParserFeatures f;
    f.allow_bracket_grouping = true;
    return f;
  }

  /** @brief Convenience factory: features for Merlin syntax */
  static ParserFeatures ForMerlin() {
    ParserFeatures f;
    f.allow_merlin_var_prefix = true;
    f.allow_merlin_bitwise_ops = true;
    f.merlin_byte_ops_greedy = true;
    return f;
  }

  /** @brief Convenience factory: default (no dialect extensions) */
  static ParserFeatures Default() { return {}; }
};

/**
 * @brief Interface for custom number format parsing
 *
 * Allows syntax-specific parsers to extend ExpressionParser with custom
 * number formats (e.g., Z80's "0FFH" hex suffix, "377O" octal suffix).
 *
 * @par Usage Example
 * @code
 * class Z80NumberParser : public INumberParser {
 *   bool TryParse(const std::string& token, int64_t& value) const override {
 *     // Parse Z80-specific formats: 0FFH, 377O, 11111111B, etc.
 *   }
 * };
 * @endcode
 */
class INumberParser {
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~INumberParser() = default;

  /**
   * @brief Attempt to parse a custom number format
   *
   * @param token Token string to parse (e.g., "0FFH", "377O")
   * @param value Output parameter for parsed value
   * @return true if token was successfully parsed, false otherwise
   *
   * @note If TryParse returns false, ExpressionParser will fall back to
   *       standard number formats ($FF, 0xFF, %10101010, etc.)
   */
  virtual bool TryParse(const std::string &token, int64_t &value) const = 0;
};

/**
 * @brief Recursive descent expression parser
 *
 * Parses string expressions into Expression AST trees using recursive descent
 * with operator precedence. Supports:
 * - Literals: decimal, hexadecimal ($FF, 0xFF), binary (%10101010, 0b10101010)
 * - Operators: +, -, *, /, %, &, |, ^, <<, >>, &&, ||
 * - Merlin dialect operators: `.` (bitwise OR), `!` (bitwise XOR)
 * - Unary operators: -, ~, !, <, >
 * - Parentheses for grouping
 * - Function calls: LOW(), HIGH()
 * - Symbol references (via SymbolTable)
 *
 * Grammar (operator precedence, low to high):
 * @code
 * expression    := logical_or
 * logical_or    := logical_and ('||' logical_and)*
 * logical_and   := comparison ('&&' comparison)*
 * comparison    := bitwise_or (('==' | '!=' | '<' | '>' | '<=' | '>=')
 * bitwise_or)* bitwise_or    := bitwise_xor ('|' bitwise_xor)* bitwise_xor   :=
 * bitwise_and ('^' bitwise_and)* bitwise_and   := shift ('&' shift)* shift :=
 * add_sub (('<<' | '>>') add_sub)* add_sub       := mul_div (('+' | '-')
 * mul_div)* mul_div       := unary (('*' | '/' | '%') unary)* unary         :=
 * ('-' | '~' | '!') unary | primary primary       := NUMBER | SYMBOL | '('
 * expression ')' | function_call function_call := IDENTIFIER '(' expression ')'
 * @endcode
 *
 * @par Usage Example
 * @code
 * MockSymbolTable symbols;
 * symbols.AddSymbol("start", 0x8000);
 *
 * ExpressionParser parser(&symbols);
 * auto expr = parser.Parse("start + $10 * 2");
 * int64_t value = expr->Evaluate(symbols);  // 0x8020
 * @endcode
 *
 * @par Thread Safety
 * Not thread-safe. Each thread should have its own ExpressionParser instance.
 */
class ExpressionParser {
public:
  /**
   * @brief Construct an expression parser with a symbol table
   *
   * @param symbols Symbol table for resolving symbol references.
   *                Must remain valid for the lifetime of the parser.
   * @param number_parser Optional custom number parser for syntax-specific
   *                      number formats. If null, only standard formats are
   *                      supported. Must remain valid for lifetime of parser.
   * @param features Optional dialect feature flags (defaults to no extensions).
   */
  explicit ExpressionParser(const SymbolTable *symbols,
                            const INumberParser *number_parser = nullptr,
                            ParserFeatures features = {});

  /**
   * @brief Parse an expression string into an Expression AST
   *
   * @param str Expression string to parse
   * @return Parsed expression tree (never null)
   * @throws std::runtime_error if expression has invalid syntax
   */
  std::shared_ptr<Expression> Parse(const std::string &str);

private:
  const SymbolTable *symbols_;         ///< Symbol table for symbol resolution
  const INumberParser *number_parser_; ///< Optional custom number parser
  ParserFeatures features_;            ///< Dialect feature flags
  std::string expr_ = {};              ///< Current expression being parsed
  size_t pos_ = 0;                     ///< Current position in expression

  // ========================================================================
  // Recursive descent parsing methods (precedence order: low to high)
  // ========================================================================

  /**
   * @brief Parse logical OR expression (||)
   * @return Expression tree for logical OR
   */
  std::shared_ptr<Expression> ParseLogicalOr();

  /**
   * @brief Parse logical AND expression (&&)
   * @return Expression tree for logical AND
   */
  std::shared_ptr<Expression> ParseLogicalAnd();

  /**
   * @brief Parse comparison expression (==, !=, <, >, <=, >=)
   * @return Expression tree for comparison
   */
  std::shared_ptr<Expression> ParseComparison();

  /**
   * @brief Parse bitwise OR expression (|)
   * @return Expression tree for bitwise OR
   */
  std::shared_ptr<Expression> ParseBitwiseOr();

  /**
   * @brief Parse bitwise XOR expression (^)
   * @return Expression tree for bitwise XOR
   */
  std::shared_ptr<Expression> ParseBitwiseXor();

  /**
   * @brief Parse bitwise AND expression (&)
   * @return Expression tree for bitwise AND
   */
  std::shared_ptr<Expression> ParseBitwiseAnd();

  /**
   * @brief Parse shift expression (<<, >>)
   * @return Expression tree for shifts
   */
  std::shared_ptr<Expression> ParseShift();

  /**
   * @brief Parse addition/subtraction expression (+, -)
   * @return Expression tree for add/subtract
   */
  std::shared_ptr<Expression> ParseAddSub();

  /**
   * @brief Parse multiplication/division/modulo expression (*, /, %)
   * @return Expression tree for mul/div/mod
   */
  std::shared_ptr<Expression> ParseMulDiv();

  /**
   * @brief Parse unary expression (-, ~, !, <, >)
   * @return Expression tree for unary operations
   */
  std::shared_ptr<Expression> ParseUnary();

  /**
   * @brief Parse primary expression (literal, symbol, parentheses, function)
   * @return Expression tree for primary element
   */
  std::shared_ptr<Expression> ParsePrimary();

  // ========================================================================
  // Helper methods
  // ========================================================================

  /**
   * @brief Skip whitespace at current position
   */
  void SkipWhitespace();

  /**
   * @brief Peek at current character without consuming
   * @return Current character, or '\0' if at end
   */
  char Peek() const;

  /**
   * @brief Consume and return current character
   * @return Current character, or '\0' if at end
   */
  char Consume();

  /**
   * @brief Check if current position matches a string
   * @param str String to match
   * @return true if match and consume, false otherwise
   */
  bool Match(const std::string &str);

  /**
   * @brief Parse a number literal (decimal, hex, binary)
   * @return Parsed number value
   * @throws std::runtime_error if invalid number format
   */
  int64_t ParseNumber();

  /**
   * @brief Parse an identifier (symbol or function name)
   * @return Parsed identifier string
   */
  std::string ParseIdentifier();
};

} // namespace xasm
