/**
 * @file scmasm_syntax.h
 * @brief S-C Macro Assembler (SCMASM) syntax parser
 *
 * This file defines a parser for S-C Macro Assembler syntax, which was
 * developed by Bob Sander-Cederlof for the Apple II. SCMASM features
 * BASIC-style line numbering and distinctive dot-prefix directives.
 *
 * @note Phase 2: Integrated with shared ExpressionParser
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "xasm++/common/expression_parser.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"

namespace xasm {

// Forward declaration

/**
 * @brief SCMASM-specific number parser
 *
 * Handles SCMASM number formats:
 * - $hex (e.g., $1234)
 * - %binary (e.g., %10101010, %1111.0000 with dot separators)
 * - Decimal (e.g., 42)
 * - Character constants with high-bit rule ('A, "A, etc.)
 */
class SCMASMNumberParser : public INumberParser {
 public:
  /**
   * @brief Attempt to parse an SCMASM-specific number format
   *
   * @param token The token to parse (e.g., "$FF", "%10101010", "'A")
   * @param value Output parameter - receives the parsed value
   * @return true if successfully parsed, false otherwise
   */
  bool TryParse(const std::string& token, int64_t& value) const override;
};

/**
 * @brief S-C Macro Assembler (SCMASM) syntax parser
 *
 * The ScmasmSyntaxParser implements the S-C Macro Assembler syntax used on
 * the Apple II. SCMASM has a BASIC-style interface with line numbering and
 * uses dot-prefix directives that distinguish it from other assemblers.
 *
 * @par Supported Features (Phase 1)
 * - **Comment Styles**:
 *   - `;` for inline comments
 *   - `*` in column 1 for full-line comments
 * - **Line Numbers**:
 *   - Optional BASIC-style line numbers (0-65535)
 *   - Lines auto-sorted by number
 * - **Label Types**:
 *   - `GLOBAL` - Standard global labels
 *   - `.N` - Local numeric labels (0-9, forward-reference only)
 * - **Core Directives**:
 *   - `.OR` - Set origin address
 *   - `.EQ` - Define constant (immutable)
 *   - `.SE` - Set variable (redefinable)
 * - **Number Formats**:
 *   - `$hex` - Hexadecimal (e.g., `$1234`, `$ABCD`)
 *   - `%binary` - Binary with optional `.` separators (e.g., `%1111.0000`)
 *   - Decimal - Plain numbers (e.g., `42`, `65535`)
 *   - ASCII - Character constants with high-bit rule:
 *     - Delimiter ASCII < 0x27 (`'`) → high bit SET
 *     - Delimiter ASCII ≥ 0x27 → high bit CLEAR
 *
 * @par String Delimiter High-Bit Rule (CRITICAL)
 * The delimiter character's ASCII value determines whether the high bit is set:
 * - `"` (0x22) → high bit SET (inverse video)
 * - `'` (0x27) → high bit CLEAR (normal)
 * - `/` (0x2F) → high bit CLEAR (normal)
 *
 * Examples:
 * @code
 * VALUE1  .EQ "A    ; $C1 (0x41 | 0x80) - high bit SET
 * VALUE2  .EQ 'A    ; $41 - high bit CLEAR
 * VALUE3  .EQ /A    ; $41 - high bit CLEAR
 * @endcode
 *
 * @par Line Number Format
 * Lines may optionally begin with a line number (0-65535):
 * @code
 * 1000 START    .OR $0800
 * 1010          LDA #$00
 * 1020          RTS
 * @endcode
 *
 * @par Example Usage
 * @code
 * ScmasmSyntaxParser parser;
 * Section section("CODE", 0x01, 0x0800);
 * ConcreteSymbolTable symbols;
 *
 * std::string source = R"(
 * 1000          .OR $0800
 * 1010 START    LDA #$42
 * 1020          STA $C000
 * 1030 .1       JMP .1
 * )";
 *
 * parser.Parse(source, section, symbols);
 * @endcode
 */

/// @brief Helper struct for ApplyHighBitRule - groups adjacent char parameters
struct HighBitChars {
  char input = '\0';      ///< Input character
  char delimiter = '\0';  ///< Delimiter character (controls high-bit rule)
};

