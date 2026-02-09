/**
 * @file flex_syntax.cpp
 * @brief FLEX ASM09 assembly syntax parser implementation
 *
 * Implementation of the FLEX ASM09 assembler syntax for Motorola 6809.
 */

#include "xasm++/syntax/flex_syntax.h"
#include "xasm++/parse_utils.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace xasm {

// ============================================================================
// Helper Functions
// ============================================================================

std::string FlexAsmSyntax::Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t");
  return str.substr(start, end - start + 1);
}

std::string FlexAsmSyntax::ToUpper(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

std::string FlexAsmSyntax::StripComments(const std::string &line) {
  size_t comment_pos = line.find(';');
  if (comment_pos != std::string::npos) {
    return line.substr(0, comment_pos);
  }
  return line;
}

bool FlexAsmSyntax::IsCommentLine(const std::string &line) {
  std::string trimmed = Trim(line);
  return !trimmed.empty() && trimmed[0] == '*';
}

uint32_t FlexAsmSyntax::ParseNumber(const std::string &str) {
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

// ============================================================================
// Directive Parsing
// ============================================================================

void FlexAsmSyntax::ParseDirective(const std::string &directive,
                                   const std::string &operands,
                                   const std::string &label, Section &section,
                                   ConcreteSymbolTable &symbols) {
  std::string dir_upper = ToUpper(directive);

  // ORG - Set origin address
  if (dir_upper == "ORG") {
    uint32_t address = ParseNumber(operands);
    // Note: Not creating OrgAtom for now to match test expectations
    // TODO: Revisit this - should ORG create an atom?
    // section.atoms.push_back(std::make_shared<OrgAtom>(address));
    current_address_ = address;
    return;
  }

  // END - End of assembly
  if (dir_upper == "END") {
    // END can have optional entry point, but we don't create an atom
    return;
  }

  // EQU - Equate (constant symbol)
  if (dir_upper == "EQU") {
    if (label.empty()) {
      throw std::runtime_error("EQU requires a label");
    }
    uint32_t value = ParseNumber(operands);
    symbols.DefineLabel(label, static_cast<int64_t>(value));
    return;
  }

  // SET - Set (variable symbol)
  if (dir_upper == "SET") {
    if (label.empty()) {
      throw std::runtime_error("SET requires a label");
    }
    uint32_t value = ParseNumber(operands);
    // SET allows redefinition, so we define/redefine
    symbols.DefineLabel(label, static_cast<int64_t>(value));
    return;
  }

  // FCB - Form Constant Byte
  if (dir_upper == "FCB") {
    std::vector<uint8_t> data;
    std::istringstream iss(operands);
    std::string value_str;

    while (std::getline(iss, value_str, ',')) {
      value_str = Trim(value_str);
      if (!value_str.empty()) {
        uint32_t value = ParseNumber(value_str);
        data.push_back(static_cast<uint8_t>(value & 0xFF));
      }
    }

    if (!data.empty()) {
      section.atoms.push_back(std::make_shared<DataAtom>(data));
    }
    return;
  }

  // FDB - Form Double Byte (16-bit big-endian)
  if (dir_upper == "FDB") {
    std::vector<uint8_t> data;
    std::istringstream iss(operands);
    std::string value_str;

    while (std::getline(iss, value_str, ',')) {
      value_str = Trim(value_str);
      if (!value_str.empty()) {
        uint32_t value = ParseNumber(value_str);
        // Big-endian (MSB first)
        data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(value & 0xFF));
      }
    }

    if (!data.empty()) {
      section.atoms.push_back(std::make_shared<DataAtom>(data));
    }
    return;
  }

  // FCC - Form Constant Characters
  if (dir_upper == "FCC") {
    std::string trimmed = Trim(operands);
    if (trimmed.length() < 2) {
      throw std::runtime_error("FCC requires delimited string");
    }

    char delimiter = trimmed[0];
    size_t end_pos = trimmed.find(delimiter, 1);
    if (end_pos == std::string::npos) {
      throw std::runtime_error("FCC missing closing delimiter");
    }

    std::string str_content = trimmed.substr(1, end_pos - 1);
    std::vector<uint8_t> data(str_content.begin(), str_content.end());
    section.atoms.push_back(std::make_shared<DataAtom>(data));
    return;
  }

  // RMB - Reserve Memory Bytes
  if (dir_upper == "RMB") {
    uint32_t count = ParseNumber(operands);
    section.atoms.push_back(std::make_shared<SpaceAtom>(count));
    return;
  }

  // SETDP - Set Direct Page
  if (dir_upper == "SETDP") {
    uint32_t value = ParseNumber(operands);
    direct_page_ = static_cast<uint8_t>(value & 0xFF);
    // SETDP is assembler directive, doesn't create atom
    return;
  }

  // NAM/TTL - Set program title
  if (dir_upper == "NAM" || dir_upper == "TTL") {
    title_ = Trim(operands);
    // Listing control directive, no atom
    return;
  }

  // STTL - Set subtitle
  if (dir_upper == "STTL") {
    subtitle_ = Trim(operands);
    // Listing control directive, no atom
    return;
  }

  // PAGE - Form feed
  if (dir_upper == "PAGE") {
    // Listing control directive, no atom
    return;
  }

  // SPC - Space lines
  if (dir_upper == "SPC") {
    // Listing control directive, no atom
    return;
  }

  // IFC - Conditional assembly (If Condition)
  if (dir_upper == "IFC") {
    // Check if we should evaluate condition or just track nesting
    bool should_evaluate = ShouldAssemble();
    
    if (should_evaluate) {
      // Evaluate condition and push result onto stack
      bool condition_result = EvaluateCondition(operands);
      conditional_stack_.push(condition_result);
      
      // If condition is false AND we're currently assembling, enter skipping mode
      if (!condition_result) {
        mode_ = ParserMode::SkippingConditional;
      }
    } else {
      // We're already skipping, just push false to track nesting
      conditional_stack_.push(false);
    }
    
    return;
  }

  // ENDC - End conditional assembly
  if (dir_upper == "ENDC") {
    if (conditional_stack_.empty()) {
      throw std::runtime_error("ENDC without matching IFC");
    }
    
    // Pop condition from stack
    conditional_stack_.pop();
    
    // Exit skipping mode if stack is now empty or top is true
    if (mode_ == ParserMode::SkippingConditional) {
      if (conditional_stack_.empty() || conditional_stack_.top()) {
        mode_ = ParserMode::Normal;
      }
    }
    
    return;
  }

  // MACRO - Begin macro definition
  if (dir_upper == "MACRO") {
    if (mode_ != ParserMode::Normal) {
      throw std::runtime_error("Nested MACRO not allowed");
    }
    
    // Enter macro definition mode
    mode_ = ParserMode::InMacroDefinition;
    
    // Set up current macro
    current_macro_.name = label;
    current_macro_.parameters.clear();
    current_macro_.body.clear();
    current_macro_.definition_line = 0; // TODO: Track line numbers
    
    // Parse parameters from operands (comma-separated)
    if (!operands.empty()) {
      std::istringstream iss(operands);
      std::string param;
      while (std::getline(iss, param, ',')) {
        param = Trim(param);
        if (!param.empty()) {
          current_macro_.parameters.push_back(param);
        }
      }
    }
    
    return;
  }

  // ENDM - End macro definition
  if (dir_upper == "ENDM") {
    if (mode_ != ParserMode::InMacroDefinition) {
      throw std::runtime_error("ENDM without MACRO");
    }
    
    // Store the macro in MacroProcessor
    macro_processor_.DefineMacro(current_macro_.name,
                                 current_macro_.parameters,
                                 current_macro_.body);
    
    // Also store in local macros_ map for compatibility
    std::string name_upper = ToUpper(current_macro_.name);
    macros_[name_upper] = current_macro_;
    
    // Return to normal mode
    mode_ = ParserMode::Normal;
    current_macro_ = MacroDefinition{};
    
    return;
  }

  throw std::runtime_error("Unknown directive: " + directive);
}

// ============================================================================
// Line Parsing
// ============================================================================

void FlexAsmSyntax::ParseLine(const std::string &line, Section &section,
                              ConcreteSymbolTable &symbols) {
  // Strip inline comments
  std::string cleaned = StripComments(line);

  // Skip blank lines and comment lines (but not during macro definition or conditional skipping)
  if (mode_ != ParserMode::InMacroDefinition && mode_ != ParserMode::SkippingConditional) {
    if (cleaned.empty() || IsCommentLine(cleaned)) {
      return;
    }
  }

  std::string trimmed = Trim(cleaned);
  if (mode_ != ParserMode::InMacroDefinition && mode_ != ParserMode::SkippingConditional && trimmed.empty()) {
    return;
  }
  
  // If we're in conditional skipping mode, only look for IFC/ENDC directives
  if (mode_ == ParserMode::SkippingConditional) {
    // Parse just enough to find IFC/ENDC directives for nesting
    std::string opcode;
    
    // Extract opcode (handle both label+opcode and opcode-only lines)
    if (!std::isspace(static_cast<unsigned char>(line[0]))) {
      // Label present, skip to opcode
      size_t space_pos = trimmed.find_first_of(" \t");
      if (space_pos != std::string::npos) {
        std::string rest = Trim(trimmed.substr(space_pos));
        size_t next_space = rest.find_first_of(" \t");
        if (next_space != std::string::npos) {
          opcode = rest.substr(0, next_space);
        } else {
          opcode = rest;
        }
      }
    } else {
      // No label
      std::string rest = Trim(trimmed);
      size_t space_pos = rest.find_first_of(" \t");
      if (space_pos != std::string::npos) {
        opcode = rest.substr(0, space_pos);
      } else {
        opcode = rest;
      }
    }
    
    std::string opcode_upper = ToUpper(opcode);
    
    // Only process IFC/ENDC while skipping
    if (opcode_upper == "IFC" || opcode_upper == "ENDC") {
      // Extract operands for IFC
      std::string operands;
      size_t opcode_pos = trimmed.find(opcode);
      if (opcode_pos != std::string::npos) {
        std::string after_opcode = trimmed.substr(opcode_pos + opcode.length());
        operands = Trim(after_opcode);
      }
      
      // Let ParseDirective handle it
      ParseDirective(opcode, operands, "", section, symbols);
    }
    
    // Skip all other lines while in conditional skipping mode
    return;
  }
  
  // If we're in macro definition mode, collect body lines
  if (mode_ == ParserMode::InMacroDefinition) {
    // Check if this is the ENDM directive
    std::string opcode;
    size_t space_pos = trimmed.find_first_of(" \t");
    if (space_pos != std::string::npos) {
      opcode = trimmed.substr(0, space_pos);
    } else {
      opcode = trimmed;
    }
    
    std::string opcode_upper = ToUpper(opcode);
    if (opcode_upper == "ENDM") {
      // Let ParseDirective handle ENDM
      // Continue with normal parsing
    } else {
      // Add this line to macro body
      current_macro_.body.push_back(line); // Use original line, not cleaned
      return;
    }
  }

  // Parse line into fields: label, opcode, operands
  std::string label, opcode, operands;

  // FLEX syntax: label starts in column 1, opcode has leading whitespace
  // For simplicity, we'll use a simpler approach: first token is label if
  // line doesn't start with whitespace, else it's opcode

  if (!std::isspace(static_cast<unsigned char>(line[0]))) {
    // Label present
    size_t space_pos = trimmed.find_first_of(" \t");
    if (space_pos != std::string::npos) {
      label = trimmed.substr(0, space_pos);
      std::string rest = Trim(trimmed.substr(space_pos));

      space_pos = rest.find_first_of(" \t");
      if (space_pos != std::string::npos) {
        opcode = rest.substr(0, space_pos);
        operands = Trim(rest.substr(space_pos));
      } else {
        opcode = rest;
      }
    } else {
      // Just a label
      label = trimmed;
    }
  } else {
    // No label, starts with whitespace
    std::string rest = Trim(trimmed);
    size_t space_pos = rest.find_first_of(" \t");
    if (space_pos != std::string::npos) {
      opcode = rest.substr(0, space_pos);
      operands = Trim(rest.substr(space_pos));
    } else {
      opcode = rest;
    }
  }

  // If we have an opcode, check if it's a directive or macro
  if (!opcode.empty()) {
    // Check if it's a directive (directives are handled specially)
    std::string opcode_upper = ToUpper(opcode);

    // List of known directives
    static const std::unordered_set<std::string> directives = {
        "ORG",   "END",   "EQU", "SET",  "FCB",  "FDB",  "FCC",
        "RMB",   "SETDP", "NAM", "TTL",  "STTL", "PAGE", "SPC",
        "MACRO", "ENDM",  "IFC", "ENDC", "RPT",  "ENDR"};

    // Directives that define symbols but don't create label atoms
    static const std::unordered_set<std::string> symbol_directives = {"EQU",
                                                                      "SET"};

    if (directives.find(opcode_upper) != directives.end()) {
      // For non-symbol-defining directives, create label atom BEFORE directive
      if (!label.empty() &&
          symbol_directives.find(opcode_upper) == symbol_directives.end()) {
        section.atoms.push_back(
            std::make_shared<LabelAtom>(label, current_address_));
        symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
      }

      // Handle directive
      ParseDirective(opcode, operands, label, section, symbols);
    } else if (macro_processor_.IsMacro(opcode_upper)) {
      // It's a macro invocation - expand and parse
      
      // Parse arguments from operands (comma-separated)
      std::vector<std::string> arguments;
      if (!operands.empty()) {
        std::istringstream iss(operands);
        std::string arg;
        while (std::getline(iss, arg, ',')) {
          arg = Trim(arg);
          if (!arg.empty()) {
            arguments.push_back(arg);
          }
        }
      }
      
      // Expand the macro
      std::vector<std::string> expanded_lines = macro_processor_.ExpandMacro(opcode_upper, arguments);
      
      // Parse each expanded line recursively
      for (const std::string &expanded_line : expanded_lines) {
        ParseLine(expanded_line, section, symbols);
      }
      
      // Create label atom if label is present (before macro expansion)
      if (!label.empty()) {
        section.atoms.push_back(
            std::make_shared<LabelAtom>(label, current_address_));
        symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
      }
    } else {
      // It's an instruction - create a placeholder data atom for now
      // Full instruction parsing will be implemented in a later phase
      
      // Create label atom if label is present (before instruction)
      if (!label.empty()) {
        section.atoms.push_back(
            std::make_shared<LabelAtom>(label, current_address_));
        symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
      }
      
      // Create placeholder data atom for instruction
      // For testing purposes, we'll create a 2-byte data atom
      std::vector<uint8_t> dummy_data = {0x00, 0x00};
      section.atoms.push_back(std::make_shared<DataAtom>(dummy_data));
      current_address_ += 2;
    }
  } else if (!label.empty()) {
    // Label only (no opcode) - create label atom
    section.atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address_));
    symbols.DefineLabel(label, static_cast<int64_t>(current_address_));
  }
}

