/**
 * @file parser_error_utils.h
 * @brief Utility functions for consistent parser error formatting
 *
 * Provides reusable error formatting and throwing functions that eliminate
 * duplicate error handling patterns across parser implementations.
 *
 * **Purpose:** Extract common error patterns from parser code (P2.3 refactoring)
 *
 * **Benefits:**
 * - Consistent error message formatting across all parsers
 * - Automatic file:line: context injection
 * - Reduces code duplication (176 instances of throw std::runtime_error)
 * - Single source of truth for error message patterns
 * - Easier to update error formats project-wide
 *
 * **Common Patterns Extracted:**
 * - "X requires a label" (7 instances)
 * - "X requires operand" (5 instances)
 * - "X without matching Y" (8 instances)
 * - "X not yet implemented" (5 instances)
 * - "Invalid X value" (numerous instances)
 *
 * **Example Usage:**
 * @code
 * // Before (inconsistent, no context):
 * if (label.empty()) {
 *   throw std::runtime_error("EQU requires a label");
 * }
 *
 * // After (consistent, with file:line context):
 * if (label.empty()) {
 *   ThrowRequiresLabel(context, "EQU");
 * }
 * // Output: "test.asm:42: EQU requires a label"
 * @endcode
 */

#pragma once

#include "xasm++/syntax/directive_registry.h"
#include <string>

namespace xasm {

/**
 * @brief Format error message with file:line: prefix
 *
 * Adds source location context to error messages when available.
 * If file/line not available, returns message unchanged.
 *
 * @param ctx Directive context with file/line information
 * @param message Error message to format
 * @return Formatted error string with location prefix
 *
 * **Example:**
 * @code
 * // context.current_file = "test.asm"
 * // context.current_line = 42
 * std::string error = FormatError(context, "Test error");
 * // Returns: "test.asm:42: Test error"
 * @endcode
 */
std::string FormatError(const DirectiveContext &ctx,
                        const std::string &message);

/**
 * @brief Throw formatted error with file:line: context
 *
 * Convenience function that formats and throws in one call.
 * Equivalent to: throw std::runtime_error(FormatError(ctx, message))
 *
 * @param ctx Directive context with file/line information
 * @param message Error message to throw
 * @throws std::runtime_error with formatted message
 *
 * **Example:**
 * @code
 * ThrowFormattedError(context, "Invalid operand");
 * // Throws: "test.asm:42: Invalid operand"
 * @endcode
 */
[[noreturn]] void ThrowFormattedError(const DirectiveContext &ctx,
                                      const std::string &message);

/**
 * @brief Throw "X requires a label" error
 *
 * Standard error for directives that require a label but none was provided.
 * Replaces 7 instances of this pattern across parsers.
 *
 * @param ctx Directive context with file/line information
 * @param directive_name Name of directive (e.g., "EQU", "SET", "MACRO")
 * @throws std::runtime_error formatted as "file:line: X requires a label"
 *
 * **Example:**
 * @code
 * if (label.empty()) {
 *   ThrowRequiresLabel(context, "EQU");
 * }
 * // Throws: "test.asm:42: EQU requires a label"
 * @endcode
 */
[[noreturn]] void ThrowRequiresLabel(const DirectiveContext &ctx,
                                     const std::string &directive_name);

/**
 * @brief Throw "X requires operand" error
 *
 * Standard error for directives that require an operand but none was provided.
 * Replaces 5 instances of this pattern across parsers.
 *
 * @param ctx Directive context with file/line information
 * @param directive_name Name of directive (e.g., "FCC", "ORG")
 * @throws std::runtime_error formatted as "file:line: X requires operand"
 *
 * **Example:**
 * @code
 * if (operand.empty()) {
 *   ThrowRequiresOperand(context, "FCC");
 * }
 * // Throws: "test.asm:42: FCC requires operand"
 * @endcode
 */
[[noreturn]] void ThrowRequiresOperand(const DirectiveContext &ctx,
                                       const std::string &directive_name);

/**
 * @brief Throw "X without matching Y" error
 *
 * Standard error for mismatched block directives (ENDM without MACRO, etc.).
 * Replaces 8 instances of this pattern across parsers.
 *
 * @param ctx Directive context with file/line information
 * @param closing_directive Name of closing directive (e.g., "ENDM", "ENDIF")
 * @param opening_directive Name of expected opening directive (e.g., "MACRO",
 * "IF")
 * @throws std::runtime_error formatted as "file:line: X without matching Y"
 *
 * **Example:**
 * @code
 * if (conditional_stack.empty()) {
 *   ThrowMissingMatchingDirective(context, "ENDIF", "IF");
 * }
 * // Throws: "test.asm:42: ENDIF without matching IF"
 * @endcode
 *
 * **Multi-Option Example:**
 * @code
 * ThrowMissingMatchingDirective(context, "ENDM", "MACRO/REPT/IRP");
 * // Throws: "test.asm:42: ENDM without matching MACRO/REPT/IRP"
 * @endcode
 */
[[noreturn]] void
ThrowMissingMatchingDirective(const DirectiveContext &ctx,
                              const std::string &closing_directive,
                              const std::string &opening_directive);

/**
 * @brief Throw "X directive not yet implemented" error
 *
 * Standard error for directives that are recognized but not yet implemented.
 * Replaces 5 instances of this pattern across parsers.
 *
 * @param ctx Directive context with file/line information
 * @param directive_name Name of directive (e.g., "INCLUDE", "DD")
 * @param operand Optional operand value to include in message
 * @throws std::runtime_error formatted as "file:line: X directive not yet
 * implemented: operand"
 *
 * **Example:**
 * @code
 * ThrowNotImplemented(context, "INCLUDE", "myfile.asm");
 * // Throws: "test.asm:42: INCLUDE directive not yet implemented: myfile.asm"
 *
 * ThrowNotImplemented(context, "DD", "");
 * // Throws: "test.asm:42: DD directive not yet implemented"
 * @endcode
 */
[[noreturn]] void ThrowNotImplemented(const DirectiveContext &ctx,
                                      const std::string &directive_name,
                                      const std::string &operand);

/**
 * @brief Throw "Invalid X value" error with optional reason
 *
 * Standard error for invalid values with context about what went wrong.
 * Useful for validation errors with specific constraints.
 *
 * @param ctx Directive context with file/line information
 * @param value_type Type of value (e.g., "RADIX", "address", "count")
 * @param value The invalid value as string
 * @param reason Optional explanation (e.g., "must be between 2 and 16")
 * @throws std::runtime_error formatted as "file:line: Invalid X value: Y
 * (reason)"
 *
 * **Example:**
 * @code
 * if (radix < 2 || radix > 16) {
 *   ThrowInvalidValue(context, "RADIX", std::to_string(radix),
 *                     "must be between 2 and 16");
 * }
 * // Throws: "test.asm:42: Invalid RADIX value: 20 (must be between 2 and 16)"
 *
 * if (address < 0) {
 *   ThrowInvalidValue(context, "address", std::to_string(address), "");
 * }
 * // Throws: "test.asm:42: Invalid address value: -100"
 * @endcode
 */
[[noreturn]] void ThrowInvalidValue(const DirectiveContext &ctx,
                                    const std::string &value_type,
                                    const std::string &value,
                                    const std::string &reason);

} // namespace xasm
