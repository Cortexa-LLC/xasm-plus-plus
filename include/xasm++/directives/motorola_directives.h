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
constexpr const char* kFCB = "FCB";

/// FDB - Form Double Byte (define 16-bit word data, big-endian)
constexpr const char* kFDB = "FDB";

/// FCC - Form Constant Character (define string data)
constexpr const char* kFCC = "FCC";

/// SET - Define variable symbol (can be redefined)
constexpr const char* kSET = "SET";

// ============================================================================
// Direct Page Addressing
// ============================================================================

/// SETDP - Set direct page register value for optimization hints
constexpr const char* kSETDP = "SETDP";

// ============================================================================
// Listing Control Directives
// ============================================================================

/// NAM - Set program name
constexpr const char* kNAM = "NAM";

/// TTL - Set listing title
constexpr const char* kTTL = "TTL";

/// STTL - Set subtitle
constexpr const char* kSTTL = "STTL";

/// PAGE - Page eject (start new page in listing)
constexpr const char* kPAGE = "PAGE";

/// SPC - Insert space/blank lines in listing
constexpr const char* kSPC = "SPC";

// ============================================================================
// Conditional Assembly
// ============================================================================

/// IFC - If Condition (begin conditional assembly block)
constexpr const char* kIFC = "IFC";

/// ENDC - End Condition (end conditional assembly block)
constexpr const char* kENDC = "ENDC";

// ============================================================================
// Option Values
// ============================================================================

/// ON - Enable option
constexpr const char* kON = "ON";

/// OFF - Disable option
constexpr const char* kOFF = "OFF";

}  // namespace motorola
}  // namespace xasm
