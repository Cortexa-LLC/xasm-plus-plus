/**
 * @file scmasm_directive_handlers.cpp
 * @brief SCMASM directive handler implementations
 *
 * Free function implementations of SCMASM directive handlers.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#include "xasm++/directives/scmasm_directive_handlers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/directive_error_utils.h"
#include "xasm++/directives/scmasm_constants.h"
#include "xasm++/directives/scmasm_directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/parse_utils.h"  // For radix parsing utilities
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include "xasm++/util/string_utils.h"  // For ToUpper

namespace xasm::scmasm {

using namespace directive_utils;

// ============================================================================
// Helper Functions (internal)
// ============================================================================

namespace {

using namespace constants;

/**
 * @brief Trim whitespace from both ends of string
 */
std::string Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

/**
 * @brief Parse string with delimiter semantics
 *
 * Extracts string from operand as plain 7-bit ASCII. The delimiter character
 * is used only to find the start/end of the string; it does NOT affect the
 * encoding of the string bytes.  The high-bit rule (delimiter < 0x27 → set
 * bit 7) applies only to single-character literals in instruction operands
 * (handled by ExpandCharLiteralsInExpr), not to data string directives.
 *
 * @param operand String operand (with delimiters)
 * @param result Output vector of bytes
 * @return Delimiter character used
 * @throws std::runtime_error if string is malformed
 */
char ParseString(const std::string& operand, std::vector<uint8_t>& result) {
  result.clear();

  std::string trimmed = Trim(operand);

  // Find delimiter (first character)
  char delimiter = trimmed[0];

  // Find closing delimiter. Original SCMASM accepts end-of-line as terminator
  // when no closing delimiter is found (lenient parsing).
  size_t end = trimmed.find(delimiter, 1);
  if (end == std::string::npos) {
    end = trimmed.size();
  }

  // Extract string content as plain 7-bit ASCII (no high-bit manipulation).
  for (size_t i = 1; i < end; ++i) {
    result.push_back(static_cast<uint8_t>(trimmed[i]));
  }

  return delimiter;
}

// Apply INVERTED high-bit rule (for .PS directive)
// Delimiter >= 0x27: SET high bit
// Delimiter < 0x27: CLEAR high bit
uint8_t ApplyInvertedHighBitRule(HighBitChars hbc) {
  auto result = static_cast<uint8_t>(hbc.input);

  if (hbc.delimiter >= HIGH_BIT_DELIMITER_THRESHOLD) {
    // Set high bit (inverted from normal rule)
    result |= HIGH_BIT_MASK;
  } else {
    // Clear high bit (inverted from normal rule)
    result &= LOW_7_BITS_MASK;
  }

  return result;
}