// ============================================================================
// Main Parse Function
// ============================================================================

void FlexAsmSyntax::Parse(const std::string &source, Section &section,
                          ConcreteSymbolTable &symbols) {
  // Store symbol table pointer for conditional evaluation
  current_symbols_ = &symbols;
  
  // Split source into lines
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    ParseLine(line, section, symbols);
  }
  
  // Check for unclosed conditionals
  if (!conditional_stack_.empty()) {
    throw std::runtime_error("IFC without matching ENDC");
  }
  
  // Clear symbol table pointer
  current_symbols_ = nullptr;
}

// ============================================================================
// Macro Processor Stubs (Phase 2)
// ============================================================================

bool FlexAsmSyntax::IsMacro(const std::string &name) const {
  std::string name_copy = name; // Need non-const for ToUpper
  std::transform(name_copy.begin(), name_copy.end(), name_copy.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return macros_.find(name_copy) != macros_.end();
}

bool FlexAsmSyntax::IsMacroDefined(const std::string &name) const {
  return macros_.find(name) != macros_.end();
}

std::vector<std::string>
FlexAsmSyntax::ExpandMacro(const std::string &name,
                           const std::vector<std::string> &arguments) {
  // Find the macro definition
  auto it = macros_.find(name);
  if (it == macros_.end()) {
    return {}; // Macro not found
  }
  
  const MacroDefinition &macro = it->second;
  std::vector<std::string> result;
  
  // Generate unique expansion ID
  int expansion_id = ++expansion_counter_;
  
  // Process each line in the macro body
  for (const std::string &line : macro.body) {
    // Step 1: Substitute parameters with arguments
    std::string expanded = SubstituteParameters(line, macro, arguments);
    
    // Step 2: Make local labels unique
    // Find all labels in the line (labels start with .)
    std::string final_line;
    size_t pos = 0;
    while (pos < expanded.length()) {
      // Look for local label markers (.)
      size_t dot_pos = expanded.find('.', pos);
      if (dot_pos == std::string::npos) {
        // No more dots, append rest of line
        final_line += expanded.substr(pos);
        break;
      }
      
      // Append everything before the dot
      final_line += expanded.substr(pos, dot_pos - pos);
      
      // Extract the label name
      size_t label_start = dot_pos;
      size_t label_end = dot_pos + 1;
      while (label_end < expanded.length() &&
             (std::isalnum(static_cast<unsigned char>(expanded[label_end])) ||
              expanded[label_end] == '_')) {
        label_end++;
      }
      
      // Check if this looks like a local label (starts with . followed by alphanum)
      if (label_end > label_start + 1 &&
          std::isalpha(static_cast<unsigned char>(expanded[label_start + 1]))) {
        // This is a local label - make it unique
        std::string label = expanded.substr(label_start, label_end - label_start);
        std::string unique_label = MakeLocalLabelUnique(label, expansion_id);
        final_line += unique_label;
        pos = label_end;
      } else {
        // Not a local label, just append the dot and continue
        final_line += '.';
        pos = dot_pos + 1;
      }
    }
    
    result.push_back(final_line);
  }
  
  return result;
}

std::string FlexAsmSyntax::SubstituteParameters(
    const std::string &line, const MacroDefinition &macro,
    const std::vector<std::string> &arguments) {
  std::string result = line;
  
  // Replace each parameter with its corresponding argument
  for (size_t i = 0; i < macro.parameters.size(); ++i) {
    const std::string &param = macro.parameters[i];
    const std::string &arg = (i < arguments.size()) ? arguments[i] : "";
    
    // Find and replace all occurrences of this parameter (word boundaries)
    size_t pos = 0;
    while ((pos = result.find(param, pos)) != std::string::npos) {
      // Check if this is a whole word match
      bool is_start_boundary = (pos == 0 || !std::isalnum(static_cast<unsigned char>(result[pos - 1])));
      bool is_end_boundary = (pos + param.length() >= result.length() ||
                              !std::isalnum(static_cast<unsigned char>(result[pos + param.length()])));
      
      if (is_start_boundary && is_end_boundary) {
        // This is a whole word match, replace it
        result.replace(pos, param.length(), arg);
        pos += arg.length(); // Move past the replacement
      } else {
        // Not a whole word, skip this occurrence
        pos += param.length();
      }
    }
  }
  
  return result;
}

std::string FlexAsmSyntax::MakeLocalLabelUnique(const std::string &label,
                                                int expansion_id) {
  // Local labels start with '.' in FLEX ASM09
  if (label.empty() || label[0] != '.') {
    // Not a local label, return unchanged
    return label;
  }

  // Make local label unique by appending expansion ID
  // Format: .LOOP -> .LOOP_001
  std::ostringstream oss;
  oss << label << "_" << std::setfill('0') << std::setw(3) << expansion_id;
  return oss.str();
}

// ============================================================================
// Conditional Assembly Implementation (Phase 3)
// ============================================================================

bool FlexAsmSyntax::EvaluateCondition(const std::string &condition) {
  std::string trimmed = Trim(condition);
  
  if (trimmed.empty()) {
    // Empty condition is false
    return false;
  }
  
  // Try to evaluate as a numeric expression
  // If it contains operators (+, -, *, /), evaluate the expression
  // If it's just a symbol name, check if symbol exists
  
  // For now, simple implementation:
  // - If it's an expression with operators, evaluate it
  // - If it's a number, check if non-zero
  // - If it's a symbol name (identifier), check if symbol exists
  
  // Check if it contains arithmetic operators FIRST (before trying ParseNumber)
  // This prevents "5-5" from being parsed as just "5"
  if (trimmed.find('+') != std::string::npos ||
      trimmed.find('-') != std::string::npos ||
      trimmed.find('*') != std::string::npos ||
      trimmed.find('/') != std::string::npos) {
    // Contains operators - evaluate expression
    try {
      // Simple evaluation: split by operator and compute
      // For MVP, we'll do basic arithmetic
      
      int64_t result = 0;
      
      // Handle addition
      size_t plus_pos = trimmed.find('+');
      if (plus_pos != std::string::npos) {
        std::string left = Trim(trimmed.substr(0, plus_pos));
        std::string right = Trim(trimmed.substr(plus_pos + 1));
        int64_t left_val = static_cast<int64_t>(ParseNumber(left));
        int64_t right_val = static_cast<int64_t>(ParseNumber(right));
        result = left_val + right_val;
        return result != 0;
      }
      
      // Handle subtraction
      size_t minus_pos = trimmed.find('-');
      if (minus_pos != std::string::npos && minus_pos > 0) {
        std::string left = Trim(trimmed.substr(0, minus_pos));
        std::string right = Trim(trimmed.substr(minus_pos + 1));
        int64_t left_val = static_cast<int64_t>(ParseNumber(left));
        int64_t right_val = static_cast<int64_t>(ParseNumber(right));
        result = left_val - right_val;
        return result != 0;
      }
      
      // Default: couldn't evaluate
      return false;
    } catch (const std::exception &) {
      return false;
    }
  }
  
  // No operators - try to parse as simple number
  try {
    uint32_t value = ParseNumber(trimmed);
    return value != 0; // Non-zero is true
  } catch (const std::exception &) {
    // Not a number, might be a symbol name
    // Check if symbol exists in the symbol table
    if (current_symbols_ != nullptr) {
      int64_t value;
      // Symbol exists and has a value means condition is true
      return current_symbols_->Lookup(trimmed, value);
    }
    
    // No symbol table available, undefined symbols evaluate to false
    return false;
  }
}

bool FlexAsmSyntax::ShouldAssemble() const {
  // Should assemble if all conditions on stack are true
  // Empty stack means no conditionals, should assemble
  if (conditional_stack_.empty()) {
    return true;
  }
  
  // Check if top of stack is true
  return conditional_stack_.top();
}

} // namespace xasm
