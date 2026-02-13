/**
 * @file merlin_directive_handlers.cpp
 * @brief Merlin directive handler implementations
 *
 * Free function implementations of Merlin directive handlers.
 * Part of Phase 6c - Directive handler extraction refactoring.
 */

#include "xasm++/directives/merlin_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/expression.h"
#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/util/string_utils.h"
#include <stdexcept>

namespace xasm {
namespace merlin {

using xasm::util::ToUpper;
using xasm::util::Trim;

// ============================================================================
// Directive Handlers
// ============================================================================

void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleOrg(operand, *context.section, *context.symbols);
}

void HandleEqu(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)context.section;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleEqu(label, operand, *context.symbols);
}

void HandleDb(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleDB(operand, *context.section, *context.symbols);
}

void HandleDw(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleDW(operand, *context.section, *context.symbols);
}

void HandleHex(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleHex(operand, *context.section);
}

void HandleDs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
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
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleDum(operand, *context.symbols);
}

void HandleDend(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleDend();
}

void HandlePut(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandlePut(operand, *context.section, *context.symbols);
}

void HandleDo(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleDo(operand, *context.symbols);
}

void HandleElse(const std::string &label, const std::string &operand,
                DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleElse();
}

void HandleFin(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleFin();
}

void HandleLst(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleLst(operand);
}

void HandleLstdo(const std::string &label, const std::string &operand,
                 DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleLstdo();
}

void HandleTr(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleTr(operand);
}

void HandleAsc(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleAsc(operand, *context.section);
}

void HandleDci(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleDCI(operand, *context.section);
}

void HandleInv(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleINV(operand, *context.section);
}

void HandleFls(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleFLS(operand, *context.section);
}

void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  // Create label atom first if label present
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleDA(operand, *context.section, *context.symbols);
}

void HandlePmc(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandlePMC(label.empty() ? operand : label);
}

void HandleEom(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)operand;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleEOM();
}

void HandleMac(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
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
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  // USR is a no-op - no atoms generated
}

void HandleEnd(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)operand;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  if (!label.empty()) {
    uint32_t current_address = parser->GetCurrentAddress();
    context.symbols->Define(label, SymbolType::Label,
                            std::make_shared<LiteralExpr>(current_address));
    context.section->atoms.push_back(
        std::make_shared<LabelAtom>(label, current_address));
    parser->SetGlobalLabel(label);
  }
  
  parser->HandleEnd();
}

void HandleSav(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleSav(operand);
}

void HandleXc(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleXc(operand);
}

void HandleMx(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleMx(operand);
}

void HandleRev(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  if (label.empty()) {
    throw std::runtime_error("REV requires a label");
  }
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleRev(label, operand, *context.section, *context.symbols);
}

void HandleLup(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label;
  (void)context.section;
  (void)context.symbols;
  
  // Get parser instance from context
  auto *parser = static_cast<MerlinSyntaxParser *>(context.parser_state);
  if (!parser) {
    throw std::runtime_error("Internal error: parser_state is null");
  }
  
  parser->HandleLup(operand);
}

} // namespace merlin
} // namespace xasm
