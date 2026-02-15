// Parse utility implementations
// Implements [C1] fix: Safe ParseHex with validation and error handling

#include "xasm++/parse_utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

uint32_t ParseHex(const std::string &str) {
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
  } catch (const std::exception &e) {
    throw std::runtime_error("Failed to parse hex string '" + str +
                             "': " + e.what());
  }
}

uint32_t ParseHexSafe(const std::string &str, bool &success,
                      std::string &error_msg) {
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
    oss << "ParseHex: Value too large (max 8 hex digits, got "
        << hex_part.length() << ")";
    error_msg = oss.str();
    return 0;
  }

  // Parse the hex string with exception handling
  try {
    // std::stoul handles the conversion
    uint32_t value = static_cast<uint32_t>(std::stoul(hex_part, nullptr, 16));
    success = true;
    return value;
  } catch (const std::invalid_argument &e) {
    std::ostringstream oss;
    oss << "ParseHex: Invalid hex format in '" << str << "': " << e.what();
    error_msg = oss.str();
    return 0;
  } catch (const std::out_of_range &e) {
    std::ostringstream oss;
    oss << "ParseHex: Value out of range in '" << str << "': " << e.what();
    error_msg = oss.str();
    return 0;
  }
}

uint64_t ParseBinary(const std::string &str) {
  if (str.empty()) {
    throw std::invalid_argument("Empty string cannot be parsed as binary");
  }

  uint64_t value = 0;
  for (char c : str) {
    if (c != '0' && c != '1') {
      throw std::invalid_argument("Invalid binary digit '" + std::string(1, c) +
                                  "' in binary string: '" + str + "'");
    }
    value = value * 2 + (c - '0');
  }

  return value;
}

uint64_t ParseDecimal(const std::string &str) {
  if (str.empty()) {
    throw std::invalid_argument("Empty string cannot be parsed as decimal");
  }

  uint64_t value = 0;
  for (char c : str) {
    if (c < '0' || c > '9') {
      throw std::invalid_argument("Invalid decimal digit '" +
                                  std::string(1, c) +
                                  "' in decimal string: '" + str + "'");
    }
    value = value * 10 + (c - '0');
  }

  return value;
}

uint64_t ParseOctal(const std::string &str) {
  if (str.empty()) {
    throw std::invalid_argument("Empty string cannot be parsed as octal");
  }

  uint64_t value = 0;
  for (char c : str) {
    if (c < '0' || c > '7') {
      throw std::invalid_argument("Invalid octal digit '" + std::string(1, c) +
                                  "' in octal string: '" + str + "'");
    }
    value = value * 8 + (c - '0');
  }

  return value;
}

bool ParseBinaryDigit(char c, int &digit) {
  if (c == '0' || c == '1') {
    digit = c - '0';
    return true;
  }
  return false;
}

bool ParseDecimalDigit(char c, int &digit) {
  if (c >= '0' && c <= '9') {
    digit = c - '0';
    return true;
  }
  return false;
}

bool ParseOctalDigit(char c, int &digit) {
  if (c >= '0' && c <= '7') {
    digit = c - '0';
    return true;
  }
  return false;
}

bool ParseHexDigit(char c, int &digit) {
  if (c >= '0' && c <= '9') {
    digit = c - '0';
    return true;
  } else if (c >= 'a' && c <= 'f') {
    digit = c - 'a' + 10;
    return true;
  } else if (c >= 'A' && c <= 'F') {
    digit = c - 'A' + 10;
    return true;
  }
  return false;
}

// Note: Trim() removed from parse_utils.cpp - available from string_utils.h
// (xasm::util::Trim) This avoids conflicts with static Trim implementations in
// other modules

} // namespace xasm
