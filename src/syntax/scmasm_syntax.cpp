/**
 * @file scmasm_syntax.cpp
 * @brief S-C Macro Assembler (SCMASM) syntax parser implementation
 *
 * Implementation of SCMASM syntax parser for xasm++.
 * Phase 2: Integrated with shared ExpressionParser
 * Phase 6c.2: Handler extraction with free functions
 */

#include "xasm++/syntax/scmasm_syntax.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_plugin.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/directives/scmasm_directive_handlers.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace xasm {

namespace {

// Radix values for number parsing
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_DECIMAL = 10;
constexpr int RADIX_HEXADECIMAL = 16;

} // anonymous namespace

// ============================================================================
// SCMASMNumberParser Implementation
// ============================================================================

bool SCMASMNumberParser::TryParse(const std::string &token,
                                  int64_t &value) const {
  if (token.empty()) {
    return false;
  }

  // Hexadecimal: $
  if (token[0] == '$') {
    if (token.length() < 2) {
      return false;
    }

    std::string hex = token.substr(1);

    // Validate hex digits
    for (char c : hex) {
      if (!std::isxdigit(static_cast<unsigned char>(c))) {
        return false;
      }
    }

    try {
      value = std::stoll(hex, nullptr, RADIX_HEXADECIMAL);
      return true;
    } catch (...) {
      return false;
    }
  }

  // Binary: %
  if (token[0] == '%') {
    if (token.length() < 2) {
      return false;
    }

    std::string binary = token.substr(1);

    // Validate binary digits before removing separators
    for (char c : binary) {
      if (c != '0' && c != '1' && c != '.') {
        return false;
      }
    }

    // Remove . separators
    binary.erase(std::remove(binary.begin(), binary.end(), '.'), binary.end());

    if (binary.empty()) {
      return false;
    }

    try {
      value = std::stoll(binary, nullptr, RADIX_BINARY);
      return true;
    } catch (...) {
      return false;
    }
  }

  // ASCII character constant: delimiter followed by character
  // Delimiter determines high bit rule
  // Must be exactly 2 characters (delimiter + char)
  if (!std::isdigit(token[0]) && token.length() == 2) {
    char delimiter = token[0];
    char c = token[1];

    // Apply high-bit rule:
    // If delimiter ASCII < 0x27 (apostrophe '), high bit is SET
    // Otherwise, high bit is CLEAR
    uint8_t result = static_cast<uint8_t>(c);
    if (delimiter < 0x27) {
      result |= 0x80; // Set high bit
    } else {
      result &= 0x7F; // Clear high bit
    }

    value = result;
    return true;
  }

  // If not a digit and not a 2-char constant, not a valid number for us
  if (!std::isdigit(static_cast<unsigned char>(token[0]))) {
    return false;
  }

  // Validate decimal digits
  for (char c : token) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  // Decimal
  try {
    value = std::stoll(token, nullptr, RADIX_DECIMAL);
    return true;
  } catch (...) {
    return false;
  }
}

// ============================================================================
// Constructor
// ============================================================================

ScmasmSyntaxParser::ScmasmSyntaxParser()
    : current_address_(0), current_file_("<source>"), current_line_(0),
      cpu_(nullptr), in_macro_definition_(false), macro_invocation_depth_(0) {
  InitializeDirectiveRegistry();
}

