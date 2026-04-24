/**
 * @file scmasm_syntax.cpp
 * @brief S-C Macro Assembler (SCMASM) syntax parser implementation
 *
 * Implementation of SCMASM syntax parser for xasm++.
 * Phase 2: Integrated with shared ExpressionParser
 * Phase 6c.2: Handler extraction with free functions
 */

#include "xasm++/syntax/scmasm_syntax.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/cpu_constants.h"
#include "xasm++/cpu/cpu_plugin.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/directives/scmasm_directive_handlers.h"
#include "xasm++/syntax/label_policy.h"
#include "xasm++/syntax/scmasm_expression_utils.h"
#include "xasm++/util/string_utils.h"  // For ToUpper

namespace xasm {

namespace {

// Radix values for number parsing
constexpr int kRADIX_BINARY = 2;
constexpr int kRADIX_DECIMAL = 10;
constexpr int kRADIX_HEXADECIMAL = 16;

// ---------------------------------------------------------------------------
// IsEqOperandSafe  (used by StripComments to validate star-label .kEQ values)
//
// Returns true if the operand can be resolved without consulting the symbol
// table — i.e. it contains only numeric literals, `*` (current address), and
// arithmetic operators.  Returns false if it contains:
//   • A bare symbol name  (e.g. "K.FClose"  — would be a forward reference)
//   • An invalid character inside a hex literal  (e.g. "$Cn5C" where 'n' is
//     not a hex digit)
//
// When this returns false, StripComments treats the *LABEL .kEQ line as an
// ordinary full-line comment.
// ---------------------------------------------------------------------------
// Advances i past a hex literal (digits only). Returns false if no hex digits.
// Pre: s[i-1] == '$' and i is positioned at the first potential hex digit.
static bool ScanHexLiteralDigits(const std::string& s, size_t& i) {
  bool any_digit = false;
  while (i < s.size() && std::isxdigit(static_cast<unsigned char>(s[i]))) {
    any_digit = true;
    ++i;
  }
  return any_digit;
}

// Advances i past a binary literal (0/1/. digits). Returns false if no digits.
// Pre: s[i-1] == '%' and i is positioned at the first potential bit char.
static bool ScanBinaryLiteralDigits(const std::string& s, size_t& i) {
  bool any_bit = false;
  while (i < s.size() && (s[i] == '0' || s[i] == '1' || s[i] == '.')) {
    any_bit = true;
    ++i;
  }
  return any_bit;
}

// Advances i past consecutive decimal digit characters.
static void ScanDecimalDigits(const std::string& s, size_t& i) {
  while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
    ++i;
  }
}

bool IsEqOperandSafe(const std::string& operand) {
  // Trim leading/trailing whitespace
  size_t start = operand.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return false;  // empty operand — treat as unsafe
  }
  size_t end = operand.find_last_not_of(" \t");
  std::string s = operand.substr(start, end - start + 1);

  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    ++i;

    if (c == '$') {
      if (i >= s.size() || !ScanHexLiteralDigits(s, i)) {
        return false;  // bare '$' or invalid hex
      }
    } else if (c == '%') {
      if (i >= s.size() || !ScanBinaryLiteralDigits(s, i)) {
        return false;  // bare '%' or invalid binary
      }
    } else if (std::isdigit(static_cast<unsigned char>(c))) {
      ScanDecimalDigits(s, i);
    } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      return false;  // unquoted symbol reference → unsafe
    }
    // Operators, parens, '#', ',', '*', '+', '-', '/', whitespace: safe
  }

  return true;
}

}  // anonymous namespace

// ============================================================================
// SCMASMNumberParser Implementation
// ============================================================================

// Tries to parse a hexadecimal literal (token starting with '$').
// Returns false if the format is invalid.
static bool TryParseHex(const std::string& token, int64_t& value) {
  if (token.length() < 2) {
    return false;
  }
  std::string hex = token.substr(1);
  for (char c : hex) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  try {
    value = std::stoll(hex, nullptr, kRADIX_HEXADECIMAL);
    return true;
  } catch (...) {
    return false;
  }
}

// Tries to parse a binary literal (token starting with '%').
// Returns false if the format is invalid.
static bool TryParseBinary(const std::string& token, int64_t& value) {
  if (token.length() < 2) {
    return false;
  }
  std::string binary = token.substr(1);
  for (char c : binary) {
    if (c != '0' && c != '1' && c != '.') {
      return false;
    }
  }
  binary.erase(std::remove(binary.begin(), binary.end(), '.'), binary.end());
  if (binary.empty()) {
    return false;
  }
  try {
    value = std::stoll(binary, nullptr, kRADIX_BINARY);
    return true;
  } catch (...) {
    return false;
  }
}

// Tries to parse an kASCII character constant (delimiter + char, length==2).
// Returns false if not a char constant format.
static bool TryParseCharLiteral(const std::string& token, int64_t& value) {
  if (std::isalnum(static_cast<unsigned char>(token[0])) || token.length() != 2) {
    return false;
  }
  // Apply high-bit rule: delimiter < 0x27 → set high bit, else clear it.
  auto result = static_cast<uint8_t>(token[1]);
  if (token[0] < 0x27) {
    result |= 0x80;
  } else {
    result &= 0x7F;
  }
  value = result;
  return true;
}

// Tries to parse a decimal literal (all digits).
// Returns false if any non-digit character is present.
static bool TryParseDecimal(const std::string& token, int64_t& value) {
  if (!std::isdigit(static_cast<unsigned char>(token[0]))) {
    return false;
  }
  for (char c : token) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  try {
    value = std::stoll(token, nullptr, kRADIX_DECIMAL);
    return true;
  } catch (...) {
    return false;
  }
}

bool SCMASMNumberParser::TryParse(const std::string& token, int64_t& value) const {
  if (token.empty()) {
    return false;
  }
  if (token[0] == '$') {
    return TryParseHex(token, value);
  }
  if (token[0] == '%') {
    return TryParseBinary(token, value);
  }
  if (TryParseCharLiteral(token, value)) {
    return true;
  }
  return TryParseDecimal(token, value);
}

// ============================================================================
// Constructor
// ============================================================================

ScmasmSyntaxParser::ScmasmSyntaxParser() {
  InitializeDirectiveRegistry();
}

void ScmasmSyntaxParser::InitializeDirectiveRegistry() {
  // Phase 6c.2: Use extracted free functions with directive name constants
  using namespace scmasm::directives;

  // .kOR - Set origin address
  directive_registry_[kOR] = scmasm::HandleOr;

  // .kEQ - Define constant
  directive_registry_[kEQ] = scmasm::HandleEq;

  // .kSE - Set variable (redefinable)
  directive_registry_[kSE] = scmasm::HandleSe;

  // .kAS - kASCII string
  directive_registry_[kAS] = scmasm::HandleAs;

  // .kAT - kASCII text (high bit on last char)
  directive_registry_[kAT] = scmasm::HandleAt;

  // .kAZ - kASCII zero-terminated
  directive_registry_[kAZ] = scmasm::HandleAz;

  // .kDA / .kDFB - Define byte(s)
  directive_registry_[kDA] = scmasm::HandleDa;
  directive_registry_[kDFB] = scmasm::HandleDa;  // Alias

  // .kHS - Hex string
  directive_registry_[kHS] = scmasm::HandleHs;

  // .kBS - Binary string
  directive_registry_[kBS] = scmasm::HandleBs;

  // .kMA - Begin macro definition
  directive_registry_[kMA] = scmasm::HandleMa;

  // .kENDM / .kEM - End macro definition
  directive_registry_[kENDM] = scmasm::HandleEndm;
  directive_registry_[kEM] = scmasm::HandleEndm;  // Alias

  // P0 Priority Directives (A2oSX Critical)
  directive_registry_[kPS] = scmasm::HandlePs;        // Pascal string
  directive_registry_[kINB] = scmasm::HandleInb;      // Include source file
  directive_registry_[kINCLUDE] = scmasm::HandleInb;  // Alias: .kINCLUDE = .kINB
  directive_registry_[kLIST] = scmasm::HandleList;    // Listing control
  directive_registry_[kDUMMY] = scmasm::HandleDummy;  // Dummy section
  directive_registry_[kED] = scmasm::HandleEd;        // End dummy section
  directive_registry_[kOP] = scmasm::HandleOp;        // kCPU operation mode

  // Phase 3: 100% Coverage Directives
  directive_registry_[kCS] = scmasm::HandleCs;  // C-string with escapes
  directive_registry_[kCZ] = scmasm::HandleCz;  // C-string zero-terminated
  directive_registry_[kTF] = scmasm::HandleTf;  // Text file/title metadata
  directive_registry_[kEP] = scmasm::HandleEp;  // Entry point / end phase
  directive_registry_[kPH] = scmasm::HandlePh;  // Phase assembly
  directive_registry_[kHX] = scmasm::HandleHx;  // Hex nibble storage
  directive_registry_[kTA] = scmasm::HandleTa;  // Target address (no-op)
  directive_registry_[kAC] = scmasm::HandleAc;  // kASCII with prefix

  // Note: Control flow directives (.kDO, .kELSE, .kFIN, .kLU, .kENDU) are NOT
  // registered here because they require special handling in ParseLine with
  // line skipping and nested scoping. They cannot be dispatched via the simple
  // registry pattern.
}

