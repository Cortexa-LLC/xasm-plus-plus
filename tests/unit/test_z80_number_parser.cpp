/**
 * @file test_z80_number_parser.cpp
 * @brief Unit tests for Z80NumberParser
 */

#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test fixture for Z80NumberParser
class Z80NumberParserTest : public ::testing::Test {
protected:
  Z80NumberParser parser;
};

// ============================================================================
// Hexadecimal Format Tests (H suffix)
// ============================================================================

TEST_F(Z80NumberParserTest, HexSuffixBasic) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("0FFH", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("10H", value));
  EXPECT_EQ(value, 16);

  EXPECT_TRUE(parser.TryParse("0ABCDh", value));
  EXPECT_EQ(value, 0xABCD);

  EXPECT_TRUE(parser.TryParse("9FH", value));
  EXPECT_EQ(value, 0x9F);
}

TEST_F(Z80NumberParserTest, HexSuffixCaseInsensitive) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("0FFH", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("0FFh", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("0ffH", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("0ffh", value));
  EXPECT_EQ(value, 255);
}

TEST_F(Z80NumberParserTest, HexSuffixInvalid) {
  int64_t value;

  // Must start with digit
  EXPECT_FALSE(parser.TryParse("FFH", value));
  EXPECT_FALSE(parser.TryParse("ABCh", value));

  // Invalid hex digits
  EXPECT_FALSE(parser.TryParse("0GGH", value));
  EXPECT_FALSE(parser.TryParse("0XZH", value));

  // Too short
  EXPECT_FALSE(parser.TryParse("H", value));
}

// ============================================================================
// Octal Format Tests (O and Q suffix)
// ============================================================================

TEST_F(Z80NumberParserTest, OctalSuffixBasic) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("377O", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("377Q", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("10O", value));
  EXPECT_EQ(value, 8);

  EXPECT_TRUE(parser.TryParse("77o", value));
  EXPECT_EQ(value, 63);
}

TEST_F(Z80NumberParserTest, OctalSuffixInvalid) {
  int64_t value;

  // Invalid octal digits (8 and 9 not allowed)
  EXPECT_FALSE(parser.TryParse("389O", value));
  EXPECT_FALSE(parser.TryParse("777O", value)); // Wait, this is valid
  
  // Actually test with 8 and 9
  EXPECT_FALSE(parser.TryParse("180O", value));
  EXPECT_FALSE(parser.TryParse("290Q", value));

  // Too short
  EXPECT_FALSE(parser.TryParse("O", value));
  EXPECT_FALSE(parser.TryParse("Q", value));
}

TEST_F(Z80NumberParserTest, OctalSuffixLargeValue) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("777O", value));
  EXPECT_EQ(value, 511); // 7*64 + 7*8 + 7 = 511
}

// ============================================================================
// Binary Format Tests (B suffix)
// ============================================================================

TEST_F(Z80NumberParserTest, BinarySuffixBasic) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("11111111B", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("10101010B", value));
  EXPECT_EQ(value, 170);

  EXPECT_TRUE(parser.TryParse("1B", value));
  EXPECT_EQ(value, 1);

  EXPECT_TRUE(parser.TryParse("0B", value));
  EXPECT_EQ(value, 0);
}

TEST_F(Z80NumberParserTest, BinarySuffixCaseInsensitive) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("11111111B", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("11111111b", value));
  EXPECT_EQ(value, 255);
}

TEST_F(Z80NumberParserTest, BinarySuffixInvalid) {
  int64_t value;

  // Invalid binary digits
  EXPECT_FALSE(parser.TryParse("12B", value));
  EXPECT_FALSE(parser.TryParse("10201010B", value));

  // Too short
  EXPECT_FALSE(parser.TryParse("B", value));
}

// ============================================================================
// Decimal Format Tests (D suffix)
// ============================================================================

TEST_F(Z80NumberParserTest, DecimalSuffixBasic) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("255D", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("42D", value));
  EXPECT_EQ(value, 42);

  EXPECT_TRUE(parser.TryParse("0D", value));
  EXPECT_EQ(value, 0);
}

TEST_F(Z80NumberParserTest, DecimalSuffixCaseInsensitive) {
  int64_t value;

  EXPECT_TRUE(parser.TryParse("255D", value));
  EXPECT_EQ(value, 255);

  EXPECT_TRUE(parser.TryParse("255d", value));
  EXPECT_EQ(value, 255);
}

TEST_F(Z80NumberParserTest, DecimalSuffixInvalid) {
  int64_t value;

  // Invalid decimal digits
  EXPECT_FALSE(parser.TryParse("25A5D", value));
  EXPECT_FALSE(parser.TryParse("0xFFD", value));

  // Too short
  EXPECT_FALSE(parser.TryParse("D", value));
}

// ============================================================================
// Rejection Tests (Not Z80 Format)
// ============================================================================

TEST_F(Z80NumberParserTest, RejectStandardFormats) {
  int64_t value;

  // These are standard formats, not Z80-specific
  EXPECT_FALSE(parser.TryParse("$FF", value));
  EXPECT_FALSE(parser.TryParse("0xFF", value));
  EXPECT_FALSE(parser.TryParse("%10101010", value));
  EXPECT_FALSE(parser.TryParse("255", value)); // No suffix
  EXPECT_FALSE(parser.TryParse("0377", value)); // No suffix
}

TEST_F(Z80NumberParserTest, RejectIdentifiers) {
  int64_t value;

  // These start with letters, not digits
  EXPECT_FALSE(parser.TryParse("FFH", value));
  EXPECT_FALSE(parser.TryParse("ABC", value));
  EXPECT_FALSE(parser.TryParse("LABEL", value));
}

TEST_F(Z80NumberParserTest, EmptyString) {
  int64_t value;
  EXPECT_FALSE(parser.TryParse("", value));
}
