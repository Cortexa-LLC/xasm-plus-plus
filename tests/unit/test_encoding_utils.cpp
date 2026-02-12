/**
 * @file test_encoding_utils.cpp
 * @brief Unit tests for common encoding utilities
 *
 * Tests for encoding utilities extracted from CPU implementations:
 * - Endianness conversion (little-endian, big-endian)
 * - Byte extraction (low byte, high byte, nibbles)
 * - Range validation
 * - Vector helpers (prefix construction)
 */

#include <gtest/gtest.h>
#include "xasm++/cpu/encoding_utils.h"
#include <array>
#include <vector>

using namespace xasm::encoding;

// ============================================================================
// Endianness Conversion Tests
// ============================================================================

TEST(EncodingUtilsTest, ToLittleEndian16_StandardValue) {
  auto result = ToLittleEndian16(0x1234);
  EXPECT_EQ(result[0], 0x34);  // Low byte first
  EXPECT_EQ(result[1], 0x12);  // High byte second
}

TEST(EncodingUtilsTest, ToLittleEndian16_EdgeCase_0x0000) {
  auto result = ToLittleEndian16(0x0000);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x00);
}

TEST(EncodingUtilsTest, ToLittleEndian16_EdgeCase_0xFFFF) {
  auto result = ToLittleEndian16(0xFFFF);
  EXPECT_EQ(result[0], 0xFF);
  EXPECT_EQ(result[1], 0xFF);
}

TEST(EncodingUtilsTest, ToLittleEndian16_EdgeCase_0x0001) {
  auto result = ToLittleEndian16(0x0001);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x00);
}

TEST(EncodingUtilsTest, ToLittleEndian16_EdgeCase_0xFF00) {
  auto result = ToLittleEndian16(0xFF00);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0xFF);
}

TEST(EncodingUtilsTest, ToBigEndian16_StandardValue) {
  auto result = ToBigEndian16(0x1234);
  EXPECT_EQ(result[0], 0x12);  // High byte first
  EXPECT_EQ(result[1], 0x34);  // Low byte second
}

TEST(EncodingUtilsTest, ToBigEndian16_EdgeCase_0x0000) {
  auto result = ToBigEndian16(0x0000);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x00);
}

TEST(EncodingUtilsTest, ToBigEndian16_EdgeCase_0xFFFF) {
  auto result = ToBigEndian16(0xFFFF);
  EXPECT_EQ(result[0], 0xFF);
  EXPECT_EQ(result[1], 0xFF);
}

TEST(EncodingUtilsTest, ToBigEndian16_EdgeCase_0x0001) {
  auto result = ToBigEndian16(0x0001);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x01);
}

TEST(EncodingUtilsTest, ToBigEndian16_EdgeCase_0xFF00) {
  auto result = ToBigEndian16(0xFF00);
  EXPECT_EQ(result[0], 0xFF);
  EXPECT_EQ(result[1], 0x00);
}

TEST(EncodingUtilsTest, ToLittleEndian24_StandardValue) {
  auto result = ToLittleEndian24(0x123456);
  EXPECT_EQ(result[0], 0x56);  // Low byte
  EXPECT_EQ(result[1], 0x34);  // Middle byte
  EXPECT_EQ(result[2], 0x12);  // High byte
}

TEST(EncodingUtilsTest, ToLittleEndian24_EdgeCase_0x000000) {
  auto result = ToLittleEndian24(0x000000);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x00);
  EXPECT_EQ(result[2], 0x00);
}

TEST(EncodingUtilsTest, ToLittleEndian24_EdgeCase_0xFFFFFF) {
  auto result = ToLittleEndian24(0xFFFFFF);
  EXPECT_EQ(result[0], 0xFF);
  EXPECT_EQ(result[1], 0xFF);
  EXPECT_EQ(result[2], 0xFF);
}

TEST(EncodingUtilsTest, ToLittleEndian24_TruncatesHighByte) {
  auto result = ToLittleEndian24(0xAB123456);
  EXPECT_EQ(result[0], 0x56);  // Low byte
  EXPECT_EQ(result[1], 0x34);  // Middle byte
  EXPECT_EQ(result[2], 0x12);  // High byte (0xAB truncated)
}

// ============================================================================
// Byte Extraction Tests
// ============================================================================

TEST(EncodingUtilsTest, LowByte_ExtractsLowBits) {
  EXPECT_EQ(LowByte(0x1234), 0x34);
  EXPECT_EQ(LowByte(0x0000), 0x00);
  EXPECT_EQ(LowByte(0xFFFF), 0xFF);
  EXPECT_EQ(LowByte(0x0001), 0x01);
  EXPECT_EQ(LowByte(0xFF00), 0x00);
}