void ScmasmSyntaxParser::InitializeDirectiveRegistry() {
  // Phase 6c.2: Use extracted free functions with directive name constants
  using namespace scmasm::directives;

  // .OR - Set origin address
  directive_registry_[OR] = scmasm::HandleOr;

  // .EQ - Define constant
  directive_registry_[EQ] = scmasm::HandleEq;

  // .SE - Set variable (redefinable)
  directive_registry_[SE] = scmasm::HandleSe;

  // .AS - ASCII string
  directive_registry_[AS] = scmasm::HandleAs;

  // .AT - ASCII text (high bit on last char)
  directive_registry_[AT] = scmasm::HandleAt;

  // .AZ - ASCII zero-terminated
  directive_registry_[AZ] = scmasm::HandleAz;

  // .DA / .DFB - Define byte(s)
  directive_registry_[DA] = scmasm::HandleDa;
  directive_registry_[DFB] = scmasm::HandleDa; // Alias

  // .HS - Hex string
  directive_registry_[HS] = scmasm::HandleHs;

  // .BS - Binary string
  directive_registry_[BS] = scmasm::HandleBs;

  // .MA - Begin macro definition
  directive_registry_[MA] = scmasm::HandleMa;

  // .ENDM / .EM - End macro definition
  directive_registry_[ENDM] = scmasm::HandleEndm;
  directive_registry_[EM] = scmasm::HandleEndm; // Alias

  // P0 Priority Directives (A2oSX Critical)
  directive_registry_[PS] = scmasm::HandlePs;       // Pascal string
  directive_registry_[INB] = scmasm::HandleInb;     // Include binary
  directive_registry_[LIST] = scmasm::HandleList;   // Listing control
  directive_registry_[DUMMY] = scmasm::HandleDummy; // Dummy section
  directive_registry_[OP] = scmasm::HandleOp;       // CPU operation mode

  // Phase 3: 100% Coverage Directives
  directive_registry_[CS] = scmasm::HandleCs; // C-string with escapes
  directive_registry_[CZ] = scmasm::HandleCz; // C-string zero-terminated
  directive_registry_[TF] = scmasm::HandleTf; // Text file/title metadata
  directive_registry_[EP] = scmasm::HandleEp; // Entry point
  directive_registry_[HX] = scmasm::HandleHx; // Hex nibble storage
  directive_registry_[TA] = scmasm::HandleTa; // Target address (no-op)
  directive_registry_[AC] = scmasm::HandleAc; // ASCII with prefix

  // Note: Control flow directives (.DO, .ELSE, .FIN, .LU, .ENDU) are NOT
  // registered here because they require special handling in ParseLine with
  // line skipping and nested scoping. They cannot be dispatched via the simple
  // registry pattern.
}

// ============================================================================
// CPU Plugin Configuration
// ============================================================================

void ScmasmSyntaxParser::SetCpu(CpuPlugin *cpu) { cpu_ = cpu; }

// ============================================================================
// Main Parse Function
// ============================================================================

void ScmasmSyntaxParser::Parse(const std::string &source, Section &section,
                               ConcreteSymbolTable &symbols) {
  // Split source into lines
  std::vector<std::string> lines;
  std::istringstream stream(source);
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  // Process lines
  current_line_ = 0;
  size_t line_idx = 0;

  while (line_idx < lines.size()) {
    current_line_ = line_idx + 1;
    line = lines[line_idx];

    // Strip line number (if present)
    line = StripLineNumber(line);

    // Strip comments
    line = StripComments(line);

    // Trim whitespace
    line = Trim(line);

    // Skip empty lines
    if (line.empty()) {
      line_idx++;
      continue;
    }

    // If we're in a macro definition, collect lines
    if (in_macro_definition_) {
      // Check for .EM or .ENDM
      std::string upper_line = line;
      std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(),
                     ::toupper);

      if (upper_line.find(".EM") == 0 || upper_line.find(".ENDM") == 0) {
        // End macro definition
        HandleEm();
        line_idx++;
        continue;
      }

      // Add line to macro body
      current_macro_body_.push_back(line);
      line_idx++;
      continue;
    }

    // Parse the line
    try {
      ParseLine(line, section, symbols, lines, line_idx);
    } catch (const std::exception &e) {
      throw std::runtime_error(FormatError(e.what()));
    }

    line_idx++;
  }

  // Check for unclosed macro definition
  if (in_macro_definition_) {
    throw std::runtime_error("Unclosed macro definition: " +
                             current_macro_name_);
  }
}

// ============================================================================
// Helper Functions
// ============================================================================

std::string ScmasmSyntaxParser::StripLineNumber(const std::string &line) {
  // SCMASM allows optional line numbers (0-65535) at start
  // Format: [0-9]+ followed by whitespace

  size_t pos = 0;

  // Skip leading whitespace
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }

  // Check if line starts with digits
  size_t digit_start = pos;
  while (pos < line.length() && std::isdigit(line[pos])) {
    pos++;
  }

  // If we found digits, verify they're followed by whitespace or end
  if (pos > digit_start) {
    if (pos < line.length() && !std::isspace(line[pos])) {
      // Digits followed by non-whitespace - not a line number
      return line;
    }

    // Valid line number - skip it and following whitespace
    while (pos < line.length() && std::isspace(line[pos])) {
      pos++;
    }

    return line.substr(pos);
  }

  return line;
}

std::string ScmasmSyntaxParser::StripComments(const std::string &line) {
  // Two comment styles:
  // 1. * in column 1 (full-line comment)
  // 2. ; anywhere (rest of line is comment)

  // Check for * in column 1 (after any leading whitespace)
  size_t first_non_space = line.find_first_not_of(" \t");
  if (first_non_space != std::string::npos && line[first_non_space] == '*') {
    return ""; // Entire line is comment
  }

  // Find semicolon comment
  size_t semicolon = line.find(';');
  if (semicolon != std::string::npos) {
    return line.substr(0, semicolon);
  }

  return line;
}

