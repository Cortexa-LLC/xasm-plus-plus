// MerlinSyntax - Merlin assembly syntax parser implementation
// Phases 1-3: Foundation, Local Labels, DUM Blocks

#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/util/string_utils.h"
#include "xasm++/directives/merlin_directives.h"
#include "xasm++/directives/directive_constants.h"
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

// ============================================================================
// Constructor
// ============================================================================

MerlinSyntaxParser::MerlinSyntaxParser()
    : in_macro_definition_(false), macro_expansion_depth_(0),
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
  // ORG - Set origin address
  directive_registry_[ORG] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    HandleOrg(operand, *ctx.section, *ctx.symbols);
  };

  // EQU - Define constant (label is handled separately)
  directive_registry_[EQU] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)ctx.section;
    HandleEqu(label, operand, *ctx.symbols);
  };

  // DB/DFB - Define byte data
  auto handle_db_with_label = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    // Create label atom first if label present
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDB(operand, *ctx.section, *ctx.symbols);
  };
  directive_registry_[DB] = handle_db_with_label;
  directive_registry_[DFB] = handle_db_with_label;

  // DW - Define word data
  directive_registry_[DW] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    // Create label atom first if label present
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDW(operand, *ctx.section, *ctx.symbols);
  };

  // HEX - Define hex bytes
  directive_registry_[HEX] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    // Create label atom first if label present
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleHex(operand, *ctx.section);
  };

  // DS - Define space
  directive_registry_[DS] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    // Create label atom first if label present
    if (!label.empty()) {
      // Use dum_address_ if in DUM block, otherwise current_address_
      uint32_t label_address = in_dum_block_ ? dum_address_ : current_address_;
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(label_address));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, label_address));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDS(operand, *ctx.section, *ctx.symbols);
  };

  // DUM/DEND - Dummy section
  directive_registry_[DUM] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    HandleDumDirective(operand, *ctx.symbols, in_dum_block_, dum_address_, &ctx);
  };
  directive_registry_[DEND] = [this](const std::string &label,
                                        const std::string &operand,
                                        DirectiveContext &ctx) {
    (void)label;
    (void)operand;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleDend();
  };

  // PUT - Include file
  directive_registry_[PUT] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    HandlePut(operand, *ctx.section, *ctx.symbols);
  };

  // Conditional assembly
  directive_registry_[DO] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    HandleDo(operand, *ctx.symbols);
  };
  directive_registry_[ELSE] = [this](const std::string &label,
                                        const std::string &operand,
                                        DirectiveContext &ctx) {
    (void)label;
    (void)operand;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleElse();
  };
  directive_registry_[FIN] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    (void)operand;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleFin();
  };

  // Listing control
  directive_registry_[LST] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleLst(operand);
  };
  directive_registry_[LSTDO] = [this](const std::string &label,
                                         const std::string &operand,
                                         DirectiveContext &ctx) {
    (void)label;
    (void)operand;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleLstdo();
  };

  // TR - Truncate listing
  directive_registry_[TR] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleTr(operand);
  };

  // String directives (with label support)
  auto handle_string_with_label = [this](
      const std::string &label, const std::string &operand,
      DirectiveContext &ctx,
      void (MerlinSyntaxParser::*handler)(const std::string &, Section &)) {
    // Create label atom first if label present
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    (this->*handler)(operand, *ctx.section);
  };

  directive_registry_[ASC] = [handle_string_with_label](
                                    const std::string &label,
                                    const std::string &operand,
                                    DirectiveContext &ctx) {
    handle_string_with_label(label, operand, ctx,
                             &MerlinSyntaxParser::HandleAsc);
  };

  directive_registry_[DCI] = [handle_string_with_label](
                                    const std::string &label,
                                    const std::string &operand,
                                    DirectiveContext &ctx) {
    handle_string_with_label(label, operand, ctx,
                             &MerlinSyntaxParser::HandleDCI);
  };

  directive_registry_[INV] = [handle_string_with_label](
                                    const std::string &label,
                                    const std::string &operand,
                                    DirectiveContext &ctx) {
    handle_string_with_label(label, operand, ctx,
                             &MerlinSyntaxParser::HandleINV);
  };

  directive_registry_[FLS] = [handle_string_with_label](
                                    const std::string &label,
                                    const std::string &operand,
                                    DirectiveContext &ctx) {
    handle_string_with_label(label, operand, ctx,
                             &MerlinSyntaxParser::HandleFLS);
  };

  // DA - Define address
  directive_registry_[DA] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    // Create label atom first if label present
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleDA(operand, *ctx.section, *ctx.symbols);
  };

  // Macro directives
  directive_registry_[PMC] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)ctx.section;
    (void)ctx.symbols;
    HandlePMC(label.empty() ? operand : label);
  };

  directive_registry_[EOM] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    (void)operand;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleEOM();
  };

  // MAC - Macro definition or invocation (complex logic)
  directive_registry_[MAC] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    std::string macro_name;
    std::string params_str;

    // Check if label is present - if so, it's the macro name
    if (!label.empty()) {
      macro_name = ToUpper(label);
      params_str = operand;
    } else {
      // Parse macro name and parameters from operand
      size_t semicolon_pos = operand.find(';');
      if (semicolon_pos != std::string::npos) {
        macro_name = ToUpper(Trim(operand.substr(0, semicolon_pos)));
        params_str = Trim(operand.substr(semicolon_pos + 1));
      } else {
        macro_name = ToUpper(Trim(operand));
        params_str = "";
      }
    }

    if (macro_name.empty()) {
      throw std::runtime_error(FormatError("MAC requires macro name"));
    }

    // Check if macro exists (invocation) or is being defined
    if (macros_.find(macro_name) != macros_.end()) {
      // Macro exists - invoke it
      ExpandMacro(macro_name, params_str, *ctx.section, *ctx.symbols);
    } else {
      // Macro doesn't exist - start definition
      if (in_macro_definition_) {
        throw std::runtime_error(
            FormatError("Nested macro definitions not allowed"));
      }
      in_macro_definition_ = true;
      current_macro_.name = macro_name;
      current_macro_.body.clear();
      current_macro_.param_count = 0;
    }
  };

  // USR - User-defined subroutine (no-op with optional label)
  directive_registry_[USR] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)operand;
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    // USR is a no-op - no atoms generated
  };

  // END - End of source
  directive_registry_[END] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)operand;
    if (!label.empty()) {
      ctx.symbols->Define(label, SymbolType::Label,
                     std::make_shared<LiteralExpr>(current_address_));
      ctx.section->atoms.push_back(
          std::make_shared<LabelAtom>(label, current_address_));
      current_scope_.global_label = label;
      current_scope_.local_labels.clear();
    }
    HandleEnd();
  };

  // SAV - Save output filename
  directive_registry_[SAV] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleSav(operand);
  };

  // XC - Toggle 65C02 mode
  directive_registry_[XC] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleXc(operand);
  };

  // MX - Set 65816 register widths
  directive_registry_[MX] = [this](const std::string &label,
                                      const std::string &operand,
                                      DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleMx(operand);
  };

  // REV - Reverse ASCII string (requires label)
  directive_registry_[REV] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    if (label.empty()) {
      throw std::runtime_error(FormatError("REV requires a label"));
    }
    HandleRev(label, operand, *ctx.section, *ctx.symbols);
  };

  // LUP - Loop assembly
  directive_registry_[LUP] = [this](const std::string &label,
                                       const std::string &operand,
                                       DirectiveContext &ctx) {
    (void)label;
    (void)ctx.section;
    (void)ctx.symbols;
    HandleLup(operand);
  };
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
  } else if (!expr.empty()) {
    // Symbol reference (defined or not - let Evaluate() throw if undefined)
    return std::make_shared<SymbolExpr>(expr);
  }

  // Empty expression - return literal 0
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
  // ORG directive - set assembly origin address
  std::string op = Trim(operand);
  uint32_t address = 0;

  if (op.empty()) {
    throw std::runtime_error(FormatError("ORG directive requires an address operand"));
  }

  // Parse address (number or symbol)
  if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
    // Numeric literal
    address = ParseNumber(op);
  } else {
    // Symbol - look it up
    int64_t value = 0;
    if (symbols.Lookup(op, value)) {
      address = static_cast<uint32_t>(value);
    } else {
      // Undefined symbol - use 0 (forward reference issue)
      address = 0;
    }
  }

  section.atoms.push_back(std::make_shared<OrgAtom>(address));
  current_address_ = address;
}

