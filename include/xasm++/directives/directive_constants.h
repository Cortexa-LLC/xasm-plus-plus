/**
 * @file directive_constants.h
 * @brief String constants for assembly directives
 *
 * Eliminates magic strings from directive handling code.
 * Part of God-Class Phase 6b refactoring.
 *
 * @section usage Usage Example
 * @code
 * // Instead of:
 * if (directive == "ORG") { ... }  // Magic string
 *
 * // Use:
 * if (directive == directives::ORG) { ... }  // Named constant
 * @endcode
 */

#pragma once

namespace xasm {
namespace directives {

// ============================================================================
// Origin and Address Directives
// ============================================================================

/// ORG directive - Set program counter to specific address
constexpr const char* ORG = "ORG";

// ============================================================================
// Symbol Definition Directives
// ============================================================================

/// EQU directive - Define constant symbol
constexpr const char* EQU = "EQU";

// ============================================================================
// Data Definition Directives
// ============================================================================

/// DB directive - Define byte data (primary name)
constexpr const char* DB = "DB";

/// DEFB directive - Define byte data (alias for DB)
constexpr const char* DEFB = "DEFB";

/// DFB directive - Define byte data (Merlin alias for DB)
constexpr const char* DFB = "DFB";

/// BYTE directive - Define byte data (alias for DB)
constexpr const char* BYTE = "BYTE";

/// DW directive - Define word data (primary name)
constexpr const char* DW = "DW";

/// DEFW directive - Define word data (alias for DW)
constexpr const char* DEFW = "DEFW";

/// WORD directive - Define word data (alias for DW)
constexpr const char* WORD = "WORD";

// ============================================================================
// Space Reservation Directives
// ============================================================================

/// DS directive - Define space / reserve bytes (primary name)
constexpr const char* DS = "DS";

/// DEFS directive - Define space (alias for DS)
constexpr const char* DEFS = "DEFS";

/// BLOCK directive - Define space (alias for DS)
constexpr const char* BLOCK = "BLOCK";

/// RMB directive - Reserve memory bytes (alias for DS)
constexpr const char* RMB = "RMB";

// ============================================================================
// Merlin-Specific Directives
// ============================================================================

/// HEX directive - Define hex bytes
constexpr const char* HEX = "HEX";

/// ASC directive - ASCII string with high bit set
constexpr const char* ASC = "ASC";

/// DCI directive - DCI string (last char with high bit set)
constexpr const char* DCI = "DCI";

/// INV directive - Inverse ASCII (all chars with high bit set)
constexpr const char* INV = "INV";

/// FLS directive - Flash ASCII (alternating high bit)
constexpr const char* FLS = "FLS";

/// DA directive - Define address (same as DW)
constexpr const char* DA = "DA";

/// END directive - End of source
constexpr const char* END = "END";

/// SAV directive - Save output filename
constexpr const char* SAV = "SAV";

/// XC directive - Toggle 65C02 mode
constexpr const char* XC = "XC";

/// MX directive - Set 65816 register widths
constexpr const char* MX = "MX";

/// REV directive - Reverse ASCII string
constexpr const char* REV = "REV";

/// DUM directive - Start dummy section
constexpr const char* DUM = "DUM";

/// DEND directive - End dummy section
constexpr const char* DEND = "DEND";

/// LST directive - Listing control
constexpr const char* LST = "LST";

/// LSTDO directive - List during DO blocks
constexpr const char* LSTDO = "LSTDO";

/// TR directive - Truncate listing
constexpr const char* TR = "TR";

/// USR directive - User-defined subroutine
constexpr const char* USR = "USR";

/// LUP directive - Loop assembly
constexpr const char* LUP = "LUP";

/// DO directive - Start conditional assembly block
constexpr const char* DO = "DO";

/// FIN directive - End DO block
constexpr const char* FIN = "FIN";

/// PUT directive - Include source file
constexpr const char* PUT = "PUT";

/// MAC directive - Define macro
constexpr const char* MAC = "MAC";

/// EOM directive - End of macro definition
constexpr const char* EOM = "EOM";

/// PMC directive - Print macro call
constexpr const char* PMC = "PMC";

// ============================================================================
// EDTASM-M80++ Specific Directives
// ============================================================================

// Segment Control
/// ASEG directive - Absolute segment
constexpr const char* ASEG = "ASEG";

/// CSEG directive - Code segment
constexpr const char* CSEG = "CSEG";

/// DSEG directive - Data segment
constexpr const char* DSEG = "DSEG";

// Conditional Assembly
/// IF directive - Start conditional block
constexpr const char* IF = "IF";

/// ELSE directive - Else clause
constexpr const char* ELSE = "ELSE";

/// ENDIF directive - End conditional block
constexpr const char* ENDIF = "ENDIF";

/// IFDEF directive - If symbol defined
constexpr const char* IFDEF = "IFDEF";

/// IFNDEF directive - If symbol not defined
constexpr const char* IFNDEF = "IFNDEF";

/// IFEQ directive - If equal
constexpr const char* IFEQ = "IFEQ";

/// IFNE directive - If not equal
constexpr const char* IFNE = "IFNE";

/// IFLT directive - If less than
constexpr const char* IFLT = "IFLT";

/// IFGT directive - If greater than
constexpr const char* IFGT = "IFGT";

/// IFLE directive - If less than or equal
constexpr const char* IFLE = "IFLE";

/// IFGE directive - If greater than or equal
constexpr const char* IFGE = "IFGE";

/// IF1 directive - If first pass
constexpr const char* IF1 = "IF1";

/// IF2 directive - If second pass
constexpr const char* IF2 = "IF2";

/// IFB directive - If blank
constexpr const char* IFB = "IFB";

/// IFNB directive - If not blank
constexpr const char* IFNB = "IFNB";

/// IFIDN directive - If identical
constexpr const char* IFIDN = "IFIDN";

/// IFDIF directive - If different
constexpr const char* IFDIF = "IFDIF";

// Macro System
/// MACRO directive - Start macro definition
constexpr const char* MACRO = "MACRO";

/// ENDM directive - End macro/repeat block
constexpr const char* ENDM = "ENDM";

/// LOCAL directive - Define local symbol in macro
constexpr const char* LOCAL = "LOCAL";

/// EXITM directive - Exit macro early
constexpr const char* EXITM = "EXITM";

/// REPT directive - Repeat block
constexpr const char* REPT = "REPT";

/// IRP directive - Iterate over parameters
constexpr const char* IRP = "IRP";

/// IRPC directive - Iterate over characters
constexpr const char* IRPC = "IRPC";

// Listing Control
/// LIST directive - Enable listing
constexpr const char* LIST = "LIST";

/// NOLIST directive - Disable listing
constexpr const char* NOLIST = "NOLIST";

/// XLIST directive - Disable listing (alias for NOLIST)
constexpr const char* XLIST = "XLIST";

/// LALL directive - List all (including macro expansions)
constexpr const char* LALL = "LALL";

/// SALL directive - Suppress all listing
constexpr const char* SALL = "SALL";

/// TITLE directive - Set listing title
constexpr const char* TITLE = "TITLE";

/// SUBTTL directive - Set subtitle
constexpr const char* SUBTTL = "SUBTTL";

/// EJECT directive - Page eject
constexpr const char* EJECT = "EJECT";

/// SPACE directive - Add blank lines
constexpr const char* SPACE = "SPACE";

/// NAME directive - Set module name
constexpr const char* NAME = "NAME";

/// RADIX directive - Set number radix
constexpr const char* RADIX = "RADIX";

// Dot-prefixed variants (EDTASM-M80++ alternate syntax)
/// .LIST directive - Enable listing (dot-prefixed variant)
constexpr const char* DOT_LIST = ".LIST";

/// .XLIST directive - Disable listing (dot-prefixed variant)
constexpr const char* DOT_XLIST = ".XLIST";

/// .TITLE directive - Set listing title (dot-prefixed variant)
constexpr const char* DOT_TITLE = ".TITLE";

/// .SUBTTL directive - Set subtitle (dot-prefixed variant)
constexpr const char* DOT_SUBTTL = ".SUBTTL";

/// .RADIX directive - Set number radix (dot-prefixed variant)
constexpr const char* DOT_RADIX = ".RADIX";

// Star-prefixed variants (EDTASM-M80++ special syntax)
/// *LIST directive - Toggle listing
constexpr const char* STAR_LIST = "*LIST";

/// *RADIX directive - Set radix (star-prefixed variant)
constexpr const char* STAR_RADIX = "*RADIX";

// ============================================================================
// Error Message Components
// ============================================================================

namespace errors {

/// Error message prefix for ORG directive
constexpr const char* ORG_PREFIX = "ORG: ";

/// Missing operand error suffix
constexpr const char* MISSING_OPERAND = "Missing operand";

/// Missing address error
constexpr const char* MISSING_ADDRESS = "Missing address operand";

/// Error message prefix for EQU directive
constexpr const char* EQU_PREFIX = "EQU: ";

/// Missing label error
constexpr const char* MISSING_LABEL = "Missing label";

/// Error message prefix for DS directive
constexpr const char* DS_PREFIX = "DS: ";

/// Negative count error
constexpr const char* NEGATIVE_COUNT = "Negative count not allowed";

/// Negative address error
constexpr const char* NEGATIVE_ADDRESS = "Negative address not allowed";

} // namespace errors

} // namespace directives
} // namespace xasm
