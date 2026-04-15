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
void HandleAsegDirective(DirectiveContext& ctx);

/**
 * @brief Handle CSEG directive - Switch to code segment
 */
void HandleCsegDirective(DirectiveContext& ctx);

/**
 * @brief Handle DSEG directive - Switch to data segment
 */
void HandleDsegDirective(DirectiveContext& ctx);

// ============================================================================
// Conditional Assembly Handlers
// ============================================================================

/**
 * @brief Handle IF directive - Start conditional block
 */
void HandleIfDirective(DirectiveContext& ctx);

/**
 * @brief Handle ELSE directive - Else clause of conditional
 */
void HandleElseDirective(DirectiveContext& ctx);

/**
 * @brief Handle ENDIF directive - End conditional block
 */
void HandleEndifDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFDEF directive - If symbol defined
 */
void HandleIfdefDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFNDEF directive - If symbol not defined
 */
void HandleIfndefDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFEQ directive - If equal
 */
void HandleIfeqDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFNE directive - If not equal
 */
void HandleIfneDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFLT directive - If less than
 */
void HandleIfltDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFGT directive - If greater than
 */
void HandleIfgtDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFLE directive - If less than or equal
 */
void HandleIfleDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFGE directive - If greater than or equal
 */
void HandleIfgeDirective(DirectiveContext& ctx);

/**
 * @brief Handle IF1 directive - If first pass
 */
void HandleIf1Directive(DirectiveContext& ctx);

/**
 * @brief Handle IF2 directive - If second pass
 */
void HandleIf2Directive(DirectiveContext& ctx);

/**
 * @brief Handle IFB directive - If blank
 */
void HandleIfbDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFNB directive - If not blank
 */
void HandleIfnbDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFIDN directive - If identical
 */
void HandleIfidnDirective(DirectiveContext& ctx);

/**
 * @brief Handle IFDIF directive - If different
 */
void HandleIfdifDirective(DirectiveContext& ctx);

// ============================================================================
// Data and Symbol Directives
// ============================================================================

/**
 * @brief Handle ORG directive - Set origin address
 */
void HandleOrgDirective(DirectiveContext& ctx);

/**
 * @brief Handle EQU directive - Define constant symbol
 */
void HandleEquDirective(DirectiveContext& ctx);

/**
 * @brief Handle SET/=/DEFL directive - Define redefinable symbol
 */
void HandleSetDirective(DirectiveContext& ctx);

/**
 * @brief Handle DB/DEFB/BYTE/DM/DEFM/TEXT/ASCII directive - Define byte(s)
 */
void HandleDbDirective(DirectiveContext& ctx);

/**
 * @brief Handle DW/DEFW/WORD directive - Define word(s)
 */
void HandleDwDirective(DirectiveContext& ctx);

/**
 * @brief Handle DS/DEFS/DSPACE/BLOCK directive - Reserve space
 */
void HandleDsDirective(DirectiveContext& ctx);

/**
 * @brief Handle PUBLIC/GLOBAL/ENTRY directive - Export symbols
 */
void HandlePublicDirective(DirectiveContext& ctx);

/**
 * @brief Handle EXTERN/EXTRN/EXT directive - Import symbols
 */
void HandleExternDirective(DirectiveContext& ctx);

// ============================================================================
// Macro System Handlers
// ============================================================================

/**
 * @brief Handle MACRO directive - Start macro definition
 */
void HandleMacroDirective(DirectiveContext& ctx);

/**
 * @brief Handle ENDM directive - End macro/repeat block
 */
void HandleEndmDirective(DirectiveContext& ctx);

/**
 * @brief Handle LOCAL directive - Define local symbol in macro
 */
void HandleLocalDirective(DirectiveContext& ctx);

/**
 * @brief Handle EXITM directive - Exit macro early
 */
void HandleExitmDirective(DirectiveContext& ctx);

/**
 * @brief Handle REPT directive - Repeat block
 */
void HandleReptDirective(DirectiveContext& ctx);

/**
 * @brief Handle IRP directive - Iterate over parameters
 */
void HandleIrpDirective(DirectiveContext& ctx);

/**
 * @brief Handle IRPC directive - Iterate over characters
 */
void HandleIrpcDirective(DirectiveContext& ctx);

// ============================================================================
// Listing Control Handlers
// ============================================================================

/**
 * @brief Handle LIST directive - Enable listing
 */
void HandleListDirective(DirectiveContext& ctx);

/**
 * @brief Handle NOLIST/XLIST directive - Disable listing
 */
void HandleNolistDirective(DirectiveContext& ctx);

/**
 * @brief Handle *LIST directive - Toggle listing (Z80ASM style)
 */
void HandleStarListDirective(DirectiveContext& ctx);

/**
 * @brief Handle LALL directive - List all (macro expansion)
 */
void HandleLallDirective(DirectiveContext& ctx);

/**
 * @brief Handle SALL directive - Suppress all (macro expansion)
 */
void HandleSallDirective(DirectiveContext& ctx);

/**
 * @brief Handle TITLE directive - Set listing title
 */
void HandleTitleDirective(DirectiveContext& ctx);

/**
 * @brief Handle SUBTTL directive - Set subtitle
 */
void HandleSubttlDirective(DirectiveContext& ctx);

/**
 * @brief Handle EJECT directive - Page eject
 */
void HandleEjectDirective(DirectiveContext& ctx);

/**
 * @brief Handle SPACE directive - Add blank lines
 */
void HandleSpaceDirective(DirectiveContext& ctx);

/**
 * @brief Handle NAME directive - Set module name
 */
void HandleNameDirective(DirectiveContext& ctx);

/**
 * @brief Handle RADIX directive - Set number radix
 */
void HandleRadixDirective(DirectiveContext& ctx);

// ============================================================================
// Miscellaneous Handlers
// ============================================================================

/**
 * @brief Handle END directive - End of source
 */
void HandleEndDirective(DirectiveContext& ctx);

// ============================================================================
// Registration Function
// ============================================================================

/**
 * @brief Register all EDTASM-M80++ directive handlers
 * @param registry The directive registry to register with
 */
void RegisterEdtasmDirectiveHandlers(DirectiveRegistry& registry);

}  // namespace xasm
