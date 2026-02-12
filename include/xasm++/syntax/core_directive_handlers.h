/**
 * @file core_directive_handlers.h
 * @brief Core directive handlers extracted from syntax parsers
 *
 * This file contains free functions for handling common assembly directives
 * (ORG, EQU, DB, DW, DS) that are shared across multiple syntax parsers.
 * Part of God-Class Phase 6b refactoring to extract common functionality.
 *
 * @section handlers Handler Functions
 *
 * Each handler function follows the pattern:
 * - Takes operand string, section, symbol table, and current address
 * - Parses operand (supports decimal, hex $XX, binary %XX, symbols)
 * - Creates appropriate atom and adds to section
 * - Updates current_address by reference
 * - Throws std::runtime_error on invalid input
 *
 * @section usage Usage Example
 * @code
 * Section section("code");
 * ConcreteSymbolTable symbols;
 * uint32_t address = 0x1000;
 *
 * // ORG directive
 * HandleOrgDirective("$C000", section, symbols, address);
 * // address is now 0xC000
 *
 * // EQU directive
 * HandleEquDirective("BUFFER", "256", symbols);
 * // symbols now contains BUFFER=256
 *
 * // DB directive
 * HandleDbDirective("1,2,3", section, symbols, address);
 * // section contains DataAtom with 3 bytes, address advanced by 3
 * @endcode
 */

#pragma once

#include "xasm++/section.h"
#include "xasm++/symbol.h"  // For ConcreteSymbolTable
#include <cstdint>
#include <string>

// Forward declaration for DirectiveRegistry
namespace xasm {
class DirectiveRegistry;
}

namespace xasm {

/**
 * @brief Handle ORG directive - set program counter address
 *
 * Creates an OrgAtom and updates the current address. The operand can be:
 * - Decimal number: "2048"
 * - Hex number: "$C000"
 * - Binary number: "%11111111"
 * - Symbol reference: "START"
 *
 * @param operand Address operand (number or symbol)
 * @param section Section to add OrgAtom to
 * @param symbols Symbol table for resolving symbol references
 * @param current_address Current address (updated to new origin)
 *
 * @throws std::runtime_error if operand is empty or invalid
 *
 * @par Example
 * @code
 * HandleOrgDirective("$C000", section, symbols, address);
 * // address is now 0xC000
 * // section contains OrgAtom(0xC000)
 * @endcode
 */
// Forward declare DirectiveContext
struct DirectiveContext;

void HandleOrgDirective(const std::string &operand, Section &section,
                        ConcreteSymbolTable &symbols, uint32_t &current_address,
                        const DirectiveContext *ctx = nullptr);

/**
 * @brief Handle EQU directive - define constant symbol
 *
 * Defines a symbol with a constant value. The operand is evaluated as an
 * expression and associated with the label. Does not create any atoms.
 *
 * @param label Symbol name to define
 * @param operand Value expression (number, symbol, or arithmetic)
 * @param symbols Symbol table to define symbol in
 *
 * @throws std::runtime_error if label is empty or expression is invalid
 *
 * @par Example
 * @code
 * HandleEquDirective("BUFFER_SIZE", "256", symbols);
 * // symbols now contains BUFFER_SIZE=256
 *
 * HandleEquDirective("IO_PORT", "$D000", symbols);
 * // symbols now contains IO_PORT=0xD000
 * @endcode
 */
void HandleEquDirective(const std::string &label, const std::string &operand,
                        ConcreteSymbolTable &symbols, const DirectiveContext *ctx = nullptr);

/**
 * @brief Handle DB directive - define byte data
 *
 * Creates a DataAtom with byte-sized data. The operand is a comma-separated
 * list of expressions that will be evaluated to bytes.
 *
 * @param operand Comma-separated byte expressions
 * @param section Section to add DataAtom to
 * @param symbols Symbol table for expression evaluation
 * @param current_address Current address (advanced by number of bytes)
 *
 * @par Example
 * @code
 * HandleDbDirective("1,2,3,$FF", section, symbols, address);
 * // section contains DataAtom with 4 byte expressions
 * // address advanced by 4
 * @endcode
 */
void HandleDbDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address);

/**
 * @brief Handle DW directive - define word data
 *
 * Creates a DataAtom with word-sized (16-bit) data. The operand is a
 * comma-separated list of expressions that will be evaluated to words.
 *
 * @param operand Comma-separated word expressions
 * @param section Section to add DataAtom to
 * @param symbols Symbol table for expression evaluation
 * @param current_address Current address (advanced by number of words * 2)
 *
 * @par Example
 * @code
 * HandleDwDirective("$1000,$2000", section, symbols, address);
 * // section contains DataAtom with 2 word expressions
 * // address advanced by 4 (2 words * 2 bytes)
 * @endcode
 */
void HandleDwDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address);

/**
 * @brief Handle DS directive - define space (reserve bytes)
 *
 * Creates a SpaceAtom to reserve uninitialized space. The operand specifies
 * the number of bytes to reserve and can be a number, symbol, or expression.
 *
 * @param operand Count expression (number, symbol, or arithmetic)
 * @param section Section to add SpaceAtom to
 * @param symbols Symbol table for expression evaluation
 * @param current_address Current address (advanced by count)
 *
 * @throws std::runtime_error if count is negative
 *
 * @par Example
 * @code
 * HandleDsDirective("100", section, symbols, address);
 * // section contains SpaceAtom(100)
 * // address advanced by 100
 *
 * HandleDsDirective("SIZE*2", section, symbols, address);
 * // Evaluates SIZE*2 and reserves that many bytes
 * @endcode
 */
void HandleDsDirective(const std::string &operand, Section &section,
                       ConcreteSymbolTable &symbols, uint32_t &current_address,
                       const DirectiveContext *ctx = nullptr);

// ============================================================================
// DirectiveRegistry Integration
// ============================================================================

/**
 * @brief Register core directive handlers with DirectiveRegistry
 *
 * Registers wrapper functions for all 5 core directives (ORG, EQU, DB, DW, DS)
 * with the provided registry. Wrappers convert the DirectiveHandler signature
 * (label, operand, context) to the direct handler signatures.
 *
 * **Registered Directives:**
 * - ORG: Set origin address
 * - EQU: Define constant symbol
 * - DB, DEFB, BYTE: Define byte data
 * - DW, DEFW, WORD: Define word data
 * - DS, DEFS, BLOCK, RMB: Define space / reserve bytes
 *
 * @param registry DirectiveRegistry to register handlers with
 *
 * @par Example Usage
 * @code
 * DirectiveRegistry registry;
 * RegisterCoreDirectiveHandlers(registry);
 *
 * DirectiveContext context;
 * context.section = &section;
 * context.symbols = &symbols;
 * context.current_address = &address;
 *
 * // Now directives can be executed through the registry
 * registry.Execute("ORG", "", "$C000", context);
 * registry.Execute("EQU", "CONSTANT", "42", context);
 * registry.Execute("DB", "", "1,2,3", context);
 * @endcode
 */
void RegisterCoreDirectiveHandlers(DirectiveRegistry &registry);

} // namespace xasm
