/**
 * @file edtasm_simple_directive_handlers.cpp
 * @brief EDTASM+ simple directive handler implementations
 *
 * Free function implementations for EDTASM+ (simple/CoCo) directives.
 * These handlers follow the Merlin pattern for architectural consistency.
 *
 * @note This is for the simple EDTASM parser (edtasm_syntax.cpp), not the
 *       extended EDTASM-M80++ parser (edtasm_m80_plusplus_syntax.cpp)
 */

#include "xasm++/directives/edtasm_simple_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/expression.h"
#include "xasm++/parse_utils.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/edtasm_syntax.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace {

/**
 * @brief Helper to get parser from context
 * @param ctx Directive context
 * @return Parser pointer or nullptr if not set
 */
xasm::EdtasmSyntaxParser *GetParser(xasm::DirectiveContext &ctx) {
  return static_cast<xasm::EdtasmSyntaxParser *>(ctx.parser_state);
}

/**
 * @brief Helper: Trim whitespace from both ends
 */
std::string Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

/**
 * @brief Helper: Parse numeric value (supports $hex, %binary, 'char', decimal)
 */
uint32_t ParseNumber(const std::string &str) {
  std::string trimmed = Trim(str);

  if (trimmed.empty()) {
    throw std::runtime_error("Empty number string");
  }

  // Hexadecimal ($xxxx)
  if (trimmed[0] == '$') {
    return xasm::ParseHex(trimmed.substr(1));
  }

  // Binary (%10101010)
  if (trimmed[0] == '%') {
    std::string binary = trimmed.substr(1);
    uint32_t value = 0;
    for (char c : binary) {
      if (c != '0' && c != '1') {
        throw std::runtime_error("Invalid binary digit: " + std::string(1, c));
      }
      value = (value << 1) | (c - '0');
    }
    return value;
  }

  // ASCII character ('A')
  if (trimmed[0] == '\'' && trimmed.length() >= 3 && trimmed[2] == '\'') {
    return static_cast<uint32_t>(trimmed[1]);
  }

  // Decimal (default)
  try {
    return static_cast<uint32_t>(std::stoul(trimmed, nullptr, 10));
  } catch (const std::exception &e) {
    throw std::runtime_error("Invalid decimal number: " + trimmed);
  }
}

} // anonymous namespace

namespace xasm {
namespace edtasm {

// ===========================================================================
// Directive Handlers - Free Functions in edtasm namespace
// ===========================================================================

void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  auto *parser = GetParser(context);
  if (!parser) {
    throw std::runtime_error("ORG: Invalid parser state");
  }

  uint32_t address = ParseNumber(operand);
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}

void HandleEnd(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;   // Label handled in ParseLine
  (void)operand; // Entry point not currently used
  (void)context; // END produces no atoms
  // END directive produces no atoms, signals end of assembly
}

void HandleEqu(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  if (label.empty()) {
    throw std::runtime_error("EQU requires a label");
  }

  uint32_t value = ParseNumber(operand);
  context.symbols->Define(label, SymbolType::Equate,
                          std::make_shared<LiteralExpr>(value));
}

void HandleSet(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  if (label.empty()) {
    throw std::runtime_error("SET requires a label");
  }

  uint32_t value = ParseNumber(operand);
  // SET allows redefinition, so we define it as Set type
  context.symbols->Define(label, SymbolType::Set,
                          std::make_shared<LiteralExpr>(value));
}

void HandleFcb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      bytes.push_back(static_cast<uint8_t>(ParseNumber(value)));
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleFdb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  std::vector<uint8_t> bytes;
  std::istringstream ops(operand);
  std::string value;

  while (std::getline(ops, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      uint32_t word = ParseNumber(value);
      // 6809 uses big-endian (MSB first)
      bytes.push_back(static_cast<uint8_t>((word >> 8) & 0xFF)); // High byte
      bytes.push_back(static_cast<uint8_t>(word & 0xFF));        // Low byte
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleFcc(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  std::string trimmed = Trim(operand);
  if (trimmed.empty()) {
    throw std::runtime_error("FCC requires operand");
  }

  // First non-whitespace character is the delimiter
  char delimiter = trimmed[0];
  size_t end_pos = trimmed.find(delimiter, 1);

  if (end_pos == std::string::npos) {
    throw std::runtime_error("FCC: Missing closing delimiter");
  }

  std::string text = trimmed.substr(1, end_pos - 1);
  std::vector<uint8_t> bytes(text.begin(), text.end());

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleRmb(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  uint32_t size = ParseNumber(operand);
  context.section->atoms.push_back(std::make_shared<SpaceAtom>(size));
  *context.current_address += size;
}

void HandleSetdp(const std::string &label, const std::string &operand,
                 DirectiveContext &context) {
  (void)label; // Label handled in ParseLine

  auto *parser = GetParser(context);
  if (!parser) {
    throw std::runtime_error("SETDP: Invalid parser state");
  }

  parser->direct_page_ = static_cast<uint8_t>(ParseNumber(operand));
  // SETDP produces no atoms, just informs assembler
}

} // namespace edtasm
} // namespace xasm
