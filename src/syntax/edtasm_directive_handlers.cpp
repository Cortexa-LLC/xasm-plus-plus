/**
 * @file edtasm_directive_handlers.cpp
 * @brief Implementation of EDTASM-M80++ directive handlers
 *
 * Free function implementations for EDTASM-M80++ specific directives.
 * Part of God-Class Phase 6c.3 refactoring.
 */

#include "xasm++/syntax/edtasm_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/directives/common_directives.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/directives/z80_directives.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace xasm {

using namespace directives;
using namespace CommonDirectives;
using namespace Z80Directives;

// Helper: Parse comma-separated tokens, respecting quoted strings
static std::vector<std::string> ParseDataTokens(const std::string &operand) {
  std::vector<std::string> tokens;
  std::string current_token;
  bool in_string = false;
  char string_delimiter = '\0';
  bool escape_next = false;

  for (size_t i = 0; i < operand.size(); ++i) {
    char c = operand[i];

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
static void ProcessStringLiteral(const std::string &token,
                                  std::vector<uint8_t> &bytes,
                                  uint32_t &address,
                                  const std::string & /*filename*/, int /*line*/) {
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
      case 'n': bytes.push_back('\n'); break;
      case 'r': bytes.push_back('\r'); break;
      case 't': bytes.push_back('\t'); break;
      case '\\': bytes.push_back('\\'); break;
      case '\'': bytes.push_back('\''); break;
      case '"': bytes.push_back('"'); break;
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
void HandleDbDirective(const std::string & /*label*/,
                       const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto tokens = ParseDataTokens(operand);
  std::vector<uint8_t> immediate_bytes;

  for (const std::string &token : tokens) {
    if (token.empty())
      continue;

    // Check for string literal
    if (token.size() >= 2 && (token[0] == '\'' || token[0] == '"')) {
      ProcessStringLiteral(token, immediate_bytes, *ctx.current_address,
                           ctx.current_file, ctx.current_line);
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
          data_atom->location =
              SourceLocation(ctx.current_file, ctx.current_line, 1);
          data_atom->source_line = ctx.source_line;
          ctx.section->atoms.push_back(data_atom);
          immediate_bytes.clear();
        }

        std::vector<std::string> expressions = {token};
        auto data_atom =
            std::make_shared<DataAtom>(expressions, DataSize::Byte);
        data_atom->location =
            SourceLocation(ctx.current_file, ctx.current_line, 1);
        data_atom->source_line = ctx.source_line;
        ctx.section->atoms.push_back(data_atom);
        (*ctx.current_address)++;
      }
    }
  }

  // Create atom for remaining immediate bytes
  if (!tokens.empty()) {
    auto data_atom = std::make_shared<DataAtom>(immediate_bytes);
    data_atom->location =
        SourceLocation(ctx.current_file, ctx.current_line, 1);
    data_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(data_atom);
  }
}

// DW/DEFW/WORD - Define word(s)
void HandleDwDirective(const std::string & /*label*/,
                       const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto tokens = ParseDataTokens(operand);
  std::vector<uint16_t> immediate_words;

  for (const std::string &token : tokens) {
    if (token.empty())
      continue;

    try {
      auto expr = parser->ParseExpression(token, *ctx.symbols);
      if (expr) {
        uint32_t value = expr->Evaluate(*ctx.symbols);
        immediate_words.push_back(static_cast<uint16_t>(value & 0xFFFF));
        (*ctx.current_address) += 2;
      }
    } catch (...) {
      // Forward reference - create deferred atom
      if (!immediate_words.empty()) {
        std::vector<uint8_t> bytes;
        for (uint16_t word : immediate_words) {
          bytes.push_back(word & 0xFF);
          bytes.push_back((word >> 8) & 0xFF);
        }
        auto data_atom = std::make_shared<DataAtom>(bytes);
        data_atom->location =
            SourceLocation(ctx.current_file, ctx.current_line, 1);
        data_atom->source_line = ctx.source_line;
        ctx.section->atoms.push_back(data_atom);
        immediate_words.clear();
      }

      std::vector<std::string> expressions = {token};
      auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Word);
      data_atom->location =
          SourceLocation(ctx.current_file, ctx.current_line, 1);
      data_atom->source_line = ctx.source_line;
      ctx.section->atoms.push_back(data_atom);
      (*ctx.current_address) += 2;
    }
  }

  // Create atom for remaining immediate words
  if (!tokens.empty()) {
    std::vector<uint8_t> bytes;
    for (uint16_t word : immediate_words) {
      bytes.push_back(word & 0xFF);
      bytes.push_back((word >> 8) & 0xFF);
    }
    auto data_atom = std::make_shared<DataAtom>(bytes);
    data_atom->location =
        SourceLocation(ctx.current_file, ctx.current_line, 1);
    data_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(data_atom);
  }
}

