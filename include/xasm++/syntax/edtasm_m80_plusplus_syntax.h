/**
 * @file edtasm_m80_plusplus_syntax.h
 * @brief EDTASM-M80++ Syntax Parser (M80/ZMAC/Z80ASM superset)
 *
 * This file defines a universal parser for Z80 assembly syntax that accepts
 * source code from three major Z80 assemblers without requiring mode flags:
 * - M80 (Microsoft's original Z80 assembler)
 * - ZMAC (Modern cross-assembler, superset of M80)
 * - Z80ASM (TRS-80 focused assembler with unique local label syntax)
 *
 * The parser implements a comprehensive superset of all three syntaxes with
 * context-aware disambiguation for conflicting features (e.g., $ prefix for
 * hex numbers vs. Z80ASM local labels).
 *
 * @note Universal Syntax Plugin - No mode flags required
 */

#pragma once

#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/segment_manager.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
#include <cctype>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

// Forward declarations
class CpuZ80;

/**
 * @brief Z80-specific number format parser
 *
 * Parses Z80 assembly number formats with suffixes:
 * - Hexadecimal: 0FFH, 9FH (H suffix)
 * - Octal: 377O, 377Q (O/Q suffix)
 * - Binary: 11111111B (B suffix)
 * - Decimal: 255D (D suffix)
 *
 * All formats must start with a digit to disambiguate from identifiers.
 *
 * @par Example
 * @code
 * Z80NumberParser parser;
 * int64_t value;
 * if (parser.TryParse("0FFH", value)) {
 *   // value == 255
 * }
 * @endcode
 */
class Z80NumberParser : public INumberParser {
public:
  /**
   * @brief Construct with default radix 10
   */
  Z80NumberParser() : radix_(10) {}

  /**
   * @brief Attempt to parse a Z80-specific number format
   *
   * @param token The token to parse (e.g., "0FFH", "377O")
   * @param value Output parameter - receives the parsed value
   * @return true if successfully parsed, false otherwise
   */
  bool TryParse(const std::string &token, int64_t &value) const override;

  /**
   * @brief Set default radix for numbers without explicit format
   *
   * @param radix Number base (2-16)
   */
  void SetRadix(int radix) { radix_ = radix; }

  /**
   * @brief Get current radix
   *
   * @return Current number base
   */
  int GetRadix() const { return radix_; }

private:
  int radix_; ///< Default number base (2-16, default 10)
};

