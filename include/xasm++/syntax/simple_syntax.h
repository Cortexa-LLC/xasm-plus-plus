/**
 * @file simple_syntax.h
 * @brief Simple assembly syntax parser
 * 
 * This file defines a basic assembly syntax parser that supports
 * fundamental assembly directives and instructions.
 * 
 * @note Phase 1: Minimal Viable Assembler - SimpleSyntax Plugin
 */

#pragma once

#include <string>
#include "xasm++/section.h"
#include "xasm++/symbol.h"

namespace xasm {

/**
 * @brief Simple assembly syntax parser
 * 
 * The SimpleSyntaxParser implements a straightforward assembly language
 * syntax similar to traditional assemblers. It supports basic directives,
 * labels, and instruction syntax.
 * 
 * @par Supported Syntax
 * - **Labels**: `label:` or `label`
 * - **Instructions**: `LDA #$42`, `STA $1234,X`
 * - **Directives**: `.org $8000`, `.byte $01,$02`, `.word $1234`
 * - **Comments**: `; comment` or `// comment`
 * - **Expressions**: `start+2`, `end-start`, `5*3`
 * 
 * @par Example Source
 * @code
 * ; Simple assembly example
 * .org $8000
 * 
 * start:
 *   LDA #$42      ; Load immediate value
 *   STA $0200     ; Store to zero page
 *   JMP start     ; Infinite loop
 * @endcode
 * 
 * @par Usage
 * @code
 * SimpleSyntaxParser parser;
 * Section section("CODE", 0x01, 0x8000);
 * ConcreteSymbolTable symbols;
 * 
 * std::string source = "; Load a value\nLDA #$42\nRTS";
 * parser.Parse(source, section, symbols);
 * // section.atoms now contains the parsed atoms
 * @endcode
 */
class SimpleSyntaxParser {
public:
  /**
   * @brief Default constructor
   */
  SimpleSyntaxParser() = default;

  /**
   * @brief Parse assembly source into atoms and symbols
   * 
   * Parses the provided assembly source code and populates the section
   * with atoms (labels, instructions, data, etc.) and the symbol table
   * with label definitions.
   * 
   * @param source Assembly source code (multi-line string)
   * @param section Section to populate with atoms
   * @param symbols Symbol table to populate with label definitions
   * 
   * @throws std::runtime_error on parse errors (syntax errors, invalid directives, etc.)
   */
  void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);
};

} // namespace xasm
