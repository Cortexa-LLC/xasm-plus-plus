/**
 * @file test_z80_expressions.cpp
 * @brief Unit tests for Z80 Universal Syntax expression evaluation
 *
 * Tests expression parsing and evaluation including:
 * - Arithmetic operators: +, -, *, /, %
 * - Bitwise operators: &, |, ^, ~, <<, >>
 * - Logical operators: &&, ||, !
 * - Comparison operators: ==, !=, <, >, <=, >=
 * - Unary operators: HIGH, LOW (byte extraction)
 * - Operator precedence
 * - Parentheses and brackets
 */

#include "xasm++/symbol.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

class Z80ExpressionTest : public ::testing::Test {
protected:
  EdtasmM80PlusPlusSyntaxParser parser;
  ConcreteSymbolTable symbols;

  void SetUp() override {
    // Define some test symbols
    symbols.Define("ADDR", SymbolType::Equate,
                   std::make_shared<LiteralExpr>(0x8000));
    symbols.Define("SIZE", SymbolType::Equate,
                   std::make_shared<LiteralExpr>(256));
    symbols.Define("VALUE", SymbolType::Equate,
                   std::make_shared<LiteralExpr>(42));
  }

  int64_t EvalExpr(const std::string &expr_str) {
    auto expr = parser.ParseExpression(expr_str, symbols);
    return expr->Evaluate(symbols);
  }
};

// ============================================================================
// Arithmetic Operators
// ============================================================================

TEST_F(Z80ExpressionTest, Addition) {
  EXPECT_EQ(EvalExpr("10 + 5"), 15);
  EXPECT_EQ(EvalExpr("100 + 200"), 300);
  EXPECT_EQ(EvalExpr("VALUE + 8"), 50);
}

TEST_F(Z80ExpressionTest, Subtraction) {
  EXPECT_EQ(EvalExpr("10 - 5"), 5);
  EXPECT_EQ(EvalExpr("100 - 200"), -100);
  EXPECT_EQ(EvalExpr("SIZE - 56"), 200);
}

TEST_F(Z80ExpressionTest, Multiplication) {
  EXPECT_EQ(EvalExpr("10 * 5"), 50);
  EXPECT_EQ(EvalExpr("3 * 7"), 21);
  EXPECT_EQ(EvalExpr("VALUE * 2"), 84);
}

TEST_F(Z80ExpressionTest, Division) {
  EXPECT_EQ(EvalExpr("10 / 5"), 2);
  EXPECT_EQ(EvalExpr("100 / 4"), 25);
  EXPECT_EQ(EvalExpr("SIZE / 2"), 128);
}

TEST_F(Z80ExpressionTest, Modulo) {
  EXPECT_EQ(EvalExpr("10 % 3"), 1);
  EXPECT_EQ(EvalExpr("100 % 7"), 2);
  EXPECT_EQ(EvalExpr("SIZE % 100"), 56);
}

TEST_F(Z80ExpressionTest, ComplexArithmetic) {
  EXPECT_EQ(EvalExpr("10 + 5 * 2"), 20);     // Precedence: * before +
  EXPECT_EQ(EvalExpr("(10 + 5) * 2"), 30);   // Parentheses override
  EXPECT_EQ(EvalExpr("100 - 20 / 4"), 95);   // Precedence: / before -
  EXPECT_EQ(EvalExpr("(100 - 20) / 4"), 20); // Parentheses override
}

// ============================================================================
// Bitwise Operators
// ============================================================================

TEST_F(Z80ExpressionTest, BitwiseAnd) {
  EXPECT_EQ(EvalExpr("0xFF & 0x0F"), 0x0F);
  EXPECT_EQ(EvalExpr("11110000B & 00111100B"), 0b00110000);
  EXPECT_EQ(EvalExpr("255 & 15"), 15);
}

TEST_F(Z80ExpressionTest, BitwiseOr) {
  EXPECT_EQ(EvalExpr("0xF0 | 0x0F"), 0xFF);
  EXPECT_EQ(EvalExpr("11110000B | 00001111B"), 0b11111111);
  EXPECT_EQ(EvalExpr("128 | 64"), 192);
}

TEST_F(Z80ExpressionTest, BitwiseXor) {
  EXPECT_EQ(EvalExpr("0xFF ^ 0x0F"), 0xF0);
  EXPECT_EQ(EvalExpr("11110000B ^ 00111100B"), 0b11001100);
  EXPECT_EQ(EvalExpr("255 ^ 15"), 240);
}

