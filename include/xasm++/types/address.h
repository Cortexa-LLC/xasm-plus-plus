/**
 * @file address.h
 * @brief Type-safe address value object for 6502/65C02/65816 assembly
 *
 * Provides a type-safe wrapper around uint32_t addresses with:
 * - Zero page detection (0x00-0xFF)
 * - Absolute address detection (>= 0x100)
 * - Hex string formatting ($XX, $XXXX, $XXXXXX)
 * - Implicit conversion to uint32_t for compatibility
 *
 * Supports:
 * - 6502/65C02: 16-bit addresses (0x0000-0xFFFF)
 * - 65816: 24-bit addresses (0x000000-0xFFFFFF)
 */

#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace xasm {

/**
 * @class Address
 * @brief Type-safe value object for memory addresses
 *
 * Wraps uint32_t to provide type safety and domain-specific operations
 * for 6502 family assembly addresses.
 *
 * Example usage:
 * @code
 *   Address zp(0x42);
 *   if (zp.is_zero_page()) {
 *       std::cout << "Zero page address: " << zp.to_hex() << "\n";
 *   }
 *
 *   Address abs(0x1234);
 *   uint32_t value = abs;  // Implicit conversion
 * @endcode
 */
class Address {
public:
  /**
   * @brief Construct an Address from a uint32_t value
   * @param value The address value (supports 0x000000-0xFFFFFF)
   */
  explicit constexpr Address(uint32_t value = 0) noexcept : value_(value) {}

  /**
   * @brief Get the raw address value
   * @return The uint32_t address value
   */
  [[nodiscard]] constexpr uint32_t value() const noexcept { return value_; }

  /**
   * @brief Check if address is in zero page (0x00-0xFF)
   * @return true if address is 0x00-0xFF, false otherwise
   *
   * Zero page addresses use faster, shorter instructions on 6502.
   */
  [[nodiscard]] constexpr bool is_zero_page() const noexcept {
    return value_ <= 0xFF;
  }

  /**
   * @brief Check if address is absolute (>= 0x100)
   * @return true if address is >= 0x100, false otherwise
   *
   * Absolute addresses require full 16-bit or 24-bit addressing.
   */
  [[nodiscard]] constexpr bool is_absolute() const noexcept {
    return value_ >= 0x100;
  }

  /**
   * @brief Format address as hex string with $ prefix
   * @return String formatted as "$XX", "$XXXX", or "$XXXXXX"
   *
   * Formatting rules:
   * - 0x00-0xFF: "$XX" (2 hex digits)
   * - 0x0100-0xFFFF: "$XXXX" (4 hex digits)
   * - 0x010000+: "$XXXXXX" (6 hex digits)
   */
  [[nodiscard]] std::string to_hex() const {
    std::ostringstream oss;
    oss << "$" << std::uppercase << std::hex << std::setfill('0');

    if (value_ <= 0xFF) {
      // Zero page: $XX (2 digits)
      oss << std::setw(2) << value_;
    } else if (value_ <= 0xFFFF) {
      // Absolute 16-bit: $XXXX (4 digits)
      oss << std::setw(4) << value_;
    } else {
      // Long 24-bit: $XXXXXX (6 digits)
      oss << std::setw(6) << value_;
    }

    return oss.str();
  }

  /**
   * @brief Implicit conversion to uint32_t
   * @return The raw address value
   *
   * Allows Address to be used seamlessly with existing code
   * that expects uint32_t.
   */
  constexpr operator uint32_t() const noexcept { return value_; }

  // =========================================================================
  // Comparison Operators
  // =========================================================================

  /**
   * @brief Equality comparison
   */
  constexpr bool operator==(const Address &other) const noexcept {
    return value_ == other.value_;
  }

  /**
   * @brief Equality comparison with uint32_t
   */
  constexpr bool operator==(uint32_t other) const noexcept {
    return value_ == other;
  }

  /**
   * @brief Inequality comparison
   */
  constexpr bool operator!=(const Address &other) const noexcept {
    return value_ != other.value_;
  }

  /**
   * @brief Inequality comparison with uint32_t
   */
  constexpr bool operator!=(uint32_t other) const noexcept {
    return value_ != other;
  }

  /**
   * @brief Less-than comparison
   */
  constexpr bool operator<(const Address &other) const noexcept {
    return value_ < other.value_;
  }

  /**
   * @brief Less-than comparison with uint32_t
   */
  constexpr bool operator<(uint32_t other) const noexcept {
    return value_ < other;
  }

  /**
   * @brief Less-than-or-equal comparison
   */
  constexpr bool operator<=(const Address &other) const noexcept {
    return value_ <= other.value_;
  }

  /**
   * @brief Greater-than comparison
   */
  constexpr bool operator>(const Address &other) const noexcept {
    return value_ > other.value_;
  }

  /**
   * @brief Greater-than comparison with uint32_t
   */
  constexpr bool operator>(uint32_t other) const noexcept {
    return value_ > other;
  }

  /**
   * @brief Greater-than-or-equal comparison
   */
  constexpr bool operator>=(const Address &other) const noexcept {
    return value_ >= other.value_;
  }

private:
  uint32_t value_; ///< The raw address value
};

} // namespace xasm
