/**
 * @file merlin_syntax.cpp
 * @brief Merlin assembly syntax parser implementation
 *
 * Phases 1-3: Foundation, Local Labels, DUM Blocks
 */

#include "xasm++/syntax/merlin_syntax.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "xasm++/common/expression_parser.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/merlin_directive_handlers.h"
#include "xasm++/syntax/label_policy.h"
#include "xasm++/util/string_utils.h"

namespace xasm {

using xasm::util::ToUpper;
using xasm::util::Trim;
using namespace xasm::directives;

namespace {

// Radix values for number parsing
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_DECIMAL = 10;
constexpr int RADIX_HEXADECIMAL = 16;

// Parse a numeric literal (hex with '$' prefix, binary with '%' prefix,
// or plain decimal digits).  Returns true if the string was a numeric literal
// and sets *out_value; returns false if the string looks like a symbol name.
// Throws std::runtime_error on malformed input (e.g. "$" with no hex digits).
bool ParseNumericLiteral(const std::string& op, uint32_t& out_value) {
  if (op.empty()) {
    return false;
  }
  if (op[0] == '$') {
    out_value = static_cast<uint32_t>(std::stoul(op.substr(1), nullptr, RADIX_HEXADECIMAL));
    return true;
  }
  if (op[0] == '%') {
    out_value = static_cast<uint32_t>(std::stoul(op.substr(1), nullptr, RADIX_BINARY));
    return true;
  }
  if (std::isdigit(static_cast<unsigned char>(op[0]))) {
    out_value = static_cast<uint32_t>(std::stoul(op, nullptr, RADIX_DECIMAL));
    return true;
  }
  return false;
}

// Platform-aware temp directory helper
std::string GetTempDir() {
#ifdef _WIN32
  const char* temp = std::getenv("TEMP");
  if (!temp)
    temp = std::getenv("TMP");
  if (!temp)
    temp = "C:\\Windows\\Temp";
  std::string temp_str(temp);
  // Normalize to forward slashes for consistency
  for (char& c : temp_str) {
    if (c == '\\')
      c = '/';
  }
  return temp_str;
#else
  return "/tmp";
#endif
}

}  // anonymous namespace

// ============================================================================
// Constructor
// ============================================================================

MerlinSyntaxParser::MerlinSyntaxParser() {
  InitializeDirectiveRegistry();
}

void MerlinSyntaxParser::SetCpu(Cpu6502* cpu) {
  cpu_ = cpu;
}

// ============================================================================
// Directive Registry
// ============================================================================

void MerlinSyntaxParser::InitializeDirectiveRegistry() {
  // Register directive handlers from merlin namespace
  directive_registry_[ORG] = merlin::HandleOrg;
  directive_registry_[EQU] = merlin::HandleEqu;
  directive_registry_[DB] = merlin::HandleDb;
  directive_registry_[DFB] = merlin::HandleDb;  // Alias
  directive_registry_[DW] = merlin::HandleDw;
  directive_registry_[HEX] = merlin::HandleHex;
  directive_registry_[DS] = merlin::HandleDs;
  directive_registry_[DUM] = merlin::HandleDum;
  directive_registry_[DEND] = merlin::HandleDend;
  directive_registry_[PUT] = merlin::HandlePut;
  directive_registry_[DO] = merlin::HandleDo;
  directive_registry_[ELSE] = merlin::HandleElse;
  directive_registry_[FIN] = merlin::HandleFin;
  directive_registry_[LST] = merlin::HandleLst;
  directive_registry_[LSTDO] = merlin::HandleLstdo;
  directive_registry_[TR] = merlin::HandleTr;
  directive_registry_[ASC] = merlin::HandleAsc;
  directive_registry_[DCI] = merlin::HandleDci;
  directive_registry_[INV] = merlin::HandleInv;
  directive_registry_[FLS] = merlin::HandleFls;
  directive_registry_[DA] = merlin::HandleDa;
  directive_registry_[PMC] = merlin::HandlePmc;
  directive_registry_[EOM] = merlin::HandleEom;
  directive_registry_[MAC] = merlin::HandleMac;
  directive_registry_[USR] = merlin::HandleUsr;
  directive_registry_[END] = merlin::HandleEnd;
  directive_registry_[SAV] = merlin::HandleSav;
  directive_registry_[XC] = merlin::HandleXc;
  directive_registry_[MX] = merlin::HandleMx;
  directive_registry_[REV] = merlin::HandleRev;
  directive_registry_[LUP] = merlin::HandleLup;
}

bool MerlinSyntaxParser::DispatchDirective(const std::string& directive, const std::string& label,
                                           const std::string& operand, DirectiveContext& context) {
  auto it = directive_registry_.find(directive);
  if (it != directive_registry_.end()) {
    // Found directive - invoke handler
    context.label = label;
    context.operand = operand;
    context.mnemonic = directive;
    it->second(context);
    return true;
  }
  return false;  // Unknown directive
}

// ============================================================================
// Helper Functions
// ============================================================================

// Strip comments: * in column 1 or ; anywhere
std::string MerlinSyntaxParser::StripComments(const std::string& line) {
  // Empty line
  if (line.empty()) {
    return "";
  }

  // * in column 1 is a full-line comment
  if (line[0] == '*') {
    return "";
  }

  // ; marks start of inline comment ONLY when preceded by whitespace.
  // A ';' immediately after a non-space character is a Merlin macro argument
  // separator (e.g. "stlx $E1;$9D00" passes two args to the stlx macro).
  for (size_t i = 0; i < line.size(); ++i) {
    if (line[i] == ';') {
      if (i == 0 || std::isspace(static_cast<unsigned char>(line[i - 1]))) {
        return line.substr(0, i);
      }
      // Non-whitespace before ';' — argument separator, keep scanning
    }
  }

  return line;
}

// Format error message with source location
std::string MerlinSyntaxParser::FormatError(const std::string& message) const {
  std::ostringstream oss;
  oss << current_file_ << ":" << current_line_ << ": error: " << message;
  return oss.str();
}

/// Strip Merlin-style inline comment from an instruction operand.
/// In Merlin, any whitespace-separated trailing text is a comment (no ';'
/// needed). Quoted string char literals (e.g. "#\"A\"") are not stripped.
std::string MerlinSyntaxParser::StripMerlinInlineComment(const std::string& operands) {
  // Skip leading '#' or '<' prefix operators
  size_t offset = 0;
  if (!operands.empty() && (operands[0] == '#' || operands[0] == '<')) {
    offset = 1;
  }
  // If expression itself starts with a quote, leave alone (char literal)
  if (offset < operands.size() && (operands[offset] == '"' || operands[offset] == '\'')) {
    return operands;
  }
  for (size_t i = offset; i < operands.size(); ++i) {
    if (operands[i] == ' ' || operands[i] == '\t') {
      return operands.substr(0, offset) + Trim(operands.substr(offset, i - offset));
    }
  }
  return operands;
}

// ============================================================================
// Number Parsing (DEPRECATED - delegated to ExpressionParser)
// ============================================================================

// Parse number in various formats: $hex, %binary, decimal
// NOTE: Kept for HandleOrg compatibility - will refactor in future phase
uint32_t MerlinSyntaxParser::ParseNumber(const std::string& str) {
  if (str.empty()) {
    return 0;
  }

  // Strip addressing mode suffix (,X ,Y ,S) if present in hex numbers
  std::string clean_str = str;
  if (str[0] == '$') {
    size_t comma_pos = str.find(',');
    if (comma_pos != std::string::npos) {
      clean_str = str.substr(0, comma_pos);
    }
  }

  // Delegate to ExpressionParser for actual parsing
  // Use Merlin features so ]var identifiers are recognised (ADR-005 V8).
  ConcreteSymbolTable empty_symbols;
  ExpressionParser parser(&empty_symbols, nullptr, ParserFeatures::ForMerlin());
  try {
    auto expr = parser.Parse(clean_str);
    return static_cast<uint32_t>(expr->Evaluate(empty_symbols));
  } catch (const std::runtime_error& e) {
    // Re-throw with Merlin formatting
    throw std::runtime_error(FormatError(e.what()));
  }
}

// ============================================================================
// TryParse helpers for ParseExpression
// ============================================================================

std::shared_ptr<Expression> MerlinSyntaxParser::TryParseCharLiteral(const std::string& expr,
                                                                    ConcreteSymbolTable& symbols) {
  if (expr.empty() || (expr[0] != '"' && expr[0] != '\'')) {
    return nullptr;
  }
  char quote = expr[0];
  if (expr.length() == 1) {
    return std::make_shared<LiteralExpr>(0);  // lone quote — treat as 0
  }
  size_t close = expr.find(quote, 1);
  if (close == std::string::npos) {
    return std::make_shared<LiteralExpr>(0);  // unclosed quote — Merlin compat
  }
  std::string chars = expr.substr(1, close - 1);
  int64_t char_val = chars.empty() ? 0 : (static_cast<uint8_t>(chars[0]) | 0x80);
  std::string rest = Trim(expr.substr(close + 1));
  if (rest.empty()) {
    return std::make_shared<LiteralExpr>(char_val);
  }
  return CompoundCharExpr(char_val, rest, symbols);
}

/// Build a compound expression from a char literal value and trailing "+N", "-X" etc.
std::shared_ptr<Expression> MerlinSyntaxParser::CompoundCharExpr(int64_t char_val,
                                                                 const std::string& rest,
                                                                 ConcreteSymbolTable& symbols) {
  if (rest.empty() || (rest[0] != '+' && rest[0] != '-' && rest[0] != '*' && rest[0] != '/')) {
    return std::make_shared<LiteralExpr>(char_val);
  }
  std::string rhs = Trim(rest.substr(1));
  if (rhs.empty()) {
    return std::make_shared<LiteralExpr>(char_val);
  }
  auto left_expr = std::make_shared<LiteralExpr>(char_val);
  auto right_expr = ParseExpression(rhs, symbols);
  return MakeBinaryExpr(rest[0], left_expr, right_expr);
}

/// Build a BinaryOpExpr from a char operator; returns left if op unrecognised.
std::shared_ptr<Expression> MerlinSyntaxParser::MakeBinaryExpr(
    char op, std::shared_ptr<Expression> left_expr, std::shared_ptr<Expression> right_expr) {
  switch (op) {
    case '+':
      return std::make_shared<BinaryOpExpr>(BinaryOp::Add, left_expr, right_expr);
    case '-':
      return std::make_shared<BinaryOpExpr>(BinaryOp::Subtract, left_expr, right_expr);
    case '*':
      return std::make_shared<BinaryOpExpr>(BinaryOp::Multiply, left_expr, right_expr);
    case '/':
      return std::make_shared<BinaryOpExpr>(BinaryOp::Divide, left_expr, right_expr);
    default:
      return left_expr;
  }
}

std::shared_ptr<Expression> MerlinSyntaxParser::TryParseLowByteOperator(
    const std::string& expr, ConcreteSymbolTable& symbols) {
  if (expr.empty() || (expr[0] != '<' && expr[0] != '#')) {
    return nullptr;
  }
  if (expr.length() < 2) {
    throw std::runtime_error(FormatError("Low byte operator (</#) requires an operand"));
  }
  std::string operand = Trim(expr.substr(1));
  if (operand.empty()) {
    throw std::runtime_error(FormatError("Low byte operator (</#) has empty operand"));
  }
  // Recursively parse the operand (might be expression like SHIFT0-$80)
  auto operand_expr = ParseExpression(operand, symbols);
  int64_t value = operand_expr->Evaluate(symbols);
  return std::make_shared<LiteralExpr>(value & 0xFF);  // Low byte
}

std::shared_ptr<Expression> MerlinSyntaxParser::TryParseHighByteOperator(
    const std::string& expr, ConcreteSymbolTable& symbols) {
  if (expr.empty() || expr[0] != '>') {
    return nullptr;
  }
  if (expr.length() < 2) {
    throw std::runtime_error(FormatError("High byte operator (>) requires an operand"));
  }
  std::string operand = Trim(expr.substr(1));
  if (operand.empty()) {
    throw std::runtime_error(FormatError("High byte operator (>) has empty operand"));
  }
  // Recursively parse the operand (might be expression like SHIFT0-$80)
  auto operand_expr = ParseExpression(operand, symbols);
  int64_t value = operand_expr->Evaluate(symbols);
  return std::make_shared<LiteralExpr>((value >> 8) & 0xFF);  // High byte
}

// ============================================================================

// Parse expression - delegates to ExpressionParser for standard operations
// while preserving Merlin-specific features: character literals, low/high byte
std::shared_ptr<Expression> MerlinSyntaxParser::ParseExpression(const std::string& str,
                                                                ConcreteSymbolTable& symbols) {
  std::string expr = StripMerlinExprComment(Trim(str));

  // Merlin-specific prefix operators: character literals, low/high byte
  if (auto result = TryParseCharLiteral(expr, symbols)) {
    return result;
  }
  if (auto result = TryParseLowByteOperator(expr, symbols)) {
    return result;
  }
  if (auto result = TryParseHighByteOperator(expr, symbols)) {
    return result;
  }

  if (expr.empty()) {
    return std::make_shared<LiteralExpr>(0);
  }
  if (expr == "$") {
    throw std::runtime_error(FormatError("Invalid hex number: '$' (no digits after $)"));
  }

  // Trailing operator: strip and re-parse (e.g. "X+" → X+0 = X)
  char last_char = expr[expr.length() - 1];
  if (last_char == '+' || last_char == '-' || last_char == '*') {
    std::string clean_expr = Trim(expr.substr(0, expr.length() - 1));
    if (!clean_expr.empty()) {
      return ParseExpression(clean_expr, symbols);
    }
  }

  expr = SubstitutePCInDSOperand(expr);
  ExpressionParser parser(&symbols, nullptr, ParserFeatures::ForMerlin());
  try {
    return parser.Parse(expr);
  } catch (const std::runtime_error& e) {
    throw std::runtime_error(FormatError(e.what()));
  }
}

/// Strip trailing whitespace-delimited inline comment from a Merlin expression.
/// Leaves quoted strings (char literals) untouched.
// static
std::string MerlinSyntaxParser::StripMerlinExprComment(const std::string& expr) {
  if (expr.empty() || expr[0] == '"' || expr[0] == '\'') {
    return expr;
  }
  size_t first_space = std::string::npos;
  for (size_t i = 0; i < expr.size(); ++i) {
    if (expr[i] == ' ' || expr[i] == '\t') {
      first_space = i;
      break;
    }
  }
  return (first_space != std::string::npos) ? Trim(expr.substr(0, first_space)) : expr;
}

std::string MerlinSyntaxParser::ScopeLocalLabel(const std::string& label) const {
  if (!label.empty() && label[0] == ':' && !current_scope_.global_label.empty()) {
    return current_scope_.global_label + label;
  }
  return label;
}

// ============================================================================
// SubstituteMerlinVars Sub-Helpers
// ============================================================================

/// Returns the position in 'line' where ]var substitution should begin.
/// Skips the label column (and '=' sign) for assignment lines starting with ']'.
static size_t ComputeSubstStartPos(const std::string& line) {
  if (line.empty() || line[0] != ']') {
    return 0;
  }
  // Find end of leading ]label token
  size_t label_end = 1;
  while (label_end < line.size() &&
         (std::isalnum(static_cast<unsigned char>(line[label_end])) || line[label_end] == '_')) {
    ++label_end;
  }
  // Skip optional whitespace then '=' if present (assignment form)
  size_t pos = label_end;
  while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
    ++pos;
  }
  if (pos < line.size() && line[pos] == '=') {
    return pos + 1;  // start after '='
  }
  return label_end;  // directive form — skip only the label
}