// Parse string with INVERTED high-bit rule (for .PS)
char ParseStringInverted(const std::string& operand, std::vector<uint8_t>& result) {
  result.clear();

  std::string trimmed = Trim(operand);

  // Find delimiter (first character)
  char delimiter = trimmed[0];

  // Find closing delimiter. Original SCMASM accepts end-of-line as terminator
  // when no closing delimiter is found (lenient parsing).
  size_t end = trimmed.find(delimiter, 1);
  if (end == std::string::npos) {
    end = trimmed.size();
  }

  // Extract string content (between delimiters) with inverted high-bit rule
  for (size_t i = 1; i < end; ++i) {
    char c = trimmed[i];
    uint8_t byte = ApplyInvertedHighBitRule({c, delimiter});
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
uint32_t EvaluateExpression(const std::string& str, ConcreteSymbolTable& symbols,
                            void* parser_state) {
  // For now, delegate to the parser's method (via context)
  // In future refactoring, expression evaluation could be fully standalone
  ValidateParser(parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(parser_state);

  // Use parser's EvaluateExpression method
  // Note: This creates a temporary coupling that could be eliminated
  // in future phases by making expression evaluation fully standalone
  return parser->EvaluateExpression(str, symbols);
}

}  // anonymous namespace

// ============================================================================
// Directive Handlers
// ============================================================================

void HandleOr(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately before dispatch

  RequireOperand(operand, ".OR", context);

  // Strip inline comment: anything after the first whitespace is ignored.
  // This matches SCMASM convention — e.g. ".OR ZPTMP   6 Bytes" is valid.
  std::string addr_expr = Trim(operand);
  size_t ws = addr_expr.find_first_of(" \t");
  if (ws != std::string::npos) {
    addr_expr = addr_expr.substr(0, ws);
  }

  // Evaluate address expression
  uint32_t address = EvaluateExpression(addr_expr, *context.symbols, context.parser_state);

  // Inside a .DUMMY/.ED block, .OR repositions the dummy-section PC so that
  // labels defined there get the correct zero-page (or other) address.
  // We emit a DummyOrgAtom so that ResolveSymbols() can track the ZP address
  // when it walks the atom list, while the code emitter (EncodeInstructions /
  // binary output) silently skips it and leaves the real program counter alone.
  // Outside a dummy section, .OR emits a normal OrgAtom that moves the real PC.
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (parser->InDummySection()) {
    auto dummy_org_atom = std::make_shared<DummyOrgAtom>(address);
    context.section->atoms.push_back(dummy_org_atom);
  } else {
    auto org_atom = std::make_shared<OrgAtom>(address);
    context.section->atoms.push_back(org_atom);
  }

  // Update current address
  *context.current_address = address;
}

void HandleEq(DirectiveContext& context) {
  const std::string& operand = context.operand;
  RequireOperand(operand, ".EQ", context);

  // Trim operand to remove CRLF line endings and whitespace
  std::string value_expr = Trim(operand);

  // Comment stripping: everything after the first whitespace NOT inside a
  // quoted char literal is a comment.  Walk char-by-char so that operands
  // like ' ' (space char literal) and '*' are not truncated prematurely.
  {
    size_t j = 0;
    while (j < value_expr.size()) {
      char ch = value_expr[j];
      // Skip over 'X', 'X', "X", "X" quoted char literals
      if ((ch == '\'' || ch == '"') && j + 1 < value_expr.size()) {
        j += 2;  // delimiter + char
        // Consume optional matching closing delimiter
        if (j < value_expr.size() && value_expr[j] == ch) {
          j++;
        }
        continue;
      }
      if (ch == ' ' || ch == '\t') {
        value_expr = value_expr.substr(0, j);
        break;
      }
      j++;
    }
  }

  // Normalize label to uppercase for case-insensitive SCMASM compatibility
  std::string norm_label = util::ToUpper(context.label);

  // For local labels (':N' or '.N'), use the scoped name to match how branch
  // operands reference them (via LocalLabelScope in ParseLine). This is
  // critical for ':1 .EQ *' patterns inside macros (e.g. INCW.G), where each
  // macro invocation must have its own per-invocation scope prefix so that the
  // EquateAtom re-evaluates the correct scoped symbol across multi-pass runs.
  if (context.parser_state && !context.label.empty() &&
      (context.label[0] == ':' || context.label[0] == '.')) {
    auto* eq_parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
    norm_label = eq_parser->ScopedLocalLabelName(norm_label);
  }

  // Evaluate value expression.  If a symbol referenced in the expression is
  // not yet defined (forward reference, or defined only in another translation
  // unit that is not included by this source file), use 0 as a placeholder and
  // push an EquateAtom so the symbol is re-evaluated on every assembly pass.
  // This matches original SCMASM/vasm behaviour where undefined symbols in
  // .EQ expressions default to 0 rather than aborting the parse.
  uint32_t value = 0;
  bool needs_reassembly_eval = false;
  try {
    value = EvaluateExpression(value_expr, *context.symbols, context.parser_state);
  } catch (const std::exception&) {
    // Undefined symbol or other evaluation error — use 0 and defer to assembly
    value = 0;
    needs_reassembly_eval = true;
  }

  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(norm_label, SymbolType::Equate, expr);

  // Push an EquateAtom when:
  // 1. The expression contains '*' (current address) — must track PC changes
  //    across branch-relaxation passes; OR
  // 2. Evaluation failed above (forward/undefined reference) — must retry
  //    during assembly passes once all symbols are defined.
  //
  // EXCEPTION for case 1: inside a .DUMMY section, '*' refers to the
  // dummy-section's ZP address counter which is fixed, so no EquateAtom needed.
  bool in_dummy = context.parser_state &&
                  static_cast<ScmasmSyntaxParser*>(context.parser_state)->InDummySection();
  bool has_star = value_expr.find('*') != std::string::npos;
  if ((has_star && !in_dummy) || needs_reassembly_eval) {
    auto eq_atom = std::make_shared<EquateAtom>(norm_label, value_expr);
    context.section->atoms.push_back(eq_atom);
  }
}

void HandleSe(DirectiveContext& context) {
  const std::string& operand = context.operand;
  RequireOperand(operand, ".SE", context);

  // Strip inline comment: anything after first whitespace is ignored.
  std::string val_expr = Trim(operand);
  size_t ws = val_expr.find_first_of(" \t");
  if (ws != std::string::npos) {
    val_expr = val_expr.substr(0, ws);
  }

  // Evaluate value expression
  uint32_t value = EvaluateExpression(val_expr, *context.symbols, context.parser_state);

  // .SE creates Set type (redefinable)
  // Normalize label to uppercase for case-insensitive SCMASM compatibility
  auto expr = std::make_shared<LiteralExpr>(value);
  context.symbols->Define(util::ToUpper(context.label), SymbolType::Set, expr);
}

void HandleAs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".AS", context);

  // SCMASM prefix modifiers before the opening delimiter:
  //   -"text"  set high bit on the LAST byte (same as .AT)
  // Strip the prefix and delegate appropriately.
  std::string trimmed_op = Trim(operand);
  bool high_bit_last = false;
  if (!trimmed_op.empty() && trimmed_op[0] == '-') {
    high_bit_last = true;
    trimmed_op = Trim(trimmed_op.substr(1));
  }

  std::vector<uint8_t> data;
  ParseString(trimmed_op, data);

  if (high_bit_last && !data.empty()) {
    data.back() |= constants::HIGH_BIT_MASK;
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleAt(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".AT", context);

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

void HandleAz(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".AZ", context);

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
      b |= constants::HIGH_BIT_MASK;
    }
  }

  // Add null terminator (always plain, no high bit)
  data.push_back(constants::NULL_TERMINATOR);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

// ============================================================================
// HandleDa helpers
// ============================================================================

