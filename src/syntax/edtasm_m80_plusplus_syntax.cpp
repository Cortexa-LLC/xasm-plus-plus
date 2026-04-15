/**
 * @file z80_universal_syntax.cpp
 * @brief EDTASM-M80++ Syntax Parser Implementation
 */

#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "edtasm_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/cpu/opcodes_z80.h"
#include "xasm++/directives/common_directives.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/z80_directives.h"
#include "xasm++/parse_utils.h"  // For radix parsing utilities

namespace xasm {

namespace {

// Numeric literal prefixes
constexpr char HEX_PREFIX_DOLLAR = '$';  // $FF
constexpr char HEX_PREFIX_0X = 'x';      // 0xFF

// Radix values
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_OCTAL = 8;
constexpr int RADIX_DECIMAL = 10;
constexpr int RADIX_HEXADECIMAL = 16;

// String delimiters (moved to edtasm_directive_handlers.cpp)
// constexpr char SINGLE_QUOTE = '\'';
// constexpr char DOUBLE_QUOTE = '"';

// Z80 instruction size constants (for size estimation)
constexpr int INSTRUCTION_SIZE_SINGLE_BYTE = 1;  // RST, register-only operations
constexpr int INSTRUCTION_SIZE_TWO_BYTES = 2;    // JR, DJNZ, immediate 8-bit operands
constexpr int INSTRUCTION_SIZE_THREE_BYTES = 3;  // JP, CALL, 16-bit immediate operands

}  // anonymous namespace

// Namespace aliases for directive constants
using namespace CommonDirectives;
using namespace Z80Directives;

// Import specific Z80 mnemonics (avoid conflicts with CommonDirectives::SET)
using Z80Mnemonics::CALL;
using Z80Mnemonics::DJNZ;
using Z80Mnemonics::JP;
using Z80Mnemonics::JR;
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

namespace {

// Parse a suffix-delimited literal using the provided per-character digit
// decoder. Returns false if the token is too short, does not start with a
// digit, or contains an invalid digit for the given radix.
static bool ParseSuffixedLiteral(const std::string& token, int radix, bool (*digit_fn)(char, int&),
                                 int64_t& value) {
  if (token.length() < 2 || !std::isdigit(token[0])) {
    return false;
  }
  std::string digits = token.substr(0, token.length() - 1);
  value = 0;
  for (char c : digits) {
    int d = 0;
    if (!digit_fn(c, d)) {
      return false;
    }
    value = (value * radix) + d;
  }
  return true;
}

// Returns true if all characters in token are valid for the given radix (no
// suffix). Uses ParseHexDigit to convert, checking digit_value < radix.
static bool ParseUnsuffixedLiteral(const std::string& token, int radix, int64_t& value) {
  value = 0;
  for (char c : token) {
    int digit = 0;
    if (!ParseHexDigit(c, digit)) {
      return false;
    }
    if (digit >= radix) {
      return false;
    }
    value = (value * radix) + digit;
  }
  return true;
}

}  // namespace

bool Z80NumberParser::TryParse(const std::string& token, int64_t& value) const {
  if (token.empty()) {
    return false;
  }

  char suffix = token.back();

  // Hexadecimal with H suffix: 0FFH, 9FH, etc.
  // Must start with digit to distinguish from labels.
  if (suffix == 'H' || suffix == 'h') {
    // Extra validation: all non-suffix chars must be hex digits.
    std::string hex_part = token.substr(0, token.length() - 1);
    for (char c : hex_part) {
      if (!std::isxdigit(c)) {
        return false;
      }
    }
    return ParseSuffixedLiteral(token, RADIX_HEXADECIMAL, ParseHexDigit, value);
  }

  // Octal with O or Q suffix: 377O, 77Q, etc.
  if (suffix == 'O' || suffix == 'o' || suffix == 'Q' || suffix == 'q') {
    return ParseSuffixedLiteral(token, RADIX_OCTAL, ParseOctalDigit, value);
  }

  // Binary with B suffix: 11111111B, 10101010B, etc.
  if (suffix == 'B' || suffix == 'b') {
    return ParseSuffixedLiteral(token, RADIX_BINARY, ParseBinaryDigit, value);
  }

  // Decimal with D suffix: 255D, 42D, etc.
  if (suffix == 'D' || suffix == 'd') {
    return ParseSuffixedLiteral(token, RADIX_DECIMAL, ParseDecimalDigit, value);
  }

  // No explicit suffix — use the current radix.
  return ParseUnsuffixedLiteral(token, radix_, value);
}

// ============================================================================
// EdtasmM80PlusPlusSyntaxParser Implementation
// ============================================================================

EdtasmM80PlusPlusSyntaxParser::EdtasmM80PlusPlusSyntaxParser() {
  InitializeDirectiveRegistry();
}

void EdtasmM80PlusPlusSyntaxParser::InitializeDirectiveRegistry() {
  // Register all EDTASM directive handlers (implemented as free functions)
  RegisterEdtasmDirectiveHandlers(directive_registry_);
}

void EdtasmM80PlusPlusSyntaxParser::SetCpu(CpuZ80* cpu) {
  cpu_ = cpu;
}

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
  auto& block = conditional_stack_.back();
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
  return std::any_of(conditional_stack_.begin(), conditional_stack_.end(),
                     [](const auto& block) { return !block.should_emit; });
}

