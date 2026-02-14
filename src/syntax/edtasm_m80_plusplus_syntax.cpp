/**
 * @file z80_universal_syntax.cpp
 * @brief EDTASM-M80++ Syntax Parser Implementation
 */

#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "edtasm_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/cpu/opcodes_z80.h"
#include "xasm++/directives/common_directives.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/z80_directives.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace xasm {

namespace {

// Numeric literal prefixes
constexpr char HEX_PREFIX_DOLLAR = '$';    // $FF
constexpr char HEX_PREFIX_0X = 'x';        // 0xFF

// Radix values
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_OCTAL = 8;
constexpr int RADIX_DECIMAL = 10;
constexpr int RADIX_HEXADECIMAL = 16;

// String delimiters (moved to edtasm_directive_handlers.cpp)
// constexpr char SINGLE_QUOTE = '\'';
// constexpr char DOUBLE_QUOTE = '"';

// Z80 instruction size constants (for size estimation)
constexpr int INSTRUCTION_SIZE_SINGLE_BYTE = 1;   // RST, register-only operations
constexpr int INSTRUCTION_SIZE_TWO_BYTES = 2;     // JR, DJNZ, immediate 8-bit operands
constexpr int INSTRUCTION_SIZE_THREE_BYTES = 3;   // JP, CALL, 16-bit immediate operands

} // anonymous namespace

// Namespace aliases for directive constants
using namespace CommonDirectives;
using namespace Z80Directives;

// Import specific Z80 mnemonics (avoid conflicts with CommonDirectives::SET)
using Z80Mnemonics::JR;
using Z80Mnemonics::DJNZ;
using Z80Mnemonics::JP;
using Z80Mnemonics::CALL;
using Z80Mnemonics::RST;

// Import EDTASM-M80++ specific directive constants
using xasm::directives::DOT_LIST;
using xasm::directives::DOT_RADIX;
using xasm::directives::DOT_SUBTTL;
using xasm::directives::DOT_TITLE;
using xasm::directives::DOT_XLIST;
using xasm::directives::LALL;
using xasm::directives::SALL;
using xasm::directives::STAR_LIST;
using xasm::directives::STAR_RADIX;

// ============================================================================
// Z80NumberParser Implementation
// ============================================================================

bool Z80NumberParser::TryParse(const std::string &token, int64_t &value) const {
  if (token.empty()) {
    return false;
  }

  char suffix = token.back();

  // Hexadecimal with H suffix: 0FFH, 9FH, etc.
  // Must start with digit to distinguish from labels
  if (suffix == 'H' || suffix == 'h') {
    if (token.length() < 2 || !std::isdigit(token[0])) {
      return false;
    }

    std::string hex_part = token.substr(0, token.length() - 1);

    // Validate all characters are hex digits
    for (char c : hex_part) {
      if (!std::isxdigit(c)) {
        return false;
      }
    }

    // Convert to value
    value = 0;
    for (char c : hex_part) {
      value *= RADIX_HEXADECIMAL;
      if (c >= '0' && c <= '9') {
        value += c - '0';
      } else if (c >= 'A' && c <= 'F') {
        value += c - 'A' + 10;
      } else if (c >= 'a' && c <= 'f') {
        value += c - 'a' + 10;
      }
    }
    return true;
  }

  // Octal with O or Q suffix: 377O, 77Q, etc.
  // Must start with digit to distinguish from labels
  if (suffix == 'O' || suffix == 'o' || suffix == 'Q' || suffix == 'q') {
    if (token.length() < 2 || !std::isdigit(token[0])) {
      return false;
    }

    std::string octal_part = token.substr(0, token.length() - 1);

    // Validate all characters are octal digits (0-7)
    for (char c : octal_part) {
      if (c < '0' || c > '7') {
        return false;
      }
    }

    // Convert to value
    value = 0;
    for (char c : octal_part) {
      value = value * RADIX_OCTAL + (c - '0');
    }
    return true;
  }

  // Binary with B suffix: 11111111B, 10101010B, etc.
  // Must start with digit to distinguish from labels
  if (suffix == 'B' || suffix == 'b') {
    if (token.length() < 2 || !std::isdigit(token[0])) {
      return false;
    }

    std::string binary_part = token.substr(0, token.length() - 1);

    // Validate all characters are binary digits (0-1)
    for (char c : binary_part) {
      if (c != '0' && c != '1') {
        return false;
      }
    }

    // Convert to value
    value = 0;
    for (char c : binary_part) {
      value = value * RADIX_BINARY + (c - '0');
    }
    return true;
  }

  // Decimal with D suffix: 255D, 42D, etc.
  // Must start with digit to distinguish from labels
  if (suffix == 'D' || suffix == 'd') {
    if (token.length() < 2 || !std::isdigit(token[0])) {
      return false;
    }

    std::string decimal_part = token.substr(0, token.length() - 1);

    // Validate all characters are decimal digits
    for (char c : decimal_part) {
      if (!std::isdigit(c)) {
        return false;
      }
    }

    // Convert to value
    value = 0;
    for (char c : decimal_part) {
      value = value * 10 + (c - '0');
    }
    return true;
  }

  // No explicit suffix - use current radix
  // Check if ALL characters are valid digits for the current radix
  for (char c : token) {
    int digit_value;
    if (c >= '0' && c <= '9') {
      digit_value = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      digit_value = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      digit_value = c - 'a' + 10;
    } else {
      // Invalid character - not a number
      return false;
    }

    if (digit_value >= radix_) {
      // Character value exceeds radix - not valid in this radix
      return false;
    }
  }

  // All characters are valid for the radix - convert to value
  value = 0;
  for (char c : token) {
    value *= radix_;
    if (c >= '0' && c <= '9') {
      value += c - '0';
    } else if (c >= 'A' && c <= 'F') {
      value += c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      value += c - 'a' + 10;
    }
  }
  return true;
}