namespace {

/**
 * @brief Token produced by TokenizeDaOperand.
 *
 * Represents one comma-separated entry in a .DA operand, or signals that an
 * inline comment was encountered (comment_ended == true).
 */
struct DaToken {
  std::string expr;           ///< Trimmed expression (empty when comment_ended)
  bool comment_ended{false};  ///< True when an inline comment stopped scanning
};

// ============================================================================
// TokenizeDaOperand helpers
// ============================================================================

/// Advances @p pos past a $$"..." or $$'...' literal.
/// Returns true and updates @p pos if a match was found; otherwise no-op.
bool TrySkipDollarDollarLiteral(const std::string& s, size_t& pos) {
  if (pos + 2 < s.length() && s[pos] == '$' && s[pos + 1] == '$' &&
      (s[pos + 2] == '"' || s[pos + 2] == '\'')) {
    char delim = s[pos + 2];
    pos += 3;
    while (pos < s.length() && s[pos] != delim) {
      ++pos;
    }
    if (pos < s.length()) {
      ++pos;  // skip closing delimiter
    }
    return true;
  }
  return false;
}

/// Advances @p pos past a "X" or 'X' single-character quoted literal.
/// Returns true and updates @p pos if a match was found; otherwise no-op.
bool TrySkipSingleCharLiteral(const std::string& s, size_t& pos) {
  if (pos + 2 < s.length() && (s[pos] == '"' || s[pos] == '\'') && s[pos + 2] == s[pos]) {
    pos += 3;  // opening-delim + char-content + closing-delim
    return true;
  }
  return false;
}

/// Returns the position of the first unquoted space or tab in @p token,
/// or std::string::npos if none found.
size_t FindUnquotedWhitespace(const std::string& token) {
  size_t j = 0;
  while (j < token.size()) {
    char ch = token[j];
    // Skip single-char quoted literals so spaces inside them are ignored
    if ((ch == '"' || ch == '\'') && j + 2 < token.size() && token[j + 2] == ch) {
      j += 3;
      continue;
    }
    if (ch == ' ' || ch == '\t') {
      return j;
    }
    ++j;
  }
  return std::string::npos;
}

/**
 * @brief Extract all .DA tokens from the operand string.
 *
 * Scans @p src for comma-separated expressions, honouring:
 *  - $$"..." / $$'...' string literals (commas inside are not separators)
 *  - Single-char quoted literals ('A', "A") — spaces inside quotes are NOT
 *    treated as inline-comment starts
 *  - Inline comments: unquoted whitespace inside a token terminates the list
 *
 * Returns the list of tokens in order; if an inline comment was detected the
 * final DaToken has comment_ended == true and an empty expr.
 */
std::vector<DaToken> TokenizeDaOperand(const std::string& src) {
  std::string trimmed = Trim(src);
  std::vector<DaToken> tokens;
  size_t start = 0;
  size_t pos = 0;

  while (pos <= trimmed.length()) {
    // Skip quoted literals so commas/spaces inside them are not separators.
    if (TrySkipDollarDollarLiteral(trimmed, pos)) {
      continue;
    }
    if (TrySkipSingleCharLiteral(trimmed, pos)) {
      continue;
    }

    if (pos == trimmed.length() || trimmed[pos] == ',') {
      std::string token = trimmed.substr(start, pos - start);
      size_t ws_pos = FindUnquotedWhitespace(token);

      if (ws_pos != std::string::npos) {
        // Everything from ws_pos onward belongs to the inline comment.
        std::string expr = Trim(token.substr(0, ws_pos));
        if (!expr.empty()) {
          tokens.push_back({expr, false});
        }
        tokens.push_back({"", true});  // sentinel: inline comment found
        return tokens;
      }

      std::string expr = Trim(token);
      if (!expr.empty()) {
        tokens.push_back({expr, false});
      }

      start = pos + 1;
    }
    ++pos;
  }

  return tokens;
}

/**
 * @brief Emit bytes/expressions for one .DA expression token.
 *
 * Interprets the SCMASM prefix (#, /, <, >) to determine width, then
 * appends the appropriate byte placeholder(s) to @p data and the
 * corresponding expression string(s) to @p byte_expressions.
 *
 * Prefix summary:
 *   #expr → 8-bit low byte   (<expr)
 *   /expr → 8-bit high byte  (>expr)
 *   <expr → 24-bit little-endian (3 bytes)
 *   >expr → 32-bit little-endian (4 bytes)
 *   expr  → 16-bit default   (2 bytes, little-endian)
 */

/// Strip trailing inline comment from a .DA token expression.
/// Scans char-by-char to avoid treating spaces inside quoted literals as
/// comment starts (e.g. the space in #" " must not trigger truncation).
std::string StripTrailingComment(const std::string& expr) {
  std::string result = Trim(expr);
  size_t j = 0;
  while (j < result.size()) {
    char ch = result[j];
    // Skip single-char quoted literals ("X" or 'X')
    if ((ch == '"' || ch == '\'') && j + 2 < result.size() && result[j + 2] == ch) {
      j += 3;
      continue;
    }
    if (ch == ' ' || ch == '\t') {
      result = result.substr(0, j);
      break;
    }
    ++j;
  }
  return Trim(result);
}

/// Emit bytes for a $$"..." Apple II encoded string in a .DA operand.
void EmitDollarDollarString(const std::string& trimmed_expr,
                            std::vector<std::string>& byte_expressions,
                            std::vector<uint8_t>& data) {
  char delim = trimmed_expr[2];
  std::string str_content = trimmed_expr.substr(3);
  if (!str_content.empty() && str_content.back() == delim) {
    str_content.pop_back();
  }
  for (char c : str_content) {
    uint8_t byte = static_cast<uint8_t>(c);
    if (delim >= HIGH_BIT_DELIMITER_THRESHOLD) {
      byte |= HIGH_BIT_MASK;
    } else {
      byte &= LOW_7_BITS_MASK;
    }
    data.push_back(byte);
    byte_expressions.push_back(std::to_string(byte));
  }
}

/// Emit 1 low byte for .DA #expr.
void EmitDaLowByte(const std::string& base_expr, const DirectiveContext& context,
                   std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  byte_expressions.push_back("<(" + base_expr + ")");
  try {
    uint32_t num = EvaluateExpression(base_expr, *context.symbols, context.parser_state);
    data.push_back(static_cast<uint8_t>(num & BYTE_MASK));
  } catch (...) {
    data.push_back(0);
  }
}

/// Emit 1 high byte for .DA /expr.
void EmitDaHighByte(const std::string& base_expr, const DirectiveContext& context,
                    std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  byte_expressions.push_back(">(" + base_expr + ")");
  try {
    uint32_t num = EvaluateExpression(base_expr, *context.symbols, context.parser_state);
    data.push_back(static_cast<uint8_t>((num >> BYTE_1_SHIFT) & BYTE_MASK));
  } catch (...) {
    data.push_back(0);
  }
}

/// Emit 3 bytes (little-endian 24-bit) for .DA <expr.
void EmitDa24Bit(const std::string& base_expr, const DirectiveContext& context,
                 std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  byte_expressions.push_back("<(" + base_expr + ")");
  byte_expressions.push_back(">(" + base_expr + ")");
  byte_expressions.push_back("<((" + base_expr + ")/65536)");
  try {
    uint32_t num = EvaluateExpression(base_expr, *context.symbols, context.parser_state);
    data.push_back(static_cast<uint8_t>(num & BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> BYTE_1_SHIFT) & BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> BYTE_2_SHIFT) & BYTE_MASK));
  } catch (...) {
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
  }
}

