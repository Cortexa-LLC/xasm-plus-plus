// Parse utility functions
// Extracted from assembler.cpp for testability and reusability

#pragma once

#include <cstdint>
#include <string>

namespace xasm {

/**
 * Parse a hexadecimal string into a uint32_t value (exception-throwing variant)
 *
 * This function provides flexible hex parsing with:
 * - Optional '$' prefix (strips if present)
 * - Addressing mode suffix stripping (,X ,Y ,S)
 * - Exception-based error reporting
 *
 * @param str Hex string (e.g., "$1234", "1234", "$10,X")
 * @return Parsed uint32_t value
 * @throws std::invalid_argument if string is not valid hex
 *
 * @note Consolidates duplicate implementations from string_utils and
 * simple_syntax
 */
uint32_t ParseHex(const std::string &str);

/**
 * Parse a hexadecimal string with $ prefix into a uint32_t value (safe variant)
 *
 * This function provides safe parsing with:
 * - Input validation (checks for valid hex characters)
 * - Exception handling (catches std::stoul exceptions)
 * - Error reporting (returns success/failure with error message)
 *
 * @param str Input string expected to start with '$' followed by hex digits
 * @param success Output parameter set to true if parsing succeeded, false
 * otherwise
 * @param error_msg Output parameter containing error description if parsing
 * failed
 * @return Parsed uint32_t value, or 0 if parsing failed
 *
 * @note This function addresses [C1] Critical Security Issue from code review
 * @see .ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md
 */
uint32_t ParseHexSafe(const std::string &str, bool &success,
                      std::string &error_msg);

// Note: Trim() is still available from xasm::util namespace (string_utils.h)
// Not consolidated here to avoid conflicts with static Trim in
// simple_syntax.cpp

} // namespace xasm
