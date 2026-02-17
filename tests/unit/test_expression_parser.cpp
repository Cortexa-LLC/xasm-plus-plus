/**
 * @file test_expression_parser.cpp
 * @brief Unit tests for ExpressionParser
 *
 * Tests the shared expression parser component that extracts duplicated
 * parsing logic from Z80Universal, Merlin, and SCMASM syntax parsers.
 */

#include "xasm++/common/expression_parser.h"
#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>
#include <map>
#include <memory>

using namespace xasm;

/**
 * @brief Mock symbol table for testing
 *
 * Provides a simple in-memory symbol table for testing expression
 * evaluation without depending on the full assembler infrastructure.
 */
class MockSymbolTable : public SymbolTable {
public:
  MockSymbolTable() : current_location_(0) {}

  /**
   * @brief Add a symbol to the mock table
   * @param name Symbol name
   * @param value Symbol value
   */
  void AddSymbol(const std::string &name, int64_t value) {
    symbols_[name] = value;
  }

  /**
   * @brief Look up a symbol in the mock table
   * @param name Symbol name to look up
   * @param value Output parameter for symbol value
   * @return true if symbol found, false otherwise
   */
  bool Lookup(const std::string &name, int64_t &value) const override {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
      value = it->second;
      return true;
    }
    return false;
  }

  /**
   * @brief Get the current assembly location
   * @return Current location counter value
   */
  int64_t GetCurrentLocation() const override { return current_location_; }

  /**
   * @brief Set the current assembly location (for testing)
   * @param loc New location value
   */
  void SetCurrentLocation(int64_t loc) { current_location_ = loc; }

  /**
   * @brief Define a symbol (helper for tests, not part of SymbolTable
   * interface)
   * @param name Symbol name
   * @param type Symbol type (unused in mock)
   * @param value_expr Symbol value expression
   */
  void Define(const std::string &name, SymbolType type,
              std::shared_ptr<Expression> value_expr) {
    (void)type; // Unused in mock implementation
    int64_t value = value_expr->Evaluate(*this);
    symbols_[name] = value;
  }

private:
  std::map<std::string, int64_t> symbols_;
  int64_t current_location_;
};

// ============================================================================
// Phase 1: Literal parsing
// ============================================================================

TEST(ExpressionParserTest, DecimalLiterals) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("42");
  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->Evaluate(symbols), 42);
  EXPECT_TRUE(expr->IsConstant());
  EXPECT_FALSE(expr->IsRelocatable());

  expr = parser.Parse("0");
  EXPECT_EQ(expr->Evaluate(symbols), 0);

  expr = parser.Parse("65535");
  EXPECT_EQ(expr->Evaluate(symbols), 65535);
}

TEST(ExpressionParserTest, HexadecimalLiterals) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("$FF");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  expr = parser.Parse("0xFF");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  expr = parser.Parse("$AbCd");
  EXPECT_EQ(expr->Evaluate(symbols), 0xABCD);
}

TEST(ExpressionParserTest, BinaryLiterals) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("%11110000");
  EXPECT_EQ(expr->Evaluate(symbols), 0xF0);

  expr = parser.Parse("0b11110000");
  EXPECT_EQ(expr->Evaluate(symbols), 0xF0);
}

// ============================================================================
// Phase 2: Addition and subtraction
// ============================================================================

TEST(ExpressionParserTest, Addition) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("1 + 2");
  EXPECT_EQ(expr->Evaluate(symbols), 3);

  expr = parser.Parse("10 + 20 + 30");
  EXPECT_EQ(expr->Evaluate(symbols), 60);

  expr = parser.Parse("5+3");
  EXPECT_EQ(expr->Evaluate(symbols), 8);
}

TEST(ExpressionParserTest, Subtraction) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("10 - 3");
  EXPECT_EQ(expr->Evaluate(symbols), 7);

  expr = parser.Parse("100 - 20 - 5");
  EXPECT_EQ(expr->Evaluate(symbols), 75);

  expr = parser.Parse("10 + 5 - 3");
  EXPECT_EQ(expr->Evaluate(symbols), 12);
}

// ============================================================================
// Phase 3: Multiplication, division, modulo
// ============================================================================

TEST(ExpressionParserTest, Multiplication) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("5 * 3");
  EXPECT_EQ(expr->Evaluate(symbols), 15);

  expr = parser.Parse("2 * 3 * 4");
  EXPECT_EQ(expr->Evaluate(symbols), 24);
}

TEST(ExpressionParserTest, Division) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("15 / 3");
  EXPECT_EQ(expr->Evaluate(symbols), 5);

  expr = parser.Parse("17 / 5");
  EXPECT_EQ(expr->Evaluate(symbols), 3);

  expr = parser.Parse("10 / 0");
  EXPECT_THROW(expr->Evaluate(symbols), std::runtime_error);
}

