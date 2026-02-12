/**
 * @file edtasm_directive_handlers.cpp
 * @brief Implementation of EDTASM-M80++ directive handlers
 *
 * Free function implementations for EDTASM-M80++ specific directives.
 * Part of God-Class Phase 6c.3 refactoring.
 */

#include "xasm++/syntax/edtasm_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/util/string_utils.h"
#include <algorithm>
#include <stdexcept>

// Namespace aliases for cleaner code
using namespace xasm::directives;
using namespace xasm::util;
using ConditionalBlock = xasm::EdtasmM80PlusPlusSyntaxParser::ConditionalBlock;

namespace {

/**
 * @brief Helper to get parser from context
 * @param ctx Directive context
 * @return Parser pointer or nullptr if not set
 */
xasm::EdtasmM80PlusPlusSyntaxParser* GetParser(xasm::DirectiveContext& ctx) {
  return static_cast<xasm::EdtasmM80PlusPlusSyntaxParser*>(ctx.parser_state);
}

} // anonymous namespace

namespace xasm {

// ============================================================================
// Segment Control Handlers
// ============================================================================

void HandleAsegDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  // Get the parser from context
  auto *parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  if (!parser) {
    throw std::runtime_error("ASEG: Invalid parser state");
  }
  
  // Switch to absolute segment
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Absolute);
  
  // If operand provided, set origin address
  // (Not implementing operand parsing yet - that's a future enhancement)
  (void)operand;
}

void HandleCsegDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  // Get the parser from context
  auto *parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  if (!parser) {
    throw std::runtime_error("CSEG: Invalid parser state");
  }
  
  // Switch to code segment
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Code);
  
  // If operand provided, set origin address
  // (Not implementing operand parsing yet - that's a future enhancement)
  (void)operand;
}

void HandleDsegDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  // Get the parser from context
  auto *parser = static_cast<EdtasmM80PlusPlusSyntaxParser *>(ctx.parser_state);
  if (!parser) {
    throw std::runtime_error("DSEG: Invalid parser state");
  }
  
  // Switch to data segment
  parser->GetSegmentManager().SwitchToSegment(SegmentType::Data);
  
  // If operand provided, set origin address
  // (Not implementing operand parsing yet - that's a future enhancement)
  (void)operand;
}

// ============================================================================
// Conditional Assembly Handlers
// ============================================================================

void HandleIfDirective(const std::string &label, const std::string &operand, 
                       DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IF: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  parser->PushConditional(value != 0);
}

void HandleElseDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("ELSE: Parser context not available");
  }
  
  parser->ToggleConditional();
}

void HandleEndifDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("ENDIF: Parser context not available");
  }
  
  parser->PopConditional();
}

void HandleIfdefDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFDEF: Parser context not available");
  }
  
  std::string symbol = Trim(operand);
  bool is_defined = ctx.symbols->IsDefined(symbol);
  
  ConditionalBlock block;
  block.condition = is_defined;
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfndefDirective(const std::string &label, const std::string &operand, 
                           DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFNDEF: Parser context not available");
  }
  
  std::string symbol = Trim(operand);
  bool is_defined = ctx.symbols->IsDefined(symbol);
  
  ConditionalBlock block;
  block.condition = !is_defined;
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfeqDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFEQ: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value == 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfneDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFNE: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value != 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfltDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFLT: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value < 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfgtDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFGT: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value > 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfleDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFLE: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value <= 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfgeDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFGE: Parser context not available");
  }
  
  auto expr = parser->ParseExpression(operand, *ctx.symbols);
  int64_t value = expr->Evaluate(*ctx.symbols);
  
  ConditionalBlock block;
  block.condition = (value >= 0);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIf1Directive(const std::string &label, const std::string &operand, 
                        DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IF1: Parser context not available");
  }
  
  ConditionalBlock block;
  block.condition = false; // Single-pass assembler, never in pass 1
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIf2Directive(const std::string &label, const std::string &operand, 
                        DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IF2: Parser context not available");
  }
  
  ConditionalBlock block;
  block.condition = false; // Single-pass assembler, never in pass 2
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfbDirective(const std::string &label, const std::string &operand, 
                        DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFB: Parser context not available");
  }
  
  std::string trimmed = Trim(operand);
  
  ConditionalBlock block;
  block.condition = trimmed.empty();
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfnbDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFNB: Parser context not available");
  }
  
  std::string trimmed = Trim(operand);
  
  ConditionalBlock block;
  block.condition = !trimmed.empty();
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfidnDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFIDN: Parser context not available");
  }
  
  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IFIDN requires two operands separated by comma");
  }
  
  std::string op1 = Trim(operand.substr(0, comma_pos));
  std::string op2 = Trim(operand.substr(comma_pos + 1));
  
  // Case-insensitive comparison
  std::string upper1 = op1;
  std::string upper2 = op2;
  std::transform(upper1.begin(), upper1.end(), upper1.begin(), ::toupper);
  std::transform(upper2.begin(), upper2.end(), upper2.begin(), ::toupper);
  
  ConditionalBlock block;
  block.condition = (upper1 == upper2);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

void HandleIfdifDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IFDIF: Parser context not available");
  }
  
  // Parse two operands separated by comma
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IFDIF requires two operands separated by comma");
  }
  
  std::string op1 = Trim(operand.substr(0, comma_pos));
  std::string op2 = Trim(operand.substr(comma_pos + 1));
  
  // Case-insensitive comparison
  std::string upper1 = op1;
  std::string upper2 = op2;
  std::transform(upper1.begin(), upper1.end(), upper1.begin(), ::toupper);
  std::transform(upper2.begin(), upper2.end(), upper2.begin(), ::toupper);
  
  ConditionalBlock block;
  block.condition = (upper1 != upper2);
  block.in_else_block = false;
  block.should_emit = block.condition;
  parser->conditional_stack_.push_back(block);
}

// ============================================================================
// Macro System Handlers
// ============================================================================

void HandleMacroDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  (void)ctx;
  throw std::runtime_error("MACRO directive not yet implemented");
}

void HandleEndmDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("ENDM: Parser context not available");
  }
  
  if (parser->in_repeat_block_ != EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE) {
    // End of repeat block - expand and parse
    // IMPORTANT: Make a copy of repeat_body_ before iterating to avoid use-after-free
    std::vector<std::string> body_copy = parser->repeat_body_;
    EdtasmM80PlusPlusSyntaxParser::RepeatType repeat_type = parser->in_repeat_block_;
    int rept_count = parser->rept_count_;
    std::string repeat_param = parser->repeat_param_;
    std::vector<std::string> repeat_values = parser->repeat_values_;
    
    // Reset repeat state BEFORE expansion (so lines can be parsed normally)
    parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::NONE;
    parser->repeat_body_.clear();
    
    if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT) {
      // Repeat the body rept_count_ times
      for (int i = 0; i < rept_count; ++i) {
        parser->ExpandAndParseLines(body_copy, *ctx.section, *ctx.symbols);
      }
    } else if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::IRP) {
      // Iterate over values, substituting parameter
      for (const auto &value : repeat_values) {
        std::vector<std::string> param_names = {repeat_param};
        std::vector<std::string> param_values = {value};
        
        std::vector<std::string> expanded_lines;
        for (const auto &line : body_copy) {
          expanded_lines.push_back(
              parser->SubstituteMacroParameters(line, param_names, param_values));
        }
        parser->ExpandAndParseLines(expanded_lines, *ctx.section, *ctx.symbols);
      }
    } else if (repeat_type == EdtasmM80PlusPlusSyntaxParser::RepeatType::IRPC) {
      // Iterate over characters
      for (char c : repeat_values[0]) {
        std::vector<std::string> param_names = {repeat_param};
        std::vector<std::string> param_values = {std::string(1, c)};
        
        std::vector<std::string> expanded_lines;
        for (const auto &line : body_copy) {
          expanded_lines.push_back(
              parser->SubstituteMacroParameters(line, param_names, param_values));
        }
        parser->ExpandAndParseLines(expanded_lines, *ctx.section, *ctx.symbols);
      }
    }
    
  } else if (parser->in_macro_definition_) {
    // End of macro definition - store it
    parser->macros_[parser->current_macro_.name] = parser->current_macro_;
    parser->in_macro_definition_ = false;
    parser->current_macro_.body.clear();
    parser->current_macro_.params.clear();
    parser->current_macro_.locals.clear();
    
  } else {
    throw std::runtime_error("ENDM without matching MACRO/REPT/IRP/IRPC");
  }
}

void HandleLocalDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  (void)ctx;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("LOCAL: Parser context not available");
  }
  
  if (!parser->in_macro_definition_) {
    throw std::runtime_error("LOCAL can only be used inside MACRO definitions");
  }
  
  // Parse local label names (comma-separated)
  std::string remaining = operand;
  size_t comma_pos;
  while ((comma_pos = remaining.find(',')) != std::string::npos) {
    std::string local_name = Trim(remaining.substr(0, comma_pos));
    if (!local_name.empty()) {
      parser->current_macro_.locals.push_back(local_name);
    }
    remaining = remaining.substr(comma_pos + 1);
  }
  // Last local name
  std::string local_name = Trim(remaining);
  if (!local_name.empty()) {
    parser->current_macro_.locals.push_back(local_name);
  }
}

void HandleExitmDirective(const std::string &label, const std::string &operand, 
                          DirectiveContext &ctx) {
  (void)label;
  (void)operand;
  (void)ctx;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("EXITM: Parser context not available");
  }
  
  // Set flag to stop macro expansion
  parser->exitm_triggered_ = true;
}

void HandleReptDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  (void)ctx;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("REPT: Parser context not available");
  }
  
  // Parse repeat count
  std::string count_str = Trim(operand);
  if (count_str.empty()) {
    throw std::runtime_error("REPT requires a repeat count");
  }
  
  // Try to parse as number
  int count = 0;
  try {
    count = std::stoi(count_str);
  } catch (...) {
    throw std::runtime_error("REPT count must be a number: " + count_str);
  }
  
  if (count < 0) {
    throw std::runtime_error("REPT count cannot be negative: " + count_str);
  }
  
  // Start capturing repeat block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::REPT;
  parser->rept_count_ = count;
  parser->repeat_body_.clear();
}

void HandleIrpDirective(const std::string &label, const std::string &operand, 
                        DirectiveContext &ctx) {
  (void)label;
  (void)ctx;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IRP: Parser context not available");
  }
  
  // Parse: IRP param, <value1, value2, ...>
  // If no comma, treat entire operand as param name with empty list
  size_t comma_pos = operand.find(',');
  std::string param_name;
  std::string list_str;
  
  if (comma_pos == std::string::npos) {
    // No comma - parameter name with empty list
    param_name = Trim(operand);
    list_str = "";
  } else {
    // Extract parameter name
    param_name = Trim(operand.substr(0, comma_pos));
    // Extract value list
    list_str = Trim(operand.substr(comma_pos + 1));
  }
  
  if (param_name.empty()) {
    throw std::runtime_error("IRP parameter name cannot be empty");
  }
  
  // Parse list - can be <value1, value2> or just value1, value2
  std::vector<std::string> values;
  if (!list_str.empty()) {
    // Remove angle brackets if present
    if (list_str.front() == '<' && list_str.back() == '>') {
      list_str = list_str.substr(1, list_str.size() - 2);
    }
    
    // Trim after removing brackets
    list_str = Trim(list_str);
    if (!list_str.empty()) {
      // Split by comma
      std::string remaining = list_str;
      size_t comma;
      while ((comma = remaining.find(',')) != std::string::npos) {
        std::string value = Trim(remaining.substr(0, comma));
        if (!value.empty()) {
          values.push_back(value);
        }
        remaining = remaining.substr(comma + 1);
      }
      // Last value
      std::string value = Trim(remaining);
      if (!value.empty()) {
        values.push_back(value);
      }
    }
  }
  
  // IRP requires at least one value in the list
  if (values.empty()) {
    throw std::runtime_error("IRP requires a value list");
  }
  
  // Start capturing IRP block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::IRP;
  parser->repeat_param_ = param_name;
  parser->repeat_values_ = values;
  parser->repeat_body_.clear();
}