// DS/DEFS/DSPACE/BLOCK - Define storage
void HandleDsDirective(const std::string & /*label*/,
                       const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
  // Parse size and optional fill value
  size_t comma_pos = operand.find(',');
  std::string size_str = util::Trim(
      comma_pos == std::string::npos ? operand : operand.substr(0, comma_pos));
  std::string fill_str =
      comma_pos == std::string::npos ? "" : util::Trim(operand.substr(comma_pos + 1));

  auto size_expr = parser->ParseExpression(size_str, *ctx.symbols);
  if (!size_expr) {
    throw std::runtime_error("Invalid DS size expression");
  }

  uint32_t size = size_expr->Evaluate(*ctx.symbols);
  uint8_t fill_value = 0;

  if (!fill_str.empty()) {
    auto fill_expr = parser->ParseExpression(fill_str, *ctx.symbols);
    if (fill_expr) {
      fill_value = static_cast<uint8_t>(fill_expr->Evaluate(*ctx.symbols) & 0xFF);
    }
  }

  std::vector<uint8_t> bytes(size, fill_value);
  auto data_atom = std::make_shared<DataAtom>(bytes);
  data_atom->location =
      SourceLocation(ctx.current_file, ctx.current_line, 1);
  data_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(data_atom);
  (*ctx.current_address) += size;
}

// DD/DEFD/DWORD - Define doubleword(s)
void HandleDdDirective(const std::string & /*label*/,
                       const std::string &operand, DirectiveContext & /*ctx*/) {
  // TODO: Implement DD directive when DataSize::DWord is added to atom.h
  throw std::runtime_error("DD directive not yet implemented: " + operand);
}

// PUBLIC/GLOBAL/ENTRY - Mark symbols as public
void HandlePublicDirective(const std::string & /*label*/,
                            const std::string &operand, DirectiveContext &ctx) {
  auto tokens = ParseDataTokens(operand);
  for (const std::string &symbol : tokens) {
    if (!symbol.empty()) {
      auto *symbol_ptr = ctx.symbols->GetSymbol(symbol);
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
void HandleExternDirective(const std::string & /*label*/,
                            const std::string &operand, DirectiveContext &ctx) {
  auto tokens = ParseDataTokens(operand);
  for (const std::string &symbol : tokens) {
    if (!symbol.empty()) {
      ctx.symbols->Define(symbol, SymbolType::Label, nullptr);
      ctx.symbols->GetSymbol(symbol)->is_imported = true;
    }
  }
}

// CSEG - Code segment
void HandleCsegDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Code);
}

// DSEG - Data segment
void HandleDsegDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Data);
}

// ASEG - Absolute segment
void HandleAsegDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Absolute);
}

// COMMON - Common block
void HandleCommonDirective(const std::string & /*label*/,
                           const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  std::string block_name = util::Trim(operand);
  parser->GetSegmentManager().SwitchToCommon(block_name);
}

// ENDC - End common block
void HandleEndcDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Code);
}

// IF - Conditional assembly
void HandleIfDirective(const std::string & /*label*/,
                       const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IF expression");
  }
  
  uint32_t value = expr->Evaluate(*ctx.symbols);
  parser->PushConditional(value != 0);
}

// ELSE - Conditional assembly else
void HandleElseDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->ToggleConditional();
}

// ENDIF - End conditional assembly
void HandleEndifDirective(const std::string & /*label*/,
                          const std::string & /*operand*/,
                          DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->PopConditional();
}

