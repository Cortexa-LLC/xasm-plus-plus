/**
 * @file flex_syntax.h
 * @brief FLEX ASM09 assembly syntax parser for Motorola 6809
 *
 * This file defines the FLEX ASM09 assembly syntax parser for the 6809.
 * FLEX ASM09 is the assembler for the FLEX Disk Operating System (DOS)
 * developed by Technical Systems Consultants (TSC).
 *
 * @note Phase: FLEX ASM09 Parser Implementation
 */

#pragma once

#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/common/macro_processor.h"
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

/**
 * @brief FLEX ASM09 assembly syntax parser
 *
 * The FlexAsmSyntax implements the FLEX ASM09 assembler syntax used on
 * FLEX DOS systems. It supports standard 6809 directives, macro capabilities,
 * and conditional assembly.
 *
 * @par Supported Syntax
 * - **Comments**: `* comment` (column 1) or `; comment` (inline)
 * - **Labels**: `LABEL` or `LABEL    OPCODE`
 * - **Directives**: `ORG`, `END`, `EQU`, `SET`, `FCB`, `FDB`, `FCC`, `RMB`,
 * `SETDP`
 * - **Macros**: `MACRO`/`ENDM` with parameter substitution
 * - **Conditionals**: `IFC`/`ENDC` for conditional assembly
 * - **Numbers**: Decimal (default), `$` hex, `%` binary, `'A'` ASCII
 * - **Instructions**: Standard 6809 mnemonics
 *
 * @par Example Source
 * @code
 * * FLEX program example
 *         ORG     $0400
 *
 * SCREEN  EQU     $0400
 *
 * START   LDA     #$00
 *         STA     SCREEN
 *         RTS
 *
 *         END     START
 * @endcode
 *
 * @par Macro Example
 * @code
 * MOVB    MACRO   SRC,DEST
 *         LDA     SRC
 *         STA     DEST
 *         ENDM
 *
 *         MOVB    $80,$90
 * @endcode
 *
 * @par Key Features
 * - Case-insensitive by default (6809 tradition)
 * - Macro preprocessor with parameter substitution
 * - Conditional assembly (IFC/ENDC)
 * - Flexible FCC delimiter support
 * - Big-endian 16-bit values (FDB)
 * - Direct page optimization (SETDP)
 *
 * @par Usage
 * @code
 * FlexAsmSyntax parser;
 * Section section("CODE", 0);
 * ConcreteSymbolTable symbols;
 *
 * std::string source = "        ORG $0400\nSTART   LDA #$00\n        RTS\n END
 * START"; parser.Parse(source, section, symbols);
 * // section.atoms now contains the parsed atoms
 * @endcode
 */
class FlexAsmSyntax {
public:
  /**
   * @brief Default constructor
   */
  FlexAsmSyntax() = default;

  /**
   * @brief Parse FLEX ASM09 assembly source into atoms and symbols
   *
   * Parses the provided FLEX assembly source code and populates the section
   * with atoms (labels, instructions, data, etc.) and the symbol table
   * with label definitions and constants.
   *
   * @param source Assembly source code (multi-line string)
   * @param section Section to populate with atoms
   * @param symbols Symbol table to populate with label definitions
   *
   * @throws std::runtime_error on parse errors (syntax errors, invalid
   * directives, etc.)
   */
  void Parse(const std::string &source, Section &section,
             ConcreteSymbolTable &symbols);

protected:
  // ========== Macro Processor Methods (Protected for Testing) ==========

  /**
   * @brief Substitute parameters in macro line
   *
   * @param line Macro body line
   * @param macro Macro definition
   * @param arguments Argument values
   * @return Line with parameters substituted
   */
  std::string SubstituteParameters(const std::string &line,
                                   const MacroDefinition &macro,
                                   const std::vector<std::string> &arguments);

  /**
   * @brief Make local label unique for this expansion
   *
   * @param label Label name (may start with .)
   * @param expansion_id Unique expansion ID
   * @return Unique label name
   */
  std::string MakeLocalLabelUnique(const std::string &label, int expansion_id);