TEST(ExpressionParserTest, Modulo) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("17 % 5");
  EXPECT_EQ(expr->Evaluate(symbols), 2);

  expr = parser.Parse("10 % 0");
  EXPECT_THROW(expr->Evaluate(symbols), std::runtime_error);
}

// ============================================================================
// Phase 4: Operator precedence
// ============================================================================

TEST(ExpressionParserTest, OperatorPrecedence) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("2 + 3 * 4");
  EXPECT_EQ(expr->Evaluate(symbols), 14); // Not 20

  expr = parser.Parse("20 - 10 / 2");
  EXPECT_EQ(expr->Evaluate(symbols), 15); // Not 5

  expr = parser.Parse("10 - 5 - 2");
  EXPECT_EQ(expr->Evaluate(symbols), 3); // (10-5)-2, not 10-(5-2)=7
}

// ============================================================================
// Phase 5: Parentheses
// ============================================================================

TEST(ExpressionParserTest, Parentheses) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("(2 + 3) * 4");
  EXPECT_EQ(expr->Evaluate(symbols), 20); // Not 14

  expr = parser.Parse("((2 + 3) * 4) - 5");
  EXPECT_EQ(expr->Evaluate(symbols), 15);

  expr = parser.Parse("(10 + 5) * (3 - 1)");
  EXPECT_EQ(expr->Evaluate(symbols), 30);

  EXPECT_THROW(parser.Parse("(2 + 3"), std::runtime_error);
  EXPECT_THROW(parser.Parse("2 + 3)"), std::runtime_error);
}

// ============================================================================
// Phase 6: Unary operators
// ============================================================================

TEST(ExpressionParserTest, UnaryMinus) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("-5");
  EXPECT_EQ(expr->Evaluate(symbols), -5);

  expr = parser.Parse("-(2 + 3)");
  EXPECT_EQ(expr->Evaluate(symbols), -5);

  expr = parser.Parse("--5");
  EXPECT_EQ(expr->Evaluate(symbols), 5);
}

TEST(ExpressionParserTest, BitwiseNOT) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("~0");
  EXPECT_EQ(expr->Evaluate(symbols), ~0);

  expr = parser.Parse("~$FF");
  EXPECT_EQ((expr->Evaluate(symbols) & 0xFF), 0);
}

TEST(ExpressionParserTest, LogicalNOT) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("!0");
  EXPECT_EQ(expr->Evaluate(symbols), 1);

  expr = parser.Parse("!5");
  EXPECT_EQ(expr->Evaluate(symbols), 0);
}

// ============================================================================
// Phase 7: Bitwise operators
// ============================================================================

TEST(ExpressionParserTest, BitwiseAND) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("$F0 & $0F");
  EXPECT_EQ(expr->Evaluate(symbols), 0);

  expr = parser.Parse("$FF & $AA");
  EXPECT_EQ(expr->Evaluate(symbols), 0xAA);
}

TEST(ExpressionParserTest, BitwiseOR) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("$F0 | $0F");
  EXPECT_EQ(expr->Evaluate(symbols), 0xFF);
}

TEST(ExpressionParserTest, BitwiseXOR) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("$FF ^ $AA");
  EXPECT_EQ(expr->Evaluate(symbols), 0x55);
}

TEST(ExpressionParserTest, ShiftLeft) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("1 << 8");
  EXPECT_EQ(expr->Evaluate(symbols), 256);

  expr = parser.Parse("$FF << 4");
  EXPECT_EQ(expr->Evaluate(symbols), 0xFF0);
}

TEST(ExpressionParserTest, ShiftRight) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("256 >> 8");
  EXPECT_EQ(expr->Evaluate(symbols), 1);

  expr = parser.Parse("$FF0 >> 4");
  EXPECT_EQ(expr->Evaluate(symbols), 0xFF);
}

// ============================================================================
// Phase 8: Logical operators
// ============================================================================

TEST(ExpressionParserTest, LogicalAND) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("1 && 1");
  EXPECT_EQ(expr->Evaluate(symbols), 1);

  expr = parser.Parse("1 && 0");
  EXPECT_EQ(expr->Evaluate(symbols), 0);

  expr = parser.Parse("0 && 0");
  EXPECT_EQ(expr->Evaluate(symbols), 0);
}

TEST(ExpressionParserTest, LogicalOR) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("1 || 1");
  EXPECT_EQ(expr->Evaluate(symbols), 1);

  expr = parser.Parse("1 || 0");
  EXPECT_EQ(expr->Evaluate(symbols), 1);

  expr = parser.Parse("0 || 0");
  EXPECT_EQ(expr->Evaluate(symbols), 0);
}

