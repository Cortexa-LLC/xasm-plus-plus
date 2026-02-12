/**
 * @file scmasm_directive_handlers.cpp
 * @brief SCMASM directive handler implementations
 *
 * Free function implementations of SCMASM directive handlers.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#include "xasm++/directives/scmasm_directive_handlers.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {
namespace scmasm {

// ============================================================================
// Helper Functions (internal)
// ============================================================================

namespace {

using namespace constants;

/**
 * @brief Trim whitespace from both ends of string
 */
std::string Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

/**
 * @brief Apply SCMASM high-bit rule based on delimiter
 *
 * If delimiter ASCII < 0x27 (apostrophe '), high bit is SET.
 * Otherwise, high bit is CLEAR.
 *
 * @param c Character to transform
 * @param delimiter Delimiter character
 * @return Transformed character
 */
uint8_t ApplyHighBitRule(char c, char delimiter) {
  uint8_t result = static_cast<uint8_t>(c);

  if (delimiter < HIGH_BIT_DELIMITER_THRESHOLD) {
    // Set high bit
    result |= HIGH_BIT_MASK;
  } else {
    // Clear high bit
    result &= LOW_7_BITS_MASK;
  }

  return result;
}

/**
 * @brief Parse string with delimiter semantics
 *
 * Extracts string from operand and applies high-bit rule.
 *
 * @param operand String operand (with delimiters)
 * @param result Output vector of bytes
 * @return Delimiter character used
 * @throws std::runtime_error if string is malformed
 */
char ParseString(const std::string& operand, std::vector<uint8_t>& result) {
  result.clear();

  std::string trimmed = Trim(operand);
  if (trimmed.empty()) {
    throw std::runtime_error("String directive requires operand");
  }

  // Find delimiter (first character)
  char delimiter = trimmed[0];

  // Find closing delimiter
  size_t end = trimmed.find(delimiter, 1);
  if (end == std::string::npos) {
    throw std::runtime_error("Unterminated string");
  }

  // Extract string content (between delimiters)
  for (size_t i = 1; i < end; ++i) {
    char c = trimmed[i];
    uint8_t byte = ApplyHighBitRule(c, delimiter);
    result.push_back(byte);
  }

  return delimiter;
}

/**
 * @brief Evaluate expression using shared expression parser
 *
 * @param str Expression string
 * @param symbols Symbol table for lookups
 * @param parser_state Opaque parser state (ScmasmSyntaxParser*)
 * @return Evaluated value
 * @throws std::runtime_error on evaluation failure
 */
uint32_t EvaluateExpression(const std::string& str,
                            ConcreteSymbolTable& symbols,
                            void* parser_state) {
  // For now, delegate to the parser's method (via context)
  // In future refactoring, expression evaluation could be fully standalone
  auto* parser = static_cast<ScmasmSyntaxParser*>(parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }

  // Use parser's EvaluateExpression method
  // Note: This creates a temporary coupling that could be eliminated
  // in future phases by making expression evaluation fully standalone
  return parser->EvaluateExpression(str, symbols);
}

} // anonymous namespace

// ============================================================================
// Directive Handlers
// ============================================================================

void HandleOr(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately before dispatch

  if (operand.empty()) {
    throw std::runtime_error(".OR requires an address");
  }

  // Evaluate address expression
  uint32_t address = EvaluateExpression(operand, *context.symbols,
                                       context.parser_state);

  // Create ORG atom
  auto org_atom = std::make_shared<OrgAtom>(address);
  context.section->atoms.push_back(org_atom);

  // Update current address
  *context.current_address = address;
}

void HandleEq(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  if (operand.empty()) {
    throw std::runtime_error(".EQ requires a value");
  }

  // Evaluate value expression
  uint32_t value = EvaluateExpression(operand, *context.symbols,
                                     context.parser_state);

  // Define symbol (immutable) - .EQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(label, SymbolType::Equate, expr);
}

void HandleSe(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  if (operand.empty()) {
    throw std::runtime_error(".SE requires a value");
  }

  // Evaluate value expression
  uint32_t value = EvaluateExpression(operand, *context.symbols,
                                     context.parser_state);

  // .SE creates Set type (redefinable)
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(label, SymbolType::Set, expr);
}

