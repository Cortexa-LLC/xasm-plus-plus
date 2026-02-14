/**
 * @file scmasm_directive_handlers.cpp
 * @brief SCMASM directive handler implementations
 *
 * Free function implementations of SCMASM directive handlers.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#include "xasm++/directives/scmasm_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace xasm {
namespace scmasm {

// ============================================================================
// Helper Functions (internal)
// ============================================================================

namespace {

using namespace constants;

// Radix values for number parsing
constexpr int RADIX_HEXADECIMAL = 16;

/**
 * @brief Trim whitespace from both ends of string
 */
std::string Trim(const std::string &str) {
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
char ParseString(const std::string &operand, std::vector<uint8_t> &result) {
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

// Apply INVERTED high-bit rule (for .PS directive)
// Delimiter >= 0x27: SET high bit
// Delimiter < 0x27: CLEAR high bit
uint8_t ApplyInvertedHighBitRule(char c, char delimiter) {
  uint8_t result = static_cast<uint8_t>(c);

  if (delimiter >= HIGH_BIT_DELIMITER_THRESHOLD) {
    // Set high bit (inverted from normal rule)
    result |= HIGH_BIT_MASK;
  } else {
    // Clear high bit (inverted from normal rule)
    result &= LOW_7_BITS_MASK;
  }

  return result;
}

// Parse string with INVERTED high-bit rule (for .PS)
char ParseStringInverted(const std::string &operand,
                         std::vector<uint8_t> &result) {
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

  // Extract string content (between delimiters) with inverted high-bit rule
  for (size_t i = 1; i < end; ++i) {
    char c = trimmed[i];
    uint8_t byte = ApplyInvertedHighBitRule(c, delimiter);
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
uint32_t EvaluateExpression(const std::string &str,
                            ConcreteSymbolTable &symbols, void *parser_state) {
  // For now, delegate to the parser's method (via context)
  // In future refactoring, expression evaluation could be fully standalone
  auto *parser = static_cast<ScmasmSyntaxParser *>(parser_state);
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

void HandleOr(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately before dispatch

  if (operand.empty()) {
    throw std::runtime_error(".OR requires an address");
  }

  // Evaluate address expression
  uint32_t address =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // Create ORG atom
  auto org_atom = std::make_shared<OrgAtom>(address);
  context.section->atoms.push_back(org_atom);

  // Update current address
  *context.current_address = address;
}

void HandleEq(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  if (operand.empty()) {
    throw std::runtime_error(".EQ requires a value");
  }

  // Evaluate value expression
  uint32_t value =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // Define symbol (immutable) - .EQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(label, SymbolType::Equate, expr);
}

void HandleSe(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  if (operand.empty()) {
    throw std::runtime_error(".SE requires a value");
  }

  // Evaluate value expression
  uint32_t value =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // .SE creates Set type (redefinable)
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(label, SymbolType::Set, expr);
}

void HandleAs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseString(operand, data);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleAt(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
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

void HandleAz(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
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

void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
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
  for (const auto &val : values) {
    std::string value_trimmed = Trim(val);

    if (value_trimmed.empty()) {
      continue;
    }

    char prefix = value_trimmed[0];
    std::string expr;

    if (prefix == '#') {
      // 8-bit: low byte only
      expr = Trim(value_trimmed.substr(1));
      uint32_t num =
          EvaluateExpression(expr, *context.symbols, context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
    } else if (prefix == '/') {
      // 8-bit: second byte (bits 8-15)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num =
          EvaluateExpression(expr, *context.symbols, context.parser_state);
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                          constants::BYTE_MASK));
    } else if (prefix == '<') {
      // 24-bit: three bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num =
          EvaluateExpression(expr, *context.symbols, context.parser_state);
      data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                          constants::BYTE_MASK));
      data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) &
                                          constants::BYTE_MASK));
    } else if (prefix == '>') {
      // 32-bit: four bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num =
          EvaluateExpression(expr, *context.symbols, context.parser_state);
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

void HandleHs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
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
    uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, RADIX_HEXADECIMAL));
    data.push_back(byte);
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleBs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  // .BS (Block Storage) - Reserve N bytes of space
  // SCMASM syntax: .BS count
  // Where count is a decimal or hex number ($hex, %binary)
  // This reserves 'count' bytes filled with zeros
  
  if (operand.empty()) {
    throw std::runtime_error(".BS requires a byte count");
  }

  std::string trimmed = Trim(operand);
  
  // Evaluate the byte count expression (supports symbols, hex, decimal)
  uint32_t byte_count =
      EvaluateExpression(trimmed, *context.symbols, context.parser_state);

  // Validate byte count (reasonable limit: 64KB)
  if (byte_count > 65536) {
    throw std::runtime_error(".BS byte count too large (max 65536)");
  }

  // Create data filled with zeros
  std::vector<uint8_t> data(byte_count, 0x00);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += byte_count;
}

