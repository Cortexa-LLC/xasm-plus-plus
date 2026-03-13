// MerlinSyntax - Merlin assembly syntax parser implementation
// Phases 1-3: Foundation, Local Labels, DUM Blocks

#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/merlin_directive_handlers.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace xasm {

using xasm::util::ToUpper;
using xasm::util::Trim;
using namespace xasm::directives;

namespace {

// Radix values for number parsing
constexpr int RADIX_BINARY = 2;
constexpr int RADIX_DECIMAL = 10;
constexpr int RADIX_HEXADECIMAL = 16;

// Platform-aware temp directory helper
static std::string get_temp_dir() {
#ifdef _WIN32
  const char *temp = std::getenv("TEMP");
  if (!temp)
    temp = std::getenv("TMP");
  if (!temp)
    temp = "C:\\Windows\\Temp";
  std::string temp_str(temp);
  // Normalize to forward slashes for consistency
  for (char &c : temp_str) {
    if (c == '\\')
      c = '/';
  }
  return temp_str;
#else
  return "/tmp";
#endif
}

} // anonymous namespace

// ============================================================================
// Constructor
// ============================================================================

MerlinSyntaxParser::MerlinSyntaxParser()
    : in_macro_definition_(false), macro_expansion_depth_(0),
      in_lup_block_(false), lup_count_(0), lup_nesting_depth_(0),
      in_dum_block_(false), dum_address_(0), current_address_(0),
      end_directive_seen_(false), current_file_("<stdin>"), current_line_(0),
      cpu_(nullptr) {
  InitializeDirectiveRegistry();
}

void MerlinSyntaxParser::SetCpu(Cpu6502 *cpu) { cpu_ = cpu; }

// ============================================================================
// Directive Registry
// ============================================================================

