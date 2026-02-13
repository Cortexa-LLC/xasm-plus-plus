/**
 * @file scmasm_directive_handlers.h
 * @brief SCMASM directive handler functions
 *
 * Free function handlers for SCMASM directives.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#ifndef XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H

#include "xasm++/syntax/directive_registry.h"
#include <string>

namespace xasm {
namespace scmasm {

/**
 * @brief Handle .OR directive (set origin address)
 * @param label Label (unused for .OR)
 * @param operand Address expression
 * @param context Directive execution context
 */
void HandleOr(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .EQ directive (define constant)
 * @param label Symbol name (required)
 * @param operand Value expression
 * @param context Directive execution context
 */
void HandleEq(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .SE directive (define/redefine variable)
 * @param label Symbol name (required)
 * @param operand Value expression
 * @param context Directive execution context
 */
void HandleSe(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .AS directive (ASCII string)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAs(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .AT directive (ASCII text, high bit on last char)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAt(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .AZ directive (ASCII zero-terminated)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAz(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .DA / .DFB directive (define address/bytes)
 * @param label Label (unused)
 * @param operand Comma-separated values with size prefixes
 * @param context Directive execution context
 */
void HandleDa(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .HS directive (hex string)
 * @param label Label (unused)
 * @param operand Hex digits (whitespace allowed)
 * @param context Directive execution context
 */
void HandleHs(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .BS directive (binary string)
 * @param label Label (unused)
 * @param operand Binary digits (whitespace allowed)
 * @param context Directive execution context
 */
void HandleBs(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .MA directive (begin macro definition)
 * @param label Macro name (if not in operand)
 * @param operand Macro name (if not in label)
 * @param context Directive execution context
 */
void HandleMa(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .ENDM / .EM directive (end macro definition)
 * @param label Label (unused)
 * @param operand Operand (unused)
 * @param context Directive execution context
 */
void HandleEndm(const std::string &label, const std::string &operand,
                DirectiveContext &context);

// ============================================================================
// P0 Priority Directive Handlers (A2oSX Critical)
// ============================================================================

/**
 * @brief Handle .PS directive (Pascal string - length-prefixed)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandlePs(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle .INB directive (include binary file)
 * @param label Label (unused)
 * @param operand File path
 * @param context Directive execution context
 */
void HandleInb(const std::string &label, const std::string &operand,
               DirectiveContext &context);

/**
 * @brief Handle .LIST directive (listing control - stub)
 * @param label Label (unused)
 * @param operand Control parameter (ON/OFF)
 * @param context Directive execution context
 */
void HandleList(const std::string &label, const std::string &operand,
                DirectiveContext &context);

/**
 * @brief Handle .DUMMY directive (dummy section for structures)
 * @param label Label (unused)
 * @param operand Optional operand
 * @param context Directive execution context
 */
void HandleDummy(const std::string &label, const std::string &operand,
                 DirectiveContext &context);

/**
 * @brief Handle .OP directive (CPU operation mode)
 * @param label Label (unused)
 * @param operand CPU type (6502, 65C02, 65816)
 * @param context Directive execution context
 */
void HandleOp(const std::string &label, const std::string &operand,
              DirectiveContext &context);

} // namespace scmasm
} // namespace xasm

#endif // XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H
