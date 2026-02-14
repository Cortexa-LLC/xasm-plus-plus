/**
 * @file edtasm_directive_handlers.h
 * @brief EDTASM-M80++ directive handler declarations
 *
 * Free function declarations for EDTASM-M80++ specific directives.
 * Part of God-Class Phase 6c.3 refactoring.
 */

#pragma once

#include <cstdint>
#include <string>

namespace xasm {

// Forward declarations
class DirectiveRegistry;
struct DirectiveContext;
class Section;
class ConcreteSymbolTable;

// ============================================================================
// Segment Control Handlers
// ============================================================================

/**
 * @brief Handle ASEG directive - Switch to absolute segment
 */
void HandleAsegDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle CSEG directive - Switch to code segment
 */
void HandleCsegDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle DSEG directive - Switch to data segment
 */
void HandleDsegDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

// ============================================================================
// Conditional Assembly Handlers
// ============================================================================

/**
 * @brief Handle IF directive - Start conditional block
 */
void HandleIfDirective(const std::string &label, const std::string &operand,
                       DirectiveContext &ctx);

/**
 * @brief Handle ELSE directive - Else clause of conditional
 */
void HandleElseDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle ENDIF directive - End conditional block
 */
void HandleEndifDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle IFDEF directive - If symbol defined
 */
void HandleIfdefDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle IFNDEF directive - If symbol not defined
 */
void HandleIfndefDirective(const std::string &label, const std::string &operand,
                           DirectiveContext &ctx);

/**
 * @brief Handle IFEQ directive - If equal
 */
void HandleIfeqDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFNE directive - If not equal
 */
void HandleIfneDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFLT directive - If less than
 */
void HandleIfltDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFGT directive - If greater than
 */
void HandleIfgtDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFLE directive - If less than or equal
 */
void HandleIfleDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFGE directive - If greater than or equal
 */
void HandleIfgeDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IF1 directive - If first pass
 */
void HandleIf1Directive(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle IF2 directive - If second pass
 */
void HandleIf2Directive(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle IFB directive - If blank
 */
void HandleIfbDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle IFNB directive - If not blank
 */
void HandleIfnbDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IFIDN directive - If identical
 */
void HandleIfidnDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle IFDIF directive - If different
 */
void HandleIfdifDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

// ============================================================================
// Data and Symbol Directives
// ============================================================================

/**
 * @brief Handle ORG directive - Set origin address
 */
void HandleOrgDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle EQU directive - Define constant symbol
 */
void HandleEquDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle SET/=/DEFL directive - Define redefinable symbol
 */
void HandleSetDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle DB/DEFB/BYTE/DM/DEFM/TEXT/ASCII directive - Define byte(s)
 */
void HandleDbDirective(const std::string &label, const std::string &operand,
                       DirectiveContext &ctx);

/**
 * @brief Handle DW/DEFW/WORD directive - Define word(s)
 */
void HandleDwDirective(const std::string &label, const std::string &operand,
                       DirectiveContext &ctx);

/**
 * @brief Handle DS/DEFS/DSPACE/BLOCK directive - Reserve space
 */
void HandleDsDirective(const std::string &label, const std::string &operand,
                       DirectiveContext &ctx);

/**
 * @brief Handle PUBLIC/GLOBAL/ENTRY directive - Export symbols
 */
void HandlePublicDirective(const std::string &label, const std::string &operand,
                           DirectiveContext &ctx);

/**
 * @brief Handle EXTERN/EXTRN/EXT directive - Import symbols
 */
void HandleExternDirective(const std::string &label, const std::string &operand,
                           DirectiveContext &ctx);

// ============================================================================
// Macro System Handlers
// ============================================================================

/**
 * @brief Handle MACRO directive - Start macro definition
 */
void HandleMacroDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle ENDM directive - End macro/repeat block
 */
void HandleEndmDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle LOCAL directive - Define local symbol in macro
 */
void HandleLocalDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle EXITM directive - Exit macro early
 */
void HandleExitmDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle REPT directive - Repeat block
 */
void HandleReptDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle IRP directive - Iterate over parameters
 */
void HandleIrpDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

/**
 * @brief Handle IRPC directive - Iterate over characters
 */
void HandleIrpcDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

// ============================================================================
// Listing Control Handlers
// ============================================================================

/**
 * @brief Handle LIST directive - Enable listing
 */
void HandleListDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle NOLIST/XLIST directive - Disable listing
 */
void HandleNolistDirective(const std::string &label, const std::string &operand,
                           DirectiveContext &ctx);

/**
 * @brief Handle *LIST directive - Toggle listing (Z80ASM style)
 */
void HandleStarListDirective(const std::string &label,
                             const std::string &operand, DirectiveContext &ctx);

/**
 * @brief Handle LALL directive - List all (macro expansion)
 */
void HandleLallDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle SALL directive - Suppress all (macro expansion)
 */
void HandleSallDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle TITLE directive - Set listing title
 */
void HandleTitleDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle SUBTTL directive - Set subtitle
 */
void HandleSubttlDirective(const std::string &label, const std::string &operand,
                           DirectiveContext &ctx);

/**
 * @brief Handle EJECT directive - Page eject
 */
void HandleEjectDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle SPACE directive - Add blank lines
 */
void HandleSpaceDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

/**
 * @brief Handle NAME directive - Set module name
 */
void HandleNameDirective(const std::string &label, const std::string &operand,
                         DirectiveContext &ctx);

/**
 * @brief Handle RADIX directive - Set number radix
 */
void HandleRadixDirective(const std::string &label, const std::string &operand,
                          DirectiveContext &ctx);

// ============================================================================
// Miscellaneous Handlers
// ============================================================================

/**
 * @brief Handle END directive - End of source
 */
void HandleEndDirective(const std::string &label, const std::string &operand,
                        DirectiveContext &ctx);

// ============================================================================
// Registration Function
// ============================================================================

/**
 * @brief Register all EDTASM-M80++ directive handlers
 * @param registry The directive registry to register with
 */
void RegisterEdtasmDirectiveHandlers(DirectiveRegistry &registry);

} // namespace xasm
