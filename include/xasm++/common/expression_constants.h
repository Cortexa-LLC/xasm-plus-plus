/**
 * @file expression_constants.h
 * @brief Expression parser function name constants
 *
 * Defines constants for built-in expression functions (HIGH, LOW, etc.)
 * to eliminate magic strings in expression parsing code.
 */

#pragma once

namespace xasm {
namespace expression {

// ============================================================================
// Byte Extraction Functions
// ============================================================================

/// HIGH - Extract high byte of 16-bit value (uppercase)
constexpr const char *HIGH = "HIGH";

/// high - Extract high byte of 16-bit value (lowercase)
constexpr const char *high = "high";

/// High - Extract high byte of 16-bit value (mixed case)
constexpr const char *High = "High";

/// LOW - Extract low byte of 16-bit value (uppercase)
constexpr const char *LOW = "LOW";

/// low - Extract low byte of 16-bit value (lowercase)
constexpr const char *low = "low";

/// Low - Extract low byte of 16-bit value (mixed case)
constexpr const char *Low = "Low";

} // namespace expression
} // namespace xasm