// ============================================================================
// kCPU Plugin Configuration
// ============================================================================

void ScmasmSyntaxParser::SetCpu(CpuPlugin* cpu) {
  cpu_ = cpu;
}

void ScmasmSyntaxParser::SetCpu(const std::string& cpu_name) {
  // Allocate a Cpu6502 instance owned by this parser.
  // The pointer is stored in owned_cpu_ so that it outlives this function
  // and is freed when the parser is destroyed.  Previously cpu6502 was a
  // local variable, causing cpu_ to become a dangling pointer and crash
  // the first time an instruction was assembled after a .kOP directive.
  auto cpu6502 = std::make_unique<Cpu6502>();

  // Set the appropriate kCPU mode based on the requested variant
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

void ScmasmSyntaxParser::SetIncludePaths(const std::vector<std::string>& paths) {
  include_paths_ = paths;
}

void ScmasmSyntaxParser::SetPathMappings(const std::map<std::string, std::string>& mappings) {
  path_mappings_.clear();

  // Normalize all path mapping keys to use forward slashes for cross-platform
  // compatibility
  for (const auto& [virtual_path, actual_path] : mappings) {
    // Manually replace backslashes with forward slashes
    std::string normalized_virtual = virtual_path;
    std::replace(normalized_virtual.begin(), normalized_virtual.end(), '\\', '/');

    path_mappings_[normalized_virtual] = actual_path;
  }
}

// ============================================================================
// Main Parse Function
// ============================================================================

void ScmasmSyntaxParser::Parse(const std::string& source, Section& section,
                               ConcreteSymbolTable& symbols) {
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

  current_line_ = 0;
  size_t line_idx = 0;
  while (line_idx < lines.size()) {
    current_line_ = line_idx + 1;
    ProcessOneLine(lines[line_idx], section, symbols, lines, line_idx);
    line_idx++;
  }

  FlushPendingLabel(section, symbols);

  // Check for unclosed macro definition
  if (in_macro_definition_) {
    throw std::runtime_error("Unclosed macro definition: " + current_macro_name_);
  }
}

// Strips trailing whitespace characters from a line.
static std::string StripTrailingWhitespace(const std::string& line) {
  size_t end = line.size();
  while (end > 0 && (line[end - 1] == ' ' || line[end - 1] == '\t' || line[end - 1] == '\r' ||
                     line[end - 1] == '\n')) {
    --end;
  }
  return line.substr(0, end);
}

// Process a single raw source line. Increments line_idx only for macro lines.
void ScmasmSyntaxParser::ProcessOneLine(const std::string& raw, Section& section,
                                        ConcreteSymbolTable& symbols,
                                        const std::vector<std::string>& lines, size_t& line_idx) {
  std::string line = StripLineNumber(raw);
  line = StripComments(line);
  line = StripEditorCommands(line);

  if (Trim(line).empty()) {
    return;
  }

  line = StripTrailingWhitespace(line);

  if (in_macro_definition_) {
    HandleMacroBodyLine(line, line_idx);
    return;
  }

  try {
    ParseLine(line, section, symbols, lines, line_idx);
  } catch (const std::exception& e) {
    const std::string msg = FormatError(e.what());
    throw std::runtime_error(msg);
  }
}

// Handles a line received while inside a .kMA/.kENDM block.
void ScmasmSyntaxParser::HandleMacroBodyLine(const std::string& line, size_t& line_idx) {
  std::string upper_line = Trim(line);
  std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);
  if (upper_line.starts_with(".EM") || upper_line.starts_with(".ENDM")) {
    HandleEm();
    return;
  }
  current_macro_body_.push_back(line);
  (void)line_idx;  // line_idx still incremented by the outer loop
}

// Flushes any pending label accumulated at end-of-file.
void ScmasmSyntaxParser::FlushPendingLabel(Section& section, ConcreteSymbolTable& symbols) {
  if (pending_label_.empty()) {
    return;
  }
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

// ============================================================================
// Helper Functions
// ============================================================================

std::string ScmasmSyntaxParser::StripLineNumber(const std::string& line) {
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

// ============================================================================
// TryParse helpers for StripComments
// ============================================================================

// Helper: handle the inner body of a *LABEL directive line.
// Returns true (always), setting result to the stripped line or "" for comment.
// Called only when line[0] == '*' and line[1] is alpha.
// ============================================================================
// TryHandlePrivateLabelEq helpers
// ============================================================================

// Returns label length (>= 1) or 0 if label chars are invalid.
// Scans from line[1] (after the leading '*').
static size_t ScanPrivateLabelLength(const std::string& line) {
  size_t pos = 1;
  while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t') {
    char c = line[pos];
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '.' && c != '_') {
      return 0;  // invalid char
    }
    ++pos;
  }
  return pos - 1;  // number of label chars scanned
}

// Scan the opcode token starting after label_end whitespace.
// Returns the uppercased opcode token, or "" if absent / not a dot-directive.
static std::string ExtractOpcodeFromStarLine(const std::string& line, size_t label_end) {
  size_t op = label_end + 1;  // skip leading '*'
  while (op < line.size() && (line[op] == ' ' || line[op] == '\t')) {
    ++op;
  }
  if (op >= line.size() || line[op] != '.') {
    return "";
  }
  size_t end = op;
  while (end < line.size() && line[end] != ' ' && line[end] != '\t') {
    ++end;
  }
  std::string tok = line.substr(op, end - op);
  for (char& ch : tok) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }
  return tok;
}

// Extract operand string (after opcode whitespace) trimmed of trailing ; comment.
static std::string ExtractOperandFromStarLine(const std::string& line, size_t label_end) {
  size_t op = label_end + 1;
  while (op < line.size() && (line[op] == ' ' || line[op] == '\t')) {
    ++op;
  }
  // skip opcode token
  while (op < line.size() && line[op] != ' ' && line[op] != '\t') {
    ++op;
  }
  while (op < line.size() && (line[op] == ' ' || line[op] == '\t')) {
    ++op;
  }
  std::string operand = line.substr(op);
  size_t semi = operand.find(';');
  if (semi != std::string::npos) {
    operand = operand.substr(0, semi);
  }
  return operand;
}

bool ScmasmSyntaxParser::TryHandlePrivateLabelEq(const std::string& line, std::string& result) {
  size_t label_len = ScanPrivateLabelLength(line);
  if (label_len == 0) {
    result = "";
    return true;
  }
  size_t label_end = label_len;  // position past the last label char

  std::string opcode = ExtractOpcodeFromStarLine(line, label_end);
  if (opcode != ".EQ" && opcode != ".SE") {
    result = "";
    return true;
  }

  std::string operand = ExtractOperandFromStarLine(line, label_end);
  if (!IsEqOperandSafe(operand)) {
    result = "";
    return true;
  }

  // Strip the leading '*' — label starts at position 1.
  result = line.substr(1);
  return true;
}

bool ScmasmSyntaxParser::TryHandleAsteriskLine(const std::string& line, size_t first_non_space,
                                               std::string& result) {
  if (first_non_space == std::string::npos || line[first_non_space] != '*') {
    return false;
  }
  if (first_non_space == 0 && line.size() > 1 &&
      std::isalpha(static_cast<unsigned char>(line[1]))) {
    return TryHandlePrivateLabelEq(line, result);
  }
  result = "";  // Entire line is comment
  return true;
}

// ============================================================================

// Finds the position of a semicolon comment delimiter outside quoted strings.
// Returns std::string::npos if no semicolon comment found.
static size_t FindSemicolonComment(const std::string& line) {
  bool in_string = false;
  char string_delim = 0;
  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];
    if (in_string) {
      // SCMASM: delimiters are not escapable, so e.g. '\"' ends the string.
      if (c == string_delim) {
        in_string = false;
      }
    } else if (c == '"' || c == '\'') {
      in_string = true;
      string_delim = c;
    } else if (c == ';') {
      return i;
    }
  }
  return std::string::npos;
}

