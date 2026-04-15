/**
 * @file scmasm_symbol_table.h
 * @brief Case-insensitive symbol table wrapper for SCMASM
 *
 * SCMASM traditionally uses case-insensitive symbol lookup for compatibility
 * with A2osX API constants. This wrapper normalizes symbol names to uppercase
 * for both definition and lookup.
 */

#pragma once

#include <string>

#include "xasm++/symbol.h"
#include "xasm++/util/string_utils.h"

namespace xasm {

/**
 * @brief Case-insensitive symbol table for SCMASM
 *
 * Wraps ConcreteSymbolTable and normalizes all symbol names to uppercase
 * before Define/Lookup operations. This ensures case-insensitive behavior
 * for SCMASM compatibility.
 */
class ScmasmSymbolTable : public SymbolTable {
 public:
  explicit ScmasmSymbolTable() = default;

  bool Lookup(const std::string& name, int64_t& value) const override {
    return table_.Lookup(util::ToUpper(name), value);
  }

  void Define(const std::string& name, SymbolType type,
              std::shared_ptr<Expression> value) override {
    table_.Define(util::ToUpper(name), type, value);
  }

  const Symbol* GetSymbol(const std::string& name) const override {
    return table_.GetSymbol(util::ToUpper(name));
  }

  Symbol* GetSymbol(const std::string& name) override {
    return table_.GetSymbol(util::ToUpper(name));
  }

  void GetAll(std::vector<const Symbol*>& symbols) const override { table_.GetAll(symbols); }

  void Clear() override { table_.Clear(); }

  /**
   * @brief Get underlying concrete symbol table (for testing/debugging)
   */
  ConcreteSymbolTable& GetConcreteTable() { return table_; }

  const ConcreteSymbolTable& GetConcreteTable() const { return table_; }

 private:
  ConcreteSymbolTable table_;
};

}  // namespace xasm
