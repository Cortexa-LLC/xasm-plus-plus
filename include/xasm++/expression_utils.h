/**
 * @file expression_utils.h
 * @brief Expression evaluation utility functions
 *
 * Extracted common expression evaluation patterns to reduce code duplication.
 * Part of P2.2: Extract expression evaluation utilities
 *
 * These utilities encapsulate the common pattern of:
 * 1. Parse expression string
 * 2. Check if parse succeeded
 * 3. Evaluate expression
 * 4. Cast/mask result appropriately
 * 5. Throw exception with context on failure
 */

#pragma once

#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace xasm {

/**
 * Parse and evaluate an expression, throwing on error
 *
 * This utility encapsulates the common pattern:
 * ```cpp
 * auto expr = parser->ParseExpression(operand, symbols);
 * if (!expr) {
 *     throw std::runtime_error("Invalid ... expression");
 * }
 * uint32_t value = expr->Evaluate(symbols);
 * ```
 *
 * @tparam ParserType Type of parser (must have ParseExpression method)
 * @param expr_str Expression string to parse and evaluate
 * @param parser Parser instance to use
 * @param symbols Symbol table for evaluation
 * @param context_name Context name for error messages (e.g., "IF directive")
 * @return Evaluated uint32_t value
 * @throws std::runtime_error if expression is invalid or evaluation fails
 */
// Forward declaration
class ConcreteSymbolTable;

template <typename ParserType>
uint32_t ParseAndEvaluateExpression(const std::string &expr_str,
                                     ParserType &parser, ConcreteSymbolTable &symbols,
                                     const std::string &context_name) {
  auto expr = parser.ParseExpression(expr_str, symbols);
  if (!expr) {
    throw std::runtime_error("Invalid " + context_name + " expression");
  }
  return static_cast<uint32_t>(expr->Evaluate(symbols));
}

/**
 * Parse and evaluate an expression as a byte (uint8_t)
 *
 * This utility combines parsing, evaluation, and masking to 8 bits:
 * ```cpp
 * auto expr = parser->ParseExpression(str, symbols);
 * if (!expr) {
 *     throw std::runtime_error("Invalid ... expression");
 * }
 * uint8_t value = static_cast<uint8_t>(expr->Evaluate(symbols) & 0xFF);
 * ```
 *
 * @tparam ParserType Type of parser (must have ParseExpression method)
 * @param expr_str Expression string to parse and evaluate
 * @param parser Parser instance to use
 * @param symbols Symbol table for evaluation
 * @param context_name Context name for error messages
 * @return Evaluated and masked uint8_t value
 * @throws std::runtime_error if expression is invalid or evaluation fails
 */
template <typename ParserType>
uint8_t ParseAndEvaluateAsByte(const std::string &expr_str, ParserType &parser,
                                ConcreteSymbolTable &symbols,
                                const std::string &context_name) {
  auto expr = parser.ParseExpression(expr_str, symbols);
  if (!expr) {
    throw std::runtime_error("Invalid " + context_name + " expression");
  }
  return static_cast<uint8_t>(expr->Evaluate(symbols) & 0xFF);
}

/**
 * Parse and evaluate an expression as a word (uint16_t)
 *
 * This utility combines parsing, evaluation, and masking to 16 bits:
 * ```cpp
 * auto expr = parser->ParseExpression(str, symbols);
 * if (!expr) {
 *     throw std::runtime_error("Invalid ... expression");
 * }
 * uint16_t value = static_cast<uint16_t>(expr->Evaluate(symbols) & 0xFFFF);
 * ```
 *
 * @tparam ParserType Type of parser (must have ParseExpression method)
 * @param expr_str Expression string to parse and evaluate
 * @param parser Parser instance to use
 * @param symbols Symbol table for evaluation
 * @param context_name Context name for error messages
 * @return Evaluated and masked uint16_t value
 * @throws std::runtime_error if expression is invalid or evaluation fails
 */
template <typename ParserType>
uint16_t ParseAndEvaluateAsWord(const std::string &expr_str,
                                 ParserType &parser, ConcreteSymbolTable &symbols,
                                 const std::string &context_name) {
  auto expr = parser.ParseExpression(expr_str, symbols);
  if (!expr) {
    throw std::runtime_error("Invalid " + context_name + " expression");
  }
  return static_cast<uint16_t>(expr->Evaluate(symbols) & 0xFFFF);
}

/**
 * Parse and evaluate an expression as a signed 32-bit integer
 *
 * This utility is used for conditional directives that need signed comparison:
 * ```cpp
 * auto expr = parser->ParseExpression(str, symbols);
 * if (!expr) {
 *     throw std::runtime_error("Invalid ... expression");
 * }
 * int32_t value = static_cast<int32_t>(expr->Evaluate(symbols));
 * ```
 *
 * @tparam ParserType Type of parser (must have ParseExpression method)
 * @param expr_str Expression string to parse and evaluate
 * @param parser Parser instance to use
 * @param symbols Symbol table for evaluation
 * @param context_name Context name for error messages
 * @return Evaluated int32_t value
 * @throws std::runtime_error if expression is invalid or evaluation fails
 */
template <typename ParserType>
int32_t ParseAndEvaluateAsSignedInt(const std::string &expr_str,
                                     ParserType &parser, ConcreteSymbolTable &symbols,
                                     const std::string &context_name) {
  auto expr = parser.ParseExpression(expr_str, symbols);
  if (!expr) {
    throw std::runtime_error("Invalid " + context_name + " expression");
  }
  return static_cast<int32_t>(expr->Evaluate(symbols));
}

/**
 * Try to parse and evaluate an expression (non-throwing variant)
 *
 * This utility is used when expressions may contain forward references
 * and should be handled gracefully:
 * ```cpp
 * try {
 *   auto expr = parser->ParseExpression(str, symbols);
 *   if (expr) {
 *     result = expr->Evaluate(symbols);
 *     return true;
 *   }
 * } catch (...) {
 *   return false;
 * }
 * ```
 *
 * @tparam ParserType Type of parser (must have ParseExpression method)
 * @param expr_str Expression string to parse and evaluate
 * @param parser Parser instance to use
 * @param symbols Symbol table for evaluation
 * @param result Output parameter for evaluated value (only set if successful)
 * @return true if parsing and evaluation succeeded, false otherwise
 */
template <typename ParserType>
bool TryParseAndEvaluateExpression(const std::string &expr_str,
                                    ParserType &parser, ConcreteSymbolTable &symbols,
                                    uint32_t &result) {
  try {
    auto expr = parser.ParseExpression(expr_str, symbols);
    if (expr) {
      result = static_cast<uint32_t>(expr->Evaluate(symbols));
      return true;
    }
  } catch (...) {
    // Expression could not be evaluated (e.g., forward reference)
  }
  return false;
}

} // namespace xasm