/// Copy a quoted string from line[i] into result; return index after close quote.
static size_t CopyQuotedString(const std::string& line, size_t i, std::string& result) {
  char quote = line[i];
  result += line[i++];
  while (i < line.size() && line[i] != quote) {
    result += line[i++];
  }
  if (i < line.size()) {
    result += line[i++];  // closing quote
  }
  return i;
}

/// Substitute a ]var token starting at line[i] into result; return updated i.
static size_t SubstituteVarToken(const std::string& line, size_t i,
                                 const ConcreteSymbolTable& symbols, std::string& result) {
  size_t tok_start = i++;
  while (i < line.size() && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
    ++i;
  }
  std::string var_name = line.substr(tok_start, i - tok_start);
  int64_t sym_val = 0;
  if (symbols.Lookup(var_name, sym_val)) {
    result += std::to_string(sym_val);
  } else {
    result += var_name;
  }
  return i;
}

std::string MerlinSyntaxParser::SubstituteMerlinVars(const std::string& line,
                                                     const ConcreteSymbolTable& symbols) {
  // Fast path: no ] in line
  if (line.find(']') == std::string::npos) {
    return line;
  }

  // Compute where substitution should start.
  // If line starts with ']', skip the label/LHS to avoid renaming it.
  size_t start_substitute = ComputeSubstStartPos(line);

  std::string result = line.substr(0, start_substitute);
  result.reserve(line.size());

  for (size_t i = start_substitute; i < line.size();) {
    if (line[i] == '"' || line[i] == '\'') {
      i = CopyQuotedString(line, i, result);
      continue;
    }
    if (line[i] == ']') {
      i = SubstituteVarToken(line, i, symbols, result);
      continue;
    }
    result += line[i++];
  }

  return result;
}