// ============================================================================
// Phase 9: Symbol resolution
// ============================================================================

TEST(ExpressionParserTest, SymbolResolution) {
  MockSymbolTable symbols;
  symbols.AddSymbol("start", 0x8000);
  symbols.AddSymbol("end", 0x9000);

  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("start");
  EXPECT_EQ(expr->Evaluate(symbols), 0x8000);
  EXPECT_FALSE(expr->IsConstant());
  EXPECT_TRUE(expr->IsRelocatable());

  expr = parser.Parse("start + 2");
  EXPECT_EQ(expr->Evaluate(symbols), 0x8002);

  expr = parser.Parse("end - start");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1000);

  expr = parser.Parse("undefined");
  EXPECT_THROW(expr->Evaluate(symbols), std::runtime_error);
}

TEST(ExpressionParserTest, SymbolsWithDots) {
  MockSymbolTable symbols;
  symbols.AddSymbol("A2osX.GP", 0x1000);
  symbols.AddSymbol("A2osX.LIBC", 0x1100);
  symbols.AddSymbol("MSG.INIT0", 0x2000);

  ExpressionParser parser(&symbols);

  // Test parsing symbols with dots
  auto expr = parser.Parse("A2osX.GP");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1000);

  expr = parser.Parse("A2osX.LIBC");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1100);

  // Test dots in expressions (like A2osX line 59)
  expr = parser.Parse("A2osX.GP + 0");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1000);

  expr = parser.Parse("A2osX.LIBC + $10");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1110);

  // Test multiple dots
  expr = parser.Parse("MSG.INIT0");
  EXPECT_EQ(expr->Evaluate(symbols), 0x2000);
}

// ============================================================================
// Phase 10: Whitespace handling
// ============================================================================

TEST(ExpressionParserTest, WhitespaceHandling) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("  42");
  EXPECT_EQ(expr->Evaluate(symbols), 42);

  expr = parser.Parse("42  ");
  EXPECT_EQ(expr->Evaluate(symbols), 42);

  expr = parser.Parse("10   +   5");
  EXPECT_EQ(expr->Evaluate(symbols), 15);

  expr = parser.Parse("10+5*2");
  EXPECT_EQ(expr->Evaluate(symbols), 20);
}

// ============================================================================
// Phase 11: Edge cases
// ============================================================================

TEST(ExpressionParserTest, EdgeCases) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("");
  EXPECT_EQ(expr->Evaluate(symbols), 0);

  expr = parser.Parse("   ");
  EXPECT_EQ(expr->Evaluate(symbols), 0);

  expr = parser.Parse("((10 + 5) * 2 - 3) / 3");
  EXPECT_EQ(expr->Evaluate(symbols), 9);
}

// ============================================================================
// Phase 12: Function calls (LOW, HIGH, etc.)
// ============================================================================

TEST(ExpressionParserTest, LOWFunction) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("LOW($ABCD)");
  EXPECT_EQ(expr->Evaluate(symbols), 0xCD);

  expr = parser.Parse("<$ABCD");
  EXPECT_EQ(expr->Evaluate(symbols), 0xCD);
}

TEST(ExpressionParserTest, HIGHFunction) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  auto expr = parser.Parse("HIGH($ABCD)");
  EXPECT_EQ(expr->Evaluate(symbols), 0xAB);

  expr = parser.Parse(">$ABCD");
  EXPECT_EQ(expr->Evaluate(symbols), 0xAB);
}

// ============================================================================
// Phase 13: Custom Number Parser Interface
// ============================================================================

/**
 * @brief Mock custom number parser for testing
 *
 * Implements INumberParser to parse hex numbers with H suffix (e.g., "0FFH")
 */
class MockNumberParser : public INumberParser {
public:
  bool TryParse(const std::string &token, int64_t &value) const override {
    // Parse hex with H suffix: 0FFH
    if (token.length() >= 2 && (token.back() == 'H' || token.back() == 'h')) {
      std::string hex_part = token.substr(0, token.length() - 1);

      // Check if all characters are valid hex digits
      for (char c : hex_part) {
        if (!std::isxdigit(c)) {
          return false;
        }
      }

      // Convert to value
      value = 0;
      for (char c : hex_part) {
        value *= 16;
        if (c >= '0' && c <= '9') {
          value += c - '0';
        } else if (c >= 'A' && c <= 'F') {
          value += c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
          value += c - 'a' + 10;
        }
      }
      return true;
    }
    return false;
  }
};

