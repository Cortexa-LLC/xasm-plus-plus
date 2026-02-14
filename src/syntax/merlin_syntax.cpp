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

  // ; anywhere marks start of inline comment
  size_t comment_pos = line.find(';');
  if (comment_pos != std::string::npos) {
    return line.substr(0, comment_pos);
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
  // Merlin-Specific Features (handle before delegating to ExpressionParser)
  // ========================================================================

  // Check for character literal: "x" or 'x'
  if (!expr.empty() && (expr[0] == '"' || expr[0] == '\'')) {
    // Character literal - extract the character
    if (expr.length() >= 3 && expr[expr.length() - 1] == expr[0]) {
      // Proper quote-enclosed character: "j" or 'j'
      char ch = expr[1];
      return std::make_shared<LiteralExpr>(static_cast<uint8_t>(ch));
    } else if (expr.length() == 1) {
      // Just a quote character (like in SPECIALK.S weird case)
      // Treat as the ASCII value of the quote itself
      return std::make_shared<LiteralExpr>(static_cast<uint8_t>(expr[0]));
    } else {
      // Malformed character literal - treat as 0
      return std::make_shared<LiteralExpr>(0);
    }
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
  // This handles edge cases like "X+" where trailing operator leaves empty right side
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

  // Check for trailing operators (legacy Merlin behavior: treat missing operand as 0)
  // Examples: "X+", "Y-", "Z*"
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

  // All other expressions (arithmetic, symbols, literals) handled by shared parser
  ExpressionParser parser(&symbols);
  return parser.Parse(expr);
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
  symbols.Define(label, SymbolType::Label, expr);
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
    section.atoms.push_back(std::make_shared<SpaceAtom>(count));
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
    throw std::runtime_error(FormatError("DUM directive requires an address operand"));
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
      // Symbol not found - use 0 for now (forward reference issue)
      dum_address_ = 0;
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

  // Try to open the file - first as given, then with /tmp/ prefix
  std::ifstream file(filename);
  std::string actual_filename = filename;

  if (!file.is_open() && filename[0] != '/') {
    // Try with /tmp/ prefix for relative paths
    actual_filename = "/tmp/" + filename;
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

      // Repeat the body count times
      for (int i = 0; i < count; ++i) {
        for (const auto &lup_line : body_copy) {
          ParseLine(lup_line, section, symbols);
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
      // Create label atom and define symbol
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));

      // Update current scope for local labels
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
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

  if (DispatchDirective(directive, label, operands, ctx)) {
    // Directive was handled by registry
    return;
  }

  // Not a directive - check if it's a macro invocation
  std::string upper_directive = ToUpper(directive);
  if (macros_.find(upper_directive) != macros_.end()) {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    // Expand macro
    ExpandMacro(directive, operands, section, symbols);
    return;
  }

  // Assume it's an instruction
  // Create label atom first if label present
  if (!label.empty()) {
    symbols.Define(label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(current_address_));
    section.atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address_));
    current_scope_.global_label = label;
    current_scope_.local_labels.clear();
  }
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
