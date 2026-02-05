// MerlinSyntax - Merlin assembly syntax parser implementation
// Phases 1-3: Foundation, Local Labels, DUM Blocks

#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/cpu/cpu_6502.h"
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

// ============================================================================
// Constructor
// ============================================================================

MerlinSyntaxParser::MerlinSyntaxParser()
    : in_macro_definition_(false), macro_expansion_depth_(0),
      in_dum_block_(false), dum_address_(0), current_address_(0),
      end_directive_seen_(false), current_file_("<stdin>"), current_line_(0),
      cpu_(nullptr) {}

void MerlinSyntaxParser::SetCpu(Cpu6502 *cpu) { cpu_ = cpu; }

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
// Number Parsing
// ============================================================================

// Parse number in various formats: $hex, %binary, decimal
uint32_t MerlinSyntaxParser::ParseNumber(const std::string &str) {
  if (str.empty()) {
    return 0;
  }

  // Hex: $FFFF (may have addressing mode suffix like $200,x)
  if (str[0] == '$') {
    std::string hex_part = str.substr(1);
    if (hex_part.empty()) {
      throw std::runtime_error(
          FormatError("Invalid hex number: '" + str + "' (no digits after $)"));
    }

    // Strip addressing mode suffix (,X ,Y ,S) before parsing
    size_t comma_pos = hex_part.find(',');
    if (comma_pos != std::string::npos) {
      hex_part = hex_part.substr(0, comma_pos);
    }

    // Validate hex digits BEFORE calling stoul
    for (char c : hex_part) {
      if (!std::isxdigit(static_cast<unsigned char>(c))) {
        throw std::runtime_error(FormatError(
            "Invalid hex digit '" + std::string(1, c) + "' in hex number: '" +
            str + "' (hex_part after strip: '" + hex_part + "')"));
      }
    }

    try {
      return std::stoul(hex_part, nullptr, 16);
    } catch (const std::invalid_argument &e) {
      throw std::runtime_error(FormatError("Invalid hex number: '" + str +
                                           "' (hex_part: '" + hex_part +
                                           "') - " + e.what()));
    } catch (const std::out_of_range &e) {
      throw std::runtime_error(
          FormatError("Hex number out of range: '" + str + "' - " + e.what()));
    }
  }

  // Binary: %11110000
  if (str[0] == '%') {
    std::string bin_part = str.substr(1);
    if (bin_part.empty()) {
      throw std::runtime_error(FormatError("Invalid binary number: '" + str +
                                           "' (no digits after %)"));
    }

    // Validate binary digits BEFORE calling stoul
    for (char c : bin_part) {
      if (c != '0' && c != '1') {
        throw std::runtime_error(
            FormatError("Invalid binary digit '" + std::string(1, c) +
                        "' in binary number: '" + str + "'"));
      }
    }

    try {
      return std::stoul(bin_part, nullptr, 2);
    } catch (const std::invalid_argument &e) {
      throw std::runtime_error(FormatError("Invalid binary number: '" + str +
                                           "' (bin_part: '" + bin_part +
                                           "') - " + e.what()));
    } catch (const std::out_of_range &e) {
      throw std::runtime_error(FormatError("Binary number out of range: '" +
                                           str + "' - " + e.what()));
    }
  }

  // Decimal: 42
  // Validate decimal digits BEFORE calling stoul
  for (size_t i = 0; i < str.length(); ++i) {
    char c = str[i];
    // Allow leading - sign
    if (i == 0 && c == '-') {
      continue;
    }
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw std::runtime_error(
          FormatError("Invalid decimal digit '" + std::string(1, c) +
                      "' in decimal number: '" + str + "'"));
    }
  }

  try {
    return std::stoul(str, nullptr, 10);
  } catch (const std::invalid_argument &e) {
    throw std::runtime_error(
        FormatError("Invalid decimal number: '" + str + "' - " + e.what()));
  } catch (const std::out_of_range &e) {
    throw std::runtime_error(FormatError("Decimal number out of range: '" +
                                         str + "' - " + e.what()));
  }
}

