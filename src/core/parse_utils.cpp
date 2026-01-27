// Parse utility implementations
// Implements [C1] fix: Safe ParseHex with validation and error handling

#include "xasm++/parse_utils.h"
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace xasm {

uint32_t ParseHexSafe(const std::string& str, bool& success, std::string& error_msg) {
    // Initialize outputs
    success = false;
    error_msg = "";
    
    // Validation 1: Empty string
    if (str.empty()) {
        error_msg = "ParseHex: Empty string";
        return 0;
    }
    
    // Validation 2: Must start with '$'
    if (str[0] != '$') {
        error_msg = "ParseHex: String must start with '$' (got: '" + str + "')";
        return 0;
    }
    
    // Validation 3: Must have at least one hex digit after '$'
    if (str.length() < 2) {
        error_msg = "ParseHex: No hex digits after '$'";
        return 0;
    }
    
    // Extract hex part (after '$')
    std::string hex_part = str.substr(1);
    
    // Validation 4: All characters must be valid hex digits (0-9, A-F, a-f)
    for (char c : hex_part) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            std::ostringstream oss;
            oss << "ParseHex: Invalid hex character '" << c << "' in '" << str << "'";
            error_msg = oss.str();
            return 0;
        }
    }
    
    // Validation 5: Check length doesn't exceed uint32_t (8 hex digits max)
    if (hex_part.length() > 8) {
        std::ostringstream oss;
        oss << "ParseHex: Value too large (max 8 hex digits, got " << hex_part.length() << ")";
        error_msg = oss.str();
        return 0;
    }
    
    // Parse the hex string with exception handling
    try {
        // std::stoul handles the conversion
        uint32_t value = static_cast<uint32_t>(std::stoul(hex_part, nullptr, 16));
        success = true;
        return value;
    } catch (const std::invalid_argument& e) {
        std::ostringstream oss;
        oss << "ParseHex: Invalid hex format in '" << str << "': " << e.what();
        error_msg = oss.str();
        return 0;
    } catch (const std::out_of_range& e) {
        std::ostringstream oss;
        oss << "ParseHex: Value out of range in '" << str << "': " << e.what();
        error_msg = oss.str();
        return 0;
    }
}

std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

} // namespace xasm
