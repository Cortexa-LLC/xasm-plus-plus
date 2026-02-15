// EdtasmSyntax - EDTASM+ assembly syntax parser for TRS-80 Color Computer
// Supports standard 6809 directives and syntax

#include "xasm++/syntax/edtasm_syntax.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/edtasm_simple_directive_handlers.h"
#include "xasm++/parse_utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

namespace {} // anonymous namespace

// ===========================================================================
// Constructor
// ===========================================================================

EdtasmSyntaxParser::EdtasmSyntaxParser() { InitializeDirectiveRegistry(); }

// ===========================================================================
// Directive Registration
// ===========================================================================

void EdtasmSyntaxParser::InitializeDirectiveRegistry() {
  using namespace directives;

  // Register directive handlers from edtasm namespace (direct assignment)
  directive_registry_[ORG] = edtasm::HandleOrg;
  directive_registry_[END] = edtasm::HandleEnd;
  directive_registry_[EQU] = edtasm::HandleEqu;
  directive_registry_[SET] = edtasm::HandleSet;
  directive_registry_[FCB] = edtasm::HandleFcb;
  directive_registry_[FDB] = edtasm::HandleFdb;
  directive_registry_[FCC] = edtasm::HandleFcc;
  directive_registry_[RMB] = edtasm::HandleRmb;
  directive_registry_[SETDP] = edtasm::HandleSetdp;
}

// ===========================================================================
// Helper Functions
// ===========================================================================

// Helper: Trim whitespace from both ends
std::string EdtasmSyntaxParser::Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

// Helper: Convert to uppercase
std::string EdtasmSyntaxParser::ToUpper(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

// Helper: Strip comments (semicolon to end of line)
std::string EdtasmSyntaxParser::StripComments(const std::string &str) {
  size_t comment_pos = str.find(';');
  if (comment_pos != std::string::npos) {
    return str.substr(0, comment_pos);
  }
  return str;
}

// Helper: Check if line is a comment line (starts with *)
bool EdtasmSyntaxParser::IsCommentLine(const std::string &line) {
  std::string trimmed = Trim(line);
  return !trimmed.empty() && trimmed[0] == '*';
}

// Helper: Parse numeric value (supports $hex, %binary, 'char', decimal)
uint32_t EdtasmSyntaxParser::ParseNumber(const std::string &str) {
  std::string trimmed = Trim(str);

  if (trimmed.empty()) {
    throw std::runtime_error("Empty number string");
  }

  // Hexadecimal ($xxxx)
  if (trimmed[0] == '$') {
    return ParseHex(trimmed.substr(1));
  }

  // Binary (%10101010)
  if (trimmed[0] == '%') {
    std::string binary = trimmed.substr(1);
    uint32_t value = 0;
    for (char c : binary) {
      int digit;
      if (!ParseBinaryDigit(c, digit)) {
        throw std::runtime_error("Invalid binary digit: " + std::string(1, c));
      }
      value = (value << 1) | digit;
    }
    return value;
  }

  // ASCII character ('A')
  if (trimmed[0] == '\'' && trimmed.length() >= 3 && trimmed[2] == '\'') {
    return static_cast<uint32_t>(trimmed[1]);
  }

  // Decimal (default)
  try {
    return static_cast<uint32_t>(xasm::ParseDecimal(trimmed));
  } catch (const std::exception &e) {
    throw std::runtime_error("Invalid decimal number: " + trimmed);
  }
}

// ===========================================================================
// Directive Parsing (Using DirectiveContext Pattern)
// ===========================================================================

// Parse directive using registry pattern (O(1) lookup)
void EdtasmSyntaxParser::ParseDirective(const std::string &directive,
                                        const std::string &operands,
                                        const std::string &label,
                                        Section &section,
                                        ConcreteSymbolTable &symbols) {
  std::string dir_upper = ToUpper(directive);

  // Lookup directive handler in registry
  auto it = directive_registry_.find(dir_upper);
  if (it != directive_registry_.end()) {
    // Create DirectiveContext for handler
    DirectiveContext context;
    context.section = &section;
    context.symbols = &symbols;
    context.current_address = &current_address_;
    context.parser_state = this;

    // Call registered handler with correct parameter order (label, operand,
    // context)
    it->second(label, operands, context);
    return;
  }

  throw std::runtime_error("Unknown directive: " + directive);
}

// Parse a single line
void EdtasmSyntaxParser::ParseLine(const std::string &line, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // Strip comments first
  std::string processed = StripComments(line);
  processed = Trim(processed);

  if (processed.empty()) {
    return;
  }

  // Check if this is a comment line
  if (IsCommentLine(processed)) {
    return;
  }

  // Parse line structure: [label] [opcode] [operands]
  std::string label, opcode, operands;

  // Check if line starts with whitespace (no label)
  if (line.empty() || std::isspace(static_cast<unsigned char>(line[0]))) {
    // No label, find opcode
    size_t opcode_start = processed.find_first_not_of(" \t");
    if (opcode_start == std::string::npos) {
      return;
    }

    size_t opcode_end = processed.find_first_of(" \t", opcode_start);
    if (opcode_end == std::string::npos) {
      opcode = processed.substr(opcode_start);
    } else {
      opcode = processed.substr(opcode_start, opcode_end - opcode_start);
      operands = Trim(processed.substr(opcode_end));
    }
  } else {
    // Has label, extract it
    size_t label_end = processed.find_first_of(" \t");
    if (label_end == std::string::npos) {
      // Label only, no opcode
      label = processed;
    } else {
      label = processed.substr(0, label_end);
      std::string rest = Trim(processed.substr(label_end));

      size_t opcode_end = rest.find_first_of(" \t");
      if (opcode_end == std::string::npos) {
        opcode = rest;
      } else {
        opcode = rest.substr(0, opcode_end);
        operands = Trim(rest.substr(opcode_end));
      }
    }
  }

  if (opcode.empty()) {
    // Label-only line
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
    }
    return;
  }

  // Check if opcode is a directive
  std::string opcode_upper = ToUpper(opcode);
  bool is_directive =
      (opcode_upper == directives::ORG || opcode_upper == directives::END ||
       opcode_upper == directives::EQU || opcode_upper == directives::SET ||
       opcode_upper == directives::FCB || opcode_upper == directives::FDB ||
       opcode_upper == directives::FCC || opcode_upper == directives::RMB ||
       opcode_upper == directives::SETDP);

  // Create label atom for non-EQU/SET directives and instructions
  // EQU and SET handle their labels internally (they don't create address
  // labels)
  if (!label.empty() && opcode_upper != directives::EQU &&
      opcode_upper != directives::SET) {
    symbols.Define(label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(current_address_));
    section.atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address_));
  }

  if (is_directive) {
    ParseDirective(opcode, operands, label, section, symbols);
    return;
  }

  // Otherwise, it's an instruction
  section.atoms.push_back(
      std::make_shared<InstructionAtom>(opcode_upper, operands));
  current_address_ += 1; // Placeholder (actual size determined during encoding)
}

// Main parse function
void EdtasmSyntaxParser::Parse(const std::string &source, Section &section,
                               ConcreteSymbolTable &symbols) {
  if (source.empty()) {
    return;
  }

  // Reset state
  current_address_ = 0;
  direct_page_ = 0;

  // Split into lines and parse
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    ParseLine(line, section, symbols);
  }
}

} // namespace xasm
