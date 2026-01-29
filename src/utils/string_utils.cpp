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
    
    return std::stoul(hex_str, nullptr, 16);
}

} // namespace util
} // namespace xasm
