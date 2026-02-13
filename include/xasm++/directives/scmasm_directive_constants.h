/**
 * @file scmasm_directive_constants.h
 * @brief SCMASM directive name constants
 *
 * Constants for SCMASM directive names to eliminate magic strings.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#ifndef XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H
#define XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H

namespace xasm {
namespace scmasm {
namespace directives {

// Origin and addressing
constexpr const char *OR = ".OR";

// Symbol definition
constexpr const char *EQ = ".EQ";
constexpr const char *SE = ".SE";

// String directives
constexpr const char *AS = ".AS";
constexpr const char *AT = ".AT";
constexpr const char *AZ = ".AZ";

// Data directives
constexpr const char *DA = ".DA";
constexpr const char *DFB = ".DFB"; // Alias to .DA
constexpr const char *HS = ".HS";
constexpr const char *BS = ".BS";

// Macro directives
constexpr const char *MA = ".MA";
constexpr const char *ENDM = ".ENDM";
constexpr const char *EM = ".EM"; // Alias to .ENDM

// P0 Priority Directives (A2oSX Critical)
constexpr const char *PS = ".PS";       // Pascal string (length-prefixed)
constexpr const char *INB = ".INB";     // Include binary file
constexpr const char *LIST = ".LIST";   // Listing control
constexpr const char *DUMMY = ".DUMMY"; // Dummy section
constexpr const char *OP = ".OP";       // CPU operation mode

// Phase 3: 100% Coverage Directives
constexpr const char *CS = ".CS";   // C-string with escape sequences
constexpr const char *CZ = ".CZ";   // C-string zero-terminated
constexpr const char *TF = ".TF";   // Text file/title metadata
constexpr const char *EP = ".EP";   // Entry point
constexpr const char *HX = ".HX";   // Hex nibble storage
constexpr const char *TA = ".TA";   // Target address (no-op)
constexpr const char *AC = ".AC";     // ASCII with numeric prefix
constexpr const char *DO = ".DO";     // Conditional assembly start
constexpr const char *ELSE = ".ELSE"; // Conditional assembly else
constexpr const char *FIN = ".FIN";   // Conditional assembly end
constexpr const char *LU = ".LU";     // Lookup table
constexpr const char *ENDU = ".ENDU"; // End user macro/loop

} // namespace directives
} // namespace scmasm
} // namespace xasm

#endif // XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H
