// Parse utility functions
// Extracted from assembler.cpp for testability and reusability

#pragma once

#include <string>
#include <cstdint>

namespace xasm {

/**
 * Parse a hexadecimal string with $ prefix into a uint32_t value
 * 
 * This function provides safe parsing with:
 * - Input validation (checks for valid hex characters)
 * - Exception handling (catches std::stoul exceptions)
 * - Error reporting (returns success/failure with error message)
 * 
 * @param str Input string expected to start with '$' followed by hex digits
 * @param success Output parameter set to true if parsing succeeded, false otherwise
 * @param error_msg Output parameter containing error description if parsing failed
 * @return Parsed uint32_t value, or 0 if parsing failed
 * 
 * @note This function addresses [C1] Critical Security Issue from code review
 * @see .ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md
 */
uint32_t ParseHexSafe(const std::string& str, bool& success, std::string& error_msg);

/**
 * Trim whitespace from both ends of a string
 * 
 * @param str Input string to trim
 * @return Trimmed string with leading/trailing whitespace removed
 */
std::string Trim(const std::string& str);

} // namespace xasm
