/**
 * @file directive_registry.h
 * @brief Registry pattern for directive dispatch
 *
 * Replaces if/elseif chains with a registry of handler functions,
 * improving maintainability and extensibility of directive processing.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace xasm {

// Forward declarations
class Section;
class ConcreteSymbolTable;

/**
 * @brief Context for directive execution
 *
 * Bundles the state needed by directive handlers so they don't
 * all need the same long parameter list. Also makes it easy to
 * extend with new context (e.g., parser state) without changing
 * all handler signatures.
 */
struct DirectiveContext {
  Section *section = nullptr;             ///< Current assembly section
  ConcreteSymbolTable *symbols = nullptr; ///< Symbol table
  uint32_t *current_address = nullptr;    ///< Pointer to current address
  void *parser_state = nullptr;           ///< Opaque parser state (for casting)
  
  // Source location tracking for listing output
  std::string current_file;               ///< Current source filename
  int current_line = 0;                   ///< Current line number
  std::string source_line;                ///< Original source line text

  /**
   * @brief Constructor with common context
   */
  DirectiveContext() = default;
};

/**
 * @brief Handler function signature for directive processing
 *
 * @param label Label on the line (if any), empty string if no label
 * @param operand Operand field after directive mnemonic
 * @param context Directive execution context (section, symbols, etc.)
 *
 * @throws std::runtime_error on directive processing errors
 */
using DirectiveHandler = std::function<void(const std::string &label,
                                            const std::string &operand,
                                            DirectiveContext &context)>;

/**
 * @brief Registry for directive handlers
 *
 * Maps directive mnemonics to handler functions using std::function.
 * Provides O(1) lookup performance and eliminates massive if/elseif chains.
 *
 * **Benefits:**
 * - **Open/Closed Principle:** Add new directives without modifying dispatch
 * code
 * - **Lower Complexity:** Dispatch function cyclomatic complexity ~5 vs ~50
 * - **Better Testability:** Each handler can be tested independently
 * - **Easier Maintenance:** Handler logic co-located with registration
 *
 * **Example Usage:**
 * @code
 * DirectiveRegistry registry;
 *
 * // Register handlers
 * registry.Register("ORG", [this](const std::string& label,
 *                                  const std::string& operand,
 *                                  DirectiveContext& ctx) {
 *   // ORG implementation
 *   auto expr = ParseExpression(operand, *ctx.symbols);
 *   uint32_t address = expr->Evaluate(*ctx.symbols);
 *   *ctx.current_address = address;
 * });
 *
 * registry.Register("EQU", [this](const std::string& label,
 *                                  const std::string& operand,
 *                                  DirectiveContext& ctx) {
 *   // EQU implementation
 *   if (!label.empty()) {
 *     auto expr = ParseExpression(operand, *ctx.symbols);
 *     ctx.symbols->Define(label, SymbolType::Equate, expr);
 *   }
 * });
 *
 * // Execute directive
 * DirectiveContext ctx;
 * ctx.symbols = &symbols;
 * ctx.current_address = &current_address_;
 * registry.Execute("ORG", "", "1000H", ctx);
 * @endcode
 */
class DirectiveRegistry {
public:
  /**
   * @brief Register a directive handler
   *
   * Associates a directive mnemonic with its handler function.
   * Registration is case-insensitive (mnemonic converted to uppercase).
   *
   * @param mnemonic Directive name (e.g., "ORG", "EQU", "DB")
   * @param handler Handler function to execute for this directive
   *
   * @note Multiple mnemonics can share the same handler (e.g., "DB" and
   * "DEFB")
   */
  void Register(const std::string &mnemonic, DirectiveHandler handler);

  /**
   * @brief Register a directive handler for multiple aliases
   *
   * Associates multiple directive mnemonics with the same handler function.
   * Useful for directives with multiple names (e.g., DB, DEFB, BYTE).
   *
   * @param mnemonics List of directive names (all aliases)
   * @param handler Handler function to execute for all these directives
   */
  void Register(std::initializer_list<std::string> mnemonics,
                DirectiveHandler handler);

  /**
   * @brief Execute a registered directive handler
   *
   * Looks up and executes the handler for the given directive mnemonic.
   * Lookup is case-insensitive.
   *
   * @param mnemonic Directive name to execute
   * @param label Label on the line (empty string if no label)
   * @param operand Operand field after directive
   * @param context Execution context (section, symbols, etc.)
   *
   * @throws std::runtime_error if directive not registered
   */
  void Execute(const std::string &mnemonic, const std::string &label,
               const std::string &operand, DirectiveContext &context);

  /**
   * @brief Check if a directive is registered
   *
   * @param mnemonic Directive name to check (case-insensitive)
   * @return true if directive is registered, false otherwise
   */
  bool IsRegistered(const std::string &mnemonic) const;

private:
  /**
   * @brief Convert mnemonic to uppercase for case-insensitive lookup
   *
   * @param mnemonic Input mnemonic
   * @return Uppercase version of mnemonic
   */
  std::string ToUpper(const std::string &mnemonic) const;

  /// Map of uppercase mnemonic -> handler function
  std::unordered_map<std::string, DirectiveHandler> handlers_;
};

} // namespace xasm