// Parse expression (for now, just handle simple numbers and basic operators)
std::shared_ptr<Expression>
MerlinSyntaxParser::ParseExpression(const std::string &str,
                                    ConcreteSymbolTable &symbols) {

  // For now, handle simple cases:
  // - Pure number: $100
  // - Simple addition: $100+$20, VALUE+5
  // - Symbol reference: LABEL
  // - Negative number: -1
  // - Low byte operator: <ADDRESS
  // - High byte operator: >ADDRESS
  // - Multiplication: 24*30

  std::string expr = Trim(str);

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

  // Check for addition/subtraction first (lower precedence than multiplication)
  // We need to handle expressions like BASE+OFFSET*2 properly
  size_t plus_pos = expr.find('+');
  size_t minus_pos =
      expr.find('-', 1); // Skip first char (could be negative sign)

  if (plus_pos != std::string::npos) {
    // Addition: BASE+OFFSET*2
    std::string left = Trim(expr.substr(0, plus_pos));
    std::string right = Trim(expr.substr(plus_pos + 1));

    // Recursively parse left side (might be complex)
    auto left_expr = ParseExpression(left, symbols);
    int64_t left_val = left_expr->Evaluate(symbols);

    // Recursively parse right side (might contain multiplication)
    auto right_expr = ParseExpression(right, symbols);
    int64_t right_val = right_expr->Evaluate(symbols);

    return std::make_shared<LiteralExpr>(left_val + right_val);
  } else if (minus_pos != std::string::npos) {
    // Subtraction: $200-$10 or VALUE-5
    std::string left = Trim(expr.substr(0, minus_pos));
    std::string right = Trim(expr.substr(minus_pos + 1));

    // Recursively parse left side
    auto left_expr = ParseExpression(left, symbols);
    int64_t left_val = left_expr->Evaluate(symbols);

    // Recursively parse right side
    auto right_expr = ParseExpression(right, symbols);
    int64_t right_val = right_expr->Evaluate(symbols);

    return std::make_shared<LiteralExpr>(left_val - right_val);
  }

  // Check for multiplication (higher precedence, parsed after +/-)
  size_t mult_pos = expr.find('*');
  if (mult_pos != std::string::npos) {
    std::string left = Trim(expr.substr(0, mult_pos));
    std::string right = Trim(expr.substr(mult_pos + 1));

    // Parse left side (could be number or symbol)
    int64_t left_val = 0;
    if (symbols.IsDefined(left)) {
      auto sym_expr = std::make_shared<SymbolExpr>(left);
      left_val = sym_expr->Evaluate(symbols);
    } else {
      left_val = ParseNumber(left);
    }

    // Parse right side (could be number or symbol)
    int64_t right_val = 0;
    if (symbols.IsDefined(right)) {
      auto sym_expr = std::make_shared<SymbolExpr>(right);
      right_val = sym_expr->Evaluate(symbols);
    } else {
      right_val = ParseNumber(right);
    }

    return std::make_shared<LiteralExpr>(left_val * right_val);
  }

  // Check for negative number
  if (!expr.empty() && expr[0] == '-') {
    // Negative number: -1, -128
    // Check if rest is all digits (valid negative number)
    bool is_neg_number = true;
    for (size_t i = 1; i < expr.length(); ++i) {
      if (!std::isdigit(static_cast<unsigned char>(expr[i]))) {
        is_neg_number = false;
        break;
      }
    }
    if (is_neg_number && expr.length() > 1) {
      // Valid negative number - use stoll for proper sign extension to int64_t
      int64_t value = std::stoll(expr);
      return std::make_shared<LiteralExpr>(value);
    }
    // Otherwise might be subtraction expression, fall through to operator
    // handling
  } else if (!expr.empty() &&
             (expr[0] == '$' || expr[0] == '%' || std::isdigit(expr[0]))) {
    // Pure number
    return std::make_shared<LiteralExpr>(ParseNumber(expr));
  } else if (!expr.empty() && symbols.IsDefined(expr)) {
    // Symbol reference
    return std::make_shared<SymbolExpr>(expr);
  }

  // Unknown or empty - return literal 0 for now
  return std::make_shared<LiteralExpr>(0);
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

void MerlinSyntaxParser::HandleOrg(const std::string &operand, Section &section,
                                   ConcreteSymbolTable &symbols) {
  std::string op = Trim(operand);
  uint32_t address = 0;

  // Check if operand is empty
  if (op.empty()) {
    throw std::runtime_error(
        FormatError("ORG directive requires an address operand"));
  }

  // Check if operand is a symbol or a number
  if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
    // It's a number
    address = ParseNumber(op);
  } else {
    // It's a symbol - look it up
    int64_t value = 0;
    if (symbols.Lookup(op, value)) {
      address = static_cast<uint32_t>(value);
    } else {
      // Symbol not found - this should throw an error in proper assembly
      address = 0;
    }
  }

  section.atoms.push_back(std::make_shared<OrgAtom>(address));
  current_address_ = address;
}