TEST(ExpressionParserTest, CustomNumberParserHSuffix) {
  MockSymbolTable symbols;
  MockNumberParser custom_parser;
  ExpressionParser parser(&symbols, &custom_parser);

  // Should parse H-suffix hex numbers
  auto expr = parser.Parse("0FFH");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  expr = parser.Parse("10H");
  EXPECT_EQ(expr->Evaluate(symbols), 16);

  expr = parser.Parse("0ABCDh");
  EXPECT_EQ(expr->Evaluate(symbols), 0xABCD);

  // Should still parse standard formats
  expr = parser.Parse("$FF");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  expr = parser.Parse("0xFF");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  expr = parser.Parse("42");
  EXPECT_EQ(expr->Evaluate(symbols), 42);
}

TEST(ExpressionParserTest, CustomNumberParserInExpression) {
  MockSymbolTable symbols;
  MockNumberParser custom_parser;
  ExpressionParser parser(&symbols, &custom_parser);

  // Should work in arithmetic expressions
  auto expr = parser.Parse("10H + 5");
  EXPECT_EQ(expr->Evaluate(symbols), 21);

  expr = parser.Parse("0FFH & 0FH");
  EXPECT_EQ(expr->Evaluate(symbols), 0x0F);
}

// ============================================================================
// Phase 14: Current Location Operator ($)
// ============================================================================

TEST(ExpressionParserTest, ParsesCurrentLocationOperator) {
  MockSymbolTable symbols;
  symbols.SetCurrentLocation(0x8000);
  ExpressionParser parser(&symbols);

  // $ should parse as current location
  auto expr = parser.Parse("$");
  ASSERT_NE(expr, nullptr);
  EXPECT_EQ(expr->Evaluate(symbols), 0x8000);
}

TEST(ExpressionParserTest, CurrentLocationInArithmetic) {
  MockSymbolTable symbols;
  symbols.SetCurrentLocation(0x8000);
  ExpressionParser parser(&symbols);

  // $ + 5
  auto expr = parser.Parse("$ + 5");
  EXPECT_EQ(expr->Evaluate(symbols), 0x8005);

  // $ - 10
  expr = parser.Parse("$ - 10");
  EXPECT_EQ(expr->Evaluate(symbols), 0x7FF6);
}

TEST(ExpressionParserTest, CurrentLocationWithLabel) {
  MockSymbolTable symbols;
  symbols.SetCurrentLocation(0x8010);
  symbols.Define("START", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));
  ExpressionParser parser(&symbols);

  // $ - START (size calculation)
  auto expr = parser.Parse("$ - START");
  EXPECT_EQ(expr->Evaluate(symbols), 16);

  // START + ($ - START)
  expr = parser.Parse("START + ($ - START)");
  EXPECT_EQ(expr->Evaluate(symbols), 0x8010);
}

TEST(ExpressionParserTest, CurrentLocationInComplexExpression) {
  MockSymbolTable symbols;
  symbols.SetCurrentLocation(0x8042);
  ExpressionParser parser(&symbols);

  // ($ - START) / 2 - count of words
  symbols.Define("START", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));
  auto expr = parser.Parse("($ - START) / 2");
  EXPECT_EQ(expr->Evaluate(symbols), 33); // 66 / 2 = 33

  // 256 - ($ & 255) - bytes to next page boundary
  expr = parser.Parse("256 - ($ & 255)");
  EXPECT_EQ(expr->Evaluate(symbols), 190); // 256 - 66 = 190
}

TEST(ExpressionParserTest, DollarFollowedByHexDigitIsNumber) {
  MockSymbolTable symbols;
  ExpressionParser parser(&symbols);

  // $FF should be hex number, not current location
  auto expr = parser.Parse("$FF");
  EXPECT_EQ(expr->Evaluate(symbols), 255);

  // $ABCD should be hex number
  expr = parser.Parse("$ABCD");
  EXPECT_EQ(expr->Evaluate(symbols), 0xABCD);
}

TEST(ExpressionParserTest, DollarWithoutHexDigitIsCurrentLocation) {
  MockSymbolTable symbols;
  symbols.SetCurrentLocation(0x1000);
  ExpressionParser parser(&symbols);

  // $ + should be current location + ...
  auto expr = parser.Parse("$ + 10");
  EXPECT_EQ(expr->Evaluate(symbols), 0x100A);

  // $ - should be current location - ...
  expr = parser.Parse("$ - 5");
  EXPECT_EQ(expr->Evaluate(symbols), 0x0FFB);

  // $ ) should be current location in parens
  expr = parser.Parse("($ )");
  EXPECT_EQ(expr->Evaluate(symbols), 0x1000);
}