void MerlinSyntaxParser::HandleEqu(const std::string &label,
                                   const std::string &operand,
                                   ConcreteSymbolTable &symbols) {
  // EQU directive - define symbolic constant (no code generated)
  auto expr = ParseExpression(operand, symbols);
  symbols.Define(label, SymbolType::Label, expr);
}

void MerlinSyntaxParser::HandleDB(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  // DB directive - define byte(s)
  // Parse comma-separated expressions and evaluate immediately
  std::vector<uint8_t> bytes;
  std::istringstream iss(operand);
  std::string value;

  while (std::getline(iss, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      auto expr = ParseExpression(value, symbols);
      int64_t result = expr->Evaluate(symbols);
      bytes.push_back(static_cast<uint8_t>(result & 0xFF));
    }
  }

  section.atoms.push_back(std::make_shared<DataAtom>(bytes));
  current_address_ += bytes.size();
}

void MerlinSyntaxParser::HandleDW(const std::string &operand, Section &section,
                                  ConcreteSymbolTable & /*symbols*/) {
  // DW directive - define word(s) (16-bit values)
  // Store expressions for multi-pass evaluation (support forward references)
  std::vector<std::string> expressions;
  std::istringstream iss(operand);
  std::string value;

  while (std::getline(iss, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      expressions.push_back(value);
    }
  }

  // Create DataAtom with expressions (assembler will evaluate in multi-pass)
  auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Word);
  section.atoms.push_back(data_atom);
  
  // Reserve space (2 bytes per word expression)
  current_address_ += expressions.size() * 2;
}

