/**
 * @file scmasm_expression_utils.cpp
 * @brief Implementation of SCMASM expression utilities
 */

#include "xasm++/syntax/scmasm_expression_utils.h"

#include <cctype>

namespace {

// Scans [A-Za-z_][A-Za-z0-9_.]*  and appends uppercased chars to result.
// Leaves i pointing at the last consumed character (caller's loop increments past it).
void ScanIdentifier(const std::string& expr, size_t& i, std::string& result) {
  while (i < expr.size() && (std::isalnum(expr[i]) || expr[i] == '_' || expr[i] == '.')) {
    result += std::toupper(static_cast<unsigned char>(expr[i]));
    ++i;
  }
  --i;
}

// Scans a hex literal starting at expr[i] (either '$' prefix or '0x'/'0X' prefix).
// Appends the normalised form ($XX or 0xXX with uppercase digits) to result.
// Leaves i pointing at the last consumed character.
void ScanHexNumber(const std::string& expr, size_t& i, std::string& result) {
  result += expr[i];  // '$' or '0'
  if (expr[i] == '0') {
    ++i;
    result += 'x';  // normalise to lowercase 0x
    ++i;
  } else {
    ++i;
  }
  while (i < expr.size() && std::isxdigit(expr[i])) {
    result += std::toupper(static_cast<unsigned char>(expr[i]));
    ++i;
  }
  --i;
}

// Scans a binary literal starting at expr[i] (either '%' prefix or '0b'/'0B' prefix).
// Appends the normalised form to result.
// Leaves i pointing at the last consumed character.
void ScanBinaryNumber(const std::string& expr, size_t& i, std::string& result) {
  result += expr[i];  // '%' or '0'
  if (expr[i] == '0') {
    ++i;
    result += 'b';  // normalise to lowercase 0b
    ++i;
  } else {
    ++i;
  }
  while (i < expr.size() && (expr[i] == '0' || expr[i] == '1')) {
    result += expr[i];
    ++i;
  }
  --i;
}

}  // namespace

namespace xasm::scmasm {

std::string NormalizeExpression(const std::string& expr) {
  std::string result;
  result.reserve(expr.size());

  for (size_t i = 0; i < expr.size(); ++i) {
    char c = expr[i];
    if (std::isalpha(c) || c == '_' || c == '.') {
      ScanIdentifier(expr, i, result);
    } else if (c == '$' ||
               (c == '0' && i + 1 < expr.size() && (expr[i + 1] == 'x' || expr[i + 1] == 'X'))) {
      ScanHexNumber(expr, i, result);
    } else if (c == '%' ||
               (c == '0' && i + 1 < expr.size() && (expr[i + 1] == 'b' || expr[i + 1] == 'B'))) {
      ScanBinaryNumber(expr, i, result);
    } else {
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
