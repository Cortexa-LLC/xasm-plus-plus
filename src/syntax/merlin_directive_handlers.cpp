/**
 * @file merlin_directive_handlers.cpp
 * @brief Merlin directive handler implementations
 *
 * Direct implementations of Merlin directive handlers matching SCMASM pattern.
 * Eliminates unnecessary indirection through MerlinSyntaxParser methods.
 */

#include "xasm++/directives/merlin_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/directive_error_utils.h"
#include "xasm++/expression.h"
#include "xasm++/expression_utils.h"
#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/util/string_utils.h"
#include <sstream>
#include <stdexcept>

namespace xasm {
namespace merlin {

using xasm::util::ToUpper;
using xasm::util::Trim;
using namespace xasm::directive_utils;

namespace {

// Radix values for number parsing
constexpr int RADIX_HEXADECIMAL = 16;

} // anonymous namespace

// ============================================================================
// Helper Functions (temporary delegation to parser methods)
// ============================================================================

/**
 * @brief Parse numeric expression
 *
 * Parses numeric expressions including hex ($), binary (%), decimal literals,
 * and symbol references. Uses ExpressionParser for evaluation.
 *
 * @param str String to parse (e.g., "$1000", "255", "symbol+1")
 * @param symbols Symbol table for lookups
 * @return Parsed numeric value
 * @throws std::runtime_error on parse failure
 */
static uint32_t ParseNumber(const std::string &str,
                            ConcreteSymbolTable &symbols) {
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

  // Use ExpressionParser for parsing
  ExpressionParser parser(&symbols);
  try {
    auto expr = parser.Parse(clean_str);
    return static_cast<uint32_t>(expr->Evaluate(symbols));
  } catch (const std::runtime_error &e) {
    // Re-throw with context
    throw std::runtime_error(std::string("Parse error: ") + e.what());
  }
}

// FormatError moved to directive_error_utils.h (P2.5 refactoring)

// ============================================================================
// Directive Handlers
// ============================================================================

void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  
  // ORG directive - set assembly origin address
  RequireOperand(operand, "ORG", context);
  std::string op = Trim(operand);
  
  uint32_t address = 0;
  
  // Parse address (number or symbol)
  if (op[0] == '$' || op[0] == '%' || std::isdigit(op[0])) {
    // Numeric literal
    address = ParseNumber(op, *context.symbols);
  } else {
    // Symbol - look it up
    int64_t value = 0;
    if (context.symbols->Lookup(op, value)) {
      address = static_cast<uint32_t>(value);
    } else {
      // Undefined symbol - use 0 (forward reference issue)
      address = 0;
    }
  }
  
  context.section->atoms.push_back(std::make_shared<OrgAtom>(address));
  *context.current_address = address;
}

void HandleEqu(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)context.section;
  
  // Get parser instance for expression parsing
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // EQU directive - define symbolic constant (no code generated)
  auto expr = parser->ParseExpression(operand, *context.symbols);
  context.symbols->Define(label, SymbolType::Label, expr);
}

void HandleDb(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // DB directive - define byte(s)
  // Parse comma-separated expressions and evaluate immediately
  std::vector<uint8_t> bytes;
  std::istringstream iss(operand);
  std::string value;

  while (std::getline(iss, value, ',')) {
    value = Trim(value);
    if (!value.empty()) {
      uint8_t byte_value = ParseAndEvaluateAsByte(value, *parser, *context.symbols, "DB");
      bytes.push_back(byte_value);
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleDw(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
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
  context.section->atoms.push_back(data_atom);

  // Reserve space (2 bytes per word expression)
  *context.current_address += expressions.size() * 2;
}

void HandleHex(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // HEX directive - Direct implementation from merlin_directives.cpp
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
            ThrowFormattedError("Invalid hex digit '" + std::string(1, c) +
                            "' in HEX directive: '" + token + "'", context);
          }
        }
        try {
          bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, RADIX_HEXADECIMAL)));
        } catch (const std::exception &e) {
          ThrowFormattedError("Invalid hex value in HEX directive: '" + token +
                          "' - " + e.what(), context);
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
        ThrowFormattedError("Invalid hex digit '" + std::string(1, c) +
                        "' in HEX directive: '" + operand + "'", context);
      }
    }

    // Parse pairs of hex digits
    for (size_t i = 0; i + 1 < hex_str.length(); i += 2) {
      std::string byte_str = hex_str.substr(i, 2);
      try {
        bytes.push_back(
            static_cast<uint8_t>(std::stoul(byte_str, nullptr, RADIX_HEXADECIMAL)));
      } catch (const std::exception &e) {
        ThrowFormattedError("Invalid hex value in HEX directive: '" + byte_str +
                        "' - " + e.what(), context);
      }
    }
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleDs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    // Use dum_address_ if in DUM block, otherwise current_address_
    uint32_t label_address = parser->IsInDumBlock() 
        ? parser->GetDumAddress() 
        : parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(label_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, label_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleDS(operand, *context.section, *context.symbols);
}