std::string ScmasmSyntaxParser::StripComments(const std::string& line) {
  // Two comment styles:
  // 1. * in column 1 (full-line comment)
  // 2. ; anywhere outside a string literal (rest of line is comment)

  // Check for * in column 1 (after any leading whitespace)
  //
  // SCMASM private-label marker: *LABEL .kEQ value
  // In SCMASM, a line starting with *<labelchar> at column 0 is NOT a full
  // comment — the * is a private/reserved label marker that the assembler
  // still processes (defining the label), only the opcode emission is
  // suppressed for real instructions.  .kEQ/.kSE directives are zero-emission
  // anyway, so *LABEL .kEQ value fully defines the label.
  //
  // Rule:
  //   * at column 0 followed by a valid label-start char → strip * and
  //     process the rest of the line as a normal source line.
  //   * anywhere else (column 0 + non-label char, or after whitespace) →
  //     full-line comment (return empty).
  size_t first_non_space = line.find_first_not_of(" \t");
  {
    std::string asterisk_result;
    if (TryHandleAsteriskLine(line, first_non_space, asterisk_result)) {
      return asterisk_result;
    }
  }

  // Strip semicolon comment, respecting quoted strings.
  size_t semi = FindSemicolonComment(line);
  return (semi != std::string::npos) ? line.substr(0, semi) : line;
}

// Returns true if a rest-of-line operand looks like an assembly operand
// (rather than a file path), meaning the LOAD/SAVE command should not be stripped.
static bool LooksLikeAsmOperand(const std::string& rest_of_line) {
  if (rest_of_line.empty()) {
    return false;
  }
  char first_char = rest_of_line[0];
  return first_char == '#' || first_char == '$' || first_char == '(' || first_char == '[' ||
         first_char == '<' || first_char == '>';
}

// Returns true if rest_of_line looks like a file path (contains slash/backslash).
static bool LooksLikeFilePath(const std::string& rest_of_line) {
  return rest_of_line.find('/') != std::string::npos ||
         rest_of_line.find('\\') != std::string::npos;
}

std::string ScmasmSyntaxParser::StripEditorCommands(const std::string& line) {
  // Apple II line editor commands that should be ignored during assembly
  // Format: command at start of line (case-insensitive), optionally followed by
  // arguments
  static const std::unordered_set<std::string> kEDITOR_COMMANDS = {"NEW",  "AUTO", "MAN",    "SAVE",
                                                                  "LOAD", "ASM",  "DELETE", "LIST"};

  // Find first non-whitespace/control character
  // Include backspace (\b) which appears before MAN in some files
  size_t first_non_space = line.find_first_not_of(" \t\r\n\b");
  if (first_non_space == std::string::npos) {
    return line;  // Empty line
  }

  // Build uppercased first token
  size_t token_end = first_non_space;
  while (token_end < line.length() && !std::isspace(line[token_end])) {
    token_end++;
  }
  std::string upper_token = line.substr(first_non_space, token_end - first_non_space);
  for (char& c : upper_token) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  if (!kEDITOR_COMMANDS.contains(upper_token)) {
    return line;
  }

  // For LOAD/SAVE, only strip when operand looks like a file path.
  if (upper_token == "LOAD" || upper_token == "SAVE") {
    std::string rest = (token_end < line.length()) ? Trim(line.substr(token_end)) : "";
    if (!rest.empty() && LooksLikeAsmOperand(rest)) {
      return line;  // Assembly operand — don't strip
    }
    return rest.empty() || LooksLikeFilePath(rest) ? "" : line;
  }

  return "";  // All other editor commands — always strip
}

// Strip trailing whitespace only, preserving leading whitespace.
// Used when preparing lines for ParseLine so that the original column
// positions are preserved (leading whitespace distinguishes column-0 labels
// from mnemonic-column opcodes).
static std::string TrimRight(const std::string& str) {
  size_t end = str.size();
  while (end > 0 && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' ||
                     str[end - 1] == '\n')) {
    --end;
  }
  return str.substr(0, end);
}

std::string ScmasmSyntaxParser::Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }

  // Strip leading non-printable non-tab control chars (Apple II editor
  // artifacts like \x01 SOH that sometimes precede instruction lines).
  while (start < str.length() && (unsigned char)str[start] < 0x20 && str[start] != '\t') {
    start++;
  }
  if (start >= str.length()) {
    return "";
  }

  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

std::string ScmasmSyntaxParser::FormatError(const std::string& message) const {
  std::ostringstream oss;
  oss << current_file_ << ":" << current_line_ << ": " << message;
  return oss.str();
}

// ============================================================================
// Line Parsing
// ============================================================================

// ============================================================================
// TryParse helpers for ParseLine
// ============================================================================

// Build a DirectiveContext from parser state for a registry dispatch.
static DirectiveContext MakeDirectiveContext(
    ScmasmSyntaxParser* parser_state, Section* section, ConcreteSymbolTable* symbols,
    uint32_t* current_address, const std::string& current_file, int current_line,
    const std::vector<std::string>* include_paths,
    const std::map<std::string, std::string>* path_mappings, const std::string& label,
    const std::string& operand) {
  DirectiveContext ctx;
  ctx.section = section;
  ctx.symbols = symbols;
  ctx.current_address = current_address;
  ctx.parser_state = parser_state;
  ctx.current_file = current_file;
  ctx.current_line = current_line;
  ctx.include_paths = include_paths;
  ctx.path_mappings = path_mappings;
  ctx.label = label;
  ctx.operand = operand;
  return ctx;
}

// Returns true if opcode is a control-flow directive that must not appear
// without a matching opening directive (mismatched kELSE/kFIN/kENDU).
static bool IsMismatchedControlFlow(const std::string& opcode) {
  using namespace scmasm::directives;
  return opcode == kELSE || opcode == kFIN || opcode == kENDU;
}

// Dispatch control-flow directives that are not in the registry.
// Returns true if handled, false if the opcode is not a control-flow directive.
bool ScmasmSyntaxParser::TryDispatchControlFlow(const std::string& opcode_upper,
                                                const std::string& operand,
                                                const std::string& label, Section& section,
                                                ConcreteSymbolTable& symbols,
                                                const std::vector<std::string>& source,
                                                size_t& line_idx) {
  using namespace scmasm::directives;
  if (opcode_upper == kDO) {
    std::string do_operand = operand;
    size_t ws = do_operand.find_first_of(" \t");
    if (ws != std::string::npos) {
      do_operand = do_operand.substr(0, ws);
    }
    DirectiveContext do_ctx;
    do_ctx.label = label;
    do_ctx.operand = do_operand;
    HandleDo(do_ctx, section, symbols, source, line_idx);
    return true;
  }
  if (opcode_upper == kLU) {
    DirectiveContext lu_ctx;
    lu_ctx.label = label;
    lu_ctx.operand = operand;
    HandleLu(lu_ctx, section, symbols, source, line_idx);
    return true;
  }
  if (IsMismatchedControlFlow(opcode_upper)) {
    throw std::runtime_error("Mismatched " + opcode_upper);
  }
  return false;
}

// Set of data-emitting directives that need a LabelAtom for address tracking.
static const std::unordered_set<std::string_view>& DataEmittingDirectives() {
  static const std::unordered_set<std::string_view> kSet = {
      ".DA", ".DB", ".DFB", ".DW", ".DS", ".DC",  ".HB", ".HX",  ".AS",   ".AT", ".AZ",
      ".CS", ".CZ", ".TF",  ".PS", ".HS", ".STR", ".BS", ".BYT", ".WORD", ".PH",
  };
  return kSet;
}

// Define the label for a directive, using scmasm namespace scoping rules.
void ScmasmSyntaxParser::DefineLabelForDirectiveSCMASM(const std::string& opcode_upper,
                                                       const std::string& label, Section& section,
                                                       ConcreteSymbolTable& symbols) {
  auto scmasm_scope_fn = [this](const std::string& lbl) -> std::string {
    return IsLocalLabel(lbl) ? ScopedLocalLabelName(lbl) : util::ToUpper(lbl);
  };
  LabelPolicy policy = ClassifyLabelPolicy(opcode_upper, !label.empty(), in_dummy_section_);
  bool emit_atom = DataEmittingDirectives().contains(opcode_upper) && !in_dummy_section_;
  DefineLabelForDirective(label, current_address_, policy, emit_atom, symbols, section,
                          local_labels_, last_global_label_, scmasm_scope_fn,
                          ScmasmSyntaxParser::IsLocalLabel);
}

