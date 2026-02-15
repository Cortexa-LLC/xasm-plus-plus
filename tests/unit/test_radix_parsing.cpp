// Unit tests for radix parsing utilities
// Tests ParseBinary, ParseDecimal, ParseOctal, and ParseHexDigit functions

#include "xasm++/parse_utils.h"
#include <gtest/gtest.h>
#include <stdexcept>

// ============================================================================
// ParseBinary Tests
// ============================================================================

TEST(ParseBinaryTest, ValidBinaryString) {
  EXPECT_EQ(xasm::ParseBinary("1010"), 10u);
  EXPECT_EQ(xasm::ParseBinary("11111111"), 255u);
  EXPECT_EQ(xasm::ParseBinary("1"), 1u);
  EXPECT_EQ(xasm::ParseBinary("0"), 0u);
}

TEST(ParseBinaryTest, EmptyString) {
  EXPECT_THROW(xasm::ParseBinary(""), std::invalid_argument);
}

TEST(ParseBinaryTest, InvalidCharacters) {
  EXPECT_THROW(xasm::ParseBinary("102"), std::invalid_argument);
  EXPECT_THROW(xasm::ParseBinary("1a1"), std::invalid_argument);
  EXPECT_THROW(xasm::ParseBinary("10 10"), std::invalid_argument);
}

TEST(ParseBinaryTest, LeadingZeros) {
  EXPECT_EQ(xasm::ParseBinary("00001010"), 10u);
  EXPECT_EQ(xasm::ParseBinary("000"), 0u);
}

// ============================================================================
// ParseDecimal Tests
// ============================================================================

TEST(ParseDecimalTest, ValidDecimalString) {
  EXPECT_EQ(xasm::ParseDecimal("123"), 123u);
  EXPECT_EQ(xasm::ParseDecimal("0"), 0u);
  EXPECT_EQ(xasm::ParseDecimal("9999"), 9999u);
  EXPECT_EQ(xasm::ParseDecimal("42"), 42u);
}

TEST(ParseDecimalTest, EmptyString) {
  EXPECT_THROW(xasm::ParseDecimal(""), std::invalid_argument);
}

TEST(ParseDecimalTest, InvalidCharacters) {
  EXPECT_THROW(xasm::ParseDecimal("12a3"), std::invalid_argument);
  EXPECT_THROW(xasm::ParseDecimal("12.3"), std::invalid_argument);
  EXPECT_THROW(xasm::ParseDecimal("12 3"), std::invalid_argument);
}

TEST(ParseDecimalTest, LeadingZeros) {
  EXPECT_EQ(xasm::ParseDecimal("00123"), 123u);
  EXPECT_EQ(xasm::ParseDecimal("000"), 0u);
}

// ============================================================================
// ParseOctal Tests
// ============================================================================

TEST(ParseOctalTest, ValidOctalString) {
  EXPECT_EQ(xasm::ParseOctal("10"), 8u);    // 10 octal = 8 decimal
  EXPECT_EQ(xasm::ParseOctal("77"), 63u);   // 77 octal = 63 decimal
  EXPECT_EQ(xasm::ParseOctal("377"), 255u); // 377 octal = 255 decimal
  EXPECT_EQ(xasm::ParseOctal("0"), 0u);
}

TEST(ParseOctalTest, EmptyString) {
  EXPECT_THROW(xasm::ParseOctal(""), std::invalid_argument);
}

TEST(ParseOctalTest, InvalidCharacters) {
  EXPECT_THROW(xasm::ParseOctal("89"),
               std::invalid_argument); // 8 and 9 not valid in octal
  EXPECT_THROW(xasm::ParseOctal("12a"), std::invalid_argument);
  EXPECT_THROW(xasm::ParseOctal("1 2"), std::invalid_argument);
}

TEST(ParseOctalTest, LeadingZeros) {
  EXPECT_EQ(xasm::ParseOctal("0010"), 8u);
  EXPECT_EQ(xasm::ParseOctal("000"), 0u);
}

// ============================================================================
// ParseBinaryDigit Tests
// ============================================================================

