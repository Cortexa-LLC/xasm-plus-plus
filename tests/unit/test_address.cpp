/**
 * @file test_address.cpp
 * @brief Unit tests for Address value object
 *
 * Tests type-safe address handling, validation, and utility methods.
 */

#include "xasm++/types/address.h"
#include <gtest/gtest.h>

using namespace xasm;

// =============================================================================
// RED Phase: Construction and Basic Properties
// =============================================================================

/**
 * Test: Address should construct from uint32_t
 * Expected: Address wraps the value and provides type safety
 */
TEST(AddressTest, ConstructFromUint32) {
  Address addr(0x1234);
  EXPECT_EQ(addr.value(), 0x1234u);
}

/**
 * Test: Address should construct from zero
 * Expected: Zero address is valid
 */
TEST(AddressTest, ConstructFromZero) {
  Address addr(0);
  EXPECT_EQ(addr.value(), 0u);
}

/**
 * Test: Address should construct from maximum 16-bit value
 * Expected: 0xFFFF is valid for 6502 (64KB address space)
 */
TEST(AddressTest, ConstructFromMaxUint16) {
  Address addr(0xFFFF);
  EXPECT_EQ(addr.value(), 0xFFFFu);
}

/**
 * Test: Address should construct from 24-bit value (65816)
 * Expected: 24-bit addresses supported for 65816 long addressing
 */
TEST(AddressTest, ConstructFrom24Bit) {
  Address addr(0x123456);
  EXPECT_EQ(addr.value(), 0x123456u);
}

// =============================================================================
// RED Phase: Zero Page Detection
// =============================================================================

/**
 * Test: is_zero_page() should return true for addresses 0x00-0xFF
 * Expected: Zero page is first 256 bytes (0x00-0xFF)
 */
TEST(AddressTest, IsZeroPageTrue) {
  EXPECT_TRUE(Address(0x00).is_zero_page());
  EXPECT_TRUE(Address(0x80).is_zero_page());
  EXPECT_TRUE(Address(0xFF).is_zero_page());
}

/**
 * Test: is_zero_page() should return false for addresses >= 0x100
 * Expected: Addresses outside zero page range
 */
TEST(AddressTest, IsZeroPageFalse) {
  EXPECT_FALSE(Address(0x100).is_zero_page());
  EXPECT_FALSE(Address(0x1234).is_zero_page());
  EXPECT_FALSE(Address(0xFFFF).is_zero_page());
}

// =============================================================================
// RED Phase: Absolute Address Detection
// =============================================================================

/**
 * Test: is_absolute() should return true for 16-bit addresses
 * Expected: Addresses 0x0100-0xFFFF are absolute
 */
TEST(AddressTest, IsAbsoluteTrue) {
  EXPECT_TRUE(Address(0x100).is_absolute());
  EXPECT_TRUE(Address(0x1234).is_absolute());
  EXPECT_TRUE(Address(0xFFFF).is_absolute());
}

/**
 * Test: is_absolute() should return false for zero page
 * Expected: Zero page addresses (0x00-0xFF) are not absolute
 */
TEST(AddressTest, IsAbsoluteFalseForZeroPage) {
  EXPECT_FALSE(Address(0x00).is_absolute());
  EXPECT_FALSE(Address(0x80).is_absolute());
  EXPECT_FALSE(Address(0xFF).is_absolute());
}

/**
 * Test: is_absolute() should return true for 24-bit addresses
 * Expected: 65816 long addresses (> 0xFFFF) are absolute
 */
TEST(AddressTest, IsAbsoluteTrueFor24Bit) {
  EXPECT_TRUE(Address(0x10000).is_absolute());
  EXPECT_TRUE(Address(0x123456).is_absolute());
}

// =============================================================================
// RED Phase: Hex String Conversion
// =============================================================================

/**
 * Test: to_hex() should format 8-bit addresses as $XX
 * Expected: Zero page addresses formatted with 2 hex digits
 */
TEST(AddressTest, ToHexZeroPage) {
  EXPECT_EQ(Address(0x00).to_hex(), "$00");
  EXPECT_EQ(Address(0x42).to_hex(), "$42");
  EXPECT_EQ(Address(0xFF).to_hex(), "$FF");
}

/**
 * Test: to_hex() should format 16-bit addresses as $XXXX
 * Expected: Absolute addresses formatted with 4 hex digits
 */
TEST(AddressTest, ToHexAbsolute) {
  EXPECT_EQ(Address(0x1234).to_hex(), "$1234");
  EXPECT_EQ(Address(0xFFFF).to_hex(), "$FFFF");
  EXPECT_EQ(Address(0x0100).to_hex(), "$0100");
}

/**
 * Test: to_hex() should format 24-bit addresses as $XXXXXX
 * Expected: 65816 long addresses formatted with 6 hex digits
 */
TEST(AddressTest, ToHex24Bit) {
  EXPECT_EQ(Address(0x123456).to_hex(), "$123456");
  EXPECT_EQ(Address(0x010000).to_hex(), "$010000");
}

// =============================================================================
// RED Phase: Implicit Conversion to uint32_t
// =============================================================================

/**
 * Test: Address should implicitly convert to uint32_t
 * Expected: Seamless compatibility with existing code
 */
TEST(AddressTest, ImplicitConversionToUint32) {
  Address addr(0x1234);
  uint32_t value = addr; // Implicit conversion
  EXPECT_EQ(value, 0x1234u);
}

/**
 * Test: Address should work in arithmetic expressions
 * Expected: Can use Address like uint32_t in calculations
 */
TEST(AddressTest, UseInArithmetic) {
  Address addr(0x1000);
  uint32_t offset = 0x20;
  uint32_t result = addr + offset;
  EXPECT_EQ(result, 0x1020u);
}

/**
 * Test: Address should work in comparisons
 * Expected: Can compare Address values directly
 */
TEST(AddressTest, UseInComparison) {
  Address addr1(0x1000);
  Address addr2(0x2000);
  uint32_t value = 0x1000;

  EXPECT_TRUE(addr1 < addr2);
  EXPECT_TRUE(addr1 == value);
  EXPECT_FALSE(addr1 > addr2);
}

// =============================================================================
// RED Phase: Copy and Assignment
// =============================================================================

/**
 * Test: Address should be copyable
 * Expected: Copy constructor creates independent copy
 */
TEST(AddressTest, CopyConstruction) {
  Address addr1(0x1234);
  Address addr2(addr1);
  EXPECT_EQ(addr2.value(), 0x1234u);
}

/**
 * Test: Address should be assignable
 * Expected: Copy assignment works correctly
 */
TEST(AddressTest, CopyAssignment) {
  Address addr1(0x1234);
  Address addr2(0x5678);
  addr2 = addr1;
  EXPECT_EQ(addr2.value(), 0x1234u);
}

// =============================================================================
// RED Phase: Equality and Inequality
// =============================================================================

/**
 * Test: Address equality operator
 * Expected: Two addresses with same value are equal
 */
TEST(AddressTest, Equality) {
  Address addr1(0x1234);
  Address addr2(0x1234);
  Address addr3(0x5678);

  EXPECT_TRUE(addr1 == addr2);
  EXPECT_FALSE(addr1 == addr3);
}

/**
 * Test: Address inequality operator
 * Expected: Two addresses with different values are not equal
 */
TEST(AddressTest, Inequality) {
  Address addr1(0x1234);
  Address addr2(0x5678);

  EXPECT_TRUE(addr1 != addr2);
  EXPECT_FALSE(addr1 != addr1);
}

// =============================================================================
// Main Entry Point
// =============================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
