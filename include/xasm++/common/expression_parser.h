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
 * comparison    := bitwise_or (('==' | '!=' | '<' | '>' | '<=' | '>=') bitwise_or)*
 * bitwise_or    := bitwise_xor ('|' bitwise_xor)*
 * bitwise_xor   := bitwise_and ('^' bitwise_and)*
 * bitwise_and   := shift ('&' shift)*
 * shift         := add_sub (('<<' | '>>') add_sub)*
 * add_sub       := mul_div (('+' | '-') mul_div)*
 * mul_div       := unary (('*' | '/' | '%') unary)*
 * unary         := ('-' | '~' | '!') unary | primary
 * primary       := NUMBER | SYMBOL | '(' expression ')' | function_call
 * function_call := IDENTIFIER '(' expression ')'
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
   */
  explicit ExpressionParser(const SymbolTable *symbols,
                            const INumberParser *number_parser = nullptr);

  /**
   * @brief Parse an expression string into an Expression AST
   *
   * @param str Expression string to parse
   * @return Parsed expression tree (never null)
   * @throws std::runtime_error if expression has invalid syntax
   */
  std::shared_ptr<Expression> Parse(const std::string &str);

private:
  const SymbolTable *symbols_;       ///< Symbol table for symbol resolution
  const INumberParser *number_parser_; ///< Optional custom number parser
  std::string expr_;                 ///< Current expression being parsed
  size_t pos_;                       ///< Current position in expression

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