TEST_F(Z80ExpressionTest, BitwiseNot) {
  EXPECT_EQ(EvalExpr("~0") & 0xFF, 0xFF);
  EXPECT_EQ(EvalExpr("~0xFF") & 0xFF, 0x00);
  EXPECT_EQ(EvalExpr("~0xF0") & 0xFF, 0x0F);
}

TEST_F(Z80ExpressionTest, ShiftLeft) {
  EXPECT_EQ(EvalExpr("1 << 0"), 1);
  EXPECT_EQ(EvalExpr("1 << 4"), 16);
  EXPECT_EQ(EvalExpr("5 << 2"), 20);
  EXPECT_EQ(EvalExpr("0xFF << 8"), 0xFF00);
}

TEST_F(Z80ExpressionTest, ShiftRight) {
  EXPECT_EQ(EvalExpr("16 >> 4"), 1);
  EXPECT_EQ(EvalExpr("20 >> 2"), 5);
  EXPECT_EQ(EvalExpr("0xFF00 >> 8"), 0xFF);
}

// ============================================================================
// Logical Operators
// ============================================================================

TEST_F(Z80ExpressionTest, LogicalAnd) {
  EXPECT_EQ(EvalExpr("1 && 1"), 1);
  EXPECT_EQ(EvalExpr("1 && 0"), 0);
  EXPECT_EQ(EvalExpr("0 && 1"), 0);
  EXPECT_EQ(EvalExpr("0 && 0"), 0);
  EXPECT_EQ(EvalExpr("5 && 10"), 1);
}

TEST_F(Z80ExpressionTest, LogicalOr) {
  EXPECT_EQ(EvalExpr("1 || 1"), 1);
  EXPECT_EQ(EvalExpr("1 || 0"), 1);
  EXPECT_EQ(EvalExpr("0 || 1"), 1);
  EXPECT_EQ(EvalExpr("0 || 0"), 0);
  EXPECT_EQ(EvalExpr("5 || 10"), 1);
}

TEST_F(Z80ExpressionTest, LogicalNot) {
  EXPECT_EQ(EvalExpr("!0"), 1);
  EXPECT_EQ(EvalExpr("!1"), 0);
  EXPECT_EQ(EvalExpr("!42"), 0);
}

// ============================================================================
// Comparison Operators
// ============================================================================

TEST_F(Z80ExpressionTest, Equal) {
  EXPECT_EQ(EvalExpr("5 == 5"), 1);
  EXPECT_EQ(EvalExpr("5 == 6"), 0);
  EXPECT_EQ(EvalExpr("VALUE == 42"), 1);
}

TEST_F(Z80ExpressionTest, NotEqual) {
  EXPECT_EQ(EvalExpr("5 != 5"), 0);
  EXPECT_EQ(EvalExpr("5 != 6"), 1);
  EXPECT_EQ(EvalExpr("VALUE != 43"), 1);
}

TEST_F(Z80ExpressionTest, LessThan) {
  EXPECT_EQ(EvalExpr("5 < 10"), 1);
  EXPECT_EQ(EvalExpr("10 < 5"), 0);
  EXPECT_EQ(EvalExpr("5 < 5"), 0);
}

TEST_F(Z80ExpressionTest, GreaterThan) {
  EXPECT_EQ(EvalExpr("10 > 5"), 1);
  EXPECT_EQ(EvalExpr("5 > 10"), 0);
  EXPECT_EQ(EvalExpr("5 > 5"), 0);
}

TEST_F(Z80ExpressionTest, LessOrEqual) {
  EXPECT_EQ(EvalExpr("5 <= 10"), 1);
  EXPECT_EQ(EvalExpr("5 <= 5"), 1);
  EXPECT_EQ(EvalExpr("10 <= 5"), 0);
}

TEST_F(Z80ExpressionTest, GreaterOrEqual) {
  EXPECT_EQ(EvalExpr("10 >= 5"), 1);
  EXPECT_EQ(EvalExpr("5 >= 5"), 1);
  EXPECT_EQ(EvalExpr("5 >= 10"), 0);
}

// ============================================================================
// Unary Operators: HIGH and LOW
// ============================================================================

