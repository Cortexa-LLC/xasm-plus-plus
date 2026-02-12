/**
 * @file merlin_syntax.h
 * @brief Merlin assembly syntax parser
 *
 * This file defines a parser for Merlin assembler syntax, which was used
 * in many classic Apple II programs including Prince of Persia. The parser
 * implements Merlin's distinctive syntax and directive set.
 *
 * @note Phases 1-3: Foundation, Local Labels, DUM Blocks
 */

#pragma once

#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/common/conditional_assembler.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

// Forward declarations
class Cpu6502;

/**
 * @brief Merlin assembly syntax parser
 *
 * The MerlinSyntaxParser implements the Merlin assembler syntax used on
 * the Apple II. Merlin has a distinctive syntax style with special label
 * types and directives that differ from other assemblers.
 *
 * @par Supported Features
 * - **Comment Styles**:
 *   - `*` in column 1 for full-line comments
 *   - `;` for inline comments
 * - **Label Types**:
 *   - `GLOBAL` - Normal global labels
 *   - `:LOCAL` - Local labels scoped to the previous global label
 *   - `]VARIABLE` - DUM block variables
 * - **Directives**:
 *   - `ORG` - Set origin address
 *   - `EQU` - Define constant
 *   - `DB`/`DW` - Define byte/word data
 *   - `HEX` - Define hex bytes
 *   - `DS` - Define space
 *   - `DUM`/`DEND` - Dummy section (for variable definitions)
 *   - `PUT` - Include file
 *   - `ASC` - ASCII string
 *   - `DO`/`ELSE`/`FIN` - Conditional assembly
 * - **Number Formats**:
 *   - `$hex` - Hexadecimal (e.g., `$1234`)
 *   - `%binary` - Binary (e.g., `%10101010`)
 *   - Decimal - Plain numbers (e.g., `42`)
 *
 * @par Local Label Scoping
 * Local labels (starting with `:`) are scoped to the most recent global
 * label. This allows reusing simple names like `:loop` without conflicts:
 * @code
 * SUBROUTINE1
 *   :loop    ; Resolves to SUBROUTINE1:loop
 *     DEX
 *     BNE :loop
 *     RTS
 *
 * SUBROUTINE2
 *   :loop    ; Resolves to SUBROUTINE2:loop (different from above)
 *     INX
 *     BNE :loop
 *     RTS
 * @endcode
 *
 * @par DUM Blocks
 * DUM blocks define dummy variables that don't generate output but
 * reserve space for variable definitions:
 * @code
 * DUM $20        ; Start dummy block at zero page $20
 * ]COUNTER  DS 1 ; Reserve 1 byte at $20
 * ]POINTER  DS 2 ; Reserve 2 bytes at $21
 * DEND           ; End dummy block
 *
 * ; Use variables in code
 * INC ]COUNTER   ; INC $20
 * LDA ]POINTER   ; LDA $21
 * @endcode
 *
 * @par Example Usage
 * @code
 * MerlinSyntaxParser parser;
 * Section section("CODE", 0x01, 0x6000);
 * ConcreteSymbolTable symbols;
 *
 * std::string source = R"(
 *         ORG $6000
 * START   LDA #$42
 *         STA $C000
 * :LOOP   JMP :LOOP
 * )";
 *
 * parser.Parse(source, section, symbols);
 * @endcode
 */
class MerlinSyntaxParser {
public:
  // Friend declarations for extracted directive handlers
  friend void RegisterMerlinDirectiveHandlers(DirectiveRegistry &registry,
                                              MerlinSyntaxParser *parser);
  friend void HandleEndDirective(bool &end_directive_seen);
  friend void HandleXcDirective(const std::string &operand, Cpu6502 *cpu);
  friend void HandleDumDirective(const std::string &operand,
                                 ConcreteSymbolTable &symbols,
                                 bool &in_dum_block, uint32_t &dum_address);
  friend void HandleDendDirective(bool &in_dum_block);

  /**
   * @brief Constructor - initializes parser state
   */
  MerlinSyntaxParser();

  /**
   * @brief Set CPU plugin for mode switching (e.g., XC directive)
   *
   * @param cpu Pointer to CPU plugin (must remain valid during parsing)
   */
  void SetCpu(Cpu6502 *cpu);

  /**
   * @brief Parse Merlin assembly source into atoms and symbols
   *
   * Parses the provided Merlin-format assembly source and populates
   * the section with atoms and the symbol table with label definitions.
   *
   * @param source Assembly source code (multi-line string)
   * @param section Section to populate with atoms
   * @param symbols Symbol table to populate with label definitions
   *
   * @throws std::runtime_error on parse errors (syntax errors, undefined
   * labels, etc.)
   */
  void Parse(const std::string &source, Section &section,
             ConcreteSymbolTable &symbols);

private:
  /**
   * @brief Label scope for managing :LOCAL labels
   *
   * Local labels are scoped to the most recent global label.
   */
  struct LabelScope {
    std::string global_label; ///< The global label this scope belongs to
    std::unordered_map<std::string, uint32_t>
        local_labels; ///< :label -> address
  };

  LabelScope current_scope_; ///< Current label scope (for :LOCAL labels)

  /**
   * @brief Macro definition
   *
   * Stores a macro body for later expansion via MAC directive.
   */
  struct MacroDefinition {
    std::string name;              ///< Macro name
    std::vector<std::string> body; ///< Lines of macro body (unexpanded)
    int param_count;               ///< Number of parameters used (]1, ]2, etc.)
  };

