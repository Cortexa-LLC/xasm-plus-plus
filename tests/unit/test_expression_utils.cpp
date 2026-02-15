/**
 * @file test_expression_utils.cpp
 * @brief Unit tests for expression evaluation utilities
 *
 * Tests the expression evaluation utility functions extracted
 * to reduce code duplication (P2.2: Extract expression evaluation utilities)
 */

#include "xasm++/expression_utils.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>
#include <memory>

using namespace xasm;

// Mock ExpressionParser for testing
class MockExpressionParser {
public:
  std::shared_ptr<Expression> ParseExpression(const std::string &str,
                                               SymbolTable & /*symbols*/) {
    // Return a simple literal expression for testing
    if (str == "42") {
      return std::make_shared<LiteralExpr>(42);
    } else if (str == "0") {
      return std::make_shared<LiteralExpr>(0);
    } else if (str == "255") {
      return std::make_shared<LiteralExpr>(255);
    } else if (str == "65535") {
      return std::make_shared<LiteralExpr>(65535);
    } else if (str == "-5") {
      return std::make_shared<LiteralExpr>(-5);
    } else if (str == "invalid") {
      return nullptr;
    }
    return std::make_shared<LiteralExpr>(42);
  }
};

// ==============================================================================
// ParseAndEvaluateExpression Tests
// ==============================================================================

TEST(ParseAndEvaluateExpressionTest, ValidExpression) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint32_t result =
      ParseAndEvaluateExpression("42", parser, symbols, "test expression");
  EXPECT_EQ(result, 42u);
}

TEST(ParseAndEvaluateExpressionTest, ZeroValue) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint32_t result =
      ParseAndEvaluateExpression("0", parser, symbols, "test expression");
  EXPECT_EQ(result, 0u);
}

TEST(ParseAndEvaluateExpressionTest, InvalidExpressionThrows) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  EXPECT_THROW(
      ParseAndEvaluateExpression("invalid", parser, symbols, "test expression"),
      std::runtime_error);
}

TEST(ParseAndEvaluateExpressionTest, ErrorMessageContainsContext) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  try {
    ParseAndEvaluateExpression("invalid", parser, symbols, "IF directive");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("IF directive"), std::string::npos);
  }
}

// ==============================================================================
// ParseAndEvaluateAsByte Tests
// ==============================================================================

TEST(ParseAndEvaluateAsByteTest, ValidByte) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint8_t result =
      ParseAndEvaluateAsByte("42", parser, symbols, "test expression");
  EXPECT_EQ(result, 42u);
}

TEST(ParseAndEvaluateAsByteTest, MaxByte) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint8_t result =
      ParseAndEvaluateAsByte("255", parser, symbols, "test expression");
  EXPECT_EQ(result, 255u);
}

TEST(ParseAndEvaluateAsByteTest, ValueMaskedTo8Bits) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  // Value 65535 should be masked to 255 (0xFF)
  uint8_t result =
      ParseAndEvaluateAsByte("65535", parser, symbols, "test expression");
  EXPECT_EQ(result, 255u);
}

TEST(ParseAndEvaluateAsByteTest, InvalidExpressionThrows) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  EXPECT_THROW(
      ParseAndEvaluateAsByte("invalid", parser, symbols, "test expression"),
      std::runtime_error);
}

// ==============================================================================
// ParseAndEvaluateAsWord Tests
// ==============================================================================

TEST(ParseAndEvaluateAsWordTest, ValidWord) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint16_t result =
      ParseAndEvaluateAsWord("42", parser, symbols, "test expression");
  EXPECT_EQ(result, 42u);
}

TEST(ParseAndEvaluateAsWordTest, MaxWord) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint16_t result =
      ParseAndEvaluateAsWord("65535", parser, symbols, "test expression");
  EXPECT_EQ(result, 65535u);
}

TEST(ParseAndEvaluateAsWordTest, InvalidExpressionThrows) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  EXPECT_THROW(
      ParseAndEvaluateAsWord("invalid", parser, symbols, "test expression"),
      std::runtime_error);
}

// ==============================================================================
// ParseAndEvaluateAsSignedInt Tests
// ==============================================================================

TEST(ParseAndEvaluateAsSignedIntTest, PositiveValue) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  int32_t result =
      ParseAndEvaluateAsSignedInt("42", parser, symbols, "test expression");
  EXPECT_EQ(result, 42);
}

TEST(ParseAndEvaluateAsSignedIntTest, NegativeValue) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  int32_t result =
      ParseAndEvaluateAsSignedInt("-5", parser, symbols, "test expression");
  EXPECT_EQ(result, -5);
}

TEST(ParseAndEvaluateAsSignedIntTest, Zero) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  int32_t result =
      ParseAndEvaluateAsSignedInt("0", parser, symbols, "test expression");
  EXPECT_EQ(result, 0);
}

TEST(ParseAndEvaluateAsSignedIntTest, InvalidExpressionThrows) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  EXPECT_THROW(
      ParseAndEvaluateAsSignedInt("invalid", parser, symbols, "test expression"),
      std::runtime_error);
}

// ==============================================================================
// TryParseAndEvaluateExpression Tests
// ==============================================================================

TEST(TryParseAndEvaluateExpressionTest, ValidExpressionReturnsTrue) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint32_t result;
  bool success = TryParseAndEvaluateExpression("42", parser, symbols, result);
  EXPECT_TRUE(success);
  EXPECT_EQ(result, 42u);
}

TEST(TryParseAndEvaluateExpressionTest, InvalidExpressionReturnsFalse) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint32_t result = 999; // Should remain unchanged
  bool success =
      TryParseAndEvaluateExpression("invalid", parser, symbols, result);
  EXPECT_FALSE(success);
  EXPECT_EQ(result, 999u); // Unchanged
}

TEST(TryParseAndEvaluateExpressionTest, ZeroValue) {
  MockExpressionParser parser;
  ConcreteSymbolTable symbols;
  uint32_t result;
  bool success = TryParseAndEvaluateExpression("0", parser, symbols, result);
  EXPECT_TRUE(success);
  EXPECT_EQ(result, 0u);
}