/**
 * @brief EDTASM-M80++ Syntax Parser
 *
 * The EdtasmM80PlusPlusSyntaxParser (formerly Z80UniversalSyntaxParser)
 * implements a universal syntax that accepts source code from M80, ZMAC,
 * and Z80ASM assemblers as a comprehensive superset. No mode flags are
 * required; the parser automatically handles syntax variations through
 * context-aware parsing.
 *
 * @par Supported Features
 * - **Comment Styles**:
 *   - `;` for inline and full-line comments
 *   - `.COMMENT delim...delim` for block comments (M80)
 *   - `comment X...X` for block comments (ZMAC)
 *
 * - **Label Types**:
 *   - `LABEL:` - Private label (single colon)
 *   - `LABEL::` - Public label (double colon, M80 style)
 *   - `$LOCAL:` - Local label scoped to parent (Z80ASM style)
 *   - `.local:` - Local label scoped to parent (ZMAC style)
 *   - `?TEMP:` - Temporary label (ZMAC style)
 *
 * - **Data Directives**:
 *   - `DB`/`DEFB`/`BYTE`/`DM`/`DEFM`/`TEXT`/`ASCII` - Define bytes
 *   - `DW`/`DEFW`/`WORD` - Define words (16-bit)
 *   - `DS`/`DEFS`/`DSPACE`/`BLOCK` - Reserve space
 *   - `DC` - Define constant with count or high-bit-set string
 *   - `DEF3`/`D3` - Define 24-bit constants (ZMAC)
 *   - `DEFD`/`DWORD` - Define 32-bit constants (ZMAC)
 *   - `INCBIN` - Include binary file
 *
 * - **Symbol Directives**:
 *   - `EQU`, `=` - Fixed equate (constant)
 *   - `SET`, `DEFL`, `=` - Redefinable equate (variable)
 *   - `PUBLIC`/`GLOBAL`/`ENTRY` - Export symbols
 *   - `EXTERN`/`EXTRN`/`EXT` - Import symbols
 *
 * - **Segment Directives**:
 *   - `CSEG` - Code segment
 *   - `DSEG` - Data segment
 *   - `ASEG` - Absolute segment
 *   - `COMMON /name/` - Common block
 *   - `ORG address` - Set origin
 *   - `PHASE`/`DEPHASE` - Overlay sections (ZMAC)
 *   - `END [entry]` - End assembly
 *
 * - **Macro Directives**:
 *   - `MACRO`/`ENDM` - Macro definition
 *   - `LOCAL` - Local symbols in macros
 *   - `EXITM` - Early exit from macro
 *   - `REPT count` - Repeat block
 *   - `IRP symbol,<list>` - Iterate over list
 *   - `IRPC symbol,string` - Iterate over string
 *   - Parameter substitution with & and %
 *
 * - **Conditional Directives**:
 *   - `IF`/`ELSE`/`ENDIF` - Basic conditional
 *   - `IFDEF`/`IFNDEF` - Symbol existence
 *   - `IFEQ`/`IFNE` - Equality comparison
 *   - `IFLT`/`IFGT`/`IFLE`/`IFGE` - Relational comparison
 *   - `IF1`/`IF2` - Pass-based conditional (M80)
 *   - `IFB`/`IFNB` - Blank/non-blank (M80)
 *   - `IFIDN`/`IFDIF` - Identical/different (M80)
 *
 * - **Number Formats**:
 *   - `$FF` - Hex with $ prefix
 *   - `0xFF` - Hex with 0x prefix
 *   - `0FFH` - Hex with H suffix
 *   - `11111111B` - Binary with B suffix
 *   - `377O`, `377Q` - Octal with O/Q suffix
 *   - `255`, `255D` - Decimal
 *   - Context disambiguation: $ before digit = hex, $ before letter = label
 *
 * - **Listing Control**:
 *   - `.LIST`/`.XLIST` - Toggle listing
 *   - `*LIST ON/OFF` - Z80ASM style
 *   - `.TITLE`, `.SUBTTL` - Titles
 *   - `EJECT` - Page break
 *   - `SPACE` - Blank lines
 *   - `NAME` - Module name
 *
 * - **Special Features**:
 *   - `*RADIX`, `.RADIX` - Set default number base (2-16)
 *   - `DATE`/`TIME` - Insert system date/time (Z80ASM)
 *   - Undocumented Z-80: `ixh`/`ixl`/`iyh`/`iyl`, `sl1`
 *   - Optional: Cycle counting (`t`, `tilo`, `tihi`, `ocf`) - ZMAC
 *
 * @par Local Label Scoping
 * Local labels (starting with `.` or `$` or `?`) are scoped to the most
 * recent global label. This allows reusing simple names without conflicts:
 * @code
 * SUBROUTINE1:
 * .loop:           ; Resolves to SUBROUTINE1.loop
 *   DJNZ .loop
 *   RET
 *
 * SUBROUTINE2:
 * .loop:           ; Resolves to SUBROUTINE2.loop (different from above)
 *   DJNZ .loop
 *   RET
 * @endcode
 *
 * @par Syntax Disambiguation
 * The parser uses context to disambiguate conflicting syntax:
 * - `$FF` - Hex number ($ before hex digits)
 * - `$LOOP` - Local label ($ before identifier)
 * - `.LIST` - Directive (dot before keyword)
 * - `.loop` - Local label (dot before identifier)
 *
 * @par Example Usage
 * @code
 * EdtasmM80PlusPlusSyntaxParser parser;
 * Section section("CODE", 0x01, 0x8000);
 * ConcreteSymbolTable symbols;
 *
 * std::string source = R"(
 *         ORG 8000H
 * START:  LD A,$42
 *         LD (HL),A
 * .loop:  DJNZ .loop
 *         RET
 * )";
 *
 * parser.Parse(source, section, symbols);
 * @endcode
 *
 * @note Renamed from Z80UniversalSyntaxParser to align with architecture plan.
 *       The name EDTASM-M80++ reflects its role as a universal M80/ZMAC/Z80ASM
 *       superset parser in the Zilog syntax family.
 */
