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
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/cpu_constants.h"
#include "xasm++/cpu/cpu_plugin.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/directives/scmasm_directive_handlers.h"
#include "xasm++/syntax/scmasm_expression_utils.h"
#include "xasm++/util/string_utils.h" // For ToUpper
#include <algorithm>
#include <cctype>
#include <filesystem>
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

// ---------------------------------------------------------------------------
// IsEqOperandSafe  (used by StripComments to validate star-label .EQ values)
//
// Returns true if the operand can be resolved without consulting the symbol
// table — i.e. it contains only numeric literals, `*` (current address), and
// arithmetic operators.  Returns false if it contains:
//   • A bare symbol name  (e.g. "K.FClose"  — would be a forward reference)
//   • An invalid character inside a hex literal  (e.g. "$Cn5C" where 'n' is
//     not a hex digit)
//
// When this returns false, StripComments treats the *LABEL .EQ line as an
// ordinary full-line comment.
// ---------------------------------------------------------------------------
bool IsEqOperandSafe(const std::string &operand) {
  // Trim leading/trailing whitespace
  size_t start = operand.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return false; // empty operand — treat as unsafe
  }
  size_t end = operand.find_last_not_of(" \t");
  std::string s = operand.substr(start, end - start + 1);

  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];

    if (c == '$') {
      // Hex literal: all following chars until delimiter must be [0-9A-Fa-f]
      ++i;
      if (i >= s.size()) {
        return false; // bare '$' with no digits
      }
      bool any_digit = false;
      while (i < s.size()) {
        char h = s[i];
        if (std::isxdigit(static_cast<unsigned char>(h))) {
          any_digit = true;
          ++i;
        } else {
          break; // end of hex literal (operator or end-of-string)
        }
      }
      if (!any_digit) {
        return false; // '$' followed by a non-hex char
      }
    } else if (c == '%') {
      // Binary literal: following chars must be [0-1.]
      ++i;
      if (i >= s.size()) {
        return false;
      }
      bool any_bit = false;
      while (i < s.size() &&
             (s[i] == '0' || s[i] == '1' || s[i] == '.')) {
        any_bit = true;
        ++i;
      }
      if (!any_bit) {
        return false;
      }
    } else if (std::isdigit(static_cast<unsigned char>(c))) {
      // Decimal digits
      while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
        ++i;
      }
    } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      // A letter or underscore NOT preceded by '$' → symbol reference
      return false;
    } else {
      // Operators, parens, '#', ',', '*', '+', '-', '/', whitespace — all ok
      ++i;
    }
  }

  return true;
}

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
  // The delimiter must be a special (non-alphanumeric) character such as
  // ', ", #, / etc.  Alphanumeric first characters (e.g. "AB", "GP") are
  // symbol names, NOT character constants.
  if (!std::isalnum(static_cast<unsigned char>(token[0])) && token.length() == 2) {
    char delimiter = token[0];
    char c = token[1];

    // Apply high-bit rule:
    // If delimiter ASCII < 0x27 (apostrophe '), high bit is SET
    // Otherwise, high bit is CLEAR
    auto result = static_cast<uint8_t>(c);
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

ScmasmSyntaxParser::ScmasmSyntaxParser() { InitializeDirectiveRegistry(); }

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
  directive_registry_[INB] = scmasm::HandleInb;     // Include source file
  directive_registry_[INCLUDE] = scmasm::HandleInb; // Alias: .INCLUDE = .INB
  directive_registry_[LIST] = scmasm::HandleList;   // Listing control
  directive_registry_[DUMMY] = scmasm::HandleDummy; // Dummy section
  directive_registry_[ED] = scmasm::HandleEd;       // End dummy section
  directive_registry_[OP] = scmasm::HandleOp;       // CPU operation mode

  // Phase 3: 100% Coverage Directives
  directive_registry_[CS] = scmasm::HandleCs; // C-string with escapes
  directive_registry_[CZ] = scmasm::HandleCz; // C-string zero-terminated
  directive_registry_[TF] = scmasm::HandleTf; // Text file/title metadata
  directive_registry_[EP] = scmasm::HandleEp; // Entry point / end phase
  directive_registry_[PH] = scmasm::HandlePh; // Phase assembly
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

void ScmasmSyntaxParser::SetCpu(const std::string &cpu_name) {
  // Allocate a Cpu6502 instance owned by this parser.
  // The pointer is stored in owned_cpu_ so that it outlives this function
  // and is freed when the parser is destroyed.  Previously cpu6502 was a
  // local variable, causing cpu_ to become a dangling pointer and crash
  // the first time an instruction was assembled after a .OP directive.
  auto cpu6502 = std::make_unique<Cpu6502>();

  // Set the appropriate CPU mode based on the requested variant
  if (cpu_name == "6502") {
    cpu6502->SetCpuMode(CpuMode::Cpu6502);
  } else if (cpu_name == "65C02") {
    cpu6502->SetCpuMode(CpuMode::Cpu65C02);
  } else if (cpu_name == "65C02Rock") {
    cpu6502->SetCpuMode(CpuMode::Cpu65C02Rock);
  } else if (cpu_name == "65816") {
    cpu6502->SetCpuMode(CpuMode::Cpu65816);
  } else {
    throw std::runtime_error("Invalid CPU name: " + cpu_name);
  }

  owned_cpu_ = std::move(cpu6502);
  cpu_ = owned_cpu_.get();
}

void ScmasmSyntaxParser::SetIncludePaths(
    const std::vector<std::string> &paths) {
  include_paths_ = paths;
}

void ScmasmSyntaxParser::SetPathMappings(
    const std::map<std::string, std::string> &mappings) {
  path_mappings_.clear();

  // Normalize all path mapping keys to use forward slashes for cross-platform
  // compatibility
  for (const auto &[virtual_path, actual_path] : mappings) {
    // Manually replace backslashes with forward slashes
    std::string normalized_virtual = virtual_path;
    std::replace(normalized_virtual.begin(), normalized_virtual.end(), '\\',
                 '/');

    path_mappings_[normalized_virtual] = actual_path;
  }
}

// ============================================================================
// Main Parse Function
// ============================================================================

void ScmasmSyntaxParser::Parse(const std::string &source, Section &section,
                               ConcreteSymbolTable &symbols) {
  // SCMASM normalises all symbols to UPPERCASE at definition time.  Enable the
  // uppercase-fallback mode in the symbol table so that mixed-case references
  // (e.g. "TmpPtr2") resolve to their normalised UPPERCASE counterparts.
  // ADR-005 V1: this is the canonical place for the fallback; it is no longer
  // duplicated in assembler.cpp ParseExpression or the branch-resolution loop.
  symbols.SetUppercaseFallback(true);
  symbols.SetDottedNamespaceFallback(true);

  // Split source into lines
  std::vector<std::string> lines;
  std::istringstream stream(source);
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  // Reset per-pass state.  The macro invocation counter must restart at 0
  // each pass so that ':N' macro-local labels get the same scoped names in
  // every pass (stable symbol identity is required for multi-pass convergence).
  macro_invocation_counter_ = 0;

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

    // Strip Apple II editor commands (NEW, AUTO, MAN, SAVE, ASM, etc.)
    line = StripEditorCommands(line);

    // Skip empty lines (check against fully trimmed line)
    if (Trim(line).empty()) {
      line_idx++;
      continue;
    }

    // Strip trailing whitespace only — preserve leading whitespace so that
    // ParseLabel can detect whether a token is at column 0 (label field) or
    // in the mnemonic column (after leading whitespace).  The label_start
    // position in ParseLabel must reflect the original indentation.
    {
      size_t end = line.size();
      // NOLINT(bugprone-infinite-loop): false positive — --end decrements the
      // loop variable 'end' inside the loop body on every iteration.
      while (end > 0 && (line[end - 1] == ' ' || line[end - 1] == '\t' || // NOLINT(bugprone-infinite-loop)
                         line[end - 1] == '\r' || line[end - 1] == '\n')) {
        --end;
      }
      line = line.substr(0, end);
    }

    // If we're in a macro definition, collect lines
    if (in_macro_definition_) {
      // Check for .EM or .ENDM (trim leading whitespace before checking)
      std::string upper_line = Trim(line);
      std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(),
                     ::toupper);

      if (upper_line.starts_with(".EM") || upper_line.starts_with(".ENDM")) {
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
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
      throw std::runtime_error(FormatError(e.what()));
    }

    line_idx++;
  }

  // Flush any pending label that was left over at end-of-file.
  // This happens when the last substantive line is a label-only line (e.g.
  // "DRV.END") followed only by editor commands (MAN, SAVE, ASM) that get
  // stripped by StripEditorCommands.  Without this flush, the label is never
  // added to the symbol table and resolves as 0.
  if (!pending_label_.empty()) {
    std::string pl = pending_label_;
    pending_label_ = "";
    if (IsLocalLabel(pl)) {
      local_labels_[pl] = current_address_;
      std::string scoped = ScopedLocalLabelName(pl);
      auto expr = std::make_shared<LiteralExpr>(current_address_);
      symbols.Define(scoped, SymbolType::Label, expr);
      if (!in_dummy_section_) {
        auto atom = std::make_shared<LabelAtom>(scoped, current_address_);
        section.atoms.push_back(atom);
      }
    } else {
      std::string norm = util::ToUpper(pl);
      auto expr = std::make_shared<LiteralExpr>(current_address_);
      symbols.Define(norm, SymbolType::Label, expr);
      if (!in_dummy_section_) {
        auto atom = std::make_shared<LabelAtom>(norm, current_address_);
        section.atoms.push_back(atom);
      }
      last_global_label_ = norm;
    }
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

    // Valid line number — skip exactly ONE separator character.
    // Do NOT strip all following whitespace: the indentation after the
    // separator distinguishes column-0 labels from indented mnemonics.
    // e.g. "1010 LABEL    .EQ $00"  → strip "1010 " → "LABEL    .EQ $00"  (col 0 = label)
    //      "1010           JMP $A132" → strip "1010 " → "          JMP $A132" (indented = mnemonic)
    if (pos < line.length() && std::isspace(line[pos])) {
      pos++;  // Skip exactly one separator space/tab
    }

    return line.substr(pos);
  }

  return line;
}

