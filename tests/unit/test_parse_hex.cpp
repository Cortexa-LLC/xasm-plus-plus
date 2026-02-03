// ParseHex security and validation tests
// Tests for [C1] Critical Issue: ParseHex() Security Risk

#include <gtest/gtest.h>
#include <stdexcept>
#include <cstdint>
#include "xasm++/parse_utils.h"

namespace xasm {

// ParseHexSafe is already declared in parse_utils.h
// ParseHex (exception-throwing variant) will be added

// ============================================================================
// ============================================================================

// Test 1: Empty string should fail
TEST(ParseHexTest, EmptyString) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(error.empty());
}

// Test 2: Missing $ prefix should fail
TEST(ParseHexTest, MissingDollarSign) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("1234", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(error.empty());
}

// Test 3: Invalid hex characters should fail
TEST(ParseHexTest, InvalidHexCharacters) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$12XZ", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(error.empty());
}

// Test 4: Non-hex characters should fail
TEST(ParseHexTest, NonHexCharacters) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$GHIJ", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(error.empty());
}

// Test 5: Overflow should fail
TEST(ParseHexTest, ValueOverflow) {
    bool success = false;
    std::string error;
    
    // Value larger than uint32_t max (FFFFFFFF + 1)
    ParseHexSafe("$100000000", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(error.empty());
}

// Test 6: Just $ with no digits should fail
TEST(ParseHexTest, OnlyDollarSign) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(error.empty());
}

// Test 7: Whitespace should fail (not trimmed)
TEST(ParseHexTest, WhitespaceNotAllowed) {
    bool success = false;
    std::string error;
    
    ParseHexSafe("$ 1234", success, error);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(error.empty());
}

// ============================================================================
// ============================================================================

// Test 8: Valid single byte hex
TEST(ParseHexTest, ValidSingleByte) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$42", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0x42);
    EXPECT_TRUE(error.empty());
}

// Test 9: Valid two-byte hex (little endian)
TEST(ParseHexTest, ValidTwoBytes) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$1234", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0x1234);
    EXPECT_TRUE(error.empty());
}

// Test 10: Valid four-byte hex
TEST(ParseHexTest, ValidFourBytes) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$ABCD1234", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0xABCD1234);
    EXPECT_TRUE(error.empty());
}

// Test 11: Lowercase hex digits
TEST(ParseHexTest, LowercaseHexDigits) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$abcd", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0xABCD);
    EXPECT_TRUE(error.empty());
}

// Test 12: Mixed case hex digits
TEST(ParseHexTest, MixedCaseHexDigits) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$AbCd", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0xABCD);
    EXPECT_TRUE(error.empty());
}

// Test 13: Zero value
TEST(ParseHexTest, ZeroValue) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$0", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(error.empty());
}

// Test 14: Maximum uint32_t value
TEST(ParseHexTest, MaxUint32Value) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$FFFFFFFF", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0xFFFFFFFF);
    EXPECT_TRUE(error.empty());
}

// Test 15: Leading zeros (should be valid)
TEST(ParseHexTest, LeadingZeros) {
    bool success = false;
    std::string error;
    
    uint32_t result = ParseHexSafe("$0042", success, error);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result, 0x42);
    EXPECT_TRUE(error.empty());
}

// ============================================================================
// ParseHex (Exception-Throwing Variant) Tests
// ============================================================================

// Test: ParseHex with $ prefix
TEST(ParseHexExceptionTest, WithDollarPrefix) {
    EXPECT_EQ(0x1234, ParseHex("$1234"));
    EXPECT_EQ(0xFF, ParseHex("$FF"));
    EXPECT_EQ(0xABCD, ParseHex("$ABCD"));
}

// Test: ParseHex without $ prefix (should work)
TEST(ParseHexExceptionTest, WithoutDollarPrefix) {
    EXPECT_EQ(0x1234, ParseHex("1234"));
    EXPECT_EQ(0xFF, ParseHex("FF"));
    EXPECT_EQ(0xABCD, ParseHex("ABCD"));
}

// Test: ParseHex strips addressing mode suffixes
TEST(ParseHexExceptionTest, StripAddressingModeSuffixes) {
    EXPECT_EQ(0x10, ParseHex("$10,X"));
    EXPECT_EQ(0x20, ParseHex("$20,Y"));
    EXPECT_EQ(0x30, ParseHex("$30,S"));
    EXPECT_EQ(0x1234, ParseHex("$1234,X"));
}

// Test: ParseHex throws on empty string
TEST(ParseHexExceptionTest, ThrowsOnEmptyString) {
    EXPECT_THROW(ParseHex(""), std::invalid_argument);
}

// Test: ParseHex throws on invalid hex characters
TEST(ParseHexExceptionTest, ThrowsOnInvalidHexCharacters) {
    EXPECT_THROW(ParseHex("$GHIJ"), std::invalid_argument);
    EXPECT_THROW(ParseHex("$12XZ"), std::invalid_argument);
    EXPECT_THROW(ParseHex("GHIJ"), std::invalid_argument);
}

// Test: ParseHex throws on only $ prefix
TEST(ParseHexExceptionTest, ThrowsOnOnlyDollarSign) {
    EXPECT_THROW(ParseHex("$"), std::invalid_argument);
}

// Test: ParseHex handles lowercase
TEST(ParseHexExceptionTest, HandlesLowercase) {
    EXPECT_EQ(0xabcd, ParseHex("$abcd"));
    EXPECT_EQ(0xabcd, ParseHex("abcd"));
}

// Test: ParseHex handles mixed case
TEST(ParseHexExceptionTest, HandlesMixedCase) {
    EXPECT_EQ(0xAbCd, ParseHex("$AbCd"));
    EXPECT_EQ(0xAbCd, ParseHex("AbCd"));
}

// Test: ParseHex handles zero
TEST(ParseHexExceptionTest, HandlesZero) {
    EXPECT_EQ(0, ParseHex("$0"));
    EXPECT_EQ(0, ParseHex("0"));
}

} // namespace xasm
