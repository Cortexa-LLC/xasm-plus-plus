/**
 * @file edtasm_directive_handlers.cpp
 * @brief Implementation of EDTASM-M80++ directive handlers
 *
 * Free function implementations for EDTASM-M80++ specific directives.
 * Part of God-Class Phase 6c.3 refactoring.
 */

#include "xasm++/syntax/edtasm_directive_handlers.h"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "xasm++/atom.h"
#include "xasm++/directives/common_directives.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/directive_error_utils.h"
#include "xasm++/directives/z80_directives.h"
#include "xasm++/expression_utils.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/util/string_utils.h"

namespace xasm {

using namespace directives;
using namespace directive_utils;
using namespace CommonDirectives;
using namespace Z80Directives;

// Helper: Parse comma-separated tokens, respecting quoted strings
static std::vector<std::string> ParseDataTokens(const std::string& operand) {
  std::vector<std::string> tokens;
  std::string current_token;
  bool in_string = false;
  char string_delimiter = '\0';
  bool escape_next = false;

  for (char c : operand) {
    if (escape_next) {
      current_token += c;
      escape_next = false;
    } else if (in_string && c == '\\') {
      current_token += c;
      escape_next = true;
    } else if (in_string && c == string_delimiter) {
      in_string = false;
      current_token += c;
    } else if (!in_string && (c == '\'' || c == '"')) {
      in_string = true;
      string_delimiter = c;
      current_token += c;
    } else if (!in_string && c == ',') {
      if (!current_token.empty()) {
        tokens.push_back(util::Trim(current_token));
        current_token.clear();
      }
    } else {
      current_token += c;
    }
  }

  if (!current_token.empty()) {
    tokens.push_back(util::Trim(current_token));
  }

  return tokens;
}

// Helper: Process string literal and add bytes
static void ProcessStringLiteral(const std::string& token, std::vector<uint8_t>& bytes,
                                 uint32_t& address, const std::string& /*filename*/, int /*line*/) {
  if (token.size() < 2) {
    throw std::runtime_error("Invalid string literal");
  }

  char delimiter = token[0];
  size_t end_pos = token.size() - 1;
  if (token[end_pos] != delimiter) {
    throw std::runtime_error("String missing closing delimiter");
  }

  // Process escape sequences
  for (size_t i = 1; i < end_pos; ++i) {
    char c = token[i];
    if (c == '\\' && i + 1 < end_pos) {
      char next = token[i + 1];
      switch (next) {
        case 'n':
          bytes.push_back('\n');
          break;
        case 'r':
          bytes.push_back('\r');
          break;
        case 't':
          bytes.push_back('\t');
          break;
        case '\\':
          bytes.push_back('\\');
          break;
        case '\'':
          bytes.push_back('\'');
          break;
        case '"':
          bytes.push_back('"');
          break;
        default:
          bytes.push_back(c);
          bytes.push_back(next);
          break;
      }
      i++;
      address++;
    } else {
      bytes.push_back(static_cast<uint8_t>(c));
      address++;
    }
  }
}

// ============================================================================
// Directive Handler Implementations
// ============================================================================

// DB/DEFB/BYTE/DM/DEFM/TEXT/ASCII - Define byte(s)
void HandleDbDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  auto tokens = ParseDataTokens(operand);
  std::vector<uint8_t> immediate_bytes;

  for (const std::string& token : tokens) {
    if (token.empty())
      continue;

    // Check for string literal
    if (token.size() >= 2 && (token[0] == '\'' || token[0] == '"')) {
      ProcessStringLiteral(token, immediate_bytes, *ctx.current_address, ctx.current_file,
                           ctx.current_line);
    } else {
      // Try to evaluate expression
      try {
        auto expr = parser->ParseExpression(token, *ctx.symbols);
        if (expr) {
          uint32_t value = expr->Evaluate(*ctx.symbols);
          immediate_bytes.push_back(static_cast<uint8_t>(value & 0xFF));
          (*ctx.current_address)++;
        }
      } catch (...) {
        // Forward reference - create deferred atom
        if (!immediate_bytes.empty()) {
          auto data_atom = std::make_shared<DataAtom>(immediate_bytes);
          data_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
          data_atom->source_line = ctx.source_line;
          ctx.section->atoms.push_back(data_atom);
          immediate_bytes.clear();
        }

        std::vector<std::string> expressions = {token};
        auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Byte);
        data_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
        data_atom->source_line = ctx.source_line;
        ctx.section->atoms.push_back(data_atom);
        (*ctx.current_address)++;
      }
    }
  }

  // Create atom for remaining immediate bytes
  if (!tokens.empty()) {
    auto data_atom = std::make_shared<DataAtom>(immediate_bytes);
    data_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    data_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(data_atom);
  }
}