void EdtasmM80PlusPlusSyntaxParser::Parse(const std::string& source, Section& section,
                                          ConcreteSymbolTable& symbols) {
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
  std::tm* local_time = std::localtime(&now_time);

  // DATE: YYYYMMDD format
  int date_value = ((local_time->tm_year + 1900) * 10000) + ((local_time->tm_mon + 1) * 100) +
                   local_time->tm_mday;
  symbols.DefineLabel("DATE", date_value);

  // TIME: HHMMSS format
  int time_value = (local_time->tm_hour * 10000) + (local_time->tm_min * 100) + local_time->tm_sec;
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
      throw std::runtime_error("Unclosed MACRO definition: " + current_macro_.name);
    }
    if (in_repeat_block_ != RepeatType::NONE) {
      throw std::runtime_error("Unclosed REPT/IRP/IRPC block");
    }
  }
}

std::string EdtasmM80PlusPlusSyntaxParser::StripComments(const std::string& line) {
  // Find semicolon comment
  size_t semi_pos = line.find(';');
  if (semi_pos != std::string::npos) {
    return line.substr(0, semi_pos);
  }
  return line;
}

std::string EdtasmM80PlusPlusSyntaxParser::Trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, last - first + 1);
}

// ---------------------------------------------------------------------------
// MakeDirectiveContext — build a fully populated DirectiveContext from the
// current parser state.  Eliminates the copy-paste block that appeared twice
// in ParseLine.
// ---------------------------------------------------------------------------
DirectiveContext EdtasmM80PlusPlusSyntaxParser::MakeDirectiveContext(
    Section& section, ConcreteSymbolTable& symbols, const std::string& original_line,
    const std::string& mnemonic, const std::string& label, const std::string& operand) {
  DirectiveContext ctx;
  ctx.section = &section;
  ctx.symbols = &symbols;
  ctx.current_address = &current_address_;
  ctx.parser_state = this;
  ctx.current_file = current_file_;
  ctx.current_line = current_line_;
  ctx.source_line = original_line;
  ctx.mnemonic = mnemonic;
  ctx.label = label;
  ctx.operand = operand;
  return ctx;
}

