/**
 * @file symbol.h
 * @brief Symbol table and symbol management
 *
 * This file defines the symbol system for managing labels, constants,
 * and variables during assembly. Symbols are resolved across multiple
 * passes to handle forward references.
 *
 * @note Phase 1: Minimal Viable Assembler - Core Abstractions
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xasm++/atom.h" // For SourceLocation
#include "xasm++/expression.h"

namespace xasm {

// Forward declarations
class Section;

/**
 * @brief Types of symbols in the symbol table
 *
 * Different symbol types have different semantics regarding
 * definition, redefinition, and relocation.
 */
enum class SymbolType {
  Label,  ///< Address label (e.g., "start:") - cannot be redefined
  Equate, ///< EQU constant (e.g., "SCREEN = $400") - cannot be redefined
  Set,    ///< SET variable (e.g., "counter .set 0") - can be redefined
          // Macro will be added in later phases
};

/**
 * @brief Represents a single symbol definition
 *
 * A symbol associates a name with a value (typically an address or constant).
 * Symbols can be labels, equates, or set variables, each with different
 * semantics regarding redefinition and relocation.
 *
 * @par Example
 * @code
 * start:           ; Label symbol (address)
 * SCREEN = $400    ; Equate symbol (constant)
 * counter .set 0   ; Set symbol (variable)
 * @endcode
 */
class Symbol {
public:
  std::string name;                  ///< Symbol name
  SymbolType type;                   ///< Symbol type (label, equate, set)
  std::shared_ptr<Expression> value; ///< Symbol value (expression tree)
  Section *section; ///< Section where defined (nullptr for absolute symbols)
  bool is_exported; ///< True if exported to other modules
  bool is_imported; ///< True if imported from another module
  SourceLocation definition; ///< Source location where defined

  /**
   * @brief Default constructor
   *
   * Creates an invalid symbol. Needed for std::unordered_map.
   */
  Symbol()
      : name(), type(SymbolType::Label), value(nullptr), section(nullptr),
        is_exported(false), is_imported(false) {}

  /**
   * @brief Construct a symbol with name, type, and value
   *
   * @param n Symbol name
   * @param t Symbol type
   * @param val Symbol value (expression tree)
   */
  Symbol(const std::string &n, SymbolType t, std::shared_ptr<Expression> val)
      : name(n), type(t), value(val), section(nullptr), is_exported(false),
        is_imported(false) {}
};

/**
 * @brief Concrete symbol table implementation
 *
 * The symbol table stores all symbols defined during assembly and provides
 * lookup operations for symbol resolution. It implements the abstract
 * SymbolTable interface required by the Expression system.
 *
 * @par Thread Safety
 * This class is not thread-safe. External synchronization is required
 * if accessed from multiple threads.
 *
 * @par Example
 * @code
 * ConcreteSymbolTable symbols;
 *
 * // Define a label at address 0x8000
 * symbols.Define("start", SymbolType::Label,
 *                std::make_shared<LiteralExpr>(0x8000));
 *
 * // Look up the label
 * int64_t addr;
 * if (symbols.Lookup("start", addr)) {
 *     std::cout << "start = " << std::hex << addr << "\n";
 * }
 * @endcode
 */
class ConcreteSymbolTable : public SymbolTable {
public:
  /**
   * @brief Default constructor - creates an empty symbol table
   */
  ConcreteSymbolTable() = default;

  /**
   * @brief Define a new symbol
   *
   * Adds or updates a symbol in the table. For Label and Equate symbols,
   * redefinition may be an error (enforced by parser). For Set symbols,
   * redefinition is allowed.
   *
   * @param name Symbol name
   * @param type Symbol type (Label, Equate, or Set)
   * @param value Symbol value (expression tree)
   */
  void Define(const std::string &name, SymbolType type,
              std::shared_ptr<Expression> value);

  /**
   * @brief Define a label with a numeric value (convenience method)
   *
   * Helper method for defining simple labels with integer values.
   * Wraps the value in a LiteralExpr.
   *
   * @param name Label name
   * @param value Numeric value for the label
   */
  void DefineLabel(const std::string &name, int64_t value);

  /**
   * @brief Look up a symbol value
   *
   * Evaluates the symbol's expression and returns its numeric value.
   * This is part of the SymbolTable interface used by expressions.
   *
   * @param name Symbol name to look up
   * @param value Output parameter - receives the symbol's value if found
   * @return true if symbol was found and evaluated, false otherwise
   */
  bool Lookup(const std::string &name, int64_t &value) const override;

  /**
   * @brief Get a symbol by name (const version)
   *
   * Returns a pointer to the symbol object, allowing access to all
   * symbol metadata (type, value, section, etc.).
   *
   * @param name Symbol name to look up
   * @return Pointer to symbol if found, nullptr otherwise
   */
  const Symbol *GetSymbol(const std::string &name) const;

  /**
   * @brief Get a symbol by name (mutable version)
   *
   * Returns a mutable pointer to the symbol object.
   *
   * @param name Symbol name to look up
   * @return Pointer to symbol if found, nullptr otherwise
   */
  Symbol *GetSymbol(const std::string &name);

  /**
   * @brief Check if a symbol is defined
   *
   * @param name Symbol name to check
   * @return true if symbol exists in the table, false otherwise
   */
  bool IsDefined(const std::string &name) const;

  /**
   * @brief Get all symbols in the table
   *
   * Returns the internal symbol map for iteration or listing generation.
   *
   * @return Const reference to the symbol map
   */
  const std::unordered_map<std::string, Symbol> &GetAllSymbols() const;

  /**
   * @brief Get all symbol names
   *
   * Returns a vector containing just the names of all defined symbols.
   * Useful for symbol suggestion algorithms.
   *
   * @return Vector of symbol names
   */
  std::vector<std::string> GetAllSymbolNames() const;

  /**
   * @brief Clear all symbols from the table
   *
   * Removes all symbol definitions. Used when resetting the assembler
   * for a new assembly operation.
   */
  void Clear();

  /**
   * @brief Get the current assembly location counter
   *
   * Returns the current address during assembly. This is used by the
   * current location operator ($) in expressions.
   *
   * @return Current assembly address
   */
  int64_t GetCurrentLocation() const override;

  /**
   * @brief Set the current assembly location counter
   *
   * Updates the current address during assembly. This should be called
   * as atoms are processed and sized.
   *
   * @param location New assembly address
   */
  void SetCurrentLocation(int64_t location);

private:
  std::unordered_map<std::string, Symbol> symbols_; ///< Internal symbol storage
  int64_t current_location_ = 0; ///< Current assembly address for $ operator
};

} // namespace xasm