bool ScmasmSyntaxParser::TryHandleDirectiveLine(const std::string& opcode_upper,
                                                const std::string& operand,
                                                const std::string& label, Section& section,
                                                ConcreteSymbolTable& symbols,
                                                const std::vector<std::string>& source,
                                                size_t& line_idx) {
  if (opcode_upper.empty() || opcode_upper[0] != '.') {
    return false;
  }

  DefineLabelForDirectiveSCMASM(opcode_upper, label, section, symbols);

  using namespace scmasm::directives;
  if ((opcode_upper == ".EQ" || opcode_upper == ".SE") && label.empty()) {
    throw std::runtime_error(opcode_upper + " requires a label");
  }

  if (TryDispatchControlFlow(opcode_upper, operand, label, section, symbols, source, line_idx)) {
    return true;
  }

  // Dispatch via registry
  auto it = directive_registry_.find(opcode_upper);
  if (it == directive_registry_.end()) {
    throw std::runtime_error("Unknown directive: " + opcode_upper);
  }
  std::string directive_operand = ExpandLocalLabelsInOperand(operand);
  DirectiveContext context = MakeDirectiveContext(this, &section, &symbols, &current_address_,
                                                  current_file_, current_line_, &include_paths_,
                                                  &path_mappings_, label, directive_operand);
  it->second(context);
  return true;
}

// Parse comma-separated macro arguments from operand string.
// Arguments stop at whitespace unless followed by a comma (SCMASM convention).
static std::vector<std::string> ParseMacroArgs(const std::string& operand) {
  std::vector<std::string> params;
  size_t pos = 0;
  while (pos < operand.length()) {
    while (pos < operand.length() && (operand[pos] == ' ' || operand[pos] == '\t')) {
      ++pos;
    }
    if (pos >= operand.length()) {
      break;
    }
    size_t arg_start = pos;
    while (pos < operand.length() && operand[pos] != ' ' && operand[pos] != '\t' &&
           operand[pos] != ',') {
      ++pos;
    }
    std::string param = operand.substr(arg_start, pos - arg_start);
    if (!param.empty()) {
      params.push_back(param);
    }
    if (pos < operand.length() && operand[pos] == ',') {
      ++pos;  // comma → another argument follows
    } else {
      break;  // whitespace/end → rest is inline comment
    }
  }
  return params;
}

bool ScmasmSyntaxParser::TryHandleMacroLine(const std::string& opcode_upper,
                                            const std::string& operand, Section& section,
                                            ConcreteSymbolTable& symbols) {
  // SCMASM uses >MacroName syntax for invocation; bare name also allowed.
  std::string macro_lookup_name = opcode_upper;
  if (!opcode_upper.empty() && opcode_upper[0] == '>') {
    macro_lookup_name = opcode_upper.substr(1);
  }

  auto it = macros_.find(macro_lookup_name);
  if (it == macros_.end()) {
    if (!opcode_upper.empty() && opcode_upper[0] == '>') {
      throw std::runtime_error("Undefined macro: " + macro_lookup_name);
    }
    return false;
  }

  std::vector<std::string> params =
      operand.empty() ? std::vector<std::string>{} : ParseMacroArgs(operand);
  InvokeMacro(macro_lookup_name, params, section, symbols);
  return true;
}

// Strip inline assembler comments: everything after the first whitespace that
// is outside a quoted string ('x' / "x").  Modifies operand in-place.
static void StripInlineComment(std::string& operand) {
  bool in_quote = false;
  char quote_ch = 0;
  for (size_t k = 0; k < operand.size(); ++k) {
    char ch = operand[k];
    if (in_quote) {
      if (ch == quote_ch) {
        in_quote = false;
      }
    } else if (ch == '\'' || ch == '"') {
      in_quote = true;
      quote_ch = ch;
    } else if (ch == ' ' || ch == '\t') {
      operand = operand.substr(0, k);
      return;
    }
  }
}

// Estimate instruction size, applying ZP correction when symbol is known.
// EvalFn: evaluates an expression string and returns its value; throws on failure.
using EvalFn = std::function<uint32_t(const std::string&)>;

static size_t EstimateInstrSize(const std::string& opcode, const std::string& operand,
                                CpuPlugin* cpu, const EvalFn& eval) {
  size_t est = cpu->GetInstructionSize(opcode, operand);
  // GetInstructionSize assumes absolute (3-byte) for symbol operands.  For
  // many 6502 mnemonics a ZP form exists (2 bytes).  Evaluate the operand
  // with the current symbol table; if it resolves to $00–$kFF, correct to 2.
  // Exclusions: JSR and JMP have no ZP form — always 3.
  if (est == 3 && opcode != "JSR" && opcode != "JMP") {
    // Strip index suffix (,X or ,Y) so the base expression evaluates.
    std::string eval_expr = operand;
    auto comma = eval_expr.rfind(',');
    if (comma != std::string::npos) {
      eval_expr = eval_expr.substr(0, comma);
    }
    try {
      uint32_t val = eval(eval_expr);
      if (val <= 0xFF) {
        est = 2;
      }
    } catch (...) {
      // Forward reference or unevaluable — keep est=3.
      static_cast<void>(0);
    }
  }
  return est;
}

void ScmasmSyntaxParser::HandleInstructionLine(const std::string& opcode_upper,
                                               const std::string& operand, Section& section,
                                               ConcreteSymbolTable& symbols) {
  std::string instr_operand = operand;
  StripInlineComment(instr_operand);

  instr_operand = ExpandCharLiteralsInExpr(instr_operand);
  instr_operand = ExpandLocalLabelsInOperand(instr_operand);

  if (!in_dummy_section_) {
    auto instr_atom = std::make_shared<InstructionAtom>(opcode_upper, instr_operand);
    section.atoms.push_back(instr_atom);
  }

  if (cpu_) {
    auto eval = [this, &symbols](const std::string& expr) -> uint32_t {
      return EvaluateExpression(expr, symbols);
    };
    current_address_ += EstimateInstrSize(opcode_upper, instr_operand, cpu_, eval);
  }
}

// ============================================================================
// ParseLine helpers
// ============================================================================

void ScmasmSyntaxParser::FlushPendingLabelAsAddress(Section& section,
                                                    ConcreteSymbolTable& symbols) {
  if (pending_label_.empty()) {
    return;
  }
  std::string pl = std::move(pending_label_);
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

// ============================================================================

void ScmasmSyntaxParser::ParseLine(const std::string& line, Section& section,
                                   ConcreteSymbolTable& symbols,
                                   const std::vector<std::string>& source, size_t& line_idx) {
  size_t pos = 0;

  // Parse label (if present)
  std::string label = ParseLabel(line, pos, section, symbols);

  // Skip whitespace after label
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }

  // If nothing left (just a label, nothing else), defer it as pending_label_.
  // It may be the label for a .kEQ/.kSE on the next line (SCMASM pattern where
  // the label appears on a separate line before the .kEQ directive).
  // Otherwise it will be defined at the next instruction's address.
  if (pos >= line.length()) {
    if (!label.empty()) {
      // Flush any previous pending label first (consecutive label-only lines
      // all share the same address as whatever instruction follows).
      FlushPendingLabelAsAddress(section, symbols);
      pending_label_ = label;  // defer; don't define yet
    }
    return;
  }

  // Parse opcode/directive
  size_t opcode_start = pos;
  while (pos < line.length() && !std::isspace(line[pos])) {
    pos++;
  }
  std::string opcode = line.substr(opcode_start, pos - opcode_start);

  // Skip whitespace after opcode, counting tabs.
  // In SCMASM/Merlin 3-column format, a SINGLE tab separates mnemonic from
  // operand.  When 2+ tabs appear between the mnemonic and the next text, the
  // text is a visual-alignment comment and the operand column is empty.
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
    operand = Trim(line.substr(pos));
  }

  // Convert opcode to uppercase for comparison
  std::string opcode_upper = opcode;
  std::transform(opcode_upper.begin(), opcode_upper.end(), opcode_upper.begin(), ::toupper);

  // Resolve pending label from previous label-only line.
  ResolvePendingLabel(opcode_upper, label, section, symbols);

  // Handle directives (must start with .)
  if (TryHandleDirectiveLine(opcode_upper, operand, label, section, symbols, source, line_idx)) {
    return;
  }

  // Not a directive — define label and create label atom for instructions/macros.
  {
    auto scmasm_scope_fn = [this](const std::string& lbl) -> std::string {
      return IsLocalLabel(lbl) ? ScopedLocalLabelName(lbl) : util::ToUpper(lbl);
    };
    DefineLabelForDirective(label, current_address_, LabelPolicy::AtPc, !in_dummy_section_, symbols,
                            section, local_labels_, last_global_label_, scmasm_scope_fn,
                            ScmasmSyntaxParser::IsLocalLabel);
  }

  // Try macro invocation first, then fall through to instruction handling
  if (!TryHandleMacroLine(opcode_upper, operand, section, symbols)) {
    HandleInstructionLine(opcode_upper, operand, section, symbols);
  }
}

