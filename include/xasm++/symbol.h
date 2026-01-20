// Symbol - Symbol table and symbol management
// Phase 1: Minimal Viable Assembler - Core Abstractions

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "xasm++/atom.h"      // For SourceLocation
#include "xasm++/expression.h"

namespace xasm {

// Forward declarations
class Section;

// Symbol types
enum class SymbolType {
    Label,      // Address label
    Equate,     // EQU constant
    Set,        // SET variable (can be redefined)
    // Macro will be added in later phases
};

// Symbol definition
class Symbol {
public:
    std::string name;
    SymbolType type;
    std::shared_ptr<Expression> value;
    Section* section;               // nullptr for absolute symbols
    bool is_exported;
    bool is_imported;
    SourceLocation definition;

    // Default constructor (needed for std::unordered_map)
    Symbol()
        : name(), type(SymbolType::Label), value(nullptr), section(nullptr),
          is_exported(false), is_imported(false) {}

    Symbol(const std::string& n, SymbolType t, std::shared_ptr<Expression> val)
        : name(n), type(t), value(val), section(nullptr),
          is_exported(false), is_imported(false) {}
};

// Concrete symbol table implementation
class ConcreteSymbolTable : public SymbolTable {
public:
    ConcreteSymbolTable() = default;

    // Define a symbol
    void Define(const std::string& name, SymbolType type, std::shared_ptr<Expression> value) {
        symbols_[name] = Symbol(name, type, value);
    }

    // Lookup a symbol value
    bool Lookup(const std::string& name, int64_t& value) const override {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            value = it->second.value->Evaluate(*this);
            return true;
        }
        return false;
    }

    // Get symbol by name (returns nullptr if not found)
    const Symbol* GetSymbol(const std::string& name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Get symbol by name (mutable version)
    Symbol* GetSymbol(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Check if symbol exists
    bool IsDefined(const std::string& name) const {
        return symbols_.find(name) != symbols_.end();
    }

    // Get all symbols (for iteration/listing)
    const std::unordered_map<std::string, Symbol>& GetAllSymbols() const {
        return symbols_;
    }

    // Clear all symbols
    void Clear() {
        symbols_.clear();
    }

private:
    std::unordered_map<std::string, Symbol> symbols_;
};

} // namespace xasm