// IFDEF - If symbol defined
void HandleIfdefDirective(const std::string & /*label*/,
                          const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  std::string symbol = util::Trim(operand);
  bool defined = ctx.symbols->GetSymbol(symbol) != nullptr;
  parser->PushConditional(defined);
}

// IFNDEF - If symbol not defined
void HandleIfndefDirective(const std::string & /*label*/,
                           const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  std::string symbol = util::Trim(operand);
  bool defined = ctx.symbols->GetSymbol(symbol) != nullptr;
  parser->PushConditional(!defined);
}

// IFE - If equal to zero
void HandleIfeDirective(const std::string & /*label*/,
                        const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFE expression");
  }
  uint32_t value = expr->Evaluate(*ctx.symbols);
  parser->PushConditional(value == 0);
}

// IFEQ - If equal
void HandleIfeqDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  HandleIfeDirective(std::string(), operand, ctx);
}

// IFNE - If not equal to zero
void HandleIfneDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFNE expression");
  }
  uint32_t value = expr->Evaluate(*ctx.symbols);
  parser->PushConditional(value != 0);
}

// IFGT - If greater than zero
void HandleIfgtDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFGT expression");
  }
  int32_t value = static_cast<int32_t>(expr->Evaluate(*ctx.symbols));
  parser->PushConditional(value > 0);
}

// IFGE - If greater than or equal to zero
void HandleIfgeDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFGE expression");
  }
  int32_t value = static_cast<int32_t>(expr->Evaluate(*ctx.symbols));
  parser->PushConditional(value >= 0);
}

// IFLT - If less than zero
void HandleIfltDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFLT expression");
  }
  int32_t value = static_cast<int32_t>(expr->Evaluate(*ctx.symbols));
  parser->PushConditional(value < 0);
}

// IFLE - If less than or equal to zero
void HandleIfleDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid IFLE expression");
  }
  int32_t value = static_cast<int32_t>(expr->Evaluate(*ctx.symbols));
  parser->PushConditional(value <= 0);
}

// IF1 - If first pass (currently always true - single pass assembler)
void HandleIf1Directive(const std::string & /*label*/,
                        const std::string & /*operand*/,
                        DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  // TODO: Implement pass tracking when multi-pass support is added
  parser->PushConditional(true);
}

// IF2 - If second pass (currently always false - single pass assembler)
void HandleIf2Directive(const std::string & /*label*/,
                        const std::string & /*operand*/,
                        DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  // TODO: Implement pass tracking when multi-pass support is added
  parser->PushConditional(false);
}

// IFB - If blank
void HandleIfbDirective(const std::string & /*label*/,
                        const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->PushConditional(util::Trim(operand).empty());
}

// IFNB - If not blank
void HandleIfnbDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->PushConditional(!util::Trim(operand).empty());
}

// IFIDN - If identical
void HandleIfidnDirective(const std::string & /*label*/,
                          const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IFIDN requires two operands");
  }
  
  std::string str1 = util::Trim(operand.substr(0, comma_pos));
  std::string str2 = util::Trim(operand.substr(comma_pos + 1));
  parser->PushConditional(str1 == str2);
}

// IFDIF - If different
void HandleIfdifDirective(const std::string & /*label*/,
                          const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IFDIF requires two operands");
  }
  
  std::string str1 = util::Trim(operand.substr(0, comma_pos));
  std::string str2 = util::Trim(operand.substr(comma_pos + 1));
  parser->PushConditional(str1 != str2);
}

// INCLUDE - Include file
void HandleIncludeDirective(const std::string & /*label*/,
                            const std::string &operand, DirectiveContext & /*ctx*/) {
  // TODO: Implement file inclusion when needed
  throw std::runtime_error("INCLUDE directive not yet implemented: " + operand);
}

// LIST - Enable listing
void HandleListDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->EnableListing();
}

// NOLIST - Disable listing
void HandleNolistDirective(const std::string & /*label*/,
                           const std::string & /*operand*/,
                           DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->DisableListing();
}

