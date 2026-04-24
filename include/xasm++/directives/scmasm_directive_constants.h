/**
 * @file scmasm_directive_constants.h
 * @brief SCMASM directive name constants
 *
 * Constants for SCMASM directive names to eliminate magic strings.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#ifndef XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H
#define XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H

namespace xasm::scmasm::directives {

// Origin and addressing
constexpr const char* kOR = ".OR";
constexpr const char* kPH = ".PH";  // Phase assembly

// Symbol definition
constexpr const char* kEQ = ".EQ";
constexpr const char* kSE = ".SE";

// String directives
constexpr const char* kAS = ".AS";
constexpr const char* kAT = ".AT";
constexpr const char* kAZ = ".AZ";

// Data directives
constexpr const char* kDA = ".DA";
constexpr const char* kDFB = ".DFB";  // Alias to .DA
constexpr const char* kHS = ".HS";
constexpr const char* kBS = ".BS";

// Macro directives
constexpr const char* kMA = ".MA";
constexpr const char* kENDM = ".ENDM";
constexpr const char* kEM = ".EM";  // Alias to .ENDM

// P0 Priority Directives (A2oSX Critical)
constexpr const char* kPS = ".PS";            // Pascal string (length-prefixed)
constexpr const char* kINB = ".INB";          // Include source file
constexpr const char* kINCLUDE = ".INCLUDE";  // Alias: .INCLUDE = .INB
constexpr const char* kLIST = ".LIST";        // Listing control
constexpr const char* kDUMMY = ".DUMMY";      // Dummy section
constexpr const char* kED = ".ED";            // End dummy section
constexpr const char* kOP = ".OP";            // CPU operation mode

// Phase 3: 100% Coverage Directives
constexpr const char* kCS = ".CS";      // C-string with escape sequences
constexpr const char* kCZ = ".CZ";      // C-string zero-terminated
constexpr const char* kTF = ".TF";      // Text file/title metadata
constexpr const char* kEP = ".EP";      // Entry point
constexpr const char* kHX = ".HX";      // Hex nibble storage
constexpr const char* kTA = ".TA";      // Target address (no-op)
constexpr const char* kAC = ".AC";      // ASCII with numeric prefix
constexpr const char* kDO = ".DO";      // Conditional assembly start
constexpr const char* kELSE = ".ELSE";  // Conditional assembly else
constexpr const char* kFIN = ".FIN";    // Conditional assembly end
constexpr const char* kLU = ".LU";      // Lookup table
constexpr const char* kENDU = ".ENDU";  // End user macro/loop

}  // namespace xasm::scmasm::directives

#endif  // XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H