class EdtasmM80PlusPlusSyntaxParser {
  // Friend declarations for directive handlers
  friend void RegisterEdtasmDirectiveHandlers(DirectiveRegistry &registry);

public:
  /**
   * @brief Constructor - initializes parser state
   */
  EdtasmM80PlusPlusSyntaxParser();

  /**
   * @brief Set CPU plugin for undocumented instruction support
   *
   * @param cpu Pointer to CPU plugin (must remain valid during parsing)
   */
  void SetCpu(CpuZ80 *cpu);

  /**
   * @brief Mark END directive as seen
   *
   * Called by directive handlers when END directive is processed.
   */
  void MarkEndDirectiveSeen() { end_directive_seen_ = true; }

  /**
   * @brief Set listing title
   * @param title New listing title
   */
  void SetListingTitle(const std::string &title) { listing_title_ = title; }

  /**
   * @brief Set listing subtitle
   * @param subtitle New listing subtitle
   */
  void SetListingSubtitle(const std::string &subtitle) {
    listing_subtitle_ = subtitle;
  }

  /**
   * @brief Enable listing output
   */
  void EnableListing() { listing_enabled_ = true; }

  /**
   * @brief Disable listing output
   */
  void DisableListing() { listing_enabled_ = false; }

  /**
   * @brief Check if listing is enabled
   * @return True if listing enabled
   */
  bool IsListingEnabled() const { return listing_enabled_; }

  /**
   * @brief Set module name
   * @param name Module name from NAME directive
   */
  void SetModuleName(const std::string &name) { module_name_ = name; }

  /**
   * @brief Get segment manager
   * @return Reference to segment manager
   */
  SegmentManager &GetSegmentManager() { return segment_manager_; }

  /**
   * @brief Set number radix
   * @param radix Number base (2-16)
   * @throw std::runtime_error if radix out of range
   */
  void SetRadix(int radix);

  /**
   * @brief Push conditional block onto stack
   * @param condition Evaluation result of IF condition
   */
  void PushConditional(bool condition);

  /**
   * @brief Toggle to ELSE block
   * @throw std::runtime_error if no matching IF
   */
  void ToggleConditional();

  /**
   * @brief Pop conditional block from stack
   * @throw std::runtime_error if stack empty
   */
  void PopConditional();

  /**
   * @brief Check if code emission should be suppressed
   * @return True if inside false conditional
   */
  bool ShouldSuppressEmission() const;

