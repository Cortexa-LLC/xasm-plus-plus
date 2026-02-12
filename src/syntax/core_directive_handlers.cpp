/**
 * @file core_directive_handlers.cpp
 * @brief Implementation of core directive handlers
 *
 * Free function implementations for common assembly directives.
 * Part of God-Class Phase 6b refactoring.
 */

#include "xasm++/syntax/core_directive_handlers.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/atom.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include "xasm++/common/expression_parser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace xasm {

// Helper functions (internal linkage)
namespace {

/**
 * @brief Trim whitespace from both ends of a string
 */
std::string Trim(const std::string &str) {
  auto start = std::find_if_not(str.begin(), str.end(),
                                 [](unsigned char ch) { return std::isspace(ch); });
  auto end = std::find_if_not(str.rbegin(), str.rend(),
                               [](unsigned char ch) { return std::isspace(ch); })
                 .base();

  return (start < end) ? std::string(start, end) : std::string();
}

/**
 * @brief Format error message with file:line: prefix
 */
std::string FormatError(const DirectiveContext &ctx, const std::string &message) {
  if (!ctx.current_file.empty() && ctx.current_line > 0) {
    return ctx.current_file + ":" + std::to_string(ctx.current_line) + ": " + message;
  }
  return message;
}

/**
 * @brief Parse a number from string (supports decimal, $hex, %binary)
 */
/**
 * @brief Parse an expression from string
 */
std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                           ConcreteSymbolTable &symbols) {
  std::string trimmed = Trim(str);

  if (trimmed.empty()) {
    throw std::runtime_error("Empty expression");
  }

  // Use the full expression parser to handle literals, symbols, and arithmetic
  ExpressionParser parser(&symbols);
  return parser.Parse(trimmed);
}

/**
 * @brief Split string by comma delimiter, trim each part
 */
std::vector<std::string> SplitByComma(const std::string &str) {
  std::vector<std::string> result;
  std::string current;
  std::istringstream stream(str);

  while (std::getline(stream, current, ',')) {
    std::string trimmed = Trim(current);
    if (!trimmed.empty()) {
      result.push_back(trimmed);
    }
  }

  return result;
}

} // anonymous namespace

// ============================================================================
// Public Handler Functions
// ============================================================================

void HandleOrgDirective(const std::string &operand, Section &section,
                        ConcreteSymbolTable &symbols, uint32_t &current_address,
                        const DirectiveContext *ctx) {
  std::string op = Trim(operand);

  if (op.empty()) {
    std::string error = std::string(directives::errors::ORG_PREFIX) +
                        directives::errors::MISSING_ADDRESS;
    if (ctx) {
      error = FormatError(*ctx, error);
    }
    throw std::runtime_error(error);
  }

  // Parse address (number or symbol)
  auto expr = ParseExpression(op, symbols);
  int64_t address = expr->Evaluate(symbols);

  if (address < 0) {
    std::string error = std::string(directives::errors::ORG_PREFIX) +
                        directives::errors::NEGATIVE_ADDRESS + ": " +
                        std::to_string(address);
    if (ctx) {
      error = FormatError(*ctx, error);
    }
    throw std::runtime_error(error);
  }

  // Create OrgAtom and update address
  section.atoms.push_back(std::make_shared<OrgAtom>(static_cast<uint32_t>(address)));
  current_address = static_cast<uint32_t>(address);
}

void HandleEquDirective(const std::string &label, const std::string &operand,
                        ConcreteSymbolTable &symbols, const DirectiveContext *ctx) {
  std::string lbl = Trim(label);
  std::string op = Trim(operand);

  if (lbl.empty()) {
    std::string error = std::string(directives::errors::EQU_PREFIX) +
                        directives::errors::MISSING_LABEL;
    if (ctx) {
      error = FormatError(*ctx, error);
    }
    throw std::runtime_error(error);
  }

  // Parse value expression
  auto expr = ParseExpression(op, symbols);

  // Define symbol with expression
  symbols.Define(lbl, SymbolType::Equate, expr);
}

void HandleDbDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address) {
  (void)symbols;  // May be used for expression evaluation in future
  std::string op = Trim(operand);

  // Split by commas
  std::vector<std::string> expressions;
  if (!op.empty()) {
    expressions = SplitByComma(op);
  }

  // Create DataAtom with byte size
  auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Byte);

  section.atoms.push_back(data_atom);

  // Advance address by number of bytes
  current_address += static_cast<uint32_t>(expressions.size());
}

void HandleDwDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address) {
  (void)symbols;  // May be used for expression evaluation in future
  std::string op = Trim(operand);

  // Split by commas
  std::vector<std::string> expressions;
  if (!op.empty()) {
    expressions = SplitByComma(op);
  }

  // Create DataAtom with word size
  auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Word);

  section.atoms.push_back(data_atom);

  // Advance address by number of words * 2
  current_address += static_cast<uint32_t>(expressions.size() * 2);
}

void HandleDsDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address,
                       const DirectiveContext *ctx) {
  std::string op = Trim(operand);

  uint32_t count = 0;

  if (!op.empty()) {
    // Parse count expression
    auto expr = ParseExpression(op, symbols);
    int64_t value = expr->Evaluate(symbols);

    if (value < 0) {
      std::string error = std::string(directives::errors::DS_PREFIX) +
                          directives::errors::NEGATIVE_COUNT + ": " +
                          std::to_string(value);
      if (ctx) {
        error = FormatError(*ctx, error);
      }
      throw std::runtime_error(error);
    }

    count = static_cast<uint32_t>(value);
  }

  // Create SpaceAtom
  section.atoms.push_back(std::make_shared<SpaceAtom>(count));

  // Advance address
  current_address += count;
}

// ============================================================================
// DirectiveRegistry Integration
// ============================================================================

void RegisterCoreDirectiveHandlers(DirectiveRegistry &registry) {
  // ORG directive - Set origin address
  registry.Register(directives::ORG, 
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // ORG doesn't use label
      HandleOrgDirective(operand, *ctx.section, *ctx.symbols, *ctx.current_address, &ctx);
    });

  // EQU directive - Define constant symbol
  registry.Register(directives::EQU,
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      HandleEquDirective(label, operand, *ctx.symbols, &ctx);
    });

  // DB directive and aliases - Define byte data
  registry.Register({directives::DB, directives::DEFB, directives::BYTE},
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // DB doesn't use label (could be used for auto-label feature)
      HandleDbDirective(operand, *ctx.section, *ctx.symbols, *ctx.current_address);
    });

  // DW directive and aliases - Define word data
  registry.Register({directives::DW, directives::DEFW, directives::WORD},
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // DW doesn't use label (could be used for auto-label feature)
      HandleDwDirective(operand, *ctx.section, *ctx.symbols, *ctx.current_address);
    });

  // DS directive and aliases - Define space
  registry.Register({directives::DS, directives::DEFS, directives::BLOCK, directives::RMB},
    [](const std::string &label, const std::string &operand, DirectiveContext &ctx) {
      (void)label; // DS doesn't use label (could be used for auto-label feature)
      HandleDsDirective(operand, *ctx.section, *ctx.symbols, *ctx.current_address, &ctx);
    });
}

} // namespace xasm