// DW/DEFW/WORD - Define word(s)
void HandleDwDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto tokens = ParseDataTokens(operand);

  // DW now always stores as expressions for consistent forward/backward ref
  // handling
  std::vector<std::string> expressions;

  for (const std::string& token : tokens) {
    if (token.empty())
      continue;

    expressions.push_back(token);
    (*ctx.current_address) += 2;
  }

  // Create single DataAtom with all expressions
  if (!expressions.empty()) {
    auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Word);
    data_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    data_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(data_atom);
  }
}

// DS/DEFS/DSPACE/BLOCK - Define storage
void HandleDsDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse size and optional fill value
  size_t comma_pos = operand.find(',');
  std::string size_str =
      util::Trim(comma_pos == std::string::npos ? operand : operand.substr(0, comma_pos));
  std::string fill_str =
      comma_pos == std::string::npos ? "" : util::Trim(operand.substr(comma_pos + 1));

  uint32_t size = ParseAndEvaluateExpression(size_str, *parser, *ctx.symbols, "DS");

  // DS/DEFS should create SpaceAtom for reserve space semantics
  if (fill_str.empty()) {
    // No fill value - use SpaceAtom
    auto space_atom = std::make_shared<SpaceAtom>(size);
    space_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    space_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(space_atom);
  } else {
    // With fill value - use DataAtom with initialized bytes
    uint8_t fill_value = ParseAndEvaluateAsByte(fill_str, *parser, *ctx.symbols, "DS fill");
    std::vector<uint8_t> bytes(size, fill_value);
    auto data_atom = std::make_shared<DataAtom>(bytes);
    data_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    data_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(data_atom);
  }

  (*ctx.current_address) += size;
}

// DD/DEFD/DWORD - Define doubleword(s)
void HandleDdDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;

  // TODO: Implement DD directive when DataSize::DWord is added to atom.h
  throw std::runtime_error("DD directive not yet implemented: " + operand);
}

// PUBLIC/GLOBAL/ENTRY - Mark symbols as public
void HandlePublicDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto tokens = ParseDataTokens(operand);
  for (const std::string& symbol : tokens) {
    if (!symbol.empty()) {
      auto* symbol_ptr = ctx.symbols->GetSymbol(symbol);
      if (symbol_ptr) {
        symbol_ptr->is_exported = true;
      } else {
        // Create placeholder for forward reference
        Symbol new_symbol;
        new_symbol.name = symbol;
        new_symbol.is_exported = true;
        ctx.symbols->Define(symbol, SymbolType::Label, nullptr);
        ctx.symbols->GetSymbol(symbol)->is_exported = true;
      }
    }
  }
}

// EXTERN/EXTRN/EXT - Mark symbols as external
void HandleExternDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto tokens = ParseDataTokens(operand);
  for (const std::string& symbol : tokens) {
    if (!symbol.empty()) {
      ctx.symbols->Define(symbol, SymbolType::Label, nullptr);
      ctx.symbols->GetSymbol(symbol)->is_imported = true;
    }
  }
}

// CSEG - Code segment
void HandleCsegDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Code);
}

// DSEG - Data segment
void HandleDsegDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Data);
}

// ASEG - Absolute segment
void HandleAsegDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Absolute);
}

// COMMON - Common block
void HandleCommonDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  std::string block_name = util::Trim(operand);
  parser->GetSegmentManager().SwitchToCommon(block_name);
}

// ENDC - End common block
void HandleEndcDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Code);
}

// IF - Conditional assembly
void HandleIfDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t value = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "IF");
  parser->PushConditional(value != 0);
}

// ELSE - Conditional assembly else
void HandleElseDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->ToggleConditional();
}

// ENDIF - End conditional assembly
void HandleEndifDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->PopConditional();
}

// IFDEF - If symbol defined
void HandleIfdefDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  std::string symbol = util::Trim(operand);
  bool defined = ctx.symbols->GetSymbol(symbol) != nullptr;
  parser->PushConditional(defined);
}