/// @brief Helper struct for StartPhase - groups adjacent address parameters
struct PhaseAddresses {
  uint32_t real_addr = 0;     ///< Real (physical) address
  uint32_t virtual_addr = 0;  ///< Virtual address
};

class ScmasmSyntaxParser {
 public:
  /**
   * @brief Constructor - initializes parser state
   */
  ScmasmSyntaxParser();

  /**
   * @brief Set CPU plugin for opcode validation
   *
   * @param cpu Pointer to CPU plugin (must remain valid during parsing)
   */
  void SetCpu(CpuPlugin* cpu);

  /**
   * @brief Set CPU plugin by name for opcode validation
   *
   * @param cpu_name CPU name string (e.g., "6502", "65C02", "65816")
   * @throws std::runtime_error if invalid CPU name
   */
  void SetCpu(const std::string& cpu_name);

  /**
   * @brief Set include search paths for .INB directive
   *
   * @param paths Vector of directory paths to search for included files
   */
  void SetIncludePaths(const std::vector<std::string>& paths);

  /**
   * @brief Set path mappings for .INB directive
   *
   * Path mappings allow virtual paths to be substituted with actual paths.
   * This is different from include paths which only add search directories.
   *
   * Example: mapping["usr/src/shared"] = "SHARED"
   * converts .INB usr/src/shared/x.printf.s → SHARED/x.printf.s
   *
   * @param mappings Map of virtual paths to actual paths
   */
  void SetPathMappings(const std::map<std::string, std::string>& mappings);

  /**
   * @brief Return output path set by a .TF directive during parsing.
   *
   * Returns empty string if no .TF was encountered (caller should use the
   * -o/output value in that case).
   */
  const std::string& GetTfOutput() const { return tf_output_; }

  /**
   * @brief Set the .TF output path (called by HandleTf directive handler).
   */
  void SetTfOutput(const std::string& path) { tf_output_ = path; }

