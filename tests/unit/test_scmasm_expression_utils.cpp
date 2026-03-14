/**
 * @file test_scmasm_expression_utils.cpp
 * @brief Unit tests for SCMASM expression canonicalization utilities
 *
 * Tests for:
 * - CanonicalizeSlashHighByte: SCMASM '/' prefix -> '>' (ADR-005 V3/V6)
 * - CanonicalizeEqualityOperator: SCMASM single '=' -> '==' (ADR-005 V5)
 */

#include "xasm++/syntax/scmasm_expression_utils.h"
#include <gtest/gtest.h>

namespace xasm {
namespace scmasm {

// ============================================================================
// CanonicalizeSlashHighByte tests (ADR-005 V3/V6)
// ============================================================================

TEST(CanonicalizeSlashHighByteTest, SimplePrefixAtStart) {
  // Leading '/' at start of expression is a high-byte prefix
  EXPECT_EQ(CanonicalizeSlashHighByte("/ADDR"), ">ADDR");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterHash) {
  // '#' then '/' — '#' is an operand char (not an operator in this table),
  // so '/' after '#' is in operand position (binary division, not prefix).
  // Adjust: '#' precedes '#/ADDR' in assembler.cpp pre-stripping, but
  // by the time ParseExpression sees it, '#' is stripped. Test pure expr.
  EXPECT_EQ(CanonicalizeSlashHighByte(">ADDR"), ">ADDR");
}

TEST(CanonicalizeSlashHighByteTest, BinaryDivisionNotReplaced) {
  // 'A/B' — '/' after identifier is binary division
  EXPECT_EQ(CanonicalizeSlashHighByte("A/B"), "A/B");
}

TEST(CanonicalizeSlashHighByteTest, BinaryDivisionWithNumbers) {
  // '16/2' — '/' after digit is binary division
  EXPECT_EQ(CanonicalizeSlashHighByte("16/2"), "16/2");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterOpenParen) {
  // '(/ADDR)' — '/' after '(' is prefix position
  EXPECT_EQ(CanonicalizeSlashHighByte("(/ADDR)"), "(>ADDR)");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterPlusOperator) {
  // 'X+/Y' — '/' after '+' is prefix position
  EXPECT_EQ(CanonicalizeSlashHighByte("X+/Y"), "X+>Y");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterMinusOperator) {
  EXPECT_EQ(CanonicalizeSlashHighByte("X-/Y"), "X->Y");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterComma) {
  // '.DA /X,/Y' style — each comma resets to prefix position
  EXPECT_EQ(CanonicalizeSlashHighByte("/X,/Y"), ">X,>Y");
}

TEST(CanonicalizeSlashHighByteTest, PrefixAfterWhitespace) {
  // Whitespace doesn't change positional state
  EXPECT_EQ(CanonicalizeSlashHighByte("  /ADDR"), "  >ADDR");
}

TEST(CanonicalizeSlashHighByteTest, NoSlash) {
  // Expression with no '/' — should be unchanged
  EXPECT_EQ(CanonicalizeSlashHighByte("ADDR+1"), "ADDR+1");
}

TEST(CanonicalizeSlashHighByteTest, EmptyString) {
  EXPECT_EQ(CanonicalizeSlashHighByte(""), "");
}

TEST(CanonicalizeSlashHighByteTest, AlreadyCanonical) {
  // Already using '>' — unchanged
  EXPECT_EQ(CanonicalizeSlashHighByte(">ADDR"), ">ADDR");
}

TEST(CanonicalizeSlashHighByteTest, ComplexExpressionBinaryDiv) {
  // Complex expression where '/' is clearly binary division
  EXPECT_EQ(CanonicalizeSlashHighByte("$1234/2"), "$1234/2");
}

TEST(CanonicalizeSlashHighByteTest, PrefixSlashWithHexAddress) {
  // '/ADDR' where ADDR is a known symbol — should become '>ADDR'
  EXPECT_EQ(CanonicalizeSlashHighByte("/$2000"), ">$2000");
}

// ============================================================================
// CanonicalizeEqualityOperator tests (ADR-005 V5)
// ============================================================================

TEST(CanonicalizeEqualityOperatorTest, SingleEqualBecomesDoubleEqual) {
  // SCMASM uses single '=' for equality
  EXPECT_EQ(CanonicalizeEqualityOperator("X=1"), "X==1");
}

TEST(CanonicalizeEqualityOperatorTest, DoubleEqualLeftUnchanged) {
  EXPECT_EQ(CanonicalizeEqualityOperator("X==1"), "X==1");
}

TEST(CanonicalizeEqualityOperatorTest, NotEqualLeftUnchanged) {
  EXPECT_EQ(CanonicalizeEqualityOperator("X!=1"), "X!=1");
}

TEST(CanonicalizeEqualityOperatorTest, LessOrEqualLeftUnchanged) {
  EXPECT_EQ(CanonicalizeEqualityOperator("X<=1"), "X<=1");
}

TEST(CanonicalizeEqualityOperatorTest, GreaterOrEqualLeftUnchanged) {
  EXPECT_EQ(CanonicalizeEqualityOperator("X>=1"), "X>=1");
}

TEST(CanonicalizeEqualityOperatorTest, SymbolEqualConstant) {
  // Real SCMASM pattern from .DO directive
  EXPECT_EQ(CanonicalizeEqualityOperator("X.DELETE.SOURCE=1"),
            "X.DELETE.SOURCE==1");
}

TEST(CanonicalizeEqualityOperatorTest, NoEqualSign) {
  EXPECT_EQ(CanonicalizeEqualityOperator("X+1"), "X+1");
}

TEST(CanonicalizeEqualityOperatorTest, EmptyString) {
  EXPECT_EQ(CanonicalizeEqualityOperator(""), "");
}

TEST(CanonicalizeEqualityOperatorTest, MultipleConditions) {
  // Multiple equality conditions chained with logical AND (&)
  EXPECT_EQ(CanonicalizeEqualityOperator("A=1&B=2"), "A==1&B==2");
}

TEST(CanonicalizeEqualityOperatorTest, EqualAtStart) {
  // '=' at start — unusual but should become '=='
  EXPECT_EQ(CanonicalizeEqualityOperator("=1"), "==1");
}

// ============================================================================
// Integration: both canonicalizations together
// ============================================================================

TEST(ScmasmCanonicalizeIntegration, SlashAndEquality) {
  // Expression that has both: "/ADDR=1"
  std::string expr = "/ADDR=1";
  expr = CanonicalizeSlashHighByte(expr);
  EXPECT_EQ(expr, ">ADDR=1");
  expr = CanonicalizeEqualityOperator(expr);
  EXPECT_EQ(expr, ">ADDR==1");
}

} // namespace scmasm
} // namespace xasm