TEST_F(Z80ExpressionTest, HighByte) {
  EXPECT_EQ(EvalExpr("HIGH(0x1234)"), 0x12);
  EXPECT_EQ(EvalExpr("HIGH(0xABCD)"), 0xAB);
  EXPECT_EQ(EvalExpr("HIGH(ADDR)"), 0x80);
  EXPECT_EQ(EvalExpr("HIGH(256)"), 0x01);
}

TEST_F(Z80ExpressionTest, LowByte) {
  EXPECT_EQ(EvalExpr("LOW(0x1234)"), 0x34);
  EXPECT_EQ(EvalExpr("LOW(0xABCD)"), 0xCD);
  EXPECT_EQ(EvalExpr("LOW(ADDR)"), 0x00);
  EXPECT_EQ(EvalExpr("LOW(256)"), 0x00);
}

TEST_F(Z80ExpressionTest, HighLowCombination) {
  EXPECT_EQ(EvalExpr("HIGH(ADDR) * 256 + LOW(ADDR)"), 0x8000);
  EXPECT_EQ(EvalExpr("(HIGH(0x1234) << 8) | LOW(0x1234)"), 0x1234);
}

// ============================================================================
// Operator Precedence
// ============================================================================

TEST_F(Z80ExpressionTest, ArithmeticPrecedence) {
  EXPECT_EQ(EvalExpr("2 + 3 * 4"), 14); // * before +
  EXPECT_EQ(EvalExpr("10 - 8 / 2"), 6); // / before -
  EXPECT_EQ(EvalExpr("15 % 4 + 2"), 5); // % before +
}

TEST_F(Z80ExpressionTest, BitwisePrecedence) {
  EXPECT_EQ(EvalExpr("8 | 4 & 2"), 8);  // & before |
  EXPECT_EQ(EvalExpr("8 ^ 4 | 2"), 14); // ^ before |
}

TEST_F(Z80ExpressionTest, ShiftPrecedence) {
  EXPECT_EQ(EvalExpr("4 << 2 + 1"),
            32); // + before <<: 4 << (2+1) = 4 << 3 = 32
  EXPECT_EQ(EvalExpr("16 >> 2 - 1"),
            8); // - before >>: 16 >> (2-1) = 16 >> 1 = 8
}

TEST_F(Z80ExpressionTest, ComparisonPrecedence) {
  EXPECT_EQ(EvalExpr("5 + 3 > 7"), 1);  // + before >
  EXPECT_EQ(EvalExpr("10 - 5 < 3"), 0); // - before <
}

TEST_F(Z80ExpressionTest, LogicalPrecedence) {
  EXPECT_EQ(EvalExpr("1 || 0 && 0"), 1); // && before ||
  EXPECT_EQ(EvalExpr("0 && 1 || 1"), 1); // && before ||
}

TEST_F(Z80ExpressionTest, MixedPrecedence) {
  EXPECT_EQ(EvalExpr("5 + 3 * 2 & 0xFF"), 11);    // *, +, & in order
  EXPECT_EQ(EvalExpr("10 << 1 | 4 >> 1"), 22);    // <<, >>, | in order
  EXPECT_EQ(EvalExpr("5 * 2 == 10 && 3 < 5"), 1); // *, ==, <, && in order
}

// ============================================================================
// Parentheses and Brackets
// ============================================================================

TEST_F(Z80ExpressionTest, Parentheses) {
  EXPECT_EQ(EvalExpr("(5 + 3) * 2"), 16);
  EXPECT_EQ(EvalExpr("10 / (2 + 3)"), 2);
  EXPECT_EQ(EvalExpr("((5 + 3) * 2) - 1"), 15);
}

TEST_F(Z80ExpressionTest, Brackets) {
  EXPECT_EQ(EvalExpr("[5 + 3] * 2"), 16);
  EXPECT_EQ(EvalExpr("10 / [2 + 3]"), 2);
  EXPECT_EQ(EvalExpr("[[5 + 3] * 2] - 1"), 15);
}

TEST_F(Z80ExpressionTest, MixedParentheses) {
  EXPECT_EQ(EvalExpr("(5 + [3 * 2]) - 1"), 10);
  EXPECT_EQ(EvalExpr("[5 + (3 * 2)] - 1"), 10);
}

// ============================================================================
// Number Formats
// ============================================================================

TEST_F(Z80ExpressionTest, HexNumbers) {
  EXPECT_EQ(EvalExpr("$FF"), 255);
  EXPECT_EQ(EvalExpr("0xFF"), 255);
  EXPECT_EQ(EvalExpr("0FFH"), 255);
  EXPECT_EQ(EvalExpr("$10 + 0x20 + 30H"), 0x10 + 0x20 + 0x30);
}

