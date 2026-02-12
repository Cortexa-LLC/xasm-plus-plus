/**
 * @file test_scmasm_directive_registry.cpp
 * @brief Unit tests for SCMASM DirectiveRegistry pattern
 *
 * Tests that directives are correctly registered and dispatched through
 * the registry pattern.
 */

#include "xasm++/syntax/scmasm_syntax.h"
#include <gtest/gtest.h>

using namespace xasm;

/**
 * @brief Test fixture for DirectiveRegistry tests
 */
class DirectiveRegistryTest : public ::testing::Test {
protected:
  void SetUp() override {
    parser = std::make_unique<ScmasmSyntaxParser>();
    section = Section();
    symbols = ConcreteSymbolTable();
  }

  std::unique_ptr<ScmasmSyntaxParser> parser;
  Section section;
  ConcreteSymbolTable symbols;
};

/**
 * @brief Test that .OR directive is registered and works
 */
TEST_F(DirectiveRegistryTest, OrDirectiveRegistered) {
  std::string source = "  .OR $1000\n"
                       "  LDA #$00\n";
  parser->Parse(source, section, symbols);

  // .OR should set the origin - atoms should start at $1000
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .EQ directive is registered and works
 */
TEST_F(DirectiveRegistryTest, EqDirectiveRegistered) {
  std::string source = "VAL .EQ $42\n";
  parser->Parse(source, section, symbols);

  // VAL should be defined as a constant
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("VAL", value));
  EXPECT_EQ(value, 0x42);
}

/**
 * @brief Test that .SE directive is registered and works
 */
TEST_F(DirectiveRegistryTest, SeDirectiveRegistered) {
  std::string source = "START .SE $2000\n"
                       "      LDA #$00\n";
  parser->Parse(source, section, symbols);

  // START should be defined
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("START", value));
  EXPECT_EQ(value, 0x2000);
}

/**
 * @brief Test that .AS (ASCII string) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, AsDirectiveRegistered) {
  std::string source = "  .AS \"HELLO\"\n";
  parser->Parse(source, section, symbols);

  // Should create data atoms for the string
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .DA directive is registered and works
 */
TEST_F(DirectiveRegistryTest, DaDirectiveRegistered) {
  std::string source = "  .DA $12,$34\n";
  parser->Parse(source, section, symbols);

  // Should create data atoms
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .DFB (alias for .DA) is registered and works
 */
TEST_F(DirectiveRegistryTest, DfbAliasRegistered) {
  std::string source = "  .DFB $AB,$CD\n";
  parser->Parse(source, section, symbols);

  // .DFB should work the same as .DA
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .HS (hex string) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, HsDirectiveRegistered) {
  std::string source = "  .HS 0123456789ABCDEF\n";
  parser->Parse(source, section, symbols);

  // Should create data atoms for hex bytes
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .BS (bit string) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, BsDirectiveRegistered) {
  std::string source = "  .BS 10101010\n";  // Binary string (8 bits = 1 byte)
  parser->Parse(source, section, symbols);

  // .BS should create a data atom
  EXPECT_FALSE(section.atoms.empty());
}

/**
 * @brief Test that .MA (macro definition) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, MaDirectiveRegistered) {
  std::string source = "MYMAC .MA\n"
                       "      LDA #$00\n"
                       "      .EM\n";
  parser->Parse(source, section, symbols);

  // Macro should be defined (we can't easily test this without exposing
  // internal state, but we can verify no exception was thrown)
  SUCCEED();
}

/**
 * @brief Test that .EM (end macro) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, EmDirectiveRegistered) {
  std::string source = "TEST .MA\n"
                       "     .EM\n";

  // Should not throw
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

/**
 * @brief Test that .ENDM (alias for .EM) is registered and works
 */
TEST_F(DirectiveRegistryTest, EndmAliasRegistered) {
  std::string source = "TEST .MA\n"
                       "     LDA #$00\n"
                       "     .ENDM\n"
                       "     LDA #$FF\n";  // Code after macro should parse

  // .ENDM should work as alias for .EM
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

/**
 * @brief Test that unknown directives throw appropriate error
 */
TEST_F(DirectiveRegistryTest, UnknownDirectiveThrows) {
  std::string source = "  .INVALID\n";

  // Unknown directive should throw
  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

/**
 * @brief Test that .EQ requires a label
 */
TEST_F(DirectiveRegistryTest, EqRequiresLabel) {
  std::string source = "  .EQ $42\n";

  // .EQ without label should throw
  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

/**
 * @brief Test that .SE requires a label
 */
TEST_F(DirectiveRegistryTest, SeRequiresLabel) {
  std::string source = "  .SE $1000\n";

  // .SE without label should throw
  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

/**
 * @brief Test that control flow directives are NOT in registry
 *
 * Control flow directives (.DO, .ELSE, .FIN, .LU, .ENDU) require special
 * handling for line skipping and cannot be dispatched via simple registry.
 */
TEST_F(DirectiveRegistryTest, ControlFlowDirectivesSpecialHandled) {
  // .DO should work (but is specially handled, not via registry)
  std::string source_do = "  .DO 1\n"
                          "  LDA #$00\n"
                          "  .FIN\n";
  EXPECT_NO_THROW(parser->Parse(source_do, section, symbols));

  // .LU should work (but is specially handled, not via registry)
  parser = std::make_unique<ScmasmSyntaxParser>();
  section = Section();
  std::string source_lu = "  .LU 2\n"
                          "  LDA #$00\n"
                          "  .ENDU\n";
  EXPECT_NO_THROW(parser->Parse(source_lu, section, symbols));
}

/**
 * @brief Test that mismatched control flow directives throw errors
 */
TEST_F(DirectiveRegistryTest, MismatchedControlFlowThrows) {
  // .ELSE without .DO should throw
  std::string source_else = "  .ELSE\n";
  EXPECT_THROW(parser->Parse(source_else, section, symbols), std::runtime_error);

  // .FIN without .DO should throw
  parser = std::make_unique<ScmasmSyntaxParser>();
  section = Section();
  std::string source_fin = "  .FIN\n";
  EXPECT_THROW(parser->Parse(source_fin, section, symbols), std::runtime_error);

  // .ENDU without .LU should throw
  parser = std::make_unique<ScmasmSyntaxParser>();
  section = Section();
  std::string source_endu = "  .ENDU\n";
  EXPECT_THROW(parser->Parse(source_endu, section, symbols),
               std::runtime_error);
}

/**
 * @brief Test that directives are case-insensitive
 */
TEST_F(DirectiveRegistryTest, DirectivesAreCaseInsensitive) {
  // Test lowercase directive
  std::string source_lower = "VAL1 .eq $1000\n";
  EXPECT_NO_THROW(parser->Parse(source_lower, section, symbols));
  
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("VAL1", value));
  EXPECT_EQ(value, 0x1000);

  // Test mixed case directive
  parser = std::make_unique<ScmasmSyntaxParser>();
  section = Section();
  std::string source_mixed = "VAL2 .Eq $2000\n";
  EXPECT_NO_THROW(parser->Parse(source_mixed, section, symbols));
  
  ASSERT_TRUE(symbols.Lookup("VAL2", value));
  EXPECT_EQ(value, 0x2000);
}
