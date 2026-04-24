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

namespace xasm::Z80Directives {

// ============================================================================
// Segment Control (Z80 Linker Model)
// ============================================================================

/// Code segment - relocatable code section
constexpr const char* kCSEG = "CSEG";

/// Data segment - relocatable data section
constexpr const char* kDSEG = "DSEG";

/// Absolute segment - fixed address section
constexpr const char* kASEG = "ASEG";

// ============================================================================
// Z80-Specific Data Definition
// ============================================================================

/// Define doubleword(s) (32-bit values) - some Z80 assemblers
constexpr const char* kDD = "DD";

/// Define doubleword(s) - alternate name
constexpr const char* kDEFD = "DEFD";

/// Define doubleword(s) - alternate name
constexpr const char* kDWORD = "DWORD";

// ============================================================================
// Z80 Common Block (BSS-like)
// ============================================================================

/// Begin common block definition
constexpr const char* kCOMMON = "COMMON";

/// End common block definition
constexpr const char* kENDC = "ENDC";

// ============================================================================
// Z80 Conditional Assembly Extensions
// ============================================================================

/// Conditional assembly - if defined
constexpr const char* kIFDEF = "IFDEF";

/// Conditional assembly - if not defined
constexpr const char* kIFNDEF = "IFNDEF";

/// Conditional assembly - if equal (expression = 0)
constexpr const char* kIFE = "IFE";

/// Conditional assembly - if equal (two operands)
constexpr const char* kIFEQ = "IFEQ";

/// Conditional assembly - if not equal
constexpr const char* kIFNE = "IFNE";

/// Conditional assembly - if greater than
constexpr const char* kIFGT = "IFGT";

/// Conditional assembly - if greater than or equal
constexpr const char* kIFGE = "IFGE";

/// Conditional assembly - if less than
constexpr const char* kIFLT = "IFLT";

/// Conditional assembly - if less than or equal
constexpr const char* kIFLE = "IFLE";

/// Conditional assembly - if first pass
constexpr const char* kIF1 = "IF1";

/// Conditional assembly - if second pass
constexpr const char* kIF2 = "IF2";

/// Conditional assembly - if blank (empty operand)
constexpr const char* kIFB = "IFB";

/// Conditional assembly - if not blank (non-empty operand)
constexpr const char* kIFNB = "IFNB";

/// Conditional assembly - if identical (string comparison)
constexpr const char* kIFIDN = "IFIDN";

/// Conditional assembly - if different (string comparison)
constexpr const char* kIFDIF = "IFDIF";

// ============================================================================
// Z80 Include and Library
// ============================================================================

/// Include source file
constexpr const char* kINCLUDE = "INCLUDE";

/// Include binary file
constexpr const char* kINCBIN = "INCBIN";

/// Search library (some Z80 linkers)
constexpr const char* kLIBRARY = "LIBRARY";

// ============================================================================
// Z80 Listing Control
// ============================================================================

/// Enable listing output
constexpr const char* kLIST = "LIST";

/// Disable listing output
constexpr const char* kNOLIST = "NOLIST";

/// Page break in listing
constexpr const char* kPAGE = "PAGE";

/// Set listing title
constexpr const char* kTITLE = "TITLE";

/// Set listing subtitle
constexpr const char* kSUBTTL = "SUBTTL";

/// Disable listing output (alternate form)
constexpr const char* kXLIST = "XLIST";

/// Page eject in listing
constexpr const char* kEJECT = "EJECT";

/// Insert blank lines in listing (operand = number of lines)
constexpr const char* kSPACE = "SPACE";

/// Set module name
constexpr const char* kNAME = "NAME";

// ============================================================================
// Z80 Macro Extensions
// ============================================================================

/// End macro - alternate form
constexpr const char* kENDMACRO = "ENDMACRO";

/// Local symbol declaration (macro scope)
constexpr const char* kLOCAL = "LOCAL";

/// Exit macro early
constexpr const char* kEXITM = "EXITM";

/// Infinite repeat (until ENDR)
constexpr const char* kIRP = "IRP";

/// Iterate over characters in string (IRPC param, string)
constexpr const char* kIRPC = "IRPC";

/// End repeat block
constexpr const char* kENDR = "ENDR";

// ============================================================================
// Z80 Module/Section Control
// ============================================================================

/// Define module name
constexpr const char* kMODULE = "MODULE";

/// Define named section
constexpr const char* kSECTION = "SECTION";

/// End module
constexpr const char* kENDMOD = "ENDMOD";

// ============================================================================
// Z80 Alignment
// ============================================================================

/// Align to boundary
constexpr const char* kALIGN = "ALIGN";

/// Phase (logical address different from physical)
constexpr const char* kPHASE = "PHASE";

/// End phase
constexpr const char* kDEPHASE = "DEPHASE";

/// Set default number base (2-16)
constexpr const char* kRADIX = "RADIX";

/// Get current date as integer (YYYYMMDD)
constexpr const char* kDATE = "DATE";

/// Get current time as integer (HHMMSS)
constexpr const char* kTIME = "TIME";

}  // namespace xasm::Z80Directives
