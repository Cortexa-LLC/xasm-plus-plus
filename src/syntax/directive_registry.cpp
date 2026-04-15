/**
 * @file directive_registry.cpp
 * @brief Implementation of DirectiveRegistry
 */

#include "xasm++/syntax/directive_registry.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace xasm {

void DirectiveRegistry::Register(const std::string& mnemonic, DirectiveHandler handler) {
  std::string upper = ToUpper(mnemonic);
  handlers_[upper] = handler;
}

void DirectiveRegistry::Register(std::initializer_list<std::string> mnemonics,
                                 DirectiveHandler handler) {
  // Register the same handler for all mnemonics (aliases)
  for (const auto& mnemonic : mnemonics) {
    Register(mnemonic, handler);
  }
}

void DirectiveRegistry::Execute(DirectiveContext& context) {
  std::string upper = ToUpper(context.mnemonic);

  auto it = handlers_.find(upper);
  if (it == handlers_.end()) {
    throw std::runtime_error("Unknown directive: " + context.mnemonic);
  }

  // Normalize mnemonic to uppercase
  context.mnemonic = upper;

  // Execute the handler (mnemonic, label, operand are already in context)
  it->second(context);
}

bool DirectiveRegistry::IsRegistered(const std::string& mnemonic) const {
  std::string upper = ToUpper(mnemonic);
  return handlers_.contains(upper);
}

std::string DirectiveRegistry::ToUpper(const std::string& mnemonic) {
  std::string result;
  result.reserve(mnemonic.size());
  for (char c : mnemonic) {
    result += std::toupper(static_cast<unsigned char>(c));
  }
  return result;
}

}  // namespace xasm