// ============================================================================
// EdtasmM80PlusPlusSyntaxParser Implementation
// ============================================================================

EdtasmM80PlusPlusSyntaxParser::EdtasmM80PlusPlusSyntaxParser()
    : in_macro_definition_(false), macro_expansion_depth_(0),
      macro_unique_counter_(0), next_macro_unique_id_(0),
      exitm_triggered_(false), macro_nesting_depth_(0),
      in_repeat_block_(RepeatType::NONE), rept_count_(0),
      repeat_nesting_depth_(0), current_address_(0), end_directive_seen_(false),
      current_line_(0), listing_enabled_(true), current_radix_(RADIX_DECIMAL) {
  InitializeDirectiveRegistry();
}

void EdtasmM80PlusPlusSyntaxParser::InitializeDirectiveRegistry() {
  // Register all EDTASM directive handlers (implemented as free functions)
  RegisterEdtasmDirectiveHandlers(directive_registry_);
}

void EdtasmM80PlusPlusSyntaxParser::SetCpu(CpuZ80 *cpu) { cpu_ = cpu; }

void EdtasmM80PlusPlusSyntaxParser::SetRadix(int radix) {
  if (radix < 2 || radix > 16) {
    throw std::runtime_error("RADIX must be between 2 and 16");
  }
  current_radix_ = radix;
  z80_number_parser_.SetRadix(radix);
}

void EdtasmM80PlusPlusSyntaxParser::PushConditional(bool condition) {
  ConditionalBlock block;
  block.condition = condition;
  block.in_else_block = false;
  block.should_emit = condition;
  conditional_stack_.push_back(block);
}

void EdtasmM80PlusPlusSyntaxParser::ToggleConditional() {
  if (conditional_stack_.empty()) {
    throw std::runtime_error("ELSE without matching IF");
  }
  auto &block = conditional_stack_.back();
  if (block.in_else_block) {
    throw std::runtime_error("Multiple ELSE for same IF");
  }
  block.in_else_block = true;
  block.should_emit = !block.condition;
}

void EdtasmM80PlusPlusSyntaxParser::PopConditional() {
  if (conditional_stack_.empty()) {
    throw std::runtime_error("ENDIF without matching IF");
  }
  conditional_stack_.pop_back();
}

bool EdtasmM80PlusPlusSyntaxParser::ShouldSuppressEmission() const {
  for (const auto &block : conditional_stack_) {
    if (!block.should_emit) {
      return true;
    }
  }
  return false;
}