std::string MerlinSyntaxParser::ScopeLocalLabelsInOperand(const std::string& operand) const {
  if (current_scope_.global_label.empty() || operand.empty()) {
    return operand;
  }

  std::string result;
  result.reserve(operand.size() + (current_scope_.global_label.size() * 2));

  for (size_t i = 0; i < operand.size();) {
    if (StartsLocalLabelRef(operand, i)) {
      result += current_scope_.global_label;
      result += ':';
      ++i;  // skip the ':'
      while (i < operand.size() &&
             (std::isalnum(static_cast<unsigned char>(operand[i])) || operand[i] == '_')) {
        result += operand[i++];
      }
    } else {
      result += operand[i++];
    }
  }

  return result;
}

/// Return true if operand[i] begins a ':local' label reference.
// static
bool MerlinSyntaxParser::StartsLocalLabelRef(const std::string& operand, size_t i) {
  if (operand[i] != ':') {
    return false;
  }
  // Must be at start of a word (position 0 or after a non-ident char)
  bool at_word_start = (i == 0) || (!std::isalnum(static_cast<unsigned char>(operand[i - 1])) &&
                                    operand[i - 1] != '_');
  // Must be followed by an identifier char
  bool valid_next =
      (i + 1 < operand.size() &&
       (std::isalnum(static_cast<unsigned char>(operand[i + 1])) || operand[i + 1] == '_'));
  return at_word_start && valid_next;
}

// Expand Merlin character literals ("X" or 'X') in an instruction operand
// to their Apple II high-bit hex equivalents ($XX where XX = ASCII | 0x80).
// This is the Merlin-specific equivalent of SCMASM's ExpandCharLiteralsInExpr:
// it pre-processes the operand string at Parse() time so that the shared
// assembler.cpp::ParseExpression never has to handle Merlin char literals
// (ADR-005 compliance: no Merlin-specific behaviour in shared core).
//
// Only single-character literals are handled here ("X" or 'X' where X is
// one printable character).  Compound forms like "A"+1 work correctly
// because only the "A" token is replaced; the +1 is left for the generic
// expression evaluator.
std::string MerlinSyntaxParser::ExpandMerlinCharLiterals(const std::string& operand) {
  // Fast path: no quote character → nothing to expand
  if (operand.find('"') == std::string::npos && operand.find('\'') == std::string::npos) {
    return operand;
  }

  std::string result;
  result.reserve(operand.size() + 16);

  size_t i = 0;
  while (i < operand.size()) {
    char c = operand[i];
    if (c == '"' || c == '\'') {
      // Potential char literal: quote followed by exactly one printable char
      // followed by the same closing quote.
      if (i + 2 < operand.size() && std::isprint(static_cast<unsigned char>(operand[i + 1])) &&
          operand[i + 2] == c) {
        // Replace "X" or 'X' with $XX (Apple II high-bit value)
        uint8_t val = static_cast<uint8_t>(operand[i + 1]) | 0x80;
        // Format as $XX
        // Format as $XX using stream formatting instead of snprintf
        std::ostringstream oss;
        oss << '$' << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<unsigned>(val);
        result += oss.str();
        i += 3;  // skip quote, char, closing quote
        continue;
      }
    }
    result += c;
    ++i;
  }
  return result;
}