/// Emit 4 bytes (little-endian 32-bit) for .DA >expr.
void EmitDa32Bit(const std::string& base_expr, const DirectiveContext& context,
                 std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  byte_expressions.push_back("<(" + base_expr + ")");
  byte_expressions.push_back(">(" + base_expr + ")");
  byte_expressions.push_back("<((" + base_expr + ")/65536)");
  byte_expressions.push_back("<((" + base_expr + ")/16777216)");
  try {
    uint32_t num = EvaluateExpression(base_expr, *context.symbols, context.parser_state);
    data.push_back(static_cast<uint8_t>(num & constants::BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> constants::BYTE_1_SHIFT) & constants::BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> constants::BYTE_2_SHIFT) & constants::BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> constants::BYTE_3_SHIFT) & constants::BYTE_MASK));
  } catch (...) {
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
    data.push_back(0);
  }
}

/// Emit 2 bytes (little-endian 16-bit) for .DA expr (no prefix).
void EmitDa16Bit(const std::string& base_expr, const DirectiveContext& context,
                 std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  byte_expressions.push_back("<(" + base_expr + ")");
  byte_expressions.push_back(">(" + base_expr + ")");
  try {
    uint32_t num = EvaluateExpression(base_expr, *context.symbols, context.parser_state);
    data.push_back(static_cast<uint8_t>(num & BYTE_MASK));
    data.push_back(static_cast<uint8_t>((num >> BYTE_1_SHIFT) & BYTE_MASK));
  } catch (...) {
    data.push_back(0);
    data.push_back(0);
  }
}

void ProcessDaExpression(const std::string& raw_expr, const DirectiveContext& context,
                         std::vector<std::string>& byte_expressions, std::vector<uint8_t>& data) {
  std::string trimmed_expr = StripTrailingComment(raw_expr);

  if (trimmed_expr.empty()) {
    return;
  }

  // Handle $$"..." Apple II encoded strings in .DA.
  // Delimiter < 0x27 → clear high bit; delimiter >= 0x27 → set high bit.
  if (trimmed_expr.size() >= 3 && trimmed_expr[0] == '$' && trimmed_expr[1] == '$') {
    EmitDollarDollarString(trimmed_expr, byte_expressions, data);
    return;
  }

  // Expand character literals BEFORE checking prefix so that #'N' becomes
  // #$4E before the '#' is stripped.
  ValidateParser(context.parser_state);
  trimmed_expr = ScmasmSyntaxParser::ExpandCharLiteralsInExpr(trimmed_expr);

  // Dispatch by prefix character to the appropriate emit helper.
  // Each helper appends the correct number of byte placeholders and data bytes.
  char prefix = trimmed_expr[0];
  std::string base_expr = Trim(trimmed_expr.substr(1));

  if (prefix == '#') {
    EmitDaLowByte(base_expr, context, byte_expressions, data);
  } else if (prefix == '/') {
    EmitDaHighByte(base_expr, context, byte_expressions, data);
  } else if (prefix == '<') {
    EmitDa24Bit(base_expr, context, byte_expressions, data);
  } else if (prefix == '>') {
    EmitDa32Bit(base_expr, context, byte_expressions, data);
  } else {
    // Default: 16-bit word (no recognised prefix) → 2 bytes (little-endian).
    // Use the full expression unchanged (prefix is part of the expression).
    EmitDa16Bit(trimmed_expr, context, byte_expressions, data);
  }
}

}  // anonymous namespace

// ============================================================================
// HandleDa
// ============================================================================

void HandleDa(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  // In SCMASM, .DA operands are comma-separated, but each operand may be
  // followed by whitespace-delimited comment text on the same line. A comment
  // begins at the first whitespace inside a token (i.e. after the expression
  // value). Crucially, any comma that appears inside the comment text must NOT
  // be treated as a .DA list separator.
  //
  // Example (from LIBBLKDEV.S.txt):
  //   .DA #$61   6502,Level 1 (65c02)
  // Here "6502,Level 1 (65c02)" is a comment. The comma after "6502" must NOT
  // trigger a second .DA entry.
  std::vector<std::string> byte_expressions;
  std::vector<uint8_t> data;

  for (const auto& tok : TokenizeDaOperand(operand)) {
    if (tok.comment_ended) {
      break;
    }
    ProcessDaExpression(tok.expr, context, byte_expressions, data);
  }

  if (!data.empty()) {
    auto atom = std::make_shared<DataAtom>(byte_expressions, DataSize::Byte);
    atom->data = data;
    atom->size = data.size();
    context.section->atoms.push_back(atom);
    *context.current_address += data.size();
  }
}

// ============================================================================
// HandleHs helpers
// ============================================================================