void EdtasmM80PlusPlusSyntaxParser::Parse(const std::string &source,
                                          Section &section,
                                          ConcreteSymbolTable &symbols) {
  // Reset state
  end_directive_seen_ = false;
  current_line_ = 0;
  current_address_ = 0;
  conditional_stack_.clear();
  current_scope_.global_label.clear();
  current_scope_.local_labels.clear();
  listing_enabled_ = true;
  listing_title_.clear();
  listing_subtitle_.clear();
  module_name_.clear();
  current_radix_ = 10;
  z80_number_parser_.SetRadix(RADIX_DECIMAL);
  in_macro_definition_ = false;
  in_repeat_block_ = RepeatType::NONE;
  macro_expansion_depth_ = 0;
  macro_nesting_depth_ = 0;

  // Define special predefined symbols (DATE, TIME)
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm *local_time = std::localtime(&now_time);

  // DATE: YYYYMMDD format
  int date_value = (local_time->tm_year + 1900) * 10000 +
                   (local_time->tm_mon + 1) * 100 + local_time->tm_mday;
  symbols.DefineLabel("DATE", date_value);

  // TIME: HHMMSS format
  int time_value = local_time->tm_hour * 10000 + local_time->tm_min * 100 +
                   local_time->tm_sec;
  symbols.DefineLabel("TIME", time_value);

  // Split source into lines
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    current_line_++;

    // Stop processing if END directive was seen
    if (end_directive_seen_) {
      break;
    }

    // Strip comments
    line = StripComments(line);

    // Skip empty lines
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    // Parse the line
    ParseLine(line, section, symbols);
  }

  // Check for unclosed blocks (only if END directive wasn't seen)
  // END directive stops assembly, so unclosed blocks after it are acceptable
  if (!end_directive_seen_) {
    if (in_macro_definition_) {
      throw std::runtime_error("Unclosed MACRO definition: " +
                               current_macro_.name);
    }
    if (in_repeat_block_ != RepeatType::NONE) {
      throw std::runtime_error("Unclosed REPT/IRP/IRPC block");
    }
  }
}

std::string
EdtasmM80PlusPlusSyntaxParser::StripComments(const std::string &line) {
  // Find semicolon comment
  size_t semi_pos = line.find(';');
  if (semi_pos != std::string::npos) {
    return line.substr(0, semi_pos);
  }
  return line;
}

std::string EdtasmM80PlusPlusSyntaxParser::Trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, last - first + 1);
}