void HandleIrpcDirective(const std::string &label, const std::string &operand, 
                         DirectiveContext &ctx) {
  (void)label;
  (void)ctx;
  
  auto* parser = GetParser(ctx);
  if (!parser) {
    throw std::runtime_error("IRPC: Parser context not available");
  }
  
  // Parse: IRPC param, string (or <string>)
  size_t comma_pos = operand.find(',');
  if (comma_pos == std::string::npos) {
    throw std::runtime_error("IRPC requires parameter name and string");
  }
  
  // Extract parameter name
  std::string param_name = Trim(operand.substr(0, comma_pos));
  if (param_name.empty()) {
    throw std::runtime_error("IRPC parameter name cannot be empty");
  }
  
  // Extract string
  std::string str = Trim(operand.substr(comma_pos + 1));
  
  // Remove angle brackets if present
  if (!str.empty() && str.front() == '<' && str.back() == '>') {
    str = str.substr(1, str.size() - 2);
  }
  
  // Start capturing IRPC block
  parser->in_repeat_block_ = EdtasmM80PlusPlusSyntaxParser::RepeatType::IRPC;
  parser->repeat_param_ = param_name;
  parser->repeat_values_ = {str};
  parser->repeat_body_.clear();
}

// ============================================================================
// Listing Control Handlers
// ============================================================================

void HandleListDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Enable listing and create ListingControlAtom
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->EnableListing();
  }
  
  auto list_atom = std::make_shared<ListingControlAtom>(ListingControlType::List);
  list_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  list_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(list_atom);
}

void HandleNolistDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Disable listing and create ListingControlAtom
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->DisableListing();
  }
  
  auto nolist_atom = std::make_shared<ListingControlAtom>(ListingControlType::Nolist);
  nolist_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  nolist_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(nolist_atom);
}

void HandleStarListDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  std::string op = util::Trim(operand);
  (void)label;
  std::transform(op.begin(), op.end(), op.begin(), ::toupper);
  
  auto* parser = GetParser(ctx);
  
  if (op == "ON") {
    if (parser) {
      parser->EnableListing();
    }
    auto list_atom = std::make_shared<ListingControlAtom>(ListingControlType::List);
    list_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    list_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(list_atom);
  } else if (op == "OFF") {
    if (parser) {
      parser->DisableListing();
    }
    auto nolist_atom = std::make_shared<ListingControlAtom>(ListingControlType::Nolist);
    nolist_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
    nolist_atom->source_line = ctx.source_line;
    ctx.section->atoms.push_back(nolist_atom);
  }
  // Silently ignore invalid operands for compatibility
}

void HandleLallDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Create ListingControlAtom for list-all mode
  auto lall_atom = std::make_shared<ListingControlAtom>(ListingControlType::Lall);
  lall_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  lall_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(lall_atom);
}

void HandleSallDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Create ListingControlAtom for suppress-all mode
  auto sall_atom = std::make_shared<ListingControlAtom>(ListingControlType::Sall);
  sall_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  sall_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(sall_atom);
}

void HandleTitleDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  std::string title = util::Trim(operand);
  (void)label;
  
  // Update parser state
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->SetListingTitle(title);
  }
  
  // Create ListingControlAtom so listing output can see it
  auto title_atom = std::make_shared<ListingControlAtom>(ListingControlType::Title, title);
  title_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  title_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(title_atom);
}

void HandleSubttlDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  std::string subtitle = util::Trim(operand);
  (void)label;
  
  // Update parser state
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->SetListingSubtitle(subtitle);
  }
  
  // Create ListingControlAtom so listing output can see it
  auto subtitle_atom = std::make_shared<ListingControlAtom>(ListingControlType::Subtitle, subtitle);
  subtitle_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  subtitle_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(subtitle_atom);
}

void HandleEjectDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Create ListingControlAtom for page eject
  auto eject_atom = std::make_shared<ListingControlAtom>(ListingControlType::Page);
  eject_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  eject_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(eject_atom);
}

void HandleSpaceDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  // Parse number of blank lines (default to 1)
  (void)label;
  int count = 1;
  if (!operand.empty()) {
    try {
      count = std::stoi(util::Trim(operand));
    } catch (...) {
      count = 1; // Default on parse error
    }
  }
  
  // Create ListingControlAtom for spacing
  auto space_atom = std::make_shared<ListingControlAtom>(
      ListingControlType::Space, std::to_string(count));
  space_atom->location = SourceLocation(ctx.current_file, ctx.current_line, 1);
  space_atom->source_line = ctx.source_line;
  ctx.section->atoms.push_back(space_atom);
}

void HandleNameDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)ctx;
  (void)label;
  
  std::string name = util::Trim(operand);
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->SetModuleName(name);
  }
}

void HandleRadixDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)ctx;
  (void)label;
  
  int radix = std::stoi(util::Trim(operand));
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->SetRadix(radix);  // Validates range internally
  }
}

// ============================================================================
// Miscellaneous Handlers
// ============================================================================

void HandleEndDirective(const std::string &label, const std::string &operand, DirectiveContext &ctx) {
  (void)operand;
  (void)label;
  
  // Mark that END directive has been seen
  auto* parser = GetParser(ctx);
  if (parser) {
    parser->MarkEndDirectiveSeen();
  }
}

// ============================================================================
// Registration Function
// ============================================================================

void RegisterEdtasmDirectiveHandlers(DirectiveRegistry &registry) {
  using namespace xasm::directives;
  
  // Segment control directives (no-ops for now)
  registry.Register(ASEG, HandleAsegDirective);
  
  registry.Register(CSEG, HandleCsegDirective);
  
  registry.Register(DSEG, HandleDsegDirective);
  
  // Conditional assembly directives
  registry.Register(IF, HandleIfDirective);
  
  registry.Register(ELSE, HandleElseDirective);
  
  registry.Register(ENDIF, HandleEndifDirective);
  
  registry.Register(IFDEF, HandleIfdefDirective);
  
  registry.Register(IFNDEF, HandleIfndefDirective);
  
  registry.Register(IFEQ, HandleIfeqDirective);
  
  registry.Register(IFNE, HandleIfneDirective);
  
  registry.Register(IFLT, HandleIfltDirective);
  
  registry.Register(IFLE, HandleIfleDirective);
  
  registry.Register(IFGT, HandleIfgtDirective);
  
  registry.Register(IFGE, HandleIfgeDirective);
  
  // Listing control directives
  registry.Register({LIST, DOT_LIST}, HandleListDirective);
  
  registry.Register(NOLIST, HandleNolistDirective);
  
  registry.Register(STAR_LIST, HandleStarListDirective);
  
  registry.Register(LALL, HandleLallDirective);

  registry.Register(SALL, HandleSallDirective);
  
  registry.Register(TITLE, HandleTitleDirective);
  
  registry.Register(EJECT, HandleEjectDirective);
  
  registry.Register(SPACE, HandleSpaceDirective);
  
  // Miscellaneous directives
  registry.Register(NAME, HandleNameDirective);
  
  registry.Register({RADIX, STAR_RADIX}, HandleRadixDirective);
  
  registry.Register(END, HandleEndDirective);
}

} // namespace xasm