void HandleDum(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleDum(operand, *context.symbols);
}

void HandleDend(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleDend();
}

void HandlePut(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandlePut(operand, *context.section, *context.symbols);
}

void HandleDo(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleDo(operand, *context.symbols);
}

void HandleElse(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleElse();
}

void HandleFin(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleFin();
}

void HandleLst(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context;
  
  // LST directive - Direct implementation from merlin_directives.cpp
  // LST/LST OFF - listing control
  // No-op for compatibility (listing not implemented)
}

void HandleLstdo(const std::string &label, const std::string &operand,
                 DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context;
  
  // LSTDO directive - Direct implementation from merlin_directives.cpp
  // LSTDO - list during DO blocks
  // No-op for compatibility (listing not implemented)
}

void HandleTr(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context;
  
  // TR directive - Direct implementation from merlin_directives.cpp
  // TR [ADR|ON|OFF] - truncate listing
  // No-op for compatibility (listing not implemented)
}

void HandleAsc(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // ASC directive - Direct implementation from merlin_directives.cpp
  // ASC 'string' or ASC "string" - ASCII string directive
  // Apple II/Merlin standard: Sets high bit on ALL characters (0x80 | char)
  
  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleDci(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // DCI directive - Direct implementation from merlin_directives.cpp
  // DCI 'string' - DCI string (last character with high bit set)
  
  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleInv(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // INV directive - Direct implementation from merlin_directives.cpp
  // INV 'string' - Inverse ASCII (all characters with high bit set)
  
  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleFls(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // FLS directive - Direct implementation from merlin_directives.cpp
  // FLS 'string' - Flash ASCII (alternating high bit for flashing effect)
  
  std::vector<uint8_t> bytes;
  std::string op = Trim(operand);

  if (op.empty()) {
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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
    context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
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

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // DA (Define Address) - same as DW, word definitions in little-endian
  // Delegate to HandleDw which has the full implementation
  HandleDw(label, operand, context);
}

void HandlePmc(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandlePMC(label.empty() ? operand : label);
}

void HandleEom(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleEOM();
}

void HandleMac(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
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
    throw std::runtime_error("MAC requires macro name");
  }

  // Check if macro exists (invocation) or is being defined
  if (parser->MacroExists(macro_name)) {
    // Macro exists - invoke it
    parser->ExpandMacro(macro_name, params_str, *context.section, *context.symbols);
  } else {
    // Macro doesn't exist - start definition
    if (parser->IsInMacroDefinition()) {
      throw std::runtime_error("Nested macro definitions not allowed");
    }
    parser->StartMacroDefinition(macro_name);
  }
}

void HandleUsr(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)operand;
  
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // USR directive - Direct implementation from merlin_directives.cpp
  // Create label if present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  // USR is a no-op - user-defined subroutine (no atoms generated)
}

void HandleEnd(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)operand;
  
  // Get parser instance for label handling and state
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // END directive - Direct implementation from merlin_directives.cpp
  // Create label if present
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  // END - mark end of source (stop processing further lines)
  // Note: Needs parser state access for end_directive_seen_ flag
  parser->HandleEnd();
}

void HandleSav(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context;
  
  // SAV directive - Direct implementation from merlin_directives.cpp
  // SAV filename - Save output filename directive
  // No-op for compatibility (output filename controlled by command-line args)
}

void HandleXc(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleXc(operand);
}

void HandleMx(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleMx(operand);
}

void HandleRev(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance for label handling
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  // REV "string" - Reverse ASCII string
  std::string op = Trim(operand);

  if (op.empty()) {
    throw std::runtime_error("REV requires a string operand");
  }

  // Find string delimiter (single or double quote)
  char quote = '\0';
  size_t start_pos = 0;

  if (op[0] == '\'' || op[0] == '"') {
    quote = op[0];
    start_pos = 1;
  } else {
    throw std::runtime_error("REV requires quoted string");
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
    throw std::runtime_error("REV requires non-empty string");
  }

  // Create label at current address (before emitting bytes)
  if (!label.empty()) {
    uint32_t current_address = *context.current_address;
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }

  // Reverse the string
  std::string reversed(text.rbegin(), text.rend());

  // Emit reversed bytes as data
  std::vector<uint8_t> bytes;
  for (char ch : reversed) {
    bytes.push_back(static_cast<uint8_t>(ch));
  }

  context.section->atoms.push_back(std::make_shared<DataAtom>(bytes));
  *context.current_address += bytes.size();
}

void HandleLup(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  ValidateParser(context.parser_state);
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  
  parser->HandleLup(operand);
}

} // namespace merlin
} // namespace xasm