std::string ScmasmSyntaxParser::Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }

  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

std::string ScmasmSyntaxParser::FormatError(const std::string &message) const {
  std::ostringstream oss;
  oss << current_file_ << ":" << current_line_ << ": " << message;
  return oss.str();
}

// ============================================================================
// Line Parsing
// ============================================================================

void ScmasmSyntaxParser::ParseLine(const std::string &line, Section &section,
                                   ConcreteSymbolTable &symbols,
                                   const std::vector<std::string> &source,
                                   size_t &line_idx) {
  size_t pos = 0;

  // Parse label (if present)
  std::string label = ParseLabel(line, pos, section, symbols);

  // Skip whitespace after label
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }

  // If nothing left (just a label, nothing else), define it and create atom
  if (pos >= line.length()) {
    if (!label.empty()) {
      // Define label
      if (IsLocalLabel(label)) {
        local_labels_[label] = current_address_;
        // Local labels don't create label atoms
      } else {
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(label, SymbolType::Label, expr);

        // Create label atom for non-local labels
        auto label_atom = std::make_shared<LabelAtom>(label, current_address_);
        section.atoms.push_back(label_atom);
      }
    }
    return;
  }

  // Parse opcode/directive
  size_t opcode_start = pos;

  // For opcodes, stop at whitespace
  // For directives starting with ., stop at whitespace (directives are like
  // .OR, .EQ) But NOT for local labels like .1 which appear in operands
  while (pos < line.length() && !std::isspace(line[pos])) {
    pos++;
  }

  std::string opcode = line.substr(opcode_start, pos - opcode_start);

  // Skip whitespace after opcode
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }

  // Rest is operand
  std::string operand = line.substr(pos);
  operand = Trim(operand);

  // Convert opcode to uppercase for comparison
  std::string opcode_upper = opcode;
  std::transform(opcode_upper.begin(), opcode_upper.end(), opcode_upper.begin(),
                 ::toupper);

  // Handle directives (must start with .)
  if (!opcode.empty() && opcode[0] == '.') {
    // For directives, define the label but DON'T create a label atom
    // Exception: .OR creates a label atom before changing address
    if (!label.empty()) {
      if (opcode_upper == ".OR") {
        // Label before .OR - define it and create label atom at current address
        if (IsLocalLabel(label)) {
          local_labels_[label] = current_address_;
          // Local labels don't create label atoms
        } else {
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(label, SymbolType::Label, expr);

          // Create label atom for non-local labels
          auto label_atom =
              std::make_shared<LabelAtom>(label, current_address_);
          section.atoms.push_back(label_atom);
        }
      } else {
        // Other directives: just define the label, no atom
        if (IsLocalLabel(label)) {
          local_labels_[label] = current_address_;
        } else {
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(label, SymbolType::Label, expr);
        }
      }
    }

    // Special validation for directives that require labels
    if ((opcode_upper == ".EQ" || opcode_upper == ".SE") && label.empty()) {
      throw std::runtime_error(opcode_upper + " requires a label");
    }

    // Control flow directives require special handling (not in registry)
    using namespace scmasm::directives;
    if (opcode_upper == DO) {
      HandleDo(operand, section, symbols, source, line_idx);
    } else if (opcode_upper == LU) {
      HandleLu(operand, section, symbols, source, line_idx);
    } else if (opcode_upper == ELSE || opcode_upper == FIN ||
               opcode_upper == ENDU) {
      // These are handled by their opening directives (.DO, .LU)
      // If we encounter them here, they're mismatched
      throw std::runtime_error("Mismatched " + opcode_upper);
    } else {
      // Try to dispatch via registry
      auto it = directive_registry_.find(opcode_upper);
      if (it != directive_registry_.end()) {
        // Found in registry - dispatch with DirectiveContext
        DirectiveContext context;
        context.section = &section;
        context.symbols = &symbols;
        context.current_address = &current_address_;
        context.parser_state =
            this; // Phase 6c.2: Set parser for handler access
        context.current_file = current_file_;
        context.current_line = current_line_;
        it->second(label, operand, context);
      } else {
        // Not in registry and not a control flow directive
        throw std::runtime_error("Unknown directive: " + opcode);
      }
    }
  } else {
    // Not a directive - define label and create label atom for
    // instructions/macros
    if (!label.empty()) {
      if (IsLocalLabel(label)) {
        local_labels_[label] = current_address_;
        // Local labels don't create label atoms
      } else {
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(label, SymbolType::Label, expr);

        // Create label atom for non-local labels
        auto label_atom = std::make_shared<LabelAtom>(label, current_address_);
        section.atoms.push_back(label_atom);
      }
    }

    // Check if it's a macro invocation
    auto it = macros_.find(opcode_upper);
    if (it != macros_.end()) {
      // Parse macro parameters from operand
      std::vector<std::string> params;
      if (!operand.empty()) {
        // Split by comma
        size_t start = 0;
        size_t pos = 0;

        while (pos <= operand.length()) {
          if (pos == operand.length() || operand[pos] == ',') {
            std::string param = Trim(operand.substr(start, pos - start));
            if (!param.empty()) {
              params.push_back(param);
            }
            start = pos + 1;
          }
          ++pos;
        }
      }
      // Invoke the macro
      InvokeMacro(opcode_upper, params, section, symbols);
    } else {
      // Assume it's an assembly instruction (6502, 65C02, etc.)
      // Phase 3: We don't parse instructions yet, just store them as
      // InstructionAtom
      auto instr_atom =
          std::make_shared<InstructionAtom>(opcode_upper, operand);
      section.atoms.push_back(instr_atom);

      // Assume each instruction is 1-3 bytes (will be properly sized by CPU
      // plugin later) For now, advance by a conservative 3 bytes
      current_address_ += 3;
    }
  }
}

