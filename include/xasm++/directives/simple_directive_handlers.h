/**
 * @file simple_directive_handlers.h
 * @brief Simple syntax directive handler functions
 *
 * Free function handlers for Simple syntax directives.
 * Refactored from lambda-based registration to follow Merlin pattern.
 */

#ifndef XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H

#include <string>

#include "xasm++/syntax/directive_registry.h"

namespace xasm::simple {

/**
 * @brief Handle ORG directive (set origin address)
 * @param context Directive execution context (operand contains address expression)
 */
void HandleOrg(DirectiveContext& context);

/**
 * @brief Handle DB directive (define byte data)
 * @param context Directive execution context (operand contains comma-separated byte values)
 */
void HandleDb(DirectiveContext& context);

/**
 * @brief Handle DW directive (define word data, little-endian)
 * @param context Directive execution context (operand contains comma-separated word values)
 */
void HandleDw(DirectiveContext& context);

}  // namespace xasm::simple

#endif  // XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H