  /**
   * @brief Parse Z80 assembly source into atoms and symbols
   *
   * Parses the provided Z80-format assembly source and populates
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

  /**
   * @brief Parse expression from string
   * @param str Expression string
   * @param symbols Symbol table for symbol resolution
   * @return Expression AST
   */
  std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                              ConcreteSymbolTable &symbols);

  /**
   * @brief Parse comma-separated symbol list
   * @param operand Symbol list string
   * @return Vector of symbol names
   */
  std::vector<std::string> ParseSymbolList(const std::string &operand);

  /**
   * @brief Conditional assembly block state
   *
   * Tracks IF/ELSE/ENDIF conditional assembly blocks.
   */
  struct ConditionalBlock {
    bool condition;     ///< True if condition is met
    bool in_else_block; ///< True if currently in ELSE block
    bool should_emit;   ///< True if code should be emitted
  };

  std::vector<ConditionalBlock>
      conditional_stack_; ///< Stack of nested conditionals

  /**
   * @brief Macro definition
   *
   * Stores a macro body for later expansion.
   */
  struct MacroDefinition {
    std::string name;                ///< Macro name
    std::vector<std::string> body;   ///< Lines of macro body (unexpanded)
    std::vector<std::string> params; ///< Parameter names
    std::vector<std::string> locals; ///< LOCAL symbols in macro
  };

  /**
   * @brief Type of repeat block being processed
   *
   * Used to track whether we're capturing REPT, IRP, or IRPC blocks.
   */
  enum class RepeatType { NONE, REPT, IRP, IRPC };

  // Macro state (accessible to directive handlers)
  bool in_macro_definition_ = false; ///< True if defining a macro
  MacroDefinition current_macro_;    ///< Current macro being defined
  std::unordered_map<std::string, MacroDefinition> macros_; ///< Defined macros
  int macro_expansion_depth_ = 0; ///< Prevent infinite recursion
  int macro_unique_counter_ = 0;  ///< Counter for LOCAL label uniqueness
  int next_macro_unique_id_ =
      0; ///< ID for next macro expansion (for LOCAL labels)
  bool exitm_triggered_ =
      false; ///< True if EXITM was encountered in current macro expansion
  int macro_nesting_depth_ =
      0; ///< Track nesting depth when capturing macro body
  std::set<std::string>
      macro_local_labels_; ///< Set of current macro LOCAL labels (unique names)
                           ///< that should not create atoms

  // Repeat block state (REPT/IRP/IRPC) (accessible to directive handlers)
  RepeatType in_repeat_block_ =
      RepeatType::NONE; ///< Type of repeat block being captured
  int rept_count_ = 0;  ///< Repeat count for REPT
  std::vector<std::string> repeat_body_; ///< Lines in repeat block
  int repeat_nesting_depth_ = 0;         ///< Track nested REPT/IRP/IRPC blocks
  std::string repeat_param_;             ///< Parameter name for IRP/IRPC
  std::vector<std::string> repeat_values_; ///< Values for IRP iteration

  /**
   * @brief Expand and parse lines (for directive handlers)
   *
   * Used by REPT/IRP/IRPC directive handlers to expand repeat blocks.
   * @note Internal use - called by directive handlers
   */
  void ExpandAndParseLines(const std::vector<std::string> &lines,
                           Section &section, ConcreteSymbolTable &symbols);

  /**
   * @brief Substitute macro parameters in a line (for directive handlers)
   *
   * Used by IRP/IRPC directive handlers for parameter substitution.
   * @note Internal use - called by directive handlers
   */
  std::string
  SubstituteMacroParameters(const std::string &line,
                            const std::vector<std::string> &param_names,
                            const std::vector<std::string> &param_values);

private:
  /**
   * @brief Label scope for managing local labels
   *
   * Local labels (.local, $local, ?temp) are scoped to the most recent
   * global label.
   */
  struct LabelScope {
    std::string global_label; ///< The global label this scope belongs to
    std::unordered_map<std::string, uint32_t>
        local_labels; ///< local_name -> address
  };

  LabelScope current_scope_; ///< Current label scope (for local labels)

  uint32_t current_address_; ///< Current address (for tracking label addresses)
  bool end_directive_seen_;  ///< True if END directive has been processed

  // Source location tracking (for error reporting)
  std::string current_file_; ///< Current source filename
  int current_line_;         ///< Current line number

  // Listing control state
  bool listing_enabled_;         ///< True if listing output enabled
  std::string listing_title_;    ///< Listing title (TITLE directive)
  std::string listing_subtitle_; ///< Listing subtitle (SUBTTL directive)
  std::string module_name_;      ///< Module name (NAME directive)

  // Special features state
  int current_radix_; ///< Current number base (2-16, default 10)

  CpuZ80 *cpu_ = nullptr; ///< CPU plugin for undocumented instructions

  DirectiveRegistry directive_registry_; ///< Registry for directive handlers
  SegmentManager segment_manager_; ///< Manages segments (CSEG/DSEG/ASEG/COMMON)

  // Expression and number parsing
  Z80NumberParser z80_number_parser_; ///< Z80-specific number parser

  // Parsing helpers
  void InitializeDirectiveRegistry(); ///< Register all directives
  std::string StripComments(const std::string &line);
  std::string Trim(const std::string &str);

  // Macro helpers
  std::string MakeLocalLabelUnique(const std::string &line,
                                   const std::vector<std::string> &local_labels,
                                   int unique_id);

  void ParseLine(const std::string &line, Section &section,
                 ConcreteSymbolTable &symbols);

  // Label parsing
  std::string ParseLabel(const std::string &line, size_t &pos, Section &section,
                         ConcreteSymbolTable &symbols);

  // Instruction size estimation
  uint32_t EstimateZ80InstructionSize(const std::string &mnemonic,
                                      const std::string &operand);

  // Expression/number parsing
  uint32_t ParseNumber(const std::string &str);

  // Error formatting with source location
  std::string FormatError(const std::string &message) const;
};

} // namespace xasm
