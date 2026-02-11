/**
 * @file common_directives.h
 * @brief Common assembler directive name constants
 *
 * Defines constants for directive names that are common across multiple
 * assembler syntaxes. These constants eliminate magic strings in directive
 * handlers and enable compile-time checking.
 *
 * **Usage Example:**
 * @code
 * #include "xasm++/directives/common_directives.h"
 *
 * using namespace xasm::CommonDirectives;
 *
 * directive_registry_.Register(ORG, [this](...) { ... });
 * directive_registry_.Register(EQU, [this](...) { ... });
 * @endcode
 */

#pragma once

namespace xasm {
namespace CommonDirectives {

// ============================================================================
// Origin and Address Control
// ============================================================================

/// Set assembly origin address (start address for code generation)
constexpr const char* ORG = "ORG";

// ============================================================================
// Symbol Definition
// ============================================================================

/// Define constant symbol (equate) - value cannot be redefined
constexpr const char* EQU = "EQU";

/// Define variable symbol - value can be redefined (Z80, some others)
constexpr const char* SET = "SET";

/// Define variable symbol - alternate name for SET
constexpr const char* DEFL = "DEFL";

/// Assignment operator (alternate syntax for EQU/SET in some assemblers)
constexpr const char* EQUALS = "=";

// ============================================================================
// Data Definition
// ============================================================================

/// Define byte(s) - most common data directive
constexpr const char* DB = "DB";

/// Define byte(s) - alternate name
constexpr const char* DEFB = "DEFB";

/// Define byte(s) - alternate name
constexpr const char* BYTE = "BYTE";

/// Define message (byte string) - alternate name for DB
constexpr const char* DM = "DM";

/// Define message (byte string) - alternate name for DB
constexpr const char* DEFM = "DEFM";

/// Define text (byte string) - alternate name for DB
constexpr const char* TEXT = "TEXT";

/// Define ASCII string (byte string) - alternate name for DB
constexpr const char* ASCII = "ASCII";

/// Define word(s) (16-bit values)
constexpr const char* DW = "DW";

/// Define word(s) - alternate name
constexpr const char* DEFW = "DEFW";

/// Define word(s) - alternate name
constexpr const char* WORD = "WORD";

/// Define space - reserve bytes without initialization
constexpr const char* DS = "DS";

/// Define space - alternate name
constexpr const char* DEFS = "DEFS";

/// Define space - alternate name
constexpr const char* DSPACE = "DSPACE";

/// Define space - alternate name (used by some assemblers)
constexpr const char* BLOCK = "BLOCK";

// ============================================================================
// Symbol Visibility
// ============================================================================

/// Make symbol(s) public/exported (visible to linker)
constexpr const char* PUBLIC = "PUBLIC";

/// Make symbol(s) public - alternate name
constexpr const char* GLOBAL = "GLOBAL";

/// Make symbol(s) public - alternate name (entry point)
constexpr const char* ENTRY = "ENTRY";

/// Declare external/imported symbol(s)
constexpr const char* EXTERN = "EXTERN";

/// Declare external symbol(s) - alternate name
constexpr const char* EXTRN = "EXTRN";

/// Declare external symbol(s) - short form
constexpr const char* EXT = "EXT";

// ============================================================================
// Conditional Assembly
// ============================================================================

/// Begin conditional assembly block
constexpr const char* IF = "IF";

/// Alternative branch in conditional assembly
constexpr const char* ELSE = "ELSE";

/// End conditional assembly block
constexpr const char* ENDIF = "ENDIF";

// ============================================================================
// Macro Definition
// ============================================================================

/// Begin macro definition
constexpr const char* MACRO = "MACRO";

/// End macro definition
constexpr const char* ENDM = "ENDM";

/// Repeat block (loop)
constexpr const char* REPT = "REPT";

// ============================================================================
// Assembly Control
// ============================================================================

/// End assembly (stop processing source)
constexpr const char* END = "END";

} // namespace CommonDirectives
} // namespace xasm