void ScmasmSyntaxParser::ResolvePendingLabel(const std::string& opcode_upper, std::string& label,
                                             Section& section, ConcreteSymbolTable& symbols) {
  if (pending_label_.empty()) {
    return;
  }
  bool used_for_eq = (label.empty() && (opcode_upper == ".EQ" || opcode_upper == ".SE"));
  if (used_for_eq) {
    label = pending_label_;
  } else {
    std::string pl = pending_label_;
    auto scmasm_scope_fn = [this](const std::string& lbl) -> std::string {
      return IsLocalLabel(lbl) ? ScopedLocalLabelName(lbl) : util::ToUpper(lbl);
    };
    DefineLabelForDirective(pl, current_address_, LabelPolicy::AtPc, !in_dummy_section_, symbols,
                            section, local_labels_, last_global_label_, scmasm_scope_fn,
                            ScmasmSyntaxParser::IsLocalLabel);
  }
  pending_label_ = "";
}

// ============================================================================
// ParseLabel helpers
// ============================================================================

// Advance pos past leading whitespace and Apple II control chars (0x01-0x1F).
void ScmasmSyntaxParser::SkipToLabelStart(const std::string& line, size_t& pos) {
  while (pos < line.length() && std::isspace(line[pos])) {
    pos++;
  }
  while (pos < line.length() && (unsigned char)line[pos] < 0x20 && line[pos] != '\t') {
    pos++;
  }
}

// Scan the label body starting at pos. Handles optional '&' prefix.
// Returns the label token (excluding '&'); advances pos past it.
// Returns "" and resets pos if label is syntactically invalid.
std::string ScmasmSyntaxParser::ScanLabelToken(const std::string& line, size_t& pos) {
  const size_t label_start = pos;
  bool had_ampersand = (line[pos] == '&');
  if (had_ampersand) {
    ++pos;
    if (pos >= line.length() || !std::isalpha(line[pos])) {
      pos = label_start;
      return "";
    }
  }
  while (pos < line.length() &&
         (std::isalnum(line[pos]) || line[pos] == '_' || line[pos] == '.' || line[pos] == ':')) {
    pos++;
  }
  size_t name_start = had_ampersand ? label_start + 1 : label_start;
  return line.substr(name_start, pos - name_start);
}

// Returns true if a label token at a non-zero column is actually an opcode/macro.
bool ScmasmSyntaxParser::IsOpcodeOrMacro(const std::string& label_upper) const {
  if (macros_.contains(label_upper)) {
    return true;
  }
  if (cpu_ != nullptr && cpu_->HasOpcode(label_upper)) {
    return true;
  }
  static const std::unordered_set<std::string> kPseudoOps = {"DB", "DW", "DS"};
  return kPseudoOps.contains(label_upper);
}

// Returns true if ch is a valid first character for a SCMASM label.
// Valid start chars: letter, '.', ':', '_', '&' (macro-label prefix).
static bool IsValidLabelStartChar(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) || ch == '.' || ch == ':' || ch == '_' ||
         ch == '&';
}

// Returns true if a dot/colon-prefixed token contains alpha chars after the
// prefix — meaning it is a directive token, not a local numeric label.
// Local labels (e.g. ".0", ".99") contain only digits after the prefix.
static bool IsDirectiveNotLocalLabel(const std::string& label) {
  if (label.length() < 2 || (label[0] != '.' && label[0] != ':')) {
    return false;
  }
  for (size_t i = 1; i < label.length(); i++) {
    if (std::isalpha(static_cast<unsigned char>(label[i]))) {
      return true;
    }
  }
  return false;
}

std::string ScmasmSyntaxParser::ParseLabel(const std::string& line, size_t& pos,
                                           Section& /*section*/, ConcreteSymbolTable& /*symbols*/) {
  SkipToLabelStart(line, pos);

  // Labels must start with letter, ., :, _, or & (SCMASM & macro-label prefix)
  if (pos >= line.length() || !IsValidLabelStartChar(line[pos])) {
    return "";
  }

  const size_t label_start = pos;
  std::string label = ScanLabelToken(line, pos);
  if (label.empty()) {
    return "";
  }

  // Directives start with '.'/':', local labels are digit-only after the prefix.
  if (IsDirectiveNotLocalLabel(label)) {
    pos = label_start;
    return "";
  }

  // Column-0 tokens are always labels (SCMASM convention). Only check for
  // opcode/macro names when the token is in the mnemonic column.
  if (label_start > 0) {
    std::string label_upper = label;
    std::transform(label_upper.begin(), label_upper.end(), label_upper.begin(), ::toupper);
    if (IsOpcodeOrMacro(label_upper)) {
      pos = label_start;
      return "";
    }
  }

  // Label must be followed by whitespace (or end of line)
  if (pos < line.length() && !std::isspace(line[pos])) {
    pos = label_start;
    return "";
  }

  return label;
}

// ============================================================================
// Directive Handlers
// ============================================================================

void ScmasmSyntaxParser::HandleOr(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& symbols) {
  if (operand.empty()) {
    throw std::runtime_error(".OR requires an address");
  }

  // Parse address (with expression evaluation)
  uint32_t address = EvaluateExpression(operand, symbols);

  // Create kORG atom
  auto org_atom = std::make_shared<OrgAtom>(address);
  section.atoms.push_back(org_atom);

  // Update current address
  current_address_ = address;
}

void ScmasmSyntaxParser::HandleEq(const DirectiveContext& ctx, ConcreteSymbolTable& symbols) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".EQ requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Define symbol (immutable) - .kEQ creates Equate type
  auto expr = std::make_shared<LiteralExpr>(value);
  symbols.Define(label, SymbolType::Equate, expr);
}

void ScmasmSyntaxParser::HandleSe(const DirectiveContext& ctx, ConcreteSymbolTable& symbols) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".SE requires a value");
  }

  // Parse value (with expression evaluation)
  uint32_t value = EvaluateExpression(operand, symbols);

  // Check if symbol already exists (kSE allows redefinition)
  int64_t existing_value = 0;
  auto expr = std::make_shared<LiteralExpr>(value);

  if (symbols.Lookup(label, existing_value)) {
    // Redefine it - .kSE creates Set type (redefinable)
    symbols.Define(label, SymbolType::Set, expr);
  } else {
    // First definition - .kSE creates Set type (redefinable)
    symbols.Define(label, SymbolType::Set, expr);
    variable_symbols_[label] = true;
  }
}

// ============================================================================
// Number Parsing helpers (one per format, CC ≤ 10 each)
// ============================================================================

uint32_t ScmasmSyntaxParser::ParseHexNumber(const std::string& trimmed) {
  // trimmed starts with '$'
  std::string hex = trimmed.substr(1);
  if (hex.empty()) {
    throw std::runtime_error("Invalid hex number (no digits after $): " + trimmed);
  }
  for (char c : hex) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      throw std::runtime_error("Invalid hex digit '" + std::string(1, c) + "' in: " + trimmed);
    }
  }
  try {
    return std::stoul(hex, nullptr, kRADIX_HEXADECIMAL);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse hex number '" + trimmed + "': " + e.what());
  }
}

uint32_t ScmasmSyntaxParser::ParseBinaryNumber(const std::string& trimmed) {
  // trimmed starts with '%'
  std::string binary = trimmed.substr(1);
  if (binary.empty()) {
    throw std::runtime_error("Invalid binary number (no digits after %): " + trimmed);
  }
  for (char c : binary) {
    if (c != '0' && c != '1' && c != '.') {
      throw std::runtime_error("Invalid binary digit '" + std::string(1, c) + "' in: " + trimmed);
    }
  }
  binary.erase(std::remove(binary.begin(), binary.end(), '.'), binary.end());
  if (binary.empty()) {
    throw std::runtime_error("Binary number has no digits: " + trimmed);
  }
  try {
    return std::stoul(binary, nullptr, kRADIX_BINARY);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse binary number '" + trimmed + "': " + e.what());
  }
}

uint32_t ScmasmSyntaxParser::ParseCharConstant(const std::string& trimmed) {
  // Exactly 2 chars: delimiter + kASCII char
  char delimiter = trimmed[0];
  char c = trimmed[1];
  return ApplyHighBitRule({c, delimiter});
}

uint32_t ScmasmSyntaxParser::ParseDecimalNumber(const std::string& trimmed) {
  if (!std::isdigit(static_cast<unsigned char>(trimmed[0]))) {
    throw std::runtime_error("Not a valid number: " + trimmed);
  }
  for (char c : trimmed) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw std::runtime_error("Invalid decimal digit '" + std::string(1, c) + "' in: " + trimmed);
    }
  }
  try {
    return std::stoul(trimmed, nullptr, kRADIX_DECIMAL);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse decimal number '" + trimmed + "': " + e.what());
  }
}

