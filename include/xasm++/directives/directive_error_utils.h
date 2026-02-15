/**
 * @file directive_error_utils.h
 * @brief Common error handling utilities for directive handlers
 *
 * Provides reusable error formatting and validation functions to eliminate
 * duplicate error handling patterns across directive handler implementations.
 *
 * **Extracted Patterns:**
 * - Error message formatting with file:line context
 * - Operand validation (empty check)
 * - Parser state validation (null check)
 * - Generic non-empty validation
 *
 * **Usage Example:**
 * @code
 * #include "xasm++/directives/directive_error_utils.h"
 *
 * void HandleOrg(const std::string &label, const std::string &operand,
 *                DirectiveContext &context) {
 *   using namespace xasm::directive_utils;
 *
 *   // Validate operand is present
 *   RequireOperand(operand, "ORG", context);
 *
 *   // Validate parser state
 *   auto *parser = static_cast<MyParser *>(context.parser_state);
 *   ValidateParser(parser);
 *
 *   // Throw formatted error if needed
 *   if (invalid_condition) {
 *     ThrowFormattedError("Invalid address format", context);
 *   }
 * }
 * @endcode
 *
 * **Part of:** P2.5 - Extract directive error handling patterns
 * **Eliminates:** ~50+ lines of duplicate error handling code
 */

#pragma once

#include "xasm++/syntax/directive_registry.h"
#include "xasm++/util/string_utils.h"
#include <sstream>
#include <stdexcept>
#include <string>

namespace xasm {
namespace directive_utils {

/**
 * @brief Format error message with source location context
 *
 * Creates standardized error messages in the format:
 *   "file:line: error: message"
 *
 * If file or line is not available, returns just the message.
 *
 * **Replaces Pattern:**
 * ```cpp
 * static std::string FormatError(const std::string &message,
 *                                const DirectiveContext &context) {
 *   std::ostringstream oss;
 *   if (!context.current_file.empty() && context.current_line > 0) {
 *     oss << context.current_file << ":" << context.current_line << ": error:
 * "; } oss << message; return oss.str();
 * }
 * ```
 *
 * @param message Error message
 * @param context Directive execution context (contains file/line info)
 * @return Formatted error message with location context
 */
inline std::string FormatError(const std::string &message,
                               const DirectiveContext &context) {
  std::ostringstream oss;

  // Add file:line prefix if available
  if (!context.current_file.empty() && context.current_line > 0) {
    oss << context.current_file << ":" << context.current_line << ": error: ";
  }

  oss << message;
  return oss.str();
}

/**
 * @brief Validate operand is not empty (trimmed) and throw if missing
 *
 * Trims the operand and checks if result is empty. If empty, throws
 * a formatted error message indicating the directive requires an operand.
 *
 * **Replaces Pattern:**
 * ```cpp
 * if (operand.empty()) {
 *   throw std::runtime_error(
 *     FormatError("DIRECTIVE requires an operand", context));
 * }
 * ```
 *
 * @param operand Operand string to validate
 * @param directive_name Name of directive (for error message)
 * @param context Directive execution context (for error formatting)
 * @throws std::runtime_error if operand is empty or whitespace-only
 */
inline void RequireOperand(const std::string &operand,
                           const std::string &directive_name,
                           const DirectiveContext &context) {
  // Trim and check if empty
  std::string trimmed = util::Trim(operand);
  if (trimmed.empty()) {
    std::string message = directive_name + " directive requires an operand";
    throw std::runtime_error(FormatError(message, context));
  }
}

/**
 * @brief Validate parser state pointer is not null
 *
 * Checks if parser_state is null and throws standardized error if so.
 * This validates the internal consistency of the directive execution context.
 *
 * **Replaces Pattern:**
 * ```cpp
 * auto *parser = static_cast<ParserType *>(context.parser_state);
 * if (!parser) {
 *   throw std::runtime_error("Internal error: parser_state is null");
 * }
 * ```
 *
 * @param parser_state Parser state pointer to validate
 * @throws std::runtime_error if parser_state is null
 */
inline void ValidateParser(const void *parser_state) {
  if (!parser_state) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
}

/**
 * @brief Validate string is not empty and throw formatted error if so
 *
 * Generic validation for any string value that must be non-empty.
 * Useful for validating strings after parsing, extraction, etc.
 *
 * **Usage Example:**
 * ```cpp
 * std::string macro_name = ExtractMacroName(operand);
 * RequireNonEmpty(macro_name, "Macro name", context);
 * ```
 *
 * @param value String value to validate
 * @param description Description of what the value represents (for error
 * message)
 * @param context Directive execution context (for error formatting)
 * @throws std::runtime_error if value is empty
 */
inline void RequireNonEmpty(const std::string &value,
                            const std::string &description,
                            const DirectiveContext &context) {
  if (value.empty()) {
    std::string message = description + " cannot be empty";
    throw std::runtime_error(FormatError(message, context));
  }
}

/**
 * @brief Throw formatted error with context
 *
 * Convenience function to throw a runtime_error with formatted message
 * including file:line context.
 *
 * **Replaces Pattern:**
 * ```cpp
 * throw std::runtime_error(FormatError("Error message", context));
 * ```
 *
 * @param message Error message
 * @param context Directive execution context (for error formatting)
 * @throws std::runtime_error with formatted message
 */
[[noreturn]] inline void ThrowFormattedError(const std::string &message,
                                             const DirectiveContext &context) {
  throw std::runtime_error(FormatError(message, context));
}

} // namespace directive_utils
} // namespace xasm
