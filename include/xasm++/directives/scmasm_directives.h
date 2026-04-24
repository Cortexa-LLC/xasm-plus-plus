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
constexpr const char* kDO = ".DO";

/// Alternative branch in conditional (.ELSE)
constexpr const char* kELSE = ".ELSE";

/// End conditional block (.FIN)
constexpr const char* kFIN = ".FIN";

/// Begin IF block (.IF)
constexpr const char* kIF = ".IF";

/// End IF block (.ENDIF)
constexpr const char* kENDIF = ".ENDIF";

// ============================================================================
// SCMASM Loop Constructs
// ============================================================================

/// Begin WHILE loop (.WHILE)
constexpr const char* kWHILE = ".WHILE";

/// End WHILE loop (.ENDW)
constexpr const char* kENDW = ".ENDW";

/// Begin REPEAT loop (.REPEAT)
constexpr const char* kREPEAT = ".REPEAT";

/// UNTIL condition for REPEAT (.UNTIL)
constexpr const char* kUNTIL = ".UNTIL";

/// End REPEAT loop (.ENDR)
constexpr const char* kENDR = ".ENDR";

// ============================================================================
// SCMASM Origin and Section
// ============================================================================

/// Set origin address (.ORG)
constexpr const char* kORG = ".ORG";

/// Relocatable section (.SECTION)
constexpr const char* kSECTION = ".SECTION";

/// End section (.ENDS)
constexpr const char* kENDS = ".ENDS";

// ============================================================================
// SCMASM Data Definition
// ============================================================================

/// Define byte (.DB)
constexpr const char* kDB = ".DB";

/// Define word (.DW)
constexpr const char* kDW = ".DW";

/// Define doubleword (.DD)
constexpr const char* kDD = ".DD";

/// Define space (.DS)
constexpr const char* kDS = ".DS";

/// ASCII string (.ASCII)
constexpr const char* kASCII = ".ASCII";

/// ASCII string with zero terminator (.ASCIZ)
constexpr const char* kASCIZ = ".ASCIZ";

/// Fill memory (.FILL)
constexpr const char* kFILL = ".FILL";

/// Block storage (.BLOCK)
constexpr const char* kBLOCK = ".BLOCK";

// ============================================================================
// SCMASM Symbol Definition
// ============================================================================

/// Equate symbol (.EQU)
constexpr const char* kEQU = ".EQU";

/// Set symbol (.SET)
constexpr const char* kSET = ".SET";

/// Assignment (.=)
constexpr const char* kEQUALS = ".=";

// ============================================================================
// SCMASM File Inclusion
// ============================================================================

/// Include source file (.INCLUDE)
constexpr const char* kINCLUDE = ".INCLUDE";

/// Include binary file (.INCBIN)
constexpr const char* kINCBIN = ".INCBIN";

// ============================================================================
// SCMASM Macro Definition
// ============================================================================

/// Begin macro definition (.MACRO)
constexpr const char* kMACRO = ".MACRO";

/// End macro definition (.ENDM)
constexpr const char* kENDM = ".ENDM";

/// Local symbol in macro (.LOCAL)
constexpr const char* kLOCAL = ".LOCAL";

/// Exit macro (.EXITM)
constexpr const char* kEXITM = ".EXITM";

// ============================================================================
// SCMASM Listing Control
// ============================================================================

/// Enable listing (.LIST)
constexpr const char* kLIST = ".LIST";

/// Disable listing (.NOLIST)
constexpr const char* kNOLIST = ".NOLIST";

/// Page break (.PAGE)
constexpr const char* kPAGE = ".PAGE";

/// Set title (.TITLE)
constexpr const char* kTITLE = ".TITLE";

/// Set subtitle (.SBTTL)
constexpr const char* kSBTTL = ".SBTTL";

/// Set page width (.WIDTH)
constexpr const char* kWIDTH = ".WIDTH";

/// Set page length (.LENGTH)
constexpr const char* kLENGTH = ".LENGTH";

// ============================================================================
// SCMASM Error Control
// ============================================================================

/// Force error with message (.ERROR)
constexpr const char* kERROR = ".ERROR";

/// Display warning message (.WARNING)
constexpr const char* kWARNING = ".WARNING";

/// Display message (.MESSAGE)
constexpr const char* kMESSAGE = ".MESSAGE";

// ============================================================================
// SCMASM Symbol Control
// ============================================================================

/// Global symbol (.GLOBAL)
constexpr const char* kGLOBAL = ".GLOBAL";

/// External symbol (.EXTERN)
constexpr const char* kEXTERN = ".EXTERN";

/// Public symbol (.PUBLIC)
constexpr const char* kPUBLIC = ".PUBLIC";

/// Export symbol (.EXPORT)
constexpr const char* kEXPORT = ".EXPORT";

/// Import symbol (.IMPORT)
constexpr const char* kIMPORT = ".IMPORT";

// ============================================================================
// SCMASM Alignment
// ============================================================================

/// Align to boundary (.ALIGN)
constexpr const char* kALIGN = ".ALIGN";

/// Align to even address (.EVEN)
constexpr const char* kEVEN = ".EVEN";

/// Align to odd address (.ODD)
constexpr const char* kODD = ".ODD";

// ============================================================================
// SCMASM Special Directives
// ============================================================================

/// End assembly (.END)
constexpr const char* kEND = ".END";

/// Option control (.OPT)
constexpr const char* kOPT = ".OPT";

/// Processor selection (.PROCESSOR)
constexpr const char* kPROCESSOR = ".PROCESSOR";

/// CPU directive (.CPU)
constexpr const char* kCPU = ".CPU";

// ============================================================================
// SCMASM Object Format
// ============================================================================

/// Object file format (.FORMAT)
constexpr const char* kFORMAT = ".FORMAT";

/// Entry point (.ENTRY)
constexpr const char* kENTRY = ".ENTRY";

/// Library search (.LIBRARY)
constexpr const char* kLIBRARY = ".LIBRARY";

}  // namespace SCMASMDirectives
}  // namespace xasm