// ============================================================================
// Number Parsing — dispatch wrapper
// ============================================================================

uint32_t ScmasmSyntaxParser::ParseNumber(const std::string& str) {
  std::string trimmed = Trim(str);
  if (trimmed.empty()) {
    throw std::runtime_error("Empty number string");
  }
  if (trimmed[0] == '$') {
    return ParseHexNumber(trimmed);
  }
  if (trimmed[0] == '%') {
    return ParseBinaryNumber(trimmed);
  }
  // kASCII char constant: non-digit + exactly one char
  if (!std::isdigit(static_cast<unsigned char>(trimmed[0])) && trimmed.length() == 2) {
    return ParseCharConstant(trimmed);
  }
  return ParseDecimalNumber(trimmed);
}

const std::string& ScmasmSyntaxParser::LocalLabelScope(const std::string& label) const {
  // ':N' labels inside macros use the per-invocation scope so that multiple
  // expansions of the same macro under the same global label get unique names.
  if (!label.empty() && label[0] == ':' && macro_invocation_depth_ > 0) {
    return current_macro_label_scope_;
  }
  return last_global_label_;
}

std::string ScmasmSyntaxParser::ScopedLocalLabelName(const std::string& label) const {
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
  const std::string& scope = LocalLabelScope(label);
  if (!scope.empty() && !label.empty() && label[0] == '.') {
    return scope + "@" + label;
  }
  return scope + label;
}

bool ScmasmSyntaxParser::IsLocalLabel(const std::string& label) {
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

// Replace bare '*' (current-address) tokens in expr_str with replacement.
// '*' preceded by an identifier character is treated as multiplication and
// left alone.
static std::string SubstituteCurrentAddress(const std::string& expr_str,
                                            const std::string& replacement) {
  std::string out = expr_str;
  size_t pos = 0;
  while ((pos = out.find('*', pos)) != std::string::npos) {
    bool preceded_by_ident = false;
    if (pos > 0) {
      char prev = out[pos - 1];
      preceded_by_ident = std::isalnum(static_cast<unsigned char>(prev)) || prev == '.' ||
                          prev == '_' || prev == '?' || prev == '$';
    }
    if (preceded_by_ident) {
      ++pos;
    } else {
      out.replace(pos, 1, replacement);
      pos += replacement.length();
    }
  }
  return out;
}

uint32_t ScmasmSyntaxParser::EvaluateExpression(const std::string& str,
                                                ConcreteSymbolTable& symbols) {
  std::string trimmed = Trim(str);

  // Pre-expand char literals before the '*' substitution loop.
  trimmed = ExpandCharLiteralsInExpr(trimmed);

  // Handle * (current address) — replace when not used as multiplication.
  if (trimmed.find('*') != std::string::npos) {
    std::string star_rep = std::to_string(current_address_);
    std::string expr_str = SubstituteCurrentAddress(trimmed, star_rep);
    if (expr_str == star_rep) {
      return current_address_;
    }
    return static_cast<uint32_t>(ParseExpression(expr_str, symbols)->Evaluate(symbols));
  }

  // Handle local labels (.0-.9)
  if (IsLocalLabel(trimmed)) {
    auto it = local_labels_.find(trimmed);
    if (it != local_labels_.end()) {
      return it->second;
    }
    throw std::runtime_error("Undefined local label: " + trimmed);
  }

  // SCMASM-specific number formats not handled by ExpressionParser.
  if (!trimmed.empty()) {
    if (trimmed[0] == '%' && trimmed.find('.') != std::string::npos) {
      try {
        return ParseNumber(trimmed);
      } catch (...) {
        // Not a valid SCMASM binary literal — fall through to ExpressionParser
        static_cast<void>(0);
      }
    }
    if (trimmed.length() == 2 && !std::isalnum(trimmed[0]) && trimmed[0] != '$' &&
        trimmed[0] != '%') {
      try {
        return ParseNumber(trimmed);
      } catch (...) {
        // Not a valid 2-char literal — fall through to ExpressionParser
        static_cast<void>(0);
      }
    }
  }

  auto expr = ParseExpression(trimmed, symbols);
  return static_cast<uint32_t>(expr->Evaluate(symbols));
}

uint8_t ScmasmSyntaxParser::ApplyHighBitRule(HighBitChars hbc) {
  // SCMASM high-bit rule:
  // If delimiter kASCII < 0x27 (apostrophe '), high bit is kSET
  // Otherwise, high bit is CLEAR

  auto result = static_cast<uint8_t>(hbc.input);

  if (hbc.delimiter < 0x27) {
    // Set high bit — explicit cast resolves narrowing: uint8_t |= unsigned
    result = static_cast<uint8_t>(static_cast<unsigned>(result) | 0x80U);
  } else {
    // Clear high bit — explicit cast resolves narrowing: uint8_t &= unsigned
    result = static_cast<uint8_t>(static_cast<unsigned>(result) & 0x7FU);
  }

  return result;
}

// Expand one local-label reference starting at operand[k].
// Returns the expanded text and advances k past the reference.
// Returns "" (without advancing k) if not a local-label reference.
static std::string TryExpandLocalRef(const std::string& operand, size_t& k,
                                     const std::string& scope_fn_result_dot,
                                     const std::string& scope_fn_result_colon) {
  char c = operand[k];
  bool at_word_start = (k == 0) || (!std::isalnum((unsigned char)operand[k - 1]) &&
                                    operand[k - 1] != '_' && operand[k - 1] != '.');
  if (!at_word_start || k + 1 >= operand.size() || !std::isdigit((unsigned char)operand[k + 1])) {
    return "";
  }
  // Consume label like ".10" or ":5"
  std::string ref_label(1, c);
  size_t kk = k + 1;
  while (kk < operand.size() && std::isdigit((unsigned char)operand[kk])) {
    ref_label += operand[kk++];
  }
  const std::string& scope_result = (c == '.') ? scope_fn_result_dot : scope_fn_result_colon;
  std::string out = scope_result;
  if (c == '.') {
    out += '@';
  }
  out += c;
  k++;
  while (k < operand.size() && std::isdigit((unsigned char)operand[k])) {
    out += operand[k++];
  }
  return out;
}

std::string ScmasmSyntaxParser::ExpandLocalLabelsInOperand(const std::string& operand) const {
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

  // Pre-compute scope strings for '.' and ':' refs.
  // (LocalLabelScope uses different logic for each prefix.)
  std::string scope_dot = LocalLabelScope(".");
  std::string scope_colon = LocalLabelScope(":");

  std::string expanded;
  expanded.reserve(operand.size() + (last_global_label_.size() * 2));
  for (size_t k = 0; k < operand.size();) {
    char c = operand[k];
    if (c == '.' || c == ':') {
      std::string sub = TryExpandLocalRef(operand, k, scope_dot, scope_colon);
      if (!sub.empty()) {
        expanded += sub;
        continue;
      }
    }
    expanded += c;
    k++;
  }
  return expanded;
}

// Return true when character at s[i] appears in a context that could be a
// SCMASM character literal (preceded by arithmetic op/# or at position 0 with
// matching closing delimiter).
static bool IsCharLiteralContext(const std::string& s, size_t i) {
  if (i > 0) {
    char prev = s[i - 1];
    return (prev == '#' || prev == '+' || prev == '-' || prev == '*' || prev == '/' ||
            prev == '^' || prev == '(' || prev == '<' || prev == '>');
  }
  // Position 0: only when both delimiters present (e.g., '*' or ' ').
  return (i + 2 < s.size() && s[i + 2] == s[i]);
}

std::string ScmasmSyntaxParser::ExpandCharLiteralsInExpr(const std::string& s) {
  // Replace SCMASM character literals ("X", 'X', "X", 'X) with their
  // numeric hex equivalents so that the generic ParseExpression engine can
  // evaluate expressions like #"0"+1 or #'A'.
  std::string result;
  result.reserve(s.size() * 2);
  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    bool is_quote = (c == '"' || c == '\'');
    if (is_quote && i + 1 < s.size() && std::isprint(static_cast<unsigned char>(s[i + 1])) &&
        IsCharLiteralContext(s, i)) {
      char delim = c;
      char ch = s[i + 1];
      size_t len = (i + 2 < s.size() && s[i + 2] == delim) ? 3 : 2;
      uint8_t val = ApplyHighBitRule({ch, delim});
      char hex[8];
      snprintf(hex, sizeof(hex), "$%02X", val);
      result += hex;
      i += len;
    } else {
      result += c;
      ++i;
    }
  }
  return result;
}

