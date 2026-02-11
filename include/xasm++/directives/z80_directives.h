/**
 * @file z80_directives.h
 * @brief Z80-specific assembler directive name constants
 *
 * Defines constants for directives specific to Z80 assembler syntax
 * (and related processors like Z180, R800, etc.). These directives
 * control segmentation, code organization, and Z80-specific features.
 *
 * **Z80 Segment Model:**
 * - ASEG: Absolute segment (fixed address)
 * - CSEG: Code segment (relocatable code)
 * - DSEG: Data segment (relocatable data)
 *
 * **Usage Example:**
 * @code
 * #include "xasm++/directives/z80_directives.h"
 *
 * using namespace xasm::Z80Directives;
 *
 * directive_registry_.Register(CSEG, [this](...) { ... });
 * directive_registry_.Register(DSEG, [this](...) { ... });
 * @endcode
 */

#pragma once

namespace xasm {
namespace Z80Directives {

// ============================================================================
// Segment Control (Z80 Linker Model)
// ============================================================================

/// Code segment - relocatable code section
constexpr const char* CSEG = "CSEG";

/// Data segment - relocatable data section
constexpr const char* DSEG = "DSEG";

/// Absolute segment - fixed address section
constexpr const char* ASEG = "ASEG";

// ============================================================================
// Z80-Specific Data Definition
// ============================================================================

/// Define doubleword(s) (32-bit values) - some Z80 assemblers
constexpr const char* DD = "DD";

/// Define doubleword(s) - alternate name
constexpr const char* DEFD = "DEFD";

/// Define doubleword(s) - alternate name
constexpr const char* DWORD = "DWORD";

// ============================================================================
// Z80 Common Block (BSS-like)
// ============================================================================

/// Begin common block definition
constexpr const char* COMMON = "COMMON";

/// End common block definition
constexpr const char* ENDC = "ENDC";

// ============================================================================
// Z80 Conditional Assembly Extensions
// ============================================================================

/// Conditional assembly - if defined
constexpr const char* IFDEF = "IFDEF";

/// Conditional assembly - if not defined
constexpr const char* IFNDEF = "IFNDEF";

/// Conditional assembly - if equal (expression = 0)
constexpr const char* IFE = "IFE";

/// Conditional assembly - if equal (two operands)
constexpr const char* IFEQ = "IFEQ";

/// Conditional assembly - if not equal
constexpr const char* IFNE = "IFNE";

/// Conditional assembly - if greater than
constexpr const char* IFGT = "IFGT";

/// Conditional assembly - if greater than or equal
constexpr const char* IFGE = "IFGE";

/// Conditional assembly - if less than
constexpr const char* IFLT = "IFLT";

/// Conditional assembly - if less than or equal
constexpr const char* IFLE = "IFLE";

/// Conditional assembly - if first pass
constexpr const char* IF1 = "IF1";

/// Conditional assembly - if second pass
constexpr const char* IF2 = "IF2";

/// Conditional assembly - if blank (empty operand)
constexpr const char* IFB = "IFB";

/// Conditional assembly - if not blank (non-empty operand)
constexpr const char* IFNB = "IFNB";

/// Conditional assembly - if identical (string comparison)
constexpr const char* IFIDN = "IFIDN";

/// Conditional assembly - if different (string comparison)
constexpr const char* IFDIF = "IFDIF";

// ============================================================================
// Z80 Include and Library
// ============================================================================

/// Include source file
constexpr const char* INCLUDE = "INCLUDE";

/// Include binary file
constexpr const char* INCBIN = "INCBIN";

/// Search library (some Z80 linkers)
constexpr const char* LIBRARY = "LIBRARY";

// ============================================================================
// Z80 Listing Control
// ============================================================================

/// Enable listing output
constexpr const char* LIST = "LIST";

/// Disable listing output
constexpr const char* NOLIST = "NOLIST";

/// Page break in listing
constexpr const char* PAGE = "PAGE";

/// Set listing title
constexpr const char* TITLE = "TITLE";

/// Set listing subtitle
constexpr const char* SUBTTL = "SUBTTL";

/// Disable listing output (alternate form)
constexpr const char* XLIST = "XLIST";

/// Page eject in listing
constexpr const char* EJECT = "EJECT";

/// Insert blank lines in listing (operand = number of lines)
constexpr const char* SPACE = "SPACE";

/// Set module name
constexpr const char* NAME = "NAME";

// ============================================================================
// Z80 Macro Extensions
// ============================================================================

/// End macro - alternate form
constexpr const char* ENDMACRO = "ENDMACRO";

/// Local symbol declaration (macro scope)
constexpr const char* LOCAL = "LOCAL";

/// Exit macro early
constexpr const char* EXITM = "EXITM";

/// Infinite repeat (until ENDR)
constexpr const char* IRP = "IRP";

/// Iterate over characters in string (IRPC param, string)
constexpr const char* IRPC = "IRPC";

/// End repeat block
constexpr const char* ENDR = "ENDR";

// ============================================================================
// Z80 Module/Section Control
// ============================================================================

/// Define module name
constexpr const char* MODULE = "MODULE";

/// Define named section
constexpr const char* SECTION = "SECTION";

/// End module
constexpr const char* ENDMOD = "ENDMOD";

// ============================================================================
// Z80 Alignment
// ============================================================================

/// Align to boundary
constexpr const char* ALIGN = "ALIGN";

/// Phase (logical address different from physical)
constexpr const char* PHASE = "PHASE";

/// End phase
constexpr const char* DEPHASE = "DEPHASE";

/// Set default number base (2-16)
constexpr const char* RADIX = "RADIX";

/// Get current date as integer (YYYYMMDD)
constexpr const char* DATE = "DATE";

/// Get current time as integer (HHMMSS)
constexpr const char* TIME = "TIME";

} // namespace Z80Directives
} // namespace xasm