std::string MerlinSyntaxParser::ExpandVarLabelsInOperand(const std::string& operand) const {
  if (operand.find(']') == std::string::npos) {
    return operand;
  }

  std::string result;
  result.reserve(operand.size() + 16);

  for (size_t i = 0; i < operand.size();) {
    if (operand[i] == ']') {
      i = AppendExpandedVarLabel(operand, i, result);
    } else {
      result += operand[i++];
    }
  }

  return result;
}

/// Expand a single ]var token starting at operand[i] into result.
/// Returns the updated index after the token.
size_t MerlinSyntaxParser::AppendExpandedVarLabel(const std::string& operand, size_t i,
                                                  std::string& result) const {
  size_t tok_start = i++;
  while (i < operand.size() &&
         (std::isalnum(static_cast<unsigned char>(operand[i])) || operand[i] == '_')) {
    ++i;
  }
  std::string var_name = operand.substr(tok_start, i - tok_start);
  bool followed_by_colon = (i < operand.size() && operand[i] == ':');

  auto it = var_label_seq_.find(var_name);
  if (!followed_by_colon && it != var_label_seq_.end() && it->second > 0) {
    result += var_name + "_" + std::to_string(it->second);
  } else if (!followed_by_colon && !var_name.empty() &&
             (it == var_label_seq_.end() || it->second != 0)) {
    result += var_name + "_1";
  } else {
    result += var_name;
  }
  return i;
}

// ============================================================================
// Label Parsing
// ============================================================================

std::string MerlinSyntaxParser::ParseLabel(const std::string& line, size_t& pos,
                                           Section& /*section*/, ConcreteSymbolTable& /*symbols*/) {
  // Merlin labels can be:
  // 1. Global label: START (starts in column 1-9, no special prefix)
  // 2. :Local label: :LOOP (prefixed with :, scoped to last global)
  // 3. ]Variable label: ]TEMP (prefixed with ], used in DUM blocks)

  // Check if line starts with label (non-whitespace in first columns)
  if (pos == 0 && !line.empty() && !std::isspace(line[0])) {
    // Find end of label (space or end of line)
    size_t label_end = pos;
    while (label_end < line.length() && !std::isspace(line[label_end])) {
      label_end++;
    }

    std::string label = line.substr(pos, label_end - pos);
    pos = label_end;

    // Just return the label - let caller decide what to do with it
    // (EQU defines symbols without atoms, other labels create atoms)
    return label;
  }

  return "";
}

// ============================================================================
// Directive Handlers
// ============================================================================

void MerlinSyntaxParser::HandleEqu(const DirectiveContext& ctx, ConcreteSymbolTable& symbols) {
  const std::string& label = ctx.label;
  const std::string& operand = ctx.operand;
  // EQU directive - define symbolic constant (no code generated)
  auto expr = ParseExpression(operand, symbols);
  // Eagerly evaluate to a literal when possible to prevent circular
  // self-references (e.g., ]var = ]var+1 in LUP loops).  Deferred evaluation
  // is still used when the expression references undefined forward symbols.
  try {
    int64_t val = expr->Evaluate(symbols);
    symbols.Define(label, SymbolType::Label, std::make_shared<LiteralExpr>(val));
  } catch (const std::exception&) {
    // Cannot evaluate yet (forward reference) — store deferred expression
    symbols.Define(label, SymbolType::Label, expr);
  }

  // Mark ]var EQU-style assignments in var_label_seq_ with sentinel 0 so
  // ExpandVarLabelsInOperand uses the plain name (e.g. ]XH) instead of the
  // uniqued code-label form (]XH_1).  Only insert if not already tracked as
  // a code label (seq > 0).
  if (!label.empty() && label[0] == ']' && !var_label_seq_.contains(label)) {
    var_label_seq_[label] = 0;
  }
}

// ============================================================================
// HandleDS Sub-Helpers
// ============================================================================

/// Returns true if '*' at position pos in str is a program counter (not mult).
static bool IsPC(const std::string& str, size_t pos) {
  bool before =
      (pos > 0 && (std::isalnum(static_cast<unsigned char>(str[pos - 1])) || str[pos - 1] == ')'));
  bool after =
      (pos + 1 < str.size() && (std::isalnum(static_cast<unsigned char>(str[pos + 1])) ||
                                str[pos + 1] == '(' || str[pos + 1] == '$' || str[pos + 1] == '%'));
  return !(before && after);
}

std::string MerlinSyntaxParser::SubstitutePCInDSOperand(const std::string& op) const {
  if (op.find('*') == std::string::npos) {
    return op;
  }
  std::ostringstream hex_stream;
  hex_stream << "$" << std::hex << (in_dum_block_ ? dum_address_ : current_address_);
  std::string pc_hex = hex_stream.str();
  std::string result = op;
  size_t pos = 0;
  while ((pos = result.find('*', pos)) != std::string::npos) {
    if (IsPC(result, pos)) {
      result.replace(pos, 1, pc_hex);
      pos += pc_hex.length();
    } else {
      ++pos;
    }
  }
  return result;
}

uint32_t MerlinSyntaxParser::ResolveDSCount(const std::string& op, ConcreteSymbolTable& symbols) {
  if (op.empty()) {
    return 0;
  }
  auto expr = ParseExpression(op, symbols);
  try {
    int64_t value = expr->Evaluate(symbols);
    if (value < 0) {
      throw std::runtime_error(
          FormatError("DS: Negative count not allowed: " + std::to_string(value)));
    }
    return static_cast<uint32_t>(value);
  } catch (const std::runtime_error& e) {
    // Re-throw with location if not already formatted
    std::string msg = e.what();
    size_t first_colon = msg.find(':');
    if (first_colon != std::string::npos && first_colon < msg.length() - 1) {
      size_t second_colon = msg.find(':', first_colon + 1);
      if (second_colon != std::string::npos && second_colon > first_colon + 1) {
        for (size_t i = first_colon + 1; i < second_colon; ++i) {
          if (std::isdigit(static_cast<unsigned char>(msg[i]))) {
            throw;  // already has location
          }
        }
      }
    }
    throw std::runtime_error(FormatError(msg));
  }
}

