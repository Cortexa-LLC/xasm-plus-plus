/**
 * @file scmasm_directive_handlers.h
 * @brief SCMASM directive handler functions
 *
 * Free function handlers for SCMASM directives.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#ifndef XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H

#include <string>

#include "xasm++/syntax/directive_registry.h"

namespace xasm::scmasm {

/**
 * @brief Handle .OR directive (set origin address)
 * @param label Label (unused for .OR)
 * @param operand Address expression
 * @param context Directive execution context
 */
void HandleOr(DirectiveContext& context);

/**
 * @brief Handle .EQ directive (define constant)
 * @param label Symbol name (required)
 * @param operand Value expression
 * @param context Directive execution context
 */
void HandleEq(DirectiveContext& context);

/**
 * @brief Handle .SE directive (define/redefine variable)
 * @param label Symbol name (required)
 * @param operand Value expression
 * @param context Directive execution context
 */
void HandleSe(DirectiveContext& context);

/**
 * @brief Handle .AS directive (ASCII string)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAs(DirectiveContext& context);

/**
 * @brief Handle .AT directive (ASCII text, high bit on last char)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAt(DirectiveContext& context);

/**
 * @brief Handle .AZ directive (ASCII zero-terminated)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAz(DirectiveContext& context);

/**
 * @brief Handle .DA / .DFB directive (define address/bytes)
 * @param label Label (unused)
 * @param operand Comma-separated values with size prefixes
 * @param context Directive execution context
 */
void HandleDa(DirectiveContext& context);

/**
 * @brief Handle .HS directive (hex string)
 * @param label Label (unused)
 * @param operand Hex digits (whitespace allowed)
 * @param context Directive execution context
 */
void HandleHs(DirectiveContext& context);

/**
 * @brief Handle .BS directive (binary string)
 * @param label Label (unused)
 * @param operand Binary digits (whitespace allowed)
 * @param context Directive execution context
 */
void HandleBs(DirectiveContext& context);

/**
 * @brief Handle .MA directive (begin macro definition)
 * @param label Macro name (if not in operand)
 * @param operand Macro name (if not in label)
 * @param context Directive execution context
 */
void HandleMa(DirectiveContext& context);

/**
 * @brief Handle .ENDM / .EM directive (end macro definition)
 * @param label Label (unused)
 * @param operand Operand (unused)
 * @param context Directive execution context
 */
void HandleEndm(DirectiveContext& context);

// ============================================================================
// P0 Priority Directive Handlers (A2oSX Critical)
// ============================================================================

/**
 * @brief Handle .PS directive (Pascal string - length-prefixed)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandlePs(DirectiveContext& context);

/**
 * @brief Handle .INB directive (include binary file)
 * @param label Label (unused)
 * @param operand File path
 * @param context Directive execution context
 */
void HandleInb(DirectiveContext& context);

/**
 * @brief Handle .LIST directive (listing control - stub)
 * @param label Label (unused)
 * @param operand Control parameter (ON/OFF)
 * @param context Directive execution context
 */
void HandleList(DirectiveContext& context);

/**
 * @brief Handle .DUMMY directive (dummy section for structures)
 * @param label Label (unused)
 * @param operand Optional operand
 * @param context Directive execution context
 */
void HandleDummy(DirectiveContext& context);

/**
 * @brief Handle .ED directive (end dummy section)
 * @param label Label (unused)
 * @param operand Operand (unused)
 * @param context Directive execution context
 */
void HandleEd(DirectiveContext& context);

/**
 * @brief Handle .OP directive (CPU operation mode)
 * @param label Label (unused)
 * @param operand CPU type (6502, 65C02, 65816)
 * @param context Directive execution context
 */
void HandleOp(DirectiveContext& context);

// ============================================================================
// Phase 3: 100% Coverage Directives
// ============================================================================

/**
 * @brief Handle .CS directive (C-string with escape sequences)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleCs(DirectiveContext& context);

/**
 * @brief Handle .CZ directive (C-string zero-terminated)
 * @param label Label (unused)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleCz(DirectiveContext& context);

/**
 * @brief Handle .TF directive (text file/title metadata)
 * @param label Label (unused)
 * @param operand File path or title
 * @param context Directive execution context
 */
void HandleTf(DirectiveContext& context);

/**
 * @brief Handle .EP directive (entry point / end phase)
 * @param label Label (unused)
 * @param operand Entry address expression (optional for phase end)
 * @param context Directive execution context
 */
void HandleEp(DirectiveContext& context);

/**
 * @brief Handle .PH directive (phase assembly)
 * @param label Label (unused)
 * @param operand Virtual address expression
 * @param context Directive execution context
 */
void HandlePh(DirectiveContext& context);

/**
 * @brief Handle .HX directive (hex nibble storage)
 * @param label Label (unused)
 * @param operand Hex digits (whitespace/dots/commas allowed)
 * @param context Directive execution context
 */
void HandleHx(DirectiveContext& context);

/**
 * @brief Handle .TA directive (target address - no-op)
 * @param label Label (unused)
 * @param operand Address expression
 * @param context Directive execution context
 */
void HandleTa(DirectiveContext& context);

/**
 * @brief Handle .DO directive (conditional assembly start)
 * @param label Label (unused)
 * @param operand Condition expression
 * @param context Directive execution context
 */
void HandleDo(DirectiveContext& context);

/**
 * @brief Handle .FIN directive (conditional assembly end)
 * @param label Label (unused)
 * @param operand Operand (unused)
 * @param context Directive execution context
 */
void HandleFin(DirectiveContext& context);

/**
 * @brief Handle .AC directive (ASCII string with optional numeric prefix)
 * @param label Label (unused)
 * @param operand String with optional prefix
 * @param context Directive execution context
 */
void HandleAc(DirectiveContext& context);

}  // namespace xasm::scmasm

#endif  // XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_HANDLERS_H
