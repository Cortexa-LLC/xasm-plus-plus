/**
 * @file simple_directive_handlers.h
 * @brief Simple syntax directive handler functions
 *
 * Free function handlers for Simple syntax directives.
 * Refactored from lambda-based registration to follow Merlin pattern.
 */

#ifndef XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H
#define XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H

#include "xasm++/syntax/directive_registry.h"
#include <string>

namespace xasm {
namespace simple {

/**
 * @brief Handle ORG directive (set origin address)
 * @param label Label (unused for ORG)
 * @param operand Address expression (hex format: $XXXX or XXXX)
 * @param context Directive execution context
 */
void HandleOrg(const std::string &label, const std::string &operand,
               DirectiveContext &context);

/**
 * @brief Handle DB directive (define byte data)
 * @param label Label (unused)
 * @param operand Comma-separated byte values (hex: $XX or decimal)
 * @param context Directive execution context
 */
void HandleDb(const std::string &label, const std::string &operand,
              DirectiveContext &context);

/**
 * @brief Handle DW directive (define word data, little-endian)
 * @param label Label (unused)
 * @param operand Comma-separated word values (hex: $XXXX or decimal)
 * @param context Directive execution context
 */
void HandleDw(const std::string &label, const std::string &operand,
              DirectiveContext &context);

} // namespace simple
} // namespace xasm

#endif // XASMPP_DIRECTIVES_SIMPLE_DIRECTIVE_HANDLERS_H
