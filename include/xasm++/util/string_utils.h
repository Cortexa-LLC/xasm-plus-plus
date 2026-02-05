#ifndef XASM_UTIL_STRING_UTILS_H
#define XASM_UTIL_STRING_UTILS_H

#include <cstdint>
#include <string>

namespace xasm {
namespace util {

/**
 * @brief Trim leading and trailing whitespace from a string
 * @param str The string to trim
 * @return Trimmed string with no leading/trailing spaces or tabs
 */
std::string Trim(const std::string &str);

/**
 * @brief Convert string to uppercase
 * @param str The string to convert
 * @return Uppercase version of the input string
 */
std::string ToUpper(const std::string &str);

// ParseHex moved to xasm++/parse_utils.h for consolidation
// See: .ai/tasks/2026-01-31_refactor-number-parsing/

} // namespace util
} // namespace xasm

#endif // XASM_UTIL_STRING_UTILS_H