std::string ScmasmSyntaxParser::StripComments(const std::string &line) {
  // Two comment styles:
  // 1. * in column 1 (full-line comment)
  // 2. ; anywhere outside a string literal (rest of line is comment)

  // Check for * in column 1 (after any leading whitespace)
  //
  // SCMASM private-label marker: *LABEL .EQ value
  // In SCMASM, a line starting with *<labelchar> at column 0 is NOT a full
  // comment — the * is a private/reserved label marker that the assembler
  // still processes (defining the label), only the opcode emission is
  // suppressed for real instructions.  .EQ/.SE directives are zero-emission
  // anyway, so *LABEL .EQ value fully defines the label.
  //
  // Rule:
  //   * at column 0 followed by a valid label-start char → strip * and
  //     process the rest of the line as a normal source line.
  //   * anywhere else (column 0 + non-label char, or after whitespace) →
  //     full-line comment (return empty).
  size_t first_non_space = line.find_first_not_of(" \t");
  if (first_non_space != std::string::npos && line[first_non_space] == '*') {
    if (first_non_space == 0 && line.size() > 1 &&
        std::isalpha(static_cast<unsigned char>(line[1]))) {
      // *LABEL <directive> value — private/reserved label marker in SCMASM.
      // Strip the leading * and process the rest as a normal line.
      //
      // Guard: ensure the label token (chars from pos 1 to first whitespace)
      // contains only valid SCMASM label characters [A-Za-z0-9._].
      // Lines like "*LCG_PARKMILLER(uint32_t)" contain '(' and are plain
      // full-line comments.
      // Scan label chars: must be [A-Za-z0-9._] only (no parens, commas, etc.)
      size_t pos = 1;
      bool label_valid = true;
      while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t') {
        char c = line[pos];
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '.' &&
            c != '_') {
          label_valid = false;
          break;
        }
        ++pos;
      }
      if (label_valid && pos > 1) {
        // Also require that the opcode field starts with '.' (SCMASM directive).
        // This rejects function-signature comments like:
        //   *LCG_PARKMILLER<TAB>(uint32_t seed) -> uint32_t
        // where the opcode would be "(uint32_t" rather than a directive.
        size_t opcode_pos = pos;
        while (opcode_pos < line.size() &&
               (line[opcode_pos] == ' ' || line[opcode_pos] == '\t')) {
          opcode_pos++;
        }
        if (opcode_pos < line.size() && line[opcode_pos] == '.') {
          // For value-defining directives (.EQ / .SE) also validate that the
          // operand does not contain forward-reference symbols or invalid
          // placeholder characters.  Examples that must be treated as comments:
          //   *K.CloseDir   .EQ K.FClose   (K.FClose defined later → fwd ref)
          //   *IO.D2.ReadSect .EQ $Cn5C    ('n' is not a valid hex digit)
          // If the operand fails validation, fall through and return "" (comment).
          //
          // Identify the end of the opcode token.
          size_t op_end = opcode_pos;
          while (op_end < line.size() && line[op_end] != ' ' &&
                 line[op_end] != '\t') {
            ++op_end;
          }

          // Extract and upper-case the opcode to compare.
          std::string opcode_token = line.substr(opcode_pos, op_end - opcode_pos);
          for (char &ch : opcode_token) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
          }

          if (opcode_token == ".EQ" || opcode_token == ".SE") {
            // Extract the operand (everything after the opcode token).
            size_t operand_start = op_end;
            while (operand_start < line.size() &&
                   (line[operand_start] == ' ' || line[operand_start] == '\t')) {
              ++operand_start;
            }
            std::string operand_str = line.substr(operand_start);

            // Strip inline comment from operand (semicolon-delimited).
            size_t semi = operand_str.find(';');
            if (semi != std::string::npos) {
              operand_str = operand_str.substr(0, semi);
            }

            if (!IsEqOperandSafe(operand_str)) {
              // Operand contains a forward-reference symbol or an invalid
              // placeholder character — treat the whole line as a comment.
              return "";
            }

            // Strip the leading * — label starts at position 1.
            return line.substr(1);
          }

          // *LABEL .BS/.DA/.DC/etc. — in SCMASM, only .EQ/.SE after *LABEL are
          // private label definitions. Any other directive makes the entire line
          // a comment (the *LABEL is just a visual marker for disabled code).
          return "";
        }
      }
    }
    return ""; // Entire line is comment
  }

  // Scan for ; comment, skipping over quoted string literals so that
  // semicolons inside .CS/.CZ strings (e.g. "\e[37;40m") are not stripped.
  bool in_string = false;
  char string_delim = 0;
  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];
    if (in_string) {
      // SCMASM: the string delimiter is never escapable by backslash,
      // so '\"' does NOT extend the string — the '"' ends it.
      if (c == string_delim) {
        in_string = false;
      }
    } else {
      if (c == '"' || c == '\'') {
        in_string = true;
        string_delim = c;
      } else if (c == ';') {
        return line.substr(0, i);
      }
    }
  }

  return line;
}

