/**
 * @file expression_parser.cpp
 * @brief Implementation of shared expression parser
 */

#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/directive_constants.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {

ExpressionParser::ExpressionParser(const SymbolTable *symbols,
                                   const INumberParser *number_parser)
    : symbols_(symbols), number_parser_(number_parser), pos_(0) {}

std::shared_ptr<Expression> ExpressionParser::Parse(const std::string &str) {
  expr_ = str;
  pos_ = 0;
  SkipWhitespace();

  // Empty expression returns 0
  if (pos_ >= expr_.length()) {
    return std::make_shared<LiteralExpr>(0);
  }

  auto result = ParseLogicalOr();

  // Check for unexpected trailing characters
  SkipWhitespace();
  if (pos_ < expr_.length()) {
    throw std::runtime_error("Unexpected character after expression: " +
                             std::string(1, expr_[pos_]));
  }

  return result;
}

// ============================================================================
// Recursive descent parsing (precedence order: low to high)
// ============================================================================

std::shared_ptr<Expression> ExpressionParser::ParseLogicalOr() {
  auto left = ParseLogicalAnd();

  while (true) {
    SkipWhitespace();
    if (Match("||")) {
      auto right = ParseLogicalAnd();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LogicalOr, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseLogicalAnd() {
  auto left = ParseComparison();

  while (true) {
    SkipWhitespace();
    if (Match("&&")) {
      auto right = ParseComparison();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LogicalAnd, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseComparison() {
  auto left = ParseBitwiseOr();

  while (true) {
    SkipWhitespace();
    // Check for two-character operators first
    if (Match("==")) {
      auto right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Equal, left, right);
    } else if (Match("!=")) {
      auto right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::NotEqual, left, right);
    } else if (Match("<=")) {
      auto right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LessOrEqual, left, right);
    } else if (Match(">=")) {
      auto right = ParseBitwiseOr();
      left =
          std::make_shared<BinaryOpExpr>(BinaryOp::GreaterOrEqual, left, right);
    } else if (Peek() == '<' && pos_ + 1 < expr_.length() &&
               expr_[pos_ + 1] != '<') {
      // Single '<' (not '<<' shift operator)
      Consume();
      auto right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LessThan, left, right);
    } else if (Peek() == '>' && pos_ + 1 < expr_.length() &&
               expr_[pos_ + 1] != '>') {
      // Single '>' (not '>>' shift operator)
      Consume();
      auto right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::GreaterThan, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseOr() {
  auto left = ParseBitwiseXor();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    // Check for single '|' (not '||')
    if (c == '|' && pos_ + 1 < expr_.length() && expr_[pos_ + 1] != '|') {
      Consume();
      auto right = ParseBitwiseXor();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseOr, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseXor() {
  auto left = ParseBitwiseAnd();

  while (true) {
    SkipWhitespace();
    if (Peek() == '^') {
      Consume();
      auto right = ParseBitwiseAnd();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseXor, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseAnd() {
  auto left = ParseShift();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    // Check for single '&' (not '&&')
    if (c == '&' && pos_ + 1 < expr_.length() && expr_[pos_ + 1] != '&') {
      Consume();
      auto right = ParseShift();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseAnd, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseShift() {
  auto left = ParseAddSub();

  while (true) {
    SkipWhitespace();
    if (Match("<<")) {
      auto right = ParseAddSub();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::ShiftLeft, left, right);
    } else if (Match(">>")) {
      auto right = ParseAddSub();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::ShiftRight, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseAddSub() {
  auto left = ParseMulDiv();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    if (c == '+') {
      Consume();
      auto right = ParseMulDiv();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Add, left, right);
    } else if (c == '-') {
      // Consume '-' as binary subtraction operator
      // ParseMulDiv -> ParseUnary will handle negative literals correctly
      Consume();
      auto right = ParseMulDiv();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Subtract, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseMulDiv() {
  auto left = ParseUnary();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    if (c == '*') {
      Consume();
      auto right = ParseUnary();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Multiply, left, right);
    } else if (c == '/') {
      Consume();
      auto right = ParseUnary();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Divide, left, right);
    } else if (c == '%') {
      Consume();
      auto right = ParseUnary();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Modulo, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseUnary() {
  SkipWhitespace();
  char c = Peek();

  // Unary minus
  if (c == '-') {
    Consume();
    auto operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::Negate, operand);
  }

  // Bitwise NOT
  if (c == '~') {
    Consume();
    auto operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::BitwiseNot, operand);
  }

  // Logical NOT
  if (c == '!') {
    Consume();
    auto operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::LogicalNot, operand);
  }

  // Low byte operator (<)
  // Note: This is prefix unary, distinct from infix comparison <
  if (c == '<') {
    Consume();
    auto operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::LowByte, operand);
  }

  // High byte operator (>)
  // Note: This is prefix unary, distinct from infix comparison >
  if (c == '>') {
    Consume();
    auto operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::HighByte, operand);
  }

  // Note: HIGH() and LOW() are also handled as function calls in ParsePrimary
  // (e.g., "HIGH(0x1234)" for Z80 syntax)

  return ParsePrimary();
}

std::shared_ptr<Expression> ExpressionParser::ParsePrimary() {
  SkipWhitespace();

  // Parenthesized expression
  if (Peek() == '(') {
    Consume();
    auto expr = ParseLogicalOr();
    SkipWhitespace();
    if (Peek() != ')') {
      throw std::runtime_error("Expected closing parenthesis");
    }
    Consume();
    return expr;
  }

  // Bracketed expression (Z80 alternative to parentheses)
  if (Peek() == '[') {
    Consume();
    auto expr = ParseLogicalOr();
    SkipWhitespace();
    if (Peek() != ']') {
      throw std::runtime_error("Expected closing bracket");
    }
    Consume();
    return expr;
  }

  // Try custom number parser first for syntax-specific formats
  // This handles cases like "0FFH", "$ABCD", "%1010.0101", "'A", "\"A", "/A",
  // etc.
  if (number_parser_) {
    char first_char = Peek();
    size_t saved_pos = pos_;

    // Extract potential number/character constant token
    std::string token;
    bool is_binary = false;

    // Hexadecimal or binary prefix
    if (first_char == '$' || first_char == '%') {
      token += Consume();
      is_binary = (first_char == '%');

      // Collect digits and allowed separators
      while (pos_ < expr_.length()) {
        char c = Peek();

        if (std::isalnum(c) || c == '_') {
          token += Consume();
        } else if (c == '.' && is_binary) {
          // Allow . separator in binary numbers only
          token += Consume();
        } else {
          break;
        }
      }
    }
    // Decimal number or suffix format
    else if (std::isdigit(first_char)) {
      while (pos_ < expr_.length()) {
        char c = Peek();
        if (std::isalnum(c) || c == '_') {
          token += Consume();
        } else {
          break;
        }
      }
    }
    // Potential ASCII character constant (non-operator, non-paren,
    // non-identifier start) Try if it's not an operator that should be handled
    // elsewhere
    else if (!std::isalnum(first_char) && first_char != '_' &&
             first_char != '(') {
      // Try treating as 2-char ASCII constant (delimiter + char)
      token += Consume(); // delimiter
      if (pos_ < expr_.length() && pos_ + 1 <= expr_.length()) {
        // Check if next char could be part of character constant
        char next = Peek();
        if (std::isprint(next)) {
          token += Consume(); // character
        }
      }
    }

    // Try parsing with custom parser if we extracted a token
    if (!token.empty()) {
      int64_t value;
      if (number_parser_->TryParse(token, value)) {
        return std::make_shared<LiteralExpr>(value);
      }

      // Not a custom format, restore position and try standard formats
      pos_ = saved_pos;

      // Check if $ was followed by non-hex-digit - if so, it's current location
      if (token == "$") {
        Consume(); // consume the $
        return std::make_shared<CurrentLocationExpr>();
      }
    }
  }

  // Check if it's $ without a hex digit (current location operator)
  if (Peek() == '$') {
    size_t saved_pos = pos_;
    Consume(); // consume $
    if (!std::isxdigit(Peek())) {
      // $ not followed by hex digit - current location operator
      return std::make_shared<CurrentLocationExpr>();
    }
    // $ followed by hex digit - restore and parse as number
    pos_ = saved_pos;
  }

  // Check if it's a number or identifier
  if (std::isdigit(Peek()) || Peek() == '$' || Peek() == '%') {
    // Number literal
    int64_t value = ParseNumber();
    return std::make_shared<LiteralExpr>(value);
  }

  // Check for hex with 0x prefix
  if (Peek() == '0' && pos_ + 1 < expr_.length() &&
      (expr_[pos_ + 1] == 'x' || expr_[pos_ + 1] == 'X' ||
       expr_[pos_ + 1] == 'b' || expr_[pos_ + 1] == 'B')) {
    int64_t value = ParseNumber();
    return std::make_shared<LiteralExpr>(value);
  }

  // Identifier (symbol or function)
  if (std::isalpha(Peek()) || Peek() == '_' || Peek() == '.' || Peek() == '$' ||
      Peek() == '?') {
    std::string ident = ParseIdentifier();

    // Try parsing as number first (for RADIX mode where "FF" is a hex number)
    if (number_parser_) {
      int64_t value;
      if (number_parser_->TryParse(ident, value)) {
        return std::make_shared<LiteralExpr>(value);
      }
    }

    // Check for function call
    SkipWhitespace();
    if (Peek() == '(') {
      Consume();
      auto arg = ParseLogicalOr();
      SkipWhitespace();
      if (Peek() != ')') {
        throw std::runtime_error(
            "Expected closing parenthesis in function call");
      }
      Consume();

      // Handle LOW and HIGH functions
      std::string ident_upper = ident;
      std::transform(ident_upper.begin(), ident_upper.end(),
                     ident_upper.begin(), ::toupper);

      if (ident_upper == directives::LOW_FUNC) {
        return std::make_shared<UnaryOpExpr>(UnaryOp::LowByte, arg);
      } else if (ident_upper == directives::HIGH_FUNC) {
        return std::make_shared<UnaryOpExpr>(UnaryOp::HighByte, arg);
      } else {
        throw std::runtime_error("Unknown function: " + ident);
      }
    }

    // Symbol reference
    // Note: symbols_ is used for validation, but SymbolExpr will look up at
    // evaluation time
    return std::make_shared<SymbolExpr>(ident);
  }

  // Unexpected character
  if (pos_ < expr_.length()) {
    throw std::runtime_error("Unexpected character: " + std::string(1, Peek()));
  }

  // Silence unused warning (symbols_ is passed for future use)
  (void)symbols_;

  throw std::runtime_error("Expected expression");
}

// ============================================================================
// Helper methods
// ============================================================================

void ExpressionParser::SkipWhitespace() {
  while (pos_ < expr_.length() && std::isspace(expr_[pos_])) {
    pos_++;
  }
}

char ExpressionParser::Peek() const {
  if (pos_ < expr_.length()) {
    return expr_[pos_];
  }
  return '\0';
}

char ExpressionParser::Consume() {
  if (pos_ < expr_.length()) {
    return expr_[pos_++];
  }
  return '\0';
}

bool ExpressionParser::Match(const std::string &str) {
  if (pos_ + str.length() > expr_.length()) {
    return false;
  }

  if (expr_.substr(pos_, str.length()) == str) {
    pos_ += str.length();
    return true;
  }

  return false;
}

int64_t ExpressionParser::ParseNumber() {
  SkipWhitespace();

  // Hexadecimal with $ prefix
  if (Peek() == '$') {
    Consume();
    int64_t value = 0;
    if (!std::isxdigit(Peek())) {
      throw std::runtime_error("Expected hexadecimal digit after $");
    }
    while (std::isxdigit(Peek())) {
      char c = Consume();
      int digit;
      if (c >= '0' && c <= '9') {
        digit = c - '0';
      } else if (c >= 'A' && c <= 'F') {
        digit = c - 'A' + 10;
      } else if (c >= 'a' && c <= 'f') {
        digit = c - 'a' + 10;
      } else {
        break;
      }
      value = value * 16 + digit;
    }
    return value;
  }

  // Binary with % prefix
  if (Peek() == '%') {
    Consume();
    int64_t value = 0;
    if (Peek() != '0' && Peek() != '1') {
      throw std::runtime_error("Invalid binary number: expected 0 or 1 after %");
    }
    while (Peek() == '0' || Peek() == '1') {
      value = value * 2 + (Consume() - '0');
    }
    return value;
  }

  // Hexadecimal with 0x prefix
  if (Peek() == '0' && pos_ + 1 < expr_.length() &&
      (expr_[pos_ + 1] == 'x' || expr_[pos_ + 1] == 'X')) {
    Consume(); // '0'
    Consume(); // 'x' or 'X'
    int64_t value = 0;
    if (!std::isxdigit(Peek())) {
      throw std::runtime_error("Expected hexadecimal digit after 0x");
    }
    while (std::isxdigit(Peek())) {
      char c = Consume();
      int digit;
      if (c >= '0' && c <= '9') {
        digit = c - '0';
      } else if (c >= 'A' && c <= 'F') {
        digit = c - 'A' + 10;
      } else if (c >= 'a' && c <= 'f') {
        digit = c - 'a' + 10;
      } else {
        break;
      }
      value = value * 16 + digit;
    }
    return value;
  }

  // Binary with 0b prefix
  if (Peek() == '0' && pos_ + 1 < expr_.length() &&
      (expr_[pos_ + 1] == 'b' || expr_[pos_ + 1] == 'B')) {
    Consume(); // '0'
    Consume(); // 'b' or 'B'
    int64_t value = 0;
    if (Peek() != '0' && Peek() != '1') {
      throw std::runtime_error("Invalid binary number: expected 0 or 1 after 0b");
    }
    while (Peek() == '0' || Peek() == '1') {
      value = value * 2 + (Consume() - '0');
    }
    return value;
  }

  // Decimal
  if (!std::isdigit(Peek())) {
    throw std::runtime_error("Expected number");
  }
  int64_t value = 0;
  while (std::isdigit(Peek())) {
    value = value * 10 + (Consume() - '0');
  }
  return value;
}

std::string ExpressionParser::ParseIdentifier() {
  SkipWhitespace();
  size_t start = pos_;

  // Identifier starts with letter, underscore, period, $, or ?
  if (!std::isalpha(Peek()) && Peek() != '_' && Peek() != '.' &&
      Peek() != '$' && Peek() != '?') {
    throw std::runtime_error("Expected identifier");
  }

  Consume();

  // Continue with alphanumeric, underscore, $, ?
  while (std::isalnum(Peek()) || Peek() == '_' || Peek() == '$' ||
         Peek() == '?') {
    Consume();
  }

  return expr_.substr(start, pos_ - start);
}

} // namespace xasm
