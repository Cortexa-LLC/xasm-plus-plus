/**
 * @file motorola_directives.h
 * @brief Motorola 6809/FLEX assembler directive constants
 *
 * Defines constants for directive names used in Motorola 6809 assemblers
 * (FLEX, EDTASM, etc.). Eliminates magic strings in directive handlers.
 */

#pragma once

namespace xasm {
namespace motorola {

// ============================================================================
// Data Definition Directives
// ============================================================================

/// FCB - Form Constant Byte (define byte data)
constexpr const char *FCB = "FCB";

/// FDB - Form Double Byte (define 16-bit word data, big-endian)
constexpr const char *FDB = "FDB";

/// FCC - Form Constant Character (define string data)
constexpr const char *FCC = "FCC";

/// SET - Define variable symbol (can be redefined)
constexpr const char *SET = "SET";

// ============================================================================
// Direct Page Addressing
// ============================================================================

/// SETDP - Set direct page register value for optimization hints
constexpr const char *SETDP = "SETDP";

// ============================================================================
// Listing Control Directives
// ============================================================================

/// NAM - Set program name
constexpr const char *NAM = "NAM";

/// TTL - Set listing title
constexpr const char *TTL = "TTL";

/// STTL - Set subtitle
constexpr const char *STTL = "STTL";

/// PAGE - Page eject (start new page in listing)
constexpr const char *PAGE = "PAGE";

/// SPC - Insert space/blank lines in listing
constexpr const char *SPC = "SPC";

// ============================================================================
// Conditional Assembly
// ============================================================================

/// IFC - If Condition (begin conditional assembly block)
constexpr const char *IFC = "IFC";

/// ENDC - End Condition (end conditional assembly block)
constexpr const char *ENDC = "ENDC";

// ============================================================================
// Option Values
// ============================================================================

/// ON - Enable option
constexpr const char *ON = "ON";

/// OFF - Disable option
constexpr const char *OFF = "OFF";

} // namespace motorola
} // namespace xasm
