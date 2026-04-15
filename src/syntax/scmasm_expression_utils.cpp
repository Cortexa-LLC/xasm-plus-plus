/**
 * @file scmasm_expression_utils.cpp
 * @brief Implementation of SCMASM expression utilities
 */

#include "xasm++/syntax/scmasm_expression_utils.h"

#include <cctype>

namespace xasm::scmasm {

std::string NormalizeExpression(const std::string& expr) {
  std::string result;
  result.reserve(expr.size());

  for (size_t i = 0; i < expr.size(); ++i) {
    char c = expr[i];

    // Identifiers: [A-Za-z_][A-Za-z0-9_.]* - uppercase them
    if (std::isalpha(c) || c == '_' || c == '.') {
      // Scan identifier
      while (i < expr.size() && (std::isalnum(expr[i]) || expr[i] == '_' || expr[i] == '.')) {
        result += std::toupper(static_cast<unsigned char>(expr[i]));
        ++i;
      }
      --i;  // Back up one since loop will increment
    }
    // Hex numbers: $XX or 0xXX - uppercase hex digits
    else if (c == '$' ||
             (c == '0' && i + 1 < expr.size() && (expr[i + 1] == 'x' || expr[i + 1] == 'X'))) {
      result += c;
      if (c == '0') {
        ++i;
        result += 'x';  // Normalize to lowercase 0x
        ++i;
      } else {
        ++i;
      }
      // Uppercase hex digits
      while (i < expr.size() && std::isxdigit(expr[i])) {
        result += std::toupper(static_cast<unsigned char>(expr[i]));
        ++i;
      }
      --i;  // Back up one
    }
    // Binary numbers: %BBBB or 0bBBBB - pass through
    else if (c == '%' ||
             (c == '0' && i + 1 < expr.size() && (expr[i + 1] == 'b' || expr[i + 1] == 'B'))) {
      result += c;
      if (c == '0') {
        ++i;
        result += 'b';  // Normalize to lowercase 0b
        ++i;
      } else {
        ++i;
      }
      while (i < expr.size() && (expr[i] == '0' || expr[i] == '1')) {
        result += expr[i];
        ++i;
      }
      --i;  // Back up one
    }
    // Everything else (operators, whitespace, digits) - pass through
    else {
      result += c;
    }
  }

  return result;
}

std::string CanonicalizeSlashHighByte(const std::string& expr) {
  // Replace '/' with '>' when '/' appears in a "prefix unary" position:
  // at the start of the expression, or immediately after an operator,
  // open-parenthesis, or comma (with optional whitespace).
  //
  // A '/' preceded by an operand character (letter, digit, '_', '.', ')',
  // or '$') is binary division and must NOT be replaced.
  std::string result;
  result.reserve(expr.size());

  // Track whether the next '/' we see is in a unary (prefix) position.
  // Initially true because we're at the start of the expression.
  bool prefix_position = true;

  for (size_t i = 0; i < expr.size(); ++i) {
    char c = expr[i];

    if (c == '/') {
      if (prefix_position) {
        result += '>';  // Replace SCMASM prefix '/' with canonical '>'
      } else {
        result += '/';  // Binary division — keep as-is
      }
      // After '/', the next operator is in a prefix position
      prefix_position = true;
    } else if (c == ' ' || c == '\t') {
      // Whitespace doesn't change positional state
      result += c;
    } else if (c == '+' || c == '-' || c == '*' || c == '%' || c == '&' || c == '|' || c == '^' ||
               c == '~' || c == '<' || c == '>' || c == '(' || c == ',') {
      // Operator or opening delimiter: next token is in prefix position
      result += c;
      prefix_position = true;
    } else {
      // Operand character (letter, digit, '$', ')', '#', etc.)
      result += c;
      prefix_position = false;
    }
  }

  return result;
}

std::string CanonicalizeEqualityOperator(const std::string& expr) {
  // Replace standalone '=' with '==' for the shared ExpressionParser.
  // Leave '==', '!=', '<=', '>=' untouched.
  std::string result;
  result.reserve(expr.size() + 4);  // Reserve a little extra for insertions

  for (size_t i = 0; i < expr.size(); ++i) {
    char c = expr[i];

    if (c == '=' &&
        (i == 0 ||
         (expr[i - 1] != '!' && expr[i - 1] != '<' && expr[i - 1] != '>' && expr[i - 1] != '=')) &&
        (i + 1 >= expr.size() || expr[i + 1] != '=')) {
      // Standalone '=' — emit as '=='
      result += "==";
    } else {
      result += c;
    }
  }

  return result;
}

}  // namespace xasm::scmasm