void MerlinSyntaxParser::HandleDS(const std::string& operand, Section& section,
                                  ConcreteSymbolTable& symbols) {
  // DS directive - define space (reserve bytes)
  // Supports: DS 100        (literal)
  //           DS COUNT      (symbol)
  //           DS *+10       (program counter arithmetic)

  std::string op = SubstitutePCInDSOperand(Trim(operand));
  uint32_t count = ResolveDSCount(op, symbols);

  // DUM blocks: advance address without emitting bytes
  if (!in_dum_block_) {
    // If the original operand contains '*' (PC-relative), store the raw
    // expression so the assembler can re-evaluate it each pass.
    std::string raw = Trim(operand);
    bool has_star = false;
    for (size_t i = 0; i < raw.size(); ++i) {
      if (raw[i] == '*' && IsPC(raw, i)) {
        has_star = true;
        break;
      }
    }
    if (has_star) {
      section.atoms.push_back(std::make_shared<SpaceAtom>(count, raw));
    } else {
      section.atoms.push_back(std::make_shared<SpaceAtom>(count));
    }
    current_address_ += count;
  } else {
    dum_address_ += count;
  }
}

void MerlinSyntaxParser::HandleDum(const std::string& operand, ConcreteSymbolTable& symbols) {
  // DUM (Dummy section) - start variable definition block
  in_dum_block_ = true;

  std::string op = Trim(operand);

  // Check if operand is empty
  if (op.empty()) {
    throw std::runtime_error(FormatError("DUM directive requires an address operand"));
  }

  // Parse number (decimal, hex, or binary) or symbol reference
  uint32_t parsed_addr = 0;
  if (ParseNumericLiteral(op, parsed_addr)) {
    dum_address_ = parsed_addr;
  } else {
    // Symbol - look it up
    int64_t value = 0;
    if (symbols.Lookup(op, value)) {
      dum_address_ = static_cast<uint32_t>(value);
    } else {
      // Symbol not found - ERROR instead of silently using 0
      throw std::runtime_error(FormatError("DUM directive: symbol '" + op + "' not defined"));
    }
  }
}

void MerlinSyntaxParser::HandleDend() {
  // DEND - end dummy section
  in_dum_block_ = false;
}

void MerlinSyntaxParser::HandlePut(const std::string& operand, Section& section,
                                   ConcreteSymbolTable& symbols) {
  // PUT filename - include another source file
  std::string filename = Trim(operand);

  // Auto-append .S extension if no extension present
  if (filename.find('.') == std::string::npos) {
    filename += ".S";
  }

  // Check for circular includes
  for (const auto& included_file : include_stack_) {
    if (included_file == filename) {
      throw std::runtime_error(FormatError("Circular include detected: " + filename));
    }
  }

  // Add to include stack
  include_stack_.push_back(filename);

  // Try to open the file - first as given, then with temp directory prefix
  std::ifstream file(filename);
  std::string actual_filename = filename;

  if (!file.is_open() && filename[0] != '/') {
    // Try with temp directory prefix for relative paths (platform-aware)
    actual_filename = GetTempDir() + "/" + filename;
    file.open(actual_filename);
  }

  if (!file.is_open()) {
    include_stack_.pop_back();  // Remove from stack on error
    throw std::runtime_error(FormatError("Cannot open file: " + Trim(operand)));
  }

  // Read and parse each line
  std::string line;
  while (std::getline(file, line)) {
    ParseLine(line, section, symbols);
  }

  // Pop from include stack
  include_stack_.pop_back();
}

void MerlinSyntaxParser::HandleDo(const std::string& operand, ConcreteSymbolTable& symbols) {
  // DO directive - begin conditional assembly block
  // Evaluate operand expression: non-zero = true, zero = false

  std::string op = Trim(operand);
  if (op.empty()) {
    throw std::runtime_error("DO directive requires an operand expression");
  }

  // Evaluate the expression to determine condition
  uint32_t value = 0;
  if (!ParseNumericLiteral(op, value)) {
    // Symbol - look it up (undefined symbol evaluates to 0 = false)
    int64_t sym_value = 0;
    if (symbols.Lookup(op, sym_value)) {
      value = static_cast<uint32_t>(sym_value);
    }
    // else: value stays 0
  }

  // Begin conditional block (non-zero = true)
  conditional_.BeginIf(value != 0);
}

void MerlinSyntaxParser::HandleElse() {
  // ELSE directive - toggle conditional assembly state
  try {
    conditional_.BeginElse();
  } catch (const std::runtime_error& e) {
    // Re-throw with location information and Merlin-specific terminology
    std::string msg = e.what();
    // Replace "IF" with "DO" for Merlin syntax
    size_t pos = msg.find("IF");
    if (pos != std::string::npos) {
      msg.replace(pos, 2, "DO");
    }
    throw std::runtime_error(FormatError(msg));
  }
}

void MerlinSyntaxParser::HandleFin() {
  // FIN directive - end conditional assembly block
  // Merlin ignores extra FIN directives (no matching DO) — treat as no-op.
  if (conditional_.IsBalanced()) {
    return;
  }
  try {
    conditional_.EndIf();
  } catch (const std::runtime_error& e) {
    // Re-throw with location information and Merlin-specific terminology
    std::string msg = e.what();
    // Replace "ENDIF" with "FIN" and "IF" with "DO" for Merlin syntax
    size_t pos = msg.find("ENDIF");
    if (pos != std::string::npos) {
      msg.replace(pos, 5, "FIN");
    }
    pos = msg.find("IF");
    if (pos != std::string::npos) {
      msg.replace(pos, 2, "DO");
    }
    throw std::runtime_error(FormatError(msg));
  }
}

void MerlinSyntaxParser::HandleEnd() {
  // END - mark end of source (stop processing further lines)
  end_directive_seen_ = true;
}

// ============================================================================
// Macro Directives
// ============================================================================

// Macro-related member functions - NOT extracted to merlin_directives.cpp
// because they are tightly coupled to parser state (macros_, current_macro_,
// in_macro_definition_) and require complex interaction with macro expansion.

void MerlinSyntaxParser::HandlePMC(const std::string& operand) {
  // PMC - Start macro definition
  if (in_macro_definition_) {
    throw std::runtime_error(FormatError("Nested macro definitions not allowed"));
  }
  in_macro_definition_ = true;
  current_macro_.name = ToUpper(Trim(operand));  // Normalize macro name
  current_macro_.body.clear();
  current_macro_.param_count = 0;
}

void MerlinSyntaxParser::HandleEOM() {
  // EOM - End macro definition
  if (!in_macro_definition_) {
    throw std::runtime_error(FormatError("EOM without matching PMC"));
  }
  macros_[current_macro_.name] = current_macro_;
  in_macro_definition_ = false;
}

void MerlinSyntaxParser::HandleMacroEnd() {
  // <<< - End macro definition (Merlin style)
  if (!in_macro_definition_) {
    throw std::runtime_error(FormatError("<<< without matching PMC"));
  }
  macros_[current_macro_.name] = current_macro_;
  in_macro_definition_ = false;
}

