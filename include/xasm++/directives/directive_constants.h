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

namespace xasm::directives {

// ============================================================================
// Origin and Address Directives
// ============================================================================

/// ORG directive - Set program counter to specific address
constexpr const char* kORG = "ORG";

// ============================================================================
// Symbol Definition Directives
// ============================================================================

/// EQU directive - Define constant symbol
constexpr const char* kEQU = "EQU";

/// SET directive - Define variable symbol (allows redefinition)
constexpr const char* kSET = "SET";

// ============================================================================
// Data Definition Directives
// ============================================================================

/// DB directive - Define byte data (primary name)
constexpr const char* kDB = "DB";

/// DEFB directive - Define byte data (alias for DB)
constexpr const char* kDEFB = "DEFB";

/// DFB directive - Define byte data (Merlin alias for DB)
constexpr const char* kDFB = "DFB";

/// BYTE directive - Define byte data (alias for DB)
constexpr const char* kBYTE = "BYTE";

/// DW directive - Define word data (primary name)
constexpr const char* kDW = "DW";

/// DEFW directive - Define word data (alias for DW)
constexpr const char* kDEFW = "DEFW";

/// WORD directive - Define word data (alias for DW)
constexpr const char* kWORD = "WORD";

// ============================================================================
// FLEX ASM09-Specific Data Directives
// ============================================================================

/// FCB directive - Form Constant Byte (FLEX ASM09)
constexpr const char* kFCB = "FCB";

/// FDB directive - Form Double Byte (FLEX ASM09 16-bit word)
constexpr const char* kFDB = "FDB";

/// FCC directive - Form Constant Characters (FLEX ASM09 string)
constexpr const char* kFCC = "FCC";

// ============================================================================
// Space Reservation Directives
// ============================================================================

/// DS directive - Define space / reserve bytes (primary name)
constexpr const char* kDS = "DS";

/// DEFS directive - Define space (alias for DS)
constexpr const char* kDEFS = "DEFS";

/// BLOCK directive - Define space (alias for DS)
constexpr const char* kBLOCK = "BLOCK";

/// RMB directive - Reserve memory bytes (alias for DS)
constexpr const char* kRMB = "RMB";

// ============================================================================
// FLEX ASM09-Specific Control Directives
// ============================================================================

/// SETDP directive - Set Direct Page register (FLEX ASM09)
constexpr const char* kSETDP = "SETDP";

/// NAM directive - Set program name/title (FLEX ASM09)
constexpr const char* kNAM = "NAM";

/// TTL directive - Set title (FLEX ASM09 alias for NAM)
constexpr const char* kTTL = "TTL";

/// STTL directive - Set subtitle (FLEX ASM09)
constexpr const char* kSTTL = "STTL";

/// PAGE directive - Form feed / new page (FLEX ASM09)
constexpr const char* kPAGE = "PAGE";

/// SPC directive - Space lines (FLEX ASM09)
constexpr const char* kSPC = "SPC";

/// IFC directive - If Condition (FLEX ASM09 conditional assembly)
constexpr const char* kIFC = "IFC";

/// ENDC directive - End Conditional (FLEX ASM09)
constexpr const char* kENDC = "ENDC";

/// RPT directive - Repeat block (FLEX ASM09)
constexpr const char* kRPT = "RPT";

/// ENDR directive - End Repeat block (FLEX ASM09)
constexpr const char* kENDR = "ENDR";

// ============================================================================
// Expression Functions
// ============================================================================

/// LOW function - Extract low byte of value
constexpr const char* kLOW_FUNC = "LOW";

/// HIGH function - Extract high byte of value
constexpr const char* kHIGH_FUNC = "HIGH";

// ============================================================================
// Merlin-Specific Directives
// ============================================================================

/// HEX directive - Define hex bytes
constexpr const char* kHEX = "HEX";

/// ASC directive - ASCII string with high bit set
constexpr const char* kASC = "ASC";

/// DCI directive - DCI string (last char with high bit set)
constexpr const char* kDCI = "DCI";

/// INV directive - Inverse ASCII (all chars with high bit set)
constexpr const char* kINV = "INV";

/// FLS directive - Flash ASCII (alternating high bit)
constexpr const char* kFLS = "FLS";

/// DA directive - Define address (same as DW)
constexpr const char* kDA = "DA";

/// END directive - End of source
constexpr const char* kEND = "END";

/// SAV directive - Save output filename
constexpr const char* kSAV = "SAV";

/// XC directive - Toggle 65C02 mode
constexpr const char* kXC = "XC";

/// ON option - Enable option
constexpr const char* kON = "ON";

/// OFF option - Disable option
constexpr const char* kOFF = "OFF";

/// MX directive - Set 65816 register widths
constexpr const char* kMX = "MX";

/// REV directive - Reverse ASCII string
constexpr const char* kREV = "REV";

/// DUM directive - Start dummy section
constexpr const char* kDUM = "DUM";

/// DEND directive - End dummy section
constexpr const char* kDEND = "DEND";

/// LST directive - Listing control
constexpr const char* kLST = "LST";

/// LSTDO directive - List during DO blocks
constexpr const char* kLSTDO = "LSTDO";

/// TR directive - Truncate listing
constexpr const char* kTR = "TR";

/// USR directive - User-defined subroutine
constexpr const char* kUSR = "USR";

/// LUP directive - Loop assembly
constexpr const char* kLUP = "LUP";

/// DO directive - Start conditional assembly block
constexpr const char* kDO = "DO";

/// FIN directive - End DO block
constexpr const char* kFIN = "FIN";

/// PUT directive - Include source file
constexpr const char* kPUT = "PUT";

/// MAC directive - Define macro
constexpr const char* kMAC = "MAC";

/// EOM directive - End of macro definition
constexpr const char* kEOM = "EOM";

/// MACRO_END_ALT directive - Alternative end of macro definition (Merlin style)
constexpr const char* kMACRO_END_ALT = "<<<";

/// PMC directive - Print macro call
constexpr const char* kPMC = "PMC";

// ============================================================================
// EDTASM-M80++ Specific Directives
// ============================================================================

// Segment Control
/// ASEG directive - Absolute segment
constexpr const char* kASEG = "ASEG";

/// CSEG directive - Code segment
constexpr const char* kCSEG = "CSEG";

/// DSEG directive - Data segment
constexpr const char* kDSEG = "DSEG";

// Conditional Assembly
/// IF directive - Start conditional block
constexpr const char* kIF = "IF";

/// ELSE directive - Else clause
constexpr const char* kELSE = "ELSE";

/// ENDIF directive - End conditional block
constexpr const char* kENDIF = "ENDIF";

/// IFDEF directive - If symbol defined
constexpr const char* kIFDEF = "IFDEF";

/// IFNDEF directive - If symbol not defined
constexpr const char* kIFNDEF = "IFNDEF";

/// IFEQ directive - If equal
constexpr const char* kIFEQ = "IFEQ";

/// IFNE directive - If not equal
constexpr const char* kIFNE = "IFNE";

/// IFLT directive - If less than
constexpr const char* kIFLT = "IFLT";

/// IFGT directive - If greater than
constexpr const char* kIFGT = "IFGT";

/// IFLE directive - If less than or equal
constexpr const char* kIFLE = "IFLE";

/// IFGE directive - If greater than or equal
constexpr const char* kIFGE = "IFGE";

/// IF1 directive - If first pass
constexpr const char* kIF1 = "IF1";

/// IF2 directive - If second pass
constexpr const char* kIF2 = "IF2";

/// IFB directive - If blank
constexpr const char* kIFB = "IFB";

/// IFNB directive - If not blank
constexpr const char* kIFNB = "IFNB";

/// IFIDN directive - If identical
constexpr const char* kIFIDN = "IFIDN";

/// IFDIF directive - If different
constexpr const char* kIFDIF = "IFDIF";

// Macro System
/// MACRO directive - Start macro definition
constexpr const char* kMACRO = "MACRO";

/// ENDM directive - End macro/repeat block
constexpr const char* kENDM = "ENDM";

/// LOCAL directive - Define local symbol in macro
constexpr const char* kLOCAL = "LOCAL";

/// EXITM directive - Exit macro early
constexpr const char* kEXITM = "EXITM";

/// REPT directive - Repeat block
constexpr const char* kREPT = "REPT";

/// IRP directive - Iterate over parameters
constexpr const char* kIRP = "IRP";

/// IRPC directive - Iterate over characters
constexpr const char* kIRPC = "IRPC";

// Listing Control
/// LIST directive - Enable listing
constexpr const char* kLIST = "LIST";

/// NOLIST directive - Disable listing
constexpr const char* kNOLIST = "NOLIST";

/// XLIST directive - Disable listing (alias for NOLIST)
constexpr const char* kXLIST = "XLIST";

/// LALL directive - List all (including macro expansions)
constexpr const char* kLALL = "LALL";

/// SALL directive - Suppress all listing
constexpr const char* kSALL = "SALL";

/// TITLE directive - Set listing title
constexpr const char* kTITLE = "TITLE";

/// SUBTTL directive - Set subtitle
constexpr const char* kSUBTTL = "SUBTTL";

/// EJECT directive - Page eject
constexpr const char* kEJECT = "EJECT";

/// SPACE directive - Add blank lines
constexpr const char* kSPACE = "SPACE";

/// NAME directive - Set module name
constexpr const char* kNAME = "NAME";

/// RADIX directive - Set number radix
constexpr const char* kRADIX = "RADIX";

// Dot-prefixed variants (EDTASM-M80++ alternate syntax)
/// .LIST directive - Enable listing (dot-prefixed variant)
constexpr const char* kDOT_LIST = ".LIST";

/// .XLIST directive - Disable listing (dot-prefixed variant)
constexpr const char* kDOT_XLIST = ".XLIST";

/// .TITLE directive - Set listing title (dot-prefixed variant)
constexpr const char* kDOT_TITLE = ".TITLE";

/// .SUBTTL directive - Set subtitle (dot-prefixed variant)
constexpr const char* kDOT_SUBTTL = ".SUBTTL";

/// .RADIX directive - Set number radix (dot-prefixed variant)
constexpr const char* kDOT_RADIX = ".RADIX";

// Star-prefixed variants (EDTASM-M80++ special syntax)
/// *LIST directive - Toggle listing
constexpr const char* kSTAR_LIST = "*LIST";

/// *RADIX directive - Set radix (star-prefixed variant)
constexpr const char* kSTAR_RADIX = "*RADIX";

// ============================================================================
// Error Message Components
// ============================================================================

namespace errors {

/// Error message prefix for ORG directive
constexpr const char* kORG_PREFIX = "ORG: ";

/// Missing operand error suffix
constexpr const char* kMISSING_OPERAND = "Missing operand";

/// Missing address error
constexpr const char* kMISSING_ADDRESS = "Missing address operand";

/// Error message prefix for EQU directive
constexpr const char* kEQU_PREFIX = "EQU: ";

/// Missing label error
constexpr const char* kMISSING_LABEL = "Missing label";

/// Error message prefix for DS directive
constexpr const char* kDS_PREFIX = "DS: ";

/// Negative count error
constexpr const char* kNEGATIVE_COUNT = "Negative count not allowed";

/// Negative address error
constexpr const char* kNEGATIVE_ADDRESS = "Negative address not allowed";

}  // namespace errors

}  // namespace xasm::directives
