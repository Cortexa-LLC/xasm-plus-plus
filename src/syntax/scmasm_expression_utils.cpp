/**
 * @file scmasm_expression_utils.cpp
 * @brief Implementation of SCMASM expression utilities
 */

#include "xasm++/syntax/scmasm_expression_utils.h"
#include <cctype>

namespace xasm {
namespace scmasm {

std::string NormalizeExpression(const std::string &expr) {
  std::string result;
  result.reserve(expr.size());

  for (size_t i = 0; i < expr.size(); ++i) {
    char c = expr[i];

    // Identifiers: [A-Za-z_][A-Za-z0-9_.]* - uppercase them
    if (std::isalpha(c) || c == '_' || c == '.') {
      // Scan identifier
      while (i < expr.size() &&
             (std::isalnum(expr[i]) || expr[i] == '_' || expr[i] == '.')) {
        result += std::toupper(static_cast<unsigned char>(expr[i]));
        ++i;
      }
      --i; // Back up one since loop will increment
    }
    // Hex numbers: $XX or 0xXX - uppercase hex digits
    else if (c == '$' || (c == '0' && i + 1 < expr.size() &&
                          (expr[i + 1] == 'x' || expr[i + 1] == 'X'))) {
      result += c;
      if (c == '0') {
        ++i;
        result += 'x'; // Normalize to lowercase 0x
        ++i;
      } else {
        ++i;
      }
      // Uppercase hex digits
      while (i < expr.size() && std::isxdigit(expr[i])) {
        result += std::toupper(static_cast<unsigned char>(expr[i]));
        ++i;
      }
      --i; // Back up one
    }
    // Binary numbers: %BBBB or 0bBBBB - pass through
    else if (c == '%' || (c == '0' && i + 1 < expr.size() &&
                          (expr[i + 1] == 'b' || expr[i + 1] == 'B'))) {
      result += c;
      if (c == '0') {
        ++i;
        result += 'b'; // Normalize to lowercase 0b
        ++i;
      } else {
        ++i;
      }
      while (i < expr.size() && (expr[i] == '0' || expr[i] == '1')) {
        result += expr[i];
        ++i;
      }
      --i; // Back up one
    }
    // Everything else (operators, whitespace, digits) - pass through
    else {
      result += c;
    }
  }

  return result;
}

} // namespace scmasm
} // namespace xasm
