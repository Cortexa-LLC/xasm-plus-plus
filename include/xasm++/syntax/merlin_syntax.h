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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xasm++/common/conditional_assembler.h"
#include "xasm++/directives/merlin_directive_handlers.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"

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
  // Friend declarations for extracted directive handlers (xasm::merlin
  // namespace)
  friend void xasm::merlin::HandleOrg(DirectiveContext&);
  friend void xasm::merlin::HandleEqu(DirectiveContext&);
  friend void xasm::merlin::HandleDb(DirectiveContext&);
  friend void xasm::merlin::HandleDw(DirectiveContext&);
  friend void xasm::merlin::HandleHex(DirectiveContext&);
  friend void xasm::merlin::HandleDs(DirectiveContext&);
  friend void xasm::merlin::HandleDum(DirectiveContext&);
  friend void xasm::merlin::HandleDend(DirectiveContext&);
  friend void xasm::merlin::HandlePut(DirectiveContext&);
  friend void xasm::merlin::HandleDo(DirectiveContext&);
  friend void xasm::merlin::HandleElse(DirectiveContext&);
  friend void xasm::merlin::HandleFin(DirectiveContext&);
  friend void xasm::merlin::HandleLst(DirectiveContext&);
  friend void xasm::merlin::HandleLstdo(DirectiveContext&);
  friend void xasm::merlin::HandleTr(DirectiveContext&);
  friend void xasm::merlin::HandleAsc(DirectiveContext&);
  friend void xasm::merlin::HandleDci(DirectiveContext&);
  friend void xasm::merlin::HandleInv(DirectiveContext&);
  friend void xasm::merlin::HandleFls(DirectiveContext&);
  friend void xasm::merlin::HandleDa(DirectiveContext&);
  friend void xasm::merlin::HandlePmc(DirectiveContext&);
  friend void xasm::merlin::HandleEom(DirectiveContext&);
  friend void xasm::merlin::HandleMac(DirectiveContext&);
  friend void xasm::merlin::HandleUsr(DirectiveContext&);
  friend void xasm::merlin::HandleEnd(DirectiveContext&);
  friend void xasm::merlin::HandleSav(DirectiveContext&);
  friend void xasm::merlin::HandleXc(DirectiveContext&);
  friend void xasm::merlin::HandleMx(DirectiveContext&);
  friend void xasm::merlin::HandleRev(DirectiveContext&);
  friend void xasm::merlin::HandleLup(DirectiveContext&);

  // Legacy friend declarations (to be migrated/removed)
  friend void RegisterMerlinDirectiveHandlers(DirectiveRegistry& registry,
                                              MerlinSyntaxParser* parser);
  friend void HandleEndDirective(bool& end_directive_seen);
  friend void HandleXcDirective(const std::string& operand, Cpu6502* cpu);
  friend void HandleDumDirective(const std::string& operand, ConcreteSymbolTable& symbols,
                                 bool& in_dum_block, uint32_t& dum_address);
  friend void HandleDendDirective(bool& in_dum_block);

  /**
   * @brief Constructor - initializes parser state
   */
  MerlinSyntaxParser();

  /**
   * @brief Set CPU plugin for mode switching (e.g., XC directive)
   *
   * @param cpu Pointer to CPU plugin (must remain valid during parsing)
   */
  void SetCpu(Cpu6502* cpu);

  Cpu6502* GetCpu() const { return cpu_; }

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
  void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);

  // Accessor methods for directive handlers
  uint32_t GetCurrentAddress() const { return current_address_; }

  uint32_t GetDumAddress() const { return dum_address_; }

  bool IsInDumBlock() const { return in_dum_block_; }

  bool IsInMacroDefinition() const { return in_macro_definition_; }

  /**
   * @brief Increment seq# for a ]var label and return its uniqued name.
   *
   * For use by directive handlers (e.g. DS in DUM blocks) to match the
   * uniquing done for instruction-label and label-only ]var definitions.
   * Only applies to labels starting with ']' and containing no ':'.
   * Returns the unchanged label for non-]var names.
   */
  std::string UniqueVarLabel(const std::string& label) {
    if (!label.empty() && label[0] == ']' && label.find(':') == std::string::npos) {
      int seq = 0;
      seq = ++var_label_seq_[label];
      return label + "_" + std::to_string(seq);
    }
    return label;
  }

  bool MacroExists(const std::string& name) const { return macros_.find(name) != macros_.end(); }

  void SetGlobalLabel(const std::string& label) {
    current_scope_.global_label = label;
    current_scope_.local_labels.clear();
  }

  void StartMacroDefinition(const std::string& name) {
    in_macro_definition_ = true;
    current_macro_.name = name;
    current_macro_.body.clear();
    current_macro_.param_count = 0;
  }

  /**
   * @brief Parse an expression string into an Expression object
   *
   * @param str Expression string to parse
   * @param symbols Symbol table for resolving symbol references
   * @return Parsed expression (may throw on invalid syntax)
   */
  std::shared_ptr<Expression> ParseExpression(const std::string& str, ConcreteSymbolTable& symbols);

  /**
   * @brief Try to parse a Merlin character literal ("x" or 'x')
   * @return Expression if matched, nullptr otherwise
   */
  std::shared_ptr<Expression> TryParseCharLiteral(const std::string& expr,
                                                  ConcreteSymbolTable& symbols);

  /**
   * @brief Try to parse a low-byte operator expression (< or #)
   * @return Expression if matched, nullptr otherwise
   */
  std::shared_ptr<Expression> TryParseLowByteOperator(const std::string& expr,
                                                      ConcreteSymbolTable& symbols);

  /**
   * @brief Try to parse a high-byte operator expression (>)
   * @return Expression if matched, nullptr otherwise
   */
  std::shared_ptr<Expression> TryParseHighByteOperator(const std::string& expr,
                                                       ConcreteSymbolTable& symbols);

  /**
   * @brief Parse a numeric literal
   *
   * @param str Numeric string (hex, decimal, binary, octal)
   * @return Parsed numeric value
   */
  uint32_t ParseNumber(const std::string& str);

  /**
   * @brief Enable/disable RW18 output mode
   *
   * When enabled, USR directive arguments are captured and prepended as a
   * 12-byte header to binary output (Merlin/Prince of Persia compatibility).
   *
   * @param enabled True to enable RW18 mode
   */
  void SetRw18Mode(bool enabled) { rw18_mode_ = enabled; }

  /**
   * @brief Check if RW18 mode is enabled
   *
   * @return True if RW18 mode is enabled
   */
  bool IsRw18Mode() const { return rw18_mode_; }

  /**
   * @brief Check if USR directive arguments were captured
   *
   * @return True if USR directive with arguments was encountered
   */
  bool HasUsrArgs() const { return has_usr_args_; }

  /**
   * @brief Get captured USR directive arguments
   *
   * @return Array of 4 uint16_t values from USR directive
   */
  const std::array<uint16_t, 4>& GetUsrArgs() const { return usr_args_; }

  /**
   * @brief Set USR directive arguments (called by HandleUsr)
   *
   * @param args Array of 4 uint16_t values
   */
  void SetUsrArgs(const std::array<uint16_t, 4>& args) {
    usr_args_ = args;
    has_usr_args_ = true;
  }

 private:
  /**
   * @brief Label scope for managing :LOCAL labels
   *
   * Local labels are scoped to the most recent global label.
   */
  struct LabelScope {
    std::string global_label = {};  ///< The global label this scope belongs to
    std::unordered_map<std::string, uint32_t> local_labels = {};  ///< :label -> address
  };

  LabelScope current_scope_ = {};  ///< Current label scope (for :LOCAL labels)

  /**
   * @brief Macro definition
   *
   * Stores a macro body for later expansion via MAC directive.
   */
  struct MacroDefinition {
    std::string name = {};               ///< Macro name
    std::vector<std::string> body = {};  ///< Lines of macro body (unexpanded)
    int param_count = 0;                 ///< Number of parameters used (]1, ]2, etc.)
  };

  // Macro state
  bool in_macro_definition_ = false;                              ///< True if defining a macro
  MacroDefinition current_macro_ = {};                             ///< Current macro being defined
  std::unordered_map<std::string, MacroDefinition> macros_ = {};  ///< Defined macros
  int macro_expansion_depth_ = 0;                                 ///< Prevent infinite recursion

  // LUP (loop/repeat) block state
  bool in_lup_block_ = false;               ///< True if currently inside a LUP block
  int lup_count_ = 0;                       ///< Number of times to repeat LUP block
  std::vector<std::string> lup_body_ = {};  ///< Lines captured in LUP block
  int lup_nesting_depth_ = 0;               ///< Track nested LUP blocks

  // DUM block state
  bool in_dum_block_ = false;  ///< True if currently inside a DUM block
  uint32_t dum_address_ = 0;   ///< Current address within DUM block
  std::unordered_map<std::string, uint32_t> variable_labels_ = {};  ///< ]variable -> offset

  /// Tracks the current instance number for each ]variable code label.
  /// Each time "]var INSTR" (label on instruction) is defined, its counter
  /// increments so subsequent references get a unique symbol name
  /// (e.g., ]rts_1, ]rts_2, ...) rather than all resolving to the last global
  /// definition of ]rts. This fixes multi-pass assembly of Merlin ]variables
  /// that are redefined across subroutines (like "]rts  rts").
  std::unordered_map<std::string, int> var_label_seq_ = {};  ///< ]varname -> seq#

  uint32_t current_address_ = 0;     ///< Current address (for tracking label addresses)
  bool end_directive_seen_ = false;  ///< True if END directive has been processed

  std::vector<std::string> include_stack_ = {};  ///< Include file tracking (for circular detection)

  // Source location tracking (for error reporting)
  std::string current_file_ = "<stdin>";  ///< Current source filename
  int current_line_ = 0;                  ///< Current line number

  Cpu6502* cpu_ = nullptr;  ///< CPU plugin for mode switching (XC directive)

  // RW18 output format state (Merlin/Prince of Persia compatibility)
  bool rw18_mode_ = false;              ///< True if --rw18 flag enabled
  bool has_usr_args_ = false;           ///< True if USR directive captured
  std::array<uint16_t, 4> usr_args_{};  ///< USR directive arguments

  /**
   * @brief Conditional assembly manager (Phase 4: shared component)
   *
   * Replaces local ConditionalBlock struct and conditional_stack_
   * with shared ConditionalAssembler component.
   */
  ConditionalAssembler conditional_ = {};  ///< Shared conditional assembly logic

  /**
   * @brief Directive handler function type (NEW signature using
   * DirectiveContext)
   *
   * Lambda/function type for directive handlers. Each handler is a lambda
   * that captures 'this' and handles the directive-specific logic.
   *
   * Updated to use DirectiveContext pattern for better extensibility.
   */
  using DirectiveHandler = std::function<void(DirectiveContext& context)>;

  /**
   * @brief Directive registry mapping directive names to handlers
   */
  std::unordered_map<std::string, DirectiveHandler> directive_registry_ = {};

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
  bool DispatchDirective(const std::string& directive, const std::string& label,
                         const std::string& operand, DirectiveContext& context);

  // Parsing helpers
  static std::string StripComments(const std::string& line);

  /**
   * @brief Scope a Merlin local label to the current global label
   *
   * A local label (starts with ':') is scoped to the most recent global label.
   * For example, ":rts" in ADDSOUND scope becomes "ADDSOUND:rts".
   * Non-local labels (and labels with no current scope) are returned unchanged.
   */
  std::string ScopeLocalLabel(const std::string& label) const;

  /**
   * @brief Translate any ':word' local label references in an operand string
   *
   * Scans the operand and replaces ':name' tokens (preceded by a non-identifier
   * character or at string start) with the current-scope-qualified name.
   * For example, ":rts" in ADDSOUND scope becomes "ADDSOUND:rts".
   */
  std::string ScopeLocalLabelsInOperand(const std::string& operand) const;

  /**
   * @brief Expand ]variable references in an operand to their current
   * unique-instance names.
   *
   * ]variable code labels are redefined across subroutines. Each definition
   * creates a unique symbol "]varname_N". This method replaces ]varname in
   * branch/instruction operands with the current instance name so that
   * multi-pass assembly resolves to the correct (nearby) definition rather
   * than the last global one.
   *
   * Example: "]rts" → "]rts_3" when var_label_seq_["]rts"] == 3
   * Also handles "]rts:local" → "]rts_3:local" for scoped locals.
   */
  std::string ExpandVarLabelsInOperand(const std::string& operand) const;

  /**
   * @brief Expand Merlin character literals in an instruction operand
   *
   * Replaces all Merlin char-literal tokens ("X" or 'X') with their
   * Apple II high-bit hex equivalents ($XX where XX = ASCII | 0x80).
   * This keeps Merlin-specific encoding out of the shared
   * assembler.cpp::ParseExpression path.
   *
   * Examples:
   *   "#\"A\""     → "#$C1"
   *   "#\"A\"+1"   → "#$C1+1"
   *   "'r'-CTRL"   → "$F2-CTRL"
   */
  static std::string ExpandMerlinCharLiterals(const std::string& operand);

  /**
   * @brief Substitute ]variable references in a line with their current values
   *
   * In LUP loops, ]variables are assembly-time mutable values.  Each LUP
   * iteration must capture the variable's current value rather than creating a
   * symbolic reference that would be resolved after all iterations complete
   * (by which time the variable has its final post-loop value).
   *
   * Scans the line for ]identifier tokens and replaces each with the decimal
   * representation of its current value from the symbol table.
   */
  static std::string SubstituteMerlinVars(const std::string& line,
                                          const ConcreteSymbolTable& symbols);

  void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Try to handle a Merlin directive line via DispatchDirective
   * @return true if handled
   */
  bool TryHandleDirectiveLine(const std::string& directive, const std::string& label,
                              const std::string& operands, Section& section,
                              ConcreteSymbolTable& symbols);

  /**
   * @brief Try to handle a macro invocation line
   * @return true if handled (throws if invocation fails)
   */
  bool TryHandleMacroLine(const std::string& directive, const std::string& label,
                          const std::string& operands, Section& section,
                          ConcreteSymbolTable& symbols);

  /**
   * @brief Handle an assembly instruction line
   */
  void HandleInstructionLine(const std::string& directive, const std::string& label,
                             std::string operands, Section& section, ConcreteSymbolTable& symbols);

  // Label parsing
  static std::string ParseLabel(const std::string& line, size_t& pos, Section& section,
                                ConcreteSymbolTable& symbols);

  // Directive handlers
  void HandleEqu(const DirectiveContext& ctx, ConcreteSymbolTable& symbols);
  void HandleHex(const std::string& operand, Section& section);
  void HandleDS(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
  void HandleDum(const std::string& operand, ConcreteSymbolTable& symbols);
  void HandleDend();
  void HandlePut(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
  void HandleLst(const std::string& operand);
  void HandleLstdo();
  void HandleTr(const std::string& operand);
  void HandleAsc(const std::string& operand, Section& section);
  void HandleDA(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
  void HandleDCI(const std::string& operand, Section& section);
  void HandleINV(const std::string& operand, Section& section);
  void HandleFLS(const std::string& operand, Section& section);
  void HandleDo(const std::string& operand, ConcreteSymbolTable& symbols);
  void HandleElse();
  void HandleFin();
  void HandleEnd();
  void HandleSav(const std::string& operand);
  void HandleXc(const std::string& operand);
  void HandleMx(const std::string& operand);
  void HandleRev(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
  void HandleLup(const std::string& operand);

  // Macro directives
  void HandlePMC(const std::string& operand);
  void HandleEOM();
  void HandleMacroEnd();  // End macro definition (<<<)
  void ExpandMacro(const DirectiveContext& ctx, Section& section, ConcreteSymbolTable& symbols);

  // Macro helpers
  static std::string SubstituteParameters(const std::string& line,
                                          const std::vector<std::string>& params);

  // ParseLine sub-handlers (extracted to reduce cognitive complexity)
  /// Handle line capture during macro definition; returns true if consumed.
  bool HandleMacroCaptureState(const std::string& code_line, const std::string& upper_trimmed);
  /// Handle line capture during LUP block; returns true if consumed.
  bool HandleLupCaptureState(const std::string& code_line, const std::string& upper_trimmed,
                             Section& section, ConcreteSymbolTable& symbols);
  /// Handle DO/ELSE/FIN directives; returns true if consumed.
  bool HandleConditionalDirective(const std::string& trimmed, const std::string& upper_trimmed,
                                  ConcreteSymbolTable& symbols);
  /// Handle label-only line (pos >= code_line.length()); returns true.
  bool HandleLabelOnlyLine(const std::string& label, Section& section,
                           ConcreteSymbolTable& symbols);
  /// Return true if an equate RHS value needs deferred re-evaluation.
  static bool EquateNeedsReeval(const std::string& value);
  /// Handle "LABEL = VALUE" equate syntax; returns true if consumed.
  bool HandleEquateLine(const std::string& label, const std::string& code_line, size_t equals_pos,
                        Section& section, ConcreteSymbolTable& symbols);

  // HandleDS sub-helpers
  /// Substitute standalone '*' (program counter) in a DS operand string.
  std::string SubstitutePCInDSOperand(const std::string& op) const;
  /// Resolve a DS operand to a byte count (numeric literal, symbol, or *+N).
  uint32_t ResolveDSCount(const std::string& op, ConcreteSymbolTable& symbols);

  // ParseExpression sub-helpers
  /// Parse a high-byte ('>') prefix expression.
  std::shared_ptr<Expression> ParseHighByteExpr(const std::string& expr,
                                                ConcreteSymbolTable& symbols);
  /// Parse a low-byte ('<') prefix expression.
  std::shared_ptr<Expression> ParseLowByteExpr(const std::string& expr,
                                               ConcreteSymbolTable& symbols);
  /// Build a compound char+operator expression from char value and trailing rest.
  std::shared_ptr<Expression> CompoundCharExpr(int64_t char_val, const std::string& rest,
                                               ConcreteSymbolTable& symbols);
  /// Build a binary expression from a single-char arithmetic operator.
  static std::shared_ptr<Expression> MakeBinaryExpr(char op, std::shared_ptr<Expression> lhs,
                                                    std::shared_ptr<Expression> rhs);

  /// Expand a ]var token into result; returns updated parse index.
  size_t AppendExpandedVarLabel(const std::string& operand, size_t i, std::string& result) const;
  /// Return true if operand[i] begins a ':local' label reference.
  static bool StartsLocalLabelRef(const std::string& operand, size_t i);

  // Error formatting with source location
  std::string FormatError(const std::string& message) const;

  // Strip Merlin-style inline comment from instruction operand.
  static std::string StripMerlinInlineComment(const std::string& operands);

  // Strip trailing whitespace-delimited inline comment from an expression.
  static std::string StripMerlinExprComment(const std::string& expr);
};

}  // namespace xasm