void EdtasmM80PlusPlusSyntaxParser::ParseLine(const std::string &line,
                                              Section &section,
                                              ConcreteSymbolTable &symbols) {
  // Store original line for listing output (before comment stripping)
  std::string original_line = line;

  // Check if we're capturing a macro/repeat block
  // First check if this line is ENDM (to end capture) or END (to stop assembly)
  std::string trimmed_line = Trim(line);
  std::string upper_line = trimmed_line;
  std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(),
                 ::toupper);

  bool is_endm = (upper_line == ENDM ||
                  upper_line.rfind(std::string(ENDM) + " ", 0) == 0 ||
                  upper_line.rfind(std::string(ENDM) + "\t", 0) == 0);

  bool is_end =
      (upper_line == END || upper_line.rfind(std::string(END) + " ", 0) == 0 ||
       upper_line.rfind(std::string(END) + "\t", 0) == 0);

  // Check if this is a LOCAL directive (should be processed immediately in
  // macro definition)
  std::string trimmed_upper = upper_line;
  size_t first_non_space = trimmed_upper.find_first_not_of(" \t");
  if (first_non_space != std::string::npos) {
    trimmed_upper = trimmed_upper.substr(first_non_space);
  }
  bool is_local = (trimmed_upper.rfind(std::string(LOCAL) + " ", 0) == 0 ||
                   trimmed_upper.rfind(std::string(LOCAL) + "\t", 0) == 0);

  if (trimmed_line.find(LOCAL) != std::string::npos) {
  }

  // If this is a LOCAL directive in a macro definition, process it now
  if (in_macro_definition_ && is_local) {
    // Extract local symbol names from the operand
    std::string operand =
        trimmed_upper.substr(std::strlen(LOCAL)); // Skip LOCAL
    operand = Trim(operand);

    // Parse comma-separated list of symbols
    std::vector<std::string> local_symbols;
    std::istringstream iss(operand);
    std::string symbol;
    while (std::getline(iss, symbol, ',')) {
      symbol = Trim(symbol);
      if (!symbol.empty()) {
        current_macro_.locals.push_back(symbol);
      }
    }

    // Don't add LOCAL directive to macro body
    return;
  }

  // END directive should always stop assembly, even in macro/repeat blocks
  if (is_end && !in_macro_definition_ && in_repeat_block_ != RepeatType::NONE) {
    // END encountered while in repeat block - process it to set
    // end_directive_seen_ This allows unclosed repeat blocks before END (Don't
    // return early - fall through to normal processing)
  }

  // If in macro definition or repeat block, capture lines (except END)
  if ((in_macro_definition_ || in_repeat_block_ != RepeatType::NONE) &&
      !is_local && !is_end) {
    if (in_macro_definition_) {
      // Check for nested MACRO/REPT/IRP/IRPC to track nesting
      if (upper_line.rfind(std::string(MACRO) + " ", 0) == 0 ||
          upper_line.rfind(std::string(MACRO) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(REPT) + " ", 0) == 0 ||
          upper_line.rfind(std::string(REPT) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(IRP) + " ", 0) == 0 ||
          upper_line.rfind(std::string(IRP) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(IRPC) + " ", 0) == 0 ||
          upper_line.rfind(std::string(IRPC) + "\t", 0) == 0) {
        macro_nesting_depth_++;
        current_macro_.body.push_back(trimmed_line);
        return;
      }

      // Check for ENDM
      if (is_endm) {
        if (macro_nesting_depth_ > 0) {
          // This ENDM closes a nested block - capture it
          macro_nesting_depth_--;
          current_macro_.body.push_back(trimmed_line);
          return;
        }
        // else: nesting_depth == 0, so this ENDM ends the macro definition
        // Fall through to process normally
      } else {
        // Regular line in macro body
        current_macro_.body.push_back(trimmed_line);
        return;
      }
    } else {
      // In repeat block - track nesting depth for nested REPT/IRP/IRPC/MACRO
      // blocks
      if (upper_line.rfind(std::string(MACRO) + " ", 0) == 0 ||
          upper_line.rfind(std::string(MACRO) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(REPT) + " ", 0) == 0 ||
          upper_line.rfind(std::string(REPT) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(IRP) + " ", 0) == 0 ||
          upper_line.rfind(std::string(IRP) + "\t", 0) == 0 ||
          upper_line.rfind(std::string(IRPC) + " ", 0) == 0 ||
          upper_line.rfind(std::string(IRPC) + "\t", 0) == 0) {
        repeat_nesting_depth_++;
        repeat_body_.push_back(trimmed_line);
        return;
      }

      // Check for ENDM
      if (is_endm) {
        if (repeat_nesting_depth_ > 0) {
          // This ENDM closes a nested block - capture it
          repeat_nesting_depth_--;
          repeat_body_.push_back(trimmed_line);
          return;
        }
        // else: nesting_depth == 0, so this ENDM ends the repeat block
        // Fall through to process normally
      } else {
        // Regular line in repeat block
        repeat_body_.push_back(trimmed_line);
        return;
      }
    }
  }

  size_t pos = 0;

  // Check for label (ends with : or ::)
  std::string label = ParseLabel(line, pos, section, symbols);

  // Skip whitespace after label
  while (pos < line.size() && std::isspace(line[pos])) {
    pos++;
  }

  // Get mnemonic/directive
  size_t start = pos;
  while (pos < line.size() && !std::isspace(line[pos])) {
    pos++;
  }

  if (start >= line.size()) {
    // Line with only a label
    return;
  }

  std::string mnemonic = line.substr(start, pos - start);

  // Convert to uppercase for case-insensitive matching
  std::string upper_mnemonic = mnemonic;
  std::transform(upper_mnemonic.begin(), upper_mnemonic.end(),
                 upper_mnemonic.begin(), ::toupper);

  // Skip whitespace after mnemonic
  while (pos < line.size() && std::isspace(line[pos])) {
    pos++;
  }

  // Get next token to check if mnemonic is actually a label for EQU/=/SET
  std::string second_token;
  size_t second_start = pos;
  while (pos < line.size() && !std::isspace(line[pos])) {
    pos++;
  }
  if (second_start < line.size()) {
    second_token = line.substr(second_start, pos - second_start);
  }

  // Convert second token to uppercase
  std::string upper_second = second_token;
  std::transform(upper_second.begin(), upper_second.end(), upper_second.begin(),
                 ::toupper);

  // Check if this is label-without-colon syntax (LABEL EQU/=/SET/MACRO value)
  if (label.empty() &&
      (upper_second == EQU || upper_second == EQUALS || upper_second == SET ||
       upper_second == DEFL || upper_second == MACRO)) {
    // First token is the label, second is the directive
    label = mnemonic;
    upper_mnemonic = upper_second;

    // Skip whitespace after directive
    while (pos < line.size() && std::isspace(line[pos])) {
      pos++;
    }
  } else {
    // Reset pos to after first token for normal processing
    pos = second_start;
  }

  // Get operand (rest of line)
  std::string operand;
  if (pos < line.size()) {
    operand = Trim(line.substr(pos));
  }

  // Conditional assembly directives must always be processed
  // (to maintain stack integrity), but other directives honor the stack
  bool is_conditional_directive =
      (upper_mnemonic == IF || upper_mnemonic == IFDEF ||
       upper_mnemonic == IFNDEF || upper_mnemonic == IFEQ ||
       upper_mnemonic == IFNE || upper_mnemonic == IFLT ||
       upper_mnemonic == IFGT || upper_mnemonic == IFLE ||
       upper_mnemonic == IFGE || upper_mnemonic == IF1 ||
       upper_mnemonic == IF2 || upper_mnemonic == IFB ||
       upper_mnemonic == IFNB || upper_mnemonic == IFIDN ||
       upper_mnemonic == IFDIF || upper_mnemonic == ELSE ||
       upper_mnemonic == ENDIF);

  // Check if we should emit code based on conditional stack
  bool should_emit = true;
  for (const auto &block : conditional_stack_) {
    if (!block.should_emit) {
      should_emit = false;
      break;
    }
  }

  // Process conditional directives even when not emitting
  // (to maintain stack balance)
  if (is_conditional_directive) {
    if (directive_registry_.IsRegistered(upper_mnemonic)) {
      DirectiveContext ctx;
      ctx.section = &section;
      ctx.symbols = &symbols;
      ctx.current_address = &current_address_;
      ctx.parser_state = this; // Pass parser for conditional stack access
      ctx.current_file = current_file_;
      ctx.current_line = current_line_;
      ctx.source_line = original_line;
      directive_registry_.Execute(upper_mnemonic, label, operand, ctx);
    }
    return;
  }

  // Skip non-conditional directives when not emitting
  if (!should_emit) {
    return;
  }

  // Dispatch via directive registry
  if (directive_registry_.IsRegistered(upper_mnemonic)) {
    DirectiveContext ctx;
    ctx.section = &section;
    ctx.symbols = &symbols;
    ctx.current_address = &current_address_;
    ctx.parser_state = this; // Pass parser state
    ctx.current_file = current_file_;
    ctx.current_line = current_line_;
    ctx.source_line = original_line;
    directive_registry_.Execute(upper_mnemonic, label, operand, ctx);
    return;
  }

  // Check if it's a macro invocation
  if (macros_.find(upper_mnemonic) != macros_.end()) {
    const MacroDefinition &macro = macros_[upper_mnemonic];

    // Parse arguments
    std::vector<std::string> args;
    std::string trimmed_operand = Trim(operand);
    if (!trimmed_operand.empty()) {
      std::string remaining = trimmed_operand;
      size_t comma_pos;
      while ((comma_pos = remaining.find(',')) != std::string::npos) {
        std::string arg = Trim(remaining.substr(0, comma_pos));
        args.push_back(arg);
        remaining = remaining.substr(comma_pos + 1);
      }
      // Last argument
      args.push_back(Trim(remaining));
    }

    // Check parameter count
    if (args.size() != macro.params.size()) {
      throw std::runtime_error("Macro " + macro.name + " expects " +
                               std::to_string(macro.params.size()) +
                               " parameters, got " +
                               std::to_string(args.size()));
    }

    // Expand macro body with parameter substitution
    std::vector<std::string> expanded_lines;
    int unique_id = next_macro_unique_id_++;

    // Build set of unique LOCAL label names to suppress atom creation
    macro_local_labels_.clear();
    for (const auto &local_label : macro.locals) {
      std::string unique_label = local_label + "_" + std::to_string(unique_id);
      macro_local_labels_.insert(unique_label);
    }

    for (const auto &body_line : macro.body) {
      std::string expanded =
          SubstituteMacroParameters(body_line, macro.params, args);
      // Handle LOCAL labels
      expanded = MakeLocalLabelUnique(expanded, macro.locals, unique_id);
      expanded_lines.push_back(expanded);
    }

    // Parse expanded lines
    ExpandAndParseLines(expanded_lines, section, symbols);

    // Clear LOCAL labels after macro expansion
    macro_local_labels_.clear();
    return;
  }

  // Assume it's a CPU instruction - create InstructionAtom
  // CPU plugin will later encode it (Phase 9+)
  auto inst_atom = std::make_shared<InstructionAtom>(upper_mnemonic, operand);
  inst_atom->location = SourceLocation(current_file_, current_line_, 1);
  inst_atom->source_line = original_line;
  section.atoms.push_back(inst_atom);

  // Estimate instruction size based on mnemonic and operand
  // This is a heuristic until CPU plugin provides exact encoding (Phase 9+)
  uint32_t estimated_size = EstimateZ80InstructionSize(upper_mnemonic, operand);
  current_address_ += estimated_size;
}