// IFNDEF - If symbol not defined
void HandleIfndefDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  std::string symbol = util::Trim(operand);
  bool defined = ctx.symbols->GetSymbol(symbol) != nullptr;
  parser->PushConditional(!defined);
}

// IFE - If equal to zero
void HandleIfeDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t value = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "IFE");
  parser->PushConditional(value == 0);
}

// IFEQ - If equal
void HandleIfeqDirective(DirectiveContext& ctx) {
  HandleIfeDirective(ctx);
}

// IFNE - If not equal to zero
void HandleIfneDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t value = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "IFNE");
  parser->PushConditional(value != 0);
}

// IFGT - If greater than zero
void HandleIfgtDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  int32_t value = ParseAndEvaluateAsSignedInt(operand, *parser, *ctx.symbols, "IFGT");
  parser->PushConditional(value > 0);
}

// IFGE - If greater than or equal to zero
void HandleIfgeDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  int32_t value = ParseAndEvaluateAsSignedInt(operand, *parser, *ctx.symbols, "IFGE");
  parser->PushConditional(value >= 0);
}

// IFLT - If less than zero
void HandleIfltDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  int32_t value = ParseAndEvaluateAsSignedInt(operand, *parser, *ctx.symbols, "IFLT");
  parser->PushConditional(value < 0);
}

// IFLE - If less than or equal to zero
void HandleIfleDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  int32_t value = ParseAndEvaluateAsSignedInt(operand, *parser, *ctx.symbols, "IFLE");
  parser->PushConditional(value <= 0);
}

// IF1 - If first pass (always false for single-pass assembler)
void HandleIf1Directive(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  // Single-pass assembler: IF1 is always false (we're never on "pass 1")
  parser->PushConditional(false);
}

// IF2 - If second pass (always false for single-pass assembler)
void HandleIf2Directive(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  // Single-pass assembler: IF2 is always false (we're never on "pass 2")
  parser->PushConditional(false);
}

// IFB - If blank
void HandleIfbDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->PushConditional(util::Trim(operand).empty());
}

// IFNB - If not blank
void HandleIfnbDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->PushConditional(!util::Trim(operand).empty());
}

// IFIDN - If identical (case-insensitive)
void HandleIfidnDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    ThrowFormattedError("IFIDN requires two operands", ctx);
  }

  std::string str1 = util::Trim(operand.substr(0, comma_pos));
  std::string str2 = util::Trim(operand.substr(comma_pos + 1));
  // Case-insensitive comparison
  parser->PushConditional(util::ToUpper(str1) == util::ToUpper(str2));
}

// IFDIF - If different (case-insensitive)
void HandleIfdifDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    ThrowFormattedError("IFDIF requires two operands", ctx);
  }

  std::string str1 = util::Trim(operand.substr(0, comma_pos));
  std::string str2 = util::Trim(operand.substr(comma_pos + 1));
  // Case-insensitive comparison
  parser->PushConditional(util::ToUpper(str1) != util::ToUpper(str2));
}

// INCLUDE - Include file
void HandleIncludeDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  // TODO: Implement file inclusion when needed
  throw std::runtime_error("INCLUDE directive not yet implemented: " + operand);
}

// LIST - Enable listing
void HandleListDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->EnableListing();
  // Create atom for listing control
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::List, "");
  ctx.section->atoms.push_back(atom);
}

// NOLIST - Disable listing
void HandleNolistDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->DisableListing();
  // Create atom for listing control
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Nolist, "");
  ctx.section->atoms.push_back(atom);
}

// TITLE - Set listing title
void HandleTitleDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->SetListingTitle(operand);
  // Create atom for listing control
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Title, operand);
  ctx.section->atoms.push_back(atom);
}

// SUBTTL - Set listing subtitle
void HandleSubttlDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->SetListingSubtitle(operand);
  // Create atom for listing control
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Subtitle, operand);
  ctx.section->atoms.push_back(atom);
}

// NAME - Set module name
void HandleNameDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->SetModuleName(util::Trim(operand));
}

// END - End of source file (stops assembly)
void HandleEndDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->MarkEndDirectiveSeen();
}

// EQU - Define constant symbol
void HandleEquDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  if (ctx.label.empty()) {
    throw std::runtime_error("EQU requires a label");
  }

  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t value = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "EQU");
  ctx.symbols->DefineLabel(ctx.label, value);
}

// SET/=/DEFL - Define redefinable symbol
void HandleSetDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  if (ctx.label.empty()) {
    throw std::runtime_error("SET requires a label");
  }

  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t value = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "SET");
  // SET allows redefinition, so just define/redefine the label
  ctx.symbols->DefineLabel(ctx.label, value);
}

