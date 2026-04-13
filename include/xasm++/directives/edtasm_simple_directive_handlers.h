/**
 * @file edtasm_simple_directive_handlers.h
 * @brief EDTASM+ simple directive handler function declarations
 *
 * This file contains forward declarations for EDTASM+ (simple/CoCo) directive
 * handlers. These are free functions in the xasm::edtasm namespace that follow
 * the common DirectiveContext pattern for consistency across syntax parsers.
 *
 * @note Pattern matches Merlin directive handlers for architectural consistency
 * @note This is for the simple EDTASM parser (edtasm_syntax.cpp), not the
 *       extended EDTASM-M80++ parser (edtasm_m80_plusplus_syntax.cpp)
 */

#pragma once

#include <string>

namespace xasm {

// Forward declaration
struct DirectiveContext;

namespace edtasm {

/**
 * @brief Handle ORG directive - Set origin address
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (address expression)
 * @param context Directive execution context
 */
void HandleOrg(DirectiveContext &context);

/**
 * @brief Handle END directive - End assembly
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (optional entry point)
 * @param context Directive execution context
 */
void HandleEnd(DirectiveContext &context);

/**
 * @brief Handle EQU directive - Define constant symbol
 *
 * @param label Label on directive line (required)
 * @param operand Operand field (value expression)
 * @param context Directive execution context
 */
void HandleEqu(DirectiveContext &context);

/**
 * @brief Handle SET directive - Define variable symbol (redefinable)
 *
 * @param label Label on directive line (required)
 * @param operand Operand field (value expression)
 * @param context Directive execution context
 */
void HandleSet(DirectiveContext &context);

/**
 * @brief Handle FCB directive - Form Constant Byte
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (comma-separated byte values)
 * @param context Directive execution context
 */
void HandleFcb(DirectiveContext &context);

/**
 * @brief Handle FDB directive - Form Double Byte (16-bit, big-endian)
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (comma-separated word values)
 * @param context Directive execution context
 */
void HandleFdb(DirectiveContext &context);

/**
 * @brief Handle FCC directive - Form Constant Characters (flexible delimiter)
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (delimited string)
 * @param context Directive execution context
 */
void HandleFcc(DirectiveContext &context);

/**
 * @brief Handle RMB directive - Reserve Memory Bytes
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (number of bytes to reserve)
 * @param context Directive execution context
 */
void HandleRmb(DirectiveContext &context);

/**
 * @brief Handle SETDP directive - Set Direct Page (assembler directive only)
 *
 * @param label Label on directive line (if any)
 * @param operand Operand field (direct page value)
 * @param context Directive execution context
 */
void HandleSetdp(DirectiveContext &context);

} // namespace edtasm
} // namespace xasm