uint32_t EdtasmM80PlusPlusSyntaxParser::EstimateZ80InstructionSize(
    const std::string &mnemonic, const std::string &operand) {
  // Heuristic Z80 instruction size estimation
  // Actual encoding will be done by CPU plugin (Phase 9+)

  // Extended instructions (ED prefix) - typically 2+ bytes
  if (mnemonic.find("LD") == 0 && (operand.find("I,") != std::string::npos ||
                                   operand.find("R,") != std::string::npos ||
                                   operand.find(",I") != std::string::npos ||
                                   operand.find(",R") != std::string::npos)) {
    return INSTRUCTION_SIZE_TWO_BYTES; // ED-prefixed LD I/R instructions
  }

  // Index register instructions (DD/FD prefix)
  if (operand.find("IX") != std::string::npos ||
      operand.find("IY") != std::string::npos) {
    // With displacement: prefix + opcode + displacement
    if (operand.find("(") != std::string::npos) {
      return operand.find(",") != std::string::npos
                 ? 4
                 : INSTRUCTION_SIZE_THREE_BYTES; // With or without immediate
    }
    return INSTRUCTION_SIZE_TWO_BYTES; // Without displacement
  }

  // Immediate 16-bit operands (e.g., LD HL,nnnn)
  if (operand.find(",") != std::string::npos &&
      (operand.find("$") != std::string::npos ||
       operand.find("0") != std::string::npos)) {
    std::string trimmed_op = Trim(operand);
    // Check if likely 16-bit value (hex with 3+ digits, or decimal > 255)
    if (trimmed_op.find("$") != std::string::npos) {
      size_t hex_start = trimmed_op.find("$") + 1;
      size_t hex_end = hex_start;
      while (hex_end < trimmed_op.size() &&
             std::isxdigit(trimmed_op[hex_end])) {
        hex_end++;
      }
      if (hex_end - hex_start > 2) {
        return INSTRUCTION_SIZE_THREE_BYTES; // opcode + 16-bit immediate
      }
    }
  }

  // Immediate 8-bit operands (e.g., LD A,n)
  if (operand.find(",") != std::string::npos) {
    return INSTRUCTION_SIZE_TWO_BYTES; // opcode + byte operand (default for immediate data)
  }

  // Relative jumps (JR, DJNZ) - opcode + displacement
  if (mnemonic == JR || mnemonic == DJNZ) {
    return INSTRUCTION_SIZE_TWO_BYTES;
  }

  // Absolute jumps/calls - opcode + 16-bit address
  if (mnemonic == JP || mnemonic == CALL) {
    return INSTRUCTION_SIZE_THREE_BYTES;
  }

  // RST instructions - single byte
  if (mnemonic == RST) {
    return INSTRUCTION_SIZE_SINGLE_BYTE;
  }

  // Default: assume single-byte instruction (register-only, implicit, etc.)
  return INSTRUCTION_SIZE_SINGLE_BYTE;
}