/// Parse macro invocation operand string into a list of parameter strings.
/// Semicolon-separated (MAC style) or comma-separated.
static std::vector<std::string> ParseMacroParams(const std::string& operand) {
  if (operand.empty()) {
    return {};
  }
  char separator = (operand.find(';') != std::string::npos) ? ';' : ',';
  std::vector<std::string> params;
  std::string token;
  bool in_string = false;
  char string_delim = '\0';
  for (char c : operand + separator) {
    if (!in_string && (c == '"' || c == '\'')) {
      in_string = true;
      string_delim = c;
      token += c;
    } else if (in_string && c == string_delim) {
      in_string = false;
      token += c;
    } else if (!in_string && c == separator) {
      std::string t = Trim(token);
      if (!t.empty()) {
        params.push_back(t);
      }
      token.clear();
    } else {
      token += c;
    }
  }
  return params;
}

void MerlinSyntaxParser::ExpandMacro(const DirectiveContext& ctx, Section& section,
                                     ConcreteSymbolTable& symbols) {
  const std::string& macro_name = ctx.mnemonic;
  const std::string& operand = ctx.operand;

  if (macro_expansion_depth_ >= 100) {
    throw std::runtime_error(
        FormatError("Macro expansion depth limit exceeded (possible recursion)"));
  }

  std::string upper_name = ToUpper(macro_name);
  auto it = macros_.find(upper_name);
  if (it == macros_.end()) {
    throw std::runtime_error(FormatError("Undefined macro: " + macro_name));
  }

  const MacroDefinition& macro = it->second;
  std::vector<std::string> params = ParseMacroParams(operand);

  macro_expansion_depth_++;
  for (const auto& line : macro.body) {
    ParseLine(SubstituteParameters(line, params), section, symbols);
  }
  macro_expansion_depth_--;
}

std::string MerlinSyntaxParser::SubstituteParameters(const std::string& line,
                                                     const std::vector<std::string>& params) {
  // Replace ]1, ]2, etc. with actual parameters
  std::string result;

  for (size_t i = 0; i < line.length(); ++i) {
    if (line[i] == ']' && i + 1 < line.length() && std::isdigit(line[i + 1])) {
      int param_num = line[i + 1] - '0';
      if (param_num > 0 && static_cast<size_t>(param_num) <= params.size()) {
        // Valid parameter reference - substitute
        result += params[param_num - 1];
        i++;  // Skip the digit
      } else {
        // Invalid parameter number - leave as is
        result += line[i];
      }
    } else {
      result += line[i];
    }
  }

  return result;
}

void MerlinSyntaxParser::HandleXc(const std::string& operand) {
  // XC [ON|OFF] - Toggle 65C02/65816 CPU instruction set
  //
  // Merlin XC semantics:
  //   XC (first)  → 65C02 mode
  //   XC (second) → 65816 mode (when already in 65C02)
  //   XC OFF      → back to base 6502

  if (!cpu_) {
    // No CPU set - silently ignore (for tests that don't need CPU)
    return;
  }

  std::string op = ToUpper(Trim(operand));

  if (op.empty() || op == directives::ON) {
    if (cpu_->GetCpuMode() == CpuMode::Cpu65C02) {
      // Second XC: upgrade to 65816
      cpu_->SetCpuMode(CpuMode::Cpu65816);
    } else {
      // First XC (or XC from 65816 back to 65C02 not typical, treat as 65C02)
      cpu_->SetCpuMode(CpuMode::Cpu65C02);
    }
  } else if (op == directives::OFF) {
    // Disable extended mode (back to 6502)
    cpu_->SetCpuMode(CpuMode::Cpu6502);
  } else {
    throw std::runtime_error(FormatError("XC: invalid operand (expected ON or OFF)"));
  }
}

