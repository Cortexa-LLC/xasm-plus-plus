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
#include "xasm++/directives/directive_error_utils.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/parse_utils.h" // For radix parsing utilities
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace xasm {
namespace scmasm {

using namespace directive_utils;

// ============================================================================
// Helper Functions (internal)
// ============================================================================

namespace {

using namespace constants;

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
  ValidateParser(parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(parser_state);

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

  RequireOperand(operand, ".OR", context);

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
  RequireOperand(operand, ".EQ", context);

  // Trim operand to remove CRLF line endings and whitespace
  std::string value_expr = Trim(operand);

  // Simple comment handling: everything after first whitespace is a comment
  // This handles all cases: "180 float", "XX+00  5/6 for 50/60Hz", "$Cn
  // comment"
  size_t ws = value_expr.find_first_of(" \t");
  if (ws != std::string::npos) {
    value_expr = value_expr.substr(0, ws);
  }

  // Evaluate value expression
  uint32_t value =
      EvaluateExpression(value_expr, *context.symbols, context.parser_state);

  // Define symbol (immutable) - .EQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(label, SymbolType::Equate, expr);
}

void HandleSe(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  RequireOperand(operand, ".SE", context);

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

  // Split by comma
  std::string trimmed = Trim(operand);
  std::vector<std::string> raw_expressions;
  size_t start = 0;
  size_t pos = 0;

  while (pos <= trimmed.length()) {
    if (pos == trimmed.length() || trimmed[pos] == ',') {
      std::string value = Trim(trimmed.substr(start, pos - start));
      if (!value.empty()) {
        raw_expressions.push_back(value);
      }
      start = pos + 1;
    }
    ++pos;
  }

  // Convert SCMASM operators to byte-level expressions for deferred evaluation
  // SCMASM .DA: Size determined by operator prefix
  // #expr → 8-bit (low byte) - convert to <expr
  // /expr → 8-bit (second byte, bits 8-15) - convert to >expr
  // expr  → 16-bit (default, little-endian) - expand to <expr, >expr
  // <expr → 24-bit (little-endian) - expand to 3 byte expressions
  // >expr → 32-bit (little-endian) - expand to 4 byte expressions
  //
  // All expressions are converted to BYTE-level so DataAtom can use
  // DataSize::Byte This allows the multi-pass assembler to resolve forward
  // references correctly
  std::vector<std::string> byte_expressions;
  std::vector<uint8_t> data; // For immediate evaluation attempt

  for (const auto &expr : raw_expressions) {
    std::string trimmed_expr = Trim(expr);

    if (trimmed_expr.empty()) {
      continue;
    }

    char prefix = trimmed_expr[0];
    std::string base_expr;

    if (prefix == '#') {
      // SCMASM # (low byte) → generic < (low byte)
      base_expr = Trim(trimmed_expr.substr(1));
      byte_expressions.push_back("<" + base_expr);

      // Try immediate evaluation
      try {
        uint32_t num = EvaluateExpression(base_expr, *context.symbols,
                                          context.parser_state);
        data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
      } catch (...) {
        // Forward reference - evaluation will happen in assembler
        data.push_back(0); // Placeholder
      }
    } else if (prefix == '/') {
      // SCMASM / (high byte) → generic > (high byte)
      base_expr = Trim(trimmed_expr.substr(1));
      byte_expressions.push_back(">" + base_expr);

      // Try immediate evaluation
      try {
        uint32_t num = EvaluateExpression(base_expr, *context.symbols,
                                          context.parser_state);
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                            constants::BYTE_MASK));
      } catch (...) {
        // Forward reference - evaluation will happen in assembler
        data.push_back(0); // Placeholder
      }
    } else if (prefix == '<') {
      // SCMASM < (24-bit) → expand to 3 bytes
      base_expr = Trim(trimmed_expr.substr(1));
      byte_expressions.push_back("<" + base_expr); // Byte 0
      byte_expressions.push_back(">" + base_expr); // Byte 1
      byte_expressions.push_back("<(" + base_expr +
                                 ")"); // Byte 2 - placeholder

      // Try immediate evaluation
      try {
        uint32_t num = EvaluateExpression(base_expr, *context.symbols,
                                          context.parser_state);
        data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                            constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) &
                                            constants::BYTE_MASK));
      } catch (...) {
        // Forward reference
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
      }
    } else if (prefix == '>') {
      // SCMASM > (32-bit) → expand to 4 bytes
      base_expr = Trim(trimmed_expr.substr(1));
      byte_expressions.push_back("<" + base_expr); // Byte 0
      byte_expressions.push_back(">" + base_expr); // Byte 1
      byte_expressions.push_back("<(" + base_expr +
                                 ")"); // Byte 2 - placeholder
      byte_expressions.push_back(">(" + base_expr +
                                 ")"); // Byte 3 - placeholder

      // Try immediate evaluation
      try {
        uint32_t num = EvaluateExpression(base_expr, *context.symbols,
                                          context.parser_state);
        data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                            constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) &
                                            constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_3_SHIFT) &
                                            constants::BYTE_MASK));
      } catch (...) {
        // Forward reference
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
      }
    } else {
      // Default: 16-bit word (no prefix) - expand to 2 bytes (little-endian)
      base_expr = trimmed_expr;
      byte_expressions.push_back("<" + base_expr); // Low byte
      byte_expressions.push_back(">" + base_expr); // High byte

      // Try immediate evaluation
      try {
        uint32_t num = EvaluateExpression(base_expr, *context.symbols,
                                          context.parser_state);
        data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
        data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) &
                                            constants::BYTE_MASK));
      } catch (...) {
        // Forward reference
        data.push_back(0);
        data.push_back(0);
      }
    }
  }

  // Create DataAtom with expressions AND initial data
  // - expressions: for multi-pass forward reference resolution
  // - data: for immediate cases (tests, simple expressions)
  auto atom = std::make_shared<DataAtom>(byte_expressions, DataSize::Byte);
  atom->data = data;
  atom->size = data.size();
  context.section->atoms.push_back(atom);

  // Update address counter by number of bytes
  *context.current_address += data.size();
}