TEST_F(Z80ExpressionTest, BinaryNumbers) {
  EXPECT_EQ(EvalExpr("11111111B"), 255);
  EXPECT_EQ(EvalExpr("10101010B"), 170);
  EXPECT_EQ(EvalExpr("11110000B | 00001111B"), 255);
}

TEST_F(Z80ExpressionTest, OctalNumbers) {
  EXPECT_EQ(EvalExpr("377O"), 255);
  EXPECT_EQ(EvalExpr("377Q"), 255);
  EXPECT_EQ(EvalExpr("100O + 77Q"), 64 + 63);
}

TEST_F(Z80ExpressionTest, DecimalNumbers) {
  EXPECT_EQ(EvalExpr("255"), 255);
  EXPECT_EQ(EvalExpr("255D"), 255);
  EXPECT_EQ(EvalExpr("100 + 200"), 300);
}

// ============================================================================
// Symbol References
// ============================================================================

TEST_F(Z80ExpressionTest, SymbolReferences) {
  EXPECT_EQ(EvalExpr("ADDR"), 0x8000);
  EXPECT_EQ(EvalExpr("SIZE"), 256);
  EXPECT_EQ(EvalExpr("VALUE"), 42);
}

TEST_F(Z80ExpressionTest, SymbolArithmetic) {
  EXPECT_EQ(EvalExpr("ADDR + SIZE"), 0x8000 + 256);
  EXPECT_EQ(EvalExpr("SIZE * 2"), 512);
  EXPECT_EQ(EvalExpr("HIGH(ADDR) + LOW(ADDR)"), 0x80);
}

// ============================================================================
// Complex Expressions
// ============================================================================

TEST_F(Z80ExpressionTest, ComplexExpression1) {
  // (ADDR + SIZE * 2) & 0xFFFF
  EXPECT_EQ(EvalExpr("(ADDR + SIZE * 2) & 0xFFFF"),
            (0x8000 + 256 * 2) & 0xFFFF);
}

TEST_F(Z80ExpressionTest, ComplexExpression2) {
  // HIGH(ADDR) << 8 | LOW(ADDR + 0x100)
  EXPECT_EQ(EvalExpr("HIGH(ADDR) << 8 | LOW(ADDR + 0x100)"),
            (0x80 << 8) | 0x00);
}

TEST_F(Z80ExpressionTest, ComplexExpression3) {
  // (VALUE + 8) * 2 - 10 / 2 + 3
  EXPECT_EQ(EvalExpr("(VALUE + 8) * 2 - 10 / 2 + 3"),
            (42 + 8) * 2 - 10 / 2 + 3);
}

TEST_F(Z80ExpressionTest, ComplexExpression4) {
  // ((SIZE & 0xFF) << 8) | (VALUE & 0xFF)
  EXPECT_EQ(EvalExpr("((SIZE & 0xFF) << 8) | (VALUE & 0xFF)"),
            ((256 & 0xFF) << 8) | (42 & 0xFF));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(Z80ExpressionTest, EmptyExpression) {
  EXPECT_EQ(EvalExpr(""), 0);
  EXPECT_EQ(EvalExpr("   "), 0);
}

TEST_F(Z80ExpressionTest, NegativeNumbers) {
  EXPECT_EQ(EvalExpr("-5"), -5);
  EXPECT_EQ(EvalExpr("-10 + 15"), 5);
  EXPECT_EQ(EvalExpr("10 + -5"), 5);
}

TEST_F(Z80ExpressionTest, UnaryMinus) {
  EXPECT_EQ(EvalExpr("-(5 + 3)"), -8);
  EXPECT_EQ(EvalExpr("-VALUE"), -42);
}

TEST_F(Z80ExpressionTest, MultipleUnary) {
  EXPECT_EQ(EvalExpr("--5"), 5);
  EXPECT_EQ(EvalExpr("~~0xFF") & 0xFF, 0xFF);
  EXPECT_EQ(EvalExpr("!!5"), 1);
}

TEST_F(Z80ExpressionTest, WhitespaceHandling) {
  EXPECT_EQ(EvalExpr("  5  +  3  "), 8);
  EXPECT_EQ(EvalExpr("10*2"), 20);
  EXPECT_EQ(EvalExpr(" ( 5 + 3 ) * 2 "), 16);
}