// TITLE - Set listing title
void HandleTitleDirective(const std::string & /*label*/,
                          const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->SetListingTitle(operand);
}

// SUBTTL - Set listing subtitle
void HandleSubttlDirective(const std::string & /*label*/,
                           const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->SetListingSubtitle(operand);
}

// NAME - Set module name
void HandleNameDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->SetModuleName(util::Trim(operand));
}

// END - End of source file (stops assembly)
void HandleEndDirective(const std::string & /*label*/,
                        const std::string & /*operand*/,
                        DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->MarkEndDirectiveSeen();
}

// EQU - Define constant symbol
void HandleEquDirective(const std::string &label,
                        const std::string &operand, DirectiveContext &ctx) {
  if (label.empty()) {
    throw std::runtime_error("EQU requires a label");
  }
  
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid EQU expression");
  }
  
  uint32_t value = expr->Evaluate(*ctx.symbols);
  ctx.symbols->DefineLabel(label, value);
}

// SET/=/DEFL - Define redefinable symbol
void HandleSetDirective(const std::string &label,
                        const std::string &operand, DirectiveContext &ctx) {
  if (label.empty()) {
    throw std::runtime_error("SET requires a label");
  }
  
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid SET expression");
  }
  
  uint32_t value = expr->Evaluate(*ctx.symbols);
  // SET allows redefinition, so just define/redefine the label
  ctx.symbols->DefineLabel(label, value);
}

// ORG - Set origin address
void HandleOrgDirective(const std::string & /*label*/,
                        const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid ORG expression");
  }
  
  uint32_t address = expr->Evaluate(*ctx.symbols);
  
  // Create OrgAtom and update address
  auto org_atom = std::make_shared<OrgAtom>(address);
  org_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  org_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(org_atom);
  *ctx.current_address = address;
}

// *LIST - Toggle listing (Z80ASM style)
void HandleStarListDirective(const std::string & /*label*/,
                             const std::string & /*operand*/,
                             DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  // Toggle listing state
  if (parser->IsListingEnabled()) {
    parser->DisableListing();
  } else {
    parser->EnableListing();
  }
}

// LALL - List all (macro expansion)
void HandleLallDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext & /*ctx*/) {
  // TODO: Implement when macro listing control is needed
  // For now, this is a no-op as we don't have macro listing control yet
}

// SALL - Suppress all (macro expansion)
void HandleSallDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext & /*ctx*/) {
  // TODO: Implement when macro listing control is needed
  // For now, this is a no-op as we don't have macro listing control yet
}

// EJECT - Page eject (listing control)
void HandleEjectDirective(const std::string & /*label*/,
                          const std::string & /*operand*/,
                          DirectiveContext & /*ctx*/) {
  // TODO: Implement when listing output is supported
  // For now, this is a no-op as we don't generate listings yet
}

// SPACE - Add blank lines (listing control)
void HandleSpaceDirective(const std::string & /*label*/,
                          const std::string & /*operand*/,
                          DirectiveContext & /*ctx*/) {
  // TODO: Implement when listing output is supported
  // For now, this is a no-op as we don't generate listings yet
}

// RADIX - Set number radix
void HandleRadixDirective(const std::string & /*label*/,
                          const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid RADIX expression");
  }
  
  int radix = static_cast<int>(expr->Evaluate(*ctx.symbols));
  parser->SetRadix(radix);
}

// MACRO - Start macro definition
void HandleMacroDirective(const std::string &label,
                          const std::string &operand, DirectiveContext &ctx) {
  if (label.empty()) {
    throw std::runtime_error("MACRO requires a label");
  }
  
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
  // Parse parameters
  std::vector<std::string> params;
  if (!operand.empty()) {
    auto tokens = ParseDataTokens(operand);
    for (const auto &token : tokens) {
      params.push_back(token);
    }
  }
  
  // Start macro definition by setting state
  parser->in_macro_definition_ = true;
  parser->current_macro_.name = label;
  parser->current_macro_.params = params;
  parser->current_macro_.body.clear();
  parser->current_macro_.locals.clear();
  parser->macro_nesting_depth_ = 0;
}

