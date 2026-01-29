#ifndef XASM_UTIL_STRING_UTILS_H
#define XASM_UTIL_STRING_UTILS_H

#include <string>
#include <cstdint>

namespace xasm {
namespace util {

/**
 * @brief Trim leading and trailing whitespace from a string
 * @param str The string to trim
 * @return Trimmed string with no leading/trailing spaces or tabs
 */
std::string Trim(const std::string& str);

/**
 * @brief Convert string to uppercase
 * @param str The string to convert
 * @return Uppercase version of the input string
 */
std::string ToUpper(const std::string& str);

/**
 * @brief Parse hexadecimal string (with or without '$' prefix)
 * @param str The hexadecimal string to parse (e.g., "$FF" or "FF")
 * @return Parsed unsigned 32-bit integer value
 * @throws std::invalid_argument if string is not valid hex
 */
uint32_t ParseHex(const std::string& str);

} // namespace util
} // namespace xasm

#endif // XASM_UTIL_STRING_UTILS_H