namespace {

/// Remove dot separators from a .HS operand string.
/// Dots are purely visual nibble-separators in SCMASM .HS syntax.
std::string RemoveHsDots(const std::string& s) {
  std::string result;
  result.reserve(s.size());
  for (char ch : s) {
    if (ch != '.') {
      result += ch;
    }
  }
  return result;
}

/// Result of scanning one whitespace run in a .HS operand.
enum class HsWhitespaceAction { kContinue, kStop, kEndOfString };

/// Skip whitespace at position @p i in @p s.
/// Returns the action the caller should take:
///   kStop         – 2+ tabs after data → treat rest as comment, stop
///   kEndOfString  – reached end of string
///   kContinue     – safe to process next word
HsWhitespaceAction SkipHsWhitespace(const std::string& s, size_t& i, bool has_data) {
  size_t tabs = 0;
  while (i < s.length() && std::isspace(s[i])) {
    if (s[i] == '\t') {
      ++tabs;
    }
    ++i;
  }
  if (i >= s.length()) {
    return HsWhitespaceAction::kEndOfString;
  }
  if (has_data && tabs >= 2) {
    return HsWhitespaceAction::kStop;
  }
  return HsWhitespaceAction::kContinue;
}

/// Result of processing one hex word in a .HS operand.
enum class HsWordResult { kAppended, kOddBeforeData, kStop };

/// Process one hex word starting at @p word_start in @p s ending at @p i.
/// Appends valid even-length hex words to @p hex_digits.
/// Returns a result code indicating whether to continue, stop, or flag error.
HsWordResult ProcessHsWord(const std::string& s, size_t word_start, size_t i,
                           std::string& hex_digits) {
  std::string word = s.substr(word_start, i - word_start);

  bool all_hex = true;
  for (char c : word) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      all_hex = false;
      break;
    }
  }

  if (all_hex && (word.length() % 2 == 0)) {
    hex_digits += word;
    return HsWordResult::kAppended;
  }
  if (all_hex && (word.length() % 2 != 0)) {
    if (hex_digits.empty()) {
      return HsWordResult::kOddBeforeData;
    }
    return HsWordResult::kStop;  // odd-length hex after valid data → comment
  }
  return HsWordResult::kStop;  // non-hex word → inline comment
}

/// Extract valid hex digits from a normalised (dots removed) .HS operand.
/// Sets @p odd_hex_before_data to true if a malformed odd-length hex word
/// appears before any valid data.
std::string ExtractHsHexDigits(const std::string& normalised, bool& odd_hex_before_data) {
  std::string hex_digits;
  odd_hex_before_data = false;
  size_t i = 0;
  while (i < normalised.length()) {
    HsWhitespaceAction action = SkipHsWhitespace(normalised, i, !hex_digits.empty());
    if (action == HsWhitespaceAction::kEndOfString) {
      break;
    }
    if (action == HsWhitespaceAction::kStop) {
      break;
    }

    size_t word_start = i;
    while (i < normalised.length() && !std::isspace(normalised[i])) {
      ++i;
    }

    if (i > word_start) {
      HsWordResult result = ProcessHsWord(normalised, word_start, i, hex_digits);
      if (result == HsWordResult::kOddBeforeData) {
        odd_hex_before_data = true;
        break;
      }
      if (result == HsWordResult::kStop) {
        break;
      }
    }
  }
  return hex_digits;
}

}  // anonymous namespace

void HandleHs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  std::vector<uint8_t> data;
  std::string normalised = RemoveHsDots(Trim(operand));

  bool odd_hex_before_data = false;
  std::string hex_digits = ExtractHsHexDigits(normalised, odd_hex_before_data);

  if (odd_hex_before_data) {
    ThrowFormattedError(".HS requires even number of hex digits", context);
  }
  if (hex_digits.length() % constants::HEX_DIGITS_PER_BYTE != 0) {
    ThrowFormattedError(".HS requires even number of hex digits", context);
  }

  for (size_t i = 0; i < hex_digits.length(); i += constants::HEX_DIGITS_PER_BYTE) {
    std::string byte_str = "$" + hex_digits.substr(i, constants::HEX_DIGITS_PER_BYTE);
    bool success = false;
    std::string error_msg;
    uint32_t byte_val = xasm::ParseHexSafe(byte_str, success, error_msg);
    if (!success) {
      ThrowFormattedError(error_msg, context);
    }
    data.push_back(static_cast<uint8_t>(byte_val));
  }

  ValidateParser(context.parser_state);
  auto* hs_parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (hs_parser->InDummySection()) {
    *context.current_address += data.size();
    return;
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);
  *context.current_address += data.size();
}

void HandleBs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  // .BS (Block Storage) - Reserve N bytes of space
  // SCMASM syntax: .BS count
  // Where count is a decimal or hex number ($hex, %binary)
  // This reserves 'count' bytes filled with zeros

  if (operand.empty()) {
    ThrowFormattedError(".BS requires a byte count", context);
  }

  std::string trimmed = Trim(operand);

  // Strip inline comment: anything after first whitespace is ignored.
  // e.g. ".BS 9   9 bytes, S.IOCTL" -> evaluates "9"
  size_t ws = trimmed.find_first_of(" \t");
  if (ws != std::string::npos) {
    trimmed = trimmed.substr(0, ws);
  }

  // Evaluate the byte count expression (supports symbols, hex, decimal)
  uint32_t byte_count = EvaluateExpression(trimmed, *context.symbols, context.parser_state);

  // Validate byte count (reasonable limit: 64KB)
  if (byte_count > 65536) {
    ThrowFormattedError(".BS byte count too large (max 65536)", context);
  }

  // Check if in dummy mode (structure definition)
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

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

void HandleMa(DirectiveContext& context) {
  const std::string& operand = context.operand;
  // Macro name can come from label or operand
  std::string macro_name;

  if (!context.label.empty()) {
    macro_name = context.label;
  } else if (!operand.empty()) {
    macro_name = Trim(operand);
  } else {
    ThrowFormattedError(".MA requires a macro name", context);
  }

  // Access parser state to set macro definition mode
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

  // Delegate to parser's HandleMa method
  // Note: This maintains coupling to parser for macro state management
  // which is acceptable as macros require parser-level state tracking
  context.operand = operand;
  parser->HandleMa(context);
}

