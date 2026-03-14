// Symbol implementation

#include "xasm++/symbol.h"
#include <algorithm>
#include <cctype>

namespace xasm {

// ConcreteSymbolTable implementation

void ConcreteSymbolTable::Define(const std::string &name, SymbolType type,
                                 std::shared_ptr<Expression> value) {
  // Preserve export/import flags if symbol already exists
  bool was_exported = false;
  bool was_imported = false;
  auto it = symbols_.find(name);
  if (it != symbols_.end()) {
    was_exported = it->second.is_exported;
    was_imported = it->second.is_imported;
  }

  symbols_[name] = Symbol(name, type, value);

  // Restore flags
  symbols_[name].is_exported = was_exported;
  symbols_[name].is_imported = was_imported;
}

void ConcreteSymbolTable::DefineLabel(const std::string &name, int64_t value) {
  Define(name, SymbolType::Label, std::make_shared<LiteralExpr>(value));
}

bool ConcreteSymbolTable::Lookup(const std::string &name,
                                 int64_t &value) const {
  auto it = symbols_.find(name);
  if (it != symbols_.end()) {
    value = it->second.value->Evaluate(*this);
    return true;
  }
  // ADR-005 V1: SCMASM uppercase fallback — when enabled, retry lookup with
  // the fully-uppercased name.  SCMASM normalises symbols to UPPERCASE at
  // definition time, so "TmpPtr2" must resolve to "TMPPTR2".
  if (uppercase_fallback_) {
    std::string upper = name;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    if (upper != name) {
      auto it2 = symbols_.find(upper);
      if (it2 != symbols_.end()) {
        value = it2->second.value->Evaluate(*this);
        return true;
      }
    }
  }
  return false;
}

const Symbol *ConcreteSymbolTable::GetSymbol(const std::string &name) const {
  auto it = symbols_.find(name);
  if (it != symbols_.end()) {
    return &it->second;
  }
  return nullptr;
}

Symbol *ConcreteSymbolTable::GetSymbol(const std::string &name) {
  auto it = symbols_.find(name);
  if (it != symbols_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool ConcreteSymbolTable::IsDefined(const std::string &name) const {
  return symbols_.find(name) != symbols_.end();
}

const std::unordered_map<std::string, Symbol> &
ConcreteSymbolTable::GetAllSymbols() const {
  return symbols_;
}

std::vector<std::string> ConcreteSymbolTable::GetAllSymbolNames() const {
  std::vector<std::string> names;
  names.reserve(symbols_.size());
  for (const auto &pair : symbols_) {
    names.push_back(pair.first);
  }
  return names;
}

void ConcreteSymbolTable::Clear() {
  symbols_.clear();
  current_location_ = 0;
}

int64_t ConcreteSymbolTable::GetCurrentLocation() const {
  return current_location_;
}

void ConcreteSymbolTable::SetCurrentLocation(int64_t location) {
  current_location_ = location;
}

} // namespace xasm
