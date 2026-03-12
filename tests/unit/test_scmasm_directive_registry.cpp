/**
 * @file test_scmasm_directive_registry.cpp
 * @brief Unit tests for SCMASM DirectiveRegistry pattern
 *
 * Tests that directives are correctly registered and dispatched through
 * the registry pattern.
 */

#include "xasm++/syntax/scmasm_syntax.h"
#include "xasm++/atom.h"
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
 * @brief Test that .BS (block storage) directive is registered and works
 */
TEST_F(DirectiveRegistryTest, BsDirectiveRegistered) {
  std::string source = "  .BS 256\n"; // Block storage - reserve 256 bytes
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
                       "     LDA #$FF\n"; // Code after macro should parse

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
  EXPECT_THROW(parser->Parse(source_else, section, symbols),
               std::runtime_error);

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

/**
 * @brief Test that .AZ -"text" sets high bit on ALL bytes (Apple II encoding)
 *
 * Regression test for bug where '-' was treated as the string delimiter,
 * causing truncation at the next '-' in the string content.
 * See: A2osX PM.NSC.S.txt uses .AZ -"NSC 'No-Slot-Clock'/..." which was
 * being truncated at "No-Slot" (the '-' in "No-Slot" was the false delimiter).
 */
TEST_F(DirectiveRegistryTest, AzDirectiveHighBitPrefixSetsAllBytes) {
  // .AZ -"text" should set HIGH BIT on ALL bytes, with null terminator at end
  std::string source = "        .AZ -\"Hi\"\n";
  parser->Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3u); // 2 chars + null
  EXPECT_EQ(data_atom->data[0], 0xC8);   // 'H' | 0x80
  EXPECT_EQ(data_atom->data[1], 0xE9);   // 'i' | 0x80
  EXPECT_EQ(data_atom->data[2], 0x00);   // null terminator (no high bit)
}

/**
 * @brief Test that .AZ -"text" does NOT truncate at '-' within string content
 *
 * Regression test: .AZ -"No-Slot" should emit the FULL string "No-Slot"
 * with high bits set, not just "No" (truncated at the '-' in "No-Slot").
 */
TEST_F(DirectiveRegistryTest, AzDirectiveHighBitPrefixNoDashTruncation) {
  // '-' in string content must NOT be treated as a delimiter
  std::string source = "        .AZ -\"No-Slot\"\n";
  parser->Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // "No-Slot" = 7 chars + null = 8 bytes
  ASSERT_EQ(data_atom->data.size(), 8u);
  EXPECT_EQ(data_atom->data[0], 0x4E | 0x80); // 'N' | 0x80
  EXPECT_EQ(data_atom->data[1], 0x6F | 0x80); // 'o' | 0x80
  EXPECT_EQ(data_atom->data[2], 0x2D | 0x80); // '-' | 0x80 (dash in string)
  EXPECT_EQ(data_atom->data[3], 0x53 | 0x80); // 'S' | 0x80
  EXPECT_EQ(data_atom->data[7], 0x00);         // null terminator (no high bit)
}

/**
 * @brief Test that .DA #'char' emits ASCII value, not 0x00
 *
 * Bug C: .DA #'N' should emit 0x4E (ASCII 'N'), but was emitting 0x00.
 * The '#' prefix requests 1-byte immediate form, and the character literal
 * 'N' should be expanded to its ASCII value before evaluation.
 */
TEST_F(DirectiveRegistryTest, DaDirectiveCharLiteralImmediate) {
  // Test #'N' character literal with immediate prefix
  std::string source = "        .DA #'N'\n";
  parser->Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // Should emit 1 byte: 0x4E (ASCII 'N')
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0x4E); // 'N' = ASCII 78 = $4E
}

/**
 * @brief Test that .DA #'X',#'Y' emits correct ASCII values
 *
 * Multiple character literals should each be expanded correctly.
 */
TEST_F(DirectiveRegistryTest, DaDirectiveMultipleCharLiterals) {
  std::string source = "        .DA #'A',#'B',#'C'\n";
  parser->Parse(source, section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  // Should emit 3 bytes
  ASSERT_EQ(data_atom->data.size(), 3u);
  EXPECT_EQ(data_atom->data[0], 0x41); // 'A' = $41
  EXPECT_EQ(data_atom->data[1], 0x42); // 'B' = $42
  EXPECT_EQ(data_atom->data[2], 0x43); // 'C' = $43
}