void HandleMa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
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
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }

  // Delegate to parser's HandleMa method
  // Note: This maintains coupling to parser for macro state management
  // which is acceptable as macros require parser-level state tracking
  parser->HandleMa(label, operand);
}

void HandleEndm(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;
  (void)operand;

  // Access parser state to end macro definition
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }

  // Delegate to parser's HandleEm method
  parser->HandleEm();
}

// ============================================================================
// P0 Priority Directive Handlers (A2oSX Critical)
// ============================================================================

void HandlePs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  // Parse string with INVERTED high-bit rule (.PS is opposite of .AS)
  std::vector<uint8_t> data;
  ParseStringInverted(operand, data);

  // Validate length (Pascal strings are max 255 bytes)
  if (data.size() > constants::PASCAL_STRING_MAX_LENGTH) {
    throw std::runtime_error(".PS string too long (max 255 bytes)");
  }

  // Create result with length prefix
  std::vector<uint8_t> result;
  result.push_back(static_cast<uint8_t>(data.size()));
  result.insert(result.end(), data.begin(), data.end());

  auto atom = std::make_shared<DataAtom>(result);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += result.size();
}

void HandleInb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled separately

  // .INB - Include Binary File
  // Reads entire binary file and emits its bytes at current position
  // Used extensively in A2oSX for modular source file includes

  if (operand.empty()) {
    throw std::runtime_error(".INB requires filename operand");
  }

  // Trim whitespace from filename
  std::string filename = Trim(operand);

  // Open file in binary mode
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error(".INB cannot open file: " + filename);
  }

  // Get file size
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Read entire file into vector
  std::vector<uint8_t> buffer(size);
  if (size > 0) {
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
      throw std::runtime_error(".INB error reading file: " + filename);
    }
  }

  // Emit binary data as DataAtom
  auto atom = std::make_shared<DataAtom>(buffer);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += buffer.size();
}

void HandleList(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Listing control parameter (ON/OFF)
  (void)context; // No state changes needed for stub

  // Stub implementation - listing control has no effect yet
  // This allows .LIST directives to be parsed without error
  // Full listing output generation is out of scope for P0
}

void HandleDummy(const std::string &label, const std::string &operand,
                 DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Optional operand
  (void)context; // State management

  // TODO: Implement dummy section (structure definition mode)
  // For now, stub implementation
  throw std::runtime_error(".DUMMY not yet implemented");
}

void HandleOp(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  // TODO: Implement CPU operation mode switching
  // For now, stub implementation
  std::string trimmed = Trim(operand);
  std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::toupper);

  // Validate CPU name (6502, 65C02, 65816)
  if (trimmed != "6502" && trimmed != "65C02" && trimmed != "65816") {
    throw std::runtime_error(".OP requires valid CPU (6502, 65C02, 65816)");
  }

  // Stub: Accept valid CPU names but don't switch yet
  // Full CPU switching requires CPU abstraction changes (out of scope for
  // immediate P0)
  (void)context;
}

// ============================================================================
// Phase 3: 100% Coverage Directive Handlers
// ============================================================================

