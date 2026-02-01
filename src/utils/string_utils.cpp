#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {
namespace util {

std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

uint32_t ParseHex(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty string cannot be parsed as hex");
    }

    // Skip '$' prefix if present
    std::string hex_str = (str[0] == '$') ? str.substr(1) : str;

    if (hex_str.empty()) {
        throw std::invalid_argument("Hex string contains only '$' prefix");
    }

    // Strip addressing mode suffix (,X ,Y ,S) before parsing
    size_t comma_pos = hex_str.find(',');
    if (comma_pos != std::string::npos) {
        hex_str = hex_str.substr(0, comma_pos);
    }

    // Validate all characters are hex digits
    for (char c : hex_str) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            throw std::invalid_argument("Invalid hex digit '" + std::string(1, c) +
                                       "' in hex string: '" + str + "'");
        }
    }

    try {
        return std::stoul(hex_str, nullptr, 16);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse hex string '" + str + "': " + e.what());
    }
}

} // namespace util
} // namespace xasm
