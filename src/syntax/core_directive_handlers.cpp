/**
 * @file core_directive_handlers.cpp
 * @brief Implementation of core directive handlers
 *
 * Free function implementations for common assembly directives.
 * Part of God-Class Phase 6b refactoring.
 */

#include "xasm++/syntax/core_directive_handlers.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/parser_error_utils.h"

namespace xasm {

// Helper functions (internal linkage)
namespace {

/**
 * @brief Trim whitespace from both ends of a string
 */
std::string Trim(const std::string& str) {
  auto start =
      std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
  auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
               return std::isspace(ch);
             }).base();

  return (start < end) ? std::string(start, end) : std::string();
}

/**
 * @brief Parse a number from string (supports decimal, $hex, %binary)
 */
/**
 * @brief Parse an expression from string
 */
std::shared_ptr<Expression> ParseExpression(const std::string& str, ConcreteSymbolTable& symbols) {
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
std::vector<std::string> SplitByComma(const std::string& str) {
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

}  // anonymous namespace

// ============================================================================
// Registry Initialization
// ============================================================================

void RegisterCoreDirectiveHandlers(DirectiveRegistry& registry) {
  // kORG directive - Set origin address
  registry.Register(directives::kORG, HandleOrg);

  // kEQU directive - Define constant symbol
  registry.Register(directives::kEQU, HandleEqu);

  // kDB directive and aliases - Define byte data
  registry.Register({directives::kDB, directives::kDEFB, directives::kBYTE}, HandleDb);

  // kDW directive and aliases - Define word data
  registry.Register({directives::kDW, directives::kDEFW, directives::kWORD}, HandleDw);

  // kDS directive and aliases - Define space
  registry.Register({directives::kDS, directives::kDEFS, directives::kBLOCK, directives::kRMB},
                    HandleDs);
}

// ============================================================================
// Public Handler Functions
// ============================================================================

void HandleOrg(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)context.label;  // kORG doesn't use context.label
  std::string op = Trim(operand);

  if (op.empty()) {
    ThrowRequiresOperand(context, directives::kORG);
  }

  // Parse address (number or symbol)
  auto expr = ParseExpression(op, *context.symbols);
  int64_t address = expr->Evaluate(*context.symbols);

  if (address < 0) {
    ThrowInvalidValue(context, "ORG address", std::to_string(address), "must be non-negative");
  }

  // Create OrgAtom and update address
  context.section->atoms.push_back(std::make_shared<OrgAtom>(static_cast<uint32_t>(address)));
  *context.current_address = static_cast<uint32_t>(address);
}

void HandleEqu(DirectiveContext& context) {
  const std::string& operand = context.operand;
  std::string lbl = Trim(context.label);
  std::string op = Trim(operand);

  if (lbl.empty()) {
    ThrowRequiresLabel(context, directives::kEQU);
  }

  // Parse value expression
  auto expr = ParseExpression(op, *context.symbols);

  // Define symbol with expression
  context.symbols->Define(lbl, SymbolType::Equate, expr);
}

void HandleDb(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)
      context.label;  // kDB doesn't use context.label (could be used for auto-context.label feature)
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

void HandleDw(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)
      context.label;  // kDW doesn't use context.label (could be used for auto-context.label feature)
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

void HandleDs(DirectiveContext& context) {
  const std::string& operand = context.operand;
  (void)
      context.label;  // kDS doesn't use context.label (could be used for auto-context.label feature)
  std::string op = Trim(operand);

  uint32_t count = 0;

  if (!op.empty()) {
    // Parse count expression
    auto expr = ParseExpression(op, *context.symbols);
    int64_t value = expr->Evaluate(*context.symbols);

    if (value < 0) {
      ThrowInvalidValue(context, "DS count", std::to_string(value), "must be non-negative");
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

}  // namespace xasm