std::string ScmasmSyntaxParser::StripEditorCommands(const std::string &line) {
  // Apple II line editor commands that should be ignored during assembly
  // Format: command at start of line (case-insensitive), optionally followed by
  // arguments
  static const std::unordered_set<std::string> EDITOR_COMMANDS = {
      "NEW", "AUTO", "MAN", "SAVE", "LOAD", "ASM", "DELETE", "LIST"};

  // Find first non-whitespace/control character
  // Include backspace (\b) which appears before MAN in some files
  size_t first_non_space = line.find_first_not_of(" \t\r\n\b");
  if (first_non_space == std::string::npos) {
    return line; // Empty line
  }

  // Extract the first token (up to whitespace or end of line)
  size_t token_end = first_non_space;
  while (token_end < line.length() && !std::isspace(line[token_end])) {
    token_end++;
  }

  std::string token = line.substr(first_non_space, token_end - first_non_space);

  // Convert to uppercase for case-insensitive comparison
  std::string upper_token = token;
  for (char &c : upper_token) {
    c = std::toupper(static_cast<unsigned char>(c));
  }

  // Check if token is an editor command
  if (EDITOR_COMMANDS.contains(upper_token)) {
    // For commands like SAVE/LOAD that can conflict with mnemonics,
    // only strip if the rest of the line looks like a file path
    // (contains slashes or looks like a path, not an assembly operand)
    if (upper_token == "LOAD" || upper_token == "SAVE") {
      // Get the rest of the line after the command
      std::string rest_of_line;
      if (token_end < line.length()) {
        rest_of_line = line.substr(token_end);
        rest_of_line = Trim(rest_of_line);
      }

      // If there's an operand, check if it looks like an assembly operand
      if (!rest_of_line.empty()) {
        char first_char = rest_of_line[0];
        // Assembly operands typically start with #, $, (, or a letter
        // File paths contain / or multiple segments with dots
        if (first_char == '#' || first_char == '$' || first_char == '(' ||
            first_char == '[' || first_char == '<' || first_char == '>') {
          // Looks like an assembly operand, don't strip
          return line;
        }
        // Check for file path indicators (slashes or .S/.ASM extension pattern)
        if (rest_of_line.find('/') != std::string::npos ||
            rest_of_line.find('\\') != std::string::npos) {
          // Looks like a file path, strip it
          return "";
        }
      }
    }
    // Other editor commands always strip
    return ""; // Strip entire line
  }

  return line;
}

// Strip trailing whitespace only, preserving leading whitespace.
// Used when preparing lines for ParseLine so that the original column
// positions are preserved (leading whitespace distinguishes column-0 labels
// from mnemonic-column opcodes).
static std::string TrimRight(const std::string &str) {
  size_t end = str.size();
  while (end > 0 && (str[end - 1] == ' ' || str[end - 1] == '\t' ||
                     str[end - 1] == '\r' || str[end - 1] == '\n')) {
    --end;
  }
  return str.substr(0, end);
}