void MerlinSyntaxParser::HandleMx(const std::string& operand) {
  // MX mode - Set 65816 accumulator and index register widths
  // This is a directive only - tracks state but doesn't change CPU encoding

  std::string op = Trim(operand);
  if (op.empty()) {
    throw std::runtime_error(FormatError("MX directive requires an operand"));
  }

  int mode = -1;

  // Check for binary format %00-%11 (bit pattern: M-bit, X-bit)
  if (op[0] == '%') {
    std::string binary = op.substr(1);
    if (binary.length() == 2 && (binary[0] == '0' || binary[0] == '1') &&
        (binary[1] == '0' || binary[1] == '1')) {
      mode = ((binary[0] - '0') * 2) + (binary[1] - '0');
    } else {
      throw std::runtime_error(FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
    }
  }
  // Check for decimal format 0-3
  else if (op.length() == 1 && op[0] >= '0' && op[0] <= '3') {
    mode = op[0] - '0';
  } else {
    throw std::runtime_error(FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
  }

  // Mode validated - in full implementation, would affect 65816 encoding
  // For now, just validate and accept
  (void)mode;  // Suppress unused variable warning
}

void MerlinSyntaxParser::HandleLup(const std::string& operand) {
  // LUP count - Loop directive (repeat following code count times)
  // Syntax: LUP count
  //         <lines>
  //         --^

  // Parse repeat count
  std::string count_str = Trim(operand);
  if (count_str.empty()) {
    throw std::runtime_error(FormatError("LUP requires a repeat count"));
  }

  // Try to parse as number
  int count = 0;
  try {
    count = static_cast<int>(ParseNumber(count_str));
  } catch (...) {
    throw std::runtime_error(FormatError("LUP count must be a number: " + count_str));
  }

  if (count < 0) {
    throw std::runtime_error(FormatError("LUP count cannot be negative: " + count_str));
  }

  // Start capturing LUP block
  in_lup_block_ = true;
  lup_count_ = count;
  lup_body_.clear();
  lup_nesting_depth_ = 0;  // Track nesting for nested LUP blocks
}

// ============================================================================
// TryParse helpers for ParseLine
// ============================================================================

bool MerlinSyntaxParser::TryHandleDirectiveLine(const std::string& directive,
                                                const std::string& label,
                                                const std::string& operands, Section& section,
                                                ConcreteSymbolTable& symbols) {
  DirectiveContext ctx;
  ctx.section = &section;
  ctx.symbols = &symbols;
  ctx.current_address = &current_address_;
  ctx.parser_state = this;
  ctx.current_file = current_file_;
  ctx.current_line = current_line_;

  // Scope the label and operands before dispatching so directive handlers
  // define label atoms under the correct scoped name (e.g. "tone:pitch" not
  // just ":pitch"), and operand references like ":loop" are expanded to their
  // scoped names (e.g. "alertstand:loop").
  std::string scoped_label = ScopeLocalLabel(label);
  std::string scoped_operands = ScopeLocalLabelsInOperand(operands);
  if (!DispatchDirective(directive, scoped_label, scoped_operands, ctx)) {
    return false;
  }
  // Directive was handled by registry.
  // Update global label scope from the RAW (un-scoped) label so that
  // subsequent ':local' labels are correctly qualified.  Only non-local
  // labels (those that don't start with ':') update the scope.
  if (!label.empty() && label[0] != ':' &&
      (label[0] != ']' || current_scope_.global_label.empty())) {
    current_scope_.global_label = label;
    current_scope_.local_labels.clear();
  }
  return true;
}

bool MerlinSyntaxParser::TryHandleMacroLine(const std::string& directive, const std::string& label,
                                            const std::string& operands, Section& section,
                                            ConcreteSymbolTable& symbols) {
  std::string upper_directive = ToUpper(directive);
  if (!macros_.contains(upper_directive)) {
    return false;
  }
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
    auto merlin_scope_fn = [this](const std::string& lbl) -> std::string {
      return ScopeLocalLabel(lbl);
    };
    auto merlin_is_local_fn = [this](const std::string& lbl) -> bool {
      return !lbl.empty() &&
             (lbl[0] == ':' || (lbl[0] == ']' && !current_scope_.global_label.empty()));
    };
    auto merlin_on_global_update = [this]() { current_scope_.local_labels.clear(); };
    DefineLabelForDirective(label, label_addr, LabelPolicy::AtPc, !in_dum_block_, symbols, section,
                            current_scope_.local_labels, current_scope_.global_label,
                            merlin_scope_fn, merlin_is_local_fn, merlin_on_global_update);
  }
  // Expand macro
  DirectiveContext ctx;
  ctx.section = &section;
  ctx.symbols = &symbols;
  ctx.current_address = &current_address_;
  ctx.parser_state = this;
  ctx.current_file = current_file_;
  ctx.current_line = current_line_;
  ctx.operand = operands;
  ctx.mnemonic = directive;
  ExpandMacro(ctx, section, symbols);
  return true;
}

void MerlinSyntaxParser::HandleInstructionLine(const std::string& directive,
                                               const std::string& label, std::string operands,
                                               Section& section, ConcreteSymbolTable& symbols) {
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
    // scope_fn handles both local-label scoping and ]variable sequencing.
    // ]variable code labels (e.g. "]rts  rts") are redefined across
    // subroutines.  Give each definition a unique name (]rts_1, ]rts_2, …) so
    // that multi-pass assembly resolves references to the *nearby* definition
    // rather than always using the last global one.  Only label-on-instruction
    // definitions get unique names; EQU-style ]var = VALUE assignments keep
    // their original name because they act as mutable numeric variables.
    auto merlin_scope_fn = [this](const std::string& lbl) -> std::string {
      std::string scoped = ScopeLocalLabel(lbl);
      if (!lbl.empty() && lbl[0] == ']' && lbl.find(':') == std::string::npos) {
        int seq = ++var_label_seq_[lbl];
        scoped = lbl + "_" + std::to_string(seq);
      }
      return scoped;
    };
    auto merlin_is_local_fn = [this](const std::string& lbl) -> bool {
      return !lbl.empty() &&
             (lbl[0] == ':' || (lbl[0] == ']' && !current_scope_.global_label.empty()));
    };
    // True global labels (no ':' or ']' prefix) always update scope.
    // ]variable labels are mutable variables, NOT scope anchors — EXCEPT at
    // the start of the file before any true global label exists.  In that
    // case they must anchor scope so :local labels around them can resolve.
    auto merlin_on_global_update = [this]() { current_scope_.local_labels.clear(); };
    DefineLabelForDirective(label, label_addr, LabelPolicy::AtPc, !in_dum_block_, symbols, section,
                            current_scope_.local_labels, current_scope_.global_label,
                            merlin_scope_fn, merlin_is_local_fn, merlin_on_global_update);
  }
  // Translate any ':word' local-label references to the scoped name, then
  // expand ]variable references to their current unique-instance names, then
  // strip Merlin inline string comments (e.g. "#99 \"stabbed\"" → "#99"),
  // then expand Merlin char literals ("X"/'X') to their Apple II high-bit hex
  // values so the shared ParseExpression never sees Merlin char-literal syntax.
  operands = ScopeLocalLabelsInOperand(operands);
  operands = ExpandVarLabelsInOperand(operands);
  operands = StripMerlinInlineComment(operands);
  operands = ExpandMerlinCharLiterals(operands);
  section.atoms.push_back(std::make_shared<InstructionAtom>(directive, operands));
  current_address_ += 1;  // Placeholder size
}

// ============================================================================
// ParseLine Sub-Handlers (extracted helpers to reduce cognitive complexity)
// ============================================================================

bool MerlinSyntaxParser::HandleMacroCaptureState(const std::string& code_line,
                                                 const std::string& upper_trimmed) {
  if (!in_macro_definition_) {
    return false;
  }
  if (upper_trimmed == directives::EOM) {
    HandleEOM();
    return true;
  }
  if (upper_trimmed == directives::MACRO_END_ALT) {
    HandleMacroEnd();
    return true;
  }
  current_macro_.body.push_back(code_line);
  return true;
}

bool MerlinSyntaxParser::HandleLupCaptureState(const std::string& code_line,
                                               const std::string& upper_trimmed, Section& section,
                                               ConcreteSymbolTable& symbols) {
  if (!in_lup_block_) {
    return false;
  }
  if (upper_trimmed != "--^") {
    // Check if this line starts a nested LUP block
    std::string lup_directive = std::string(directives::LUP) + " ";
    if (upper_trimmed.starts_with(lup_directive) || upper_trimmed == directives::LUP) {
      lup_nesting_depth_++;
    }
    lup_body_.push_back(code_line);
    return true;
  }
  // "--^" terminates LUP
  if (lup_nesting_depth_ > 0) {
    lup_nesting_depth_--;
    lup_body_.push_back(code_line);
    return true;
  }
  // End of outermost LUP — expand body
  std::vector<std::string> body_copy = lup_body_;
  int count = lup_count_;
  in_lup_block_ = false;
  lup_body_.clear();
  lup_count_ = 0;
  lup_nesting_depth_ = 0;
  for (int i = 0; i < count; ++i) {
    for (const auto& lup_line : body_copy) {
      std::string expanded = SubstituteMerlinVars(lup_line, symbols);
      ParseLine(expanded, section, symbols);
    }
  }
  return true;
}

bool MerlinSyntaxParser::HandleConditionalDirective(const std::string& trimmed,
                                                    const std::string& upper_trimmed,
                                                    ConcreteSymbolTable& symbols) {
  std::string do_directive = std::string(directives::DO) + " ";
  if (upper_trimmed.starts_with(do_directive) || upper_trimmed == directives::DO) {
    std::string operand = trimmed.length() > 3 ? Trim(trimmed.substr(3)) : "0";
    HandleDo(operand, symbols);
    return true;
  }
  if (upper_trimmed == directives::ELSE) {
    HandleElse();
    return true;
  }
  if (upper_trimmed == directives::FIN) {
    HandleFin();
    return true;
  }
  return false;
}

