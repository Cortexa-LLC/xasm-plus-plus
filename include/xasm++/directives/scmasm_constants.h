/**
 * @file scmasm_constants.h
 * @brief SCMASM processing constants
 *
 * Constants for SCMASM character/ASCII processing, data sizes, and limits.
 * Part of Phase 6c.2 - SCMASM handler extraction.
 */

#ifndef XASMPP_DIRECTIVES_SCMASM_CONSTANTS_H
#define XASMPP_DIRECTIVES_SCMASM_CONSTANTS_H

#include <cstddef>
#include <cstdint>

namespace xasm::scmasm::constants {

// High-bit rule constants (for ApplyHighBitRule)
// SCMASM rule: If delimiter ASCII < 0x27 (apostrophe '), high bit is SET
// Otherwise, high bit is CLEAR
constexpr uint8_t kHIGH_BIT_DELIMITER_THRESHOLD = 0x27;  // apostrophe '
constexpr uint8_t kHIGH_BIT_MASK = 0x80;
constexpr uint8_t kLOW_7_BITS_MASK = 0x7F;

// String processing
constexpr uint8_t kNULL_TERMINATOR = 0x00;
constexpr size_t kPASCAL_STRING_MAX_LENGTH = 255;  // Pascal strings: 1-byte length prefix

// Data size constants (in bits)
constexpr size_t kBITS_PER_BYTE = 8;
constexpr size_t kBITS_PER_WORD = 16;
constexpr size_t kBITS_PER_TRIBYTE = 24;
constexpr size_t kBITS_PER_DWORD = 32;

// Bit shift amounts for multi-byte values
constexpr size_t kBYTE_1_SHIFT = 8;   // Second byte position
constexpr size_t kBYTE_2_SHIFT = 16;  // Third byte position
constexpr size_t kBYTE_3_SHIFT = 24;  // Fourth byte position

// Byte masks
constexpr uint32_t kBYTE_MASK = 0xFF;

// Hex parsing constants
constexpr size_t kHEX_DIGITS_PER_BYTE = 2;

// Macro limits
constexpr size_t kMAX_MACRO_DEPTH = 63;

// ASCII Control Characters for escape sequences
namespace ascii {
constexpr uint8_t kNULL_CHAR = 0x00;  // \0
constexpr uint8_t kBELL = 0x07;       // \a
constexpr uint8_t kBACKSPACE = 0x08;  // \b
constexpr uint8_t kTAB = 0x09;        // \t
constexpr uint8_t kNEWLINE = 0x0A;    // \n
constexpr uint8_t kVTAB = 0x0B;       // \v
constexpr uint8_t kFORMFEED = 0x0C;   // \f
constexpr uint8_t kCR = 0x0D;         // \r
constexpr uint8_t kESCAPE = 0x1B;     // \e (non-standard)
}  // namespace ascii

}  // namespace xasm::scmasm::constants

#endif  // XASMPP_DIRECTIVES_SCMASM_CONSTANTS_H