std::string ScmasmSyntaxParser::Trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }

  // Strip leading non-printable non-tab control chars (Apple II editor
  // artifacts like \x01 SOH that sometimes precede instruction lines).
  while (start < str.length() && (unsigned char)str[start] < 0x20 &&
         str[start] != '\t') {
    start++;
  }
  if (start >= str.length()) {
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

  // If nothing left (just a label, nothing else), defer it as pending_label_.
  // It may be the label for a .EQ/.SE on the next line (SCMASM pattern where
  // the label appears on a separate line before the .EQ directive).
  // Otherwise it will be defined at the next instruction's address.
  if (pos >= line.length()) {
    if (!label.empty()) {
      // If there is already a pending_label_ from a previous label-only line,
      // define it NOW at current_address_ before overwriting.  Consecutive
      // label-only lines all share the same address (the address of whatever
      // instruction follows), so it is correct to emit the earlier label here.
      if (!pending_label_.empty()) {
        std::string pl = pending_label_;
        if (IsLocalLabel(pl)) {
          local_labels_[pl] = current_address_;
          std::string scoped = ScopedLocalLabelName(pl);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(scoped, SymbolType::Label, expr);
          if (!in_dummy_section_) {
            auto atom = std::make_shared<LabelAtom>(scoped, current_address_);
            section.atoms.push_back(atom);
          }
        } else {
          std::string norm = util::ToUpper(pl);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(norm, SymbolType::Label, expr);
          if (!in_dummy_section_) {
            auto atom = std::make_shared<LabelAtom>(norm, current_address_);
            section.atoms.push_back(atom);
          }
          last_global_label_ = norm;
        }
      }
      pending_label_ = label; // defer; don't define yet
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

  // Skip whitespace after opcode, counting tabs.
  // In SCMASM/Merlin 3-column format, a SINGLE tab separates mnemonic from
  // operand.  When 2+ tabs appear between the mnemonic and the next text, the
  // text is a visual-alignment comment and the operand column is empty.
  // e.g. "dec\t\t\t\tA=0"  → 4 tabs → empty operand (= accumulator DEA)
  //      "sta\t(ZPPtr1)"   → 1 tab  → operand = "(ZPPtr1)"
  size_t tabs_after_opcode = 0;
  while (pos < line.length() && std::isspace(line[pos])) {
    if (line[pos] == '\t') {
      ++tabs_after_opcode;
    }
    pos++;
  }

  // Rest is operand — empty when 2+ tabs precede the remaining text.
  std::string operand;
  if (tabs_after_opcode < 2) {
    operand = line.substr(pos);
    operand = Trim(operand);
  }

  // Convert opcode to uppercase for comparison
  std::string opcode_upper = opcode;
  std::transform(opcode_upper.begin(), opcode_upper.end(), opcode_upper.begin(),
                 ::toupper);

  // --- Resolve pending label from previous label-only line ---
  // If pending_label_ is set and the current line has no inline label:
  //   - For .EQ/.SE: use pending_label_ as the label (SCMASM two-line pattern)
  //   - For any other instruction/directive: define pending_label_ at current
  //     address (normal label-before-instruction pattern)
  if (!pending_label_.empty()) {
    bool used_for_eq = (label.empty() &&
                        (opcode_upper == ".EQ" || opcode_upper == ".SE"));
    if (used_for_eq) {
      label = pending_label_;
    } else {
      // Define pending_label_ at the current address
      std::string pl = pending_label_;
      if (IsLocalLabel(pl)) {
        local_labels_[pl] = current_address_;
        // Also add scoped version to global symbol table for branch resolution
        std::string scoped = ScopedLocalLabelName(pl);
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(scoped, SymbolType::Label, expr);
        if (!in_dummy_section_) {
          auto atom = std::make_shared<LabelAtom>(scoped, current_address_);
          section.atoms.push_back(atom);
        }
      } else {
        std::string norm = util::ToUpper(pl);
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(norm, SymbolType::Label, expr);
        if (!in_dummy_section_) {
          auto atom = std::make_shared<LabelAtom>(norm, current_address_);
          section.atoms.push_back(atom);
        }
        last_global_label_ = norm;
      }
    }
    pending_label_ = "";
  }

  // Handle directives (must start with .)
  if (!opcode.empty() && opcode[0] == '.') {
    // For directives, define the label but DON'T create a label atom
    // Exception: .OR creates a label atom before changing address
    // Exception: .DO/.LU defer label definition until after block processing
    using namespace scmasm::directives;
    bool is_control_flow = (opcode_upper == DO || opcode_upper == LU);
    
    // .EQ and .SE define symbols via their directive handlers (HandleEq/HandleSe),
    // which evaluate the expression and store the result. Pre-defining the label
    // as Label(current_address_) here would corrupt their expression evaluation:
    // e.g. "PAKME.ID .SE PAKME.ID+2" would evaluate PAKME.ID as the current PC
    // (from the pre-definition) instead of the previous Set value, producing
    // wrong accumulated counters like PAKME.ID.
    bool is_value_directive = (opcode_upper == ".EQ" || opcode_upper == ".SE");

    if (!label.empty() && !is_control_flow && !is_value_directive) {
      if (opcode_upper == ".OR") {
        // Label before .OR - define it and create label atom at current address
        if (IsLocalLabel(label)) {
          local_labels_[label] = current_address_;
          // Also add scoped version to global symbol table for branch resolution
          std::string scoped = ScopedLocalLabelName(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(scoped, SymbolType::Label, expr);
          auto label_atom =
              std::make_shared<LabelAtom>(scoped, current_address_);
          section.atoms.push_back(label_atom);
        } else {
          // Normalize label to uppercase for case-insensitive SCMASM
          // compatibility
          std::string normalized_label = util::ToUpper(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(normalized_label, SymbolType::Label, expr);

          // Create label atom for non-local labels (use normalized name)
          auto label_atom =
              std::make_shared<LabelAtom>(normalized_label, current_address_);
          section.atoms.push_back(label_atom);
          last_global_label_ = normalized_label;
        }
      } else {
        // Other directives: define the label.
        // For data-emitting directives (DA, DB, BS, PH, etc.), also push a
        // LabelAtom so ResolveSymbols updates the address each pass. This
        // ensures positional labels like "PAKME.CORE .DA CORE.P" and
        // "CORE.B .PH K.HiMem" track the correct physical address after branch
        // relaxation changes code sizes.
        static const std::unordered_set<std::string> kDataEmittingDirectives = {
            ".DA",  ".DB",  ".DFB", ".DW",  ".DS",  ".DC",  ".HB",  ".HX",
            ".AS",  ".AT",  ".AZ",  ".CS",  ".CZ",  ".TF",  ".PS",  ".HS",
            ".STR", ".BS",  ".BYT", ".WORD", ".PH",
        };
        bool emit_label_atom =
            kDataEmittingDirectives.contains(opcode_upper) &&
            !in_dummy_section_;

        if (IsLocalLabel(label)) {
          local_labels_[label] = current_address_;
          // Also add scoped version to global symbol table for branch resolution
          std::string scoped = ScopedLocalLabelName(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(scoped, SymbolType::Label, expr);
          if (emit_label_atom) {
            auto label_atom =
                std::make_shared<LabelAtom>(scoped, current_address_);
            section.atoms.push_back(label_atom);
          }
        } else {
          // Normalize label to uppercase for case-insensitive SCMASM
          // compatibility
          std::string normalized_label = util::ToUpper(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(normalized_label, SymbolType::Label, expr);
          last_global_label_ = normalized_label;
          if (emit_label_atom) {
            auto label_atom =
                std::make_shared<LabelAtom>(normalized_label, current_address_);
            section.atoms.push_back(label_atom);
          }
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
      // Strip SCMASM inline comment: .DO expression has no embedded spaces
      std::string do_operand = operand;
      {
        size_t ws = do_operand.find_first_of(" \t");
        if (ws != std::string::npos) {
          do_operand = do_operand.substr(0, ws);
        }
      }
      {
        DirectiveContext do_ctx;
        do_ctx.label = label;
        do_ctx.operand = do_operand;
        HandleDo(do_ctx, section, symbols, source, line_idx);
      }
    } else if (opcode_upper == LU) { // NOLINT(bugprone-branch-clone)
      {
        DirectiveContext lu_ctx;
        lu_ctx.label = label;
        lu_ctx.operand = operand;
        HandleLu(lu_ctx, section, symbols, source, line_idx);
      }
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
        // Expand local label references in operand before dispatch so that
        // e.g. ".DA .10" correctly references "GLOBAL@.10" in the symbol table.
        std::string directive_operand = ExpandLocalLabelsInOperand(operand);
        DirectiveContext context;
        context.section = &section;
        context.symbols = &symbols;
        context.current_address = &current_address_;
        context.parser_state =
            this; // Phase 6c.2: Set parser for handler access
        context.current_file = current_file_;
        context.current_line = current_line_;
        context.include_paths = &include_paths_;
        context.path_mappings = &path_mappings_;
        context.label = label;
        context.operand = directive_operand;
        it->second(context);
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
        // Also add scoped version to global symbol table for branch resolution
        std::string scoped = ScopedLocalLabelName(label);
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(scoped, SymbolType::Label, expr);
        if (!in_dummy_section_) {
          auto label_atom =
              std::make_shared<LabelAtom>(scoped, current_address_);
          section.atoms.push_back(label_atom);
        }
      } else {
        // Normalize label to uppercase for case-insensitive SCMASM
        // compatibility
        std::string normalized_label = util::ToUpper(label);
        auto expr = std::make_shared<LiteralExpr>(current_address_);
        symbols.Define(normalized_label, SymbolType::Label, expr);

        // Create label atom for non-local labels (use normalized name)
        if (!in_dummy_section_) {
          auto label_atom =
              std::make_shared<LabelAtom>(normalized_label, current_address_);
          section.atoms.push_back(label_atom);
        }
        last_global_label_ = normalized_label;
      }
    }

    // Check if it's a macro invocation
    // SCMASM uses >MacroName syntax for invocation
    std::string macro_lookup_name = opcode_upper;
    if (!opcode_upper.empty() && opcode_upper[0] == '>') {
      macro_lookup_name = opcode_upper.substr(1);
    }

    auto it = macros_.find(macro_lookup_name);
    if (it != macros_.end()) {
      // Parse macro parameters from operand
      std::vector<std::string> params;
      if (!operand.empty()) {
        // Parse macro arguments: each argument ends at the first whitespace or
        // comma.  A comma immediately following an argument introduces the next
        // argument.  Any whitespace following an argument (without a preceding
        // comma) terminates the argument list — the rest of the line is treated
        // as an inline comment (SCMASM convention).
        //
        // e.g. ">MLICALL MLI.READ   Read block" → ["MLI.READ"]
        // e.g. ">STYA ZPPtr1,Y"                → ["ZPPtr1", "Y"]
        // e.g. ">STYA ZPPtr1   f(), starting"  → ["ZPPtr1"]  (comma in comment
        //                                         must not become a 2nd arg)
        size_t pos = 0;
        while (pos < operand.length()) {
          // Skip leading whitespace before each argument (only matters after a
          // comma separator, e.g. ">MACRO arg1, arg2").
          while (pos < operand.length() &&
                 (operand[pos] == ' ' || operand[pos] == '\t')) {
            ++pos;
          }
          if (pos >= operand.length()) {
            break;
          }

          // Scan to the first whitespace or comma (argument boundary).
          size_t arg_start = pos;
          while (pos < operand.length() && operand[pos] != ' ' &&
                 operand[pos] != '\t' && operand[pos] != ',') {
            ++pos;
          }

          std::string param = operand.substr(arg_start, pos - arg_start);
          if (!param.empty()) {
            params.push_back(param);
          }

          if (pos < operand.length() && operand[pos] == ',') {
            // Comma separator → another argument follows.
            ++pos;
          } else {
            // Whitespace or end-of-string → rest is inline comment, stop.
            break;
          }
        }
      }
      // Invoke the macro (use stripped name without > prefix)
      InvokeMacro(macro_lookup_name, params, section, symbols);
    } else {
      // If opcode starts with '>' but macro not found, emit clear error
      if (!opcode_upper.empty() && opcode_upper[0] == '>') {
        throw std::runtime_error("Undefined macro: " + macro_lookup_name);
      }
      // Assume it's an assembly instruction (6502, 65C02, etc.)
      // Phase 3: We don't parse instructions yet, just store them as
      // InstructionAtom.
      //
      // Strip SCMASM inline comment: 6502 operands never have embedded spaces
      // EXCEPT inside character literals ('x' / "x").  Any whitespace outside
      // a quoted char literal is treated as a comment delimiter.
      // e.g. "TAX  %11000000 or %00111000" → operand = "" (empty after opcode)
      // e.g. "AND K.LC,y  should be %xx..." → operand = "K.LC,y"
      // e.g. "#' '" → operand = "#' '" (space inside quotes, NOT a comment)
      std::string instr_operand = operand;
      {
        bool in_quote = false;
        char quote_ch = 0;
        size_t ws_pos = std::string::npos;
        for (size_t k = 0; k < instr_operand.size(); ++k) {
          char ch = instr_operand[k];
          if (in_quote) {
            if (ch == quote_ch) {
              in_quote = false;
            }
          } else if (ch == '\'' || ch == '"') {
            in_quote = true;
            quote_ch = ch;
          } else if (ch == ' ' || ch == '\t') {
            ws_pos = k;
            break;
          }
        }
        if (ws_pos != std::string::npos) {
          instr_operand = instr_operand.substr(0, ws_pos);
        }
      }

      // Translate local label operands to scoped names for multi-pass
      // branch resolution.  e.g. ".8" → "GLOBALNAME.8",
      //                          ".8,X" → "GLOBALNAME.8,X"
      // Expand SCMASM character literals to hex before local-label scoping
      // and InstructionAtom creation.  The generic ParseExpression engine in
      // assembler.cpp has no knowledge of SCMASM quoting conventions, so
      // "X" / 'X' would be mis-parsed as undefined symbols and silently
      // evaluate to 0.  Pre-expand them here: "0"→$B0, 'A'→$41, etc.
      instr_operand = ExpandCharLiteralsInExpr(instr_operand);

      // Expand local label references (.N or :N) anywhere in the operand
      // expression to their scoped form (GLOBALNAME.N / GLOBALNAME:N).
      //
      // The old approach only handled the case where the ENTIRE operand (or
      // the part before the comma) was a local label.  That missed compound
      // expressions like:
      //   sta .1+2    — self-modified absolute address  (X.PrintF.S pattern)
      //   lda (.1),Y  — indirect indexed through local label address
      //
      // A local label reference is '.' or ':' followed by one or more digits,
      // appearing at the START of the operand string or immediately after a
      // non-identifier character (operator, paren, '#', etc.).
      instr_operand = ExpandLocalLabelsInOperand(instr_operand);

      // Instructions inside a .DUMMY section advance the virtual address (for
      // symbol placement) but do NOT emit code bytes.  Only add the atom when
      // we are in the real (non-dummy) section.
      if (!in_dummy_section_) {
        auto instr_atom =
            std::make_shared<InstructionAtom>(opcode_upper, instr_operand);
        section.atoms.push_back(instr_atom);
      }

      // Advance address by the instruction's estimated byte count.
      // Using the CPU plugin's heuristic (operand-string analysis) so that
      // current-address expressions like `.BS TARGET-*` compute correctly.
      if (cpu_) {
        size_t est =
            cpu_->GetInstructionSize(opcode_upper, instr_operand);
        // GetInstructionSize returns 3 for any symbol operand (it assumes
        // absolute mode, since it has no access to the symbol table).  But
        // many 6502 instructions have a 2-byte ZP form.  If the operand
        // resolves to a ZP address ($00–$FF) right now, correct the estimate
        // to 2.  This prevents false branch-relaxation: e.g. "dec pStack"
        // (pStack=$DC) is 2 bytes, but GetInstructionSize returns 3.  With
        // many PUSHA/PULLA macro calls between a branch and its target the
        // cumulative overcount pushes the estimated distance past 127, causing
        // the branch to be incorrectly relaxed (+3 bytes).
        //
        // Exclusions: JSR and JMP have no ZP form — keep them at 3.
        if (est == 3 && opcode_upper != "JSR" && opcode_upper != "JMP") {
          std::string eval_expr = instr_operand;
          // Strip index suffix (,X or ,Y) so the base expression evaluates.
          auto comma = eval_expr.rfind(',');
          if (comma != std::string::npos)
            eval_expr = Trim(eval_expr.substr(0, comma));
          try {
            uint32_t val = EvaluateExpression(eval_expr, symbols);
            if (val <= 0xFF) {
              est = 2;
            }
          } catch (const std::exception &e) {
            (void)e; // Expression has a forward reference or is otherwise
                     // unevaluable — keep the 3-byte estimate.
          }
        }
        current_address_ += static_cast<uint32_t>(est);
      } else {
        current_address_ += 3; // conservative fallback when no CPU plugin set
      }
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

  // Skip Apple II editor control chars (0x01–0x1F, excluding tab) that may
  // appear before the label/opcode in raw Apple II source files.
  while (pos < line.length() && (unsigned char)line[pos] < 0x20 &&
         line[pos] != '\t') {
    pos++;
  }

  // Labels must start with letter, ., :, _, or & (SCMASM & macro-label prefix)
  if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '.' &&
                               line[pos] != ':' && line[pos] != '_' &&
                               line[pos] != '&')) {
    return "";
  }

  size_t label_start = pos;

  // Strip leading '&' (SCMASM macro-label prefix; column-0 only)
  bool had_ampersand = (line[pos] == '&');
  if (had_ampersand) {
    ++pos;
    // After '&', must have a normal label-start character
    if (pos >= line.length() || !std::isalpha(line[pos])) {
      pos = label_start;
      return "";
    }
  }

  // Parse label characters (letter, digit, underscore, ., or :)
  while (pos < line.length() && (std::isalnum(line[pos]) || line[pos] == '_' ||
                                 line[pos] == '.' || line[pos] == ':')) {
    pos++;
  }

  // If the label started with '&', exclude it from the symbol name
  size_t name_start = had_ampersand ? label_start + 1 : label_start;
  std::string label = line.substr(name_start, pos - name_start);

  // Convert to uppercase for checking
  std::string label_upper = label;
  std::transform(label_upper.begin(), label_upper.end(), label_upper.begin(),
                 ::toupper);

  // Check if this is actually a directive (not a label)
  // Directives start with . and have letters after (e.g., .OR, .EQ, .SE)
  // Local labels start with . or : and have only digits (e.g., .1, .2, :1, :2)
  if (label.length() >= 2 && (label[0] == '.' || label[0] == ':')) {
    bool has_letter = false;
    for (size_t i = 1; i < label.length(); i++) {
      if (std::isalpha(label[i])) {
        has_letter = true;
        break;
      }
    }
    // If it has letters after the . or :, it's a directive or invalid, not a
    // label
    if (has_letter) {
      pos = label_start;
      return "";
    }
  }

  // In SCMASM/Merlin format, column 0 (no leading whitespace) is the LABEL
  // field.  A token at column 0 is ALWAYS a label, even when it matches an
  // opcode name or macro name (e.g. BCC, BEQ defined as data-table entry
  // labels; or a label that happens to share a name with a user-defined macro).
  // Only apply the opcode/pseudo-op/macro rejection when the token appeared
  // after leading whitespace (i.e. in the mnemonic column).
  if (label_start > 0) {
    // Check if this is a macro name (not a label)
    // Macros are invoked from the mnemonic field; a macro name in the label
    // field (column 0) is a label definition, not a macro call.
    if (macros_.contains(label_upper)) {
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
    static const std::unordered_set<std::string> kPseudoOps = {"DB", "DW",
                                                                "DS"};
    if (kPseudoOps.contains(label_upper)) {
      // This is a pseudo-op, not a label
      pos = label_start;
      return "";
    }
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

void ScmasmSyntaxParser::HandleEq(const DirectiveContext &ctx,
                                  ConcreteSymbolTable &symbols) {
  const std::string &label = ctx.label;
  const std::string &operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".EQ requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Define symbol (immutable) - .EQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  symbols.Define(label, SymbolType::Equate, expr);
}

void ScmasmSyntaxParser::HandleSe(const DirectiveContext &ctx,
                                  ConcreteSymbolTable &symbols) {
  const std::string &label = ctx.label;
  const std::string &operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".SE requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Check if symbol already exists (SE allows redefinition)
  int64_t existing_value = 0;
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
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
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
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
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
    return ApplyHighBitRule({c, delimiter});
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
  } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
    throw std::runtime_error("Failed to parse decimal number '" + trimmed +
                             "': " + e.what());
  }
}

const std::string &ScmasmSyntaxParser::LocalLabelScope(
    const std::string &label) const {
  // ':N' labels inside macros use the per-invocation scope so that multiple
  // expansions of the same macro under the same global label get unique names.
  if (!label.empty() && label[0] == ':' && macro_invocation_depth_ > 0) {
    return current_macro_label_scope_;
  }
  return last_global_label_;
}

std::string ScmasmSyntaxParser::ScopedLocalLabelName(
    const std::string &label) const {
  // Build the fully-scoped internal name for a local label.
  //
  // For '.N' dot-prefix labels: insert '@' between the global scope and the
  // label to prevent collision with global sub-labels that share the same
  // expanded name.  For example, local '.1' within the 'BITBLT' scope becomes
  // 'BITBLT@.1' internally, not 'BITBLT.1' — which is also a distinct global
  // sub-label in the same source file.  Without this separator the global
  // definition would overwrite the local one and branches that target the local
  // label would incorrectly resolve to the far-away global.
  //
  // For ':N' colon labels: the per-invocation macro scope already uniquifies
  // them, so no extra separator is needed (using ':' as-is is safe because ':N'
  // names cannot be mistaken for a user-defined global sub-label).
  const std::string &scope = LocalLabelScope(label);
  if (!scope.empty() && !label.empty() && label[0] == '.') {
    return scope + "@" + label;
  }
  return scope + label;
}

bool ScmasmSyntaxParser::IsLocalLabel(const std::string &label) {
  // Local labels are . or : followed by one or more digits
  // Examples: .0-.9 (single digit), .10, .70, .81, .98, .99 (multi-digit)
  if (label.length() >= 2 && (label[0] == '.' || label[0] == ':')) {
    for (size_t i = 1; i < label.length(); i++) {
      if (!std::isdigit(static_cast<unsigned char>(label[i]))) {
        return false;
      }
    }
    return true;
  }
  return false;
}

uint32_t ScmasmSyntaxParser::EvaluateExpression(const std::string &str,
                                                ConcreteSymbolTable &symbols) {
  std::string trimmed = Trim(str);

  // Pre-expand char literals (e.g. '*' → $2A, ' ' → $20) before the '*'
  // substitution loop so that '*' inside a quoted char literal is not
  // mistakenly treated as the current-address operator.
  trimmed = ExpandCharLiteralsInExpr(trimmed);

  // Handle * (current address) - replace with current address value
  // Handles all cases: "*", "*+4", "$1300-*", etc.
  // BUT NOT multiplication: "K.FD.MAX*2" — here * is a binary operator.
  // Rule: replace * only when it is NOT immediately preceded by an identifier
  //       character (alphanumeric, '.', '_', '?', '$').
  if (trimmed.find('*') != std::string::npos) {
    std::string expr_str = trimmed;
    size_t pos = 0;
    std::string star_replacement = std::to_string(current_address_);
    while ((pos = expr_str.find('*', pos)) != std::string::npos) {
      // Check the character immediately before this '*'
      bool preceded_by_ident = false;
      if (pos > 0) {
        char prev = expr_str[pos - 1];
        preceded_by_ident = std::isalnum(static_cast<unsigned char>(prev)) ||
                            prev == '.' || prev == '_' || prev == '?' ||
                            prev == '$';
      }
      if (preceded_by_ident) {
        // This * is multiplication — leave it alone
        pos++;
      } else {
        expr_str.replace(pos, 1, star_replacement);
        pos += star_replacement.length();
      }
    }
    // If the expression still contains an unresolved * after substitution, fall
    // through to the normal ExpressionParser path (handles SYMBOL*2 etc.)
    if (expr_str == star_replacement) {
      return current_address_;
    }
    // Otherwise, evaluate the transformed expression
    auto expr = ParseExpression(expr_str, symbols);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
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
      } catch (const std::exception &e) {
        (void)e; // Fall through to ExpressionParser
      }
    }

    // Character constant: single non-alphanumeric followed by a character
    // (delimiter + char, e.g., 'A, "A, #A, /A)
    if (trimmed.length() == 2 && !std::isalnum(trimmed[0]) &&
        trimmed[0] != '$' && trimmed[0] != '%') {
      try {
        return ParseNumber(trimmed);
      } catch (const std::exception &e) {
        (void)e; // Fall through to ExpressionParser
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

uint8_t ScmasmSyntaxParser::ApplyHighBitRule(HighBitChars hbc) {
  // SCMASM high-bit rule:
  // If delimiter ASCII < 0x27 (apostrophe '), high bit is SET
  // Otherwise, high bit is CLEAR

  auto result = static_cast<uint8_t>(hbc.input);

  if (hbc.delimiter < 0x27) {
    // Set high bit — explicit cast resolves narrowing: uint8_t |= unsigned
    result = static_cast<uint8_t>(static_cast<unsigned>(result) | 0x80U); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
  } else {
    // Clear high bit — explicit cast resolves narrowing: uint8_t &= unsigned
    result = static_cast<uint8_t>(static_cast<unsigned>(result) & 0x7FU); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
  }

  return result;
}

std::string ScmasmSyntaxParser::ExpandLocalLabelsInOperand(
    const std::string &operand) const {
  if (operand.empty()) {
    return operand;
  }
  // Skip expansion only when BOTH the global label scope AND the macro scope
  // are empty.  When inside a macro invocation, ':N' local labels are scoped
  // via current_macro_label_scope_ even if no global label has been set yet —
  // the old check `last_global_label_.empty()` caused ':N' macro-local forward
  // references to be silently left unexpanded (kept as ":2" instead of being
  // promoted to ":@1:2"), making the symbol lookup fail in EncodeInstructions
  // and the branch encode with offset = -2 (branch-to-self), producing an
  // infinite loop at runtime.
  if (last_global_label_.empty() && current_macro_label_scope_.empty()) {
    return operand;
  }

  std::string expanded;
  expanded.reserve(operand.size() + (last_global_label_.size() * 2));
  for (size_t k = 0; k < operand.size();) {
    char c = operand[k];
    bool at_word_start = // NOLINT(cppcoreguidelines-init-variables)
        (k == 0) || (!std::isalnum((unsigned char)operand[k - 1]) &&
                     operand[k - 1] != '_' && operand[k - 1] != '.');
    if ((c == '.' || c == ':') && at_word_start &&
        k + 1 < operand.size() &&
        std::isdigit((unsigned char)operand[k + 1])) {
      // Build the full label (e.g., ".10")
      std::string ref_label(1, c);
      size_t kk = k + 1;
      while (kk < operand.size() && std::isdigit((unsigned char)operand[kk])) {
        ref_label += operand[kk++];
      }
      expanded += LocalLabelScope(ref_label);
      if (c == '.') {
        expanded += '@';
      }
      expanded += c;
      k++;
      while (k < operand.size() && std::isdigit((unsigned char)operand[k])) {
        expanded += operand[k++];
      }
    } else {
      expanded += c;
      k++;
    }
  }
  return expanded;
}

std::string ScmasmSyntaxParser::ExpandCharLiteralsInExpr(
    const std::string &s) {
  // Replace SCMASM character literals ("X", 'X', "X", 'X) with their
  // numeric hex equivalents so that the generic ParseExpression engine can
  // evaluate expressions like #"0"+1 or #'A'.
  std::string result;
  result.reserve(s.size() * 2);
  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    if ((c == '"' || c == '\'') && i + 1 < s.size() &&
        std::isprint(static_cast<unsigned char>(s[i + 1]))) {
      // Only expand char literals that follow '#' or an arithmetic operator.
      // This prevents treating quoted strings in inline comments (e.g. the
      // operand text left over from  dey "/..";comment  after whitespace
      // stripping) as character literals.
      bool valid_context = false;
      if (i > 0) {
        char prev = s[i - 1];
        valid_context = (prev == '#' || prev == '+' || prev == '-' ||
                         prev == '*' || prev == '/' || prev == '^' ||
                         prev == '(' || prev == '<' || prev == '>');
      } else {
        // Position 0: expand only when both opening AND closing delimiter are
        // present (e.g., '*' or ' '), making it an unambiguous standalone
        // char literal operand (not a partial inline comment fragment).
        valid_context = (i + 2 < s.size() && s[i + 2] == c);
      }
      if (valid_context) {
        char delim = c;
        char ch = s[i + 1];
        size_t len = 2; // delimiter + char (no closing delimiter)
        if (i + 2 < s.size() && s[i + 2] == delim) {
          len = 3; // closing delimiter present: "X" or 'X'
        }
        uint8_t val = ApplyHighBitRule({ch, delim});
        char hex[8];
        snprintf(hex, sizeof(hex), "$%02X", val);
        result += hex;
        i += len;
        continue;
      }
    }
    result += c;
    i++;
  }
  return result;
}

std::shared_ptr<Expression>
ScmasmSyntaxParser::ParseExpression(const std::string &str,
                                    ConcreteSymbolTable &symbols) {
  // Phase 2: Use shared ExpressionParser with SCMASM number parser
  // Normalize expression to uppercase for case-insensitive symbol lookup
  std::string normalized_expr = util::ToUpper(str);

  // ADR-005: Canonicalise SCMASM-specific syntax before the shared parser
  // sees it, so the shared ExpressionParser stays syntax-agnostic.
  //
  // V3/V6 — Replace prefix '/' (SCMASM high-byte operator) with '>':
  //   "/ADDR" -> ">ADDR"
  normalized_expr = scmasm::CanonicalizeSlashHighByte(normalized_expr);
  //
  // V5 — Replace standalone '=' (SCMASM equality) with '==':
  //   "X=1"  -> "X==1"
  normalized_expr = scmasm::CanonicalizeEqualityOperator(normalized_expr);

  ExpressionParser parser(&symbols, &scmasm_number_parser_);
  return parser.Parse(normalized_expr);
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
    uint8_t byte = ApplyHighBitRule({c, delimiter});
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
  // SCMASM prefix modifiers before the opening delimiter:
  //   -"text"  set high bit on ALL bytes (Apple II normal-video encoding)
  // Strip the prefix and set flag for post-processing.
  std::string trimmed_op = Trim(operand);
  bool high_bit_all = false;
  if (!trimmed_op.empty() && trimmed_op[0] == '-') {
    high_bit_all = true;
    trimmed_op = Trim(trimmed_op.substr(1));
  }

  std::vector<uint8_t> data;
  ParseString(trimmed_op, data);

  if (high_bit_all) {
    for (auto &b : data) {
      b |= 0x80;
    }
  }

  // Add null terminator (always plain, no high bit)
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
    auto byte =
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

void ScmasmSyntaxParser::HandleMa(const DirectiveContext &ctx) {
  const std::string &label = ctx.label;
  const std::string &operand = ctx.operand;
  // Macro name can come from label or operand
  std::string macro_name;
  if (!label.empty()) {
    macro_name = label;
  } else if (!operand.empty()) { // NOLINT(bugprone-branch-clone)
    macro_name = Trim(operand);
  } else {
    throw std::runtime_error(".MA requires a macro name");
  }

  // Normalize macro name to uppercase for case-insensitive lookup
  std::transform(macro_name.begin(), macro_name.end(), macro_name.begin(),
                 ::toupper);

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

std::string ScmasmSyntaxParser::GetCurrentFile() const { return current_file_; }

void ScmasmSyntaxParser::SetCurrentFile(const std::string &file) {
  current_file_ = file;
}

void ScmasmSyntaxParser::InvokeMacro(const std::string &name,
                                     const std::vector<std::string> &params,
                                     Section &section,
                                     ConcreteSymbolTable &symbols) {
  // Check for infinite recursion
  if (macro_invocation_depth_ >= MAX_MACRO_NESTING_DEPTH) {
    throw std::runtime_error("Macro nesting too deep (max " +
                             std::to_string(MAX_MACRO_NESTING_DEPTH) +
                             " levels)");
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
  // Each invocation gets a unique scope for ':N' macro-local labels so that
  // multiple expansions of the same macro under the same global label do not
  // share/overwrite each other's :1, :2, etc. definitions.
  std::string saved_macro_scope = current_macro_label_scope_;
  current_macro_label_scope_ =
      last_global_label_ + ":@" + std::to_string(++macro_invocation_counter_);

  macro_invocation_depth_++;

  size_t line_idx = 0;
  while (line_idx < expanded_lines.size()) {
    try {
      ParseLine(expanded_lines[line_idx], section, symbols, expanded_lines,
                line_idx);
    } catch (const std::exception &e) {
      macro_invocation_depth_--;
      current_macro_label_scope_ = saved_macro_scope;
      throw std::runtime_error(std::string("In macro ") + name + ": " +
                               e.what());
    }
    line_idx++;
  }

  macro_invocation_depth_--;
  current_macro_label_scope_ = saved_macro_scope;
}

std::string ScmasmSyntaxParser::SubstituteParameters(
    const std::string &line, const std::vector<std::string> &params) {
  std::string result;
  size_t pos = 0;

  while (pos < line.length()) {
    // Handle SCMASM parameter syntax: ]N for parameter, ]# for count
    if (line[pos] == ']' && pos + 1 < line.length()) {
      char next = line[pos + 1];

      // ]# = parameter count
      if (next == '#') {
        result += std::to_string(params.size());
        pos += 2;
        continue;
      }

      // ]N = parameter N (1-based in SCMASM)
      if (next >= '1' && next <= '9') {
        int param_idx = next - '1'; // Convert to 0-based
        if (static_cast<size_t>(param_idx) < params.size()) {
          result += params[param_idx];
        }
        // If parameter not provided, substitute with empty string
        pos += 2;
        continue;
      }
    }

    // Handle alternative SCMASM parameter syntax: \0, \1, \2, etc. (0-based)
    if (line[pos] == '\\' && pos + 1 < line.length()) {
      char next = line[pos + 1];

      // \N = parameter N (0-based)
      if (next >= '0' && next <= '9') {
        int param_idx = next - '0'; // Already 0-based
        if (static_cast<size_t>(param_idx) < params.size()) {
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

void ScmasmSyntaxParser::HandleDo(const DirectiveContext &ctx,
                                  Section &section,
                                  ConcreteSymbolTable &symbols,
                                  const std::vector<std::string> &source,
                                  size_t &line_idx) {
  const std::string &label = ctx.label;
  const std::string &operand = ctx.operand;
  // .DO requires an expression
  if (operand.empty()) {
    throw std::runtime_error(".DO requires an expression");
  }
  
  // Bug B fix (off-by-1): Capture address at START of .DO line (before processing block)
  // The label on .DO should point to the first instruction after the .DO directive,
  // which is at the current PC before entering the block.
  uint32_t start_address = current_address_;
  // Also capture the current position in the atoms list, so we can insert the
  // LabelAtom at the correct position (before the block content, not after)
  size_t label_atom_position = section.atoms.size();
  
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

    if (line.empty()) {
      continue;
    }

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
  size_t start_line = 0;
  size_t end_line = 0;

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
      // But still need to define label if present
      if (!label.empty()) {
        if (IsLocalLabel(label)) {
          local_labels_[label] = current_address_;
          std::string scoped = ScopedLocalLabelName(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(scoped, SymbolType::Label, expr);
          auto atom = std::make_shared<LabelAtom>(scoped, current_address_);
          section.atoms.push_back(atom);
        } else {
          std::string norm = util::ToUpper(label);
          auto expr = std::make_shared<LiteralExpr>(current_address_);
          symbols.Define(norm, SymbolType::Label, expr);
          auto atom = std::make_shared<LabelAtom>(norm, current_address_);
          section.atoms.push_back(atom);
          last_global_label_ = norm;
        }
      }
      line_idx = fin_line;
      return;
    }
  }

  // If the .DO line has a global label, update the global label scope BEFORE
  // processing the block content.  This ensures that local labels inside the
  // .DO block (e.g. "bmi .1") are scoped to the .DO line's label, so they
  // match labels defined AFTER the .FIN (which see the same scope after HandleDo
  // sets last_global_label_ at the end).
  // Without this, "bmi .1" inside a labeled ".DO" block uses the *previous* global
  // scope, while ".1" after ".FIN" gets scoped to the .DO label — a mismatch that
  // causes the branch to be encoded with offset -2 (branch-to-self = FE).
  if (!label.empty() && !IsLocalLabel(label)) {
    last_global_label_ = util::ToUpper(label);
  }

  // Process lines in selected block
  for (size_t i = start_line; i < end_line;) {
    current_line_ = i + 1;
    std::string line = source[i];
    line = StripLineNumber(line);
    line = StripComments(line);
    line = TrimRight(line); // preserve leading whitespace for column detection

    if (!Trim(line).empty()) {
      size_t temp_idx = i;
      ParseLine(line, section, symbols, source, temp_idx);
      // If ParseLine skipped lines (e.g., nested .DO/.FIN), respect that
      i = temp_idx + 1;
    } else {
      i++;
    }
  }

  // Define any label on a boundary line (.ELSE or .FIN).
  // SCMASM allows labels on the same line as structural directives, e.g.:
  //   :2    .FIN   or   :1    .ELSE
  // The DO handler never calls ParseLine for the boundary line itself, so
  // labels on those lines would otherwise be silently dropped.
  auto define_boundary_label = [&](size_t boundary_idx) {
    if (boundary_idx >= source.size()) {
      return;
    }
    std::string bline = source[boundary_idx];
    bline = StripLineNumber(bline);
    bline = StripComments(bline);
    bline = Trim(bline);
    if (bline.empty()) {
      return;
    }
    // Extract the first whitespace-delimited token
    size_t lend = 0;
    while (lend < bline.length() && !std::isspace(bline[lend])) {
      lend++;
    }
    std::string blabel = bline.substr(0, lend);
    // If the first token starts with '.' and is NOT a local label (.N where N
    // is one-or-more digits), then it's the directive itself (.FIN, .ELSE) —
    // skip it.  Local labels like '.28' or '.8' on a .FIN/.ELSE line must be
    // defined at the current boundary address.
    if (blabel.empty() || (blabel[0] == '.' && !IsLocalLabel(blabel))) {
      return;
    }
    // Define the label at the current address
    if (IsLocalLabel(blabel)) {
      local_labels_[blabel] = current_address_;
      std::string scoped = ScopedLocalLabelName(blabel);
      auto expr = std::make_shared<LiteralExpr>(current_address_);
      symbols.Define(scoped, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(scoped, current_address_);
      section.atoms.push_back(atom);
    } else {
      std::string norm = util::ToUpper(blabel);
      auto expr = std::make_shared<LiteralExpr>(current_address_);
      symbols.Define(norm, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(norm, current_address_);
      section.atoms.push_back(atom);
      last_global_label_ = norm;
    }
  };

  // Define label on end_line (.ELSE when TRUE was taken, .FIN when ELSE was taken)
  define_boundary_label(end_line);

  // When the TRUE block was taken, the ELSE block is skipped entirely, so the
  // .FIN label is at the same address.  Define it too so that ELSE-block forward
  // references that happen to share the .FIN label still resolve correctly.
  if (condition != 0 && fin_line != end_line) {
    define_boundary_label(fin_line);
  }

  // Define the label from the .DO line itself (if present)
  // Bug B off-by-1 fix: The label on a .DO directive line should get the address
  // at the START of the .DO line (before entering the block), which is the address
  // of the first instruction following the .DO directive.
  // 
  // CRITICAL: Insert the LabelAtom at the position BEFORE the block content,
  // not at the end. Otherwise ResolveSymbols will compute the wrong address.
  if (!label.empty()) {
    if (IsLocalLabel(label)) {
      local_labels_[label] = start_address;
      std::string scoped = ScopedLocalLabelName(label);
      auto expr = std::make_shared<LiteralExpr>(start_address);
      symbols.Define(scoped, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(scoped, start_address);
      // Insert at the saved position (before block content), not at the end
      section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
    } else {
      std::string norm = util::ToUpper(label);
      auto expr = std::make_shared<LiteralExpr>(start_address);
      symbols.Define(norm, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(norm, start_address);
      // Insert at the saved position (before block content), not at the end
      section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
      last_global_label_ = norm;
    }
  }

  // Skip to after .FIN
  line_idx = fin_line;
}

void ScmasmSyntaxParser::HandleLu(const DirectiveContext &ctx,
                                  Section &section,
                                  ConcreteSymbolTable &symbols,
                                  const std::vector<std::string> &source,
                                  size_t &line_idx) {
  const std::string &label = ctx.label;
  const std::string &operand = ctx.operand;
  // .LU requires an expression
  if (operand.empty()) {
    throw std::runtime_error(".LU requires an expression");
  }
  
  // Bug B fix (off-by-1): Capture address at START of .LU line (before processing loop)
  // The label on .LU should point to the first instruction after the .LU directive,
  // which is at the current PC before entering the loop.
  uint32_t start_address = current_address_;
  // Also capture the current position in the atoms list, so we can insert the
  // LabelAtom at the correct position (before the loop content, not after)
  size_t label_atom_position = section.atoms.size();
  
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
    if (upper.starts_with(LU)) {
      nesting++;
    } else if (upper.starts_with(ENDU)) {
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
      line = TrimRight(line); // preserve leading whitespace for column detection

      if (!Trim(line).empty()) {
        size_t temp_idx = i;
        ParseLine(line, section, symbols, source, temp_idx);
        // If ParseLine skipped lines (e.g., nested .LU/.ENDU), respect that
        i = temp_idx + 1;
      } else {
        i++;
      }
    }
  }

  // Define the label from the .LU line itself (if present)
  // Bug B off-by-1 fix: The label on a .LU directive line should get the address
  // at the START of the .LU line (before entering the loop), which is the address
  // of the first instruction following the .LU directive.
  // 
  // CRITICAL: Insert the LabelAtom at the position BEFORE the loop content,
  // not at the end. Otherwise ResolveSymbols will compute the wrong address.
  if (!label.empty()) {
    if (IsLocalLabel(label)) {
      local_labels_[label] = start_address;
      std::string scoped = ScopedLocalLabelName(label);
      auto expr = std::make_shared<LiteralExpr>(start_address);
      symbols.Define(scoped, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(scoped, start_address);
      // Insert at the saved position (before loop content), not at the end
      section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
    } else {
      std::string norm = util::ToUpper(label);
      auto expr = std::make_shared<LiteralExpr>(start_address);
      symbols.Define(norm, SymbolType::Label, expr);
      auto atom = std::make_shared<LabelAtom>(norm, start_address);
      // Insert at the saved position (before loop content), not at the end
      section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
      last_global_label_ = norm;
    }
  }

  // Skip to after .ENDU
  line_idx = endu_line;
}

// ============================================================================
// Dummy Section Management
// ============================================================================

bool ScmasmSyntaxParser::InDummySection() const { return in_dummy_section_; }

void ScmasmSyntaxParser::StartDummySection(uint32_t current_address) {
  dummy_saved_address_ = current_address;
  in_dummy_section_ = true;
}

uint32_t ScmasmSyntaxParser::EndDummySection() {
  in_dummy_section_ = false;
  return dummy_saved_address_;
}

bool ScmasmSyntaxParser::InPhase() const { return in_phase_; }

void ScmasmSyntaxParser::StartPhase(PhaseAddresses addrs) {
  phase_real_addr_ = addrs.real_addr;
  phase_virtual_addr_ = addrs.virtual_addr;
  in_phase_ = true;
}

uint32_t ScmasmSyntaxParser::EndPhase(uint32_t current_virtual) {
  // Calculate how many bytes were emitted during phase
  uint32_t phase_size = current_virtual - phase_virtual_addr_;

  // Calculate new real address
  uint32_t new_real_addr = phase_real_addr_ + phase_size;

  // Clear phase state
  in_phase_ = false;
  phase_virtual_addr_ = 0;
  phase_real_addr_ = 0;

  return new_real_addr;
}

uint32_t ScmasmSyntaxParser::GetPhaseVirtualAddress() const {
  return phase_virtual_addr_;
}

uint32_t ScmasmSyntaxParser::GetPhaseRealAddress() const {
  return phase_real_addr_;
}

uint32_t
ScmasmSyntaxParser::GetCurrentRealAddress(uint32_t current_virtual) const {
  if (!in_phase_) {
    return current_virtual; // Not in phase, virtual == real
  }

  // Calculate bytes emitted during phase
  uint32_t phase_size = current_virtual - phase_virtual_addr_;

  // Real address = saved real address + bytes emitted
  return phase_real_addr_ + phase_size;
}

// ============================================================================
// Directive Registry
// ============================================================================

} // namespace xasm
