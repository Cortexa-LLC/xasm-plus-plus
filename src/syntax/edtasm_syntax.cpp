// EdtasmSyntax - EDTASM+ assembly syntax parser for TRS-80 Color Computer
// Supports standard 6809 directives and syntax

#include "xasm++/syntax/edtasm_syntax.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/parse_utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

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

// Parse directive (ORG, END, EQU, SET, FCB, FDB, FCC, RMB, SETDP)
void EdtasmSyntaxParser::ParseDirective(const std::string &directive,
                                        const std::string &operands,
                                        const std::string &label,
                                        Section &section,
                                        ConcreteSymbolTable &symbols) {
  std::string dir_upper = ToUpper(directive);

  // ORG - Set origin address
  if (dir_upper == directives::ORG) {
    uint32_t address = ParseNumber(operands);
    section.atoms.push_back(std::make_shared<OrgAtom>(address));
    current_address_ = address;
    return;
  }

  // END - End assembly (may have entry point)
  if (dir_upper == directives::END) {
    // END directive produces no atoms, signals end of assembly
    return;
  }

  // EQU - Equate symbol (constant)
  if (dir_upper == directives::EQU) {
    if (label.empty()) {
      throw std::runtime_error("EQU requires a label");
    }
    uint32_t value = ParseNumber(operands);
    symbols.Define(label, SymbolType::Equate,
                   std::make_shared<LiteralExpr>(value));
    return;
  }

  // SET - Set variable (can be redefined)
  if (dir_upper == directives::SET) {
    if (label.empty()) {
      throw std::runtime_error("SET requires a label");
    }
    uint32_t value = ParseNumber(operands);
    // SET allows redefinition, so we define it as Set type
    symbols.Define(label, SymbolType::Set,
                   std::make_shared<LiteralExpr>(value));
    return;
  }

  // FCB - Form Constant Byte
  if (dir_upper == directives::FCB) {
    std::vector<uint8_t> bytes;
    std::istringstream ops(operands);
    std::string value;

    while (std::getline(ops, value, ',')) {
      value = Trim(value);
      if (!value.empty()) {
        bytes.push_back(static_cast<uint8_t>(ParseNumber(value)));
      }
    }

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
    return;
  }

  // FDB - Form Double Byte (16-bit, big-endian)
  if (dir_upper == directives::FDB) {
    std::vector<uint8_t> bytes;
    std::istringstream ops(operands);
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

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
    return;
  }

  // FCC - Form Constant Characters (flexible delimiter)
  if (dir_upper == directives::FCC) {
    std::string trimmed = Trim(operands);
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

    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    current_address_ += bytes.size();
    return;
  }

  // RMB - Reserve Memory Bytes
  if (dir_upper == directives::RMB) {
    uint32_t size = ParseNumber(operands);
    section.atoms.push_back(std::make_shared<SpaceAtom>(size));
    current_address_ += size;
    return;
  }

  // SETDP - Set Direct Page (assembler directive only)
  if (dir_upper == directives::SETDP) {
    direct_page_ = static_cast<uint8_t>(ParseNumber(operands));
    // SETDP produces no atoms, just informs assembler
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
