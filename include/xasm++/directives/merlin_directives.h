/**
 * @file merlin_directives.h
 * @brief Merlin-specific directive handlers extracted from MerlinSyntaxParser
 *
 * This file contains free functions for handling Merlin assembly directives
 * that are specific to the Merlin syntax (as opposed to core directives
 * like ORG, EQU, DB, DW, DS which are handled by core_directive_handlers.h).
 * Part of God-Class Phase 6c.1 refactoring to extract Merlin functionality.
 *
 * @section handlers Handler Functions
 *
 * Each handler function follows the pattern:
 * - Takes operand string, section, symbol table, and other required context
 * - Parses operand according to Merlin syntax rules
 * - Creates appropriate atoms and adds to section
 * - Updates current_address or other state by reference
 * - Throws std::runtime_error on invalid input
 *
 * @section usage Usage Example
 * @code
 * Section section("code");
 * ConcreteSymbolTable symbols;
 * uint32_t address = 0x1000;
 *
 * // HEX directive
 * HandleHexDirective("0102030405", section, address);
 * // section contains DataAtom with 5 bytes, address advanced by 5
 *
 * // ASC directive (Apple II high-bit ASCII)
 * HandleAscDirective("'HELLO'", section, address);
 * // section contains DataAtom with "HELLO" in high-bit ASCII
 * @endcode
 */

#pragma once

#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <cstdint>
#include <string>

namespace xasm {

// Forward declarations
struct DirectiveContext;
class DirectiveRegistry;
class MerlinSyntaxParser;
class ConditionalAssembler;

// NOTE: Core directives (ORG, EQU, DB, DW, DS) are declared in
// src/syntax/core_directive_handlers.cpp and are not Merlin-specific.

// NOTE: Conditional assembly directives (DO, ELSE, FIN) and file inclusion
// (PUT) are NOT extracted as they require tight coupling with parser state
// (ConditionalAssembler, include_stack, recursive parsing). These remain as
// member functions in MerlinSyntaxParser.

/**
 * @brief Handle HEX directive - define hex bytes
 *
 * Creates a DataAtom with hex data. The operand can be:
 * - Concatenated hex digits: "0102030405"
 * - Space-separated: "01 02 03 04 05"
 * - Comma-separated: "01,02,03,04,05"
 *
 * @param operand Hex byte data
 * @param section Section to add DataAtom to
 * @param current_address Current address (advanced by number of bytes)
 *
 * @throws std::runtime_error if operand contains invalid hex digits
 *
 * @par Example
 * @code
 * HandleHexDirective("0102030405", section, address);
 * // section contains DataAtom with [0x01, 0x02, 0x03, 0x04, 0x05]
 * // address advanced by 5
 * @endcode
 */
void HandleHexDirective(const std::string &operand, Section &section,
                        uint32_t &current_address);

/**
 * @brief Handle ASC directive - ASCII string with high bit set
 *
 * Creates a DataAtom with ASCII string data. In Merlin/Apple II convention,
 * ALL characters have their high bit set (0x80 | char) for display purposes.
 *
 * @param operand String operand (single or double quoted)
 * @param section Section to add DataAtom to
 * @param current_address Current address (advanced by string length)
 *
 * @par Example
 * @code
 * HandleAscDirective("'HELLO'", section, address);
 * // section contains DataAtom with [0xC8, 0xC5, 0xCC, 0xCC, 0xCF]
 * // (H=0xC8, E=0xC5, L=0xCC, O=0xCF - all with high bit set)
 * @endcode
 */
void HandleAscDirective(const std::string &operand, Section &section,
                        uint32_t &current_address);

/**
 * @brief Handle DCI directive - DCI string (last char with high bit set)
 *
 * Creates a DataAtom with ASCII string where only the LAST character has
 * its high bit set (0x80 | last_char).
 *
 * @param operand String operand (single or double quoted)
 * @param section Section to add DataAtom to
 * @param current_address Current address (advanced by string length)
 *
 * @par Example
 * @code
 * HandleDciDirective("'HELLO'", section, address);
 * // section contains [0x48, 0x45, 0x4C, 0x4C, 0xCF]
 * // Only last 'O' has high bit set (0xCF)
 * @endcode
 */
void HandleDciDirective(const std::string &operand, Section &section,
                        uint32_t &current_address);

/**
 * @brief Handle INV directive - Inverse ASCII (all chars with high bit set)
 *
 * Creates a DataAtom with ASCII string where ALL characters have their
 * high bit set (same as ASC in Merlin).
 *
 * @param operand String operand (single or double quoted)
 * @param section Section to add DataAtom to
 * @param current_address Current address (advanced by string length)
 *
 * @par Example
 * @code
 * HandleInvDirective("'HELLO'", section, address);
 * // section contains [0xC8, 0xC5, 0xCC, 0xCC, 0xCF]
 * // All chars with high bit set
 * @endcode
 */
void HandleInvDirective(const std::string &operand, Section &section,
                        uint32_t &current_address);

/**
 * @brief Handle FLS directive - Flash ASCII (alternating high bit)
 *
 * Creates a DataAtom with ASCII string where every OTHER character (odd
 * indices: 1, 3, 5...) has its high bit set for flashing display effect.
 *
 * @param operand String operand (single or double quoted)
 * @param section Section to add DataAtom to
 * @param current_address Current address (advanced by string length)
 *
 * @par Example
 * @code
 * HandleFlsDirective("'HELLO'", section, address);
 * // section contains [0x48, 0xC5, 0x4C, 0xCC, 0x4F]
 * // Odd-indexed chars (E, second L) have high bit set
 * @endcode
 */
void HandleFlsDirective(const std::string &operand, Section &section,
                        uint32_t &current_address);

/**
 * @brief Handle DA directive - Define address (same as DW)
 *
 * Merlin-specific alias for DW (Define Word). Creates word-sized data.
 *
 * @param operand Comma-separated word expressions
 * @param section Section to add DataAtom to
 * @param symbols Symbol table for expression evaluation
 * @param current_address Current address (advanced by number of words * 2)
 *
 * @par Example
 * @code
 * HandleDaDirective("$1000,$2000", section, symbols, address);
 * // Same as DW - creates 2 words
 * @endcode
 */
void HandleDaDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address);