  // Macro state
  bool in_macro_definition_;      ///< True if defining a macro
  MacroDefinition current_macro_; ///< Current macro being defined
  std::unordered_map<std::string, MacroDefinition> macros_; ///< Defined macros
  int macro_expansion_depth_; ///< Prevent infinite recursion

  // LUP (loop/repeat) block state
  bool in_lup_block_;              ///< True if currently inside a LUP block
  int lup_count_;                  ///< Number of times to repeat LUP block
  std::vector<std::string> lup_body_; ///< Lines captured in LUP block
  int lup_nesting_depth_;          ///< Track nested LUP blocks

  // DUM block state
  bool in_dum_block_;    ///< True if currently inside a DUM block
  uint32_t dum_address_; ///< Current address within DUM block
  std::unordered_map<std::string, uint32_t>
      variable_labels_; ///< ]variable -> offset

  uint32_t current_address_; ///< Current address (for tracking label addresses)
  bool end_directive_seen_;  ///< True if END directive has been processed

  std::vector<std::string>
      include_stack_; ///< Include file tracking (for circular detection)

  // Source location tracking (for error reporting)
  std::string current_file_; ///< Current source filename
  int current_line_;         ///< Current line number

  Cpu6502 *cpu_ = nullptr; ///< CPU plugin for mode switching (XC directive)

  /**
   * @brief Conditional assembly manager (Phase 4: shared component)
   *
   * Replaces local ConditionalBlock struct and conditional_stack_
   * with shared ConditionalAssembler component.
   */
  ConditionalAssembler conditional_; ///< Shared conditional assembly logic

  /**
   * @brief Directive handler function type (NEW signature using DirectiveContext)
   *
   * Lambda/function type for directive handlers. Each handler is a lambda
   * that captures 'this' and handles the directive-specific logic.
   * 
   * Updated to use DirectiveContext pattern for better extensibility.
   */
  using DirectiveHandler = std::function<void(
      const std::string &label, const std::string &operand,
      DirectiveContext &context)>;

  /**
   * @brief Directive registry mapping directive names to handlers
   */
  std::unordered_map<std::string, DirectiveHandler> directive_registry_;

  /**
   * @brief Initialize directive registry with all supported directives
   *
   * Populates directive_registry_ with lambdas that adapt each Handle*
   * method to the common DirectiveHandler signature.
   */
  void InitializeDirectiveRegistry();

  /**
   * @brief Dispatch directive to appropriate handler via registry
   *
   * @param directive Directive name (e.g., "ORG", "DB")
   * @param label Associated label (may be empty)
   * @param operands Directive operands
   * @param section Section to populate
   * @param symbols Symbol table
   * @return true if directive was handled, false if unknown directive
   */
  bool DispatchDirective(const std::string &directive,
                          const std::string &label,
                          const std::string &operand,
                          DirectiveContext &context);

  // Parsing helpers
  std::string StripComments(const std::string &line);

  void ParseLine(const std::string &line, Section &section,
                 ConcreteSymbolTable &symbols);

  // Label parsing
  std::string ParseLabel(const std::string &line, size_t &pos, Section &section,
                         ConcreteSymbolTable &symbols);

  // Directive handlers
  void HandleOrg(const std::string &operand, Section &section,
                 ConcreteSymbolTable &symbols);
  void HandleEqu(const std::string &label, const std::string &operand,
                 ConcreteSymbolTable &symbols);
  void HandleDB(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleDW(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleHex(const std::string &operand, Section &section);
  void HandleDS(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleDum(const std::string &operand, ConcreteSymbolTable &symbols);
  void HandleDend();
  void HandlePut(const std::string &operand, Section &section,
                 ConcreteSymbolTable &symbols);
  void HandleLst(const std::string &operand);
  void HandleLstdo();
  void HandleTr(const std::string &operand);
  void HandleAsc(const std::string &operand, Section &section);
  void HandleDA(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleDCI(const std::string &operand, Section &section);
  void HandleINV(const std::string &operand, Section &section);
  void HandleFLS(const std::string &operand, Section &section);
  void HandleDo(const std::string &operand, ConcreteSymbolTable &symbols);
  void HandleElse();
  void HandleFin();
  void HandleEnd();
  void HandleSav(const std::string &operand);
  void HandleXc(const std::string &operand);
  void HandleMx(const std::string &operand);
  void HandleRev(const std::string &label, const std::string &operand,
                 Section &section, ConcreteSymbolTable &symbols);
  void HandleLup(const std::string &operand);

  // Macro directives
  void HandlePMC(const std::string &operand);
  void HandleEOM();
  void HandleMAC(const std::string &macro_name, const std::string &params,
                 Section &section,
                 ConcreteSymbolTable &symbols); // Invoke macro (Merlin style)
  void HandleMacroEnd();                        // End macro definition (<<<)
  void ExpandMacro(const std::string &macro_name, const std::string &operand,
                   Section &section, ConcreteSymbolTable &symbols);

  // Macro helpers
  std::string SubstituteParameters(const std::string &line,
                                   const std::vector<std::string> &params);

  // Expression/number parsing
  uint32_t ParseNumber(const std::string &str);
  std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                              ConcreteSymbolTable &symbols);

  // Error formatting with source location
  std::string FormatError(const std::string &message) const;
};

} // namespace xasm
