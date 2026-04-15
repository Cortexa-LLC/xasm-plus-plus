/**
 * @file scmasm_expression_utils.h
 * @brief Expression utilities for SCMASM case-insensitive symbol handling
 */

#pragma once

#include <string>

namespace xasm::scmasm {

/**
 * @brief Normalize identifiers in expression string to uppercase
 *
 * SCMASM uses case-insensitive symbols for compatibility with A2osX API
 * constants. This function normalizes all identifiers (potential symbol names)
 * to uppercase while preserving string literals, numbers, and operators.
 *
 * Examples:
 *   "start+10"         -> "START+10"
 *   "kbd.Data"         -> "KBD.DATA"
 *   "$c000"            -> "$C000"
 *   "HIGH(address)"    -> "HIGH(ADDRESS)"
 *
 * @param expr Expression string to normalize
 * @return Expression with identifiers uppercased
 */
std::string NormalizeExpression(const std::string& expr);

/**
 * @brief Canonicalise SCMASM high-byte prefix operator: '/' -> '>'
 *
 * In SCMASM, a leading '/' before an operand means "high byte of expression"
 * (same as the standard '>' unary operator). This function replaces such
 * prefix '/' occurrences with '>' so that the shared ExpressionParser can
 * handle them without SCMASM-specific logic.
 *
 * A '/' is treated as a high-byte prefix (not division) when it appears at
 * the start of the expression or after an operator, open parenthesis, or comma.
 *
 * Examples:
 *   "/ADDR"            -> ">ADDR"
 *   "$/1000"           -> "$/1000"   (hex literal — not a prefix)
 *   "A/B"              -> "A/B"      (division — not a prefix)
 *   "#/ADDR"           -> "#>ADDR"   (immediate + high-byte prefix)
 *
 * @param expr Expression string (should already be uppercased)
 * @return Expression with prefix '/' replaced by '>'
 */
std::string CanonicalizeSlashHighByte(const std::string& expr);

/**
 * @brief Canonicalise SCMASM single '=' equality operator: '=' -> '=='
 *
 * SCMASM uses a single '=' for equality comparison (e.g. in .DO conditions),
 * while the shared ExpressionParser expects '==' for equality. This function
 * replaces isolated '=' with '==' so that the shared parser handles it
 * correctly.
 *
 * Only standalone '=' tokens are replaced; '==', '!=', '<=', and '>=' are
 * left untouched.
 *
 * Examples:
 *   "X=1"              -> "X==1"
 *   "X==1"             -> "X==1"   (already canonical)
 *   "X!=1"             -> "X!=1"   (not-equal — unchanged)
 *   "X<=1"             -> "X<=1"   (less-or-equal — unchanged)
 *
 * @param expr Expression string (should already be uppercased)
 * @return Expression with standalone '=' replaced by '=='
 */
std::string CanonicalizeEqualityOperator(const std::string& expr);

}  // namespace xasm::scmasm