// ---------------------------------------------------------------------------
// HandleCapturingMode — manage macro/repeat body capture.
//
// Called at the top of ParseLine when in_macro_definition_ or
// in_repeat_block_ is active.  Returns true if the line was consumed (i.e.
// ParseLine should return immediately after calling this), false when the
// line should continue with normal processing (e.g. ENDM that closes the
// outermost block).
// ---------------------------------------------------------------------------
bool EdtasmM80PlusPlusSyntaxParser::HandleCapturingMode(const std::string& trimmed_line,
                                                        bool is_endm, bool is_end) {
  // Guard: only relevant when actually capturing.
  if (!in_macro_definition_ && in_repeat_block_ == RepeatType::NONE) {
    return false;
  }
  // Guard: LOCAL and END are never captured.
  if (is_end) {
    return false;
  }

  // Helper: returns true when the uppercased trimmed_line starts with any of
  // the block-opening keywords that increment nesting depth.
  auto is_nesting_opener = [&](const std::string& upper_line) {
    static const std::string_view kOpeners[] = {MACRO, REPT, IRP, IRPC};
    for (auto kw : kOpeners) {
      std::string kw_space = std::string(kw) + " ";
      std::string kw_tab = std::string(kw) + "\t";
      if (upper_line.starts_with(kw_space) || upper_line.starts_with(kw_tab)) {
        return true;
      }
    }
    return false;
  };

  std::string upper_line = trimmed_line;
  std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);

  if (in_macro_definition_) {
    if (is_nesting_opener(upper_line)) {
      macro_nesting_depth_++;
      current_macro_.body.push_back(trimmed_line);
      return true;  // consumed
    }
    if (is_endm) {
      if (macro_nesting_depth_ > 0) {
        macro_nesting_depth_--;
        current_macro_.body.push_back(trimmed_line);
        return true;  // consumed — nested ENDM
      }
      return false;  // outermost ENDM: fall through to normal ENDM handler
    }
    // Regular body line.
    current_macro_.body.push_back(trimmed_line);
    return true;  // consumed
  }

  // In repeat block.
  if (is_nesting_opener(upper_line)) {
    repeat_nesting_depth_++;
    repeat_body_.push_back(trimmed_line);
    return true;  // consumed
  }
  if (is_endm) {
    if (repeat_nesting_depth_ > 0) {
      repeat_nesting_depth_--;
      repeat_body_.push_back(trimmed_line);
      return true;  // consumed — nested ENDM
    }
    return false;  // outermost ENDM: fall through to normal ENDM handler
  }
  // Regular repeat body line.
  repeat_body_.push_back(trimmed_line);
  return true;  // consumed
}

// ============================================================================
// ParseLine helpers
// ============================================================================

// Forward declaration (defined below, before ParseLine).
static bool IsLabelBindingDirective(const std::string& upper_second);

// Skip whitespace from pos, then scan a non-space token.
// Returns the token; advances pos past it.
static std::string ScanToken(const std::string& line, size_t& pos) {
  while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
    pos++;
  }
  size_t start = pos;
  while (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos]))) {
    pos++;
  }
  return (start < line.size()) ? line.substr(start, pos - start) : std::string{};
}

// Convert string to uppercase in-place.
static std::string ToUpper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

bool EdtasmM80PlusPlusSyntaxParser::IsConditionalDirective(const std::string& upper_mnemonic) {
  static const std::unordered_set<std::string> kConditionalDirectives = {
      IF,  IFDEF, IFNDEF, IFEQ, IFNE,  IFLT,  IFGT, IFLE,  IFGE,
      IF1, IF2,   IFB,    IFNB, IFIDN, IFDIF, ELSE, ENDIF,
  };
  return kConditionalDirectives.count(upper_mnemonic) > 0;
}

void EdtasmM80PlusPlusSyntaxParser::RegisterMacroLocals(const std::string& operand) {
  std::istringstream iss(operand);
  std::string symbol;
  while (std::getline(iss, symbol, ',')) {
    symbol = Trim(symbol);
    if (!symbol.empty()) {
      current_macro_.locals.push_back(symbol);
    }
  }
}