void MerlinSyntaxParser::HandleEqu(const std::string &label,
                                   const std::string &operand,
                                   ConcreteSymbolTable &symbols) {
  auto expr = ParseExpression(operand, symbols);
  symbols.Define(label, SymbolType::Label, expr);
  // EQU doesn't create atoms
}

void MerlinSyntaxParser::HandleDB(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /* symbols */) {
  // Store expression strings for multi-pass evaluation
  std::vector<std::string> expressions;
  std::istringstream iss(operand);
  std::string value;

  while (std::getline(iss, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      expressions.push_back(value);
    }
  }

  // Create DataAtom with expressions (will be evaluated during assembly)
  auto atom = std::make_shared<DataAtom>(expressions, DataSize::Byte);
  section.atoms.push_back(atom);

  // Update address (1 byte per expression)
  current_address_ += expressions.size();
}

void MerlinSyntaxParser::HandleDW(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /* symbols */) {
  // Store expression strings for multi-pass evaluation
  std::vector<std::string> expressions;
  std::istringstream iss(operand);
  std::string value;

  while (std::getline(iss, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      expressions.push_back(value);
    }
  }

  // Create DataAtom with expressions (will be evaluated during assembly)
  auto atom = std::make_shared<DataAtom>(expressions, DataSize::Word);
  section.atoms.push_back(atom);

  // Update address (2 bytes per word expression)
  current_address_ += expressions.size() * 2;
}