// ENDM - End macro/repeat block
void HandleEndmDirective(const std::string & /*label*/,
                         const std::string & /*operand*/,
                         DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
  // Check if we're in a macro definition or repeat block
  if (parser->in_macro_definition_) {
    // Store the completed macro
    parser->macros_[parser->current_macro_.name] = parser->current_macro_;
    parser->in_macro_definition_ = false;
  } else if (parser->in_repeat_block_ != EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE) {
    // Save repeat state before clearing it
    auto repeat_type = parser->in_repeat_block_;
    auto repeat_body = parser->repeat_body_;  // Copy the body
    auto rept_count = parser->rept_count_;
    auto repeat_param = parser->repeat_param_;
    auto repeat_values = parser->repeat_values_;
    
    // Clear repeat state BEFORE expanding to prevent re-capture
    parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE;
    parser->repeat_body_.clear();
    parser->rept_count_ = 0;
    parser->repeat_param_.clear();
    parser->repeat_values_.clear();
    parser->repeat_nesting_depth_ = 0;
    parser->exitm_triggered_ = false;
    
    // Now expand the saved body
    if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT) {
      // REPT: repeat body N times
      for (int i = 0; i < rept_count; ++i) {
        if (parser->exitm_triggered_) {
          break;
        }
        parser->ExpandAndParseLines(repeat_body, *ctx.section, *ctx.symbols);
      }
    } else if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::IRP ||
               repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::IRPC) {
      // IRP/IRPC: iterate over values
      for (const auto &value : repeat_values) {
        if (parser->exitm_triggered_) {
          break;
        }
        // Substitute parameter in each line
        std::vector<std::string> expanded_lines;
        for (const auto &line : repeat_body) {
          std::vector<std::string> param_names = {repeat_param};
          std::vector<std::string> param_values = {value};
          std::string expanded = parser->SubstituteMacroParameters(line, param_names, param_values);
          expanded_lines.push_back(expanded);
        }
        parser->ExpandAndParseLines(expanded_lines, *ctx.section, *ctx.symbols);
      }
    }
  } else {
    throw std::runtime_error("ENDM without matching MACRO/REPT/IRP/IRPC");
  }
}

// LOCAL - Define local symbol in macro
void HandleLocalDirective(const std::string & /*label*/,
                          const std::string & /*operand*/,
                          DirectiveContext & /*ctx*/) {
  // LOCAL is handled specially during macro capture in ParseLine
  // This handler should never be called in normal execution
  throw std::runtime_error("LOCAL directive can only be used inside MACRO");
}

// EXITM - Exit macro early
void HandleExitmDirective(const std::string & /*label*/,
                          const std::string & /*operand*/,
                          DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  parser->exitm_triggered_ = true;
}

// REPT - Repeat block
void HandleReptDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  if (!expr) {
    throw std::runtime_error("Invalid REPT count expression");
  }
  
  int count = static_cast<int>(expr->Evaluate(*ctx.symbols));
  
  // Start repeat block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT;
  parser->rept_count_ = count;
  parser->repeat_body_.clear();
  parser->repeat_nesting_depth_ = 0;
}

// IRP - Iterate over parameters
void HandleIrpDirective(const std::string & /*label*/,
                        const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
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
void HandleIrpcDirective(const std::string & /*label*/,
                         const std::string &operand, DirectiveContext &ctx) {
  auto parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  
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
    args.push_back(std::string(1, c));
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

void RegisterEdtasmDirectiveHandlers(DirectiveRegistry &registry) {
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
  registry.Register("NOLIST", HandleNolistDirective);
  registry.Register(STAR_LIST, HandleStarListDirective);
  registry.Register("LALL", HandleLallDirective);
  registry.Register("SALL", HandleSallDirective);
  registry.Register("TITLE", HandleTitleDirective);
  registry.Register("SUBTTL", HandleSubttlDirective);
  registry.Register("EJECT", HandleEjectDirective);
  registry.Register("SPACE", HandleSpaceDirective);
  registry.Register("NAME", HandleNameDirective);
  registry.Register("RADIX", HandleRadixDirective);

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

} // namespace xasm
