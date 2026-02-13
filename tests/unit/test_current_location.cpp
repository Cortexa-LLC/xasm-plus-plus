/**
 * @file test_current_location.cpp
 * @brief Unit tests for current location operator ($)
 *
 * Tests the $ operator which returns the current assembly address.
 * Used in expressions for:
 * - Size calculations: DB "text", ($-start)
 * - Relative addressing: JR $+5
 * - Alignment: DS (256-$ & 255)
 * - Table generation: DW table_entry, ($-table_base)/2
 */

#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

/**
 * @brief Test fixture for current location operator tests
 */
class CurrentLocationTest : public ::testing::Test {
protected:
  ConcreteSymbolTable symbols;

  void SetUp() override {
    // Set current location to a known address
    symbols.SetCurrentLocation(0x8000);

    // Define some test symbols
    symbols.Define("START", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000));
    symbols.Define("END", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8010));
  }
};

// ============================================================================
// Basic Current Location Tests
// ============================================================================

TEST_F(CurrentLocationTest, SimpleCurrentLocation) {
  // $ should return current location
  auto expr = std::make_shared<CurrentLocationExpr>();
  EXPECT_EQ(expr->Evaluate(symbols), 0x8000);
}

TEST_F(CurrentLocationTest, CurrentLocationChanges) {
  auto expr = std::make_shared<CurrentLocationExpr>();

  // Initial location
  symbols.SetCurrentLocation(0x1000);
  EXPECT_EQ(expr->Evaluate(symbols), 0x1000);

  // Location changes
  symbols.SetCurrentLocation(0x2000);
  EXPECT_EQ(expr->Evaluate(symbols), 0x2000);

  // Location can be anywhere
  symbols.SetCurrentLocation(0xFFFF);
  EXPECT_EQ(expr->Evaluate(symbols), 0xFFFF);
}

// ============================================================================
// Arithmetic with Current Location
// ============================================================================

TEST_F(CurrentLocationTest, CurrentLocationPlusOffset) {
  // $ + 5 (for relative jumps)
  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Add, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<LiteralExpr>(5));

  symbols.SetCurrentLocation(0x8000);
  EXPECT_EQ(expr->Evaluate(symbols), 0x8005);
}

TEST_F(CurrentLocationTest, CurrentLocationMinusLabel) {
  // $ - START (size calculation)
  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Subtract, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<SymbolExpr>("START"));

  symbols.SetCurrentLocation(0x8010);
  EXPECT_EQ(expr->Evaluate(symbols), 0x10); // 16 bytes
}

TEST_F(CurrentLocationTest, LabelMinusCurrentLocation) {
  // END - $
  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Subtract, std::make_shared<SymbolExpr>("END"),
      std::make_shared<CurrentLocationExpr>());

  symbols.SetCurrentLocation(0x8005);
  EXPECT_EQ(expr->Evaluate(symbols), 0x0B); // 11 bytes remaining
}

// ============================================================================
// Complex Expressions with Current Location
// ============================================================================

TEST_F(CurrentLocationTest, SizeCalculationDividedByTwo) {
  // ($ - START) / 2 (count of 16-bit entries)
  auto size_expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Subtract, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<SymbolExpr>("START"));

  auto expr = std::make_shared<BinaryOpExpr>(BinaryOp::Divide, size_expr,
                                             std::make_shared<LiteralExpr>(2));

  symbols.SetCurrentLocation(0x8008);
  EXPECT_EQ(expr->Evaluate(symbols), 4); // 8 bytes / 2 = 4 words
}

TEST_F(CurrentLocationTest, AlignmentCalculation) {
  // 256 - ($ & 255) - align to next 256-byte boundary
  auto current_low = std::make_shared<BinaryOpExpr>(
      BinaryOp::BitwiseAnd, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<LiteralExpr>(255));

  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Subtract, std::make_shared<LiteralExpr>(256), current_low);

  symbols.SetCurrentLocation(0x8042);
  EXPECT_EQ(expr->Evaluate(symbols), 190); // 256 - 66 = 190 bytes to alignment

  symbols.SetCurrentLocation(0x8100);
  EXPECT_EQ(expr->Evaluate(symbols), 256); // Already aligned
}

// ============================================================================
// Expression Properties
// ============================================================================

TEST_F(CurrentLocationTest, IsNotConstant) {
  // $ is not a constant - it depends on assembly state
  auto expr = std::make_shared<CurrentLocationExpr>();
  EXPECT_FALSE(expr->IsConstant());
}

TEST_F(CurrentLocationTest, IsRelocatable) {
  // $ is relocatable - it's an address
  auto expr = std::make_shared<CurrentLocationExpr>();
  EXPECT_TRUE(expr->IsRelocatable());
}

TEST_F(CurrentLocationTest, ExpressionWithCurrentLocationIsNotConstant) {
  // Any expression containing $ is not constant
  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Add, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<LiteralExpr>(10));

  EXPECT_FALSE(expr->IsConstant());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(CurrentLocationTest, CurrentLocationAtZero) {
  // $ can be zero (ORG 0)
  auto expr = std::make_shared<CurrentLocationExpr>();
  symbols.SetCurrentLocation(0);
  EXPECT_EQ(expr->Evaluate(symbols), 0);
}

TEST_F(CurrentLocationTest, CurrentLocationHighAddress) {
  // $ can be at high addresses
  auto expr = std::make_shared<CurrentLocationExpr>();
  symbols.SetCurrentLocation(0xFFFF);
  EXPECT_EQ(expr->Evaluate(symbols), 0xFFFF);
}

TEST_F(CurrentLocationTest, MultipleCurrentLocationReferences) {
  // $ + $ (nonsensical but should work)
  auto expr = std::make_shared<BinaryOpExpr>(
      BinaryOp::Add, std::make_shared<CurrentLocationExpr>(),
      std::make_shared<CurrentLocationExpr>());

  symbols.SetCurrentLocation(0x1000);
  EXPECT_EQ(expr->Evaluate(symbols), 0x2000);
}
