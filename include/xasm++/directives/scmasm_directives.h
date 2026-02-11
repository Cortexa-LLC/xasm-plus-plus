/**
 * @file scmasm_directives.h
 * @brief SCMASM assembler directive name constants
 *
 * Defines constants for directives specific to the SCMASM assembler
 * (used for 6800/6809 and other Motorola processors). SCMASM uses
 * dot-prefixed directives (.DO, .ELSE, .FIN) for conditional assembly
 * and other controls.
 *
 * **SCMASM Features:**
 * - Dot-prefixed directives (distinguishes from mnemonics)
 * - Conditional assembly (.DO, .ELSE, .FIN)
 * - Structured conditionals (.IF, .WHILE, .REPEAT)
 * - Include files (.INCLUDE)
 *
 * **Usage Example:**
 * @code
 * #include "xasm++/directives/scmasm_directives.h"
 *
 * using namespace xasm::SCMASMDirectives;
 *
 * directive_registry_.Register(DO, [this](...) { ... });
 * directive_registry_.Register(ELSE, [this](...) { ... });
 * @endcode
 */

#pragma once

namespace xasm {
namespace SCMASMDirectives {

// ============================================================================
// SCMASM Conditional Assembly
// ============================================================================

/// Begin conditional block (.DO)
constexpr const char* DO = ".DO";

/// Alternative branch in conditional (.ELSE)
constexpr const char* ELSE = ".ELSE";

/// End conditional block (.FIN)
constexpr const char* FIN = ".FIN";

/// Begin IF block (.IF)
constexpr const char* IF = ".IF";

/// End IF block (.ENDIF)
constexpr const char* ENDIF = ".ENDIF";

// ============================================================================
// SCMASM Loop Constructs
// ============================================================================

/// Begin WHILE loop (.WHILE)
constexpr const char* WHILE = ".WHILE";

/// End WHILE loop (.ENDW)
constexpr const char* ENDW = ".ENDW";

/// Begin REPEAT loop (.REPEAT)
constexpr const char* REPEAT = ".REPEAT";

/// UNTIL condition for REPEAT (.UNTIL)
constexpr const char* UNTIL = ".UNTIL";

/// End REPEAT loop (.ENDR)
constexpr const char* ENDR = ".ENDR";

// ============================================================================
// SCMASM Origin and Section
// ============================================================================

/// Set origin address (.ORG)
constexpr const char* ORG = ".ORG";

/// Relocatable section (.SECTION)
constexpr const char* SECTION = ".SECTION";

/// End section (.ENDS)
constexpr const char* ENDS = ".ENDS";

// ============================================================================
// SCMASM Data Definition
// ============================================================================

/// Define byte (.DB)
constexpr const char* DB = ".DB";

/// Define word (.DW)
constexpr const char* DW = ".DW";

/// Define doubleword (.DD)
constexpr const char* DD = ".DD";

/// Define space (.DS)
constexpr const char* DS = ".DS";

/// ASCII string (.ASCII)
constexpr const char* ASCII = ".ASCII";

/// ASCII string with zero terminator (.ASCIZ)
constexpr const char* ASCIZ = ".ASCIZ";

/// Fill memory (.FILL)
constexpr const char* FILL = ".FILL";

/// Block storage (.BLOCK)
constexpr const char* BLOCK = ".BLOCK";

// ============================================================================
// SCMASM Symbol Definition
// ============================================================================

/// Equate symbol (.EQU)
constexpr const char* EQU = ".EQU";

/// Set symbol (.SET)
constexpr const char* SET = ".SET";

/// Assignment (.=)
constexpr const char* EQUALS = ".=";

// ============================================================================
// SCMASM File Inclusion
// ============================================================================

/// Include source file (.INCLUDE)
constexpr const char* INCLUDE = ".INCLUDE";

/// Include binary file (.INCBIN)
constexpr const char* INCBIN = ".INCBIN";

// ============================================================================
// SCMASM Macro Definition
// ============================================================================

/// Begin macro definition (.MACRO)
constexpr const char* MACRO = ".MACRO";

/// End macro definition (.ENDM)
constexpr const char* ENDM = ".ENDM";

/// Local symbol in macro (.LOCAL)
constexpr const char* LOCAL = ".LOCAL";

/// Exit macro (.EXITM)
constexpr const char* EXITM = ".EXITM";

// ============================================================================
// SCMASM Listing Control
// ============================================================================

/// Enable listing (.LIST)
constexpr const char* LIST = ".LIST";

/// Disable listing (.NOLIST)
constexpr const char* NOLIST = ".NOLIST";

/// Page break (.PAGE)
constexpr const char* PAGE = ".PAGE";

/// Set title (.TITLE)
constexpr const char* TITLE = ".TITLE";

/// Set subtitle (.SBTTL)
constexpr const char* SBTTL = ".SBTTL";

/// Set page width (.WIDTH)
constexpr const char* WIDTH = ".WIDTH";

/// Set page length (.LENGTH)
constexpr const char* LENGTH = ".LENGTH";

// ============================================================================
// SCMASM Error Control
// ============================================================================

/// Force error with message (.ERROR)
constexpr const char* ERROR = ".ERROR";

/// Display warning message (.WARNING)
constexpr const char* WARNING = ".WARNING";

/// Display message (.MESSAGE)
constexpr const char* MESSAGE = ".MESSAGE";

// ============================================================================
// SCMASM Symbol Control
// ============================================================================

/// Global symbol (.GLOBAL)
constexpr const char* GLOBAL = ".GLOBAL";

/// External symbol (.EXTERN)
constexpr const char* EXTERN = ".EXTERN";

/// Public symbol (.PUBLIC)
constexpr const char* PUBLIC = ".PUBLIC";

/// Export symbol (.EXPORT)
constexpr const char* EXPORT = ".EXPORT";

/// Import symbol (.IMPORT)
constexpr const char* IMPORT = ".IMPORT";

// ============================================================================
// SCMASM Alignment
// ============================================================================

/// Align to boundary (.ALIGN)
constexpr const char* ALIGN = ".ALIGN";

/// Align to even address (.EVEN)
constexpr const char* EVEN = ".EVEN";

/// Align to odd address (.ODD)
constexpr const char* ODD = ".ODD";

// ============================================================================
// SCMASM Special Directives
// ============================================================================

/// End assembly (.END)
constexpr const char* END = ".END";

/// Option control (.OPT)
constexpr const char* OPT = ".OPT";

/// Processor selection (.PROCESSOR)
constexpr const char* PROCESSOR = ".PROCESSOR";

/// CPU directive (.CPU)
constexpr const char* CPU = ".CPU";

// ============================================================================
// SCMASM Object Format
// ============================================================================

/// Object file format (.FORMAT)
constexpr const char* FORMAT = ".FORMAT";

/// Entry point (.ENTRY)
constexpr const char* ENTRY = ".ENTRY";

/// Library search (.LIBRARY)
constexpr const char* LIBRARY = ".LIBRARY";

} // namespace SCMASMDirectives
} // namespace xasm
