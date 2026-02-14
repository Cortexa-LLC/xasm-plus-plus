/**
 * @file output_format_constants.h
 * @brief Named constants for output format values
 *
 * This file defines all formatting constants used across output plugins,
 * eliminating magic numbers and improving maintainability.
 *
 * @note Part of P1.3: Eliminate magic values in output formatters
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace output_format {

// =============================================================================
// Format Field Widths
// =============================================================================

/// @brief Width for byte values in hex (2 hex digits = 1 byte)
constexpr int HEX_BYTE_WIDTH = 2;

/// @brief Width for 16-bit address in hex (4 hex digits)
constexpr int HEX_ADDRESS_16BIT_WIDTH = 4;

/// @brief Width for line numbers in listings (5 digits)
constexpr int LISTING_LINE_NUMBER_WIDTH = 5;

/// @brief Width for byte display column in listings (17 chars for 8 bytes)
constexpr int LISTING_BYTES_COLUMN_WIDTH = 17;

// =============================================================================
// Bytes Per Line Configuration
// =============================================================================

/// @brief Default bytes per line for Intel HEX format (16 bytes)
constexpr size_t INTEL_HEX_DEFAULT_BYTES_PER_LINE = 16;

/// @brief Default bytes per line for S-Record format (32 bytes)
constexpr size_t SREC_DEFAULT_BYTES_PER_LINE = 32;

/// @brief Maximum bytes to display per line in listing output (8 bytes)
constexpr size_t LISTING_MAX_BYTES_PER_LINE = 8;

/// @brief Maximum bytes per line for any format (255 = max uint8_t)
constexpr size_t MAX_BYTES_PER_LINE = 255;

// =============================================================================
// Fill Characters
// =============================================================================

/// @brief Fill character for hex output (zero padding)
constexpr char HEX_FILL_CHAR = '0';

// =============================================================================
// CoCo DOS (LOADM) Format Constants
// =============================================================================

namespace coco_loadm {

/// @brief Preamble record type for CoCo DOS format
constexpr uint8_t RECORD_TYPE_PREAMBLE = 0x00;

/// @brief Data block record type for CoCo DOS format
constexpr uint8_t RECORD_TYPE_DATA_BLOCK = 0x00;

/// @brief Postamble record type for CoCo DOS format
constexpr uint8_t RECORD_TYPE_POSTAMBLE = 0xFF;

/// @brief Postamble subtype (always 0x00)
constexpr uint8_t POSTAMBLE_SUBTYPE = 0x00;

/// @brief Padding byte for postamble (0x00)
constexpr uint8_t POSTAMBLE_PADDING = 0x00;

/// @brief Maximum address value for CoCo DOS format (16-bit)
constexpr uint64_t MAX_ADDRESS = 0xFFFF;

} // namespace coco_loadm

// =============================================================================
// Intel HEX Format Constants
// =============================================================================

namespace intel_hex {

/// @brief Data record type (00)
constexpr uint8_t RECORD_TYPE_DATA = 0x00;

/// @brief End of File record type (01)
constexpr uint8_t RECORD_TYPE_EOF = 0x01;

/// @brief Extended Linear Address record type (04)
constexpr uint8_t RECORD_TYPE_EXTENDED_LINEAR_ADDRESS = 0x04;

/// @brief Extended Segment Address record type (05) - Not currently used
constexpr uint8_t RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS = 0x05;

/// @brief Address field for EOF record (always 0x0000)
constexpr uint16_t EOF_ADDRESS = 0x0000;

/// @brief Byte count for EOF record (always 0)
constexpr uint8_t EOF_BYTE_COUNT = 0;

/// @brief Address field for extended linear address record
constexpr uint16_t EXTENDED_LINEAR_ADDRESS_FIELD = 0x0000;

/// @brief Byte count for extended linear address record (always 2)
constexpr uint8_t EXTENDED_LINEAR_ADDRESS_BYTE_COUNT = 2;

} // namespace intel_hex

// =============================================================================
// S-Record (Motorola) Format Constants
// =============================================================================

namespace srec {

/// @brief S0 - Header record type
constexpr int RECORD_TYPE_HEADER = 0;

/// @brief S1 - Data record with 16-bit address
constexpr int RECORD_TYPE_DATA_16BIT = 1;

/// @brief S2 - Data record with 24-bit address
constexpr int RECORD_TYPE_DATA_24BIT = 2;

/// @brief S3 - Data record with 32-bit address
constexpr int RECORD_TYPE_DATA_32BIT = 3;

/// @brief S5 - Count record for 16-bit count
constexpr int RECORD_TYPE_COUNT_16BIT = 5;

/// @brief S6 - Count record for 24-bit count
constexpr int RECORD_TYPE_COUNT_24BIT = 6;

/// @brief S7 - Terminator with 32-bit start address
constexpr int RECORD_TYPE_TERM_32BIT = 7;

/// @brief S8 - Terminator with 24-bit start address
constexpr int RECORD_TYPE_TERM_24BIT = 8;

/// @brief S9 - Terminator with 16-bit start address
constexpr int RECORD_TYPE_TERM_16BIT = 9;

/// @brief Address size for 16-bit addresses (2 bytes)
constexpr size_t ADDRESS_SIZE_16BIT = 2;

/// @brief Address size for 24-bit addresses (3 bytes)
constexpr size_t ADDRESS_SIZE_24BIT = 3;

/// @brief Address size for 32-bit addresses (4 bytes)
constexpr size_t ADDRESS_SIZE_32BIT = 4;

/// @brief Threshold for requiring 24-bit addresses (65536)
constexpr uint64_t THRESHOLD_24BIT_ADDRESS = 0x10000;

/// @brief Threshold for requiring 32-bit addresses (16777216)
constexpr uint64_t THRESHOLD_32BIT_ADDRESS = 0x1000000;

/// @brief Threshold for S5 vs S6 count records (65536)
constexpr size_t THRESHOLD_COUNT_24BIT = 0x10000;

/// @brief Checksum byte count (always 1 byte)
constexpr size_t CHECKSUM_BYTE_COUNT = 1;

/// @brief Header address field (always 0x0000)
constexpr uint16_t HEADER_ADDRESS = 0x0000;

/// @brief Terminator base value for calculating terminator type
constexpr int TERMINATOR_BASE = 10;

/// @brief Threshold for using 16-bit addresses (same as 24-bit threshold)
constexpr uint64_t ADDRESS_16BIT_THRESHOLD = 0x10000;

/// @brief Threshold for using 24-bit addresses (same as THRESHOLD_24BIT_ADDRESS)
constexpr uint64_t ADDRESS_24BIT_THRESHOLD = 0x1000000;

/// @brief Threshold for using 16-bit count (same as THRESHOLD_COUNT_24BIT)
constexpr size_t COUNT_16BIT_THRESHOLD = 0x10000;

} // namespace srec

// =============================================================================
// Bit Manipulation Constants
// =============================================================================

namespace bit_ops {

/// @brief Bit shift for high byte of 16-bit value (8 bits)
constexpr int SHIFT_HIGH_BYTE = 8;

/// @brief Bit shift for upper 16 bits of 32-bit value (16 bits)
constexpr int SHIFT_UPPER_WORD = 16;

/// @brief Mask for low byte (0xFF)
constexpr uint8_t MASK_LOW_BYTE = 0xFF;

/// @brief Mask for low 16 bits (0xFFFF)
constexpr uint16_t MASK_LOW_WORD = 0xFFFF;

} // namespace bit_ops

// =============================================================================
// Binary Output Constants
// =============================================================================

namespace binary {

/// @brief Zero byte for padding/initialization (0x00)
constexpr uint8_t ZERO_BYTE = 0x00;

} // namespace binary

} // namespace output_format
} // namespace xasm
