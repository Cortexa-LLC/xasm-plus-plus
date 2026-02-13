/**
 * @file encoding_utils.h
 * @brief Common encoding utilities for CPU instruction encoding
 *
 * This header provides reusable utilities extracted from CPU implementations:
 * - Endianness conversion (little-endian, big-endian)
 * - Byte and nibble extraction
 * - Range validation
 * - Vector construction helpers (for multi-byte opcodes with prefixes)
 *
 * These utilities eliminate code duplication across CPU families (6502, Z80,
 * 6809) and provide a single source of truth for common encoding patterns.
 *
 * @note All functions are constexpr where possible for compile-time evaluation
 */

#pragma once

#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace xasm {
namespace encoding {

// ============================================================================
// Endianness Conversion
// ============================================================================

/**
 * @brief Convert 16-bit value to little-endian byte array
 *
 * Used by: 6502, Z80
 * Format: [low_byte, high_byte]
 *
 * @param value 16-bit value to convert
 * @return Array with low byte first, high byte second
 *
 * @example
 * auto bytes = ToLittleEndian16(0x1234);
 * // bytes[0] = 0x34, bytes[1] = 0x12
 */
constexpr std::array<uint8_t, 2> ToLittleEndian16(uint16_t value) {
  return {static_cast<uint8_t>(value & 0xFF),         // Low byte
          static_cast<uint8_t>((value >> 8) & 0xFF)}; // High byte
}

/**
 * @brief Convert 16-bit value to big-endian byte array
 *
 * Used by: 6809
 * Format: [high_byte, low_byte]
 *
 * @param value 16-bit value to convert
 * @return Array with high byte first, low byte second
 *
 * @example
 * auto bytes = ToBigEndian16(0x1234);
 * // bytes[0] = 0x12, bytes[1] = 0x34
 */
constexpr std::array<uint8_t, 2> ToBigEndian16(uint16_t value) {
  return {static_cast<uint8_t>((value >> 8) & 0xFF), // High byte
          static_cast<uint8_t>(value & 0xFF)};       // Low byte
}

/**
 * @brief Convert 24-bit value to little-endian byte array
 *
 * Used by: 65816 (extended 6502) for 24-bit addressing
 * Format: [low_byte, middle_byte, high_byte]
 *
 * @param value 32-bit value (upper 8 bits ignored)
 * @return Array with bytes in little-endian order
 *
 * @example
 * auto bytes = ToLittleEndian24(0x123456);
 * // bytes[0] = 0x56, bytes[1] = 0x34, bytes[2] = 0x12
 */
constexpr std::array<uint8_t, 3> ToLittleEndian24(uint32_t value) {
  return {static_cast<uint8_t>(value & 0xFF),          // Low byte
          static_cast<uint8_t>((value >> 8) & 0xFF),   // Middle byte
          static_cast<uint8_t>((value >> 16) & 0xFF)}; // High byte
}

// ============================================================================
// Byte Extraction
// ============================================================================

/**
 * @brief Extract low byte (bits 0-7) from 16-bit value
 *
 * @param value 16-bit value
 * @return Low 8 bits
 *
 * @example
 * LowByte(0x1234) → 0x34
 */
constexpr uint8_t LowByte(uint16_t value) {
  return static_cast<uint8_t>(value & 0xFF);
}

/**
 * @brief Extract high byte (bits 8-15) from 16-bit value
 *
 * @param value 16-bit value
 * @return High 8 bits
 *
 * @example
 * HighByte(0x1234) → 0x12
 */
constexpr uint8_t HighByte(uint16_t value) {
  return static_cast<uint8_t>((value >> 8) & 0xFF);
}

/**
 * @brief Extract low nibble (bits 0-3) from 8-bit value
 *
 * @param value 8-bit value
 * @return Low 4 bits
 *
 * @example
 * LowNibble(0x34) → 0x04
 */
constexpr uint8_t LowNibble(uint8_t value) {
  return static_cast<uint8_t>(value & 0x0F);
}

/**
 * @brief Extract high nibble (bits 4-7) from 8-bit value
 *
 * @param value 8-bit value
 * @return High 4 bits (shifted to low nibble position)
 *
 * @example
 * HighNibble(0x34) → 0x03
 */
constexpr uint8_t HighNibble(uint8_t value) {
  return static_cast<uint8_t>((value >> 4) & 0x0F);
}

// ============================================================================
// Range Validation
// ============================================================================

/**
 * @brief Check if value fits in 8 bits (0-255)
 *
 * @param value Value to check
 * @return true if value ≤ 255, false otherwise
 */
constexpr bool FitsIn8Bits(uint32_t value) { return value <= 0xFF; }

/**
 * @brief Check if value fits in 16 bits (0-65535)
 *
 * @param value Value to check
 * @return true if value ≤ 65535, false otherwise
 */
constexpr bool FitsIn16Bits(uint32_t value) { return value <= 0xFFFF; }

/**
 * @brief Check if value fits in signed 8-bit range (-128 to 127)
 *
 * Used for relative branch offsets
 *
 * @param value Signed value to check
 * @return true if -128 ≤ value ≤ 127, false otherwise
 */
constexpr bool FitsInSignedByte(int32_t value) {
  return (value >= -128) && (value <= 127);
}

// ============================================================================
// Vector Construction Helpers
// ============================================================================

/**
 * @brief Create byte vector with single prefix byte
 *
 * Used for: Z80 CB/DD/FD/ED prefixed instructions
 *
 * @param prefix Prefix byte (e.g., 0xCB for bit operations)
 * @param bytes Opcode and operand bytes
 * @return Vector: [prefix, bytes...]
 *
 * @example
 * // Z80: BIT 0, A → CB 47
 * auto encoded = WithPrefix(0xCB, {0x47});
 */
inline std::vector<uint8_t> WithPrefix(uint8_t prefix,
                                       std::initializer_list<uint8_t> bytes) {
  std::vector<uint8_t> result;
  result.reserve(1 + bytes.size());
  result.push_back(prefix);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

/**
 * @brief Create byte vector with multiple prefix bytes
 *
 * Used for: Z80 indexed bit operations (DD CB, FD CB)
 *
 * @param prefixes Prefix bytes (e.g., {0xDD, 0xCB})
 * @param bytes Opcode and operand bytes
 * @return Vector: [prefixes..., bytes...]
 *
 * @example
 * // Z80: BIT 0, (IX+d) → DD CB d 46
 * auto encoded = WithPrefixes({0xDD, 0xCB}, {displacement, 0x46});
 */
inline std::vector<uint8_t>
WithPrefixes(std::initializer_list<uint8_t> prefixes,
             std::initializer_list<uint8_t> bytes) {
  std::vector<uint8_t> result;
  result.reserve(prefixes.size() + bytes.size());
  result.insert(result.end(), prefixes.begin(), prefixes.end());
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}

} // namespace encoding
} // namespace xasm
