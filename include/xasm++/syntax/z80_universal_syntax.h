/**
 * @file z80_universal_syntax.h
 * @brief Z80 Universal Syntax Parser (M80/ZMAC/Z80ASM superset)
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

#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

// Forward declarations
class CpuZ80;

/**
 * @brief Z80 Universal Syntax Parser
 *
 * The Z80UniversalSyntaxParser implements a universal syntax that accepts
 * source code from M80, ZMAC, and Z80ASM assemblers as a comprehensive
 * superset. No mode flags are required; the parser automatically handles
 * syntax variations through context-aware parsing.
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
 * Z80UniversalSyntaxParser parser;
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
 */
class Z80UniversalSyntaxParser {
public:
  /**
   * @brief Constructor - initializes parser state
   */
  Z80UniversalSyntaxParser();

  /**
   * @brief Set CPU plugin for undocumented instruction support
   *
   * @param cpu Pointer to CPU plugin (must remain valid during parsing)
   */
  void SetCpu(CpuZ80 *cpu);

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

  /**
   * @brief Macro definition
   *
   * Stores a macro body for later expansion.
   */
  struct MacroDefinition {
    std::string name;                 ///< Macro name
    std::vector<std::string> body;    ///< Lines of macro body (unexpanded)
    std::vector<std::string> params;  ///< Parameter names
    std::vector<std::string> locals;  ///< LOCAL symbols in macro
  };

  // Macro state
  bool in_macro_definition_;      ///< True if defining a macro
  MacroDefinition current_macro_; ///< Current macro being defined
  std::unordered_map<std::string, MacroDefinition> macros_; ///< Defined macros
  int macro_expansion_depth_; ///< Prevent infinite recursion

  uint32_t current_address_; ///< Current address (for tracking label addresses)
  bool end_directive_seen_;  ///< True if END directive has been processed

  // Source location tracking (for error reporting)
  std::string current_file_; ///< Current source filename
  int current_line_;         ///< Current line number

  CpuZ80 *cpu_ = nullptr; ///< CPU plugin for undocumented instructions

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

  // Parsing helpers
  std::string StripComments(const std::string &line);
  std::string Trim(const std::string &str);

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
  void HandleSet(const std::string &label, const std::string &operand,
                 ConcreteSymbolTable &symbols);
  void HandleDB(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleDW(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleDS(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);
  void HandleEnd();
  void HandlePublic(const std::string &operand, ConcreteSymbolTable &symbols);
  void HandleExtern(const std::string &operand, ConcreteSymbolTable &symbols);
  void HandleCseg(Section &section);
  void HandleDseg(Section &section);
  void HandleAseg(Section &section);

  // Conditional handlers
  void HandleIf(const std::string &operand, ConcreteSymbolTable &symbols);
  void HandleElse();
  void HandleEndif();

  // Macro handlers
  void HandleMacro(const std::string &name);
  void HandleEndm();
  void HandleRept(const std::string &operand);

  // Expression/number parsing
  uint32_t ParseNumber(const std::string &str);
  std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                              ConcreteSymbolTable &symbols);

  // Helper: Parse comma-separated symbol list
  std::vector<std::string> ParseSymbolList(const std::string &operand);

  // Error formatting with source location
  std::string FormatError(const std::string &message) const;
};

} // namespace xasm
