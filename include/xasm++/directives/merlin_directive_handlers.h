/**
 * @file merlin_directive_handlers.h
 * @brief Merlin directive handler functions
 *
 * Free function handlers for Merlin directives.
 * Part of Phase 6c - Directive handler extraction refactoring.
 */

#ifndef XASMPP_DIRECTIVES_MERLIN_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_MERLIN_DIRECTIVE_HANDLERS_H

#include <string>

#include "xasm++/syntax/directive_registry.h"

namespace xasm::merlin {

/**
 * @brief Handle ORG directive (set origin address)
 * @param label Label (unused for ORG)
 * @param operand Address expression
 * @param context Directive execution context
 */
void HandleOrg(DirectiveContext& context);

/**
 * @brief Handle EQU directive (define constant)
 * @param label Symbol name (required)
 * @param operand Value expression
 * @param context Directive execution context
 */
void HandleEqu(DirectiveContext& context);

/**
 * @brief Handle DB/DFB directive (define byte data)
 * @param label Label (optional)
 * @param operand Byte values
 * @param context Directive execution context
 */
void HandleDb(DirectiveContext& context);

/**
 * @brief Handle DW directive (define word data)
 * @param label Label (optional)
 * @param operand Word values
 * @param context Directive execution context
 */
void HandleDw(DirectiveContext& context);

/**
 * @brief Handle HEX directive (define hex bytes)
 * @param label Label (optional)
 * @param operand Hex digit string
 * @param context Directive execution context
 */
void HandleHex(DirectiveContext& context);

/**
 * @brief Handle DS directive (define space)
 * @param label Label (optional)
 * @param operand Space size
 * @param context Directive execution context
 */
void HandleDs(DirectiveContext& context);

/**
 * @brief Handle DUM directive (start dummy section)
 * @param label Label (unused)
 * @param operand Address expression
 * @param context Directive execution context
 */
void HandleDum(DirectiveContext& context);

/**
 * @brief Handle DEND directive (end dummy section)
 * @param label Label (unused)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleDend(DirectiveContext& context);

/**
 * @brief Handle PUT directive (include file)
 * @param label Label (unused)
 * @param operand Filename
 * @param context Directive execution context
 */
void HandlePut(DirectiveContext& context);

/**
 * @brief Handle DO directive (conditional assembly)
 * @param label Label (unused)
 * @param operand Condition expression
 * @param context Directive execution context
 */
void HandleDo(DirectiveContext& context);

/**
 * @brief Handle ELSE directive (conditional assembly)
 * @param label Label (unused)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleElse(DirectiveContext& context);

/**
 * @brief Handle FIN directive (end conditional)
 * @param label Label (unused)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleFin(DirectiveContext& context);

/**
 * @brief Handle LST directive (listing control)
 * @param label Label (unused)
 * @param operand Listing option
 * @param context Directive execution context
 */
void HandleLst(DirectiveContext& context);

/**
 * @brief Handle LSTDO directive (listing control)
 * @param label Label (unused)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleLstdo(DirectiveContext& context);

/**
 * @brief Handle TR directive (truncate listing)
 * @param label Label (unused)
 * @param operand Option
 * @param context Directive execution context
 */
void HandleTr(DirectiveContext& context);

/**
 * @brief Handle ASC directive (ASCII string)
 * @param label Label (optional)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleAsc(DirectiveContext& context);

/**
 * @brief Handle DCI directive (DCI string)
 * @param label Label (optional)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleDci(DirectiveContext& context);

/**
 * @brief Handle INV directive (inverted ASCII string)
 * @param label Label (optional)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleInv(DirectiveContext& context);

/**
 * @brief Handle FLS directive (flashing string)
 * @param label Label (optional)
 * @param operand String with delimiter
 * @param context Directive execution context
 */
void HandleFls(DirectiveContext& context);

/**
 * @brief Handle DA directive (define address)
 * @param label Label (optional)
 * @param operand Address values
 * @param context Directive execution context
 */
void HandleDa(DirectiveContext& context);

/**
 * @brief Handle PMC directive (macro definition start)
 * @param label Label (macro name)
 * @param operand Alternate macro name
 * @param context Directive execution context
 */
void HandlePmc(DirectiveContext& context);

/**
 * @brief Handle EOM directive (macro definition end)
 * @param label Label (unused)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleEom(DirectiveContext& context);

/**
 * @brief Handle MAC directive (macro definition/invocation)
 * @param label Label (macro name if defining)
 * @param operand Macro name or parameters
 * @param context Directive execution context
 */
void HandleMac(DirectiveContext& context);

/**
 * @brief Handle USR directive (user-defined subroutine)
 * @param label Label (optional)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleUsr(DirectiveContext& context);

/**
 * @brief Handle END directive (end of source)
 * @param label Label (optional)
 * @param operand Unused
 * @param context Directive execution context
 */
void HandleEnd(DirectiveContext& context);

/**
 * @brief Handle SAV directive (save output filename)
 * @param label Label (unused)
 * @param operand Filename
 * @param context Directive execution context
 */
void HandleSav(DirectiveContext& context);

/**
 * @brief Handle XC directive (toggle 65C02 mode)
 * @param label Label (unused)
 * @param operand Mode option
 * @param context Directive execution context
 */
void HandleXc(DirectiveContext& context);

/**
 * @brief Handle MX directive (set 65816 register widths)
 * @param label Label (unused)
 * @param operand Register width flags
 * @param context Directive execution context
 */
void HandleMx(DirectiveContext& context);

/**
 * @brief Handle REV directive (reverse ASCII string)
 * @param label Label (required)
 * @param operand String value
 * @param context Directive execution context
 */
void HandleRev(DirectiveContext& context);

/**
 * @brief Handle LUP directive (loop assembly)
 * @param label Label (unused)
 * @param operand Loop count
 * @param context Directive execution context
 */
void HandleLup(DirectiveContext& context);

}  // namespace xasm::merlin

#endif  // XASMPP_DIRECTIVES_MERLIN_DIRECTIVE_HANDLERS_H