// ORG - Set origin address
void HandleOrgDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  uint32_t address = ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "ORG");

  // Create OrgAtom and update address
  auto org_atom = std::make_shared<OrgAtom>(address);
  org_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  org_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(org_atom);
  *ctx.current_address = address;
}

// *LIST - Toggle listing (Z80ASM style)
void HandleStarListDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse operand to determine ON or OFF
  bool enable = true;  // Default to ON
  std::string upper_operand = operand;
  std::transform(upper_operand.begin(), upper_operand.end(), upper_operand.begin(), ::toupper);

  if (upper_operand == "OFF") {
    enable = false;
  } else if (upper_operand == "ON" || upper_operand.empty()) {
    enable = true;
  }

  // Update parser state
  if (enable) {
    parser->EnableListing();
    // Create atom for listing control
    auto atom = std::make_shared<ListingControlAtom>(ListingControlType::List, "");
    ctx.section->atoms.push_back(atom);
  } else {
    parser->DisableListing();
    // Create atom for listing control
    auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Nolist, "");
    ctx.section->atoms.push_back(atom);
  }
}

// LALL - List all (macro expansion)
void HandleLallDirective(DirectiveContext& /*ctx*/) {
  // TODO: Implement when macro listing control is needed
  // For now, this is a no-op as we don't have macro listing control yet
}

// SALL - Suppress all (macro expansion)
void HandleSallDirective(DirectiveContext& /*ctx*/) {
  // TODO: Implement when macro listing control is needed
  // For now, this is a no-op as we don't have macro listing control yet
}

// EJECT - Page eject (listing control)
void HandleEjectDirective(DirectiveContext& ctx) {
  // Create atom for listing control (page break)
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Page, "");
  ctx.section->atoms.push_back(atom);
}

// SPACE - Add blank lines (listing control)
void HandleSpaceDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  // Parse the operand to get line count
  int count = 1;  // Default to 1 line
  if (!operand.empty()) {
    try {
      count = std::stoi(operand);
    } catch (...) {
      count = 1;
    }
  }
  // Create atom for listing control
  auto atom = std::make_shared<ListingControlAtom>(ListingControlType::Space, count);
  ctx.section->atoms.push_back(atom);
}

// RADIX - Set number radix
void HandleRadixDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // RADIX operand must be parsed in decimal (base 10) always
  // to avoid chicken-and-egg problem with current radix
  parser->SetRadix(10);  // Temporarily set to decimal for parsing

  int radix = static_cast<int>(ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "RADIX"));
  parser->SetRadix(radix);  // Set to the new radix value
}

// MACRO - Start macro definition
void HandleMacroDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  std::string macro_name;
  std::vector<std::string> params;

  if (!ctx.label.empty()) {
    // Label-based syntax: MYNAME MACRO param1,param2
    macro_name = ctx.label;
    if (!operand.empty()) {
      auto tokens = ParseDataTokens(operand);
      for (const auto& token : tokens) {
        params.push_back(token);
      }
    }
  } else if (!operand.empty()) {
    // Operand-based syntax: MACRO MYNAME,param1,param2
    auto tokens = ParseDataTokens(operand);
    if (tokens.empty()) {
      throw std::runtime_error("MACRO requires a name");
    }
    macro_name = tokens[0];
    for (size_t i = 1; i < tokens.size(); i++) {
      params.push_back(tokens[i]);
    }
  } else {
    throw std::runtime_error("MACRO requires a name");
  }

  // Start macro definition by setting state
  parser->in_macro_definition_ = true;
  parser->current_macro_.name = macro_name;
  parser->current_macro_.params = params;
  parser->current_macro_.body.clear();
  parser->current_macro_.locals.clear();
  parser->macro_nesting_depth_ = 0;
}