void HandleHs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  std::vector<uint8_t> data;
  std::string trimmed = Trim(operand);

  // Extract hex digits, stopping at first WORD containing non-hex character
  // This allows inline comments without semicolons, like .EQ directive
  // Examples:
  //   .HS 01 02 03        -> 3 bytes (spaces between hex OK)
  //   .HS DEADBEEF foo    -> 4 bytes (text after hex ignored)
  //   .HS AB CD EFG       -> 2 bytes (stops at word "EFG" containing 'G')
  std::string hex_digits;
  size_t i = 0;
  while (i < trimmed.length()) {
    // Skip whitespace
    while (i < trimmed.length() && std::isspace(trimmed[i])) {
      i++;
    }

    // Find end of current word
    size_t word_start = i;
    while (i < trimmed.length() && !std::isspace(trimmed[i])) {
      i++;
    }

    if (i > word_start) {
      std::string word = trimmed.substr(word_start, i - word_start);

      // Check if ALL characters in word are hex digits
      bool all_hex = true;
      for (char c : word) {
        if (!std::isxdigit(c)) {
          all_hex = false;
          break;
        }
      }

      if (all_hex) {
        hex_digits += word;
      } else {
        // Stop at first word with non-hex character
        break;
      }
    }
  }

  // Must have even number of digits
  if (hex_digits.length() % constants::HEX_DIGITS_PER_BYTE != 0) {
    throw std::runtime_error(".HS requires even number of hex digits");
  }

  // Convert pairs to bytes using ParseHex utility
  for (size_t i = 0; i < hex_digits.length();
       i += constants::HEX_DIGITS_PER_BYTE) {
    std::string byte_str =
        "$" + hex_digits.substr(i, constants::HEX_DIGITS_PER_BYTE);
    bool success;
    std::string error_msg;
    uint32_t byte_val = xasm::ParseHexSafe(byte_str, success, error_msg);
    if (!success) {
      ThrowFormattedError(error_msg, context);
    }
    data.push_back(static_cast<uint8_t>(byte_val));
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
    ThrowFormattedError(".BS requires a byte count", context);
  }

  std::string trimmed = Trim(operand);

  // Evaluate the byte count expression (supports symbols, hex, decimal)
  uint32_t byte_count =
      EvaluateExpression(trimmed, *context.symbols, context.parser_state);

  // Validate byte count (reasonable limit: 64KB)
  if (byte_count > 65536) {
    throw std::runtime_error(".BS byte count too large (max 65536)");
  }

  // Check if in dummy mode (structure definition)
  ValidateParser(context.parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);

  if (parser->InDummySection()) {
    // In dummy mode: just advance address, don't emit bytes
    *context.current_address += byte_count;
  } else {
    // Normal mode: emit zeros
    std::vector<uint8_t> data(byte_count, 0x00);
    auto atom = std::make_shared<DataAtom>(data);
    context.section->atoms.push_back(atom);

    // Update address counter
    *context.current_address += byte_count;
  }
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
  ValidateParser(context.parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);

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
  ValidateParser(context.parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);

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

  // .INB - Include Source File
  // Parses assembly source file and includes it at current position
  // Used extensively in A2oSX for modular source file includes
  //
  // Path resolution (in order):
  // 1. If absolute path → use as-is
  // 2. Try relative to source file directory
  // 3. Try each directory in include_paths (from --include-path CLI option)
  // 4. Try relative to current working directory (fallback)

  RequireOperand(operand, ".INB", context);

  // Trim whitespace from filename
  std::string include_filename = Trim(operand);

  // Apply path mappings (if any)
  // Example: usr/src/shared/file.s → SHARED/file.s
  // This allows virtual paths in source to be mapped to actual filesystem paths
  if (context.path_mappings != nullptr && !context.path_mappings->empty()) {
    std::filesystem::path include_path_obj(include_filename);

    // Normalize path separators for comparison (use /)
    std::string normalized_path = include_path_obj.generic_string();

    // Find longest matching prefix in path mappings
    std::string longest_match_key;
    std::string longest_match_value;
    size_t longest_match_len = 0;

    for (const auto &[virtual_path, actual_path] : *context.path_mappings) {
      // Normalize virtual path for comparison
      std::filesystem::path virtual_path_obj(virtual_path);
      std::string normalized_virtual = virtual_path_obj.generic_string();

      // Check if normalized_path starts with normalized_virtual
      if (normalized_path.find(normalized_virtual) == 0) {
        // Ensure it's a complete path component match (not substring)
        size_t virtual_len = normalized_virtual.length();
        if (virtual_len > normalized_path.length()) {
          continue; // Virtual path longer than include path
        }

        // Check that match is at path boundary
        // Empty prefix is always valid (matches everything)
        if (virtual_len == 0 || virtual_len == normalized_path.length() ||
            normalized_path[virtual_len] == '/') {
          // This is a valid prefix match
          if (virtual_len >= longest_match_len) {
            longest_match_len = virtual_len;
            longest_match_key = normalized_virtual;
            longest_match_value = actual_path;
          }
        }
      }
    }

    // Apply mapping if found
    // Note: longest_match_len can be 0 for empty prefix mappings
    if (!longest_match_value.empty()) {
      // Replace virtual prefix with actual prefix
      std::string suffix = normalized_path.substr(longest_match_len);

      // Remove leading separator from suffix if present
      if (!suffix.empty() && suffix[0] == '/') {
        suffix = suffix.substr(1);
      }

      // Construct mapped path
      std::filesystem::path actual_base(longest_match_value);
      if (suffix.empty()) {
        include_filename = actual_base.string();
      } else {
        include_filename = (actual_base / suffix).string();
      }
    }
  }

  // Resolve include path
  std::filesystem::path resolved_path;
  bool found = false;
  std::vector<std::string>
      tried_paths; // Track attempted paths for error message

  // Case 1: Absolute path
  std::filesystem::path include_path(include_filename);
  if (include_path.is_absolute()) {
    tried_paths.push_back(include_filename);
    if (std::filesystem::exists(include_path)) {
      resolved_path = include_path;
      found = true;
    }
  } else {
    // Case 2: Relative to source file directory
    if (!context.current_file.empty()) {
      std::filesystem::path source_path(context.current_file);
      std::filesystem::path source_dir = source_path.parent_path();
      std::filesystem::path relative_path = source_dir / include_filename;

      tried_paths.push_back(relative_path.string());
      if (std::filesystem::exists(relative_path)) {
        resolved_path = relative_path;
        found = true;
      }
    }

    // Case 3: Try each directory in include_paths
    if (!found && context.include_paths != nullptr) {
      for (const auto &include_dir : *context.include_paths) {
        std::filesystem::path search_path =
            std::filesystem::path(include_dir) / include_filename;

        tried_paths.push_back(search_path.string());
        if (std::filesystem::exists(search_path)) {
          resolved_path = search_path;
          found = true;
          break;
        }
      }
    }

    // Case 4: Relative to current working directory (fallback)
    if (!found) {
      tried_paths.push_back(include_filename);
      if (std::filesystem::exists(include_filename)) {
        resolved_path = include_filename;
        found = true;
      }
    }
  }

  // If not found, try adding .txt extension (A2osX compatibility)
  // A2osX source files may reference "file.s" but physical file is "FILE.S.txt"
  if (!found) {
    std::string txt_filename = include_filename + ".txt";
    std::filesystem::path txt_include_path(txt_filename);

    if (txt_include_path.is_absolute()) {
      if (std::filesystem::exists(txt_include_path)) {
        resolved_path = txt_include_path;
        found = true;
      }
    } else {
      // Try relative to source directory
      if (!found && !context.current_file.empty()) {
        std::filesystem::path source_path(context.current_file);
        std::filesystem::path source_dir = source_path.parent_path();
        std::filesystem::path relative_path = source_dir / txt_filename;

        if (std::filesystem::exists(relative_path)) {
          resolved_path = relative_path;
          found = true;
        }
      }

      // Try include paths
      if (!found && context.include_paths != nullptr) {
        for (const auto &include_dir : *context.include_paths) {
          std::filesystem::path search_path =
              std::filesystem::path(include_dir) / txt_filename;

          if (std::filesystem::exists(search_path)) {
            resolved_path = search_path;
            found = true;
            break;
          }
        }
      }

      // Try current working directory
      if (!found) {
        if (std::filesystem::exists(txt_filename)) {
          resolved_path = txt_filename;
          found = true;
        }
      }
    }
  }

  if (!found) {
    std::string error_msg = ".INB cannot open file: " + include_filename;
    if (!tried_paths.empty()) {
      error_msg += " (searched: ";
      for (size_t i = 0; i < tried_paths.size(); ++i) {
        if (i > 0)
          error_msg += ", ";
        error_msg += tried_paths[i];
      }
      error_msg += ")";
    }
    throw std::runtime_error(error_msg);
  }

  // Read included file as text
  std::ifstream file(resolved_path);
  if (!file.is_open()) {
    throw std::runtime_error(".INB cannot open file: " +
                             resolved_path.string());
  }

  // Read entire file content
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source_content = buffer.str();

  // Get parser from context
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error(".INB: parser_state is null");
  }

  // Save current file for restoration
  std::string previous_file = parser->GetCurrentFile();

  // Update current file to included file (for nested includes)
  parser->SetCurrentFile(resolved_path.string());

  try {
    // Parse included source recursively
    parser->Parse(source_content, *context.section, *context.symbols);

    // Restore previous file
    parser->SetCurrentFile(previous_file);
  } catch (...) {
    // Restore previous file even on error
    parser->SetCurrentFile(previous_file);
    throw;
  }
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

  // Enter dummy section mode - data directives will update address but not emit
  // bytes
  ValidateParser(context.parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  parser->StartDummySection();
}

