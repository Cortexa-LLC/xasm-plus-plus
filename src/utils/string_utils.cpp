#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {
namespace util {

std::string Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

std::string ToUpper(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

// ParseHex moved to src/core/parse_utils.cpp for consolidation
// See: .ai/tasks/2026-01-31_refactor-number-parsing/

} // namespace util
} // namespace xasm
