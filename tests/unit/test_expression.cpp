// Expression tests
// Phase 1: Minimal Viable Assembler - Core Abstractions

#include "xasm++/expression.h"
#include <gtest/gtest.h>

using namespace xasm;

// Mock SymbolTable for testing (minimal interface)
class MockSymbolTable : public SymbolTable {
public:
  MockSymbolTable() {
    // Add some test symbols
    Define("label1", 0x1000);
    Define("label2", 0x2000);
    Define("offset", 0x0042);
  }

  void Define(const std::string &name, int64_t value) {
    symbols_[name] = value;
  }

  bool Lookup(const std::string &name, int64_t &value) const override {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
      value = it->second;
      return true;
    }
    return false;
  }

private:
  std::map<std::string, int64_t> symbols_;
};

// Test 1: LiteralExpr creation and evaluation
TEST(ExpressionTest, LiteralExprCreation) {
  MockSymbolTable symbols;

  LiteralExpr expr(42);
  EXPECT_EQ(expr.Evaluate(symbols), 42);
  EXPECT_TRUE(expr.IsConstant());
  EXPECT_FALSE(expr.IsRelocatable());
}

// Test 2: LiteralExpr with negative value
TEST(ExpressionTest, LiteralExprNegative) {
  MockSymbolTable symbols;

  LiteralExpr expr(-100);
  EXPECT_EQ(expr.Evaluate(symbols), -100);
  EXPECT_TRUE(expr.IsConstant());
}

// Test 3: SymbolExpr creation and evaluation
TEST(ExpressionTest, SymbolExprCreation) {
  MockSymbolTable symbols;

  SymbolExpr expr("label1");
  EXPECT_EQ(expr.Evaluate(symbols), 0x1000);
  EXPECT_FALSE(expr.IsConstant());
  EXPECT_TRUE(expr.IsRelocatable());
}

// Test 4: SymbolExpr with undefined symbol
TEST(ExpressionTest, SymbolExprUndefined) {
  MockSymbolTable symbols;

  SymbolExpr expr("undefined");
  EXPECT_THROW(expr.Evaluate(symbols), std::runtime_error);
}

// Test 5: BinaryOpExpr - Addition
TEST(ExpressionTest, BinaryOpAddition) {
  MockSymbolTable symbols;

  auto left = std::make_shared<LiteralExpr>(10);
  auto right = std::make_shared<LiteralExpr>(32);
  BinaryOpExpr expr(BinaryOp::Add, left, right);

  EXPECT_EQ(expr.Evaluate(symbols), 42);
  EXPECT_TRUE(expr.IsConstant()); // Both operands are constant
}

// Test 6: BinaryOpExpr - Subtraction
TEST(ExpressionTest, BinaryOpSubtraction) {
  MockSymbolTable symbols;

  auto left = std::make_shared<LiteralExpr>(100);
  auto right = std::make_shared<LiteralExpr>(58);
  BinaryOpExpr expr(BinaryOp::Subtract, left, right);

  EXPECT_EQ(expr.Evaluate(symbols), 42);
}

// Test 7: BinaryOpExpr - Multiplication
TEST(ExpressionTest, BinaryOpMultiplication) {
  MockSymbolTable symbols;

  auto left = std::make_shared<LiteralExpr>(6);
  auto right = std::make_shared<LiteralExpr>(7);
  BinaryOpExpr expr(BinaryOp::Multiply, left, right);

  EXPECT_EQ(expr.Evaluate(symbols), 42);
}

// Test 8: BinaryOpExpr - Division
TEST(ExpressionTest, BinaryOpDivision) {
  MockSymbolTable symbols;

  auto left = std::make_shared<LiteralExpr>(84);
  auto right = std::make_shared<LiteralExpr>(2);
  BinaryOpExpr expr(BinaryOp::Divide, left, right);

  EXPECT_EQ(expr.Evaluate(symbols), 42);
}

// Test 9: BinaryOpExpr with symbol
TEST(ExpressionTest, BinaryOpWithSymbol) {
  MockSymbolTable symbols;

  auto left = std::make_shared<SymbolExpr>("label1");
  auto right = std::make_shared<LiteralExpr>(0x100);
  BinaryOpExpr expr(BinaryOp::Add, left, right);

  EXPECT_EQ(expr.Evaluate(symbols), 0x1100);
  EXPECT_FALSE(expr.IsConstant()); // Contains symbol reference
  EXPECT_TRUE(expr.IsRelocatable());
}

// Test 10: UnaryOpExpr - Negation
TEST(ExpressionTest, UnaryOpNegation) {
  MockSymbolTable symbols;

  auto operand = std::make_shared<LiteralExpr>(42);
  UnaryOpExpr expr(UnaryOp::Negate, operand);

  EXPECT_EQ(expr.Evaluate(symbols), -42);
  EXPECT_TRUE(expr.IsConstant());
}

// Test 11: UnaryOpExpr - Bitwise NOT
TEST(ExpressionTest, UnaryOpBitwiseNot) {
  MockSymbolTable symbols;

  auto operand = std::make_shared<LiteralExpr>(0x00);
  UnaryOpExpr expr(UnaryOp::BitwiseNot, operand);

  EXPECT_EQ(expr.Evaluate(symbols),
            -1); // ~0 = 0xFFFFFFFFFFFFFFFF = -1 in two's complement
}

// Test 12: Complex expression tree
TEST(ExpressionTest, ComplexExpressionTree) {
  MockSymbolTable symbols;

  // Expression: (label1 + 0x100) - offset
  // = (0x1000 + 0x100) - 0x42
  // = 0x1100 - 0x42
  // = 0x10BE

  auto label = std::make_shared<SymbolExpr>("label1");
  auto offset1 = std::make_shared<LiteralExpr>(0x100);
  auto sum = std::make_shared<BinaryOpExpr>(BinaryOp::Add, label, offset1);

  auto offset2 = std::make_shared<SymbolExpr>("offset");
  BinaryOpExpr expr(BinaryOp::Subtract, sum, offset2);

  EXPECT_EQ(expr.Evaluate(symbols), 0x10BE);
}