void HandleEndm(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)operand;

  // Access parser state to end macro definition
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

  // Delegate to parser's HandleEm method
  parser->HandleEm();
}

// ============================================================================
// P0 Priority Directive Handlers (A2oSX Critical)
// ============================================================================

void HandlePs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".PS", context);

  // Parse string with INVERTED high-bit rule (.PS is opposite of .AS)
  std::vector<uint8_t> data;
  ParseStringInverted(operand, data);

  // Validate length (Pascal strings are max 255 bytes)
  if (data.size() > constants::PASCAL_STRING_MAX_LENGTH) {
    ThrowFormattedError(".PS string too long (max 255 bytes)", context);
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

// ============================================================================
// HandleInb helpers
// ============================================================================

namespace {

/**
 * @brief Apply path mappings to an include filename.
 *
 * Replaces the longest matching virtual prefix with its actual counterpart.
 * Returns @p filename unchanged if no mapping matches.
 */
std::string ApplyInbPathMappings(const std::string& filename, const DirectiveContext& context) {
  if (context.path_mappings == nullptr || context.path_mappings->empty()) {
    return filename;
  }

  std::filesystem::path include_path_obj(filename);
  std::string normalized_path = include_path_obj.generic_string();

  std::string longest_match_key;
  std::string longest_match_value;
  size_t longest_match_len = 0;

  for (const auto& [virtual_path, actual_path] : *context.path_mappings) {
    std::filesystem::path virtual_path_obj(virtual_path);
    std::string normalized_virtual = virtual_path_obj.generic_string();

    if (!normalized_path.starts_with(normalized_virtual)) {
      continue;
    }

    size_t virtual_len = normalized_virtual.length();
    if (virtual_len > normalized_path.length()) {
      continue;
    }

    // Match must be at a path-component boundary
    if (virtual_len != 0 && virtual_len != normalized_path.length() &&
        normalized_path[virtual_len] != '/') {
      continue;
    }

    if (virtual_len >= longest_match_len) {
      longest_match_len = virtual_len;
      longest_match_key = normalized_virtual;
      longest_match_value = actual_path;
    }
  }

  if (longest_match_value.empty()) {
    return filename;  // No mapping matched
  }

  std::string suffix = normalized_path.substr(longest_match_len);
  if (!suffix.empty() && suffix[0] == '/') {
    suffix = suffix.substr(1);
  }

  std::filesystem::path actual_base(longest_match_value);
  if (suffix.empty()) {
    return actual_base.string();
  }
  return (actual_base / suffix).string();
}

/**
 * @brief Try to resolve @p filename to an existing path.
 *
 * Search order (mirrors the original HandleInb logic):
 *  1. Absolute path
 *  2. Relative to source-file directory
 *  3. Each include-path directory
 *  4. Current working directory
 *  5. Parent directory (A2osX compatibility)
 *
 * On success returns the resolved path; on failure returns an empty path and
 * appends every attempted location to @p tried_paths.
 */
std::filesystem::path TryResolvePath(const std::string& filename, const DirectiveContext& context,
                                     std::vector<std::string>& tried_paths) {
  std::filesystem::path include_path(filename);

  // Case 1: Absolute path
  if (include_path.is_absolute()) {
    tried_paths.push_back(filename);
    if (std::filesystem::exists(include_path)) {
      return include_path;
    }
    return {};
  }

  // Case 2: Relative to source file directory
  if (!context.current_file.empty()) {
    std::filesystem::path source_dir = std::filesystem::path(context.current_file).parent_path();
    std::filesystem::path candidate = source_dir / filename;
    tried_paths.push_back(candidate.string());
    if (std::filesystem::exists(candidate)) {
      return candidate;
    }
  }

  // Case 3: Each include-path directory
  if (context.include_paths != nullptr) {
    for (const auto& include_dir : *context.include_paths) {
      std::filesystem::path candidate = std::filesystem::path(include_dir) / filename;
      tried_paths.push_back(candidate.string());
      if (std::filesystem::exists(candidate)) {
        return candidate;
      }
    }
  }

  // Case 4: Current working directory
  tried_paths.push_back(filename);
  if (std::filesystem::exists(filename)) {
    return std::filesystem::path(filename);
  }

  // Case 5: Parent directory (A2osX compatibility — BIN/ includes INC/)
  std::filesystem::path parent_candidate = std::filesystem::path("..") / filename;
  tried_paths.push_back(parent_candidate.string());
  if (std::filesystem::exists(parent_candidate)) {
    return parent_candidate;
  }

  return {};
}

}  // anonymous namespace

// ============================================================================
// HandleInb
// ============================================================================

void HandleInb(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  // .INB - Include Source File
  // Parses assembly source file and includes it at current position.
  // Used extensively in A2osX for modular source file includes.
  //
  // Path resolution order:
  //  1. Absolute path
  //  2. Relative to source file directory
  //  3. Each include-path directory (--include-path CLI option)
  //  4. Current working directory (fallback)
  //  5. +.txt extension retry for all cases (A2osX compatibility)

  RequireOperand(operand, ".INB", context);

  std::string include_filename = ApplyInbPathMappings(Trim(operand), context);

  std::vector<std::string> tried_paths;
  std::filesystem::path resolved_path = TryResolvePath(include_filename, context, tried_paths);

  // Retry with .txt extension (A2osX compatibility: "file.s" → "file.s.txt")
  if (resolved_path.empty()) {
    std::string txt_filename = include_filename + ".txt";
    resolved_path = TryResolvePath(txt_filename, context, tried_paths);
  }

  if (resolved_path.empty()) {
    std::string error_msg = ".INB cannot open file: " + include_filename;
    if (!tried_paths.empty()) {
      error_msg += " (searched: ";
      for (size_t i = 0; i < tried_paths.size(); ++i) {
        if (i > 0) {
          error_msg += ", ";
        }
        error_msg += tried_paths[i];
      }
      error_msg += ")";
    }
    ThrowFormattedError(error_msg, context);
  }

  // Open the resolved file
  std::ifstream file(resolved_path);
  if (!file.is_open()) {
    ThrowFormattedError(".INB cannot open file: " + resolved_path.string(), context);
  }

  // Read entire file content
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source_content = buffer.str();

  // Get parser from context
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

  // Track file for error reporting
  std::string previous_file = parser->GetCurrentFile();
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

void HandleList(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Listing control parameter (ON/OFF)
  (void)context;        // No state changes needed for stub

  // Stub implementation - listing control has no effect yet
  // This allows .LIST directives to be parsed without error
  // Full listing output generation is out of scope for P0
}

void HandleDummy(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Optional operand

  // Enter dummy section mode - data directives will update address but not emit
  // bytes.  Save the current main-section PC so .ED can restore it; this
  // prevents a .OR inside the dummy block from permanently repositioning the
  // main section's current address.
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  parser->StartDummySection(*context.current_address);
}

void HandleEd(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Operand unused

  // Exit dummy section mode - restore the main-section PC to whatever it was
  // when .DUMMY was entered.  Without this, any .OR inside the dummy block
  // (e.g. ".OR ZPDRV" in a2osx.i) would permanently reposition the assembler's
  // PC, corrupting all subsequent label addresses.
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  uint32_t saved_addr = parser->EndDummySection();
  *context.current_address = saved_addr;
}

void HandleOp(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  std::string trimmed = Trim(operand);
  // Strip SCMASM inline comment: take only the first whitespace-delimited token
  {
    size_t ws = trimmed.find_first_of(" \t");
    if (ws != std::string::npos) {
      trimmed = trimmed.substr(0, ws);
    }
  }
  std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::toupper);

  // Validate CPU name (6502, 65C02, 65816)
  if (trimmed != "6502" && trimmed != "65C02" && trimmed != "65816") {
    ThrowFormattedError(".OP requires valid CPU (6502, 65C02, 65816)", context);
  }

  // Switch CPU plugin based on operand
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (trimmed == "6502") {
    parser->SetCpu("6502");
  } else if (trimmed == "65C02") {
    parser->SetCpu("65C02");
  } else if (trimmed == "65816") {
    parser->SetCpu("65816");
  }
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
uint8_t ParseEscapeSequence(const char*& s) {
  if (*s != '\\') {
    return static_cast<uint8_t>(*s++);
  }

  s++;  // Skip backslash

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
      return '\\';  // Backslash
    case '"':
      s++;
      return '"';  // Double quote
    case '\'':
      s++;
      return '\'';  // Single quote
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
// mixed_delim: when true, a double-quote (") inside a single-quote-delimited
//   string also terminates the string.  This applies only to .CS — verified
//   against STABLE.800.po:  .CS 'Usage : CUT "line of text"\r\n'  stops at
//   the first '"', emitting only "Usage : CUT ".
//   .CZ does NOT apply the rule (e.g. .CZ '%s = "%s"\r\n' emits the full
//   string including the embedded '"' characters).
void ParseCString(const std::string& operand, std::vector<uint8_t>& result,
                  bool mixed_delim = false) {
  result.clear();

  std::string trimmed = Trim(operand);

  // Find delimiter (first character)
  char delimiter = trimmed[0];

  // Find closing delimiter. In SCMASM the delimiter is never escapable —
  // the very first occurrence of the delimiter after the opening one ends
  // the string (e.g. "|/-\" ends at the '"' after '\').
  // If no closing delimiter is found, the original SCMASM accepts end-of-line
  // as the string terminator (same as if the closing delimiter was at EOL).
  size_t end = trimmed.find(delimiter, 1);
  if (mixed_delim && delimiter == '\'') {
    // .CS mixed-delimiter rule: an embedded '"' also terminates a
    // single-quote-delimited string (verified against STABLE.800.po).
    size_t alt = trimmed.find('"', 1);
    if (alt != std::string::npos && (end == std::string::npos || alt < end)) {
      end = alt;
    }
  }
  if (end == std::string::npos) {
    end = trimmed.size();  // Accept unterminated string (SCMASM compatible)
  }

  // Parse string content with escape sequences.
  const char* s = trimmed.c_str() + 1;  // Start after opening delimiter
  const char* s_end = trimmed.c_str() + end;

  while (s < s_end) {
    if (*s == '\\' && s + 1 >= s_end) {
      // Trailing backslash at end of string content — the delimiter is not
      // escapable in SCMASM, so this '\' is a literal backslash character.
      result.push_back(static_cast<uint8_t>('\\'));
      ++s;
    } else {
      uint8_t byte = ParseEscapeSequence(s);
      result.push_back(byte);
    }
  }
}

}  // anonymous namespace

void HandleCs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".CS", context);

  std::vector<uint8_t> data;
  ParseCString(operand, data, /*mixed_delim=*/true);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleCz(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".CZ", context);

  std::vector<uint8_t> data;
  ParseCString(operand, data);

  // Add null terminator
  data.push_back(constants::NULL_TERMINATOR);

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleTf(DirectiveContext& context) {
  const std::string& operand = context.operand;

  // .TF <path>[,TSYS]  — sets the output file for this assembly unit.
  // Strip optional type suffix (e.g. ",TSYS", ",TBIN") — these were ProDOS
  // file-type hints for the on-device assembler; irrelevant for cross-assembly.
  std::string path = Trim(operand);
  auto comma = path.find(',');
  if (comma != std::string::npos) {
    path = Trim(path.substr(0, comma));
  }

  if (path.empty()) {
    ThrowFormattedError(".TF requires an output file path", context);
  }

  // Create parent directories so the assembler never fails to open the file.
  std::filesystem::path out_path(path);
  std::filesystem::path parent = out_path.parent_path();
  if (!parent.empty() && !std::filesystem::exists(parent)) {
    std::filesystem::create_directories(parent);
  }

  // Store in parser so main() can retrieve it after Parse() returns.
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);
  if (parser) {
    parser->SetTfOutput(path);
  }
}

