/**
 * @file core_directive_handlers.cpp
 * @brief Implementation of core directive handlers
 *
 * Free function implementations for common assembly directives.
 * Part of God-Class Phase 6b refactoring.
 */

#include "xasm++/syntax/core_directive_handlers.h"
#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
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
  auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
    return std::isspace(ch);
  });
  auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
               return std::isspace(ch);
             }).base();

  return (start < end) ? std::string(start, end) : std::string();
}

/**
 * @brief Format error message with file:line: prefix
 */
std::string FormatError(const DirectiveContext &ctx,
                        const std::string &message) {
  if (!ctx.current_file.empty() && ctx.current_line > 0) {
    return ctx.current_file + ":" + std::to_string(ctx.current_line) + ": " +
           message;
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

void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  (void)label; // ORG doesn't use label
  std::string op = Trim(operand);

  if (op.empty()) {
    std::string error = std::string(directives::errors::ORG_PREFIX) +
                        directives::errors::MISSING_ADDRESS;
    error = FormatError(context, error);
    throw std::runtime_error(error);
  }

  // Parse address (number or symbol)
  auto expr = ParseExpression(op, *context.symbols);
  int64_t address = expr->Evaluate(*context.symbols);

  if (address < 0) {
    std::string error = std::string(directives::errors::ORG_PREFIX) +
                        directives::errors::NEGATIVE_ADDRESS + ": " +
                        std::to_string(address);
    error = FormatError(context, error);
    throw std::runtime_error(error);
  }

  // Create OrgAtom and update address
  context.section->atoms.push_back(
      std::make_shared<OrgAtom>(static_cast<uint32_t>(address)));
  *context.current_address = static_cast<uint32_t>(address);
}

void HandleEqu(const std::string &label, const std::string &operand,
               DirectiveContext &context) {
  std::string lbl = Trim(label);
  std::string op = Trim(operand);

  if (lbl.empty()) {
    std::string error = std::string(directives::errors::EQU_PREFIX) +
                        directives::errors::MISSING_LABEL;
    error = FormatError(context, error);
    throw std::runtime_error(error);
  }

  // Parse value expression
  auto expr = ParseExpression(op, *context.symbols);

  // Define symbol with expression
  context.symbols->Define(lbl, SymbolType::Equate, expr);
}

void HandleDb(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // DB doesn't use label (could be used for auto-label feature)
  std::string op = Trim(operand);

  // Split by commas
  std::vector<std::string> expressions;
  if (!op.empty()) {
    expressions = SplitByComma(op);
  }

  // Create DataAtom with byte size
  auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Byte);

  context.section->atoms.push_back(data_atom);

  // Advance address by number of bytes
  *context.current_address += static_cast<uint32_t>(expressions.size());
}

void HandleDw(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // DW doesn't use label (could be used for auto-label feature)
  std::string op = Trim(operand);

  // Split by commas
  std::vector<std::string> expressions;
  if (!op.empty()) {
    expressions = SplitByComma(op);
  }

  // Create DataAtom with word size
  auto data_atom = std::make_shared<DataAtom>(expressions, DataSize::Word);

  context.section->atoms.push_back(data_atom);

  // Advance address by number of words * 2
  *context.current_address += static_cast<uint32_t>(expressions.size() * 2);
}

void HandleDs(const std::string &label, const std::string &operand,
              DirectiveContext &context) {
  (void)label; // DS doesn't use label (could be used for auto-label feature)
  std::string op = Trim(operand);

  uint32_t count = 0;

  if (!op.empty()) {
    // Parse count expression
    auto expr = ParseExpression(op, *context.symbols);
    int64_t value = expr->Evaluate(*context.symbols);

    if (value < 0) {
      std::string error = std::string(directives::errors::DS_PREFIX) +
                          directives::errors::NEGATIVE_COUNT + ": " +
                          std::to_string(value);
      error = FormatError(context, error);
      throw std::runtime_error(error);
    }

    count = static_cast<uint32_t>(value);
  }

  // Create SpaceAtom
  context.section->atoms.push_back(std::make_shared<SpaceAtom>(count));

  // Advance address
  *context.current_address += count;
}

// ============================================================================
// DirectiveRegistry Integration
// ============================================================================

void RegisterCoreDirectiveHandlers(DirectiveRegistry &registry) {
  // ORG directive - Set origin address
  registry.Register(directives::ORG, HandleOrg);

  // EQU directive - Define constant symbol
  registry.Register(directives::EQU, HandleEqu);

  // DB directive and aliases - Define byte data
  registry.Register({directives::DB, directives::DEFB, directives::BYTE},
                    HandleDb);

  // DW directive and aliases - Define word data
  registry.Register({directives::DW, directives::DEFW, directives::WORD},
                    HandleDw);

  // DS directive and aliases - Define space
  registry.Register(
      {directives::DS, directives::DEFS, directives::BLOCK, directives::RMB},
      HandleDs);
}

} // namespace xasm