std::string
EdtasmM80PlusPlusSyntaxParser::ParseLabel(const std::string &line, size_t &pos,
                                          Section &section,
                                          ConcreteSymbolTable &symbols) {
  // Check if line starts with a label (identifier followed by : or ::)
  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos || colon_pos == 0) {
    return ""; // No label
  }

  // Extract potential label
  std::string potential_label = Trim(line.substr(0, colon_pos));

  // Check if it's all identifier characters (no spaces)
  bool is_label = true;
  for (char c : potential_label) {
    if (!std::isalnum(c) && c != '_' && c != '$' && c != '.' && c != '?') {
      is_label = false;
      break;
    }
  }

  if (!is_label) {
    return "";
  }

  // Determine if public label (::) or private (:)
  bool is_public = false;
  if (colon_pos + 1 < line.size() && line[colon_pos + 1] == ':') {
    pos = colon_pos + 2; // Skip ::
    is_public = true;
  } else {
    pos = colon_pos + 1; // Skip :
  }

  // Define symbol first
  symbols.DefineLabel(potential_label, current_address_);

  // Mark as public (exported) if double colon was used
  if (is_public) {
    Symbol *symbol = symbols.GetSymbol(potential_label);
    if (symbol != nullptr) {
      symbol->is_exported = true;
    }
  }

  // Create label atom ONLY if not a macro LOCAL label
  // Macro LOCAL labels should not create atoms (only used for references)
  if (macro_local_labels_.find(potential_label) == macro_local_labels_.end()) {
    auto label_atom =
        std::make_shared<LabelAtom>(potential_label, current_address_);
    label_atom->location = SourceLocation(current_file_, current_line_, 1);
    label_atom->source_line = line; // Store original line
    section.atoms.push_back(label_atom);
  }

  // Update scope if it's a global label (not starting with . $ ?)
  if (potential_label[0] != '.' && potential_label[0] != '$' &&
      potential_label[0] != '?') {
    current_scope_.global_label = potential_label;
    current_scope_.local_labels.clear();
  } else {
    // Local label - add to current scope
    current_scope_.local_labels[potential_label] = current_address_;
  }

  return potential_label;
}