TEST(ParseBinaryDigitTest, ValidBinaryDigits) {
  int digit;

  EXPECT_TRUE(xasm::ParseBinaryDigit('0', digit));
  EXPECT_EQ(digit, 0);

  EXPECT_TRUE(xasm::ParseBinaryDigit('1', digit));
  EXPECT_EQ(digit, 1);
}

TEST(ParseBinaryDigitTest, InvalidCharacters) {
  int digit;

  EXPECT_FALSE(xasm::ParseBinaryDigit('2', digit));
  EXPECT_FALSE(xasm::ParseBinaryDigit('9', digit));
  EXPECT_FALSE(xasm::ParseBinaryDigit('a', digit));
  EXPECT_FALSE(xasm::ParseBinaryDigit(' ', digit));
}

// ============================================================================
// ParseDecimalDigit Tests
// ============================================================================

TEST(ParseDecimalDigitTest, ValidDecimalDigits) {
  int digit;

  EXPECT_TRUE(xasm::ParseDecimalDigit('0', digit));
  EXPECT_EQ(digit, 0);

  EXPECT_TRUE(xasm::ParseDecimalDigit('5', digit));
  EXPECT_EQ(digit, 5);

  EXPECT_TRUE(xasm::ParseDecimalDigit('9', digit));
  EXPECT_EQ(digit, 9);
}

TEST(ParseDecimalDigitTest, InvalidCharacters) {
  int digit;

  EXPECT_FALSE(xasm::ParseDecimalDigit('a', digit));
  EXPECT_FALSE(xasm::ParseDecimalDigit('f', digit));
  EXPECT_FALSE(xasm::ParseDecimalDigit(' ', digit));
  EXPECT_FALSE(xasm::ParseDecimalDigit('.', digit));
}

// ============================================================================
// ParseOctalDigit Tests
// ============================================================================

TEST(ParseOctalDigitTest, ValidOctalDigits) {
  int digit;

  EXPECT_TRUE(xasm::ParseOctalDigit('0', digit));
  EXPECT_EQ(digit, 0);

  EXPECT_TRUE(xasm::ParseOctalDigit('3', digit));
  EXPECT_EQ(digit, 3);

  EXPECT_TRUE(xasm::ParseOctalDigit('7', digit));
  EXPECT_EQ(digit, 7);
}

TEST(ParseOctalDigitTest, InvalidCharacters) {
  int digit;

  EXPECT_FALSE(xasm::ParseOctalDigit('8', digit));
  EXPECT_FALSE(xasm::ParseOctalDigit('9', digit));
  EXPECT_FALSE(xasm::ParseOctalDigit('a', digit));
  EXPECT_FALSE(xasm::ParseOctalDigit(' ', digit));
}

// ============================================================================
// ParseHexDigit Tests
// ============================================================================

TEST(ParseHexDigitTest, ValidHexDigits) {
  int digit;

  // Test digits 0-9
  EXPECT_TRUE(xasm::ParseHexDigit('0', digit));
  EXPECT_EQ(digit, 0);

  EXPECT_TRUE(xasm::ParseHexDigit('5', digit));
  EXPECT_EQ(digit, 5);

  EXPECT_TRUE(xasm::ParseHexDigit('9', digit));
  EXPECT_EQ(digit, 9);

  // Test lowercase a-f
  EXPECT_TRUE(xasm::ParseHexDigit('a', digit));
  EXPECT_EQ(digit, 10);

  EXPECT_TRUE(xasm::ParseHexDigit('f', digit));
  EXPECT_EQ(digit, 15);

  // Test uppercase A-F
  EXPECT_TRUE(xasm::ParseHexDigit('A', digit));
  EXPECT_EQ(digit, 10);

  EXPECT_TRUE(xasm::ParseHexDigit('F', digit));
  EXPECT_EQ(digit, 15);
}

TEST(ParseHexDigitTest, InvalidCharacters) {
  int digit;

  EXPECT_FALSE(xasm::ParseHexDigit('g', digit));
  EXPECT_FALSE(xasm::ParseHexDigit('G', digit));
  EXPECT_FALSE(xasm::ParseHexDigit('z', digit));
  EXPECT_FALSE(xasm::ParseHexDigit(' ', digit));
  EXPECT_FALSE(xasm::ParseHexDigit('.', digit));
}
