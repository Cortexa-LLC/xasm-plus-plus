/**
 * @file edtasm_syntax.h
 * @brief EDTASM+ assembly syntax parser for TRS-80 Color Computer
 * 
 * This file defines the EDTASM+ assembly syntax parser for the 6809/6309.
 * EDTASM+ is the standard assembler for TRS-80 Color Computer (CoCo) development.
 * 
 * @note Phase: 6809 EDTASM Parser Implementation
 */

#pragma once

#include <string>
#include "xasm++/section.h"
#include "xasm++/symbol.h"

namespace xasm {

/**
 * @brief EDTASM+ assembly syntax parser
 * 
 * The EdtasmSyntaxParser implements the EDTASM+ assembler syntax used on
 * TRS-80 Color Computer systems. It supports standard 6809 directives,
 * macro capabilities, and conditional assembly.
 * 
 * @par Supported Syntax
 * - **Comments**: `* comment` (column 1) or `; comment` (inline)
 * - **Labels**: `LABEL` or `LABEL    OPCODE`
 * - **Directives**: `ORG`, `END`, `EQU`, `SET`, `FCB`, `FDB`, `FCC`, `RMB`, `SETDP`
 * - **Numbers**: Decimal (default), `$` hex, `%` binary, `'A'` ASCII
 * - **Instructions**: Standard 6809 mnemonics
 * 
 * @par Example Source
 * @code
 * * EDTASM program example
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
 * @par Key Features
 * - Case-insensitive by default (6809 tradition)
 * - Column-oriented source format
 * - Flexible FCC delimiter support
 * - Big-endian 16-bit values (FDB)
 * - Direct page optimization (SETDP)
 * 
 * @par Usage
 * @code
 * EdtasmSyntaxParser parser;
 * Section section("CODE", 0);
 * ConcreteSymbolTable symbols;
 * 
 * std::string source = "        ORG $0400\nSTART   LDA #$00\n        RTS\n        END START";
 * parser.Parse(source, section, symbols);
 * // section.atoms now contains the parsed atoms
 * @endcode
 */
class EdtasmSyntaxParser {
public:
  /**
   * @brief Default constructor
   */
  EdtasmSyntaxParser() = default;

  /**
   * @brief Parse EDTASM+ assembly source into atoms and symbols
   * 
   * Parses the provided EDTASM+ assembly source code and populates the section
   * with atoms (labels, instructions, data, etc.) and the symbol table
   * with label definitions and constants.
   * 
   * @param source Assembly source code (multi-line string)
   * @param section Section to populate with atoms
   * @param symbols Symbol table to populate with label definitions
   * 
   * @throws std::runtime_error on parse errors (syntax errors, invalid directives, etc.)
   */
  void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);

private:
  uint32_t current_address_ = 0;    ///< Current assembly address
  uint8_t direct_page_ = 0;          ///< Current direct page value (for SETDP)

  /**
   * @brief Parse a single line of EDTASM+ source
   * 
   * @param line Line to parse (without newline)
   * @param section Section to add atoms to
   * @param symbols Symbol table for definitions
   */
  void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse directive (ORG, END, EQU, SET, FCB, FDB, FCC, RMB, SETDP)
   * 
   * @param directive Directive name (uppercase)
   * @param operands Operand string
   * @param label Optional label (may be empty)
   * @param section Section to add atoms to
   * @param symbols Symbol table
   */
  void ParseDirective(const std::string& directive, const std::string& operands,
                      const std::string& label, Section& section, ConcreteSymbolTable& symbols);

  /**
   * @brief Parse numeric value (supports $hex, %binary, 'char', decimal)
   * 
   * @param str String to parse
   * @return Parsed numeric value
   * @throws std::runtime_error on invalid format
   */
  uint32_t ParseNumber(const std::string& str);

  /**
   * @brief Strip comments from line
   * 
   * @param line Line to process
   * @return Line without comments
   */
  std::string StripComments(const std::string& line);

  /**
   * @brief Trim whitespace from both ends
   * 
   * @param str String to trim
   * @return Trimmed string
   */
  std::string Trim(const std::string& str);

  /**
   * @brief Convert string to uppercase
   * 
   * @param str String to convert
   * @return Uppercase string
   */
  std::string ToUpper(const std::string& str);

  /**
   * @brief Check if line is a comment line (starts with *)
   * 
   * @param line Line to check
   * @return true if comment line
   */
  bool IsCommentLine(const std::string& line);
};

} // namespace xasm
