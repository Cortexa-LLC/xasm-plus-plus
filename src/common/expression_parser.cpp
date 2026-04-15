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
                                   const INumberParser *number_parser,
                                   ParserFeatures features)
    : symbols_(symbols), number_parser_(number_parser), features_(features) {
  InitPrefixTable();
}

std::shared_ptr<Expression> ExpressionParser::Parse(const std::string &str) {
  expr_ = str;
  pos_ = 0;
  SkipWhitespace();

  // Empty expression returns 0
  if (pos_ >= expr_.length()) {
    return std::make_shared<LiteralExpr>(0);
  }

  std::shared_ptr<Expression> result = ParseLogicalOr();

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
  std::shared_ptr<Expression> left = ParseLogicalAnd();

  while (true) {
    SkipWhitespace();
    if (Match("||")) {
      std::shared_ptr<Expression> right = ParseLogicalAnd();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LogicalOr, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseLogicalAnd() {
  std::shared_ptr<Expression> left = ParseComparison();

  while (true) {
    SkipWhitespace();
    if (Match("&&")) {
      std::shared_ptr<Expression> right = ParseComparison();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LogicalAnd, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseComparison() {
  std::shared_ptr<Expression> left = ParseBitwiseOr();

  while (true) {
    SkipWhitespace();
    // Check for two-character operators first
    if (Match("==")) {
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Equal, left, right);
    } else if (Match("!=")) {
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::NotEqual, left, right);
    } else if (Match("<=")) {
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LessOrEqual, left, right);
    } else if (Match(">=")) {
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left =
          std::make_shared<BinaryOpExpr>(BinaryOp::GreaterOrEqual, left, right);
    } else if (Peek() == '<' && pos_ + 1 < expr_.length() &&
               expr_[pos_ + 1] != '<') {
      // Single '<' (not '<<' shift operator)
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::LessThan, left, right);
    } else if (Peek() == '>' && pos_ + 1 < expr_.length() &&
               expr_[pos_ + 1] != '>') {
      // Single '>' (not '>>' shift operator)
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseOr();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::GreaterThan, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseOr() {
  std::shared_ptr<Expression> left = ParseBitwiseXor();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    // Check for single '|' (not '||')
    if (c == '|' && pos_ + 1 < expr_.length() && expr_[pos_ + 1] != '|') {
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseXor();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseOr, left, right);
    } else if (features_.allow_merlin_bitwise_ops && c == '.') {
      // ADR-005 V9: Merlin `.` is binary bitwise OR operator (e.g. SYM.$80)
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseXor();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseOr, left, right);
    } else if (features_.allow_merlin_bitwise_ops && Peek() == '!') {
      // ADR-005 V9: Merlin `!` is binary bitwise OR operator (e.g. SYM!1)
      // Empirically verified against vasm reference: FinalDisk!1 = 1 (OR, not XOR).
      // Unary `!` (logical NOT) only appears at expression start.
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseXor();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseOr, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseXor() {
  std::shared_ptr<Expression> left = ParseBitwiseAnd();

  while (true) {
    SkipWhitespace();
    if (Peek() == '^') {
      Consume();
      std::shared_ptr<Expression> right = ParseBitwiseAnd();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseXor, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseBitwiseAnd() {
  std::shared_ptr<Expression> left = ParseShift();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    // Check for single '&' (not '&&')
    if (c == '&' && pos_ + 1 < expr_.length() && expr_[pos_ + 1] != '&') {
      Consume();
      std::shared_ptr<Expression> right = ParseShift();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::BitwiseAnd, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseShift() {
  std::shared_ptr<Expression> left = ParseAddSub();

  while (true) {
    SkipWhitespace();
    if (Match("<<")) {
      std::shared_ptr<Expression> right = ParseAddSub();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::ShiftLeft, left, right);
    } else if (Match(">>")) {
      std::shared_ptr<Expression> right = ParseAddSub();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::ShiftRight, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseAddSub() {
  std::shared_ptr<Expression> left = ParseMulDiv();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    if (c == '+') {
      Consume();
      std::shared_ptr<Expression> right = ParseMulDiv();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Add, left, right);
    } else if (c == '-') {
      // Consume '-' as binary subtraction operator
      // ParseMulDiv -> ParseUnary will handle negative literals correctly
      Consume();
      std::shared_ptr<Expression> right = ParseMulDiv();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Subtract, left, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression> ExpressionParser::ParseMulDiv() {
  std::shared_ptr<Expression> left = ParseUnary();

  while (true) {
    SkipWhitespace();
    char c = Peek();
    if (c == '*') {
      Consume();
      std::shared_ptr<Expression> right = ParseUnary();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Multiply, left, right);
    } else if (c == '/') {
      Consume();
      std::shared_ptr<Expression> right = ParseUnary();
      left = std::make_shared<BinaryOpExpr>(BinaryOp::Divide, left, right);
    } else if (c == '%') {
      Consume();
      std::shared_ptr<Expression> right = ParseUnary();
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
    std::shared_ptr<Expression> operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::Negate, operand);
  }

  // Bitwise NOT
  if (c == '~') {
    Consume();
    std::shared_ptr<Expression> operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::BitwiseNot, operand);
  }

  // Logical NOT
  if (c == '!') {
    Consume();
    std::shared_ptr<Expression> operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::LogicalNot, operand);
  }

  // Low byte operator (< or #)
  // Note: '<' is prefix unary, distinct from infix comparison <
  // '#' is used by Merlin / assembler data directives (DB/DFB) to mean
  // "low byte of address", identical semantics to '<'.
  if (c == '<' || c == '#') {
    Consume();
    // In Merlin, `<base+offset` means `<(base+offset)` — the operator
    // applies to the entire following additive expression.
    std::shared_ptr<Expression> operand = features_.merlin_byte_ops_greedy ? ParseAddSub() : ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::LowByte, operand);
  }

  // High byte operator (>)
  // Note: This is prefix unary, distinct from infix comparison >
  if (c == '>') {
    Consume();
    // In Merlin, `>base+offset` means `>(base+offset)` — the operator
    // applies to the entire following additive expression.
    std::shared_ptr<Expression> operand = features_.merlin_byte_ops_greedy ? ParseAddSub() : ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::HighByte, operand);
  }

  // Bank byte operator (^) — extracts bits 16-23 of a 24-bit address.
  // Used in SCMASM/Merlin 65816 code: `lda ^symbol` loads the bank byte.
  // Note: `^` as binary XOR is handled in ParseBitwiseXor; here it is only
  // reached when `^` appears in a unary/primary context.
  if (c == '^') {
    Consume();
    std::shared_ptr<Expression> operand = ParseUnary();
    return std::make_shared<UnaryOpExpr>(UnaryOp::BankByte, operand);
  }

  // Note: HIGH() and LOW() are also handled as function calls in ParsePrimary
  // (e.g., "HIGH(0x1234)" for Z80 syntax)

  return ParsePrimary();
}

// ============================================================================
// TryParse helpers for ParsePrimary
// ============================================================================

std::shared_ptr<Expression> ExpressionParser::TryParseParenthesized() {
  if (Peek() != '(') {
    return nullptr;
  }
  Consume();
  std::shared_ptr<Expression> expr = ParseLogicalOr();
  SkipWhitespace();
  if (Peek() != ')') {
    throw std::runtime_error("Expected closing parenthesis");
  }
  Consume();
  return expr;
}

std::shared_ptr<Expression> ExpressionParser::TryParseBracketed() {
  // ADR-005 V7: gated behind ParserFeatures.allow_bracket_grouping
  if (!features_.allow_bracket_grouping || Peek() != '[') {
    return nullptr;
  }
  Consume();
  std::shared_ptr<Expression> expr = ParseLogicalOr();
  SkipWhitespace();
  if (Peek() != ']') {
    throw std::runtime_error("Expected closing bracket");
  }
  Consume();
  return expr;
}

std::shared_ptr<Expression> ExpressionParser::TryParseCustomNumber() {
  if (!number_parser_) {
    return nullptr;
  }
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
      if (std::isalnum(c) || c == '_' || (c == '.' && is_binary)) {
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
    int64_t value = 0;
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

  return nullptr;
}

std::shared_ptr<Expression> ExpressionParser::TryParseDollarCurrentLocation() {
  if (Peek() != '$') {
    return nullptr;
  }
  size_t saved_pos = pos_;
  Consume(); // consume $
  if (!std::isxdigit(Peek())) {
    // $ not followed by hex digit - current location operator
    return std::make_shared<CurrentLocationExpr>();
  }
  // $ followed by hex digit - restore and parse as number
  pos_ = saved_pos;
  return nullptr;
}

std::shared_ptr<Expression> ExpressionParser::TryParseIdentifierOrCall() {
  // Identifier (symbol or function)
  // ADR-005 V8: ']' prefix for Merlin ]variable labels is gated behind
  // ParserFeatures.allow_merlin_var_prefix.
  // ADR-005 V9: In Merlin mode, '.' is a binary OR operator, not an identifier
  // start character.  Suppress it here so '.' after a primary is handled by
  // ParseBitwiseOr() rather than greedily consumed into an identifier.
  bool allow_dot_ident_start = !features_.allow_merlin_bitwise_ops;
  if (!std::isalpha(Peek()) && Peek() != '_' &&
      !(allow_dot_ident_start && Peek() == '.') && Peek() != '$' &&
      Peek() != '?' &&
      !(features_.allow_merlin_var_prefix && Peek() == ']')) {
    return nullptr;
  }

  std::string ident = ParseIdentifier();

  // Try parsing as number first (for RADIX mode where "FF" is a hex number)
  if (number_parser_) {
    int64_t value = 0;
    if (number_parser_->TryParse(ident, value)) {
      return std::make_shared<LiteralExpr>(value);
    }
  }

  // Check for function call
  SkipWhitespace();
  if (Peek() == '(') {
    Consume();
    std::shared_ptr<Expression> arg = ParseLogicalOr();
    SkipWhitespace();
    if (Peek() != ')') {
      throw std::runtime_error("Expected closing parenthesis in function call");
    }
    Consume();

    // Handle LOW and HIGH functions
    std::string ident_upper = ident;
    std::transform(ident_upper.begin(), ident_upper.end(), ident_upper.begin(),
                   ::toupper);

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

// ============================================================================
// Prefix dispatch table — InitPrefixTable and per-radix helpers
// ============================================================================

void ExpressionParser::InitPrefixTable() {
  // Parenthesized expression
  prefix_table_['('] = [this]() { return TryParseParenthesized(); };

  // Bracketed expression (Z80/EDTASM)
  prefix_table_['['] = [this]() { return TryParseBracketed(); };

  // '*' — current-location counter
  prefix_table_['*'] = [this]() -> std::shared_ptr<Expression> {
    Consume();
    return std::make_shared<CurrentLocationExpr>();
  };

  // '$' — current-location OR hex literal (TryParseDollarCurrentLocation
  // returns nullptr when followed by a hex digit, so fall through to hex)
  prefix_table_['$'] = [this]() -> std::shared_ptr<Expression> {
    if (auto e = TryParseDollarCurrentLocation()) {
      return e;
    }
    return std::make_shared<LiteralExpr>(ParseDollarHex());
  };

  // '%' — binary literal
  prefix_table_['%'] = [this]() -> std::shared_ptr<Expression> {
    Consume(); // consume '%'
    return std::make_shared<LiteralExpr>(ParsePercentBinary());
  };

  // '@' — octal literal (some syntaxes)
  prefix_table_['@'] = [this]() -> std::shared_ptr<Expression> {
    Consume(); // consume '@'
    return std::make_shared<LiteralExpr>(ParseOctalDigits());
  };

  // Digits — decimal, 0x-hex, or 0b-binary
  auto digit_fn = [this]() -> std::shared_ptr<Expression> {
    if (Peek() == '0' && pos_ + 1 < expr_.length()) {
      char next = expr_[pos_ + 1];
      if (next == 'x' || next == 'X') {
        Consume(); // '0'
        Consume(); // 'x'
        return std::make_shared<LiteralExpr>(Parse0xHex());
      }
      if (next == 'b' || next == 'B') {
        Consume(); // '0'
        Consume(); // 'b'
        return std::make_shared<LiteralExpr>(Parse0bBinary());
      }
    }
    return std::make_shared<LiteralExpr>(ParseDecimalDigits());
  };
  for (char d = '0'; d <= '9'; ++d) {
    prefix_table_[d] = digit_fn;
  }
}

// --- Per-radix number helpers -----------------------------------------------

int64_t ExpressionParser::ParseDollarHex() {
  // '$' is still in stream — consume it
  Consume();
  if (!std::isxdigit(Peek())) {
    throw std::runtime_error("Expected hexadecimal digit after $");
  }
  int64_t value = 0;
  while (std::isxdigit(Peek())) {
    char c = Consume();
    int digit = (c >= '0' && c <= '9') ? (c - '0')
                : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                          : (c - 'a' + 10);
    value = (value * 16) + digit;
  }
  return value;
}

int64_t ExpressionParser::ParsePercentBinary() {
  // '%' already consumed by caller
  if (Peek() != '0' && Peek() != '1') {
    throw std::runtime_error("Invalid binary number: expected 0 or 1 after %");
  }
  int64_t value = 0;
  while (Peek() == '0' || Peek() == '1' || Peek() == '.') {
    char ch = Consume();
    if (ch != '.') {
      value = (value * 2) + (ch - '0');
    }
  }
  if (std::isdigit(Peek())) {
    throw std::runtime_error("Invalid binary digit '" + std::string(1, Peek()) +
                             "' in binary literal");
  }
  return value;
}

int64_t ExpressionParser::Parse0xHex() {
  // '0x' already consumed by caller
  if (!std::isxdigit(Peek())) {
    throw std::runtime_error("Expected hexadecimal digit after 0x");
  }
  int64_t value = 0;
  while (std::isxdigit(Peek())) {
    char c = Consume();
    int digit = (c >= '0' && c <= '9') ? (c - '0')
                : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                          : (c - 'a' + 10);
    value = (value * 16) + digit;
  }
  return value;
}

int64_t ExpressionParser::Parse0bBinary() {
  // '0b' already consumed by caller
  if (Peek() != '0' && Peek() != '1') {
    throw std::runtime_error(
        "Invalid binary number: expected 0 or 1 after 0b");
  }
  int64_t value = 0;
  while (Peek() == '0' || Peek() == '1') {
    value = (value * 2) + (Consume() - '0');
  }
  return value;
}

int64_t ExpressionParser::ParseOctalDigits() {
  // '@' already consumed by caller
  if (Peek() < '0' || Peek() > '7') {
    throw std::runtime_error("Expected octal digit after @");
  }
  int64_t value = 0;
  while (Peek() >= '0' && Peek() <= '7') {
    value = (value * 8) + (Consume() - '0');
  }
  return value;
}

int64_t ExpressionParser::ParseDecimalDigits() {
  // First char must be a digit (not yet consumed)
  if (!std::isdigit(Peek())) {
    throw std::runtime_error("Expected decimal digit");
  }
  int64_t value = 0;
  while (std::isdigit(Peek())) {
    value = (value * 10) + (Consume() - '0');
  }
  return value;
}

// ============================================================================

std::shared_ptr<Expression> ExpressionParser::ParsePrimary() {
  SkipWhitespace();

  // Try custom number parser first for syntax-specific formats
  // (handles "0FFH", "'A", "\"A", "/A", etc.)
  if (auto expr = TryParseCustomNumber()) {
    return expr;
  }

  // Dispatch on first character via prefix table
  auto it = prefix_table_.find(Peek());
  if (it != prefix_table_.end()) {
    return it->second();
  }

  // Identifier, symbol, or function call
  if (auto expr = TryParseIdentifierOrCall()) {
    return expr;
  }

  // Silence unused warning (symbols_ is passed for future use)
  (void)symbols_;

  if (pos_ < expr_.length()) {
    throw std::runtime_error("Unexpected character: " + std::string(1, Peek()));
  }
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

std::string ExpressionParser::ParseIdentifier() {
  SkipWhitespace();
  size_t start = pos_;

  // Identifier starts with letter, underscore, period, $, or ?.
  // ADR-005 V8: ']' as identifier-start is Merlin-specific, gated behind
  // ParserFeatures.allow_merlin_var_prefix.
  // ADR-005 V9: In Merlin mode, '.' is an operator, not an identifier character.
  bool is_merlin_var = (features_.allow_merlin_var_prefix && Peek() == ']');
  bool allow_dot_in_ident = !features_.allow_merlin_bitwise_ops;
  if (!(std::isalpha(Peek()) || Peek() == '_' ||
        (allow_dot_in_ident && Peek() == '.') || Peek() == '$' ||
        Peek() == '?' || is_merlin_var)) {
    throw std::runtime_error("Expected identifier");
  }

  Consume();

  // Continue with alphanumeric, underscore, period (unless Merlin mode), $, ?, @, :
  // '@' is used in SCMASM scoped local label names (e.g. GLOBAL@.1)
  // ':' is used in Merlin scoped local label names (e.g. MAIN:loop)
  // ADR-005 V9: In Merlin mode, '.' is a binary OR operator, not part of names.
  while (std::isalnum(Peek()) || Peek() == '_' ||
         (allow_dot_in_ident && Peek() == '.') || Peek() == '$' ||
         Peek() == '?' || Peek() == '@' || Peek() == ':') {
    Consume();
  }

  return expr_.substr(start, pos_ - start);
}

} // namespace xasm