std::string ScmasmSyntaxParser::ParseLabel(const std::string &line, size_t &pos,
                                           Section & /*section*/,
                                           ConcreteSymbolTable & /*symbols*/) {
  // Skip leading whitespace
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }

  // Labels must start with letter or .
  if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '.')) {
    return "";
  }

  size_t label_start = pos;

  // Parse label characters (letter, digit, underscore, or .)
  while (pos < line.length() &&
         (std::isalnum(line[pos]) || line[pos] == '_' || line[pos] == '.')) {
    pos++;
  }

  std::string label = line.substr(label_start, pos - label_start);

  // Convert to uppercase for checking
  std::string label_upper = label;
  std::transform(label_upper.begin(), label_upper.end(), label_upper.begin(),
                 ::toupper);

  // Check if this is actually a directive (not a label)
  // Directives start with . and have letters after (e.g., .OR, .EQ, .SE)
  // Local labels start with . and have only digits (e.g., .1, .2)
  if (label.length() >= 2 && label[0] == '.') {
    bool has_letter = false;
    for (size_t i = 1; i < label.length(); i++) {
      if (std::isalpha(label[i])) {
        has_letter = true;
        break;
      }
    }
    // If it has letters after the dot, it's a directive, not a label
    if (has_letter) {
      pos = label_start;
      return "";
    }
  }

  // Check if this is a macro name (not a label)
  // If the token matches a defined macro, it's a macro invocation
  if (macros_.find(label_upper) != macros_.end()) {
    pos = label_start;
    return "";
  }

  // Check if this is a known opcode (not a label)
  // Query CPU plugin for real opcodes, or check pseudo-ops
  if (cpu_ != nullptr && cpu_->HasOpcode(label_upper)) {
    // This is a CPU opcode, not a label
    pos = label_start;
    return "";
  }

  // Check for pseudo-ops (not real CPU opcodes, but assembler directives)
  // These are common mnemonics that define data/storage
  static const std::unordered_set<std::string> pseudo_ops = {"DB", "DW", "DS"};
  if (pseudo_ops.find(label_upper) != pseudo_ops.end()) {
    // This is a pseudo-op, not a label
    pos = label_start;
    return "";
  }

  // Label must be followed by whitespace or colon (optional)
  if (pos < line.length() && !std::isspace(line[pos])) {
    // Not a label - backtrack
    pos = label_start;
    return "";
  }

  return label;
}

// ============================================================================
// Directive Handlers
// ============================================================================

void ScmasmSyntaxParser::HandleOr(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  if (operand.empty()) {
    throw std::runtime_error(".OR requires an address");
  }

  // Parse address (with expression evaluation)
  uint32_t address = EvaluateExpression(operand, symbols);

  // Create ORG atom
  auto org_atom = std::make_shared<OrgAtom>(address);
  section.atoms.push_back(org_atom);

  // Update current address
  current_address_ = address;
}