uint32_t EdtasmM80PlusPlusSyntaxParser::ParseNumber(const std::string &str) {
  std::string trimmed = Trim(str);
  if (trimmed.empty()) {
    return 0;
  }

  // Explicit format overrides radix
  // Hex: $FF, 0xFF, 0FFH
  if (trimmed[0] == HEX_PREFIX_DOLLAR) {
    return std::stoul(trimmed.substr(1), nullptr, RADIX_HEXADECIMAL);
  } else if (trimmed.size() >= 2 && trimmed[0] == '0' &&
             (trimmed[1] == HEX_PREFIX_0X || trimmed[1] == 'X')) {
    return std::stoul(trimmed.substr(2), nullptr, RADIX_HEXADECIMAL);
  } else if (trimmed.size() >= 2 &&
             (trimmed.back() == 'H' || trimmed.back() == 'h')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, RADIX_HEXADECIMAL);
  }
  // Binary: 11110000B
  else if (trimmed.size() >= 2 &&
           (trimmed.back() == 'B' || trimmed.back() == 'b')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, RADIX_BINARY);
  }
  // Octal: 377O, 377Q
  else if (trimmed.size() >= 2 &&
           (trimmed.back() == 'O' || trimmed.back() == 'o' ||
            trimmed.back() == 'Q' || trimmed.back() == 'q')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, RADIX_OCTAL);
  }
  // Decimal with D suffix: 255D
  else if (trimmed.size() >= 2 &&
           (trimmed.back() == 'D' || trimmed.back() == 'd')) {
    return std::stoul(trimmed.substr(0, trimmed.size() - 1), nullptr, RADIX_DECIMAL);
  }
  // No explicit format - use current radix
  else {
    return std::stoul(trimmed, nullptr, current_radix_);
  }
}

std::shared_ptr<Expression>
EdtasmM80PlusPlusSyntaxParser::ParseExpression(const std::string &str,
                                               ConcreteSymbolTable &symbols) {
  // Create ExpressionParser with symbol table and Z80 number parser
  ExpressionParser parser(&symbols, &z80_number_parser_);
  return parser.Parse(str);
}

std::string
EdtasmM80PlusPlusSyntaxParser::FormatError(const std::string &message) const {
  std::ostringstream oss;
  if (!current_file_.empty()) {
    oss << current_file_ << ":";
  }
  oss << current_line_ << ": " << message;
  return oss.str();
}

// ============================================================================
// Macro Helper Methods
// ============================================================================

/**
 * @brief Expand and parse a block of lines
 *
 * Used by REPT, IRP, IRPC to recursively parse captured lines.
 */
void EdtasmM80PlusPlusSyntaxParser::ExpandAndParseLines(
    const std::vector<std::string> &lines, Section &section,
    ConcreteSymbolTable &symbols) {
  // Note: Don't reset exitm_triggered_ here - let caller handle it
  // so EXITM can properly exit from REPT/IRP/IRPC loops
  for (const auto &line : lines) {
    if (exitm_triggered_) {
      break; // Exit early if EXITM was encountered
    }
    ParseLine(line, section, symbols);
  }
}

/**
 * @brief Substitute macro parameters in a line
 *
 * Replaces parameter references (&param, %param) with values.
 * & = textual substitution
 * % = numeric expression evaluation
 */
