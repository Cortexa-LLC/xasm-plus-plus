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

#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace xasm {

// Forward declaration
class CpuPlugin;

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
  bool TryParse(const std::string &token, int64_t &value) const override;
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
  void SetCpu(CpuPlugin *cpu);

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
  void Parse(const std::string &source, Section &section,
             ConcreteSymbolTable &symbols);

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
  uint32_t EvaluateExpression(const std::string &str,
                              ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .MA directive (macro definition start)
   *
   * Public for use by extracted directive handlers.
   * Begins macro definition. Macro name can be specified as operand or label.
   *
   * @param label Optional label before directive
   * @param operand Macro name (if not in label)
   */
  void HandleMa(const std::string &label, const std::string &operand);

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
  void SetCurrentFile(const std::string &file);

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
   * Called by .DUMMY directive handler.
   */
  void StartDummySection();

  /**
   * @brief Exit dummy section mode
   *
   * Called by .ED directive handler.
   */
  void EndDummySection();

private:
  // Directive handler function signature (DirectiveContext pattern)
  using DirectiveHandler =
      std::function<void(const std::string &label, const std::string &operand,
                         DirectiveContext &context)>;

  // Macro definition structure
  struct MacroDef {
    std::string name;
    std::vector<std::string> lines; ///< Macro body lines
  };

  // Current state
  uint32_t current_address_; ///< Current assembly address
  std::string current_file_; ///< Current source filename
  int current_line_;         ///< Current line number (for errors)

  // CPU plugin for opcode validation
  CpuPlugin *cpu_; ///< CPU plugin for opcode validation (nullable)

  // Symbol tracking for .SE (redefinable)
  std::unordered_map<std::string, bool>
      variable_symbols_; ///< Track .SE symbols

  // Phase 3: Macros, Conditionals, Local Labels, Loops
  std::unordered_map<std::string, MacroDef> macros_; ///< Defined macros
  std::unordered_map<std::string, uint32_t>
      local_labels_;               ///< Local labels (.0-.9)
  bool in_macro_definition_;       ///< Currently defining a macro
  std::string current_macro_name_; ///< Name of macro being defined
  std::vector<std::string>
      current_macro_body_;     ///< Lines of macro being defined
  int macro_invocation_depth_; ///< Nesting depth for macro invocations

  // Dummy section support (structure definitions)
  bool in_dummy_section_; ///< Currently in dummy section (.DUMMY active)

  // Directive registry
  std::unordered_map<std::string, DirectiveHandler> directive_registry_;

  // Expression and number parsing (Phase 2 integration)
  SCMASMNumberParser scmasm_number_parser_; ///< SCMASM-specific number parser

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
  std::string StripLineNumber(const std::string &line);

  /**
   * @brief Strip comments from line
   *
   * Removes `;` inline comments and `*` column-1 comments.
   *
   * @param line Input line
   * @return Line with comments removed
   */
  std::string StripComments(const std::string &line);

  /**
   * @brief Parse a single line of source
   *
   * @param line Line to parse
   * @param section Section to add atoms to
   * @param symbols Symbol table for label definitions
   * @param source All source lines (for multi-line directives)
   * @param line_idx Current line index (updated by multi-line directives)
   */
  void ParseLine(const std::string &line, Section &section,
                 ConcreteSymbolTable &symbols,
                 const std::vector<std::string> &source, size_t &line_idx);

  /**
   * @brief Parse label at start of line
   *
   * @param line Input line
   * @param pos Position in line (updated)
   * @param section Section to add label atom
   * @param symbols Symbol table
   * @return Label name (empty if none)
   */
  std::string ParseLabel(const std::string &line, size_t &pos, Section &section,
                         ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .OR directive (set origin)
   *
   * @param operand Directive operand
   * @param section Section to add org atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleOr(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .EQ directive (define constant)
   *
   * @param label Symbol name
   * @param operand Expression to evaluate
   * @param symbols Symbol table to add symbol
   */
  void HandleEq(const std::string &label, const std::string &operand,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .SE directive (set variable)
   *
   * Like .EQ but allows redefinition.
   *
   * @param label Symbol name
   * @param operand Expression to evaluate
   * @param symbols Symbol table to add/update symbol
   */
  void HandleSe(const std::string &label, const std::string &operand,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .AS directive (ASCII string)
   *
   * Emits ASCII characters. High-bit rule applies based on delimiter.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAs(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .AT directive (ASCII text with high bit on last char)
   *
   * Like .AS but sets high bit on last character.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAt(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .AZ directive (ASCII zero-terminated)
   *
   * Like .AS but adds $00 terminator.
   *
   * @param operand String operand
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleAz(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .DA directive (define address/data)
   *
   * Emits comma-separated numeric values (bytes or words).
   *
   * @param operand Comma-separated values
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleDa(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .HS directive (hex string)
   *
   * Converts hex digits to bytes (no $ prefix needed).
   *
   * @param operand Hex digits
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleHs(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

  /**
   * @brief Parse .BS directive (bit string)
   *
   * Converts binary digits to bytes.
   *
   * @param operand Binary digits
   * @param section Section to add data atom
   * @param symbols Symbol table for expression evaluation
   */
  void HandleBs(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols);

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
  void InvokeMacro(const std::string &name,
                   const std::vector<std::string> &params, Section &section,
                   ConcreteSymbolTable &symbols);

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
  void HandleDo(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols,
                const std::vector<std::string> &source, size_t &line_idx);

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
  void HandleLu(const std::string &operand, Section &section,
                ConcreteSymbolTable &symbols,
                const std::vector<std::string> &source, size_t &line_idx);

  /**
   * @brief Substitute macro parameters in line
   *
   * Replaces \0-\9 with corresponding parameters.
   *
   * @param line Line with parameter references
   * @param params Parameter values
   * @return Line with parameters substituted
   */
  std::string SubstituteParameters(const std::string &line,
                                   const std::vector<std::string> &params);

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
  uint32_t ParseNumber(const std::string &str);

  /**
   * @brief Parse expression
   *
   * @param str Expression string
   * @param symbols Symbol table for lookups
   * @return Expression object
   */
  std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                              ConcreteSymbolTable &symbols);

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
  uint8_t ApplyHighBitRule(char c, char delimiter);

  /**
   * @brief Parse string with delimiter semantics
   *
   * Extracts string from operand and applies high-bit rule.
   *
   * @param operand String operand (with delimiters)
   * @param result Output vector of bytes
   * @return Delimiter character used
   */
  char ParseString(const std::string &operand, std::vector<uint8_t> &result);

  /**
   * @brief Check if label is a local label (.0-.9)
   *
   * @param label Label to check
   * @return true if local label, false otherwise
   */
  bool IsLocalLabel(const std::string &label);

  /**
   * @brief Trim whitespace from both ends
   *
   * @param str Input string
   * @return Trimmed string
   */
  std::string Trim(const std::string &str);

  /**
   * @brief Format error message with source location
   *
   * @param message Error message
   * @return Formatted error string
   */
  std::string FormatError(const std::string &message) const;
};

} // namespace xasm