TEST(EncodingUtilsTest, HighByte_ExtractsHighBits) {
  EXPECT_EQ(HighByte(0x1234), 0x12);
  EXPECT_EQ(HighByte(0x0000), 0x00);
  EXPECT_EQ(HighByte(0xFFFF), 0xFF);
  EXPECT_EQ(HighByte(0x0001), 0x00);
  EXPECT_EQ(HighByte(0xFF00), 0xFF);
}

TEST(EncodingUtilsTest, LowNibble_ExtractsLowNibble) {
  EXPECT_EQ(LowNibble(0x34), 0x04);
  EXPECT_EQ(LowNibble(0x00), 0x00);
  EXPECT_EQ(LowNibble(0xFF), 0x0F);
  EXPECT_EQ(LowNibble(0xF0), 0x00);
  EXPECT_EQ(LowNibble(0x0F), 0x0F);
}

TEST(EncodingUtilsTest, HighNibble_ExtractsHighNibble) {
  EXPECT_EQ(HighNibble(0x34), 0x03);
  EXPECT_EQ(HighNibble(0x00), 0x00);
  EXPECT_EQ(HighNibble(0xFF), 0x0F);
  EXPECT_EQ(HighNibble(0xF0), 0x0F);
  EXPECT_EQ(HighNibble(0x0F), 0x00);
}

// ============================================================================
// Range Validation Tests
// ============================================================================

TEST(EncodingUtilsTest, FitsIn8Bits_ValidatesRange) {
  EXPECT_TRUE(FitsIn8Bits(0));
  EXPECT_TRUE(FitsIn8Bits(127));
  EXPECT_TRUE(FitsIn8Bits(255));
  EXPECT_FALSE(FitsIn8Bits(256));
  EXPECT_FALSE(FitsIn8Bits(0xFFFFFFFF));
}

TEST(EncodingUtilsTest, FitsIn16Bits_ValidatesRange) {
  EXPECT_TRUE(FitsIn16Bits(0));
  EXPECT_TRUE(FitsIn16Bits(32767));
  EXPECT_TRUE(FitsIn16Bits(65535));
  EXPECT_FALSE(FitsIn16Bits(65536));
  EXPECT_FALSE(FitsIn16Bits(0xFFFFFFFF));
}

TEST(EncodingUtilsTest, FitsInSignedByte_ValidatesSignedRange) {
  EXPECT_TRUE(FitsInSignedByte(-128));
  EXPECT_TRUE(FitsInSignedByte(-1));
  EXPECT_TRUE(FitsInSignedByte(0));
  EXPECT_TRUE(FitsInSignedByte(127));
  EXPECT_FALSE(FitsInSignedByte(128));
  EXPECT_FALSE(FitsInSignedByte(-129));
}

// ============================================================================
// Vector Helper Tests
// ============================================================================

TEST(EncodingUtilsTest, WithPrefix_SingleByte) {
  auto result = WithPrefix(0xCB, {0x40});
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0xCB);
  EXPECT_EQ(result[1], 0x40);
}

TEST(EncodingUtilsTest, WithPrefix_MultipleBytes) {
  auto result = WithPrefix(0xDD, {0x21, 0x00, 0x10});
  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 0xDD);
  EXPECT_EQ(result[1], 0x21);
  EXPECT_EQ(result[2], 0x00);
  EXPECT_EQ(result[3], 0x10);
}

TEST(EncodingUtilsTest, WithPrefix_EmptyBytes) {
  auto result = WithPrefix(0xED, {});
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0xED);
}

TEST(EncodingUtilsTest, WithPrefixes_TwoPrefixes) {
  auto result = WithPrefixes({0xDD, 0xCB}, {0x40});
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0xDD);
  EXPECT_EQ(result[1], 0xCB);
  EXPECT_EQ(result[2], 0x40);
}

TEST(EncodingUtilsTest, WithPrefixes_OnePrefixMultipleBytes) {
  auto result = WithPrefixes({0xED}, {0xB0, 0x00});
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0xED);
  EXPECT_EQ(result[1], 0xB0);
  EXPECT_EQ(result[2], 0x00);
}

TEST(EncodingUtilsTest, WithPrefixes_EmptyPrefixes) {
  auto result = WithPrefixes({}, {0x00, 0x01});
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x01);
}

TEST(EncodingUtilsTest, WithPrefixes_BothEmpty) {
  auto result = WithPrefixes({}, {});
  EXPECT_EQ(result.size(), 0);
}