// Scan @p line for up to three logical components: label, mnemonic, operand.
// On return @p upper_mnemonic is the uppercase mnemonic (empty if label-only).
void EdtasmM80PlusPlusSyntaxParser::ParseTokens(const std::string& line,
                                                std::string& upper_mnemonic, std::string& label,
                                                std::string& operand, Section& section,
                                                ConcreteSymbolTable& symbols) {
  size_t pos = 0;
  label = ParseLabel(line, pos, section, symbols);

  const std::string mnemonic = ScanToken(line, pos);
  if (mnemonic.empty()) {
    upper_mnemonic.clear();
    return;
  }
  upper_mnemonic = ToUpper(mnemonic);

  size_t second_start = pos;
  const std::string upper_second = ToUpper(ScanToken(line, pos));

  if (label.empty() && IsLabelBindingDirective(upper_second)) {
    label = mnemonic;
    upper_mnemonic = upper_second;
  } else {
    pos = second_start;
  }

  operand = (pos < line.size()) ? Trim(line.substr(pos)) : std::string{};
}

void EdtasmM80PlusPlusSyntaxParser::ExpandMacroCall(const MacroDefinition& macro,
                                                    const std::string& operand, Section& section,
                                                    ConcreteSymbolTable& symbols) {
  // Parse comma-separated arguments
  std::vector<std::string> args;
  std::string trimmed_operand = Trim(operand);
  if (!trimmed_operand.empty()) {
    std::string remaining = trimmed_operand;
    size_t comma_pos = 0;
    while ((comma_pos = remaining.find(',')) != std::string::npos) {
      args.push_back(Trim(remaining.substr(0, comma_pos)));
      remaining = remaining.substr(comma_pos + 1);
    }
    args.push_back(Trim(remaining));
  }

  if (args.size() != macro.params.size()) {
    throw std::runtime_error("Macro " + macro.name + " expects " +
                             std::to_string(macro.params.size()) + " parameters, got " +
                             std::to_string(args.size()));
  }

  // Expand macro body with parameter substitution and LOCAL label uniquification
  int unique_id = next_macro_unique_id_++;
  macro_local_labels_.clear();
  for (const auto& local_label : macro.locals) {
    macro_local_labels_.insert(local_label + "_" + std::to_string(unique_id));
  }

  std::vector<std::string> expanded_lines;
  for (const auto& body_line : macro.body) {
    std::string expanded = SubstituteMacroParameters(body_line, macro.params, args);
    expanded = MakeLocalLabelUnique(expanded, macro.locals, unique_id);
    expanded_lines.push_back(expanded);
  }

  ExpandAndParseLines(expanded_lines, section, symbols);
  macro_local_labels_.clear();
}

// Returns true when the given (already-uppercased, trimmed) line equals
// @p keyword or starts with keyword followed by whitespace.
static bool LineIsKeyword(const std::string& upper_line, const std::string& keyword) {
  return upper_line == keyword || upper_line.starts_with(keyword + " ") ||
         upper_line.starts_with(keyword + "\t");
}

// Returns true when the first non-whitespace token in @p upper_line is the
// LOCAL directive keyword.
static bool LineStartsWithLocal(const std::string& upper_line) {
  size_t pos = upper_line.find_first_not_of(" \t");
  if (pos == std::string::npos) {
    return false;
  }
  const std::string rest = upper_line.substr(pos);
  return rest.starts_with(std::string(LOCAL) + " ") || rest.starts_with(std::string(LOCAL) + "\t");
}

// Returns the substring after the LOCAL keyword (the operand), trimmed.
static std::string ExtractLocalOperand(const std::string& upper_line) {
  size_t pos = upper_line.find_first_not_of(" \t");
  const std::string rest = (pos == std::string::npos) ? upper_line : upper_line.substr(pos);
  // Skip LOCAL keyword
  std::string after = rest.substr(std::strlen(LOCAL));
  // ltrim
  size_t start = after.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return "";
  }
  // rtrim
  size_t end = after.find_last_not_of(" \t");
  return after.substr(start, end - start + 1);
}

// Returns the uppercase label binding directive if present, otherwise empty.
// Also advances `pos` past the operand start.
static bool IsLabelBindingDirective(const std::string& upper_second) {
  static const std::unordered_set<std::string> kLabelBinders = {
      EQU, EQUALS, SET, DEFL, MACRO,
  };
  return kLabelBinders.count(upper_second) > 0;
}