bool MerlinSyntaxParser::HandleLabelOnlyLine(const std::string& label, Section& section,
                                             ConcreteSymbolTable& symbols) {
  if (label.empty()) {
    return true;  // empty label on label-only line — nothing to do
  }
  uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
  std::string scoped_label = ScopeLocalLabel(label);
  // ]variable labels on their own line must be uniqued (same as
  // instruction-carrying ]var labels) so ExpandVarLabelsInOperand resolves
  // nearby references to the correct definition instance.
  if (label[0] == ']' && label.find(':') == std::string::npos) {
    int seq = ++var_label_seq_[label];
    scoped_label = label + "_" + std::to_string(seq);
  }
  symbols.Define(scoped_label, SymbolType::Label, std::make_shared<LiteralExpr>(label_addr));
  if (!in_dum_block_) {
    section.atoms.push_back(std::make_shared<LabelAtom>(scoped_label, label_addr));
  }
  // Only true global labels update the scope
  if (label[0] != ':' && (label[0] != ']' || current_scope_.global_label.empty())) {
    current_scope_.global_label = label;
    current_scope_.local_labels.clear();
  }
  return true;
}

// static
bool MerlinSyntaxParser::EquateNeedsReeval(const std::string& value) {
  for (size_t i = 0; i < value.size(); ++i) {
    char c = value[i];
    if (c == '*' && IsPC(value, i)) {
      return true;
    }
    if (c == '$') {
      // Skip hex literal digits
      while (i + 1 < value.size() && std::isxdigit(static_cast<unsigned char>(value[i + 1]))) {
        ++i;
      }
      continue;
    }
    if (std::isalpha(static_cast<unsigned char>(c))) {
      return true;
    }
  }
  return false;
}

bool MerlinSyntaxParser::HandleEquateLine(const std::string& label, const std::string& code_line,
                                          size_t equals_pos, Section& section,
                                          ConcreteSymbolTable& symbols) {
  if (equals_pos == std::string::npos || label.empty()) {
    return false;
  }
  std::string value = Trim(code_line.substr(equals_pos + 1));
  {
    DirectiveContext equ_ctx;
    equ_ctx.label = label;
    equ_ctx.operand = value;
    HandleEqu(equ_ctx, symbols);
  }
  // If expression contains '*' (PC) or a code-label reference, push an
  // EquateAtom so the assembler re-evaluates it each pass.
  if (!in_dum_block_ && EquateNeedsReeval(value)) {
    section.atoms.push_back(std::make_shared<EquateAtom>(label, value));
  }
  return true;
}

// ============================================================================
// Line Parsing
// ============================================================================

void MerlinSyntaxParser::ParseLine(const std::string& line, Section& section,
                                   ConcreteSymbolTable& symbols) {
  // If END directive seen, ignore all subsequent lines
  if (end_directive_seen_) {
    return;
  }

  // Strip comments first
  std::string code_line = StripComments(line);

  // Check if line is empty after stripping comments
  std::string trimmed = Trim(code_line);
  if (trimmed.empty()) {
    return;
  }

  std::string upper_trimmed = ToUpper(trimmed);

  if (HandleMacroCaptureState(code_line, upper_trimmed)) {
    return;
  }
  if (HandleLupCaptureState(code_line, upper_trimmed, section, symbols)) {
    return;
  }
  if (HandleConditionalDirective(trimmed, upper_trimmed, symbols)) {
    return;
  }

  // Check if we should skip this line due to conditional assembly
  if (!conditional_.ShouldEmit()) {
    return;
  }

  // Parse label (if present) - use original (untrimmed) code_line
  // Labels must start in column 1, so leading whitespace means no label
  size_t pos = 0;
  std::string label = ParseLabel(code_line, pos, section, symbols);

  // Skip whitespace after label
  while (pos < code_line.length() && std::isspace(code_line[pos])) {
    pos++;
  }

  // If only label on line (no directive/instruction)
  if (pos >= code_line.length()) {
    HandleLabelOnlyLine(label, section, symbols);
    return;
  }

  // Check for = syntax (alternate EQU): LABEL = VALUE
  size_t equals_pos = code_line.find('=', pos);
  if (HandleEquateLine(label, code_line, equals_pos, section, symbols)) {
    return;
  }

  // Extract directive/instruction and operands
  size_t space_pos = code_line.find(' ', pos);
  std::string directive;
  std::string operands;

  if (space_pos != std::string::npos) {
    directive = ToUpper(Trim(code_line.substr(pos, space_pos - pos)));
    operands = Trim(code_line.substr(space_pos + 1));
  } else {
    directive = ToUpper(Trim(code_line.substr(pos)));
    operands = "";
  }

  // Dispatch to directive, macro, or instruction handler.
  // TryHandleDirectiveLine and TryHandleMacroLine preserve the original
  // if-else ordering: directives checked first, then macros, then instructions.
  if (TryHandleDirectiveLine(directive, label, operands, section, symbols)) {
    return;
  }
  if (TryHandleMacroLine(directive, label, operands, section, symbols)) {
    return;
  }
  HandleInstructionLine(directive, label, operands, section, symbols);
}

// ============================================================================
// Main Parse Function
// ============================================================================

void MerlinSyntaxParser::Parse(const std::string& source, Section& section,
                               ConcreteSymbolTable& symbols) {
  if (source.empty()) {
    return;
  }

  // Reset parser state
  current_address_ = 0;
  in_dum_block_ = false;
  dum_address_ = 0;
  current_scope_.global_label.clear();
  current_scope_.local_labels.clear();
  variable_labels_.clear();
  current_line_ = 0;  // Reset line counter
  in_macro_definition_ = false;
  macro_expansion_depth_ = 0;
  macros_.clear();  // Clear macros from previous parse
  in_lup_block_ = false;
  lup_body_.clear();
  lup_count_ = 0;
  lup_nesting_depth_ = 0;

  // Split into lines and parse
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    current_line_++;  // Increment line counter for each line
    ParseLine(line, section, symbols);
  }

  // Validate that all DO blocks are closed (Phase 4: use shared component)
  if (!conditional_.IsBalanced()) {
    throw std::runtime_error(FormatError("Unmatched DO directive (missing FIN)"));
  }

  // Validate that macro definitions are closed
  if (in_macro_definition_) {
    throw std::runtime_error(FormatError("Unclosed macro definition (missing <<<)"));
  }

  // Validate that LUP blocks are closed
  if (in_lup_block_) {
    throw std::runtime_error(FormatError("Unclosed LUP block (missing --^)"));
  }
}

}  // namespace xasm
