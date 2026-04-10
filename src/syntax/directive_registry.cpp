/**
 * @file directive_registry.cpp
 * @brief Implementation of DirectiveRegistry
 */

#include "xasm++/syntax/directive_registry.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {

void DirectiveRegistry::Register(const std::string &mnemonic, // NOLINT(readability-convert-member-functions-to-static)
                                 DirectiveHandler handler) {
  std::string upper = ToUpper(mnemonic);
  handlers_[upper] = handler;
}

void DirectiveRegistry::Register(std::initializer_list<std::string> mnemonics,
                                 DirectiveHandler handler) {
  // Register the same handler for all mnemonics (aliases)
  for (const auto &mnemonic : mnemonics) {
    Register(mnemonic, handler);
  }
}

void DirectiveRegistry::Execute(const std::string &mnemonic, // NOLINT(readability-convert-member-functions-to-static)
                                const std::string &label,
                                const std::string &operand,
                                DirectiveContext &context) {
  std::string upper = ToUpper(mnemonic);

  auto it = handlers_.find(upper);
  if (it == handlers_.end()) {
    throw std::runtime_error("Unknown directive: " + mnemonic);
  }

  // Populate context.label so handlers can access it without an extra parameter
  context.label = label;

  // Execute the handler
  it->second(operand, context);
}

bool DirectiveRegistry::IsRegistered(const std::string &mnemonic) const { // NOLINT(readability-convert-member-functions-to-static)
  std::string upper = ToUpper(mnemonic);
  return handlers_.contains(upper);
}

std::string DirectiveRegistry::ToUpper(const std::string &mnemonic) { // NOLINT(readability-convert-member-functions-to-static)
  std::string result;
  result.reserve(mnemonic.size());
  for (char c : mnemonic) {
    result += std::toupper(static_cast<unsigned char>(c));
  }
  return result;
}

} // namespace xasm