void EdtasmM80PlusPlusSyntaxParser::ParseLine(const std::string& line, Section& section,
                                              ConcreteSymbolTable& symbols) {
  const std::string& original_line = line;

  const std::string trimmed_line = Trim(line);
  std::string upper_line = trimmed_line;
  std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);

  const bool is_endm = LineIsKeyword(upper_line, ENDM);
  const bool is_end = LineIsKeyword(upper_line, END);

  // LOCAL inside a macro definition — process immediately (not captured).
  if (in_macro_definition_ && LineStartsWithLocal(upper_line)) {
    RegisterMacroLocals(ExtractLocalOperand(upper_line));
    return;
  }

  if (HandleCapturingMode(trimmed_line, is_endm, is_end)) {
    return;
  }

  std::string upper_mnemonic;
  std::string label;
  std::string operand;
  ParseTokens(line, upper_mnemonic, label, operand, section, symbols);

  if (upper_mnemonic.empty()) {
    return;
  }  // label-only line

  // Conditional directives are always dispatched (maintain stack balance).
  if (IsConditionalDirective(upper_mnemonic)) {
    if (directive_registry_.IsRegistered(upper_mnemonic)) {
      auto ctx =
          MakeDirectiveContext(section, symbols, original_line, upper_mnemonic, label, operand);
      directive_registry_.Execute(ctx);
    }
    return;
  }

  if (ShouldSuppressEmission()) {
    return;
  }

  if (directive_registry_.IsRegistered(upper_mnemonic)) {
    auto ctx =
        MakeDirectiveContext(section, symbols, original_line, upper_mnemonic, label, operand);
    directive_registry_.Execute(ctx);
    return;
  }

  if (macros_.contains(upper_mnemonic)) {
    ExpandMacroCall(macros_[upper_mnemonic], operand, section, symbols);
    return;
  }

  // Emit CPU instruction atom (encoding deferred to CPU plugin, Phase 9+).
  auto inst_atom = std::make_shared<InstructionAtom>(upper_mnemonic, operand);
  inst_atom->location = SourceLocation(current_file_, current_line_, 1);
  inst_atom->source_line = original_line;
  section.atoms.push_back(inst_atom);

  DirectiveContext size_ctx;
  size_ctx.mnemonic = upper_mnemonic;
  size_ctx.operand = operand;
  current_address_ += EstimateZ80InstructionSize(size_ctx);
}

// ============================================================================
// Instruction Size Estimation Helpers
// ============================================================================

/**
 * @brief Estimate size of IX/IY-indexed instructions.
 *
 * All IX/IY instructions carry a DD/FD prefix.  When the operand also
 * contains an indirect-addressing bracket '(' the prefix is followed by
 * the opcode AND a displacement byte, optionally with a 16-bit immediate.
 */
uint32_t EdtasmM80PlusPlusSyntaxParser::EstimateIndexedInsnSize(const std::string& operand) {
  if (operand.find('(') == std::string::npos) {
    return INSTRUCTION_SIZE_TWO_BYTES;  // prefix + opcode, no displacement
  }
  // (IX+d) — with displacement; if also a comma operand, add immediate byte
  return (operand.find(',') != std::string::npos) ? 4 : INSTRUCTION_SIZE_THREE_BYTES;
}

/**
 * @brief Estimate size of 16-bit immediate operand instructions.
 *
 * Returns 3 if the operand contains a hex literal with more than 2 digits
 * (i.e. a value that cannot fit in 8 bits), otherwise returns 0 (unknown).
 */
static uint32_t EstimateImmediate16BitSize(const std::string& operand) {
  size_t dollar = operand.find('$');
  if (dollar == std::string::npos) {
    return 0;
  }
  size_t hex_start = dollar + 1;
  size_t hex_end = hex_start;
  while (hex_end < operand.size() && std::isxdigit(operand[hex_end])) {
    ++hex_end;
  }
  return (hex_end - hex_start > 2) ? INSTRUCTION_SIZE_THREE_BYTES : 0;
}