void MerlinSyntaxParser::HandleHex(const std::string &operand,
                                   Section &section) {
  std::vector<uint8_t> bytes;
  std::string hex_str = Trim(operand);

  // Check if operand contains commas (comma-separated format)
  if (hex_str.find(',') != std::string::npos) {
    // Comma-separated format: "01,02,03" or "01, 02, 03"
    std::istringstream iss(hex_str);
    std::string token;

    while (std::getline(iss, token, ',')) {
      token = Trim(token); // Remove whitespace around token
      if (!token.empty()) {
        // Validate hex digits before calling stoul
        for (char c : token) {
          if (!std::isxdigit(static_cast<unsigned char>(c))) {
            throw std::runtime_error(
                FormatError("Invalid hex digit '" + std::string(1, c) +
                            "' in HEX directive: '" + token + "'"));
          }
        }
        try {
          bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
        } catch (const std::exception &e) {
          throw std::runtime_error(
              FormatError("Invalid hex value in HEX directive: '" + token +
                          "' - " + e.what()));
        }
      }
    }
  } else {
    // Concatenated format: "010203" or "AB CD EF"
    // Remove all spaces from hex string
    hex_str.erase(std::remove_if(hex_str.begin(), hex_str.end(), ::isspace),
                  hex_str.end());

    // Validate all characters are hex digits
    for (char c : hex_str) {
      if (!std::isxdigit(static_cast<unsigned char>(c))) {
        throw std::runtime_error(
            FormatError("Invalid hex digit '" + std::string(1, c) +
                        "' in HEX directive: '" + operand + "'"));
      }
    }

    // Parse pairs of hex digits
    for (size_t i = 0; i + 1 < hex_str.length(); i += 2) {
      std::string byte_str = hex_str.substr(i, 2);
      try {
        bytes.push_back(
            static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
      } catch (const std::exception &e) {
        throw std::runtime_error(
            FormatError("Invalid hex value in HEX directive: '" + byte_str +
                        "' - " + e.what()));
      }
    }
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleDS(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  // DS (Define Space) - reserve bytes
  // Can be a number, symbol, or expression
  std::string op = Trim(operand);

  // Substitute * (program counter) with current address BEFORE parsing
  // In Merlin syntax, * represents the current program counter
  // BUT: Don't substitute when * is multiplication (between two operands)
  if (op.find('*') != std::string::npos) {
    std::ostringstream hex_stream;
    hex_stream << "$" << std::hex
               << (in_dum_block_ ? dum_address_ : current_address_);
    std::string pc_hex = hex_stream.str();

    // Replace * with address, but only when it's program counter (not
    // multiplication)
    size_t pos = 0;
    while ((pos = op.find('*', pos)) != std::string::npos) {
      // Check if * is multiplication by examining characters before and after
      bool is_multiplication = false;

      // Check character before *
      bool has_operand_before = false;
      if (pos > 0) {
        char before = op[pos - 1];
        // Operand before: digit, letter, or closing paren
        if (std::isalnum(static_cast<unsigned char>(before)) || before == ')') {
          has_operand_before = true;
        }
      }

      // Check character after *
      bool has_operand_after = false;
      if (pos + 1 < op.length()) {
        char after = op[pos + 1];
        // Operand after: digit, letter, or opening paren
        if (std::isalnum(static_cast<unsigned char>(after)) || after == '(' ||
            after == '$' || after == '%') {
          has_operand_after = true;
        }
      }

      // It's multiplication if there are operands on BOTH sides
      is_multiplication = has_operand_before && has_operand_after;

      if (is_multiplication) {
        // Skip this *, it's multiplication
        pos++;
      } else {
        // This is program counter, substitute it
        op.replace(pos, 1, pc_hex);
        pos += pc_hex.length();
      }
    }
  }

  // Use ParseExpression to handle all cases (numbers, symbols, arithmetic)
  uint32_t count = 0;
  if (!op.empty()) {
    auto expr = ParseExpression(op, symbols);
    int64_t value = expr->Evaluate(symbols);
    if (value < 0) {
      throw std::runtime_error(FormatError("DS: Negative count not allowed: " +
                                           std::to_string(value)));
    }
    count = static_cast<uint32_t>(value);
  }

  // Only emit SpaceAtom if NOT in DUM block
  // DUM blocks define symbols without generating output (vasm compatibility)
  if (!in_dum_block_) {
    section.atoms.push_back(std::make_shared<SpaceAtom>(count));
    current_address_ += count;
  } else {
    // In DUM block: advance address counter but don't emit bytes
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

  // Check if operand is a symbol or a number
  if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
    // It's a number
    dum_address_ = ParseNumber(op);
  } else {
    // It's a symbol - look it up
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
  // TODO: Phase 3 implementation
  in_dum_block_ = false;
  variable_labels_.clear();
}

void MerlinSyntaxParser::HandlePut(const std::string &operand, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // PUT filename - include another source file
  std::string filename = Trim(operand);

  // Auto-append .S extension if no extension present
  // Check if filename contains a dot (has extension)
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
    throw std::runtime_error(
        FormatError("Cannot open file: " +
                    Trim(operand))); // Report original filename in error
  }

  // Read entire file content
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string file_content = buffer.str();
  file.close();

  // Parse the included file content
  std::istringstream iss(file_content);
  std::string line;
  while (std::getline(iss, line)) {
    ParseLine(line, section, symbols);
  }

  // Remove from include stack when done
  include_stack_.pop_back();
}

void MerlinSyntaxParser::HandleDo(const std::string &operand,
                                  ConcreteSymbolTable &symbols) {
  // DO condition - start conditional assembly block
  auto expr = ParseExpression(operand, symbols);
  int64_t condition = expr->Evaluate(symbols);

  // Determine if code should be emitted based on parent blocks and current
  // condition
  bool parent_should_emit =
      conditional_stack_.empty() ? true : conditional_stack_.back().should_emit;
  bool should_emit = parent_should_emit && (condition != 0);

  conditional_stack_.push_back({
      condition != 0, // condition
      false,          // in_else_block
      should_emit     // should_emit
  });
}

void MerlinSyntaxParser::HandleElse() {
  // ELSE - switch to alternative branch in conditional block
  if (conditional_stack_.empty()) {
    throw std::runtime_error(FormatError("ELSE without matching DO"));
  }

  ConditionalBlock &block = conditional_stack_.back();
  if (block.in_else_block) {
    throw std::runtime_error(FormatError("Multiple ELSE in same DO block"));
  }

  block.in_else_block = true;

  // Determine if code in ELSE branch should be emitted
  bool parent_should_emit =
      conditional_stack_.size() > 1
          ? conditional_stack_[conditional_stack_.size() - 2].should_emit
          : true;
  block.should_emit = parent_should_emit && !block.condition;
}

void MerlinSyntaxParser::HandleFin() {
  // FIN - end conditional assembly block
  if (conditional_stack_.empty()) {
    // Merlin quirk: FIN without DO is a warning, not an error
    // This is needed for Prince of Persia source (SPECIALK.S has extra FINs)
    // vasm treats this as WARNING and continues processing
    // We'll do the same - just ignore the extra FIN
    std::cerr
        << "Warning: "
        << FormatError(
               "FIN without matching DO (ignored for Merlin compatibility)")
        << std::endl;
    return;
  }

  conditional_stack_.pop_back();
}

void MerlinSyntaxParser::HandleEnd() {
  // END - mark end of source (stop processing further lines)
  // This is a no-op directive - doesn't generate atoms
  // Just sets flag to stop processing
  end_directive_seen_ = true;
}

// ============================================================================
// Macro Directives
// ============================================================================

void MerlinSyntaxParser::HandlePMC(const std::string &operand) {
  // PMC MacroName - Start macro definition
  std::string macro_name = ToUpper(Trim(operand));

  if (macro_name.empty()) {
    throw std::runtime_error(FormatError("PMC requires macro name"));
  }

  if (in_macro_definition_) {
    throw std::runtime_error(
        FormatError("Nested macro definitions not allowed"));
  }

  // Start macro definition
  in_macro_definition_ = true;
  current_macro_.name = macro_name;
  current_macro_.body.clear();
  current_macro_.param_count = 0;
}

void MerlinSyntaxParser::HandleEOM() {
  // EOM - End macro definition
  if (!in_macro_definition_) {
    throw std::runtime_error(FormatError("EOM without PMC"));
  }

  // Scan macro body to count parameters
  int max_param = 0;
  for (const auto &line : current_macro_.body) {
    // Look for ]1, ]2, etc.
    for (size_t i = 0; i < line.length(); ++i) {
      if (line[i] == ']' && i + 1 < line.length() &&
          std::isdigit(line[i + 1])) {
        int param_num = line[i + 1] - '0';
        max_param = std::max(max_param, param_num);
      }
    }
  }
  current_macro_.param_count = max_param;

  // Store macro definition
  macros_[current_macro_.name] = current_macro_;

  // End macro definition mode
  in_macro_definition_ = false;
  current_macro_ = MacroDefinition();
}

void MerlinSyntaxParser::HandleMAC(const std::string &macro_name,
                                   const std::string &params, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // MAC MacroName - Invoke/expand macro (Merlin syntax)
  // MAC is ALWAYS invocation, PMC is ALWAYS definition
  std::string upper_name = ToUpper(Trim(macro_name));

  if (upper_name.empty()) {
    throw std::runtime_error(FormatError("MAC requires macro name"));
  }

  // Look up macro - if not found, throw error
  if (macros_.find(upper_name) == macros_.end()) {
    throw std::runtime_error(FormatError("Undefined macro: " + macro_name));
  }

  // Expand the macro
  ExpandMacro(macro_name, params, section, symbols);
}

void MerlinSyntaxParser::HandleMacroEnd() {
  // <<< - End macro definition (Merlin syntax)
  if (!in_macro_definition_) {
    throw std::runtime_error(FormatError("<<< without MAC"));
  }

  // Scan macro body to count parameters
  int max_param = 0;
  for (const auto &line : current_macro_.body) {
    // Look for ]1, ]2, etc.
    for (size_t i = 0; i < line.length(); ++i) {
      if (line[i] == ']' && i + 1 < line.length() &&
          std::isdigit(line[i + 1])) {
        int param_num = line[i + 1] - '0';
        max_param = std::max(max_param, param_num);
      }
    }
  }
  current_macro_.param_count = max_param;

  // Store macro definition
  macros_[current_macro_.name] = current_macro_;

  // End macro definition mode
  in_macro_definition_ = false;
  current_macro_ = MacroDefinition();
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

void MerlinSyntaxParser::HandleLst(const std::string &operand) {
  // LST/LST OFF - listing control directives
  // These control assembler output, not code generation
  // No atoms created
  (void)operand; // Unused parameter
}

void MerlinSyntaxParser::HandleLstdo() {
  // LSTDO - list during DO blocks
  // Listing control directive
  // No atoms created
}

void MerlinSyntaxParser::HandleTr(const std::string &operand) {
  // TR [ADR|ON|OFF] - truncate listing
  // Listing control directive
  // No atoms created
  (void)operand; // Unused parameter
}

void MerlinSyntaxParser::HandleAsc(const std::string &operand,
                                   Section &section) {
  // ASC 'string' or ASC "string" - ASCII string directive
  // Apple II/Merlin standard: Sets high bit on ALL characters (0x80 | char)
  // This produces "high-bit ASCII" for Apple II text display

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes with high bit set (Apple II standard)
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on ALL characters (Apple II/Merlin compatibility)
    byte |= 0x80;

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleDA(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  // DA (Define Address) - same as DW, word definitions in little-endian
  HandleDW(operand, section, symbols);
}

void MerlinSyntaxParser::HandleDCI(const std::string &operand,
                                   Section &section) {
  // DCI 'string' - DCI string (last character with high bit set)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, setting high bit on last character
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on last character
    if (i == text.length() - 1) {
      byte |= 0x80;
    }

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleINV(const std::string &operand,
                                   Section &section) {
  // INV 'string' - Inverse ASCII (all characters with high bit set)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, setting high bit on all characters
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]) | 0x80;
    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleFLS(const std::string &operand,
                                   Section &section) {
  // FLS 'string' - Flash ASCII (alternating high bit for flashing effect)
  // On Apple II, "flash" text alternates between normal and inverse characters
  // This directive sets high bit on every OTHER character (even indices: 0,
  // 2, 4...)

  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    // No quote found - empty string
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return;
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  // Convert string to bytes, alternating high bit on every other character
  for (size_t i = 0; i < text.length(); ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);

    // Set high bit on ODD-indexed characters (1, 3, 5...)
    if (i % 2 == 1) {
      byte |= 0x80;
    }

    bytes.push_back(byte);
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleSav(const std::string & /* operand */) {
  // SAV filename - Save output filename directive
  // This is a no-op for now - output filename is controlled by command-line
  // args Used in BOOT.S: "sav boot" No atoms created, no state changed
}

void MerlinSyntaxParser::HandleXc(const std::string &operand) {
  // XC [ON|OFF] - Toggle 65C02 CPU instruction set
  // xc or xc on  = enable 65C02 mode
  // xc off       = disable 65C02 mode (back to 6502)

  if (!cpu_) {
    // No CPU set - silently ignore (for tests that don't need CPU)
    return;
  }

  std::string op = ToUpper(Trim(operand));

  if (op.empty() || op == "ON") {
    // Enable 65C02 mode
    cpu_->SetCpuMode(CpuMode::Cpu65C02);
  } else if (op == "OFF") {
    // Disable 65C02 mode (back to 6502)
    cpu_->SetCpuMode(CpuMode::Cpu6502);
  } else {
    throw std::runtime_error(
        FormatError("XC: invalid operand (expected ON or OFF)"));
  }

  // No atoms created
}

void MerlinSyntaxParser::HandleMx(const std::string &operand) {
  // MX mode - Set 65816 accumulator and index register widths
  // mx %00 or mx 0 = 16-bit A, 16-bit X/Y (native mode)
  // mx %01 or mx 1 = 16-bit A, 8-bit X/Y
  // mx %10 or mx 2 = 8-bit A, 16-bit X/Y
  // mx %11 or mx 3 = 8-bit A, 8-bit X/Y (emulation mode)
  // This is a directive only - tracks state but doesn't change CPU encoding
  // (Actual 65816 instruction encoding changes based on MX are out of scope)

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

  // Store mode (no atoms created, just state tracking)
  // In a full implementation, this would affect how 65816 instructions are
  // encoded For now, just validate and accept the directive
  (void)mode; // Suppress unused variable warning
}

void MerlinSyntaxParser::HandleRev(const std::string &label,
                                   const std::string &operand, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // REV "string" - Reverse ASCII string
  // Used in SPECIALK.S: C_skip rev "SKIP"
  // Reverses the string bytes and emits them to the output
  // Creates a label at the start of the reversed string

  std::string op = Trim(operand);

  if (op.empty()) {
    throw std::runtime_error(FormatError("REV requires a string operand"));
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    throw std::runtime_error(FormatError("REV requires quoted string"));
  }

  // Find closing quote
  size_t end_pos = op.find(quote, start_pos);
  if (end_pos == std::string::npos) {
    // No closing quote - treat rest as string
    end_pos = op.length();
  }

  // Extract string content
  std::string text = op.substr(start_pos, end_pos - start_pos);

  if (text.empty()) {
    throw std::runtime_error(FormatError("REV requires non-empty string"));
  }

  // Create label at current address (before emitting bytes)
  if (!label.empty()) {
    symbols.Define(label, SymbolType::Label,
                   std::make_shared<LiteralExpr>(current_address_));
    section.atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address_));
    current_scope_.global_label = label;
    current_scope_.local_labels.clear();
  }

  // Reverse the string
  std::string reversed(text.rbegin(), text.rend());

  // Emit reversed bytes as data
  std::vector<uint8_t> bytes;
  for (char ch : reversed) {
    bytes.push_back(static_cast<uint8_t>(ch));
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleLup(const std::string & /* operand */) {
  // LUP count - Loop directive (repeat following code count times)
  // This is deferred for now - requires complex loop expansion
  // Used in TABLES.S: "lup 36"
  throw std::runtime_error(
      FormatError("LUP directive not yet implemented (deferred)"));
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
    if (upper_trimmed == "EOM") {
      HandleEOM();
      return;
    }
    // Check for <<< to end macro definition (Merlin style)
    if (upper_trimmed == "<<<") {
      HandleMacroEnd();
      return;
    }
    // Otherwise, add line to macro body
    current_macro_.body.push_back(code_line);
    return;
  }

  // Check for conditional assembly directives (DO/ELSE/FIN)
  // These must be processed even when inside a false conditional block
  if (upper_trimmed.find("DO ") == 0 || upper_trimmed == "DO") {
    // Extract operand after "DO"
    std::string operand = trimmed.length() > 3 ? Trim(trimmed.substr(3)) : "0";
    HandleDo(operand, symbols);
    return;
  } else if (upper_trimmed == "ELSE") {
    HandleElse();
    return;
  } else if (upper_trimmed == "FIN") {
    HandleFin();
    return;
  }

  // Check if we should skip this line due to conditional assembly
  if (!conditional_stack_.empty() && !conditional_stack_.back().should_emit) {
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

  // Handle directives
  if (directive == "ORG") {
    HandleOrg(operands, section, symbols);
    return;
  } else if (directive == "EQU") {
    // EQU: Define symbol but don't create label atom
    HandleEqu(label, operands, symbols);
    return;
  } else if (directive == "DB" || directive == "DFB") {
    // DB/DFB (Define Byte / Define Font Byte) - emit byte data
    // DFB is Merlin's name for DB (used in Prince of Persia source)
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDB(operands, section, symbols);
    return;
  } else if (directive == "DW") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDW(operands, section, symbols);
    return;
  } else if (directive == "HEX") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleHex(operands, section);
    return;
  } else if (directive == "DS") {
    // Create label atom first if label present
    if (!label.empty()) {
      // Use dum_address_ if in DUM block, otherwise current_address_
      uint32_t label_address = in_dum_block_ ? dum_address_ : current_address_;
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(label_address));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, label_address));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDS(operands, section, symbols);
    return;
  } else if (directive == "DUM") {
    HandleDum(operands, symbols);
    return;
  } else if (directive == "DEND") {
    HandleDend();
    return;
  } else if (directive == "PUT") {
    HandlePut(operands, section, symbols);
    return;
  } else if (directive == "DO") {
    HandleDo(operands, symbols);
    return;
  } else if (directive == "ELSE") {
    HandleElse();
    return;
  } else if (directive == "FIN") {
    HandleFin();
    return;
  } else if (directive == "LST") {
    HandleLst(operands);
    return;
  } else if (directive == "LSTDO") {
    HandleLstdo();
    return;
  } else if (directive == "TR") {
    HandleTr(operands);
    return;
  } else if (directive == "ASC") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleAsc(operands, section);
    return;
  } else if (directive == "DA") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDA(operands, section, symbols);
    return;
  } else if (directive == "DCI") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDCI(operands, section);
    return;
  } else if (directive == "INV") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleINV(operands, section);
    return;
  } else if (directive == "FLS") {
    // Create label atom first if label present
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleFLS(operands, section);
    return;
  } else if (directive == "PMC") {
    // PMC - Start macro definition
    HandlePMC(label.empty() ? operands : label);
    return;
  } else if (directive == "EOM") {
    // EOM - End macro definition
    HandleEOM();
    return;
  } else if (directive == "MAC") {
    // MAC MacroName - Can be either:
    // 1. Macro invocation (if macro already exists):
    //    Format: MAC MacroName;param1;param2;... OR  MacroName;param1;param2
    // 2. Macro definition (if macro doesn't exist) - Merlin style, terminated
    // with <<<
    //    Format: MacroName MAC param1;param2  (label is macro name)

    std::string macro_name;
    std::string params_str;

    // Check if label is present - if so, it's the macro name (Merlin
    // label-based syntax)
    if (!label.empty()) {
      // Label-based definition: stlx mac bank;addr
      // Label = "stlx" (macro name)
      // Operands = "bank;addr" (parameter names)
      macro_name = ToUpper(label);
      params_str =
          operands; // These are parameter names for definition, not values
    } else {
      // No label - macro name must be in operands
      // Parse macro name and parameters from operands
      // Parameters are separated by semicolons: MAC LoadValue;#$42
      size_t semicolon_pos = operands.find(';');

      if (semicolon_pos != std::string::npos) {
        macro_name = ToUpper(Trim(operands.substr(0, semicolon_pos)));
        params_str = Trim(operands.substr(semicolon_pos + 1));
      } else {
        macro_name = ToUpper(Trim(operands));
        params_str = "";
      }
    }

    if (macro_name.empty()) {
      throw std::runtime_error(FormatError("MAC requires macro name"));
    }

    // Check if this macro already exists (was defined with PMC/EOM or previous
    // MAC)
    if (macros_.find(macro_name) != macros_.end()) {
      // Macro exists - this is an INVOCATION
      // (No label should be present for invocation - it was already handled
      // above) Expand the macro with parameters
      ExpandMacro(macro_name, params_str, section, symbols);
      return;
    }

    // Macro doesn't exist - this is a DEFINITION (Merlin style)
    if (in_macro_definition_) {
      throw std::runtime_error(
          FormatError("Nested macro definitions not allowed"));
    }

    // Start macro definition (similar to PMC, but ended with <<< instead of
    // EOM)
    in_macro_definition_ = true;
    current_macro_.name = macro_name;
    current_macro_.body.clear();
    current_macro_.param_count = 0;
    return;
  } else if (directive == "USR") {
    // USR directive - user-defined subroutine (no-op)
    // USR doesn't generate bytes - it's for calling user-defined routines
    // during assembly The actual routine is provided by the assembler
    // environment, not the source code Create label atom if present (label
    // definition still happens)
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    // USR is a no-op - no atoms generated
    return;
  } else if (directive == "END") {
    // END - mark end of source (no more processing)
    // Create label atom if present (label definition still happens)
    if (!label.empty()) {
      symbols.Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      section.atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleEnd();
    return;
  } else if (directive == "SAV") {
    // SAV filename - Save output filename (no-op)
    HandleSav(operands);
    return;
  } else if (directive == "XC") {
    // XC [ON|OFF] - Toggle 65C02 CPU mode (no-op)
    HandleXc(operands);
    return;
  } else if (directive == "MX") {
    // MX mode - Set 65816 register widths (state tracking only)
    HandleMx(operands);
    return;
  } else if (directive == "REV") {
    // REV "string" - Reverse ASCII string
    if (label.empty()) {
      throw std::runtime_error(FormatError("REV requires a label"));
    }
    HandleRev(label, operands, section, symbols);
    return;
  } else if (directive == "LUP") {
    // LUP count - Loop directive (not implemented yet)
    HandleLup(operands);
    return;
  } else {
    // Check if it's a macro invocation
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

  // Split into lines and parse
  std::istringstream iss(source);
  std::string line;

  while (std::getline(iss, line)) {
    current_line_++; // Increment line counter for each line
    ParseLine(line, section, symbols);
  }

  // Validate that all DO blocks are closed
  if (!conditional_stack_.empty()) {
    throw std::runtime_error(
        FormatError("Unmatched DO directive (missing FIN)"));
  }

  // Validate that macro definitions are closed
  if (in_macro_definition_) {
    throw std::runtime_error(
        FormatError("Unclosed macro definition (missing <<<)"));
  }
}

} // namespace xasm