std::string EdtasmM80PlusPlusSyntaxParser::SubstituteMacroParameters(
    const std::string &line, const std::vector<std::string> &param_names,
    const std::vector<std::string> &param_values) {
  std::string result = line;

  // Substitute each parameter
  for (size_t i = 0; i < param_names.size(); ++i) {
    std::string param_name = param_names[i];
    std::string param_value = (i < param_values.size()) ? param_values[i] : "";

    // & prefix - textual substitution
    std::string amp_param = "&" + param_name;
    size_t pos = 0;
    while ((pos = result.find(amp_param, pos)) != std::string::npos) {
      result.replace(pos, amp_param.size(), param_value);
      pos += param_value.size();
    }

    // % prefix - numeric evaluation
    // First substitute with value, then evaluate the resulting expression
    std::string pct_param = "%" + param_name;
    pos = 0;
    while ((pos = result.find(pct_param, pos)) != std::string::npos) {
      // Extract the expression context around the parameter
      // We need to find the expression boundaries to evaluate
      size_t expr_end = pos + pct_param.size();

      // Find expression end (whitespace, comma, newline, or end of string)
      while (expr_end < result.size() && !std::isspace(result[expr_end]) &&
             result[expr_end] != ',' && result[expr_end] != ';') {
        expr_end++;
      }

      // Extract expression with parameter substituted
      std::string expr = result.substr(pos, expr_end - pos);
      // Replace %param with value in the expression
      size_t param_pos = expr.find(pct_param);
      if (param_pos != std::string::npos) {
        expr.replace(param_pos, pct_param.size(), param_value);
      }

      // Try to evaluate the expression
      try {
        ConcreteSymbolTable temp_symbols;
        auto parsed_expr = ParseExpression(expr, temp_symbols);
        if (parsed_expr) {
          int64_t value = parsed_expr->Evaluate(temp_symbols);
          std::string value_str = std::to_string(value);
          result.replace(pos, expr_end - pos, value_str);
          pos += value_str.size();
        } else {
          // Failed to parse - do textual substitution
          result.replace(pos, pct_param.size(), param_value);
          pos += param_value.size();
        }
      } catch (...) {
        // Evaluation failed - do textual substitution
        result.replace(pos, pct_param.size(), param_value);
        pos += param_value.size();
      }
    }
  }

  return result;
}

/**
 * @brief Make LOCAL labels unique within a macro expansion
 *
 * Appends unique ID to labels declared as LOCAL.
 */
std::string EdtasmM80PlusPlusSyntaxParser::MakeLocalLabelUnique(
    const std::string &line, const std::vector<std::string> &local_labels,
    int unique_id) {
  std::string result = line;

  for (const auto &label : local_labels) {
    // Find label references (with or without colon)
    std::string unique_label = label + "_" + std::to_string(unique_id);

    // Replace label: with unique_label:
    size_t pos = 0;
    while ((pos = result.find(label + ":", pos)) != std::string::npos) {
      result.replace(pos, label.size(), unique_label);
      pos += unique_label.size() + 1;
    }

    // Replace standalone label references
    // Check word boundaries considering Z80 identifier characters
    pos = 0;
    while ((pos = result.find(label, pos)) != std::string::npos) {
      // Check if it's a standalone reference (not part of another identifier)
      bool is_standalone = true;

      // Check character before label (if any)
      if (pos > 0) {
        char prev = result[pos - 1];
        if (std::isalnum(prev) || prev == '_' || prev == '$' || prev == '.' ||
            prev == '?') {
          is_standalone = false;
        }
      }

      // Check character after label (if any)
      if (pos + label.size() < result.size()) {
        char next = result[pos + label.size()];
        if (std::isalnum(next) || next == '_' || next == '$' || next == '.' ||
            next == '?' || next == ':') {
          is_standalone = false;
        }
      }

      if (is_standalone) {
        result.replace(pos, label.size(), unique_label);
        pos += unique_label.size();
      } else {
        pos += label.size();
      }
    }
  }

  return result;
}

std::vector<std::string>
EdtasmM80PlusPlusSyntaxParser::ParseSymbolList(const std::string &operand) {
  // Parse comma-separated list of symbols
  std::vector<std::string> symbols;
  std::istringstream iss(operand);
  std::string symbol_name;

  while (std::getline(iss, symbol_name, ',')) {
    symbol_name = Trim(symbol_name);
    if (!symbol_name.empty()) {
      symbols.push_back(symbol_name);
    }
  }

  return symbols;
}

} // namespace xasm