// Returns true when the operand contains an I- or R-register reference
// typical of ED-prefixed LD instructions (LD I,A / LD A,I etc.).
static bool HasIRRegisterOperand(const std::string& operand) {
  return operand.find("I,") != std::string::npos || operand.find("R,") != std::string::npos ||
         operand.find(",I") != std::string::npos || operand.find(",R") != std::string::npos;
}

// Returns true when the operand references an index register (IX or IY).
static bool HasIndexRegisterOperand(const std::string& operand) {
  return operand.find("IX") != std::string::npos || operand.find("IY") != std::string::npos;
}

uint32_t EdtasmM80PlusPlusSyntaxParser::EstimateZ80InstructionSize(const DirectiveContext& ctx) {
  const std::string& mnemonic = ctx.mnemonic;
  const std::string& operand = ctx.operand;

  // ED-prefixed LD I/R instructions (2 bytes)
  if (mnemonic.starts_with("LD") && HasIRRegisterOperand(operand)) {
    return INSTRUCTION_SIZE_TWO_BYTES;
  }

  // DD/FD-prefixed IX/IY instructions
  if (HasIndexRegisterOperand(operand)) {
    return EstimateIndexedInsnSize(operand);
  }

  // 16-bit immediate: opcode + 16-bit value = 3 bytes
  if (operand.find(',') != std::string::npos && operand.find('$') != std::string::npos) {
    uint32_t size = EstimateImmediate16BitSize(Trim(operand));
    if (size > 0) {
      return size;
    }
  }

  // Generic comma operand → opcode + 8-bit immediate = 2 bytes
  if (operand.find(',') != std::string::npos) {
    return INSTRUCTION_SIZE_TWO_BYTES;
  }

  // Relative jumps / absolute jumps+calls / RST → per-mnemonic sizing
  static const std::unordered_map<std::string, uint32_t> kMnemonicSizes = {
      {std::string(JR), INSTRUCTION_SIZE_TWO_BYTES},
      {std::string(DJNZ), INSTRUCTION_SIZE_TWO_BYTES},
      {std::string(JP), INSTRUCTION_SIZE_THREE_BYTES},
      {std::string(CALL), INSTRUCTION_SIZE_THREE_BYTES},
      {std::string(RST), INSTRUCTION_SIZE_SINGLE_BYTE},
  };
  auto it = kMnemonicSizes.find(mnemonic);
  if (it != kMnemonicSizes.end()) {
    return it->second;
  }

  return INSTRUCTION_SIZE_SINGLE_BYTE;
}

// Returns true when @p c is a valid assembler label identifier character.
static bool IsLabelChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$' || c == '.' ||
         c == '?';
}

// Returns true when all characters in @p s are valid label characters.
static bool IsValidLabel(const std::string& s) {
  for (char c : s) {
    if (!IsLabelChar(c)) {
      return false;
    }
  }
  return true;
}

// Returns true when the label's first character indicates a local scope label
// (starts with '.', '$', or '?').
static bool IsLocalScopeLabel(const std::string& label) {
  return label[0] == '.' || label[0] == '$' || label[0] == '?';
}