void MerlinSyntaxParser::HandleHex(const std::string &operand,
                                   Section &section) {
  HandleHexDirective(operand, section, current_address_);
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
        throw std::runtime_error(FormatError("DS: Negative count not allowed: " +
                                 std::to_string(value)));
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
        if (second_colon != std::string::npos && second_colon > first_colon + 1) {
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
  HandleDumDirective(operand, symbols, in_dum_block_, dum_address_, nullptr);
}

void MerlinSyntaxParser::HandleDend() {
  HandleDendDirective(in_dum_block_);
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
      throw std::runtime_error(FormatError("Circular include detected: " + filename));
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
    value = std::stoul(op.substr(1), nullptr, 16);
  } else if (op[0] == '%') {
    // Binary literal
    value = std::stoul(op.substr(1), nullptr, 2);
  } else if (std::isdigit(op[0])) {
    // Decimal literal
    value = std::stoul(op, nullptr, 10);
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
  HandleEndDirective(end_directive_seen_);
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
  current_macro_.name = ToUpper(Trim(operand));  // Normalize macro name
  current_macro_.body.clear();
  current_macro_.param_count = 0;
}

void MerlinSyntaxParser::HandleEOM() {
  // EOM - End macro definition
  if (!in_macro_definition_) {
    throw std::runtime_error(
        FormatError("EOM without matching PMC"));
  }
  macros_[current_macro_.name] = current_macro_;
  in_macro_definition_ = false;
}

void MerlinSyntaxParser::HandleMAC(const std::string &macro_name,
                                   const std::string &params, Section &section,
                                   ConcreteSymbolTable &symbols) {
  // MAC - Invoke macro (expand and execute)
  ExpandMacro(macro_name, params, section, symbols);
}

void MerlinSyntaxParser::HandleMacroEnd() {
  // <<< - End macro definition (Merlin style)
  if (!in_macro_definition_) {
    throw std::runtime_error(
        FormatError("<<< without matching PMC"));
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

void MerlinSyntaxParser::HandleLst(const std::string &operand) {
  HandleLstDirective(operand);
}

void MerlinSyntaxParser::HandleLstdo() {
  HandleLstdoDirective();
}

void MerlinSyntaxParser::HandleTr(const std::string &operand) {
  HandleTrDirective(operand);
}

void MerlinSyntaxParser::HandleAsc(const std::string &operand,
                                   Section &section) {
  HandleAscDirective(operand, section, current_address_);
}

void MerlinSyntaxParser::HandleDA(const std::string &operand, Section &section,
                                  ConcreteSymbolTable &symbols) {
  HandleDaDirective(operand, section, symbols, current_address_);
}

void MerlinSyntaxParser::HandleDCI(const std::string &operand,
                                   Section &section) {
  HandleDciDirective(operand, section, current_address_);
}

void MerlinSyntaxParser::HandleINV(const std::string &operand,
                                   Section &section) {
  HandleInvDirective(operand, section, current_address_);
}

void MerlinSyntaxParser::HandleFLS(const std::string &operand,
                                   Section &section) {
  HandleFlsDirective(operand, section, current_address_);
}

void MerlinSyntaxParser::HandleSav(const std::string &operand) {
  HandleSavDirective(operand);
}

void MerlinSyntaxParser::HandleXc(const std::string &operand) {
  HandleXcDirective(operand, cpu_);
}

void MerlinSyntaxParser::HandleMx(const std::string &operand) {
  HandleMxDirective(operand);
}

void MerlinSyntaxParser::HandleRev(const std::string &label,
                                   const std::string &operand, Section &section,
                                   ConcreteSymbolTable &symbols) {
  HandleRevDirective(label, operand, section, symbols, current_address_);
}

void MerlinSyntaxParser::HandleLup(const std::string &operand) {
  HandleLupDirective(operand);
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

  // Check if we should skip this line due to conditional assembly (Phase 4: use shared component)
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
  ctx.parser_state = this;  // For accessing Merlin-specific state if needed
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
}

} // namespace xasm