/**
 * @brief Handle END directive - mark end of source
 *
 * Sets a flag to stop processing further lines. No atoms created.
 *
 * @param end_directive_seen Flag to set (indicates END was encountered)
 *
 * @par Example
 * @code
 * bool end_seen = false;
 * HandleEndDirective(end_seen);
 * // end_seen is now true
 * @endcode
 */
void HandleEndDirective(bool &end_directive_seen);

/**
 * @brief Handle SAV directive - save output filename (no-op)
 *
 * In Merlin, SAV specifies the output filename. In xasm++, output is
 * controlled by command-line args, so this is a no-op for compatibility.
 *
 * @param operand Filename operand (ignored)
 *
 * @par Example
 * @code
 * HandleSavDirective("boot");
 * // Does nothing - output filename controlled by CLI
 * @endcode
 */
void HandleSavDirective(const std::string &operand);

/**
 * @brief Handle XC directive - toggle 65C02 CPU mode
 *
 * Sets the CPU mode to 65C02 (if no operand or "ON") or back to 6502 (if "OFF").
 *
 * @param operand "ON", "OFF", or empty (defaults to ON)
 * @param cpu CPU instance to configure (nullptr ignored for tests)
 *
 * @throws std::runtime_error if operand is invalid
 *
 * @par Example
 * @code
 * HandleXcDirective("", cpu); // Enable 65C02
 * HandleXcDirective("ON", cpu); // Enable 65C02
 * HandleXcDirective("OFF", cpu); // Disable (back to 6502)
 * @endcode
 */
void HandleXcDirective(const std::string &operand, class Cpu6502 *cpu);

/**
 * @brief Handle MX directive - set 65816 register widths (state tracking)
 *
 * Validates the MX mode operand (binary %00-%11 or decimal 0-3).
 * This is a state-tracking directive only - actual 65816 encoding is
 * out of scope.
 *
 * @param operand Mode operand: %00-%11 or 0-3
 *
 * @throws std::runtime_error if operand is invalid
 *
 * @par Example
 * @code
 * HandleMxDirective("%11"); // Valid - 8-bit A and X/Y
 * HandleMxDirective("3");   // Valid - same as %11
 * @endcode
 */
void HandleMxDirective(const std::string &operand);

/**
 * @brief Handle REV directive - reverse ASCII string
 *
 * Creates a label and emits a reversed string as data bytes.
 *
 * @param label Label to create at string start (required)
 * @param operand String operand (single or double quoted)
 * @param section Section to add label and data atoms to
 * @param symbols Symbol table to define label in
 * @param current_address Current address (advanced by string length)
 *
 * @throws std::runtime_error if label is empty or string is invalid
 *
 * @par Example
 * @code
 * HandleRevDirective("STR", "'HELLO'", section, symbols, address);
 * // Creates label STR at current address
 * // Emits bytes [0x4F, 0x4C, 0x4C, 0x45, 0x48] ("OLLEH")
 * @endcode
 */