void MerlinSyntaxParser::InitializeDirectiveRegistry() {
  // Register directive handlers from merlin namespace
  directive_registry_[ORG] = merlin::HandleOrg;
  directive_registry_[EQU] = merlin::HandleEqu;
  directive_registry_[DB] = merlin::HandleDb;
  directive_registry_[DFB] = merlin::HandleDb; // Alias
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

bool MerlinSyntaxParser::DispatchDirective(const std::string &directive,
                                           const std::string &label,
                                           const std::string &operand,
                                           DirectiveContext &context) {
  auto it = directive_registry_.find(directive);
  if (it != directive_registry_.end()) {
    // Found directive - invoke handler
    it->second(label, operand, context);
    return true;
  }
  return false; // Unknown directive
}

// ============================================================================
// Helper Functions
// ============================================================================

// Strip comments: * in column 1 or ; anywhere
std::string MerlinSyntaxParser::StripComments(const std::string &line) {
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
std::string MerlinSyntaxParser::FormatError(const std::string &message) const {
  std::ostringstream oss;
  oss << current_file_ << ":" << current_line_ << ": error: " << message;
  return oss.str();
}

// ============================================================================
// Number Parsing (DEPRECATED - delegated to ExpressionParser)
// ============================================================================

// Parse number in various formats: $hex, %binary, decimal
// NOTE: Kept for HandleOrg compatibility - will refactor in future phase
uint32_t MerlinSyntaxParser::ParseNumber(const std::string &str) {
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
  ConcreteSymbolTable empty_symbols;
  ExpressionParser parser(&empty_symbols);
  try {
    auto expr = parser.Parse(clean_str);
    return static_cast<uint32_t>(expr->Evaluate(empty_symbols));
  } catch (const std::runtime_error &e) {
    // Re-throw with Merlin formatting
    throw std::runtime_error(FormatError(e.what()));
  }
}

// Parse expression - delegates to ExpressionParser for standard operations
// while preserving Merlin-specific features: character literals, low/high byte
std::shared_ptr<Expression>
MerlinSyntaxParser::ParseExpression(const std::string &str,
                                    ConcreteSymbolTable &symbols) {

  std::string expr = Trim(str);

  // ========================================================================
  // Merlin-Specific Pre-Processing (V1 and V2 — moved from assembler.cpp)
  // ========================================================================

  // V1: Strip Merlin inline string comment from expression, e.g. "99 "stabbed""
  // A Merlin comment is a whitespace-separated string literal suffix:
  //   lda #99 "stabbed"  → expr is  99 "stabbed"  → strip to  99
  // Only strip when the expression does NOT start with a quote (otherwise
  // the whole expression is a character literal, e.g. #"A").
  if (!expr.empty() && expr[0] != '"' && expr[0] != '\'') {
    for (size_t i = 0; i < expr.size(); ++i) {
      if (expr[i] == ' ' || expr[i] == '\t') {
        size_t j = i;
        while (j < expr.size() && (expr[j] == ' ' || expr[j] == '\t'))
          ++j;
        if (j < expr.size() && (expr[j] == '"' || expr[j] == '\'')) {
          expr = Trim(expr.substr(0, i));
          break;
        }
      }
    }
  }

  // V2: Handle Merlin .Inc/.Dec suffixes: symbol.Inc → symbol+1, symbol.Dec → symbol-1
  // Case-insensitive check for the suffix
  auto ends_with_ci = [](const std::string &s, const char *suffix,
                          size_t slen) {
    if (s.size() < slen)
      return false;
    for (size_t i = 0; i < slen; ++i) {
      if (std::tolower(static_cast<unsigned char>(s[s.size() - slen + i])) !=
          std::tolower(static_cast<unsigned char>(suffix[i])))
        return false;
    }
    return true;
  };
  if (ends_with_ci(expr, ".inc", 4))
    expr = expr.substr(0, expr.size() - 4) + "+1";
  else if (ends_with_ci(expr, ".dec", 4))
    expr = expr.substr(0, expr.size() - 4) + "-1";

  // ========================================================================
  // Merlin-Specific Features (handle before delegating to ExpressionParser)
  // ========================================================================

  // Check for character literal: "x" or 'x'
  // Supports standalone form ("A") and compound form ("A"-CTRL, "A"+1, etc.)
  // Apple II Merlin convention: character literals set the high bit ($80).
  // e.g. EQU "A" → $C1,  EQU "r"-CTRL → $F2-CTRL
  if (!expr.empty() && (expr[0] == '"' || expr[0] == '\'')) {
    char quote = expr[0];
    if (expr.length() == 1) {
      // Just a lone quote (e.g. SPECIALK.S edge case) – treat as 0
      return std::make_shared<LiteralExpr>(0);
    }
    // Find the closing quote
    size_t close = expr.find(quote, 1);
    if (close == std::string::npos) {
      // Unclosed quote – return 0 gracefully (matches Merlin behaviour)
      return std::make_shared<LiteralExpr>(0);
    }
    // Extract character between quotes and apply Apple II high-bit convention
    std::string chars = expr.substr(1, close - 1);
    int64_t char_val =
        chars.empty() ? 0 : (static_cast<uint8_t>(chars[0]) | 0x80);
    // Check for compound expression after the closing quote (e.g. "A"-CTRL)
    std::string rest = Trim(expr.substr(close + 1));
    if (rest.empty()) {
      return std::make_shared<LiteralExpr>(char_val);
    }
    // Combine char literal value with the remaining expression via the
    // operator that leads the rest string (e.g. "+1", "-CTRL").
    if (!rest.empty() && (rest[0] == '+' || rest[0] == '-' ||
                          rest[0] == '*' || rest[0] == '/')) {
      char op = rest[0];
      std::string rhs = Trim(rest.substr(1));
      if (!rhs.empty()) {
        auto left_expr  = std::make_shared<LiteralExpr>(char_val);
        auto right_expr = ParseExpression(rhs, symbols);
        switch (op) {
          case '+':
            return std::make_shared<BinaryOpExpr>(BinaryOp::Add, left_expr,
                                                  right_expr);
          case '-':
            return std::make_shared<BinaryOpExpr>(BinaryOp::Subtract, left_expr,
                                                  right_expr);
          case '*':
            return std::make_shared<BinaryOpExpr>(BinaryOp::Multiply, left_expr,
                                                  right_expr);
          case '/':
            return std::make_shared<BinaryOpExpr>(BinaryOp::Divide, left_expr,
                                                  right_expr);
          default:
            break;
        }
      }
    }
    // No recognised compound form – return the character value alone
    return std::make_shared<LiteralExpr>(char_val);
  }

  // Check for low byte operator (< or #)
  if (!expr.empty() && (expr[0] == '<' || expr[0] == '#')) {
    if (expr.length() < 2) {
      throw std::runtime_error(
          FormatError("Low byte operator (</#) requires an operand"));
    }
    std::string operand = Trim(expr.substr(1));
    if (operand.empty()) {
      throw std::runtime_error(
          FormatError("Low byte operator (</#) has empty operand"));
    }
    // Recursively parse the operand (might be expression like SHIFT0-$80)
    auto operand_expr = ParseExpression(operand, symbols);
    int64_t value = operand_expr->Evaluate(symbols);
    return std::make_shared<LiteralExpr>(value & 0xFF); // Low byte
  }

  // Check for high byte operator (>)
  if (!expr.empty() && expr[0] == '>') {
    if (expr.length() < 2) {
      throw std::runtime_error(
          FormatError("High byte operator (>) requires an operand"));
    }
    std::string operand = Trim(expr.substr(1));
    if (operand.empty()) {
      throw std::runtime_error(
          FormatError("High byte operator (>) has empty operand"));
    }
    // Recursively parse the operand (might be expression like SHIFT0-$80)
    auto operand_expr = ParseExpression(operand, symbols);
    int64_t value = operand_expr->Evaluate(symbols);
    return std::make_shared<LiteralExpr>((value >> 8) & 0xFF); // High byte
  }

  // ========================================================================
  // Delegate to Shared ExpressionParser
  // ========================================================================

  // Check for empty expression (legacy behavior: treat as 0)
  // This handles edge cases like "X+" where trailing operator leaves empty
  // right side
  if (expr.empty()) {
    return std::make_shared<LiteralExpr>(0);
  }

  // Check for Merlin edge cases that conflict with Z80 semantics
  if (expr == "$") {
    // In Merlin, "$" alone is an error (not program counter like Z80)
    // Merlin uses "*" for program counter
    throw std::runtime_error(
        FormatError("Invalid hex number: '$' (no digits after $)"));
  }

  // Check for trailing operators (legacy Merlin behavior: treat missing operand
  // as 0) Examples: "X+", "Y-", "Z*"
  if (!expr.empty()) {
    char last_char = expr[expr.length() - 1];
    if (last_char == '+' || last_char == '-' || last_char == '*') {
      // Strip trailing operator and parse as is
      // Old Merlin behavior: X+ evaluates to X+0 = X
      std::string clean_expr = Trim(expr.substr(0, expr.length() - 1));
      if (!clean_expr.empty()) {
        return ParseExpression(clean_expr, symbols);
      }
    }
  }

  // Substitute * (program counter) with current address before delegating.
  // Handles forms like: *-CHECKER, maxgatevel=*-gatevel-1, CHECKEND=*-START.
  // A standalone '*' or '*' at start/after operator = program counter.
  // '*' with operands on BOTH sides = multiplication — leave alone.
  if (expr.find('*') != std::string::npos) {
    std::ostringstream hex_stream;
    hex_stream << "$" << std::hex
               << (in_dum_block_ ? dum_address_ : current_address_);
    std::string pc_hex = hex_stream.str();
    size_t p = 0;
    while ((p = expr.find('*', p)) != std::string::npos) {
      bool before_ident =
          p > 0 && (std::isalnum(static_cast<unsigned char>(expr[p - 1])) ||
                    expr[p - 1] == ')');
      bool after_ident =
          p + 1 < expr.length() &&
          (std::isalnum(static_cast<unsigned char>(expr[p + 1])) ||
           expr[p + 1] == '(' || expr[p + 1] == '$' || expr[p + 1] == '%');
      if (before_ident && after_ident) {
        p++; // multiplication — skip
      } else {
        expr.replace(p, 1, pc_hex);
        p += pc_hex.length();
      }
    }
  }

  // All other expressions (arithmetic, symbols, literals) handled by shared
  // parser
  ExpressionParser parser(&symbols);
  try {
    return parser.Parse(expr);
  } catch (const std::runtime_error &e) {
    // Catch exceptions from ExpressionParser and add location context
    throw std::runtime_error(FormatError(e.what()));
  }
}

// ============================================================================
// Local Label Scoping Helpers
// ============================================================================

std::string
MerlinSyntaxParser::ScopeLocalLabel(const std::string &label) const {
  if (!label.empty() && label[0] == ':' &&
      !current_scope_.global_label.empty()) {
    return current_scope_.global_label + label;
  }
  return label;
}

std::string
MerlinSyntaxParser::SubstituteMerlinVars(const std::string &line,
                                         const ConcreteSymbolTable &symbols) const {
  // Fast path: no ] in line
  if (line.find(']') == std::string::npos) {
    return line;
  }

  // In Merlin, labels (including ]variables) occupy the FIRST column (no
  // leading whitespace).  A line like "]byte = ]byte+1" has "]byte" as a
  // label/LHS — HandleEqu already evaluates the RHS eagerly, so we must NOT
  // substitute the leading ]var or we would turn the line into "0 = 0+1".
  // Strategy: if the line starts with ']', copy through the label token and
  // the '=' sign unchanged, then substitute only the RHS.
  size_t start_substitute = 0;
  if (!line.empty() && line[0] == ']') {
    // Find end of label token
    size_t label_end = 1;
    while (label_end < line.size() &&
           (std::isalnum(static_cast<unsigned char>(line[label_end])) ||
            line[label_end] == '_')) {
      ++label_end;
    }
    // Skip optional whitespace and '=' so we only substitute the RHS
    size_t pos = label_end;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
      ++pos;
    }
    if (pos < line.size() && line[pos] == '=') {
      ++pos; // skip '='
      // start_substitute points to the RHS; copy prefix unchanged
      start_substitute = pos;
    }
    // If it's not an assignment (e.g., "]var DS 1"), substitute everything
    // after the label token so we don't rename the variable being declared.
    // Actually for DS and similar directives on a ]var label, the label is
    // resolved by HandleEqu/ParseLine separately — just skip the label token.
    if (start_substitute == 0) {
      start_substitute = label_end;
    }
  }

  std::string result = line.substr(0, start_substitute);
  result.reserve(line.size());

  for (size_t i = start_substitute; i < line.size();) {
    // Skip quoted strings without substitution
    if (line[i] == '"' || line[i] == '\'') {
      char quote = line[i];
      result += line[i++];
      while (i < line.size() && line[i] != quote) {
        result += line[i++];
      }
      if (i < line.size()) {
        result += line[i++]; // closing quote
      }
      continue;
    }

    if (line[i] == ']') {
      // Collect ]identifier
      size_t tok_start = i++;
      while (i < line.size() &&
             (std::isalnum(static_cast<unsigned char>(line[i])) ||
              line[i] == '_')) {
        ++i;
      }
      std::string var_name = line.substr(tok_start, i - tok_start);
      // Look up current value
      int64_t sym_val = 0;
      if (symbols.Lookup(var_name, sym_val)) {
        result += std::to_string(sym_val);
        continue;
      }
      // Not found — output original token
      result += var_name;
      continue;
    }

    result += line[i++];
  }

  return result;
}

std::string
MerlinSyntaxParser::ScopeLocalLabelsInOperand(const std::string &operand) const {
  if (current_scope_.global_label.empty() || operand.empty()) {
    return operand;
  }

  std::string result;
  result.reserve(operand.size() + current_scope_.global_label.size() * 2);

  for (size_t i = 0; i < operand.size();) {
    char c = operand[i];
    // A ':word' token starts at position 0 or after a non-identifier char
    bool at_word_start =
        (i == 0) ||
        (!std::isalnum(static_cast<unsigned char>(operand[i - 1])) &&
         operand[i - 1] != '_');

    if (c == ':' && at_word_start && i + 1 < operand.size() &&
        (std::isalnum(static_cast<unsigned char>(operand[i + 1])) ||
         operand[i + 1] == '_')) {
      // Local label reference — prepend global scope: ADDSOUND:rts
      result += current_scope_.global_label;
      result += ':';
      i++; // skip the ':'
      while (i < operand.size() &&
             (std::isalnum(static_cast<unsigned char>(operand[i])) ||
              operand[i] == '_')) {
        result += operand[i++];
      }
    } else {
      result += c;
      i++;
    }
  }

  return result;
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
std::string
MerlinSyntaxParser::ExpandMerlinCharLiterals(const std::string &operand) const {
  // Fast path: no quote character → nothing to expand
  if (operand.find('"') == std::string::npos &&
      operand.find('\'') == std::string::npos) {
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
      if (i + 2 < operand.size() &&
          std::isprint(static_cast<unsigned char>(operand[i + 1])) &&
          operand[i + 2] == c) {
        // Replace "X" or 'X' with $XX (Apple II high-bit value)
        uint8_t val = static_cast<uint8_t>(operand[i + 1]) | 0x80;
        // Format as $XX
        char hex_buf[8];
        std::snprintf(hex_buf, sizeof(hex_buf), "$%02X",
                      static_cast<unsigned>(val));
        result += hex_buf;
        i += 3; // skip quote, char, closing quote
        continue;
      }
    }
    result += c;
    ++i;
  }
  return result;
}

std::string
MerlinSyntaxParser::ExpandVarLabelsInOperand(const std::string &operand) const {
  // Fast path: no ] in operand
  if (operand.find(']') == std::string::npos) {
    return operand;
  }

  std::string result;
  result.reserve(operand.size() + 16);

  for (size_t i = 0; i < operand.size();) {
    if (operand[i] == ']') {
      // Collect ]identifier
      size_t tok_start = i++;
      while (i < operand.size() &&
             (std::isalnum(static_cast<unsigned char>(operand[i])) ||
              operand[i] == '_')) {
        ++i;
      }
      std::string var_name = operand.substr(tok_start, i - tok_start);

      // Check if this ]varname is immediately followed by ':'.
      // If so, it's acting as a scope qualifier ("]rts:local" form) and must
      // NOT be expanded — the :local label was defined under the original
      // (non-unique) scope name "]rts", so the reference must stay as "]rts".
      bool followed_by_colon = (i < operand.size() && operand[i] == ':');

      auto it = var_label_seq_.find(var_name);
      if (!followed_by_colon && it != var_label_seq_.end() && it->second > 0) {
        // Direct reference: replace ]varname with ]varname_N
        result += var_name + "_" + std::to_string(it->second);
      } else {
        // Scope qualifier or no instance tracked — keep as-is
        result += var_name;
      }
    } else {
      result += operand[i++];
    }
  }

  return result;
}

// ============================================================================
// Label Parsing
// ============================================================================

std::string MerlinSyntaxParser::ParseLabel(const std::string &line, size_t &pos,
                                           Section & /*section*/,
                                           ConcreteSymbolTable & /*symbols*/) {
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

void MerlinSyntaxParser::HandleEqu(const std::string &label,
                                   const std::string &operand,
                                   ConcreteSymbolTable &symbols) {
  // EQU directive - define symbolic constant (no code generated)
  auto expr = ParseExpression(operand, symbols);
  // Eagerly evaluate to a literal when possible to prevent circular
  // self-references (e.g., ]var = ]var+1 in LUP loops).  Deferred evaluation
  // is still used when the expression references undefined forward symbols.
  try {
    int64_t val = expr->Evaluate(symbols);
    symbols.Define(label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(val));
  } catch (const std::exception &) {
    // Cannot evaluate yet (forward reference) — store deferred expression
    symbols.Define(label, SymbolType::Label, expr);
  }
}

void MerlinSyntaxParser::HandleDS(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  // DS directive - define space (reserve bytes)
  // Supports: DS 100        (literal)
  //           DS COUNT      (symbol)
  //           DS *+10       (program counter arithmetic)

  std::string op = Trim(operand);

  // Substitute * (program counter) with current address
  // But NOT when * is multiplication operator (between operands)
  if (op.find('*') != std::string::npos) {
    std::ostringstream hex_stream;
    hex_stream << "$" << std::hex
               << (in_dum_block_ ? dum_address_ : current_address_);
    std::string pc_hex = hex_stream.str();

    // Replace * with address, checking context
    size_t pos = 0;
    while ((pos = op.find('*', pos)) != std::string::npos) {
      bool is_multiplication = false;

      // Check operand before *
      bool has_operand_before = false;
      if (pos > 0) {
        char before = op[pos - 1];
        if (std::isalnum(static_cast<unsigned char>(before)) || before == ')') {
          has_operand_before = true;
        }
      }

      // Check operand after *
      bool has_operand_after = false;
      if (pos + 1 < op.length()) {
        char after = op[pos + 1];
        if (std::isalnum(static_cast<unsigned char>(after)) || after == '(' ||
            after == '$' || after == '%') {
          has_operand_after = true;
        }
      }

      // Multiplication only if operands on BOTH sides
      is_multiplication = has_operand_before && has_operand_after;

      if (is_multiplication) {
        pos++; // Skip multiplication operator
      } else {
        // Program counter - substitute
        op.replace(pos, 1, pc_hex);
        pos += pc_hex.length();
      }
    }
  }

  // Parse expression (supports arithmetic, symbols, literals)
  uint32_t count = 0;
  if (!op.empty()) {
    auto expr = ParseExpression(op, symbols);
    try {
      int64_t value = expr->Evaluate(symbols);
      if (value < 0) {
        throw std::runtime_error(FormatError(
            "DS: Negative count not allowed: " + std::to_string(value)));
      }
      count = static_cast<uint32_t>(value);
    } catch (const std::runtime_error &e) {
      // Re-throw with location information if not already formatted
      std::string msg = e.what();
      // Check if message already has file:line: format
      // Format is: filename:number: error: message
      size_t first_colon = msg.find(':');
      bool has_location = false;
      if (first_colon != std::string::npos && first_colon < msg.length() - 1) {
        size_t second_colon = msg.find(':', first_colon + 1);
        if (second_colon != std::string::npos &&
            second_colon > first_colon + 1) {
          // Check if there's a digit between the two colons (line number)
          bool has_digit = false;
          for (size_t i = first_colon + 1; i < second_colon; ++i) {
            if (std::isdigit(static_cast<unsigned char>(msg[i]))) {
              has_digit = true;
              break;
            }
          }
          has_location = has_digit;
        }
      }

      if (!has_location) {
        throw std::runtime_error(FormatError(msg));
      } else {
        throw;
      }
    }
  }

  // DUM blocks: advance address without emitting bytes
  if (!in_dum_block_) {
    // If the original operand contains '*' (PC-relative), store the raw
    // expression so the assembler can re-evaluate it each pass (as the PC
    // shifts when branch relaxations change code sizes between passes).
    std::string raw = Trim(operand);
    bool has_star = false;
    for (size_t i = 0; i < raw.size(); ++i) {
      if (raw[i] == '*') {
        // Check it's not a multiplication operator (operands on both sides)
        bool before = (i > 0 && (std::isalnum(static_cast<unsigned char>(raw[i-1])) || raw[i-1] == ')'));
        bool after  = (i+1 < raw.size() && (std::isalnum(static_cast<unsigned char>(raw[i+1])) || raw[i+1] == '(' || raw[i+1] == '$' || raw[i+1] == '%'));
        if (!(before && after)) { has_star = true; break; }
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

void MerlinSyntaxParser::HandleDum(const std::string &operand,
                                   ConcreteSymbolTable &symbols) {
  // DUM (Dummy section) - start variable definition block
  in_dum_block_ = true;

  std::string op = Trim(operand);

  // Check if operand is empty
  if (op.empty()) {
    throw std::runtime_error(
        FormatError("DUM directive requires an address operand"));
  }

  // Parse number (decimal, hex, or binary)
  if (op[0] == '$') {
    // Hex
    dum_address_ = std::stoul(op.substr(1), nullptr, RADIX_HEXADECIMAL);
  } else if (op[0] == '%') {
    // Binary
    dum_address_ = std::stoul(op.substr(1), nullptr, RADIX_BINARY);
  } else if (std::isdigit(op[0])) {
    // Decimal
    dum_address_ = std::stoul(op, nullptr, RADIX_DECIMAL);
  } else {
    // Symbol - look it up
    int64_t value = 0;
    if (symbols.Lookup(op, value)) {
      dum_address_ = static_cast<uint32_t>(value);
    } else {
      // Symbol not found - ERROR instead of silently using 0
      throw std::runtime_error(
          FormatError("DUM directive: symbol '" + op + "' not defined"));
    }
  }
}

void MerlinSyntaxParser::HandleDend() {
  // DEND - end dummy section
  in_dum_block_ = false;
}

void MerlinSyntaxParser::HandlePut(const std::string &operand, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // PUT filename - include another source file
  std::string filename = Trim(operand);

  // Auto-append .S extension if no extension present
  if (filename.find('.') == std::string::npos) {
    filename += ".S";
  }

  // Check for circular includes
  for (const auto &included_file : include_stack_) {
    if (included_file == filename) {
      throw std::runtime_error(
          FormatError("Circular include detected: " + filename));
    }
  }

  // Add to include stack
  include_stack_.push_back(filename);

  // Try to open the file - first as given, then with temp directory prefix
  std::ifstream file(filename);
  std::string actual_filename = filename;

  if (!file.is_open() && filename[0] != '/') {
    // Try with temp directory prefix for relative paths (platform-aware)
    actual_filename = get_temp_dir() + "/" + filename;
    file.open(actual_filename);
  }

  if (!file.is_open()) {
    include_stack_.pop_back(); // Remove from stack on error
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

void MerlinSyntaxParser::HandleDo(const std::string &operand,
                                  ConcreteSymbolTable &symbols) {
  // DO directive - begin conditional assembly block
  // Evaluate operand expression: non-zero = true, zero = false

  std::string op = Trim(operand);
  if (op.empty()) {
    throw std::runtime_error("DO directive requires an operand expression");
  }

  // Evaluate the expression to determine condition
  uint32_t value = 0;
  if (op[0] == '$') {
    // Hex literal
    value = std::stoul(op.substr(1), nullptr, RADIX_HEXADECIMAL);
  } else if (op[0] == '%') {
    // Binary literal
    value = std::stoul(op.substr(1), nullptr, RADIX_BINARY);
  } else if (std::isdigit(op[0])) {
    // Decimal literal
    value = std::stoul(op, nullptr, RADIX_DECIMAL);
  } else {
    // Symbol - look it up
    int64_t sym_value = 0;
    if (symbols.Lookup(op, sym_value)) {
      value = static_cast<uint32_t>(sym_value);
    } else {
      // Undefined symbol evaluates to 0 (false)
      value = 0;
    }
  }

  // Begin conditional block (non-zero = true)
  conditional_.BeginIf(value != 0);
}

void MerlinSyntaxParser::HandleElse() {
  // ELSE directive - toggle conditional assembly state
  try {
    conditional_.BeginElse();
  } catch (const std::runtime_error &e) {
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
  } catch (const std::runtime_error &e) {
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

void MerlinSyntaxParser::HandlePMC(const std::string &operand) {
  // PMC - Start macro definition
  if (in_macro_definition_) {
    throw std::runtime_error(
        FormatError("Nested macro definitions not allowed"));
  }
  in_macro_definition_ = true;
  current_macro_.name = ToUpper(Trim(operand)); // Normalize macro name
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

void MerlinSyntaxParser::ExpandMacro(const std::string &macro_name,
                                     const std::string &operand,
                                     Section &section,
                                     ConcreteSymbolTable &symbols) {
  // Expand a macro: MACRONAME param1,param2,...

  // Check recursion depth
  if (macro_expansion_depth_ >= 100) {
    throw std::runtime_error(FormatError(
        "Macro expansion depth limit exceeded (possible recursion)"));
  }

  std::string upper_name = ToUpper(macro_name);

  // Look up macro
  auto it = macros_.find(upper_name);
  if (it == macros_.end()) {
    throw std::runtime_error(FormatError("Undefined macro: " + macro_name));
  }

  const MacroDefinition &macro = it->second;

  // Parse parameters from operand
  // Parameters can be separated by semicolons (MAC style) or commas (alternate
  // style)
  std::vector<std::string> params;
  if (!operand.empty()) {
    std::string token;
    bool in_string = false;
    char string_delimiter = '\0';

    // Determine separator: semicolon for MAC style, comma otherwise
    char separator = (operand.find(';') != std::string::npos) ? ';' : ',';

    for (char c :
         operand + separator) { // Add trailing separator to flush last token
      if (!in_string && (c == '"' || c == '\'')) {
        in_string = true;
        string_delimiter = c;
        token += c;
      } else if (in_string && c == string_delimiter) {
        in_string = false;
        token += c;
      } else if (!in_string && (c == separator)) {
        std::string trimmed = Trim(token);
        if (!trimmed.empty()) {
          params.push_back(trimmed);
        }
        token.clear();
      } else {
        token += c;
      }
    }
  }

  // Expand macro body
  macro_expansion_depth_++;

  for (const auto &line : macro.body) {
    // Substitute parameters
    std::string expanded = SubstituteParameters(line, params);

    // Parse expanded line recursively
    ParseLine(expanded, section, symbols);
  }

  macro_expansion_depth_--;
}

std::string MerlinSyntaxParser::SubstituteParameters(
    const std::string &line, const std::vector<std::string> &params) {
  // Replace ]1, ]2, etc. with actual parameters
  std::string result;

  for (size_t i = 0; i < line.length(); ++i) {
    if (line[i] == ']' && i + 1 < line.length() && std::isdigit(line[i + 1])) {
      int param_num = line[i + 1] - '0';
      if (param_num > 0 && param_num <= static_cast<int>(params.size())) {
        // Valid parameter reference - substitute
        result += params[param_num - 1];
        i++; // Skip the digit
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

void MerlinSyntaxParser::HandleXc(const std::string &operand) {
  // XC [ON|OFF] - Toggle 65C02 CPU instruction set

  if (!cpu_) {
    // No CPU set - silently ignore (for tests that don't need CPU)
    return;
  }

  std::string op = ToUpper(Trim(operand));

  if (op.empty() || op == directives::ON) {
    // Enable 65C02 mode
    cpu_->SetCpuMode(CpuMode::Cpu65C02);
  } else if (op == directives::OFF) {
    // Disable 65C02 mode (back to 6502)
    cpu_->SetCpuMode(CpuMode::Cpu6502);
  } else {
    throw std::runtime_error(
        FormatError("XC: invalid operand (expected ON or OFF)"));
  }
}

void MerlinSyntaxParser::HandleMx(const std::string &operand) {
  // MX mode - Set 65816 accumulator and index register widths
  // This is a directive only - tracks state but doesn't change CPU encoding

  std::string op = Trim(operand);
  if (op.empty()) {
    throw std::runtime_error(FormatError("MX directive requires an operand"));
  }

  int mode = -1;

  // Check for binary format %00-%11
  if (op[0] == '%') {
    std::string binary = op.substr(1);
    if (binary == "00")
      mode = 0;
    else if (binary == "01")
      mode = 1;
    else if (binary == "10")
      mode = 2;
    else if (binary == "11")
      mode = 3;
    else {
      throw std::runtime_error(
          FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
    }
  }
  // Check for decimal format 0-3
  else if (op.length() == 1 && op[0] >= '0' && op[0] <= '3') {
    mode = op[0] - '0';
  } else {
    throw std::runtime_error(
        FormatError("MX directive expects binary %00-%11 or decimal 0-3"));
  }

  // Mode validated - in full implementation, would affect 65816 encoding
  // For now, just validate and accept
  (void)mode; // Suppress unused variable warning
}

void MerlinSyntaxParser::HandleLup(const std::string &operand) {
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
    throw std::runtime_error(
        FormatError("LUP count must be a number: " + count_str));
  }

  if (count < 0) {
    throw std::runtime_error(
        FormatError("LUP count cannot be negative: " + count_str));
  }

  // Start capturing LUP block
  in_lup_block_ = true;
  lup_count_ = count;
  lup_body_.clear();
  lup_nesting_depth_ = 0; // Track nesting for nested LUP blocks
}

// ============================================================================
// Line Parsing
// ============================================================================

void MerlinSyntaxParser::ParseLine(const std::string &line, Section &section,
                                   ConcreteSymbolTable &symbols) {
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

  // If in macro definition mode, capture lines (except EOM or <<<)
  std::string upper_trimmed = ToUpper(trimmed);
  if (in_macro_definition_) {
    // Check for EOM directive to end macro definition
    if (upper_trimmed == directives::EOM) {
      HandleEOM();
      return;
    }
    // Check for <<< to end macro definition (Merlin style)
    if (upper_trimmed == directives::MACRO_END_ALT) {
      HandleMacroEnd();
      return;
    }
    // Otherwise, add line to macro body
    current_macro_.body.push_back(code_line);
    return;
  }

  // If in LUP block mode, capture lines (except --^)
  if (in_lup_block_) {
    // Check for --^ directive to end LUP block
    if (upper_trimmed == "--^") {
      // If nesting depth > 0, this ends an inner LUP
      if (lup_nesting_depth_ > 0) {
        lup_nesting_depth_--;
        lup_body_.push_back(code_line);
        return;
      }

      // End of outermost LUP block - expand and parse
      std::vector<std::string> body_copy =
          lup_body_; // Copy to avoid use-after-free
      int count = lup_count_;

      // Reset LUP state BEFORE expansion (so lines can be parsed normally)
      in_lup_block_ = false;
      lup_body_.clear();
      lup_count_ = 0;
      lup_nesting_depth_ = 0;

      // Repeat the body count times.
      // Substitute ]variable values before each line so that each iteration
      // captures the variable's current value (mutable assembly-time counter).
      for (int i = 0; i < count; ++i) {
        for (const auto &lup_line : body_copy) {
          std::string expanded = SubstituteMerlinVars(lup_line, symbols);
          ParseLine(expanded, section, symbols);
        }
      }
      return;
    }

    // Check if this line starts a nested LUP block
    std::string lup_directive = std::string(directives::LUP) + " ";
    if (upper_trimmed.find(lup_directive) == 0 ||
        upper_trimmed == directives::LUP) {
      lup_nesting_depth_++;
    }

    // Add line to LUP body
    lup_body_.push_back(code_line);
    return;
  }

  // Check for conditional assembly directives (DO/ELSE/FIN)
  // These must be processed even when inside a false conditional block
  std::string do_directive = std::string(directives::DO) + " ";
  if (upper_trimmed.find(do_directive) == 0 ||
      upper_trimmed == directives::DO) {
    // Extract operand after "DO"
    std::string operand = trimmed.length() > 3 ? Trim(trimmed.substr(3)) : "0";
    HandleDo(operand, symbols);
    return;
  } else if (upper_trimmed == directives::ELSE) {
    HandleElse();
    return;
  } else if (upper_trimmed == directives::FIN) {
    HandleFin();
    return;
  }

  // Check if we should skip this line due to conditional assembly (Phase 4: use
  // shared component)
  if (!conditional_.ShouldEmit()) {
    return; // Skip this line - we're in a false conditional block
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
    if (!label.empty()) {
      // In DUM blocks, labels get the DUM address; no LabelAtom emitted
      // (LabelAtoms are overwritten by current_address in ResolveSymbols)
      uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
      std::string scoped_label = ScopeLocalLabel(label);
      symbols.Define(scoped_label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(label_addr));
      if (!in_dum_block_) {
        section.atoms.push_back(
            std::make_shared<LabelAtom>(scoped_label, label_addr));
      }

      // Only true global labels (no ':' or ']' prefix) update the scope.
      // ]variable labels are mutable variables, not scope anchors — EXCEPT
      // when there is no current scope yet (start of file before any true
      // global label), in which case they must anchor scope so that :local
      // labels defined after them can be resolved.
      if (label[0] != ':' &&
          (label[0] != ']' || current_scope_.global_label.empty())) {
        current_scope_.global_label = label;
        current_scope_.local_labels.clear();
      }
    }
    return;
  }

  // Check for = syntax (alternate EQU) first
  // Format: LABEL = VALUE
  size_t equals_pos = code_line.find('=', pos);
  if (equals_pos != std::string::npos && !label.empty()) {
    // This is an = equate
    std::string value = Trim(code_line.substr(equals_pos + 1));
    HandleEqu(label, value, symbols);
    // If the expression contains '*' as a PC reference (not multiplication),
    // push an EquateAtom so the assembler re-evaluates it each pass with the
    // correct virtual address.  At parse time, instruction sizes are placeholder
    // (1 byte each), so expressions like "CHECKEND = *-CHECKER" compute the
    // wrong value if evaluated only once at parse time.
    if (!in_dum_block_) {
      bool has_star = false;
      for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '*') {
          bool before =
              (i > 0 && (std::isalnum(static_cast<unsigned char>(value[i - 1]))
                         || value[i - 1] == ')'));
          bool after =
              (i + 1 < value.size() &&
               (std::isalnum(static_cast<unsigned char>(value[i + 1]))
                || value[i + 1] == '(' || value[i + 1] == '$'
                || value[i + 1] == '%'));
          if (!(before && after)) {
            has_star = true;
            break;
          }
        }
      }
      if (has_star) {
        section.atoms.push_back(std::make_shared<EquateAtom>(label, value));
      }
    }
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

  // Handle directives using DirectiveRegistry pattern
  // Build DirectiveContext for the handler
  DirectiveContext ctx;
  ctx.section = &section;
  ctx.symbols = &symbols;
  ctx.current_address = &current_address_;
  ctx.parser_state = this; // For accessing Merlin-specific state if needed
  ctx.current_file = current_file_;
  ctx.current_line = current_line_;

  // Scope the label and operands before dispatching so directive handlers
  // define label atoms under the correct scoped name (e.g. "tone:pitch" not
  // just ":pitch"), and operand references like ":loop" are expanded to their
  // scoped names (e.g. "alertstand:loop").
  std::string scoped_label_for_directive = ScopeLocalLabel(label);
  std::string scoped_operands_for_directive = ScopeLocalLabelsInOperand(operands);
  if (DispatchDirective(directive, scoped_label_for_directive,
                        scoped_operands_for_directive, ctx)) {
    // Directive was handled by registry.
    // Update global label scope from the RAW (un-scoped) label so that
    // subsequent ':local' labels are correctly qualified.  Only non-local
    // labels (those that don't start with ':') update the scope.
    if (!label.empty() && label[0] != ':' &&
        (label[0] != ']' || current_scope_.global_label.empty())) {
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    return;
  }

  // Not a directive - check if it's a macro invocation
  std::string upper_directive = ToUpper(directive);
  if (macros_.find(upper_directive) != macros_.end()) {
    // Create label atom first if label present
    if (!label.empty()) {
      uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
      std::string scoped_label = ScopeLocalLabel(label);
      symbols.Define(scoped_label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(label_addr));
      if (!in_dum_block_) {
        section.atoms.push_back(
            std::make_shared<LabelAtom>(scoped_label, label_addr));
      }
      if (label[0] != ':' &&
          (label[0] != ']' || current_scope_.global_label.empty())) {
        current_scope_.global_label = label;
        current_scope_.local_labels.clear();
      }
    }
    // Expand macro
    ExpandMacro(directive, operands, section, symbols);
    return;
  }

  // Assume it's an instruction
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t label_addr = in_dum_block_ ? dum_address_ : current_address_;
    std::string scoped_label = ScopeLocalLabel(label);

    // ]variable code labels (e.g. "]rts  rts") are redefined across
    // subroutines.  Give each definition a unique name (]rts_1, ]rts_2, …) so
    // that multi-pass assembly resolves references to the *nearby* definition
    // rather than always using the last global one.  Only label-on-instruction
    // definitions get unique names; EQU-style ]var = VALUE assignments keep
    // their original name because they act as mutable numeric variables.
    if (!label.empty() && label[0] == ']' && label.find(':') == std::string::npos) {
      int seq = ++var_label_seq_[label]; // increment and capture new seq#
      scoped_label = label + "_" + std::to_string(seq);
    }

    symbols.Define(scoped_label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(label_addr));
    if (!in_dum_block_) {
      section.atoms.push_back(
          std::make_shared<LabelAtom>(scoped_label, label_addr));
    }
    if (label[0] != ':' &&
        (label[0] != ']' || current_scope_.global_label.empty())) {
      // True global labels (no ':' or ']' prefix) always update scope.
      // ]variable labels are mutable variables, NOT scope anchors — EXCEPT at
      // the start of the file before any true global label exists.  In that
      // case they must anchor scope so :local labels around them can resolve.
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
  }
  // Translate any ':word' local-label references to the scoped name, then
  // expand ]variable references to their current unique-instance names, then
  // strip Merlin inline string comments (e.g. "#99 \"stabbed\"" → "#99"),
  // then expand Merlin char literals ("X"/'X') to their Apple II high-bit hex
  // values so the shared ParseExpression never sees Merlin char-literal syntax.
  operands = ScopeLocalLabelsInOperand(operands);
  operands = ExpandVarLabelsInOperand(operands);
  // V1: Strip Merlin inline string comment from instruction operand.
  // A Merlin inline string comment is a whitespace-separated quoted-string
  // suffix that is not itself a char literal, e.g. "#99 \"stabbed\"".
  // Only strip when the operand (after skipping a leading '#') does NOT start
  // with a quote (which would be a char literal like "#\"A\"").
  {
    std::string stripped = operands;
    // Skip leading '#' or '<' prefix (immediate / low-byte operators)
    size_t offset = 0;
    if (!stripped.empty() && (stripped[0] == '#' || stripped[0] == '<'))
      offset = 1;
    // If the expression itself starts with a quote, leave it alone (char literal)
    if (offset < stripped.size() && stripped[offset] != '"' &&
        stripped[offset] != '\'') {
      for (size_t i = offset; i < stripped.size(); ++i) {
        if (stripped[i] == ' ' || stripped[i] == '\t') {
          size_t j = i;
          while (j < stripped.size() &&
                 (stripped[j] == ' ' || stripped[j] == '\t'))
            ++j;
          if (j < stripped.size() &&
              (stripped[j] == '"' || stripped[j] == '\'')) {
            stripped = stripped.substr(0, offset) +
                       Trim(stripped.substr(offset, i - offset));
            break;
          }
        }
      }
    }
    operands = stripped;
  }
  operands = ExpandMerlinCharLiterals(operands);
  section.atoms.push_back(
      std::make_shared<InstructionAtom>(directive, operands));
  current_address_ += 1; // Placeholder size
}

// ============================================================================
// Main Parse Function
// ============================================================================

void MerlinSyntaxParser::Parse(const std::string &source, Section &section,
                               ConcreteSymbolTable &symbols) {
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
  current_line_ = 0; // Reset line counter
  in_macro_definition_ = false;
  macro_expansion_depth_ = 0;
  macros_.clear(); // Clear macros from previous parse
  in_lup_block_ = false;
  lup_body_.clear();
  lup_count_ = 0;
  lup_nesting_depth_ = 0;

  // Split into lines and parse
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    current_line_++; // Increment line counter for each line
    ParseLine(line, section, symbols);
  }

  // Validate that all DO blocks are closed (Phase 4: use shared component)
  if (!conditional_.IsBalanced()) {
    throw std::runtime_error(
        FormatError("Unmatched DO directive (missing FIN)"));
  }

  // Validate that macro definitions are closed
  if (in_macro_definition_) {
    throw std::runtime_error(
        FormatError("Unclosed macro definition (missing <<<)"));
  }

  // Validate that LUP blocks are closed
  if (in_lup_block_) {
    throw std::runtime_error(FormatError("Unclosed LUP block (missing --^)"));
  }
}

} // namespace xasm