void ScmasmSyntaxParser::HandleEq(const std::string &label,
                                  const std::string &operand,
                                  ConcreteSymbolTable &symbols) {
  if (operand.empty()) {
    throw std::runtime_error(".EQ requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Define symbol (immutable) - .EQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  symbols.Define(label, SymbolType::Equate, expr);
}

void ScmasmSyntaxParser::HandleSe(const std::string &label,
                                  const std::string &operand,
                                  ConcreteSymbolTable &symbols) {
  if (operand.empty()) {
    throw std::runtime_error(".SE requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Check if symbol already exists (SE allows redefinition)
  int64_t existing_value;
  auto expr = std::make_shared<LiteralExpr>(value);

  if (symbols.Lookup(label, existing_value)) {
    // Redefine it - .SE creates Set type (redefinable)
    symbols.Define(label, SymbolType::Set, expr);
  } else {
    // First definition - .SE creates Set type (redefinable)
    symbols.Define(label, SymbolType::Set, expr);
    variable_symbols_[label] = true;
  }
}

// ============================================================================
// Number Parsing
// ============================================================================

uint32_t ScmasmSyntaxParser::ParseNumber(const std::string &str) {
  std::string trimmed = Trim(str);

  if (trimmed.empty()) {
    throw std::runtime_error("Empty number string");
  }

  // Hexadecimal: $
  if (trimmed[0] == '$') {
    std::string hex = trimmed.substr(1);

    if (hex.empty()) {
      throw std::runtime_error("Invalid hex number (no digits after $): " +
                               trimmed);
    }

    // Validate hex digits
    for (char c : hex) {
      if (!std::isxdigit(static_cast<unsigned char>(c))) {
        throw std::runtime_error("Invalid hex digit '" + std::string(1, c) +
                                 "' in: " + trimmed);
      }
    }

    try {
      return std::stoul(hex, nullptr, RADIX_HEXADECIMAL);
    } catch (const std::exception &e) {
      throw std::runtime_error("Failed to parse hex number '" + trimmed +
                               "': " + e.what());
    }
  }

  // Binary: %
  if (trimmed[0] == '%') {
    std::string binary = trimmed.substr(1);

    if (binary.empty()) {
      throw std::runtime_error("Invalid binary number (no digits after %): " +
                               trimmed);
    }

    // Validate binary digits before removing separators
    for (char c : binary) {
      if (c != '0' && c != '1' && c != '.') {
        throw std::runtime_error("Invalid binary digit '" + std::string(1, c) +
                                 "' in: " + trimmed);
      }
    }

    // Remove . separators
    binary.erase(std::remove(binary.begin(), binary.end(), '.'), binary.end());

    if (binary.empty()) {
      throw std::runtime_error("Binary number has no digits: " + trimmed);
    }

    try {
      return std::stoul(binary, nullptr, RADIX_BINARY);
    } catch (const std::exception &e) {
      throw std::runtime_error("Failed to parse binary number '" + trimmed +
                               "': " + e.what());
    }
  }

  // ASCII character constant: delimiter followed by character
  // Delimiter determines high bit rule
  // Must be exactly 2 characters (delimiter + char)
  if (!std::isdigit(trimmed[0]) && trimmed.length() == 2) {
    char delimiter = trimmed[0];
    char c = trimmed[1];
    return ApplyHighBitRule(c, delimiter);
  }

  // If not a digit and not a 2-char constant, it's not a valid number
  if (!std::isdigit(static_cast<unsigned char>(trimmed[0]))) {
    throw std::runtime_error("Not a valid number: " + trimmed);
  }

  // Validate decimal digits
  for (char c : trimmed) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw std::runtime_error("Invalid decimal digit '" + std::string(1, c) +
                               "' in: " + trimmed);
    }
  }

  // Decimal
  try {
    return std::stoul(trimmed, nullptr, RADIX_DECIMAL);
  } catch (const std::exception &e) {
    throw std::runtime_error("Failed to parse decimal number '" + trimmed +
                             "': " + e.what());
  }
}

bool ScmasmSyntaxParser::IsLocalLabel(const std::string &label) {
  // Local labels are .0-.9
  if (label.length() == 2 && label[0] == '.' && label[1] >= '0' &&
      label[1] <= '9') {
    return true;
  }
  return false;
}

uint32_t ScmasmSyntaxParser::EvaluateExpression(const std::string &str,
                                                ConcreteSymbolTable &symbols) {
  std::string trimmed = Trim(str);

  // Handle * (current address) - special case before general parsing
  if (trimmed == "*") {
    return current_address_;
  }

  // Handle local labels (.0-.9) - check if entire expression is a local label
  if (IsLocalLabel(trimmed)) {
    auto it = local_labels_.find(trimmed);
    if (it != local_labels_.end()) {
      return it->second;
    } else {
      throw std::runtime_error("Undefined local label: " + trimmed);
    }
  }

  // Handle SCMASM-specific number formats that ExpressionParser doesn't
  // support:
  // 1. Binary with . separators: %1111.0000
  // 2. Character constants: 'A, "A, #A, /A, etc.
  //
  // These are detected by specific prefixes/patterns
  if (!trimmed.empty()) {
    // Binary with potential . separators
    if (trimmed[0] == '%' && trimmed.find('.') != std::string::npos) {
      try {
        return ParseNumber(trimmed);
      } catch (...) {
        // Fall through to ExpressionParser
      }
    }

    // Character constant: single non-alphanumeric followed by a character
    // (delimiter + char, e.g., 'A, "A, #A, /A)
    if (trimmed.length() == 2 && !std::isalnum(trimmed[0]) &&
        trimmed[0] != '$' && trimmed[0] != '%') {
      try {
        return ParseNumber(trimmed);
      } catch (...) {
        // Fall through to ExpressionParser
      }
    }
  }

  // Phase 2: Use shared ExpressionParser for all other expressions
  // This handles:
  // - Numbers ($hex without dots, %binary without dots, decimal)
  // - Symbol references
  // - Binary operators (+, -, *, /, %, <<, >>, &, |, ^)
  // - Comparison operators (==, !=, <, >, <=, >=)
  // - Logical operators (&&, ||)
  // - Unary operators (!, ~, +, -)
  // - Parentheses for grouping
  auto expr = ParseExpression(trimmed, symbols);
  return static_cast<uint32_t>(expr->Evaluate(symbols));
}

uint8_t ScmasmSyntaxParser::ApplyHighBitRule(char c, char delimiter) {
  // SCMASM high-bit rule:
  // If delimiter ASCII < 0x27 (apostrophe '), high bit is SET
  // Otherwise, high bit is CLEAR

  uint8_t result = static_cast<uint8_t>(c);

  if (delimiter < 0x27) {
    // Set high bit
    result |= 0x80;
  } else {
    // Clear high bit
    result &= 0x7F;
  }

  return result;
}

std::shared_ptr<Expression>
ScmasmSyntaxParser::ParseExpression(const std::string &str,
                                    ConcreteSymbolTable &symbols) {
  // Phase 2: Use shared ExpressionParser with SCMASM number parser
  ExpressionParser parser(&symbols, &scmasm_number_parser_);
  return parser.Parse(str);
}

// ============================================================================
// Phase 2: String & Data Directives Implementation
// ============================================================================

char ScmasmSyntaxParser::ParseString(const std::string &operand,
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

  // Extract string content (between delimiters)
  for (size_t i = 1; i < end; ++i) {
    char c = trimmed[i];
    uint8_t byte = ApplyHighBitRule(c, delimiter);
    result.push_back(byte);
  }

  return delimiter;
}

void ScmasmSyntaxParser::HandleAs(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /*symbols*/) {
  std::vector<uint8_t> data;
  ParseString(operand, data);

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleAt(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /*symbols*/) {
  std::vector<uint8_t> data;
  ParseString(operand, data);

  // Set high bit on LAST character
  if (!data.empty()) {
    data.back() |= 0x80;
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleAz(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /*symbols*/) {
  std::vector<uint8_t> data;
  ParseString(operand, data);

  // Add null terminator
  data.push_back(0x00);

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleDa(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
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
      uint32_t num = EvaluateExpression(expr, symbols);
      data.push_back(static_cast<uint8_t>(num & 0xFF));
    } else if (prefix == '/') {
      // 8-bit: second byte (bits 8-15)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, symbols);
      data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
    } else if (prefix == '<') {
      // 24-bit: three bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, symbols);
      data.push_back(static_cast<uint8_t>(num & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 16) & 0xFF));
    } else if (prefix == '>') {
      // 32-bit: four bytes (little-endian)
      expr = Trim(value_trimmed.substr(1));
      uint32_t num = EvaluateExpression(expr, symbols);
      data.push_back(static_cast<uint8_t>(num & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 16) & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 24) & 0xFF));
    } else {
      // DEFAULT: 16-bit (little-endian)
      uint32_t num = EvaluateExpression(value_trimmed, symbols);
      data.push_back(static_cast<uint8_t>(num & 0xFF));
      data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
    }
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);

  // Update address counter
  current_address_ += data.size();
}

void ScmasmSyntaxParser::HandleHs(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /*symbols*/) {
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
  if (hex_digits.length() % 2 != 0) {
    throw std::runtime_error(".HS requires even number of hex digits");
  }

  // Convert pairs to bytes
  for (size_t i = 0; i < hex_digits.length(); i += 2) {
    std::string byte_str = hex_digits.substr(i, 2);
    uint8_t byte =
        static_cast<uint8_t>(std::stoi(byte_str, nullptr, RADIX_HEXADECIMAL));
    data.push_back(byte);
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleBs(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  // .BS (Block Storage) - Reserve N bytes of space
  // SCMASM syntax: .BS count
  // Where count is a decimal or hex number ($hex, %binary)
  // This reserves 'count' bytes filled with zeros

  if (operand.empty()) {
    throw std::runtime_error(".BS requires a byte count");
  }

  std::string trimmed = Trim(operand);

  // Evaluate the byte count expression (supports symbols, hex, decimal)
  uint32_t byte_count = EvaluateExpression(trimmed, symbols);

  // Validate byte count (reasonable limit: 64KB)
  if (byte_count > 65536) {
    throw std::runtime_error(".BS byte count too large (max 65536)");
  }

  // Create data filled with zeros
  std::vector<uint8_t> data(byte_count, 0x00);

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);

  // Update address counter
  current_address_ += byte_count;
}

// ============================================================================
// Phase 3: Macros, Conditionals, Local Labels, Loops
// ============================================================================

void ScmasmSyntaxParser::HandleMa(const std::string &label,
                                  const std::string &operand) {
  // Macro name can come from label or operand
  std::string macro_name;

  if (!label.empty()) {
    macro_name = label;
  } else if (!operand.empty()) {
    macro_name = Trim(operand);
  } else {
    throw std::runtime_error(".MA requires a macro name");
  }

  // Start macro definition
  in_macro_definition_ = true;
  current_macro_name_ = macro_name;
  current_macro_body_.clear();
}

void ScmasmSyntaxParser::HandleEm() {
  if (!in_macro_definition_) {
    throw std::runtime_error(".EM without matching .MA");
  }

  // Store the macro
  MacroDef macro;
  macro.name = current_macro_name_;
  macro.lines = current_macro_body_;

  macros_[current_macro_name_] = macro;

  // Reset state
  in_macro_definition_ = false;
  current_macro_name_.clear();
  current_macro_body_.clear();
}

void ScmasmSyntaxParser::InvokeMacro(const std::string &name,
                                     const std::vector<std::string> &params,
                                     Section &section,
                                     ConcreteSymbolTable &symbols) {
  // Check for infinite recursion
  if (macro_invocation_depth_ >= 63) {
    throw std::runtime_error("Macro nesting too deep (max 63 levels)");
  }

  // Find macro
  auto it = macros_.find(name);
  if (it == macros_.end()) {
    throw std::runtime_error("Undefined macro: " + name);
  }

  const MacroDef &macro = it->second;

  // Expand macro body - substitute parameters first
  std::vector<std::string> expanded_lines;
  for (const auto &line : macro.lines) {
    std::string expanded = SubstituteParameters(line, params);
    expanded_lines.push_back(expanded);
  }

  // Parse expanded lines
  macro_invocation_depth_++;

  size_t line_idx = 0;
  while (line_idx < expanded_lines.size()) {
    try {
      ParseLine(expanded_lines[line_idx], section, symbols, expanded_lines,
                line_idx);
    } catch (const std::exception &e) {
      macro_invocation_depth_--;
      throw std::runtime_error(std::string("In macro ") + name + ": " +
                               e.what());
    }
    line_idx++;
  }

  macro_invocation_depth_--;
}

std::string ScmasmSyntaxParser::SubstituteParameters(
    const std::string &line, const std::vector<std::string> &params) {
  std::string result;
  size_t pos = 0;

  while (pos < line.length()) {
    if (line[pos] == '\\' && pos + 1 < line.length()) {
      char next = line[pos + 1];
      if (next >= '0' && next <= '9') {
        // Parameter reference
        int param_idx = next - '0';
        if (param_idx < static_cast<int>(params.size())) {
          result += params[param_idx];
        }
        // If parameter not provided, substitute with empty string
        pos += 2;
        continue;
      }
    }
    result += line[pos];
    pos++;
  }

  return result;
}

void ScmasmSyntaxParser::HandleDo(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols,
                                  const std::vector<std::string> &source,
                                  size_t &line_idx) {
  // Evaluate condition
  uint32_t condition = EvaluateExpression(operand, symbols);
  // Find matching .ELSE or .FIN
  size_t else_line = std::string::npos;
  size_t fin_line = std::string::npos;
  int nesting = 1;

  for (size_t i = line_idx + 1; i < source.size(); ++i) {
    std::string line = source[i];
    line = StripLineNumber(line);
    line = StripComments(line);
    line = Trim(line);

    if (line.empty())
      continue;

    // Extract first token (might be label + directive)
    size_t token_pos = 0;

    // Skip label if present
    while (token_pos < line.length() && !std::isspace(line[token_pos])) {
      token_pos++;
    }

    // Skip whitespace
    while (token_pos < line.length() && std::isspace(line[token_pos])) {
      token_pos++;
    }

    // Get the actual directive/opcode
    size_t directive_start = token_pos;
    while (token_pos < line.length() && !std::isspace(line[token_pos])) {
      token_pos++;
    }

    std::string directive =
        line.substr(directive_start, token_pos - directive_start);
    std::transform(directive.begin(), directive.end(), directive.begin(),
                   ::toupper);

    // Check if the first token is .DO/.ELSE/.FIN
    std::string first_token = line.substr(0, line.find(' '));
    std::transform(first_token.begin(), first_token.end(), first_token.begin(),
                   ::toupper);

    using namespace scmasm::directives;
    if (first_token == DO || directive == DO) {
      nesting++;
    } else if ((first_token == ELSE || directive == ELSE) && nesting == 1) {
      else_line = i;
    } else if (first_token == FIN || directive == FIN) {
      nesting--;
      if (nesting == 0) {
        fin_line = i;
        break;
      }
    }
  }

  if (fin_line == std::string::npos) {
    throw std::runtime_error(".DO without matching .FIN");
  }

  // Process appropriate block
  size_t start_line, end_line;

  if (condition != 0) {
    // Process .DO block
    start_line = line_idx + 1;
    end_line = (else_line != std::string::npos) ? else_line : fin_line;
  } else {
    // Process .ELSE block (if exists)
    if (else_line != std::string::npos) {
      start_line = else_line + 1;
      end_line = fin_line;
    } else {
      // No .ELSE, skip entire block
      line_idx = fin_line;
      return;
    }
  }

  // Process lines in selected block
  for (size_t i = start_line; i < end_line;) {
    current_line_ = i + 1;
    std::string line = source[i];
    line = StripLineNumber(line);
    line = StripComments(line);
    line = Trim(line);

    if (!line.empty()) {
      size_t temp_idx = i;
      ParseLine(line, section, symbols, source, temp_idx);
      // If ParseLine skipped lines (e.g., nested .DO/.FIN), respect that
      i = temp_idx + 1;
    } else {
      i++;
    }
  }

  // Skip to after .FIN
  line_idx = fin_line;
}

void ScmasmSyntaxParser::HandleLu(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols,
                                  const std::vector<std::string> &source,
                                  size_t &line_idx) {
  // Evaluate loop count
  uint32_t count = EvaluateExpression(operand, symbols);

  // Find matching .ENDU
  size_t endu_line = std::string::npos;
  int nesting = 1;

  for (size_t i = line_idx + 1; i < source.size(); ++i) {
    std::string line = source[i];
    line = StripLineNumber(line);
    line = StripComments(line);
    line = Trim(line);

    std::string upper = line;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    using namespace scmasm::directives;
    if (upper.find(LU) == 0) {
      nesting++;
    } else if (upper.find(ENDU) == 0) {
      nesting--;
      if (nesting == 0) {
        endu_line = i;
        break;
      }
    }
  }

  if (endu_line == std::string::npos) {
    throw std::runtime_error(".LU without matching .ENDU");
  }

  size_t loop_start = line_idx + 1;
  size_t loop_end = endu_line;

  // Execute loop count times
  for (uint32_t iteration = 0; iteration < count; ++iteration) {
    for (size_t i = loop_start; i < loop_end;) {
      current_line_ = i + 1;
      std::string line = source[i];
      line = StripLineNumber(line);
      line = StripComments(line);
      line = Trim(line);

      if (!line.empty()) {
        size_t temp_idx = i;
        ParseLine(line, section, symbols, source, temp_idx);
        // If ParseLine skipped lines (e.g., nested .LU/.ENDU), respect that
        i = temp_idx + 1;
      } else {
        i++;
      }
    }
  }

  // Skip to after .ENDU
  line_idx = endu_line;
}

// ============================================================================
// Directive Registry
// ============================================================================

} // namespace xasm