// ENDM - End macro/repeat block
// ---------------------------------------------------------------------------
// ExpandRepeatBlock — invoked by HandleEndmDirective when ENDM closes a
// REPT/IRP/IRPC block.  Clears parser repeat-state first (to prevent
// re-capture of expanded lines), then expands.
// ---------------------------------------------------------------------------
static void ExpandRepeatBlock(EdtasmM80PlusPlusSyntaxParser* parser,
                              EdtasmM80PlusPlusSyntaxParser::RepeatType repeat_type,
                              const std::vector<std::string>& repeat_body, int rept_count,
                              const std::string& repeat_param,
                              const std::vector<std::string>& repeat_values, Section& section,
                              ConcreteSymbolTable& symbols) {
  if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT) {
    for (int i = 0; i < rept_count; ++i) {
      if (parser->exitm_triggered_) {
        break;
      }
      parser->ExpandAndParseLines(repeat_body, section, symbols);
    }
    return;
  }

  // IRP / IRPC: iterate over value list
  for (const auto& value : repeat_values) {
    if (parser->exitm_triggered_) {
      break;
    }
    std::vector<std::string> expanded_lines;
    expanded_lines.reserve(repeat_body.size());
    for (const auto& line : repeat_body) {
      std::vector<std::string> param_names = {repeat_param};
      std::vector<std::string> param_values = {value};
      expanded_lines.push_back(parser->SubstituteMacroParameters(line, param_names, param_values));
    }
    parser->ExpandAndParseLines(expanded_lines, section, symbols);
  }
}

void HandleEndmDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  if (parser->in_macro_definition_) {
    // Store the completed macro and end capture.
    parser->macros_[parser->current_macro_.name] = parser->current_macro_;
    parser->in_macro_definition_ = false;
    return;
  }

  if (parser->in_repeat_block_ != EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE) {
    // Snapshot the repeat state before clearing it to prevent re-capture.
    auto repeat_type = parser->in_repeat_block_;
    auto repeat_body = parser->repeat_body_;
    auto rept_count = parser->rept_count_;
    auto repeat_param = parser->repeat_param_;
    auto repeat_values = parser->repeat_values_;

    // Clear repeat state BEFORE expanding.
    parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE;
    parser->repeat_body_.clear();
    parser->rept_count_ = 0;
    parser->repeat_param_.clear();
    parser->repeat_values_.clear();
    parser->repeat_nesting_depth_ = 0;
    parser->exitm_triggered_ = false;

    ExpandRepeatBlock(parser, repeat_type, repeat_body, rept_count, repeat_param, repeat_values,
                      *ctx.section, *ctx.symbols);
    return;
  }

  throw std::runtime_error("ENDM without matching MACRO/REPT/IRP/IRPC");
}

// LOCAL - Define local symbol in macro
void HandleLocalDirective(DirectiveContext& /*ctx*/) {
  // LOCAL is handled specially during macro capture in ParseLine
  // This handler should never be called in normal execution
  throw std::runtime_error("LOCAL directive can only be used inside MACRO");
}

// EXITM - Exit macro early
void HandleExitmDirective(DirectiveContext& ctx) {
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  parser->exitm_triggered_ = true;
}

// REPT - Repeat block
void HandleReptDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
  int count = static_cast<int>(ParseAndEvaluateExpression(operand, *parser, *ctx.symbols, "REPT"));

  // Start repeat block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT;
  parser->rept_count_ = count;
  parser->repeat_body_.clear();
  parser->repeat_nesting_depth_ = 0;
}

// IRP - Iterate over parameters
void HandleIrpDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse: IRP symbol,<arg1,arg2,...>
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IRP requires symbol and argument list");
  }

  std::string symbol = util::Trim(operand.substr(0, comma_pos));
  std::string args_str = util::Trim(operand.substr(comma_pos + 1));

  // Remove angle brackets if present
  if (!args_str.empty() && args_str.front() == '<' && args_str.back() == '>') {
    args_str = args_str.substr(1, args_str.length() - 2);
  }

  std::vector<std::string> args = ParseDataTokens(args_str);

  // Start IRP block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::IRP;
  parser->repeat_param_ = symbol;
  parser->repeat_values_ = args;
  parser->repeat_body_.clear();
  parser->repeat_nesting_depth_ = 0;
}

// IRPC - Iterate over characters
void HandleIrpcDirective(DirectiveContext& ctx) {
  const std::string& operand = ctx.operand;
  auto* parser = static_cast<EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);

  // Parse: IRPC symbol,<string>
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IRPC requires symbol and string");
  }

  std::string symbol = util::Trim(operand.substr(0, comma_pos));
  std::string str = util::Trim(operand.substr(comma_pos + 1));

  // Remove angle brackets if present
  if (!str.empty() && str.front() == '<' && str.back() == '>') {
    str = str.substr(1, str.length() - 2);
  }

  // Convert each character to a string argument
  std::vector<std::string> args;
  for (char c : str) {
    args.emplace_back(1, c);
  }

  // Start IRPC block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::IRPC;
  parser->repeat_param_ = symbol;
  parser->repeat_values_ = args;
  parser->repeat_body_.clear();
  parser->repeat_nesting_depth_ = 0;
}