  /**
   * @brief Parse SCMASM assembly source into atoms and symbols
   *
   * Parses the provided SCMASM-format assembly source and populates
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

  // Phase 6c.2: Public methods for handler access
  // These are called by extracted directive handlers

  /**
   * @brief Evaluate expression with symbol resolution
   *
   * Public for use by extracted directive handlers.
   * Supports all SCMASM expression features.
   *
   * @param str Expression string
   * @param symbols Symbol table for lookups
   * @return Evaluated value
   * @throws std::runtime_error on undefined symbols or invalid expressions
   */
  uint32_t EvaluateExpression(const std::string& str, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .MA directive (macro definition start)
   *
   * Public for use by extracted directive handlers.
   * Begins macro definition. Macro name can be specified as operand or label.
   *
   * @param label Optional label before directive
   * @param operand Macro name (if not in label)
   */
  void HandleMa(const DirectiveContext& ctx);

  /**
   * @brief Parse .EM directive (end macro definition)
   *
   * Public for use by extracted directive handlers.
   */
  void HandleEm();

  /**
   * @brief Get current source file path
   *
   * Used by .INB directive to resolve relative includes.
   *
   * @return Current source file path
   */
  std::string GetCurrentFile() const;

  /**
   * @brief Set current source file path
   *
   * Used by .INB directive to update context for nested includes.
   *
   * @param file New source file path
   */
  void SetCurrentFile(const std::string& file);

  /**
   * @brief Check if parser is in dummy section mode
   *
   * When in dummy section mode (.DUMMY active), data directives
   * update the address counter but don't emit bytes.
   *
   * @return true if in dummy section, false otherwise
   */
  bool InDummySection() const;

  /**
   * @brief Enter dummy section mode
   *
   * Saves the current main-section address so it can be restored by
   * EndDummySection().  Any .OR inside the dummy block repositions only the
   * dummy section's label counter; the main PC is unaffected after .ED.
   *
   * @param current_address The main-section PC at the point .DUMMY is entered.
   *        This value is saved and returned by EndDummySection().
   *
   * Called by .DUMMY directive handler.
   */
  void StartDummySection(uint32_t current_address);

  /**
   * @brief Exit dummy section mode
   *
   * Restores the main-section PC to the value saved by StartDummySection().
   *
   * @return The saved main-section address to be restored by the handler.
   *
   * Called by .ED directive handler.
   */
  uint32_t EndDummySection();

  /**
   * @brief Check if parser is in phase assembly mode
   *
   * When in phase assembly mode (.PH active), labels are defined
   * at virtual addresses while code is emitted at real addresses.
   *
   * @return true if in phase assembly, false otherwise
   */
  bool InPhase() const;

  /**
   * @brief Start phase assembly
   *
   * Called by .PH directive handler.
   *
   * @param real_addr Real address where code is stored
   * @param virtual_addr Virtual address for labels
   */
  void StartPhase(PhaseAddresses addrs);

  /**
   * @brief End phase assembly
   *
   * Called by .EP directive handler (when used without operand).
   *
   * @param current_virtual Current virtual address (for size calculation)
   * @return New real address after phase
   */
  uint32_t EndPhase(uint32_t current_virtual);

  /**
   * @brief Get phase virtual address
   *
   * @return Virtual address set by .PH directive
   */
  uint32_t GetPhaseVirtualAddress() const;

  /**
   * @brief Get phase real address
   *
   * @return Real address where phase code is stored
   */
  uint32_t GetPhaseRealAddress() const;

  /**
   * @brief Calculate current real address from virtual address
   *
   * When in phase assembly, calculate the real address corresponding
   * to the given virtual address.
   *
   * @param current_virtual Current virtual address
   * @return Corresponding real address
   */
  uint32_t GetCurrentRealAddress(uint32_t current_virtual) const;

  /**
   * @brief Returns the scope prefix for a local label definition or reference.
   *
   * For ':N' labels inside macro invocations, returns the per-invocation macro
   * scope so that multiple expansions of the same macro don't share label names.
   * For '.N' labels (or ':N' outside macros), returns last_global_label_.
   *
   * Exposed publicly so directive handlers (e.g. HandleEq) can use it when
   * processing ':N .EQ *' patterns inside macros.
   */
  const std::string& LocalLabelScope(const std::string& label) const;

  /**
   * @brief Returns the full scoped name for a local label definition or
   * reference.
   *
   * For '.N' dot-prefix labels, inserts '@' between the scope and the label to
   * prevent collision with global sub-labels that happen to share the expanded
   * name (e.g. local '.1' in BITBLT scope must not collide with the global
   * sub-label 'BITBLT.1').
   *
   * For ':N' colon labels, the scope prefix already uses a per-invocation macro
   * scope that is unique, so no extra separator is needed.
   *
   * Exposed publicly so directive handlers (e.g. HandleEq) can use it
   * consistently.
   */
  std::string ScopedLocalLabelName(const std::string& label) const;

  /**
   * @brief Expand character literals in an expression string to hex values.
   *
   * Converts SCMASM character literal syntax to numeric hex equivalents
   * before the expression is passed to the generic ParseExpression engine,
   * which has no knowledge of SCMASM quoting conventions.
   *
   * Rules (SCMASM high-bit convention):
   *   "X" or "X  (double-quote, ASCII $22 < $27) → high bit SET  (e.g. "0" → $B0)
   *   'X' or 'X  (apostrophe,   ASCII $27 NOT < $27) → high bit CLEAR (e.g. 'A' → $41)
   *
   * Exposed publicly so directive handlers can expand character literals
   * before expression evaluation (e.g., HandleDa for .DA #'N').
   *
   * @param s Expression string that may contain character literals
   * @return Expression string with character literals replaced by $XX hex tokens
   */
  static std::string ExpandCharLiteralsInExpr(const std::string& s);

  /**
   * @brief Expand local label references (.N / :N) in an operand string to
   * their scoped forms (e.g., ".10" → "TERM.SGR@.10").  Used for both
   * instruction operands and directive operands so that forward/backward local
   * label references resolve correctly in multi-pass assembly.
   */
  std::string ExpandLocalLabelsInOperand(const std::string& operand) const;

 private:
  // Directive handler function signature (DirectiveContext pattern)
  using DirectiveHandler = std::function<void(DirectiveContext& context)>;

  // Macro definition structure
  struct MacroDef {
    std::string name = {};
    std::vector<std::string> lines = {};  ///< Macro body lines
  };

  // Macro nesting limit (prevent infinite recursion)
  static constexpr int kMAX_MACRO_NESTING_DEPTH = 63;

  // Current state
  uint32_t current_address_ = 0;           ///< Current assembly address
  std::string current_file_ = "<source>";  ///< Current source filename
  int current_line_ = 0;                   ///< Current line number (for errors)

  // CPU plugin for opcode validation
  CpuPlugin* cpu_ = nullptr;  ///< CPU plugin for opcode validation (nullable)
  // Owned CPU object when .OP directive overrides the external CPU plugin.
  // Must outlive cpu_ — SetCpu(string) stores the address of this member.
  std::unique_ptr<Cpu6502> owned_cpu_ = nullptr;  ///< CPU owned by parser (via .OP directive)

  // Include search paths
  std::vector<std::string> include_paths_ = {};  ///< Directories to search for .INB files

  // Path mappings for virtual path substitution
  std::map<std::string, std::string> path_mappings_ =
      {};  ///< Virtual→actual path mappings for .INB

  // .TF target output path (overrides -o default when set)
  std::string tf_output_ = {};  ///< Output path set by .TF directive (empty = not set)

  // Symbol tracking for .SE (redefinable)
  std::unordered_map<std::string, bool> variable_symbols_ = {};  ///< Track .SE symbols

  // Phase 3: Macros, Conditionals, Local Labels, Loops
  std::unordered_map<std::string, MacroDef> macros_ = {};        ///< Defined macros
  std::unordered_map<std::string, uint32_t> local_labels_ = {};  ///< Local labels (.0-.9)
  bool in_macro_definition_ = false;                             ///< Currently defining a macro
  std::string current_macro_name_ = {};                          ///< Name of macro being defined
  std::vector<std::string> current_macro_body_ = {};             ///< Lines of macro being defined
  int macro_invocation_depth_ = 0;    ///< Nesting depth for macro invocations
  int macro_invocation_counter_ = 0;  ///< Monotonic counter, unique per invocation
  /// Per-invocation scope prefix for ':N' macro-local labels.
  /// Empty when not inside any macro.  Set to a unique string each time a
  /// macro is invoked so that :1 labels in different expansions of the same
  /// macro (under the same global label) don't collide in the symbol table.
  std::string current_macro_label_scope_ = {};

  // Pending label: a label-only line may be associated with the next .EQ/.SE
  std::string pending_label_ = {};  ///< Label deferred from previous label-only line

  // Local label scoping: track the most recent global (non-local) label so
  // that local labels like .8 are stored as "GLOBALNAME.8" in the symbol
  // table and can be resolved by the assembler during branch encoding.
  std::string last_global_label_ = {};  ///< Most recent global label (scope for local labels)

  // Dummy section support (structure definitions)
  bool in_dummy_section_ = false;     ///< Currently in dummy section (.DUMMY active)
  uint32_t dummy_saved_address_ = 0;  ///< Main PC saved at .DUMMY entry, restored at .ED

  // Phase assembly support (.PH/.EP)
  bool in_phase_ = false;            ///< Currently in phase assembly
  uint32_t phase_virtual_addr_ = 0;  ///< Virtual address for phase
  uint32_t phase_real_addr_ = 0;     ///< Real address where code is stored

  // Directive registry
  std::unordered_map<std::string, DirectiveHandler> directive_registry_ = {};

  // Expression and number parsing (Phase 2 integration)
  SCMASMNumberParser scmasm_number_parser_ = {};  ///< SCMASM-specific number parser

  /**
   * @brief Initialize directive registry with all supported directives
   *
   * Populates directive_registry_ with handlers for all SCMASM directives.
   * Called from constructor.
   */
  void InitializeDirectiveRegistry();

  /**
   * @brief Strip line number from beginning of line
   *
   * SCMASM allows optional line numbers (0-65535) at the start of lines.
   * This function removes them and returns the rest of the line.
   *
   * @param line Input line
   * @return Line with line number removed
   */
  static std::string StripLineNumber(const std::string& line);

  /**
   * @brief Strip comments from line
   *
   * Removes `;` inline comments and `*` column-1 comments.
   *
   * @param line Input line
   * @return Line with comments removed
   */
  static std::string StripComments(const std::string& line);

  /**
   * @brief Handle a line starting with '*' (either private label or comment)
   * @param line Input line
   * @param first_non_space Position of first non-space in line
   * @param result Output: the processed line (or empty for comment)
   * @return true if the '*' was recognized and result was set; false otherwise
   */
  static bool TryHandleAsteriskLine(const std::string& line, size_t first_non_space,
                                    std::string& result);

  /**
   * @brief Strip Apple II editor commands from line
   *
   * Removes lines starting with editor commands (NEW, AUTO, MAN, SAVE, ASM,
   * DELETE, LIST). These commands were used in Apple II line editors and should
   * be ignored during assembly.
   *
   * @param line Input line
   * @return Empty string if line is an editor command, original line otherwise
   */
  static std::string StripEditorCommands(const std::string& line);

  /**
   * @brief Parse a single line of source
   *
   * @param line Line to parse
   * @param section Section to add atoms to
   * @param symbols Symbol table for label definitions
   * @param source All source lines (for multi-line directives)
   * @param line_idx Current line index (updated by multi-line directives)
   */
  void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols,
                 const std::vector<std::string>& source, size_t& line_idx);