void HandleEp(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  // Cast parser state to access phase tracking
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

  // Check if we're ending a phase
  if (parser && parser->InPhase()) {
    // .EP without operand ends phase assembly
    if (operand.empty()) {
      // End phase and get new real address
      uint32_t new_real_addr = parser->EndPhase(*context.current_address);

      // Emit EndPhaseAtom so ResolveSymbols restores physical address tracking
      auto ep_atom = std::make_shared<PhaseAtom>(false, 0);
      context.section->atoms.push_back(ep_atom);

      // Restore current address to real address + bytes emitted
      *context.current_address = new_real_addr;
      return;
    }
    // .EP with operand in phase context is an error
    ThrowFormattedError(".EP with operand not allowed within .PH/.EP block", context);
  }

  // Not in phase - treat as entry point directive
  // .EP without operand sets entry to current address
  if (operand.empty()) {
    // TODO: Store current address as entry point in section metadata
    // For now, just accept the directive
    return;
  }

  // Strip inline comment then evaluate entry point address
  std::string ep_expr = Trim(operand);
  size_t ep_ws = ep_expr.find_first_of(" \t");
  if (ep_ws != std::string::npos) {
    ep_expr = ep_expr.substr(0, ep_ws);
  }
  uint32_t address = EvaluateExpression(ep_expr, *context.symbols, context.parser_state);

  // TODO: Store entry point in section metadata
  // For now, just validate the expression
  (void)address;
}