  /**
   * @brief Expand macro with arguments
   *
   * @param name Macro name
   * @param arguments Argument values
   * @return Expanded source lines
   */
  std::vector<std::string>
  ExpandMacro(const std::string &name,
              const std::vector<std::string> &arguments);

  /**
   * @brief Check if macro is defined
   *
   * @param name Macro name
   * @return true if macro exists
   */
  bool IsMacroDefined(const std::string &name) const;

  // ========== Macro Processor State (Protected for Testing) ==========
  
  std::unordered_map<std::string, MacroDefinition> macros_; ///< Defined macros
  int expansion_counter_ = 0;     ///< Unique ID for macro expansions

private:
  /**
   * @brief Parser mode state machine
   */
  enum class ParserMode {
    Normal,             ///< Normal parsing
    InMacroDefinition,  ///< Collecting macro definition
    InRepeatBlock,      ///< Collecting repeat block
    SkippingConditional ///< Skipping conditional block (IFC false)
  };

  ParserMode mode_ = ParserMode::Normal; ///< Current parser mode
  uint32_t current_address_ = 0;         ///< Current assembly address
  uint8_t direct_page_ = 0;              ///< Current direct page value
  std::string title_;                    ///< Program title (NAM/TTL)
  std::string subtitle_;                 ///< Program subtitle (STTL)

  // Macro processor state (macros_ and expansion_counter_ moved to protected)
  MacroDefinition current_macro_; ///< Macro being defined
  MacroProcessor macro_processor_; ///< Macro processor instance

  // Conditional assembly state
  std::stack<bool> conditional_stack_; ///< Condition evaluation stack
  ConcreteSymbolTable *current_symbols_ = nullptr; ///< Symbol table for current parse

  // Repeat block state
  struct RepeatBlock {
    int count;
    std::vector<std::string> lines;
  };
  std::stack<RepeatBlock> repeat_stack_;

  /**
   * @brief Parse a single line of FLEX source
   *
   * @param line Line to parse (without newline)
   * @param section Section to add atoms to
   * @param symbols Symbol table for definitions
   */
  void ParseLine(const std::string &line, Section &section,
                 ConcreteSymbolTable &symbols);

  /**
   * @brief Parse directive (ORG, END, EQU, SET, FCB, FDB, FCC, RMB, SETDP,
   * etc.)
   *
   * @param directive Directive name (uppercase)
   * @param operands Operand string
   * @param label Optional label (may be empty)
   * @param section Section to add atoms to
   * @param symbols Symbol table
   */
  void ParseDirective(const std::string &directive, const std::string &operands,
                      const std::string &label, Section &section,
                      ConcreteSymbolTable &symbols);

  /**
   * @brief Parse numeric value (supports $hex, %binary, 'char', decimal)
   *
   * @param str String to parse
   * @return Parsed numeric value
   * @throws std::runtime_error on invalid format
   */
  uint32_t ParseNumber(const std::string &str);

  /**
   * @brief Strip comments from line
   *
   * @param line Line to process
   * @return Line without comments
   */
  std::string StripComments(const std::string &line);

  /**
   * @brief Trim whitespace from both ends
   *
   * @param str String to trim
   * @return Trimmed string
   */
  std::string Trim(const std::string &str);

  /**
   * @brief Convert string to uppercase
   *
   * @param str String to convert
   * @return Uppercase string
   */
  std::string ToUpper(const std::string &str);

  /**
   * @brief Check if line is a comment line (starts with *)
   *
   * @param line Line to check
   * @return true if comment line
   */
  bool IsCommentLine(const std::string &line);

  // ========== Macro Processor Methods ==========

  /**
   * @brief Check if a name is a defined macro
   *
   * @param name Name to check
   * @return true if macro is defined
   */
  bool IsMacro(const std::string &name) const;

  // ========== Conditional Assembly Methods ==========

  /**
   * @brief Evaluate condition expression
   *
   * @param condition Condition string
   * @return true if condition is true
   */
  bool EvaluateCondition(const std::string &condition);

  /**
   * @brief Check if we should assemble current line
   *
   * @return true if all conditions on stack are true
   */
  bool ShouldAssemble() const;
};

} // namespace xasm
