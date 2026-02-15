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

/**
 * Parse a binary string (e.g., "1010") into a uint64_t value
 *
 * @param str Binary string containing only '0' and '1' characters
 * @return Parsed uint64_t value
 * @throws std::invalid_argument if string is empty or contains non-binary chars
 */
uint64_t ParseBinary(const std::string &str);

/**
 * Parse a decimal string (e.g., "123") into a uint64_t value
 *
 * @param str Decimal string containing only '0'-'9' characters
 * @return Parsed uint64_t value
 * @throws std::invalid_argument if string is empty or contains non-decimal
 * chars
 */
uint64_t ParseDecimal(const std::string &str);

/**
 * Parse an octal string (e.g., "377") into a uint64_t value
 *
 * @param str Octal string containing only '0'-'7' characters
 * @return Parsed uint64_t value
 * @throws std::invalid_argument if string is empty or contains non-octal chars
 */
uint64_t ParseOctal(const std::string &str);

/**
 * Parse a single binary digit character
 *
 * @param c Character to parse (0-1)
 * @param digit Output parameter receiving digit value (0-1)
 * @return true if character is valid binary digit, false otherwise
 */
bool ParseBinaryDigit(char c, int &digit);

/**
 * Parse a single decimal digit character
 *
 * @param c Character to parse (0-9)
 * @param digit Output parameter receiving digit value (0-9)
 * @return true if character is valid decimal digit, false otherwise
 */
bool ParseDecimalDigit(char c, int &digit);

/**
 * Parse a single octal digit character
 *
 * @param c Character to parse (0-7)
 * @param digit Output parameter receiving digit value (0-7)
 * @return true if character is valid octal digit, false otherwise
 */
bool ParseOctalDigit(char c, int &digit);

/**
 * Parse a single hexadecimal digit character
 *
 * @param c Character to parse (0-9, a-f, A-F)
 * @param digit Output parameter receiving digit value (0-15)
 * @return true if character is valid hex digit, false otherwise
 */
bool ParseHexDigit(char c, int &digit);

// Note: Trim() is still available from xasm::util namespace (string_utils.h)
// Not consolidated here to avoid conflicts with static Trim in
// simple_syntax.cpp

} // namespace xasm