void HandlePh(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  RequireOperand(operand, ".PH", context);

  // Cast parser state to access phase tracking
  ValidateParser(context.parser_state);
  auto* parser = static_cast<ScmasmSyntaxParser*>(context.parser_state);

  // Strip inline comment then evaluate virtual address
  std::string ph_expr = Trim(operand);
  size_t ph_ws = ph_expr.find_first_of(" \t");
  if (ph_ws != std::string::npos) {
    ph_expr = ph_expr.substr(0, ph_ws);
  }
  uint32_t virtual_addr = EvaluateExpression(ph_expr, *context.symbols, context.parser_state);

  // Calculate current real address
  uint32_t real_addr = 0;
  if (parser->InPhase()) {
    // If already in phase, calculate real address from phase state
    real_addr = parser->GetCurrentRealAddress(*context.current_address);
  } else {
    // Not in phase, current_address IS the real address
    real_addr = *context.current_address;
  }

  parser->StartPhase({real_addr, virtual_addr});

  // Set current address to virtual address
  *context.current_address = virtual_addr;

  // Emit PhaseAtom so ResolveSymbols tracks the virtual address for labels
  auto phase_atom = std::make_shared<PhaseAtom>(true, virtual_addr);
  context.section->atoms.push_back(phase_atom);
}

void HandleHx(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately

  std::vector<uint8_t> nibbles;
  std::string trimmed = Trim(operand);

  // Parse hex nibbles - collect all nibbles first
  // Whitespace, dots, and commas are allowed as separators
  for (char c : trimmed) {
    if (std::isspace(c) || c == '.' || c == ',') {
      continue;  // Skip separators
    }

    // Convert single hex digit to nibble value (0-15)
    int val = 0;
    if (!ParseHexDigit(c, val)) {
      ThrowFormattedError("Invalid hex digit in .HX: " + std::string(1, c), context);
    }
    nibbles.push_back(static_cast<uint8_t>(val));
  }

  // Pack nibble pairs into bytes.
  // SCMASM .HX uses nibble-swapped encoding: the first nibble of each pair
  // is the LOW nibble of the byte, and the second is the HIGH nibble.
  // e.g. .HX 02 -> byte 0x20  (not 0x02 as in standard hex)
  // e.g. .HX 00022000 -> bytes 00 20 02 00  (not 00 02 20 00)
  // This reflects the Apple II 4bpp color format where pixel nibbles are
  // stored low-nibble-first within each byte.
  std::vector<uint8_t> data;
  for (size_t i = 0; i + 1 < nibbles.size(); i += 2) {
    uint8_t low_nibble = nibbles[i];
    uint8_t high_nibble = nibbles[i + 1];
    data.push_back(static_cast<uint8_t>((high_nibble << 4) | low_nibble));
  }
  // Handle trailing odd nibble (low nibble, high nibble = 0)
  if (nibbles.size() % 2 != 0) {
    data.push_back(nibbles.back());
  }

  auto atom = std::make_shared<DataAtom>(data);
  context.section->atoms.push_back(atom);

  // Update address counter
  *context.current_address += data.size();
}

void HandleTa(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Target address
  (void)context;        // No-op

  // .TA (Target Address) is a no-op in cross-assemblers
  // Used by SCMASM editor to set assembly target on Apple II
  // In cross-assembly, has no effect
}

void HandleDo(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Condition expression
  (void)context;        // State management

  // TODO: Implement conditional assembly
  // .DO/.FIN require special handling in ParseLine (not registry dispatch)
  // For now, stub implementation
  ThrowFormattedError(".DO conditional assembly not yet implemented", context);
}

void HandleFin(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // Unused
  (void)context;        // State management

  // TODO: Implement conditional assembly
  // For now, stub implementation
  ThrowFormattedError(".FIN conditional assembly not yet implemented", context);
}

void HandleAc(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // Label handled separately
  (void)operand;        // String with optional prefix
  (void)context;        // State management

  // TODO: Implement .AC directive (ASCII string with optional numeric prefix)
  // For now, stub implementation
  ThrowFormattedError(".AC directive not yet implemented", context);
}

}  // namespace xasm::scmasm