std::string EdtasmM80PlusPlusSyntaxParser::ParseLabel(const std::string& line, size_t& pos,
                                                      Section& section,
                                                      ConcreteSymbolTable& symbols) {
  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos || colon_pos == 0) {
    return "";
  }

  const std::string potential_label = Trim(line.substr(0, colon_pos));
  if (!IsValidLabel(potential_label)) {
    return "";
  }

  // Determine if public label (::) or private (:)
  const bool is_public = (colon_pos + 1 < line.size() && line[colon_pos + 1] == ':');
  pos = colon_pos + (is_public ? 2 : 1);

  symbols.DefineLabel(potential_label, current_address_);

  if (is_public) {
    Symbol* symbol = symbols.GetSymbol(potential_label);
    if (symbol != nullptr) {
      symbol->is_exported = true;
    }
  }

  if (!macro_local_labels_.contains(potential_label)) {
    auto label_atom = std::make_shared<LabelAtom>(potential_label, current_address_);
    label_atom->location = SourceLocation(current_file_, current_line_, 1);
    label_atom->source_line = line;
    section.atoms.push_back(label_atom);
  }

  if (IsLocalScopeLabel(potential_label)) {
    current_scope_.local_labels[potential_label] = current_address_;
  } else {
    current_scope_.global_label = potential_label;
    current_scope_.local_labels.clear();
  }

  return potential_label;
}

// ============================================================================
// Number Parsing Helpers
// ============================================================================

/**
 * @brief Parse a hex number literal, throwing on failure.
 *
 * Accepts a string that has already been prefixed with '$' for ParseHexSafe.
 */
static uint32_t ParseHexOrThrow(const std::string& hex_str) {
  bool success = false;
  std::string error_msg;
  uint32_t result = ParseHexSafe(hex_str, success, error_msg);
  if (!success) {
    throw std::invalid_argument(error_msg);
  }
  return result;
}

/**
 * @brief Parse hex formats: $FF, 0xFF, 0FFH.
 *
 * Returns the parsed value.  The caller must ensure @p trimmed starts with
 * HEX_PREFIX_DOLLAR, "0x"/"0X", or ends with 'H'/'h'.
 */
uint32_t EdtasmM80PlusPlusSyntaxParser::ParseHexVariant(const std::string& trimmed) const {
  if (trimmed[0] == HEX_PREFIX_DOLLAR) {
    return ParseHexOrThrow(trimmed);
  }
  if (trimmed.size() >= 2 && trimmed[0] == '0' &&
      (trimmed[1] == HEX_PREFIX_0X || trimmed[1] == 'X')) {
    return ParseHexOrThrow("$" + trimmed.substr(2));
  }
  // H/h suffix
  return ParseHexOrThrow("$" + trimmed.substr(0, trimmed.size() - 1));
}

/**
 * @brief Parse numeric literals with an explicit base suffix (B/O/Q/D).
 *
 * Returns true and sets @p result on success; returns false if @p trimmed
 * does not carry a recognised suffix.
 */
static bool TryParseSuffixed(const std::string& trimmed, uint32_t& result) {
  if (trimmed.size() < 2) {
    return false;
  }
  const char suffix = static_cast<char>(std::toupper(trimmed.back()));
  const std::string body = trimmed.substr(0, trimmed.size() - 1);
  if (suffix == 'B') {
    result = static_cast<uint32_t>(ParseBinary(body));
    return true;
  }
  if (suffix == 'O' || suffix == 'Q') {
    result = static_cast<uint32_t>(ParseOctal(body));
    return true;
  }
  if (suffix == 'D') {
    result = static_cast<uint32_t>(ParseDecimal(body));
    return true;
  }
  return false;
}

/**
 * @brief Parse a number using the currently active radix.
 */
uint32_t EdtasmM80PlusPlusSyntaxParser::ParseByCurrentRadix(const std::string& trimmed) const {
  if (current_radix_ == RADIX_BINARY) {
    return static_cast<uint32_t>(ParseBinary(trimmed));
  }
  if (current_radix_ == RADIX_OCTAL) {
    return static_cast<uint32_t>(ParseOctal(trimmed));
  }
  if (current_radix_ == RADIX_DECIMAL) {
    return static_cast<uint32_t>(ParseDecimal(trimmed));
  }
  if (current_radix_ == RADIX_HEXADECIMAL) {
    return ParseHexOrThrow("$" + trimmed);
  }
  return static_cast<uint32_t>(std::stoul(trimmed, nullptr, current_radix_));
}

