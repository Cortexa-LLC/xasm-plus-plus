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

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/expression.h"

namespace xasm {

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
    /**
     * @brief Constructor - initializes parser state
     */
    MerlinSyntaxParser();

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
     * @throws std::runtime_error on parse errors (syntax errors, undefined labels, etc.)
     */
    void Parse(const std::string& source, Section& section, ConcreteSymbolTable& symbols);

private:
    /**
     * @brief Label scope for managing :LOCAL labels
     * 
     * Local labels are scoped to the most recent global label.
     */
    struct LabelScope {
        std::string global_label;  ///< The global label this scope belongs to
        std::unordered_map<std::string, uint32_t> local_labels;  ///< :label -> address
    };

    LabelScope current_scope_;      ///< Current label scope (for :LOCAL labels)

    // DUM block state
    bool in_dum_block_;             ///< True if currently inside a DUM block
    uint32_t dum_address_;          ///< Current address within DUM block
    std::unordered_map<std::string, uint32_t> variable_labels_;  ///< ]variable -> offset

    uint32_t current_address_;      ///< Current address (for tracking label addresses)

    std::vector<std::string> include_stack_;    ///< Include file tracking (for circular detection)

    // Source location tracking (for error reporting)
    std::string current_file_;      ///< Current source filename
    int current_line_;              ///< Current line number

    /**
     * @brief Conditional assembly block state
     * 
     * Tracks DO/ELSE/FIN conditional assembly blocks.
     */
    struct ConditionalBlock {
        bool condition;      ///< True if condition is met
        bool in_else_block;  ///< True if currently in ELSE block
        bool should_emit;    ///< True if code should be emitted
    };
    std::vector<ConditionalBlock> conditional_stack_;   ///< Stack of nested conditionals

    // Parsing helpers
    std::string StripComments(const std::string& line);
    std::string Trim(const std::string& str);
    std::string ToUpper(const std::string& str);

    void ParseLine(const std::string& line, Section& section, ConcreteSymbolTable& symbols);

    // Label parsing
    std::string ParseLabel(const std::string& line, size_t& pos, Section& section,
                          ConcreteSymbolTable& symbols);

    // Directive handlers
    void HandleOrg(const std::string& operand, Section& section,
                   ConcreteSymbolTable& symbols);
    void HandleEqu(const std::string& label, const std::string& operand,
                  ConcreteSymbolTable& symbols);
    void HandleDB(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleDW(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleHex(const std::string& operand, Section& section);
    void HandleDS(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleDum(const std::string& operand, ConcreteSymbolTable& symbols);
    void HandleDend();
    void HandlePut(const std::string& operand, Section& section, ConcreteSymbolTable& symbols);
    void HandleLst(const std::string& operand);
    void HandleLstdo();
    void HandleTr(const std::string& operand);
    void HandleAsc(const std::string& operand, Section& section);
    void HandleDo(const std::string& operand, ConcreteSymbolTable& symbols);
    void HandleElse();
    void HandleFin();

    // Expression/number parsing
    uint32_t ParseNumber(const std::string& str);
    std::shared_ptr<Expression> ParseExpression(const std::string& str,
                                               ConcreteSymbolTable& symbols);

    // Error formatting with source location
    std::string FormatError(const std::string& message) const;
};

} // namespace xasm
