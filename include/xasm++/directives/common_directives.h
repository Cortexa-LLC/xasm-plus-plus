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

namespace xasm::CommonDirectives {

// ============================================================================
// Origin and Address Control
// ============================================================================

/// Set assembly origin address (start address for code generation)
constexpr const char* kORG = "ORG";

// ============================================================================
// Symbol Definition
// ============================================================================

/// Define constant symbol (equate) - value cannot be redefined
constexpr const char* kEQU = "EQU";

/// Define variable symbol - value can be redefined (Z80, some others)
constexpr const char* kSET = "SET";

/// Define variable symbol - alternate name for SET
constexpr const char* kDEFL = "DEFL";

/// Assignment operator (alternate syntax for EQU/SET in some assemblers)
constexpr const char* kEQUALS = "=";

// ============================================================================
// Data Definition
// ============================================================================

/// Define byte(s) - most common data directive
constexpr const char* kDB = "DB";

/// Define byte(s) - alternate name
constexpr const char* kDEFB = "DEFB";

/// Define byte(s) - alternate name
constexpr const char* kBYTE = "BYTE";

/// Define message (byte string) - alternate name for DB
constexpr const char* kDM = "DM";

/// Define message (byte string) - alternate name for DB
constexpr const char* kDEFM = "DEFM";

/// Define text (byte string) - alternate name for DB
constexpr const char* kTEXT = "TEXT";

/// Define ASCII string (byte string) - alternate name for DB
constexpr const char* kASCII = "ASCII";

/// Define word(s) (16-bit values)
constexpr const char* kDW = "DW";

/// Define word(s) - alternate name
constexpr const char* kDEFW = "DEFW";

/// Define word(s) - alternate name
constexpr const char* kWORD = "WORD";

/// Define space - reserve bytes without initialization
constexpr const char* kDS = "DS";

/// Define space - alternate name
constexpr const char* kDEFS = "DEFS";

/// Define space - alternate name
constexpr const char* kDSPACE = "DSPACE";

/// Define space - alternate name (used by some assemblers)
constexpr const char* kBLOCK = "BLOCK";

// ============================================================================
// Symbol Visibility
// ============================================================================

/// Make symbol(s) public/exported (visible to linker)
constexpr const char* kPUBLIC = "PUBLIC";

/// Make symbol(s) public - alternate name
constexpr const char* kGLOBAL = "GLOBAL";

/// Make symbol(s) public - alternate name (entry point)
constexpr const char* kENTRY = "ENTRY";

/// Declare external/imported symbol(s)
constexpr const char* kEXTERN = "EXTERN";

/// Declare external symbol(s) - alternate name
constexpr const char* kEXTRN = "EXTRN";

/// Declare external symbol(s) - short form
constexpr const char* kEXT = "EXT";

// ============================================================================
// Conditional Assembly
// ============================================================================

/// Begin conditional assembly block
constexpr const char* kIF = "IF";

/// Alternative branch in conditional assembly
constexpr const char* kELSE = "ELSE";

/// End conditional assembly block
constexpr const char* kENDIF = "ENDIF";

// ============================================================================
// Macro Definition
// ============================================================================

/// Begin macro definition
constexpr const char* kMACRO = "MACRO";

/// End macro definition
constexpr const char* kENDM = "ENDM";

/// Repeat block (loop)
constexpr const char* kREPT = "REPT";

// ============================================================================
// Assembly Control
// ============================================================================

/// End assembly (stop processing source)
constexpr const char* kEND = "END";

}  // namespace xasm::CommonDirectives