  /// Process one raw source line (strip/filter, delegate to ParseLine).
  void ProcessOneLine(const std::string& raw, Section& section, ConcreteSymbolTable& symbols,
                      const std::vector<std::string>& lines, size_t& line_idx);

  /// Handle a line received while collecting a macro body.
  void HandleMacroBodyLine(const std::string& line, size_t& line_idx);

  /// Flush any pending label at end-of-file.
  void FlushPendingLabel(Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Try to handle a directive line (opcode starts with '.')
   * @return true if handled
   */
  bool TryHandleDirectiveLine(const std::string& opcode_upper, const std::string& operand,
                              const std::string& label, Section& section,
                              ConcreteSymbolTable& symbols, const std::vector<std::string>& source,
                              size_t& line_idx);

  /// Define label for a directive using SCMASM namespace scoping rules.
  void DefineLabelForDirectiveSCMASM(const std::string& opcode_upper, const std::string& label,
                                     Section& section, ConcreteSymbolTable& symbols);

  /// Dispatch control-flow directives (.DO, .LU, .ELSE, .FIN, .ENDU).
  /// @return true if the directive was handled; false if not a control-flow directive.
  bool TryDispatchControlFlow(const std::string& opcode_upper, const std::string& operand,
                              const std::string& label, Section& section,
                              ConcreteSymbolTable& symbols, const std::vector<std::string>& source,
                              size_t& line_idx);

  /**
   * @brief Try to handle a macro invocation (opcode starts with '>')
   * @return true if handled (throws if macro not found but > prefix used)
   */
  bool TryHandleMacroLine(const std::string& opcode_upper, const std::string& operand,
                          Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Handle an assembly instruction line (6502 etc.)
   */
  void HandleInstructionLine(const std::string& opcode_upper, const std::string& operand,
                             Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse label at start of line
   *
   * @param line Input line
   * @param pos Position in line (updated)
   * @param section Section to add label atom
   * @param symbols Symbol table
   * @return Label name (empty if none)
   */
  std::string ParseLabel(const std::string& line, size_t& pos, Section& section,
                         ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .OR directive (set origin)
   *
   * @param operand Directive operand
   * @param section Section to add org atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleOr(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .EQ directive (define constant)
   *
   * @param label Symbol name
   * @param operand Expression to evaluate
   * @param symbols Symbol table to add symbol
   */
  void HandleEq(const DirectiveContext& ctx, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .SE directive (set variable)
   *
   * Like .EQ but allows redefinition.
   *
   * @param label Symbol name
   * @param operand Expression to evaluate
   * @param symbols Symbol table to add/update symbol
   */
  void HandleSe(const DirectiveContext& ctx, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .AS directive (ASCII string)
   *
   * Emits ASCII characters. High-bit rule applies based on delimiter.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAs(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .AT directive (ASCII text with high bit on last char)
   *
   * Like .AS but sets high bit on last character.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAt(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .AZ directive (ASCII zero-terminated)
   *
   * Like .AS but adds $00 terminator.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAz(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .DA directive (define address/data)
   *
   * Emits comma-separated numeric values (bytes or words).
   *
   * @param operand Comma-separated values
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleDa(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  void EmitDaValue(const std::string& value_trimmed, std::vector<uint8_t>& data,
                   ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .HS directive (hex string)
   *
   * Converts hex digits to bytes (no $ prefix needed).
   *
   * @param operand Hex digits
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  static void HandleHs(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .BS directive (bit string)
   *
   * Converts binary digits to bytes.
   *
   * @param operand Binary digits
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleBs(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Invoke a macro by name
   *
   * Expands macro body, substituting \0-\9 parameters.
   *
   * @param name Macro name
   * @param params Parameters to substitute
   * @param section Section to add atoms to
   * @param symbols Symbol table
   */
  void InvokeMacro(const std::string& name, const std::vector<std::string>& params,
                   Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse .DO directive (conditional assembly)
   *
   * Assembles block if expression is non-zero.
   *
   * @param operand Expression to evaluate
   * @param section Section to add atoms to
   * @param symbols Symbol table
   * @param source Remaining source lines
   * @param line_idx Current line index (updated)
   */
  void HandleDo(const DirectiveContext& ctx, Section& section, ConcreteSymbolTable& symbols,
                const std::vector<std::string>& source, size_t& line_idx);

  /**
   * @brief Parse .LU directive (loop start)
   *
   * Repeats lines until .ENDU, count times.
   *
   * @param operand Loop count expression
   * @param section Section to add atoms to
   * @param symbols Symbol table
   * @param source Remaining source lines
   * @param line_idx Current line index (updated)
   */
  void HandleLu(const DirectiveContext& ctx, Section& section, ConcreteSymbolTable& symbols,
                const std::vector<std::string>& source, size_t& line_idx);

  /**
   * @brief Substitute macro parameters in line
   *
   * Replaces \0-\9 with corresponding parameters.
   *
   * @param line Line with parameter references
   * @param params Parameter values
   * @return Line with parameters substituted
   */
  static std::string SubstituteParameters(const std::string& line,
                                          const std::vector<std::string>& params);

  /**
   * @brief Parse number in any format
   *
   * Supports:
   * - Hexadecimal: $1234
   * - Binary: %10101010 (with optional . separators)
   * - Decimal: 42
   * - ASCII character: 'A or "A (high-bit rule applies)
   *
   * @param str String to parse
   * @return Parsed number value
   * @throws std::runtime_error on invalid format
   */
  static uint32_t ParseNumber(const std::string& str);

  /**
   * @brief Parse expression
   *
   * @param str Expression string
   * @param symbols Symbol table for lookups
   * @return Expression object
   */
  std::shared_ptr<Expression> ParseExpression(const std::string& str, ConcreteSymbolTable& symbols);

  /**
   * @brief Apply high-bit rule based on delimiter ASCII value
   *
   * If delimiter ASCII < 0x27 (apostrophe), high bit is SET.
   * Otherwise, high bit is CLEAR.
   *
   * @param c Character to transform
   * @param delimiter Delimiter character
   * @return Transformed character
   */
  static uint8_t ApplyHighBitRule(HighBitChars hbc);

  /**
   * @brief Parse string with delimiter semantics
   *
   * Extracts string from operand and applies high-bit rule.
   *
   * @param operand String operand (with delimiters)
   * @param result Output vector of bytes
   * @return Delimiter character used
   */
  static char ParseString(const std::string& operand, std::vector<uint8_t>& result);

  /**
   * @brief Check if label is a local label (.0-.9)
   *
   * @param label Label to check
   * @return true if local label, false otherwise
   */
  static bool IsLocalLabel(const std::string& label);

  /**
   * @brief Trim whitespace from both ends
   *
   * @param str Input string
   * @return Trimmed string
   */
  static std::string Trim(const std::string& str);

  /**
   * @brief Format error message with source location
   *
   * @param message Error message
   * @return Formatted error string
   */
  std::string FormatError(const std::string& message) const;

  // -------------------------------------------------------------------------
  // Complexity-reduction helpers (extracted from high-CC functions)
  // -------------------------------------------------------------------------

  /** Bounds of a .DO/.LU conditional block. */
  struct DoBlockBounds {
    size_t else_line = std::string::npos;  ///< index of .ELSE line, or npos if none
    size_t fin_line = std::string::npos;   ///< index of the matching .FIN line
  };

  /**
   * @brief Scan source for the matching .FIN (and optional .ELSE) of a
   *        .DO or .LU block starting at start_idx+1.
   */
  static DoBlockBounds FindDoBlockBounds(const std::vector<std::string>& source, size_t start_idx);

  /**
   * @brief Emit a label atom (local or global) at addr, inserting into the
   *        atoms vector at atom_insert_pos.
   */
  void EmitDoLabel(const std::string& label, uint32_t addr, size_t atom_insert_pos,
                   Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Flush pending_label_ as an address-label at current_address_.
   *        Used when pending_label_ precedes a non-.EQ/.SE opcode.
   */
  void FlushPendingLabelAsAddress(Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Check for *LABEL .EQ / *LABEL .SE private-label pattern and
   *        strip the leading '*', returning the remainder in result.
   * @return true if line is a private-label line (result set), false otherwise.
   */
  static bool TryHandlePrivateLabelEq(const std::string& line, std::string& result);

  /**
   * @brief Advance pos past leading whitespace and Apple II control chars.
   */
  static void SkipToLabelStart(const std::string& line, size_t& pos);

  /**
   * @brief Scan the label body starting at pos and return the label token.
   *        pos is left pointing to the first character after the label.
   */
  static std::string ScanLabelToken(const std::string& line, size_t& pos);

  /**
   * @brief Find matching .ENDU line index (handles nesting).
   * @return Line index or std::string::npos if not found.
   */
  size_t FindEnduBounds(const std::vector<std::string>& source, size_t start_idx) const;

  /**
   * @brief Define any label on a .ELSE or .FIN boundary line at current addr.
   */
  void DefineBoundaryLabel(const std::vector<std::string>& source, size_t boundary_idx,
                           Section& section, ConcreteSymbolTable& symbols);

  // -------------------------------------------------------------------------
  // ParseNumber helpers (one per number format)
  // -------------------------------------------------------------------------
  static uint32_t ParseHexNumber(const std::string& trimmed);
  static uint32_t ParseBinaryNumber(const std::string& trimmed);
  static uint32_t ParseCharConstant(const std::string& trimmed);
  static uint32_t ParseDecimalNumber(const std::string& trimmed);

  // -------------------------------------------------------------------------
  // TryHandlePrivateLabelEq helpers
  // -------------------------------------------------------------------------
  /** Scan *LABEL token; return label (without '*') or "" if invalid chars. */
  static std::string ScanPrivateLabelToken(const std::string& line);
  /** Find directive opcode in a *LABEL <directive> line; return "" if absent. */
  static std::string ExtractDirectiveFromStarLine(const std::string& line, size_t label_end);

  // -------------------------------------------------------------------------
  // StripEditorCommands helpers
  // -------------------------------------------------------------------------
  /** Return true if token (already uppercased) is a known editor command. */
  static bool IsEditorCommand(const std::string& token_upper);
  /** Build and return the uppercased first token starting at pos. */
  static std::string BuildCommandToken(const std::string& line, size_t pos);

  // -------------------------------------------------------------------------
  // ParseLabel helpers
  // -------------------------------------------------------------------------
  /** Return true if label_upper is a known opcode, macro, or pseudo-op. */
  bool IsOpcodeOrMacro(const std::string& label_upper) const;
  /** Classify and return the label type: local (.N / :N), private, global. */
  static std::string ClassifyAndReturnLabel(const std::string& raw, bool is_private);

  // -------------------------------------------------------------------------
  // ParseLine helpers
  // -------------------------------------------------------------------------
  /** Resolve pending_label_ when current opcode is known. */
  void ResolvePendingLabel(const std::string& opcode_upper, std::string& label, Section& section,
                           ConcreteSymbolTable& symbols);
};

}  // namespace xasm