void HandleAs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseString(operand, data);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleAt(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseString(operand, data);

  // Set high bit on LAST character
  if (!data.empty()) {
    data.back() |= constants::HIGH_BIT_MASK;
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleAz(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseString(operand, data);

  // Add null terminator
  data.push_back(constants::NULL_TERMINATOR);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleDa(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;

  // Split by comma
  std::string trimmed = Trim(operand);
  std::vector<std::string> values;
  size_t start = 0;
  size_t pos = 0;

  while (pos <= trimmed.length()) {
    if (pos == trimmed.length() || trimmed[pos] == ',') {
      std::string value = Trim(trimmed.substr(start, pos - start));
      if (!value.empty()) {
        values.push_back(value);
      }
      start = pos + 1;
    }
    ++pos;
  }

  // SCMASM .DA: Size determined by operator prefix
  // #expr → 8-bit (low byte)
  // /expr → 8-bit (second byte, bits 8-15)
  // expr  → 16-bit (default, little-endian)
  // <expr → 24-bit (little-endian)
  // >expr → 32-bit (little-endian)
  for (const auto& val : values) {
    std::string value_trimmed = Trim(val);

    if (value_trimmed.empty()) {
      continue;
    }

    char prefix = value_trimmed[0];
    std::string expr;

    if (prefix == '#') {
      // 8-bit: low byte only
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, *context.symbols,
                                       context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
    } else if (prefix == '/') {
      // 8-bit: second byte (bits 8-15)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, *context.symbols,
                                       context.parser_state);
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                         constants::BYTE_MASK));
    } else if (prefix == '<') {
      // 24-bit: three bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, *context.symbols,
                                       context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                         constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) &
                                         constants::BYTE_MASK));
    } else if (prefix == '>') {
      // 32-bit: four bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, *context.symbols,
                                       context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                         constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) &
                                         constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_3_SHIFT) &
                                         constants::BYTE_MASK));
    } else {
      // DEFAULT: 16-bit (little-endian)
      uint32_t num = EvaluateExpression(value_trimmed, *context.symbols,
                                       context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                         constants::BYTE_MASK));
    }
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleHs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  std::string trimmed = Trim(operand);

  // Remove all whitespace
  std::string hex_digits;
  for (char c : trimmed) {
    if (!std::isspace(c)) {
      if (!std::isxdigit(c)) {
        throw std::runtime_error("Invalid hex digit in .HS: " +
                                std::string(1, c));
      }
      hex_digits += c;
    }
  }

  // Must have even number of digits
  if (hex_digits.length() % constants::HEX_DIGITS_PER_BYTE != 0) {
    throw std::runtime_error(".HS requires even number of hex digits");
  }

  // Convert pairs to bytes
  for (size_t i = 0; i < hex_digits.length();
       i += constants::HEX_DIGITS_PER_BYTE) {
    std::string byte_str = hex_digits.substr(i, constants::HEX_DIGITS_PER_BYTE);
    uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
    data.push_back(byte);
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleBs(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  std::string trimmed = Trim(operand);

  // Remove all whitespace
  std::string bin_digits;
  for (char c : trimmed) {
    if (!std::isspace(c)) {
      if (c != '0' && c != '1') {
        throw std::runtime_error("Invalid binary digit in .BS: " +
                                std::string(1, c));
      }
      bin_digits += c;
    }
  }

  // Must have multiple of 8 bits
  if (bin_digits.length() % constants::BITS_PER_BYTE != 0) {
    throw std::runtime_error(".BS requires multiple of 8 bits");
  }

  // Convert 8-bit groups to bytes
  for (size_t i = 0; i < bin_digits.length(); i += constants::BITS_PER_BYTE) {
    std::string byte_str = bin_digits.substr(i, constants::BITS_PER_BYTE);
    uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 2));
    data.push_back(byte);
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleMa(const std::string& label,
              const std::string& operand,
              DirectiveContext& context) {
  // Macro name can come from label or operand
  std::string macro_name;

  if (!label.empty()) {
    macro_name = label;
  } else if (!operand.empty()) {
    macro_name = Trim(operand);
  } else {
    throw std::runtime_error(".MA requires a macro name");
  }

  // Access parser state to set macro definition mode
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }

  // Delegate to parser's HandleMa method
  // Note: This maintains coupling to parser for macro state management
  // which is acceptable as macros require parser-level state tracking
  parser->HandleMa(label, operand);
}

void HandleEndm(const std::string& label,
                const std::string& operand,
                DirectiveContext& context) {
  (void)label;
  (void)operand;

  // Access parser state to end macro definition
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }

  // Delegate to parser's HandleEm method
  parser->HandleEm();
}

} // namespace scmasm
} // namespace xasm