std::shared_ptr<Expression> ScmasmSyntaxParser::ParseExpression(const std::string& str,
                                                                ConcreteSymbolTable& symbols) {
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

char ScmasmSyntaxParser::ParseString(const std::string& operand, std::vector<uint8_t>& result) {
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

void ScmasmSyntaxParser::HandleAs(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& /*symbols*/) {
  std::vector<uint8_t> data;
  ParseString(operand, data);

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleAt(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& /*symbols*/) {
  std::vector<uint8_t> data;
  ParseString(operand, data);

  // Set high bit on LAST character
  if (!data.empty()) {
    data.back() |= 0x80;
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleAz(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& /*symbols*/) {
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
    for (auto& b : data) {
      b |= 0x80;
    }
  }

  // Add null terminator (always plain, no high bit)
  data.push_back(0x00);

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

// Emit bytes for a single .kDA value token according to its prefix character.
void ScmasmSyntaxParser::EmitDaValue(const std::string& value_trimmed, std::vector<uint8_t>& data,
                                     ConcreteSymbolTable& symbols) {
  char prefix = value_trimmed[0];
  if (prefix == '#') {
    // 8-bit: low byte only
    uint32_t num = EvaluateExpression(Trim(value_trimmed.substr(1)), symbols);
    data.push_back(static_cast<uint8_t>(num & 0xFF));
  } else if (prefix == '/') {
    // 8-bit: second byte (bits 8-15)
    uint32_t num = EvaluateExpression(Trim(value_trimmed.substr(1)), symbols);
    data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
  } else if (prefix == '<') {
    // 24-bit: three bytes (little-endian)
    uint32_t num = EvaluateExpression(Trim(value_trimmed.substr(1)), symbols);
    data.push_back(static_cast<uint8_t>(num & 0xFF));
    data.push_back(static_cast<uint8_t>((num >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((num >> 16) & 0xFF));
  } else if (prefix == '>') {
    // 32-bit: four bytes (little-endian)
    uint32_t num = EvaluateExpression(Trim(value_trimmed.substr(1)), symbols);
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

void ScmasmSyntaxParser::HandleDa(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& symbols) {
  std::vector<uint8_t> data;

  // Split comma-separated operand into individual value tokens.
  // SCMASM .kDA: Size determined by operator prefix
  // #expr → 8-bit (low byte)
  // /expr → 8-bit (second byte, bits 8-15)
  // expr  → 16-bit (default, little-endian)
  // <expr → 24-bit (little-endian)
  // >expr → 32-bit (little-endian)
  std::vector<std::string> values;
  std::string trimmed = Trim(operand);
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

  for (const auto& val : values) {
    std::string value_trimmed = Trim(val);
    if (value_trimmed.empty()) {
      continue;
    }
    EmitDaValue(value_trimmed, data, symbols);
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);

  // Update address counter
  current_address_ += data.size();
}

void ScmasmSyntaxParser::HandleHs(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& /*symbols*/) {
  std::vector<uint8_t> data;

  std::string trimmed = Trim(operand);

  // Remove all whitespace
  std::string hex_digits;
  for (char c : trimmed) {
    if (!std::isspace(c)) {
      if (!std::isxdigit(c)) {
        throw std::runtime_error("Invalid hex digit in .HS: " + std::string(1, c));
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
    auto byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, kRADIX_HEXADECIMAL));
    data.push_back(byte);
  }

  auto atom = std::make_shared<DataAtom>(data);
  section.atoms.push_back(atom);
}

void ScmasmSyntaxParser::HandleBs(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& symbols) {
  // .kBS (Block Storage) - Reserve N bytes of space
  // SCMASM syntax: .kBS count
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

void ScmasmSyntaxParser::HandleMa(const DirectiveContext& ctx) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  // Macro name can come from label or operand
  std::string macro_name;
  if (!label.empty()) {
    macro_name = label;
  } else if (!operand.empty()) {
    macro_name = Trim(operand);
  } else {
    throw std::runtime_error(".MA requires a macro name");
  }

  // Normalize macro name to uppercase for case-insensitive lookup
  std::transform(macro_name.begin(), macro_name.end(), macro_name.begin(), ::toupper);

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

std::string ScmasmSyntaxParser::GetCurrentFile() const {
  return current_file_;
}

void ScmasmSyntaxParser::SetCurrentFile(const std::string& file) {
  current_file_ = file;
}

void ScmasmSyntaxParser::InvokeMacro(const std::string& name,
                                     const std::vector<std::string>& params, Section& section,
                                     ConcreteSymbolTable& symbols) {
  // Check for infinite recursion
  if (macro_invocation_depth_ >= kMAX_MACRO_NESTING_DEPTH) {
    throw std::runtime_error("Macro nesting too deep (max " +
                             std::to_string(kMAX_MACRO_NESTING_DEPTH) + " levels)");
  }

  // Find macro
  auto it = macros_.find(name);
  if (it == macros_.end()) {
    throw std::runtime_error("Undefined macro: " + name);
  }

  const MacroDef& macro = it->second;

  // Expand macro body - substitute parameters first
  std::vector<std::string> expanded_lines;
  for (const auto& line : macro.lines) {
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
      ParseLine(expanded_lines[line_idx], section, symbols, expanded_lines, line_idx);
    } catch (const std::exception& e) {
      macro_invocation_depth_--;
      current_macro_label_scope_ = saved_macro_scope;
      throw std::runtime_error(std::string("In macro ") + name + ": " + e.what());
    }
    line_idx++;
  }

  macro_invocation_depth_--;
  current_macro_label_scope_ = saved_macro_scope;
}

// Try to substitute "]N" or "]#" parameter references.
// Returns chars consumed (0 = not a substitution), appends to result.
static size_t TrySubstituteSquare(const std::string& line, size_t pos,
                                  const std::vector<std::string>& params, std::string& result) {
  if (line[pos] != ']' || pos + 1 >= line.length()) {
    return 0;
  }
  char next = line[pos + 1];
  if (next == '#') {
    result += std::to_string(params.size());
    return 2;
  }
  if (next >= '1' && next <= '9') {
    int idx = next - '1';
    if (static_cast<size_t>(idx) < params.size()) {
      result += params[idx];
    }
    return 2;
  }
  return 0;
}

// Try to substitute "\N" parameter references (0-based).
// Returns chars consumed (0 = not a substitution), appends to result.
static size_t TrySubstituteBackslash(const std::string& line, size_t pos,
                                     const std::vector<std::string>& params, std::string& result) {
  if (line[pos] != '\\' || pos + 1 >= line.length()) {
    return 0;
  }
  char next = line[pos + 1];
  if (next >= '0' && next <= '9') {
    int idx = next - '0';
    if (static_cast<size_t>(idx) < params.size()) {
      result += params[idx];
    }
    return 2;
  }
  return 0;
}

std::string ScmasmSyntaxParser::SubstituteParameters(const std::string& line,
                                                     const std::vector<std::string>& params) {
  std::string result;
  size_t pos = 0;

  while (pos < line.length()) {
    size_t consumed = TrySubstituteSquare(line, pos, params, result);
    if (consumed == 0) {
      consumed = TrySubstituteBackslash(line, pos, params, result);
    }
    if (consumed > 0) {
      pos += consumed;
    } else {
      result += line[pos++];
    }
  }

  return result;
}

// ============================================================================
// HandleDo / HandleLu helpers
// ============================================================================

// Extract the opcode token from a (pre-trimmed) source line.
// Returns the uppercased opcode (the 2nd whitespace-delimited token, since the
// first may be a label).
static std::string ExtractUpperOpcode(const std::string& ln) {
  size_t pos = 0;
  // Skip possible label token
  while (pos < ln.length() && !std::isspace(ln[pos])) {
    ++pos;
  }
  while (pos < ln.length() && std::isspace(ln[pos])) {
    ++pos;
  }
  size_t start = pos;
  while (pos < ln.length() && !std::isspace(ln[pos])) {
    ++pos;
  }
  std::string tok = ln.substr(start, pos - start);
  std::transform(tok.begin(), tok.end(), tok.begin(), ::toupper);
  return tok;
}

// Extract the uppercased first token from ln.
static std::string ExtractFirstToken(const std::string& ln) {
  size_t end = ln.find(' ');
  std::string tok = (end == std::string::npos) ? ln : ln.substr(0, end);
  std::transform(tok.begin(), tok.end(), tok.begin(), ::toupper);
  return tok;
}

ScmasmSyntaxParser::DoBlockBounds ScmasmSyntaxParser::FindDoBlockBounds(
    const std::vector<std::string>& source, size_t start_idx) {
  DoBlockBounds bounds{std::string::npos, std::string::npos};
  int nesting = 1;

  for (size_t i = start_idx + 1; i < source.size(); ++i) {
    std::string ln = Trim(StripComments(StripLineNumber(source[i])));
    if (ln.empty()) {
      continue;
    }

    std::string first_tok = ExtractFirstToken(ln);
    std::string directive = ExtractUpperOpcode(ln);

    using namespace scmasm::directives;
    if (first_tok == kDO || directive == kDO) {
      nesting++;
    } else if ((first_tok == kELSE || directive == kELSE) && nesting == 1) {
      bounds.else_line = i;
    } else if (first_tok == kFIN || directive == kFIN) {
      nesting--;
      if (nesting == 0) {
        bounds.fin_line = i;
        break;
      }
    }
  }
  return bounds;
}

void ScmasmSyntaxParser::EmitDoLabel(const std::string& label, uint32_t addr,
                                     size_t atom_insert_pos, Section& section,
                                     ConcreteSymbolTable& symbols) {
  if (label.empty()) {
    return;
  }
  if (IsLocalLabel(label)) {
    local_labels_[label] = addr;
    std::string scoped = ScopedLocalLabelName(label);
    auto expr = std::make_shared<LiteralExpr>(addr);
    symbols.Define(scoped, SymbolType::Label, expr);
    auto atom = std::make_shared<LabelAtom>(scoped, addr);
    section.atoms.insert(section.atoms.begin() + atom_insert_pos, atom);
  } else {
    std::string norm = util::ToUpper(label);
    auto expr = std::make_shared<LiteralExpr>(addr);
    symbols.Define(norm, SymbolType::Label, expr);
    auto atom = std::make_shared<LabelAtom>(norm, addr);
    section.atoms.insert(section.atoms.begin() + atom_insert_pos, atom);
    last_global_label_ = norm;
  }
}

void ScmasmSyntaxParser::DefineBoundaryLabel(const std::vector<std::string>& source,
                                             size_t boundary_idx, Section& section,
                                             ConcreteSymbolTable& symbols) {
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
  // If the first token starts with '.' and is NOT a local label (.N digits),
  // it's the directive itself (.kFIN, .kELSE) — skip it.
  if (blabel.empty() || (blabel[0] == '.' && !IsLocalLabel(blabel))) {
    return;
  }
  // Define the label at current_address_
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
}

void ScmasmSyntaxParser::HandleDo(const DirectiveContext& ctx, Section& section,
                                  ConcreteSymbolTable& symbols,
                                  const std::vector<std::string>& source, size_t& line_idx) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".DO requires an expression");
  }

  // Capture address and atom-insert position at start of .kDO line.
  uint32_t start_address = current_address_;
  size_t label_atom_position = section.atoms.size();

  uint32_t condition = EvaluateExpression(operand, symbols);

  // Find matching .kELSE / .kFIN
  auto bounds = FindDoBlockBounds(source, line_idx);
  if (bounds.fin_line == std::string::npos) {
    throw std::runtime_error(".DO without matching .FIN");
  }
  size_t else_line = bounds.else_line;
  size_t fin_line = bounds.fin_line;

  // Determine which block to process
  size_t start_line = 0;
  size_t end_line = 0;

  if (condition != 0) {
    start_line = line_idx + 1;
    end_line = (else_line != std::string::npos) ? else_line : fin_line;
  } else if (else_line != std::string::npos) {
    start_line = else_line + 1;
    end_line = fin_line;
  } else {
    // No .kELSE, skip entire block; just emit any .kDO-line label.
    EmitDoLabel(label, current_address_, section.atoms.size(), section, symbols);
    line_idx = fin_line;
    return;
  }

  // Update global label scope before processing the block.
  if (!label.empty() && !IsLocalLabel(label)) {
    last_global_label_ = util::ToUpper(label);
  }

  // Process lines in the selected block.
  for (size_t i = start_line; i < end_line;) {
    current_line_ = i + 1;
    std::string ln = source[i];
    ln = StripLineNumber(ln);
    ln = StripComments(ln);
    ln = TrimRight(ln);
    if (!Trim(ln).empty()) {
      size_t temp_idx = i;
      ParseLine(ln, section, symbols, source, temp_idx);
      i = temp_idx + 1;
    } else {
      i++;
    }
  }

  // Define labels on boundary lines (.kELSE / .kFIN).
  DefineBoundaryLabel(source, end_line, section, symbols);
  if (condition != 0 && fin_line != end_line) {
    DefineBoundaryLabel(source, fin_line, section, symbols);
  }

  // Emit the .kDO-line label at start_address (before block content).
  EmitDoLabel(label, start_address, label_atom_position, section, symbols);

  line_idx = fin_line;
}

size_t ScmasmSyntaxParser::FindEnduBounds(const std::vector<std::string>& source,
                                          size_t start_idx) const {
  using namespace scmasm::directives;
  int nesting = 1;
  for (size_t i = start_idx + 1; i < source.size(); ++i) {
    std::string ln = Trim(StripComments(StripLineNumber(source[i])));
    std::string upper = ln;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper.starts_with(kLU)) {
      nesting++;
    } else if (upper.starts_with(kENDU) && --nesting == 0) {
      return i;
    }
  }
  return std::string::npos;
}

// Define label (local or global) at start_address; insert LabelAtom at pos.
static void DefineLuLabel(const std::string& label, uint32_t start_address,
                          size_t label_atom_position, Section& section,
                          ConcreteSymbolTable& symbols,
                          std::unordered_map<std::string, uint32_t>& local_labels,
                          std::string& last_global, bool is_local_fn(const std::string&),
                          const std::function<std::string(const std::string&)>& scope_fn) {
  if (is_local_fn(label)) {
    local_labels[label] = start_address;
    std::string scoped = scope_fn(label);
    symbols.Define(scoped, SymbolType::Label, std::make_shared<LiteralExpr>(start_address));
    auto atom = std::make_shared<LabelAtom>(scoped, start_address);
    section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
  } else {
    std::string norm = util::ToUpper(label);
    symbols.Define(norm, SymbolType::Label, std::make_shared<LiteralExpr>(start_address));
    auto atom = std::make_shared<LabelAtom>(norm, start_address);
    section.atoms.insert(section.atoms.begin() + label_atom_position, atom);
    last_global = norm;
  }
}

void ScmasmSyntaxParser::HandleLu(const DirectiveContext& ctx, Section& section,
                                  ConcreteSymbolTable& symbols,
                                  const std::vector<std::string>& source, size_t& line_idx) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  if (operand.empty()) {
    throw std::runtime_error(".LU requires an expression");
  }

  uint32_t start_address = current_address_;
  size_t label_atom_position = section.atoms.size();
  uint32_t count = EvaluateExpression(operand, symbols);

  // Find matching .kENDU (handles nesting).
  size_t endu_line = FindEnduBounds(source, line_idx);
  if (endu_line == std::string::npos) {
    throw std::runtime_error(".LU without matching .ENDU");
  }

  size_t loop_start = line_idx + 1;
  size_t loop_end = endu_line;

  for (uint32_t iteration = 0; iteration < count; ++iteration) {
    for (size_t i = loop_start; i < loop_end;) {
      current_line_ = i + 1;
      std::string ln = TrimRight(StripComments(StripLineNumber(source[i])));
      if (!Trim(ln).empty()) {
        size_t temp_idx = i;
        ParseLine(ln, section, symbols, source, temp_idx);
        i = temp_idx + 1;
      } else {
        i++;
      }
    }
  }

  if (!label.empty()) {
    DefineLuLabel(label, start_address, label_atom_position, section, symbols, local_labels_,
                  last_global_label_, ScmasmSyntaxParser::IsLocalLabel,
                  [this](const std::string& lbl) { return ScopedLocalLabelName(lbl); });
  }

  line_idx = endu_line;
}

// ============================================================================
// Dummy Section Management
// ============================================================================

bool ScmasmSyntaxParser::InDummySection() const {
  return in_dummy_section_;
}

void ScmasmSyntaxParser::StartDummySection(uint32_t current_address) {
  dummy_saved_address_ = current_address;
  in_dummy_section_ = true;
}

uint32_t ScmasmSyntaxParser::EndDummySection() {
  in_dummy_section_ = false;
  return dummy_saved_address_;
}

bool ScmasmSyntaxParser::InPhase() const {
  return in_phase_;
}

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

uint32_t ScmasmSyntaxParser::GetCurrentRealAddress(uint32_t current_virtual) const {
  if (!in_phase_) {
    return current_virtual;  // Not in phase, virtual == real
  }

  // Calculate bytes emitted during phase
  uint32_t phase_size = current_virtual - phase_virtual_addr_;

  // Real address = saved real address + bytes emitted
  return phase_real_addr_ + phase_size;
}

// ============================================================================
// Directive Registry
// ============================================================================

}  // namespace xasm