uint32_t EdtasmM80PlusPlusSyntaxParser::ParseNumber(const std::string& str) const {
  std::string trimmed = Trim(str);
  if (trimmed.empty()) {
    return 0;
  }

  // Explicit hex prefixes/suffixes override the active radix
  const char last = static_cast<char>(std::toupper(trimmed.back()));
  const bool is_hex = (trimmed[0] == HEX_PREFIX_DOLLAR) ||
                      (trimmed.size() >= 2 && trimmed[0] == '0' &&
                       (trimmed[1] == HEX_PREFIX_0X || trimmed[1] == 'X')) ||
                      (trimmed.size() >= 2 && last == 'H');
  if (is_hex) {
    return ParseHexVariant(trimmed);
  }

  // Other explicit suffixes (B, O, Q, D)
  uint32_t result = 0;
  if (TryParseSuffixed(trimmed, result)) {
    return result;
  }

  // No explicit format — use the current radix
  return ParseByCurrentRadix(trimmed);
}

std::shared_ptr<Expression> EdtasmM80PlusPlusSyntaxParser::ParseExpression(
    const std::string& str, ConcreteSymbolTable& symbols) {
  // Create ExpressionParser with symbol table, Z80 number parser, and Z80
  // dialect features (allow_bracket_grouping for [expr] syntax — ADR-005 V7)
  ExpressionParser parser(&symbols, &z80_number_parser_, ParserFeatures::ForZ80());
  return parser.Parse(str);
}

std::string EdtasmM80PlusPlusSyntaxParser::FormatError(const std::string& message) const {
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
void EdtasmM80PlusPlusSyntaxParser::ExpandAndParseLines(const std::vector<std::string>& lines,
                                                        Section& section,
                                                        ConcreteSymbolTable& symbols) {
  // Note: Don't reset exitm_triggered_ here - let caller handle it
  // so EXITM can properly exit from REPT/IRP/IRPC loops
  for (const auto& line : lines) {
    if (exitm_triggered_) {
      break;  // Exit early if EXITM was encountered
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
    const std::string& line, const std::vector<std::string>& param_names,
    const std::vector<std::string>& param_values) {
  std::string result = line;

  // Substitute each parameter
  for (size_t i = 0; i < param_names.size(); ++i) {
    const std::string& param_name = param_names[i];
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
// Returns true when the next character after the end of a found token is not
// a word character (i.e. the occurrence is a complete word/reference).
static bool IsBoundaryChar(char c) {
  return !std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '$' && c != '.' &&
         c != '?' && c != ':';
}

// Replace whole-word occurrences of @p label in @p text with @p replacement.
static std::string ReplaceWholeWord(const std::string& text, const std::string& label,
                                    const std::string& replacement) {
  std::string result = text;
  size_t pos = 0;
  while ((pos = result.find(label, pos)) != std::string::npos) {
    const bool prev_ok = (pos == 0) || IsBoundaryChar(result[pos - 1]);
    const bool next_ok =
        (pos + label.size() >= result.size()) || IsBoundaryChar(result[pos + label.size()]);
    if (prev_ok && next_ok) {
      result.replace(pos, label.size(), replacement);
      pos += replacement.size();
    } else {
      pos += label.size();
    }
  }
  return result;
}

std::string EdtasmM80PlusPlusSyntaxParser::MakeLocalLabelUnique(
    const std::string& line, const std::vector<std::string>& local_labels, int unique_id) {
  std::string result = line;

  for (const auto& label : local_labels) {
    const std::string unique_label = label + "_" + std::to_string(unique_id);

    // First: replace "label:" occurrences (definition sites)
    size_t pos = 0;
    while ((pos = result.find(label + ":", pos)) != std::string::npos) {
      result.replace(pos, label.size(), unique_label);
      pos += unique_label.size() + 1;
    }

    // Then: replace standalone references (not followed by ':')
    result = ReplaceWholeWord(result, label, unique_label);
  }

  return result;
}

std::vector<std::string> EdtasmM80PlusPlusSyntaxParser::ParseSymbolList(
    const std::string& operand) {
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

}  // namespace xasm