void HandleRevDirective(const std::string &label, const std::string &operand,
                        Section &section, ConcreteSymbolTable &symbols,
                        uint32_t &current_address);

/**
 * @brief Handle DUM directive - start dummy section
 *
 * Begins a dummy (variable definition) block. Labels defined in DUM blocks
 * don't generate output bytes, only define addresses.
 *
 * @param operand Address operand (number or symbol)
 * @param symbols Symbol table for resolving symbol references
 * @param in_dum_block Flag to set (indicates DUM block active)
 * @param dum_address Address counter for DUM block
 *
 * @throws std::runtime_error if operand is empty or invalid
 *
 * @par Example
 * @code
 * bool in_dum = false;
 * uint32_t dum_addr = 0;
 * HandleDumDirective("$200", symbols, in_dum, dum_addr);
 * // in_dum is true, dum_addr is 0x200
 * @endcode
 */
void HandleDumDirective(const std::string &operand, ConcreteSymbolTable &symbols,
                        bool &in_dum_block, uint32_t &dum_address,
                        const DirectiveContext *ctx = nullptr);

/**
 * @brief Handle DEND directive - end dummy section
 *
 * Ends a dummy (variable definition) block started with DUM.
 *
 * @param in_dum_block Flag to clear (indicates DUM block no longer active)
 *
 * @par Example
 * @code
 * bool in_dum = true;
 * HandleDendDirective(in_dum);
 * // in_dum is now false
 * @endcode
 */
void HandleDendDirective(bool &in_dum_block);

/**
 * @brief Handle LST directive - listing control (no-op)
 *
 * Controls assembler listing output. This is a no-op for compatibility.
 *
 * @param operand Listing control operand (ignored)
 */
void HandleLstDirective(const std::string &operand);

/**
 * @brief Handle LSTDO directive - list during DO blocks (no-op)
 *
 * Controls listing output during conditional assembly. No-op for compatibility.
 */
void HandleLstdoDirective();

/**
 * @brief Handle TR directive - truncate listing (no-op)
 *
 * Controls listing truncation. This is a no-op for compatibility.
 *
 * @param operand Truncation control operand (ignored)
 */
void HandleTrDirective(const std::string &operand);

/**
 * @brief Handle USR directive - user-defined subroutine (no-op)
 *
 * No-op directive for user-defined subroutines. Doesn't generate atoms.
 */
void HandleUsrDirective();

/**
 * @brief Handle LUP directive - loop assembly (not implemented)
 *
 * Loop directive for repeating code. Currently not implemented (deferred).
 *
 * @param operand Loop count operand
 *
 * @throws std::runtime_error Always (not yet implemented)
 */
void HandleLupDirective(const std::string &operand);

// ============================================================================
// DirectiveRegistry Integration
// ============================================================================

/**
 * @brief Register Merlin directive handlers with DirectiveRegistry
 *
 * Registers wrapper functions for all Merlin-specific directives with the
 * provided registry. Wrappers convert the DirectiveHandler signature
 * (label, operand, context) to the direct handler signatures.
 *
 * **Registered Directives:**
 * - HEX: Define hex bytes
 * - ASC, DCI, INV, FLS: String directives
 * - DA: Define address (word)
 * - END: End of source
 * - SAV: Save output filename (no-op)
 * - XC: Toggle 65C02 mode
 * - MX: Set 65816 register widths
 * - REV: Reverse ASCII string
 * - DUM/DEND: Dummy section
 * - LST/LSTDO/TR: Listing control (no-ops)
 * - USR: User-defined subroutine (no-op)
 * - LUP: Loop assembly (not implemented)
 *
 * @param registry DirectiveRegistry to register handlers with
 * @param parser MerlinSyntaxParser for accessing parser-specific state
 *
 * @par Example Usage
 * @code
 * DirectiveRegistry registry;
 * MerlinSyntaxParser parser;
 * RegisterMerlinDirectiveHandlers(registry, parser);
 *
 * DirectiveContext context;
 * context.section = &section;
 * context.symbols = &symbols;
 * context.current_address = &address;
 * context.parser_state = &parser;
 *
 * // Now Merlin directives can be executed through the registry
 * registry.Execute("HEX", "", "0102030405", context);
 * registry.Execute("ASC", "MSG", "'HELLO'", context);
 * @endcode
 */
void RegisterMerlinDirectiveHandlers(DirectiveRegistry &registry,
                                     MerlinSyntaxParser *parser);

} // namespace xasm