void HandleEd(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;   // Label handled separately
  (void)operand; // Operand unused

  // Exit dummy section mode - return to normal assembly
  ValidateParser(context.parser_state);
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  parser->EndDummySection();
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
  (void)label; // Label handled separately

  // Cast parser state to access phase tracking
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);

  // Check if we're ending a phase
  if (parser && parser->InPhase()) {
    // .EP without operand ends phase assembly
    if (operand.empty()) {
      // End phase and get new real address
      uint32_t new_real_addr = parser->EndPhase(*context.current_address);

      // Restore current address to real address + bytes emitted
      *context.current_address = new_real_addr;
      return;
    }
    // .EP with operand in phase context is an error
    throw std::runtime_error(
        ".EP with operand not allowed within .PH/.EP block");
  }

  // Not in phase - treat as entry point directive
  // .EP without operand sets entry to current address
  if (operand.empty()) {
    // TODO: Store current address as entry point in section metadata
    // For now, just accept the directive
    return;
  }

  // Evaluate entry point address
  uint32_t address =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // TODO: Store entry point in section metadata
  // For now, just validate the expression
  (void)address;
}

void HandlePh(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // Label handled separately

  RequireOperand(operand, ".PH", context);

  // Cast parser state to access phase tracking
  auto *parser = static_cast<ScmasmSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Parser state not available for .PH directive");
  }

  // Evaluate virtual address
  uint32_t virtual_addr =
      EvaluateExpression(operand, *context.symbols, context.parser_state);

  // Calculate current real address
  uint32_t real_addr;
  if (parser->InPhase()) {
    // If already in phase, calculate real address from phase state
    real_addr = parser->GetCurrentRealAddress(*context.current_address);
  } else {
    // Not in phase, current_address IS the real address
    real_addr = *context.current_address;
  }

  parser->StartPhase(real_addr, virtual_addr);

  // Set current address to virtual address
  *context.current_address = virtual_addr;
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

    // Convert single hex digit to nibble value (0-15)
    int val;
    if (!ParseHexDigit(c, val)) {
      throw std::runtime_error("Invalid hex digit in .HX: " +
                               std::string(1, c));
    }
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
