// Symbol implementation

#include "xasm++/symbol.h"

namespace xasm {

// ConcreteSymbolTable implementation

void ConcreteSymbolTable::Define(const std::string& name, SymbolType type,
                                  std::shared_ptr<Expression> value) {
    symbols_[name] = Symbol(name, type, value);
}

bool ConcreteSymbolTable::Lookup(const std::string& name, int64_t& value) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        value = it->second.value->Evaluate(*this);
        return true;
    }
    return false;
}

const Symbol* ConcreteSymbolTable::GetSymbol(const std::string& name) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

Symbol* ConcreteSymbolTable::GetSymbol(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ConcreteSymbolTable::IsDefined(const std::string& name) const {
    return symbols_.find(name) != symbols_.end();
}

const std::unordered_map<std::string, Symbol>& ConcreteSymbolTable::GetAllSymbols() const {
    return symbols_;
}

void ConcreteSymbolTable::Clear() {
    symbols_.clear();
}

} // namespace xasm