// ============================================================================
// Registry Initialization
// ============================================================================

void RegisterEdtasmDirectiveHandlers(DirectiveRegistry& registry) {
  // Data definition directives
  registry.Register({"DB", "DEFB", "DM", "DEFM", "TEXT", "ASCII", "BYTE"}, HandleDbDirective);
  registry.Register({"DW", "DEFW", "WORD"}, HandleDwDirective);
  registry.Register({"DS", "DEFS", "DSPACE", "BLOCK"}, HandleDsDirective);
  registry.Register("DD", HandleDdDirective);

  // Symbol visibility
  registry.Register({"PUBLIC", "GLOBAL", "ENTRY"}, HandlePublicDirective);
  registry.Register({"EXTERN", "EXTRN", "EXT"}, HandleExternDirective);

  // Segment control
  registry.Register("CSEG", HandleCsegDirective);
  registry.Register("DSEG", HandleDsegDirective);
  registry.Register("ASEG", HandleAsegDirective);
  registry.Register("COMMON", HandleCommonDirective);
  registry.Register("ENDC", HandleEndcDirective);

  // Symbol definition
  registry.Register("ORG", HandleOrgDirective);
  registry.Register("EQU", HandleEquDirective);
  registry.Register({"SET", "=", "DEFL"}, HandleSetDirective);

  // Conditional assembly
  registry.Register("IF", HandleIfDirective);
  registry.Register("ELSE", HandleElseDirective);
  registry.Register({"ENDIF", "ENDC"}, HandleEndifDirective);
  registry.Register("IFDEF", HandleIfdefDirective);
  registry.Register("IFNDEF", HandleIfndefDirective);
  registry.Register({"IFE", "IFEQ"}, HandleIfeDirective);
  registry.Register("IFNE", HandleIfneDirective);
  registry.Register("IFGT", HandleIfgtDirective);
  registry.Register("IFGE", HandleIfgeDirective);
  registry.Register("IFLT", HandleIfltDirective);
  registry.Register("IFLE", HandleIfleDirective);
  registry.Register("IF1", HandleIf1Directive);
  registry.Register("IF2", HandleIf2Directive);
  registry.Register("IFB", HandleIfbDirective);
  registry.Register("IFNB", HandleIfnbDirective);
  registry.Register("IFIDN", HandleIfidnDirective);
  registry.Register("IFDIF", HandleIfdifDirective);

  // File inclusion
  registry.Register("INCLUDE", HandleIncludeDirective);

  // Listing control
  registry.Register("LIST", HandleListDirective);
  registry.Register(DOT_LIST, HandleListDirective);  // .LIST alias
  registry.Register("NOLIST", HandleNolistDirective);
  registry.Register(DOT_XLIST, HandleNolistDirective);  // .XLIST alias
  registry.Register(STAR_LIST, HandleStarListDirective);
  registry.Register("LALL", HandleLallDirective);
  registry.Register("SALL", HandleSallDirective);
  registry.Register("TITLE", HandleTitleDirective);
  registry.Register(DOT_TITLE, HandleTitleDirective);  // .TITLE alias
  registry.Register("SUBTTL", HandleSubttlDirective);
  registry.Register(DOT_SUBTTL, HandleSubttlDirective);  // .SUBTTL alias
  registry.Register("EJECT", HandleEjectDirective);
  registry.Register("SPACE", HandleSpaceDirective);
  registry.Register("NAME", HandleNameDirective);
  registry.Register("RADIX", HandleRadixDirective);
  registry.Register(DOT_RADIX, HandleRadixDirective);  // .RADIX alias
  registry.Register(STAR_RADIX, HandleRadixDirective);

  // Macro system
  registry.Register("MACRO", HandleMacroDirective);
  registry.Register("ENDM", HandleEndmDirective);
  registry.Register("LOCAL", HandleLocalDirective);
  registry.Register("EXITM", HandleExitmDirective);
  registry.Register("REPT", HandleReptDirective);
  registry.Register("IRP", HandleIrpDirective);
  registry.Register("IRPC", HandleIrpcDirective);

  // End of source
  registry.Register("END", HandleEndDirective);
}

}  // namespace xasm
