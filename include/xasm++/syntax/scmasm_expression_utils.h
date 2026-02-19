/**
 * @file scmasm_expression_utils.h
 * @brief Expression utilities for SCMASM case-insensitive symbol handling
 */

#pragma once

#include <string>

namespace xasm {
namespace scmasm {

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
std::string NormalizeExpression(const std::string &expr);

} // namespace scmasm
} // namespace xasm
