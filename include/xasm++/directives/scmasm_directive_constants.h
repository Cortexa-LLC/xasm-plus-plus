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
constexpr const char* OR = ".OR";

// Symbol definition
constexpr const char* EQ = ".EQ";
constexpr const char* SE = ".SE";

// String directives
constexpr const char* AS = ".AS";
constexpr const char* AT = ".AT";
constexpr const char* AZ = ".AZ";

// Data directives
constexpr const char* DA = ".DA";
constexpr const char* DFB = ".DFB";  // Alias to .DA
constexpr const char* HS = ".HS";
constexpr const char* BS = ".BS";

// Macro directives
constexpr const char* MA = ".MA";
constexpr const char* ENDM = ".ENDM";
constexpr const char* EM = ".EM";    // Alias to .ENDM

} // namespace directives
} // namespace scmasm
} // namespace xasm

#endif // XASMPP_DIRECTIVES_SCMASM_DIRECTIVE_CONSTANTS_H