namespace {

/**
 * @brief Parse escape sequence in C-style string
 * @param s Input string pointer (will be advanced)
 * @return Parsed byte value
 */
uint8_t ParseEscapeSequence(const char *&s) {
  if (*s != '\\') {
    return static_cast<uint8_t>(*s++);
  }

  s++; // Skip backslash

  switch (*s) {
  case 'a':
    s++;
    return constants::ascii::BELL;
  case 'b':
    s++;
    return constants::ascii::BACKSPACE;
  case 'e':
    s++;
    return constants::ascii::ESCAPE;
  case 'f':
    s++;
    return constants::ascii::FORMFEED;
  case 'n':
    s++;
    return constants::ascii::NEWLINE;
  case 'r':
    s++;
    return constants::ascii::CR;
  case 't':
    s++;
    return constants::ascii::TAB;
  case 'v':
    s++;
    return constants::ascii::VTAB;
  case '0':
    s++;
    return constants::ascii::NULL_CHAR;
  case '\\':
    s++;
    return '\\'; // Backslash
  case '"':
    s++;
    return '"'; // Double quote
  case '\'':
    s++;
    return '\''; // Single quote
  case 'x': {
    // Hex escape: \xHH
    s++;
    if (std::isxdigit(*s) && std::isxdigit(*(s + 1))) {
      int hi = std::isdigit(*s) ? (*s - '0') : (std::toupper(*s) - 'A' + 10);
      s++;
      int lo = std::isdigit(*s) ? (*s - '0') : (std::toupper(*s) - 'A' + 10);
      s++;
      return static_cast<uint8_t>((hi << 4) | lo);
    } else {
      // Invalid hex escape - just return 'x'
      return 'x';
    }
  }
  default:
    // Unknown escape - return character as-is
    return static_cast<uint8_t>(*s++);
  }
}

/**
 * @brief Parse C-style string with escape sequences
 * @param operand String operand (with delimiters)
 * @param result Output vector of bytes
 */
void ParseCString(const std::string &operand, std::vector<uint8_t> &result) {
  result.clear();

  std::string trimmed = Trim(operand);
  if (trimmed.empty()) {
    throw std::runtime_error("String directive requires operand");
  }

  // Find delimiter (first character)
  char delimiter = trimmed[0];

  // Find closing delimiter (skip escaped ones)
  size_t end = std::string::npos;
  for (size_t i = 1; i < trimmed.length(); ++i) {
    if (trimmed[i] == delimiter && trimmed[i - 1] != '\\') {
      end = i;
      break;
    }
    // Handle escaped backslash before delimiter: \\"
    if (trimmed[i] == delimiter && i >= 2 && trimmed[i - 1] == '\\' &&
        trimmed[i - 2] == '\\') {
      end = i;
      break;
    }
  }

  if (end == std::string::npos) {
    throw std::runtime_error("Unterminated string");
  }

  // Parse string content with escape sequences
  const char *s = trimmed.c_str() + 1; // Start after opening delimiter
  const char *s_end = trimmed.c_str() + end;

  while (s < s_end) {
    uint8_t byte = ParseEscapeSequence(s);
    result.push_back(byte);
  }
}

} // anonymous namespace

void HandleCs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseCString(operand, data);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleCz(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  ParseCString(operand, data);

  // Add null terminator
  data.push_back(constants::NULL_TERMINATOR);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleTf(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // File path or title metadata
  (void)context; // No state changes needed

  // Stub implementation - .TF is metadata directive
  // Used to specify target filename or title in SCMASM
  // Has no effect on code generation in cross-assembler
}

void HandleEp(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)context; // State not needed for validation

  if (operand.empty()) {
    throw std::runtime_error(".EP requires an address");
  }

  // Evaluate entry point address
  uint32_t address =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // TODO: Store entry point in section metadata
  // For now, just validate the expression
  (void)address;
}

void HandleHx(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  std::string trimmed = Trim(operand);

  // Parse hex nibbles (single hex digits)
  // Whitespace, dots, and commas are allowed as separators
  for (char c : trimmed) {
    if (std::isspace(c) || c == '.' || c == ',') {
      continue; // Skip separators
    }

    if (!std::isxdigit(c)) {
      throw std::runtime_error("Invalid hex digit in .HX: " +
                               std::string(1, c));
    }

    // Convert single hex digit to nibble value (0-15)
    int val = std::isdigit(c) ? (c - '0') : (std::toupper(c) - 'A' + 10);
    data.push_back(static_cast<uint8_t>(val));
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleTa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Target address
  (void)context; // No-op

  // .TA (Target Address) is a no-op in cross-assemblers
  // Used by SCMASM editor to set assembly target on Apple II
  // In cross-assembly, has no effect
}

void HandleDo(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Condition expression
  (void)context; // State management

  // TODO: Implement conditional assembly
  // .DO/.FIN require special handling in ParseLine (not registry dispatch)
  // For now, stub implementation
  throw std::runtime_error(".DO conditional assembly not yet implemented");
}

void HandleFin(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Unused
  (void)context; // State management

  // TODO: Implement conditional assembly
  // For now, stub implementation
  throw std::runtime_error(".FIN conditional assembly not yet implemented");
}

void HandleAc(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // String with optional prefix
  (void)context; // State management

  // TODO: Implement .AC directive (ASCII string with optional numeric prefix)
  // For now, stub implementation
  throw std::runtime_error(".AC directive not yet implemented");
}

} // namespace scmasm
} // namespace xasm
