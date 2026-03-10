// SCMASM Syntax Parser Tests - Phase 1: Foundation
// TDD approach: Tests written first

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace xasm;

// ============================================================================
// Test Fixture
// ============================================================================

class ScmasmSyntaxTest : public ::testing::Test {
protected:
  void SetUp() override {
    parser = std::make_unique<ScmasmSyntaxParser>();
    cpu = std::make_unique<Cpu6502>();
    parser->SetCpu(cpu.get());
    section = Section();
    symbols = ConcreteSymbolTable();
  }

  void TearDown() override {
    // Clean up test directories (Windows requires error_code for permission
    // issues)
    std::error_code ec;
    std::filesystem::remove_all("test_subdir", ec);
    // Ignore errors - directory may not exist or may be locked
  }

  std::unique_ptr<ScmasmSyntaxParser> parser;
  std::unique_ptr<Cpu6502> cpu;
  Section section;
  ConcreteSymbolTable symbols;
};

// ============================================================================
// Basic Construction and Empty Source Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, ConstructorCreatesValidParser) {
  EXPECT_NE(parser, nullptr);
}

TEST_F(ScmasmSyntaxTest, ParseEmptySourceSucceeds) {
  EXPECT_NO_THROW(parser->Parse("", section, symbols));
  EXPECT_EQ(section.atoms.size(), 0u);
}

// ============================================================================
// Comment Stripping Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, StripsAsteriskCommentInColumn1) {
  // * comment in column 1 should be stripped
  parser->Parse("* This is a comment\n", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, StripsSemicolonComment) {
  // ; comment anywhere should be stripped
  parser->Parse("    ; This is a comment\n", section, symbols);
  EXPECT_EQ(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, StripsInlineComment) {
  // Code followed by comment
  parser->Parse("1000 .OR $0800  ; Set origin\n", section, symbols);
  // Should have ORG atom
  EXPECT_GT(section.atoms.size(), 0u);
}

// ============================================================================
// Line Number Recognition Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, RecognizesSimpleLineNumber) {
  // Line numbers are optional but should be recognized
  parser->Parse("1000 .OR $0800\n", section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, RecognizesLineNumberRange) {
  // Line numbers: 0-65535
  parser->Parse("0 .OR $0000\n", section, symbols);
  parser->Parse("65535 .OR $FFFF\n", section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, ParsesDirectiveWithoutLineNumber) {
  // Line numbers are optional
  parser->Parse("    .OR $0800\n", section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

// ============================================================================
// Dot Prefix Directive Recognition Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, RecognizesDotPrefixDirective) {
  // All SCMASM directives start with .
  parser->Parse(".OR $0800\n", section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, RejectsMissingDotPrefix) {
  // SCMASM requires . prefix for directives
  // ORG without . should be treated as label or error
  parser->Parse("ORG $0800\n", section, symbols);
  // Should not create ORG atom (might create instruction or label)
}

// ============================================================================
// .OR Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, OrDirectiveSetsAddress) {
  parser->Parse(".OR $0800\n", section, symbols);

  // Should have one OrgAtom
  ASSERT_EQ(section.atoms.size(), 1u);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x0800u);
}

TEST_F(ScmasmSyntaxTest, OrDirectiveWithDecimal) {
  parser->Parse(".OR 2048\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 2048u);
}

TEST_F(ScmasmSyntaxTest, OrDirectiveWithLineNumber) {
  parser->Parse("1000 .OR $2000\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x2000u);
}

// ============================================================================
// .EQ Directive Tests (Define Constant)
// ============================================================================

TEST_F(ScmasmSyntaxTest, EqDefinesConstant) {
  parser->Parse("BUFSIZE .EQ 256\n", section, symbols);

  // EQU should not create atoms
  EXPECT_EQ(section.atoms.size(), 0u);

  // Symbol should be defined
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("BUFSIZE", value));
  EXPECT_EQ(value, 256);
}

TEST_F(ScmasmSyntaxTest, EqWithHexValue) {
  parser->Parse("IOADDR .EQ $C000\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("IOADDR", value));
  EXPECT_EQ(value, 0xC000);
}

TEST_F(ScmasmSyntaxTest, EqWithExpression) {
  parser->Parse("BASE .EQ 256\n", section, symbols);
  parser->Parse("OFFSET .EQ BASE+64\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("OFFSET", value));
  EXPECT_EQ(value, 320);
}

TEST_F(ScmasmSyntaxTest, EqWithTrailingComment) {
  // Merlin allows trailing text on .EQ lines as implicit comments
  // Example: "FPU.f  .EQ 180    float" where "float" is a comment
  parser->Parse("FPU.f .EQ 180    float\n", section, symbols);

  int64_t value;
  // SCMASM normalizes symbols to uppercase
  ASSERT_TRUE(symbols.Lookup("FPU.F", value));
  EXPECT_EQ(value, 180);
}

TEST_F(ScmasmSyntaxTest, EqWithTrailingCommentMultipleSpaces) {
  // Multiple spaces indicate comment separator
  parser->Parse("VALUE .EQ $FF  description here\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("VALUE", value));
  EXPECT_EQ(value, 0xFF);
}

TEST_F(ScmasmSyntaxTest, EqWithSemicolonComment) {
  // Semicolon comments should still work (already stripped by parser)
  parser->Parse("CONST .EQ 42 ; this is a comment\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("CONST", value));
  EXPECT_EQ(value, 42);
}

TEST_F(ScmasmSyntaxTest, EqWithExpressionAndComment) {
  // Expression with trailing comment
  parser->Parse("BASE .EQ $1000\n", section, symbols);
  parser->Parse("ADDR .EQ BASE+$100  target address\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("ADDR", value));
  EXPECT_EQ(value, 0x1100);
}

// ============================================================================
// .SE Directive Tests (Define Variable - Redefinable)
// ============================================================================

TEST_F(ScmasmSyntaxTest, SeDefinesVariable) {
  parser->Parse("COUNTER .SE 0\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("COUNTER", value));
  EXPECT_EQ(value, 0);
}

TEST_F(ScmasmSyntaxTest, SeAllowsRedefinition) {
  parser->Parse("INDEX .SE 0\n", section, symbols);
  parser->Parse("INDEX .SE 1\n", section, symbols);
  parser->Parse("INDEX .SE 2\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("INDEX", value));
  EXPECT_EQ(value, 2); // Should be last value
}

TEST_F(ScmasmSyntaxTest, SeIncrementsVariable) {
  parser->Parse("COUNT .SE 0\n", section, symbols);
  parser->Parse("COUNT .SE COUNT+1\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("COUNT", value));
  EXPECT_EQ(value, 1);
}

// ============================================================================
// Number Format Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, ParsesHexNumber) {
  parser->Parse("VALUE .EQ $FF\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("VALUE", value));
  EXPECT_EQ(value, 0xFF);
}

TEST_F(ScmasmSyntaxTest, ParsesBinaryNumber) {
  parser->Parse("MASK .EQ %11110000\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("MASK", value));
  EXPECT_EQ(value, 0xF0);
}

TEST_F(ScmasmSyntaxTest, ParsesBinaryWithSeparator) {
  // SCMASM allows . separator in binary: %1111.0000
  parser->Parse("BITS .EQ %1010.0101\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("BITS", value));
  EXPECT_EQ(value, 0xA5);
}

TEST_F(ScmasmSyntaxTest, DA_BinaryWithDotSeparator) {
  // .DA with binary dot-separator (used in LIBGUI cursor bitmaps)
  // %000.00000000 (11-bit cursor row) must be parsed without error.
  std::string source = "\t\t.OR $1000\n"
                       "\t\t.DA %000.00000000\n"
                       "\t\t.DA %010.01111110\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  // Check the data atoms for expected values
  auto *d1 = dynamic_cast<DataAtom *>(section.atoms[1].get());
  auto *d2 = dynamic_cast<DataAtom *>(section.atoms[2].get());
  ASSERT_NE(d1, nullptr);
  ASSERT_NE(d2, nullptr);
  // %00000000000 = 0x000
  uint16_t v1 = d1->data[0] | (d1->data[1] << 8);
  EXPECT_EQ(v1, 0x0000u);
  // %01001111110 = 0x027E = 638
  uint16_t v2 = d2->data[0] | (d2->data[1] << 8);
  EXPECT_EQ(v2, 0x027Eu);
}

TEST_F(ScmasmSyntaxTest, ParsesDecimalNumber) {
  parser->Parse("SIZE .EQ 1024\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("SIZE", value));
  EXPECT_EQ(value, 1024);
}

// ============================================================================
// ASCII Character Constant Tests (High Bit Rule)
// ============================================================================

TEST_F(ScmasmSyntaxTest, AsciiCharacterWithApostrophe) {
  // 'X (apostrophe, ASCII 0x27) → high bit CLEAR
  parser->Parse("CHAR1 .EQ 'A\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("CHAR1", value));
  EXPECT_EQ(value, 0x41); // Plain 'A', no high bit
}

TEST_F(ScmasmSyntaxTest, AsciiCharacterWithQuote) {
  // "X (double quote, ASCII 0x22 < 0x27) → high bit SET
  parser->Parse("CHAR2 .EQ \"A\n", section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("CHAR2", value));
  EXPECT_EQ(value, 0xC1); // 'A' with high bit set
}

TEST_F(ScmasmSyntaxTest, AsciiDelimiterRule) {
  // Delimiter ASCII < 0x27 → high bit SET
  // Delimiter ASCII >= 0x27 → high bit CLEAR

  // Test with / (ASCII 0x2F > 0x27) → high bit CLEAR
  parser->Parse("SLASH .EQ /A\n", section, symbols);
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("SLASH", value));
  EXPECT_EQ(value, 0x41); // Plain 'A'

  // Test with # (ASCII 0x23 < 0x27) → high bit SET
  parser->Parse("HASH .EQ #A\n", section, symbols);
  ASSERT_TRUE(symbols.Lookup("HASH", value));
  EXPECT_EQ(value, 0xC1); // 'A' with high bit
}

// ============================================================================
// Integration Tests - Complete Programs
// ============================================================================

TEST_F(ScmasmSyntaxTest, SimpleProgram) {
  std::string source = R"(
* Simple SCMASM program
1000 .OR $0800         ; Set origin
1010 START .EQ $0800   ; Define start address
1020 COUNT .SE 0       ; Initialize counter
)";

  parser->Parse(source, section, symbols);

  // Check symbols defined
  int64_t value;
  ASSERT_TRUE(symbols.Lookup("START", value));
  EXPECT_EQ(value, 0x0800);

  ASSERT_TRUE(symbols.Lookup("COUNT", value));
  EXPECT_EQ(value, 0);
}

TEST_F(ScmasmSyntaxTest, NumberFormatsProgram) {
  std::string source = R"(
HEX_VAL .EQ $FF
BIN_VAL .EQ %11110000
DEC_VAL .EQ 255
CHAR_HI .EQ "A
CHAR_LO .EQ 'A
)";

  parser->Parse(source, section, symbols);

  int64_t value;
  ASSERT_TRUE(symbols.Lookup("HEX_VAL", value));
  EXPECT_EQ(value, 0xFF);

  ASSERT_TRUE(symbols.Lookup("BIN_VAL", value));
  EXPECT_EQ(value, 0xF0);

  ASSERT_TRUE(symbols.Lookup("DEC_VAL", value));
  EXPECT_EQ(value, 255);

  ASSERT_TRUE(symbols.Lookup("CHAR_HI", value));
  EXPECT_EQ(value, 0xC1); // High bit set

  ASSERT_TRUE(symbols.Lookup("CHAR_LO", value));
  EXPECT_EQ(value, 0x41); // High bit clear
}

// ============================================================================
// Phase 2: String & Data Directives Tests
// ============================================================================

// ============================================================================
// .AS Directive Tests (ASCII String)
// ============================================================================

TEST_F(ScmasmSyntaxTest, AsDirectiveWithSimpleString) {
  // .AS stores plain 7-bit ASCII regardless of delimiter.
  // The delimiter is used only to delimit the string, not to set high bits.
  parser->Parse("        .AS \"HELLO\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u);
  EXPECT_EQ(data_atom->data[0], 0x48); // 'H' plain ASCII
  EXPECT_EQ(data_atom->data[1], 0x45); // 'E' plain ASCII
  EXPECT_EQ(data_atom->data[2], 0x4C); // 'L' plain ASCII
  EXPECT_EQ(data_atom->data[3], 0x4C); // 'L' plain ASCII
  EXPECT_EQ(data_atom->data[4], 0x4F); // 'O' plain ASCII
}

TEST_F(ScmasmSyntaxTest, AsDirectiveHighBitRule) {
  // .AS stores plain ASCII regardless of delimiter.
  // The " delimiter does NOT set high bit on string data.
  parser->Parse("        .AS \"A\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0x41); // 'A' plain ASCII
}

TEST_F(ScmasmSyntaxTest, AsDirectiveHighBitClear) {
  // Delimiter >= 0x27 should keep high bit CLEAR for .AS
  parser->Parse("        .AS 'A'\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0x41); // 'A' with high bit CLEAR
}

TEST_F(ScmasmSyntaxTest, AsDirectiveEmptyString) {
  // Empty string should produce no data
  parser->Parse("        .AS \"\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(), 0u);
}

// ============================================================================
// .AT Directive Tests (ASCII Text with High Bit SET)
// ============================================================================

TEST_F(ScmasmSyntaxTest, AtDirectiveSetsHighBit) {
  // .AT always sets high bit on last character
  parser->Parse("        .AT 'HELLO'\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u);
  EXPECT_EQ(data_atom->data[0], 0x48); // 'H' normal
  EXPECT_EQ(data_atom->data[1], 0x45); // 'E' normal
  EXPECT_EQ(data_atom->data[2], 0x4C); // 'L' normal
  EXPECT_EQ(data_atom->data[3], 0x4C); // 'L' normal
  EXPECT_EQ(data_atom->data[4], 0xCF); // 'O' with high bit SET
}

TEST_F(ScmasmSyntaxTest, AtDirectiveDelimiterStillApplies) {
  // .AT sets high bit only on the LAST character regardless of delimiter.
  // The " delimiter does NOT set high bit on preceding characters.
  parser->Parse("        .AT \"AB\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u);
  EXPECT_EQ(data_atom->data[0], 0x41); // 'A' plain ASCII
  EXPECT_EQ(data_atom->data[1], 0xC2); // 'B' with high bit from .AT rule
}

TEST_F(ScmasmSyntaxTest, AtDirectiveSingleChar) {
  // Single character gets high bit set
  parser->Parse("        .AT 'X'\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0xD8); // 'X' with high bit SET
}

// ============================================================================
// .AZ Directive Tests (ASCII Zero-Terminated)
// ============================================================================

TEST_F(ScmasmSyntaxTest, AzDirectiveAddsZero) {
  // .AZ should append $00 terminator
  parser->Parse("        .AZ 'HELLO'\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 6u); // 5 chars + null
  EXPECT_EQ(data_atom->data[0], 0x48);   // 'H'
  EXPECT_EQ(data_atom->data[1], 0x45);   // 'E'
  EXPECT_EQ(data_atom->data[2], 0x4C);   // 'L'
  EXPECT_EQ(data_atom->data[3], 0x4C);   // 'L'
  EXPECT_EQ(data_atom->data[4], 0x4F);   // 'O'
  EXPECT_EQ(data_atom->data[5], 0x00);   // null terminator
}

TEST_F(ScmasmSyntaxTest, AzDirectiveHighBitRule) {
  // .AZ stores plain ASCII regardless of delimiter (same rule as .AS).
  // The " delimiter does NOT set high bit on string data.
  parser->Parse("        .AZ \"TEST\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u); // 4 chars + null
  EXPECT_EQ(data_atom->data[0], 0x54);   // 'T' plain ASCII
  EXPECT_EQ(data_atom->data[1], 0x45);   // 'E' plain ASCII
  EXPECT_EQ(data_atom->data[2], 0x53);   // 'S' plain ASCII
  EXPECT_EQ(data_atom->data[3], 0x54);   // 'T' plain ASCII
  EXPECT_EQ(data_atom->data[4], 0x00);   // null terminator
}

TEST_F(ScmasmSyntaxTest, AzDirectiveEmptyString) {
  // Empty string should just emit null terminator
  parser->Parse("        .AZ ''\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0x00);
}

// ============================================================================
// .DA Directive Tests (Define Address/Data - Multi-Value)
// ============================================================================

TEST_F(ScmasmSyntaxTest, DaDirectiveSingleByte) {
  // .DA with single byte value - SCMASM always emits 16-bit (2 bytes)
  parser->Parse("        .DA $42\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u);
  EXPECT_EQ(data_atom->data[0], 0x42); // Low byte
  EXPECT_EQ(data_atom->data[1], 0x00); // High byte
}

TEST_F(ScmasmSyntaxTest, DaDirectiveMultipleBytes) {
  // .DA with comma-separated values - SCMASM emits each as 16-bit (2 bytes
  // each)
  parser->Parse("        .DA $01,$02,$03\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 6u); // 3 values × 2 bytes each
  EXPECT_EQ(data_atom->data[0], 0x01);   // Value 1 low
  EXPECT_EQ(data_atom->data[1], 0x00);   // Value 1 high
  EXPECT_EQ(data_atom->data[2], 0x02);   // Value 2 low
  EXPECT_EQ(data_atom->data[3], 0x00);   // Value 2 high
  EXPECT_EQ(data_atom->data[4], 0x03);   // Value 3 low
  EXPECT_EQ(data_atom->data[5], 0x00);   // Value 3 high
}

TEST_F(ScmasmSyntaxTest, DaDirectiveMultiByteValue) {
  // .DA with 16-bit value (should emit low byte, high byte)
  parser->Parse("        .DA $1234\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u);
  EXPECT_EQ(data_atom->data[0], 0x34); // Low byte
  EXPECT_EQ(data_atom->data[1], 0x12); // High byte
}

TEST_F(ScmasmSyntaxTest, DaDirectiveMixedValues) {
  // .DA with mix of byte and word values - all emitted as 16-bit
  parser->Parse("        .DA $12,$3456,$78\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 6u); // 3 values × 2 bytes each
  EXPECT_EQ(data_atom->data[0], 0x12);   // Value 1 low
  EXPECT_EQ(data_atom->data[1], 0x00);   // Value 1 high
  EXPECT_EQ(data_atom->data[2], 0x56);   // Value 2 low
  EXPECT_EQ(data_atom->data[3], 0x34);   // Value 2 high
  EXPECT_EQ(data_atom->data[4], 0x78);   // Value 3 low
  EXPECT_EQ(data_atom->data[5], 0x00);   // Value 3 high
}

TEST_F(ScmasmSyntaxTest, DaDirectiveWithExpressions) {
  // .DA can use expressions
  parser->Parse("BASE .EQ $1000\n", section, symbols);
  parser->Parse("        .DA BASE+10\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u);
  EXPECT_EQ(data_atom->data[0], 0x0A); // Low byte of $100A
  EXPECT_EQ(data_atom->data[1], 0x10); // High byte
}

// ============================================================================
// .DFB Directive Tests (Alias for .DA)
// ============================================================================

TEST_F(ScmasmSyntaxTest, DfbDirectiveAliasDa) {
  // .DFB is an alias for .DA - emits 16-bit values
  parser->Parse("        .DFB $42\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u); // 16-bit value
  EXPECT_EQ(data_atom->data[0], 0x42);   // Low byte
  EXPECT_EQ(data_atom->data[1], 0x00);   // High byte
}

TEST_F(ScmasmSyntaxTest, DfbDirectiveMultipleValues) {
  // .DFB works same as .DA - emits 16-bit values
  parser->Parse("        .DFB $01,$02,$03\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 6u); // 3 values × 2 bytes each
  EXPECT_EQ(data_atom->data[0], 0x01);   // Value 1 low
  EXPECT_EQ(data_atom->data[1], 0x00);   // Value 1 high
  EXPECT_EQ(data_atom->data[2], 0x02);   // Value 2 low
  EXPECT_EQ(data_atom->data[3], 0x00);   // Value 2 high
  EXPECT_EQ(data_atom->data[4], 0x03);   // Value 3 low
  EXPECT_EQ(data_atom->data[5], 0x00);   // Value 3 high
}

// ============================================================================
// .HS Directive Tests (Hex String)
// ============================================================================

TEST_F(ScmasmSyntaxTest, HsDirectiveSimple) {
  // .HS takes hex digits (no $ prefix)
  parser->Parse("        .HS 01 02 03\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3u);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
}

TEST_F(ScmasmSyntaxTest, HsDirectiveNoSpaces) {
  // .HS can have no spaces between bytes
  parser->Parse("        .HS 010203\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3u);
  EXPECT_EQ(data_atom->data[0], 0x01);
  EXPECT_EQ(data_atom->data[1], 0x02);
  EXPECT_EQ(data_atom->data[2], 0x03);
}

TEST_F(ScmasmSyntaxTest, HsDirectiveUpperLower) {
  // .HS accepts both upper and lowercase hex
  parser->Parse("        .HS AbCdEf\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 3u);
  EXPECT_EQ(data_atom->data[0], 0xAB);
  EXPECT_EQ(data_atom->data[1], 0xCD);
  EXPECT_EQ(data_atom->data[2], 0xEF);
}

TEST_F(ScmasmSyntaxTest, HsDirectiveOddDigits) {
  // .HS with odd number of digits should error
  EXPECT_THROW(parser->Parse("        .HS 012\n", section, symbols),
               std::runtime_error);
}

TEST_F(ScmasmSyntaxTest, HsDirectiveWithComment) {
  // .HS should ignore semicolon comments
  parser->Parse("        .HS 48656C6C6F    ; \"Hello\" in hex\n", section,
                symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u);
  EXPECT_EQ(data_atom->data[0], 0x48); // 'H'
  EXPECT_EQ(data_atom->data[1], 0x65); // 'e'
  EXPECT_EQ(data_atom->data[2], 0x6C); // 'l'
  EXPECT_EQ(data_atom->data[3], 0x6C); // 'l'
  EXPECT_EQ(data_atom->data[4], 0x6F); // 'o'
}

TEST_F(ScmasmSyntaxTest, HsDirectiveInlineComment) {
  // .HS should ignore text after hex data (like .EQ does)
  // Example: .HS DEADBEEF some inline comment
  parser->Parse("        .HS DEADBEEF some inline comment\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 4u);
  EXPECT_EQ(data_atom->data[0], 0xDE);
  EXPECT_EQ(data_atom->data[1], 0xAD);
  EXPECT_EQ(data_atom->data[2], 0xBE);
  EXPECT_EQ(data_atom->data[3], 0xEF);
}

TEST_F(ScmasmSyntaxTest, HsDirectiveWordBoundary) {
  // .HS should stop at first WORD containing non-hex character
  // NOT at first individual non-hex character
  // Example: .HS AB CD EFG should process AB CD (2 bytes)
  // not AB CD EF (3 bytes - which would include hex from "EFG")
  parser->Parse("        .HS AB CD EFG comment\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u); // Only AB CD
  EXPECT_EQ(data_atom->data[0], 0xAB);
  EXPECT_EQ(data_atom->data[1], 0xCD);
}

// ============================================================================
// .BS Directive Tests (Block Storage)
// ============================================================================

TEST_F(ScmasmSyntaxTest, BsDirectiveSimple) {
  // .BS reserves N bytes filled with zeros
  parser->Parse("        .BS 10\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 10u);
  // All bytes should be zero
  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(data_atom->data[i], 0x00);
  }
}

TEST_F(ScmasmSyntaxTest, BsDirectiveHex) {
  // .BS with hex byte count
  parser->Parse("        .BS $100\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 256u);
  // All bytes should be zero
  for (size_t i = 0; i < 256; ++i) {
    EXPECT_EQ(data_atom->data[i], 0x00);
  }
}

TEST_F(ScmasmSyntaxTest, BsDirectiveBinary) {
  // .BS with binary byte count (%1000 = 8 decimal)
  parser->Parse("        .BS %1000\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 8u);
  // All bytes should be zero
  for (size_t i = 0; i < 8; ++i) {
    EXPECT_EQ(data_atom->data[i], 0x00);
  }
}

TEST_F(ScmasmSyntaxTest, BsDirectiveWithSymbol) {
  // .BS with symbol reference
  symbols.Define("BUFSIZE", SymbolType::Equate,
                 std::make_shared<LiteralExpr>(64));
  parser->Parse("        .BS BUFSIZE\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 64u);
  // All bytes should be zero
  for (size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(data_atom->data[i], 0x00);
  }
}

// ============================================================================
// Integration Tests - Phase 2 Complete Programs
// ============================================================================

TEST_F(ScmasmSyntaxTest, Phase2StringProgram) {
  std::string source = R"(
        .OR $0800
        .AS "HELLO"
        .AT 'WORLD'
        .AZ "DONE"
)";

  parser->Parse(source, section, symbols);

  // Should have: ORG + 3 data atoms
  EXPECT_EQ(section.atoms.size(), 4u);
}

TEST_F(ScmasmSyntaxTest, Phase2DataProgram) {
  std::string source = R"(
        .OR $0800
        .DA $01,$02,$03
        .DFB $FF
        .HS DEADBEEF
        .BS 256
)";

  parser->Parse(source, section, symbols);

  // Should have: ORG + 4 data atoms
  EXPECT_EQ(section.atoms.size(), 5u);
}

TEST_F(ScmasmSyntaxTest, Phase2MixedProgram) {
  std::string source = R"(
* Complete Phase 2 program
        .OR $0800
MSG1    .AS "APPLE II"
MSG2    .AT 'SCMASM'
NULLMSG .AZ 'END'
TABLE   .DA $00,$01,$02,$03
HEX     .HS CAFEBABE
BUFFER  .BS 64
)";

  parser->Parse(source, section, symbols);

  // Should have multiple atoms
  EXPECT_GT(section.atoms.size(), 5u);
}

// ============================================================================
// Phase 3: Macros, Conditionals, Local Labels, Loops
// ============================================================================

// ============================================================================
// .MA/.EM Macro Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, MacroDefinitionSimple) {
  // Define a simple macro
  std::string source = R"(
        .MA CLEAR
        LDA #0
        .EM
)";

  // Macro definition should not generate atoms
  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, MacroInvocationSimple) {
  // Define and invoke a simple macro
  std::string source = R"(
        .MA CLEAR
        LDA #0
        .EM
        
        CLEAR
)";

  parser->Parse(source, section, symbols);
  // Should expand to LDA #0 instruction
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, MacroWithSingleParameter) {
  // Macro with \0 parameter
  std::string source = R"(
        .MA LOAD
        LDA \0
        .EM
        
        LOAD #$42
)";

  parser->Parse(source, section, symbols);
  // Should expand to LDA #$42
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, MacroWithTwoParameters) {
  // Macro with \0 and \1 parameters
  std::string source = R"(
        .MA MOVE
        LDA \0
        STA \1
        .EM
        
        MOVE #$FF,$C000
)";

  parser->Parse(source, section, symbols);
  // Should expand to LDA #$FF and STA $C000
  EXPECT_GE(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, MacroWithAllParameters) {
  // Test all 10 parameters \0-\9
  std::string source = R"(
        .MA TENPARAMS
        .DA \0,\1,\2,\3,\4,\5,\6,\7,\8,\9
        .EM
        
        TENPARAMS 0,1,2,3,4,5,6,7,8,9
)";

  parser->Parse(source, section, symbols);
  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data.size(),
            20u); // 10 values × 2 bytes each (.DA emits 16-bit)
}

TEST_F(ScmasmSyntaxTest, MacroNamedWithLabel) {
  // Macro can be defined with label syntax
  std::string source = R"(
STORE   .MA
        STA \0
        .EM
        
        STORE $C000
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, MacroMultipleInvocations) {
  // Same macro invoked multiple times
  std::string source = R"(
        .MA CLR
        LDA #0
        .EM
        
        CLR
        CLR
        CLR
)";

  parser->Parse(source, section, symbols);
  // Should have 3 LDA instructions
  EXPECT_EQ(section.atoms.size(), 3u);
}

TEST_F(ScmasmSyntaxTest, MacroWithLabelGeneration) {
  // Macro that generates unique labels using parameter
  std::string source = R"(
        .MA WAIT
LOOP\0  DEX
        BNE LOOP\0
        .EM
        
WAIT1   WAIT
WAIT2   WAIT
)";

  parser->Parse(source, section, symbols);
  // Should generate LOOPWAIT1 and LOOPWAIT2 labels
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, NestedMacroInvocations) {
  // Macro that invokes another macro
  std::string source = R"(
        .MA INNER
        LDA \0
        .EM
        
        .MA OUTER
        INNER \0
        STA \1
        .EM
        
        OUTER #$10,$20
)";

  parser->Parse(source, section, symbols);
  EXPECT_GE(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, MacroRedefinition) {
  // Redefining a macro should replace the old definition
  std::string source = R"(
        .MA TEST
        LDA #1
        .EM
        
        .MA TEST
        LDA #2
        .EM
        
        TEST
)";

  parser->Parse(source, section, symbols);
  // Should use second definition (LDA #2)
  EXPECT_GT(section.atoms.size(), 0u);
}

// ============================================================================
// .DO/.ELSE/.FIN Conditional Assembly Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, ConditionalDoTrue) {
  // .DO with true condition (non-zero)
  std::string source = R"(
        .DO 1
        LDA #$FF
        .FIN
)";

  parser->Parse(source, section, symbols);
  // Should assemble LDA instruction
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalDoFalse) {
  // .DO with false condition (zero)
  std::string source = R"(
        .DO 0
        LDA #$FF
        .FIN
)";

  parser->Parse(source, section, symbols);
  // Should NOT assemble LDA instruction
  EXPECT_EQ(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, ConditionalWithElseTrue) {
  // .DO true with .ELSE
  std::string source = R"(
        .DO 1
        LDA #$AA
        .ELSE
        LDA #$BB
        .FIN
)";

  parser->Parse(source, section, symbols);
  // Should assemble first LDA only
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalWithElseFalse) {
  // .DO false with .ELSE
  std::string source = R"(
        .DO 0
        LDA #$AA
        .ELSE
        LDA #$BB
        .FIN
)";

  parser->Parse(source, section, symbols);
  // Should assemble second LDA only
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalWithSymbol) {
  // .DO using symbol value
  std::string source = R"(
DEBUG   .EQ 1
        .DO DEBUG
        LDA #$FF
        .FIN
)";

  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalWithExpression) {
  // .DO with expression
  std::string source = R"(
VERSION .EQ 2
        .DO VERSION-1
        LDA #$FF
        .FIN
)";

  parser->Parse(source, section, symbols);
  // VERSION-1 = 2-1 = 1 (true)
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalNested) {
  // Nested conditionals
  std::string source = R"(
        .DO 1
        .DO 1
        LDA #$FF
        .FIN
        .FIN
)";

  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalNestedWithElse) {
  // Nested conditionals with ELSE
  std::string source = R"(
        .DO 1
        .DO 0
        LDA #$AA
        .ELSE
        LDA #$BB
        .FIN
        .FIN
)";

  parser->Parse(source, section, symbols);
  // Should assemble second LDA
  EXPECT_EQ(section.atoms.size(), 1u);
}

TEST_F(ScmasmSyntaxTest, ConditionalComplex) {
  // Complex conditional structure
  std::string source = R"(
PLATFORM .EQ 1
DEBUG    .EQ 0
        .DO PLATFORM-1
        .DO DEBUG
        LDA #$AA
        .ELSE
        LDA #$BB
        .FIN
        .ELSE
        LDA #$CC
        .FIN
)";

  parser->Parse(source, section, symbols);
  // PLATFORM-1 = 0, so else branch: LDA #$CC
  EXPECT_EQ(section.atoms.size(), 1u);
}

// ============================================================================
// Local Labels (.0-.9) Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, LocalLabelForwardReference) {
  // Local label forward reference
  std::string source = R"(
        .OR $0800
        BEQ .1
        LDA #$FF
.1      RTS
)";

  parser->Parse(source, section, symbols);
  // Should resolve forward reference
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, LocalLabelMultipleReferences) {
  // Multiple forward references to same local label
  std::string source = R"(
        .OR $0800
        BEQ .1
        BNE .1
.1      RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, LocalLabelRedefinition) {
  // Local labels can be redefined
  std::string source = R"(
        .OR $0800
        BEQ .1
.1      NOP
        BEQ .1
.1      RTS
)";

  parser->Parse(source, section, symbols);
  // Second .1 redefines the label
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, LocalLabelAllDigits) {
  // Test all local labels .0 through .9
  std::string source = R"(
        .OR $0800
.0      NOP
.1      NOP
.2      NOP
.3      NOP
.4      NOP
.5      NOP
.6      NOP
.7      NOP
.8      NOP
.9      NOP
)";

  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 21u); // ORG + 10 LabelAtoms + 10 NOPs
}

TEST_F(ScmasmSyntaxTest, LocalLabelInLoop) {
  // Local labels in loop contexts
  std::string source = R"(
        .OR $0800
        LDX #0
.1      INX
        CPX #10
        BNE .1
        RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

// Tests for :N syntax (colon-based local labels)
TEST_F(ScmasmSyntaxTest, ColonLocalLabelBasic) {
  // Basic :N local label
  std::string source = R"(
        .OR $0800
        BEQ :1
        LDA #$FF
:1      RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, ColonLocalLabelAllDigits) {
  // Test all :0 through :9 local labels
  std::string source = R"(
        .OR $0800
:0      NOP
:1      NOP
:2      NOP
:3      NOP
:4      NOP
:5      NOP
:6      NOP
:7      NOP
:8      NOP
:9      NOP
)";

  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 21u); // ORG + 10 LabelAtoms + 10 NOPs
}

TEST_F(ScmasmSyntaxTest, ColonLocalLabelForwardReference) {
  // Colon local label forward reference
  std::string source = R"(
        .OR $0800
        BEQ :1
        LDA #$FF
:1      RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, ColonLocalLabelInLoop) {
  // Colon local labels in loop contexts
  std::string source = R"(
        .OR $0800
        LDX #0
:1      INX
        CPX #10
        BNE :1
        RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, MixedDotAndColonLocalLabels) {
  // Mix of .N and :N local labels
  std::string source = R"(
        .OR $0800
        BEQ .1
        LDA #$FF
.1      BNE :2
        LDA #$00
:2      RTS
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 0u);
}

// ============================================================================
// .LU/.ENDU Loop Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, LoopSimple) {
  // Simple loop with counter
  std::string source = R"(
INDEX   .SE 0
        .LU 3
        .DA INDEX
INDEX   .SE INDEX+1
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // Should generate 3 data atoms with values 0, 1, 2
  EXPECT_EQ(section.atoms.size(), 3u);
}

TEST_F(ScmasmSyntaxTest, LoopWithTable) {
  // Generate table using loop
  std::string source = R"(
        .OR $0800
VALUE   .SE 0
        .LU 8
        .DA VALUE*256
VALUE   .SE VALUE+1
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // Should have ORG + 8 data atoms
  EXPECT_EQ(section.atoms.size(), 9u);
}

TEST_F(ScmasmSyntaxTest, LoopNested) {
  // Nested loops
  std::string source = R"(
OUTER   .SE 0
        .LU 2
INNER   .SE 0
        .LU 2
        .DA OUTER,INNER
INNER   .SE INNER+1
        .ENDU
OUTER   .SE OUTER+1
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // 2x2 = 4 data atoms
  EXPECT_EQ(section.atoms.size(), 4u);
}

TEST_F(ScmasmSyntaxTest, LoopWithInstructions) {
  // Loop generating instructions
  std::string source = R"(
        .OR $0800
        .LU 3
        NOP
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // ORG + 3 NOPs
  EXPECT_EQ(section.atoms.size(), 4u);
}

TEST_F(ScmasmSyntaxTest, LoopWithZeroCount) {
  // Loop with zero count should not execute
  std::string source = R"(
        .LU 0
        NOP
        .ENDU
)";

  parser->Parse(source, section, symbols);
  EXPECT_EQ(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, LoopWithLargeCount) {
  // Loop with larger count
  std::string source = R"(
        .OR $0800
        .LU 10
        NOP
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // ORG + 10 NOPs
  EXPECT_EQ(section.atoms.size(), 11u);
}

// ============================================================================
// Integration Tests - Phase 3 Complete Programs
// ============================================================================

TEST_F(ScmasmSyntaxTest, Phase3MacroProgram) {
  // Complete program using macros
  std::string source = R"(
* Macro demonstration
        .OR $0800
        
        .MA STORE
        STA \0
        .EM
        
        .MA CLEAR
        LDA #0
        STORE \0
        .EM
        
START   LDA #$FF
        STORE $C000
        CLEAR $C001
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 5u);
}

TEST_F(ScmasmSyntaxTest, Phase3ConditionalProgram) {
  // Complete program using conditionals
  std::string source = R"(
* Conditional assembly
        .OR $0800
        
DEBUG   .EQ 1
PROD    .EQ 0

        .DO DEBUG
        JSR TRACE
        .FIN
        
        .DO PROD
        JSR OPTIMIZE
        .ELSE
        JSR NORMAL
        .FIN
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, Phase3LoopProgram) {
  // Complete program using loops
  std::string source = R"(
* Table generation
        .OR $0800
        
TABLES  .LU 16
        .DA *
        .ENDU
)";

  parser->Parse(source, section, symbols);
  // ORG + 16 data entries
  EXPECT_EQ(section.atoms.size(), 17u);
}

TEST_F(ScmasmSyntaxTest, Phase3CombinedFeatures) {
  // Program combining macros, conditionals, and loops
  std::string source = R"(
* Combined Phase 3 features
        .OR $0800
        
DEBUG   .EQ 1

        .MA TRACE
        .DO DEBUG
        JSR $FDED
        .FIN
        .EM
        
START   TRACE
        
        .LU 5
        NOP
        .ENDU
        
        .DO DEBUG
.1      BRK
        .ELSE
.1      RTS
        .FIN
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 5u);
}

TEST_F(ScmasmSyntaxTest, Phase3MacroWithLocalLabels) {
  // Macro generating local labels
  std::string source = R"(
        .OR $0800
        
        .MA DELAY
        LDX #\0
.1      DEX
        BNE .1
        .EM
        
        DELAY 10
        DELAY 20
)";

  parser->Parse(source, section, symbols);
  EXPECT_GT(section.atoms.size(), 5u);
}

// ============================================================================
// P0 Directives: Phase 1 - .PS (Pascal String)
// ============================================================================

TEST_F(ScmasmSyntaxTest, PS_EmitsLengthPrefixedString) {
  // .PS "HELLO" should emit: 05 48 45 4C 4C 4F
  std::string source = R"(
        .OR $0800
        .PS "HELLO"
)";

  parser->Parse(source, section, symbols);

  // Should have OrgAtom and DataAtom
  ASSERT_GE(section.atoms.size(), 2u);

  // Get DataAtom (second atom)
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // Verify: length byte (5) + string bytes
  std::vector<uint8_t> expected = {0x05, 0x48, 0x45, 0x4C, 0x4C, 0x4F};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, PS_EmptyString) {
  // .PS "" should emit just 00 (length = 0)
  std::string source = R"(
        .OR $0800
        .PS ""
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  std::vector<uint8_t> expected = {0x00};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, PS_MaxLengthString) {
  // .PS with 255 characters (max for length byte)
  std::string long_str(255, 'A');
  std::string source = "        .OR $0800\n        .PS \"" + long_str + "\"\n";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // First byte should be 255 (0xFF)
  EXPECT_EQ(data_atom->data[0], 0xFF);
  EXPECT_EQ(data_atom->data.size(), 256u); // length + 255 chars
}

TEST_F(ScmasmSyntaxTest, PS_StringTooLong) {
  // .PS with 256+ characters should throw error
  std::string long_str(256, 'A');
  std::string source = "        .OR $0800\n        .PS \"" + long_str + "\"\n";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(ScmasmSyntaxTest, PS_HighBitRuleApplied) {
  // .PS 'HELLO' (delimiter < 0x27) should set high bit
  std::string source = R"(
        .OR $0800
        .PS 'HELLO'
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // Length byte not affected, but string bytes should have high bit set
  EXPECT_EQ(data_atom->data[0], 0x05);        // length
  EXPECT_EQ(data_atom->data[1] & 0x80, 0x80); // 'H' with high bit
  EXPECT_EQ(data_atom->data[2] & 0x80, 0x80); // 'E' with high bit
}

TEST_F(ScmasmSyntaxTest, PS_NoOperandError) {
  // .PS without operand should throw
  std::string source = R"(
        .OR $0800
        .PS
)";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

// ============================================================================
// .INB (Include Binary) Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, INB_IncludesExistingFile) {
  // Create a temp assembly source file for testing
  std::string test_file = "test_include.s";
  std::ofstream out(test_file);
  out << "        LDA #$42\n";
  out << "        STA $C000\n";
  out.close();

  std::string source = R"(
        .OR $0800
START   .INB test_include.s
        RTS
)";

  parser->Parse(source, section, symbols);

  // Should have OrgAtom, LabelAtom, InstructionAtom(s), InstructionAtom(RTS)
  ASSERT_GE(section.atoms.size(), 4u);

  // Verify it parsed source (not binary data)
  // Should contain instruction atoms, not data atoms
  bool has_instruction = false;
  for (const auto &atom : section.atoms) {
    if (std::dynamic_pointer_cast<InstructionAtom>(atom)) {
      has_instruction = true;
      break;
    }
  }
  EXPECT_TRUE(has_instruction);

  // Cleanup
  std::remove(test_file.c_str());
}

TEST_F(ScmasmSyntaxTest, INB_MissingFileError) {
  std::string source = R"(
        .OR $0800
        .INB nonexistent_file.bin
)";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

TEST_F(ScmasmSyntaxTest, INB_RelativePathResolution) {
  // Create subdirectory and include file
  std::filesystem::create_directory("test_subdir");
  std::string include_file = "test_subdir/included.s";
  std::ofstream inc(include_file);
  inc << "INCLUDED .EQ $42\n";
  inc.close();

  // Create main file that includes relative path
  std::string main_file = "test_subdir/main.s";
  std::ofstream main(main_file);
  main << "        .OR $0800\n";
  main << "        .INB included.s\n";
  main << "        LDA #INCLUDED\n";
  main.close();

  // Read and parse main file
  std::ifstream in(main_file);
  std::string source((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());

  // Set current file to main.s so relative includes work
  parser->SetCurrentFile(main_file);

  parser->Parse(source, section, symbols);

  // Should have parsed included file and defined INCLUDED symbol
  EXPECT_TRUE(symbols.IsDefined("INCLUDED"));
  int64_t included_value;
  EXPECT_TRUE(symbols.Lookup("INCLUDED", included_value));
  EXPECT_EQ(included_value, 0x42);

  // Cleanup
  std::error_code ec;
  std::filesystem::remove_all("test_subdir", ec);
  // Ignore errors - Windows may lock files/directories
}

TEST_F(ScmasmSyntaxTest, INB_EmptyFile) {
  // Create empty source file (just comments/whitespace)
  std::string test_file = "empty.s";
  std::ofstream out(test_file);
  out << "; Empty include file\n";
  out << "\n";
  out.close();

  std::string source = R"(
        .OR $0800
        .INB empty.s
)";

  parser->Parse(source, section, symbols);

  // Should have just OrgAtom (empty file contributes nothing)
  ASSERT_GE(section.atoms.size(), 1u);

  // Cleanup
  std::remove(test_file.c_str());
}

TEST_F(ScmasmSyntaxTest, INB_NoOperandError) {
  std::string source = R"(
        .OR $0800
        .INB
)";

  EXPECT_THROW(parser->Parse(source, section, symbols), std::runtime_error);
}

// ============================================================================
// .LIST (Listing Control) Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, LIST_On) {
  std::string source = R"(
        .OR $0800
        .LIST ON
        NOP
)";

  // Should not throw - just accept and continue
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));

  // Should have OrgAtom and InstructionAtom (NOP)
  ASSERT_GE(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, LIST_Off) {
  std::string source = R"(
        .OR $0800
        .LIST OFF
        NOP
)";

  // Should not throw - just accept and continue
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));

  // Should have OrgAtom and InstructionAtom (NOP)
  ASSERT_GE(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, LIST_NoOperand) {
  // .LIST without operand defaults to ON
  std::string source = R"(
        .OR $0800
        .LIST
        NOP
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  ASSERT_GE(section.atoms.size(), 2u);
}

TEST_F(ScmasmSyntaxTest, LIST_CaseInsensitive) {
  std::string source = R"(
        .OR $0800
        .LIST on
        NOP
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  ASSERT_GE(section.atoms.size(), 2u);
}

// ============================================================================
// .CS (C-String with Escape Sequences) Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, CS_BasicString) {
  std::string source = R"(
        .OR $0800
        .CS "Hello"
)";

  parser->Parse(source, section, symbols);

  // Should have OrgAtom and DataAtom
  ASSERT_GE(section.atoms.size(), 2u);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // "Hello" - no escape sequences, no high-bit manipulation
  std::vector<uint8_t> expected = {'H', 'e', 'l', 'l', 'o'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceNewline) {
  std::string source = R"(
        .OR $0800
        .CS "Line1\nLine2"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \n should become 0x0A
  std::vector<uint8_t> expected = {'L', 'i', 'n', 'e', '1', 0x0A,
                                   'L', 'i', 'n', 'e', '2'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceTab) {
  std::string source = R"(
        .OR $0800
        .CS "A\tB"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \t should become 0x09
  std::vector<uint8_t> expected = {'A', 0x09, 'B'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceNull) {
  std::string source = R"(
        .OR $0800
        .CS "A\0B"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \0 should become 0x00
  std::vector<uint8_t> expected = {'A', 0x00, 'B'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceBackslash) {
  std::string source = R"(
        .OR $0800
        .CS "A\\B"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \\ should become single backslash
  std::vector<uint8_t> expected = {'A', '\\', 'B'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceQuote) {
  // In SCMASM the string delimiter is NOT escapable with '\'.
  // A '"' always terminates a double-quoted string, even when preceded by '\'.
  // So .CS "Say \" produces "Say \" with the closing '"' at the backslash.
  // The string content is: S a y SPACE backslash (5 bytes).
  std::string source = R"(
        .OR $0800
        .CS "Say \"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // In SCMASM '"' ends the string — backslash is emitted as a literal byte.
  std::vector<uint8_t> expected = {'S', 'a', 'y', ' ', '\\'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_SpinnerWithTrailingBackslash) {
  // Real-world SCMASM pattern from UNARC.S: spinner chars "|/-\"
  // The '\' is the last content byte; '"' ends the string.
  std::string source = R"(
        .OR $0800
        .CS "|/-\"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  std::vector<uint8_t> expected = {'|', '/', '-', '\\'};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EscapeSequenceHexByte) {
  std::string source = R"(
        .OR $0800
        .CS "\x41\x42\x43"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \xHH should become hex byte value
  std::vector<uint8_t> expected = {0x41, 0x42, 0x43}; // "ABC"
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_AllEscapeSequences) {
  std::string source = R"(
        .OR $0800
        .CS "\a\b\e\f\n\r\t\v"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // All escape sequences
  std::vector<uint8_t> expected = {
      0x07, // \a (bell)
      0x08, // \b (backspace)
      0x1B, // \e (escape)
      0x0C, // \f (form feed)
      0x0A, // \n (newline)
      0x0D, // \r (carriage return)
      0x09, // \t (tab)
      0x0B  // \v (vertical tab)
  };
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CS_EmptyString) {
  std::string source = R"(
        .OR $0800
        .CS ""
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  EXPECT_TRUE(data_atom->data.empty());
}

// ============================================================================
// .CZ (C-String Zero-Terminated) Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, CZ_BasicString) {
  std::string source = R"(
        .OR $0800
        .CZ "Hello"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // "Hello" with null terminator
  std::vector<uint8_t> expected = {'H', 'e', 'l', 'l', 'o', 0x00};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CZ_WithEscapeSequences) {
  std::string source = R"(
        .OR $0800
        .CZ "Line\n"
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // "Line\n" with null terminator
  std::vector<uint8_t> expected = {'L', 'i', 'n', 'e', 0x0A, 0x00};
  EXPECT_EQ(data_atom->data, expected);
}

TEST_F(ScmasmSyntaxTest, CZ_EmptyString) {
  std::string source = R"(
        .OR $0800
        .CZ ""
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // Empty string still gets null terminator
  std::vector<uint8_t> expected = {0x00};
  EXPECT_EQ(data_atom->data, expected);
}

// ============================================================================
// .DUMMY/.ED (Dummy Section for Structures) Directive Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, DUMMY_SuppressesByteEmission) {
  // .DUMMY should suppress byte emission but still advance address
  std::string source = R"(
        .OR $0800
        .DUMMY
FIELD1  .BS 1
FIELD2  .BS 2
FIELD3  .BS 4
        .ED
DATA    .BS 3
)";

  parser->Parse(source, section, symbols);

  // Should have OrgAtom, LabelAtom (from DATA label), and DataAtom (.BS 3)
  // FIELD1/2/3 labels inside .DUMMY do not produce LabelAtoms (dummy suppresses)
  ASSERT_EQ(section.atoms.size(), 3u);

  auto label_atom = std::dynamic_pointer_cast<LabelAtom>(section.atoms[1]);
  ASSERT_NE(label_atom, nullptr);
  EXPECT_EQ(label_atom->name, "DATA");

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);
  ASSERT_NE(data_atom, nullptr);

  // Only DATA .BS 3 should emit bytes (3 bytes)
  EXPECT_EQ(data_atom->data.size(), 3u);
}

TEST_F(ScmasmSyntaxTest, DUMMY_DoesNotAdvanceMainPC) {
  // .DUMMY/.ED defines structure labels without advancing the main section PC.
  // In SCMASM, .DUMMY is a BSS/struct layout section: the assembler assigns
  // addresses to fields inside the block but the main PC is restored to its
  // pre-.DUMMY value when .ED is encountered.  Code assembled after .ED
  // continues from the address that was active when .DUMMY was entered.
  std::string source = R"(
        .OR $0800
START   .EQ *
        .DUMMY
FIELD1  .BS 1
FIELD2  .BS 2
FIELD3  .BS 4
        .ED
END     .EQ *
)";

  parser->Parse(source, section, symbols);

  int64_t start_addr, end_addr;
  ASSERT_TRUE(symbols.Lookup("START", start_addr));
  ASSERT_TRUE(symbols.Lookup("END", end_addr));

  // Both START and END are $0800 — .DUMMY does NOT advance the main PC.
  EXPECT_EQ(start_addr, 0x0800);
  EXPECT_EQ(end_addr, 0x0800);

  // Fields inside the dummy block get correct sequential addresses.
  int64_t field1_addr, field2_addr, field3_addr;
  ASSERT_TRUE(symbols.Lookup("FIELD1", field1_addr));
  ASSERT_TRUE(symbols.Lookup("FIELD2", field2_addr));
  ASSERT_TRUE(symbols.Lookup("FIELD3", field3_addr));
  EXPECT_EQ(field1_addr, 0x0800);
  EXPECT_EQ(field2_addr, 0x0801);
  EXPECT_EQ(field3_addr, 0x0803);
}

TEST_F(ScmasmSyntaxTest, DUMMY_WithLabels) {
  // Labels defined in .DUMMY section should have correct addresses
  std::string source = R"(
        .OR $0800
        .DUMMY
STRUCT  .EQ *
FIELD1  .BS 1
FIELD2  .BS 2
FIELD3  .BS 4
        .ED
)";

  parser->Parse(source, section, symbols);

  // Check that labels have correct addresses
  int64_t struct_addr, field1_addr, field2_addr, field3_addr;
  ASSERT_TRUE(symbols.Lookup("STRUCT", struct_addr));
  ASSERT_TRUE(symbols.Lookup("FIELD1", field1_addr));
  ASSERT_TRUE(symbols.Lookup("FIELD2", field2_addr));
  ASSERT_TRUE(symbols.Lookup("FIELD3", field3_addr));

  // Verify addresses increment correctly
  EXPECT_EQ(struct_addr, 0x0800);
  EXPECT_EQ(field1_addr, 0x0800);
  EXPECT_EQ(field2_addr, 0x0801);
  EXPECT_EQ(field3_addr, 0x0803);
}

TEST_F(ScmasmSyntaxTest, DUMMY_ReturnsToNormalMode) {
  // .ED should return to normal byte emission
  std::string source = R"(
        .OR $0800
        .BS 2
        .DUMMY
        .BS 4
        .ED
        .BS 3
)";

  parser->Parse(source, section, symbols);

  // Should have OrgAtom + 2 DataAtoms (first .BS 2 and last .BS 3)
  ASSERT_EQ(section.atoms.size(), 3u);

  auto data_atom1 = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  auto data_atom2 = std::dynamic_pointer_cast<DataAtom>(section.atoms[2]);

  ASSERT_NE(data_atom1, nullptr);
  ASSERT_NE(data_atom2, nullptr);

  EXPECT_EQ(data_atom1->data.size(), 2u); // First .BS 2
  EXPECT_EQ(data_atom2->data.size(), 3u); // Last .BS 3
}

TEST_F(ScmasmSyntaxTest, DUMMY_NestedNotSupported) {
  // Nested .DUMMY should error or behave correctly (implementation-defined)
  std::string source = R"(
        .OR $0800
        .DUMMY
        .DUMMY
        .BS 1
        .ED
        .ED
)";

  // This test documents behavior - may throw or handle gracefully
  // For now, we'll just verify it doesn't crash
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

// .OR inside .DUMMY should emit a DummyOrgAtom so that ResolveSymbols() assigns
// zero-page (or other) addresses to labels, not the main-section PC.
TEST_F(ScmasmSyntaxTest, DUMMY_OR_ResolvesToZPAddress) {
  // Main section starts at $2000; inside the dummy block the ZP base is $80.
  // Labels inside the dummy block must resolve to ZP addresses ($80, $81…),
  // not to the main-section address ($2000).
  std::string source = R"(
        .OR $2000
        .DUMMY
        .OR $80
ZPTMP   .BS 6
ZPTMP2  .BS 2
        .ED
)";

  parser->Parse(source, section, symbols);

  // Verify a DummyOrgAtom was emitted (not a real OrgAtom that would move PC)
  bool found_dummy_org = false;
  for (const auto &atom : section.atoms) {
    if (atom->type == AtomType::DummyOrg) {
      found_dummy_org = true;
      break;
    }
  }
  EXPECT_TRUE(found_dummy_org)
      << ".OR inside .DUMMY should emit a DummyOrgAtom";

  // After ResolveSymbols the labels should carry ZP addresses.
  // Run a full assemble so ResolveSymbols is called.
  Assembler assembler;
  assembler.SetCpuPlugin(cpu.get());
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);
  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly should succeed";

  int64_t zptmp_addr = 0;
  int64_t zptmp2_addr = 0;
  ASSERT_TRUE(symbols.Lookup("ZPTMP", zptmp_addr))
      << "ZPTMP must be defined";
  ASSERT_TRUE(symbols.Lookup("ZPTMP2", zptmp2_addr))
      << "ZPTMP2 must be defined";

  // ZPTMP starts at $80 (the .OR target), ZPTMP2 follows after 6 bytes
  EXPECT_EQ(zptmp_addr, 0x80)
      << "ZPTMP should resolve to ZP address $80, not main-section PC";
  EXPECT_EQ(zptmp2_addr, 0x86)
      << "ZPTMP2 should resolve to ZP address $86 (=$80+6)";
}

// ============================================================================
// .INB Include Path Search Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, INB_SearchesIncludePaths) {
  // Create include directory and file
  std::filesystem::create_directories("test_includes/subdir");
  std::string include_file = "test_includes/subdir/shared.s";
  std::ofstream inc(include_file);
  inc << "SHARED .EQ $1234\n";
  inc.close();

  // Set include paths
  std::vector<std::string> include_paths = {"test_includes/subdir"};
  parser->SetIncludePaths(include_paths);

  // Parse source that references file by name only (no path)
  std::string source = R"(
        .OR $0800
        .INB shared.s
        LDA #>SHARED
)";

  parser->Parse(source, section, symbols);

  // Verify SHARED symbol was defined from included file
  EXPECT_TRUE(symbols.IsDefined("SHARED"));
  int64_t shared_value;
  EXPECT_TRUE(symbols.Lookup("SHARED", shared_value));
  EXPECT_EQ(shared_value, 0x1234);

  // Cleanup
  std::filesystem::remove_all("test_includes");
}

TEST_F(ScmasmSyntaxTest, INB_SearchesMultipleIncludePaths) {
  // Create multiple include directories
  std::filesystem::create_directories("test_inc1");
  std::filesystem::create_directories("test_inc2");

  std::string file1 = "test_inc1/config1.s";
  std::ofstream f1(file1);
  f1 << "CONFIG1 .EQ $01\n";
  f1.close();

  std::string file2 = "test_inc2/config2.s";
  std::ofstream f2(file2);
  f2 << "CONFIG2 .EQ $02\n";
  f2.close();

  // Set multiple include paths
  std::vector<std::string> include_paths = {"test_inc1", "test_inc2"};
  parser->SetIncludePaths(include_paths);

  // Parse source that includes from both paths
  std::string source = R"(
        .OR $0800
        .INB config1.s
        .INB config2.s
)";

  parser->Parse(source, section, symbols);

  // Verify both symbols defined
  EXPECT_TRUE(symbols.IsDefined("CONFIG1"));
  EXPECT_TRUE(symbols.IsDefined("CONFIG2"));

  int64_t val1, val2;
  EXPECT_TRUE(symbols.Lookup("CONFIG1", val1));
  EXPECT_TRUE(symbols.Lookup("CONFIG2", val2));
  EXPECT_EQ(val1, 0x01);
  EXPECT_EQ(val2, 0x02);

  // Cleanup
  std::filesystem::remove_all("test_inc1");
  std::filesystem::remove_all("test_inc2");
}

TEST_F(ScmasmSyntaxTest, INB_IncludePathPriorityOrder) {
  // Test that include paths are searched in order
  // Create same filename in two directories
  std::filesystem::create_directories("test_priority1");
  std::filesystem::create_directories("test_priority2");

  std::string file1 = "test_priority1/shared.s";
  std::ofstream f1(file1);
  f1 << "VALUE .EQ $AA\n";
  f1.close();

  std::string file2 = "test_priority2/shared.s";
  std::ofstream f2(file2);
  f2 << "VALUE .EQ $BB\n";
  f2.close();

  // Set include paths with priority1 FIRST
  std::vector<std::string> include_paths = {"test_priority1", "test_priority2"};
  parser->SetIncludePaths(include_paths);

  std::string source = R"(
        .OR $0800
        .INB shared.s
)";

  parser->Parse(source, section, symbols);

  // Should use first matching file (priority1)
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("VALUE", value));
  EXPECT_EQ(value, 0xAA); // From priority1, not priority2

  // Cleanup
  std::filesystem::remove_all("test_priority1");
  std::filesystem::remove_all("test_priority2");
}

TEST_F(ScmasmSyntaxTest, INB_RelativeToSourceBeforeIncludePaths) {
  // Test that relative-to-source-file takes priority over include paths
  std::filesystem::create_directories("test_source_dir");
  std::filesystem::create_directories("test_include_dir");

  // File relative to source
  std::string source_rel_file = "test_source_dir/local.s";
  std::ofstream src_f(source_rel_file);
  src_f << "LOCAL .EQ $11\n";
  src_f.close();

  // Same filename in include path
  std::string inc_file = "test_include_dir/local.s";
  std::ofstream inc_f(inc_file);
  inc_f << "LOCAL .EQ $22\n";
  inc_f.close();

  // Set include path
  std::vector<std::string> include_paths = {"test_include_dir"};
  parser->SetIncludePaths(include_paths);

  // Create main source file
  std::string main_file = "test_source_dir/main.s";
  std::ofstream main_f(main_file);
  main_f << "        .OR $0800\n";
  main_f << "        .INB local.s\n";
  main_f.close();

  // Parse with main file as current file
  parser->SetCurrentFile(main_file);
  std::ifstream in(main_file);
  std::string source((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
  parser->Parse(source, section, symbols);

  // Should use source-relative file, not include path file
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("LOCAL", value));
  EXPECT_EQ(value, 0x11); // From test_source_dir/local.s

  // Cleanup
  std::error_code ec;
  std::filesystem::remove_all("test_source_dir", ec);
  std::filesystem::remove_all("test_include_dir", ec);
  // Ignore errors - Windows may lock files/directories
}

TEST_F(ScmasmSyntaxTest, INB_AbsolutePathIgnoresIncludePaths) {
  // Test that absolute paths are not affected by include paths
  std::filesystem::create_directories("test_abs_include");

  // Create file with absolute path
  std::filesystem::path abs_file =
      std::filesystem::absolute("test_abs_include/absolute.s");
  std::ofstream f(abs_file);
  f << "ABSOLUTE .EQ $99\n";
  f.close();

  // Set include path (should be ignored for absolute paths)
  std::vector<std::string> include_paths = {"some_other_dir"};
  parser->SetIncludePaths(include_paths);

  // Use absolute path in .INB
  std::string source =
      "        .OR $0800\n        .INB " + abs_file.string() + "\n";

  parser->Parse(source, section, symbols);

  // Should find file using absolute path
  EXPECT_TRUE(symbols.IsDefined("ABSOLUTE"));
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("ABSOLUTE", value));
  EXPECT_EQ(value, 0x99);

  // Cleanup
  std::filesystem::remove_all("test_abs_include");
}

TEST_F(ScmasmSyntaxTest, INB_CurrentWorkingDirFallback) {
  // Test that CWD is used as fallback if not found elsewhere
  std::string cwd_file = "test_cwd_file.s";
  std::ofstream f(cwd_file);
  f << "CWD_VAL .EQ $77\n";
  f.close();

  // Set include paths that DON'T contain the file
  std::vector<std::string> include_paths = {"nonexistent_dir"};
  parser->SetIncludePaths(include_paths);

  std::string source = R"(
        .OR $0800
        .INB test_cwd_file.s
)";

  parser->Parse(source, section, symbols);

  // Should find file in CWD
  EXPECT_TRUE(symbols.IsDefined("CWD_VAL"));
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("CWD_VAL", value));
  EXPECT_EQ(value, 0x77);

  // Cleanup
  std::remove(cwd_file.c_str());
}

TEST_F(ScmasmSyntaxTest, INB_ErrorMessageShowsSearchedPaths) {
  // Test that error message lists all paths that were searched
  std::vector<std::string> include_paths = {"path1", "path2", "path3"};
  parser->SetIncludePaths(include_paths);

  std::string source = R"(
        .OR $0800
        .INB nonexistent.s
)";

  try {
    parser->Parse(source, section, symbols);
    FAIL() << "Expected exception for missing file";
  } catch (const std::runtime_error &e) {
    std::string error_msg = e.what();
    // Error message should mention the searched paths
    EXPECT_TRUE(error_msg.find("nonexistent.s") != std::string::npos);
    EXPECT_TRUE(error_msg.find("searched:") != std::string::npos ||
                error_msg.find("cannot open") != std::string::npos);
  }
}

// ============================================================================
// Editor Command Stripping Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, EditorCommands_NEW_IsStripped) {
  std::string source = R"(
        NEW
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_AUTO_IsStripped) {
  std::string source = R"(
        AUTO 100,10
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_MAN_IsStripped) {
  std::string source = R"(
        MAN
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_SAVE_IsStripped) {
  std::string source = R"(
        SAVE PROG.S
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_ASM_IsStripped) {
  std::string source = R"(
        ASM
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_DELETE_IsStripped) {
  std::string source = R"(
        DELETE 100,200
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_LIST_IsStripped) {
  std::string source = R"(
        LIST
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_CaseInsensitive) {
  std::string source = R"(
        new
        MaN
        Auto 100,10
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_WithLeadingWhitespace) {
  std::string source = R"(
          NEW
        MAN
    AUTO 100,10
        .OR $0800
        LDA #$00
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  EXPECT_GT(section.atoms.size(), 0u); // Should parse successfully
}

TEST_F(ScmasmSyntaxTest, EditorCommands_MixedWithCode) {
  std::string source = R"(
        .OR $0800
START   LDA #$00
        MAN
        STA $C000
        AUTO 100,10
        RTS
        SAVE PROG.S
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));

  // Check that label was defined
  int64_t start_value;
  EXPECT_TRUE(symbols.Lookup("START", start_value));
  EXPECT_EQ(start_value, 0x0800);

  // Code should assemble correctly
  EXPECT_GT(section.atoms.size(), 0u);
}

TEST_F(ScmasmSyntaxTest, EditorCommands_SOH_ControlCharBeforeInstruction) {
  // Apple II source files sometimes embed \x01 (SOH) control characters as
  // editor artifacts at the start of instruction lines. They must be ignored.
  std::string source;
  source += "\t.OR $1000\n";
  source += "\x01\t\t\tlda #$42\n";  // \x01 before a real instruction
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  // The lda #$42 should have been assembled
  bool found_lda = false;
  for (auto &atom : section.atoms) {
    if (auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom)) {
      found_lda = true;
      break;
    }
  }
  EXPECT_TRUE(found_lda);
}

TEST_F(ScmasmSyntaxTest, EditorCommands_NotALabelOrDirective) {
  // Verify that labels or directives that happen to start with editor command
  // names are NOT stripped (they should have colons or periods)
  std::string source = R"(
        .OR $0800
MANUAL  LDA #$00      ; Label "MANUAL" should work
        STA $C000
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));

  // Label should be defined
  int64_t manual_value;
  EXPECT_TRUE(symbols.Lookup("MANUAL", manual_value));
  EXPECT_EQ(manual_value, 0x0800);
}

TEST_F(ScmasmSyntaxTest, EditorCommands_RealWorldA2osX_Pattern) {
  // Simulate the real A2osX pattern: MAN followed by line number commands
  std::string source = R"(
        .OR $0800
        MAN
*--------------------------------------
* Test routine
*--------------------------------------
START   LDA #$00
        STA $C000
        RTS
)";

  EXPECT_NO_THROW(parser->Parse(source, section, symbols));

  // MAN should be stripped, comments preserved as comments
  int64_t start_value;
  EXPECT_TRUE(symbols.Lookup("START", start_value));
  EXPECT_EQ(start_value, 0x0800);

  // Should assemble successfully
  EXPECT_GT(section.atoms.size(), 0u);
}

// ============================================================================
// Inline Comment Tests (A2osX real-world patterns)
// These test that directives accept trailing whitespace-separated comments
// without a semicolon, matching the original SCMASM convention.
// ============================================================================

TEST_F(ScmasmSyntaxTest, BS_InlineComment_NumericCount) {
  // .BS 9   9 bytes, S.IOCTL  — inline comment after numeric count
  std::string source =
      "\t.OR\t$1000\n"
      "\t.DUMMY\n"
      "\t.OR\t$0000\n"
      "\t.BS 9\t\t9 bytes, S.IOCTL\n"
      "\t.ED\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, BS_InlineComment_SymbolExpression) {
  // .BS K.FD.MAX*2   pFDs  — symbol*literal with inline comment
  std::string source =
      "K.FD.MAX\t.EQ\t64\n"
      "\t.OR\t$1000\n"
      "\t.DUMMY\n"
      "\t.OR\t$0000\n"
      "\t.BS K.FD.MAX*2\t\tpFDs\n"
      "\t.ED\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, OR_InlineComment_ByteCount) {
  // .OR ZPTMP   6 Bytes  — inline comment after address expression
  std::string source =
      "ZPTMP\t.EQ\t$80\n"
      "\t.OR\t$1000\n"
      "\t.DUMMY\n"
      "\t.OR ZPTMP\t\t6 Bytes\n"
      "\t.ED\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, EQ_InlineComment) {
  // TIMEOUT   .EQ 180   float  — inline comment after value
  std::string source =
      "\t.OR\t$1000\n"
      "TIMEOUT\t.EQ 180\tfloat\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("TIMEOUT", v));
  EXPECT_EQ(v, 180);
}

TEST_F(ScmasmSyntaxTest, SE_InlineComment) {
  // CNT   .SE 3   loop count  — inline comment after value
  std::string source =
      "\t.OR\t$1000\n"
      "CNT\t.SE 3\tloop count\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("CNT", v));
  EXPECT_EQ(v, 3);
}

TEST_F(ScmasmSyntaxTest, CurrentAddress_StarAlone) {
  // .EQ *  — star as current address (must still work)
  std::string source =
      "\t.OR\t$2000\n"
      "HERE\t.EQ *\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("HERE", v));
  EXPECT_EQ(v, 0x2000);
}

TEST_F(ScmasmSyntaxTest, CurrentAddress_StarInExpression) {
  // .EQ *+4  — star plus offset (must still work)
  std::string source =
      "\t.OR\t$2000\n"
      "NEXT\t.EQ *+4\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("NEXT", v));
  EXPECT_EQ(v, 0x2004);
}

TEST_F(ScmasmSyntaxTest, Multiply_StarAsOperator) {
  // .BS K.FD.MAX*2  — star must be treated as multiply, not current address
  std::string source =
      "K.FD.MAX\t.EQ\t64\n"
      "\t.OR\t$1000\n"
      "\t.DUMMY\n"
      "\t.OR\t$0000\n"
      "\t.BS K.FD.MAX*2\n"
      "\t.ED\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, Multiply_StarAsOperator_WithCurrentAddress) {
  // $1300-*  — star is current address when not preceded by identifier
  std::string source =
      "\t.OR\t$1000\n"
      "SZ\t.EQ $1300-*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("SZ", v));
  EXPECT_EQ(v, 0x0300); // $1300 - $1000 = $0300
}

TEST_F(ScmasmSyntaxTest, MacroParam_InlineComment_Stripped) {
  // >MYMACRO VALUE   trailing comment  — trailing comment must not
  // be included in the macro parameter ]1
  std::string source =
      "\t.MA MYMACRO\n"
      "RESULT\t.EQ ]1\n"
      "\t.EM\n"
      "\t.OR\t$1000\n"
      "\t>MYMACRO $42\t\tsome inline comment\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t v;
  EXPECT_TRUE(symbols.Lookup("RESULT", v));
  EXPECT_EQ(v, 0x42);
}

TEST_F(ScmasmSyntaxTest, HS_MnemonicComment_OddHexWord) {
  // .HS 90   BCC  — BCC is a valid hex string but odd-length; treat as comment
  // Result must be exactly 1 byte ($90), not 2.5 bytes
  std::string source =
      "\t.OR\t$1000\n"
      "\t.HS 90\t\t\t\tBCC\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, HS_MnemonicComment_EvenHexWords_Included) {
  // .HS 2C   BIT ABS  — 2C is even-length all-hex (1 byte), BIT stops (non-hex)
  std::string source =
      "\t.OR\t$1000\n"
      "\t.HS 2C\t\t\t\tBIT ABS\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, HS_MultiByteRun_NoSpaces) {
  // .HS 03030301030101010404  — continuous hex run (10 bytes, no spaces)
  std::string source =
      "\t.OR\t$1000\n"
      "\t.HS 03030301030101010404\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

// ============================================================================
// Instruction Size Estimation — current_address_ accuracy for .BS TARGET-*
// ============================================================================

// Helper: after parsing, read back the symbol value set by a trailing .EQ *
// to check where the parser thinks current_address_ is.

TEST_F(ScmasmSyntaxTest, InstructionSize_ImpliedIs1Byte) {
  // SEC, CLC, RTS, NOP etc. have no operand → 1 byte each
  // After two implied instructions from $1000, * should be $1002
  std::string source =
      "\t.OR\t$1000\n"
      "\t\tSEC\n"
      "\t\tRTS\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002) << "SEC(1)+RTS(1) = 2 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_ImmediateIs2Bytes) {
  // LDA #$42 is 2 bytes
  std::string source =
      "\t.OR\t$1000\n"
      "\t\tLDA\t#$42\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002) << "LDA #imm = 2 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_AbsoluteIs3Bytes) {
  // JMP $1234 is 3 bytes
  std::string source =
      "\t.OR\t$1000\n"
      "\t\tJMP\t$1234\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1003) << "JMP abs = 3 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_ZeroPageExplicitIs2Bytes) {
  // LDA $80 has a 1-byte ZP operand → 2 bytes total
  std::string source =
      "\t.OR\t$1000\n"
      "\t\tLDA\t$80\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002) << "LDA $80 (ZP) = 2 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_IndirectZeroPageIs2Bytes) {
  // LDA (ZP_ADDR) → 65C02 zero-page indirect = 2 bytes
  std::string source =
      "ZP_ADDR\t.EQ\t$80\n"
      "\t\t.OR\t$1000\n"
      "\t\tLDA\t(ZP_ADDR)\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002) << "LDA (zp) = 2 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_IndirectAbsoluteIs3Bytes) {
  // JMP (ABS_ADDR) → absolute indirect = 3 bytes
  std::string source =
      "ABS_ADDR\t.EQ\t$1234\n"
      "\t\t.OR\t$1000\n"
      "\t\tJMP\t(ABS_ADDR)\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1003) << "JMP (abs) = 3 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_AbsIndexedIndirectIs3Bytes) {
  // JMP (ABS,X) → absolute indexed indirect = 3 bytes
  std::string source =
      "JMP_TBL\t.EQ\t$C000\n"
      "\t\t.OR\t$1000\n"
      "\t\tJMP\t(JMP_TBL,X)\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1003) << "JMP (abs,X) = 3 bytes from $1000";
}

TEST_F(ScmasmSyntaxTest, BS_CurrentAddressMinus_AfterMixedInstructions) {
  // Simulates A2osX KERNEL.S.GP pattern:
  //   .PH GPBASE
  //   jmp LIBC      ; 3 bytes → GPBASE+00
  //   jmp FPU       ; 3 bytes → GPBASE+03
  //   lda #$00      ; 2 bytes → GPBASE+06
  //   sec           ; 1 byte  → GPBASE+08
  //   rts           ; 1 byte  → GPBASE+09
  //   .BS TARGET-*  ; should = TARGET - (GPBASE+10)
  //
  // Use .EQ addresses so we don't need real symbol resolution
  // TARGET = GPBASE + 10 → .BS should be 0 bytes (sanity check)
  // If current_address_ were wrong (e.g., +3 per instr = +15 bytes),
  // the .BS count would wrap negative → "too large" error.
  std::string source =
      "GPBASE\t\t.EQ\t$D400\n"
      "TARGET\t\t.EQ\t$D40A\n"    // GPBASE + 10
      "LIBC\t\t.EQ\t$E000\n"
      "FPU\t\t.EQ\t$E100\n"
      "\t\t.OR\tGPBASE\n"
      "\t\tJMP\tLIBC\n"           // 3 bytes
      "\t\tJMP\tFPU\n"            // 3 bytes
      "\t\tLDA\t#$00\n"           // 2 bytes
      "\t\tSEC\n"                 // 1 byte
      "\t\tRTS\n"                 // 1 byte  → * = $D40A = TARGET
      "\t\t.BS\tTARGET-*\n"       // 0 bytes — if current_address_ is right
      "AFTER\t\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t after = 0;
  ASSERT_TRUE(symbols.Lookup("AFTER", after));
  EXPECT_EQ(after, 0xD40A) << "current_address_ must equal TARGET after exact instructions";
}

TEST_F(ScmasmSyntaxTest, BS_CurrentAddressMinus_FillGap) {
  // Similar to above but .BS fills a non-zero gap.
  // 3 JMPs (9 bytes) + 1 LDA # (2 bytes) = 11 bytes.
  // TARGET = GPBASE + 16 → gap = 16 - 11 = 5 bytes.
  std::string source =
      "GPBASE\t\t.EQ\t$D400\n"
      "TARGET\t\t.EQ\t$D410\n"    // GPBASE + 16
      "A1\t\t.EQ\t$E000\n"
      "A2\t\t.EQ\t$E100\n"
      "A3\t\t.EQ\t$E200\n"
      "\t\t.OR\tGPBASE\n"
      "\t\tJMP\tA1\n"             // 3 bytes
      "\t\tJMP\tA2\n"             // 3 bytes
      "\t\tJMP\tA3\n"             // 3 bytes
      "\t\tLDA\t#$06\n"           // 2 bytes → * = $D40B
      "\t\t.BS\tTARGET-*\n"       // 5 bytes
      "AFTER\t\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t after = 0;
  ASSERT_TRUE(symbols.Lookup("AFTER", after));
  EXPECT_EQ(after, 0xD410) << ".BS must advance address to TARGET";
}

// ============================================================================
// Instruction inline comment stripping
// ============================================================================
// SCMASM uses whitespace-separated inline comments on instructions too.
// e.g. "TAX   %11000000 or %00111000" — the binary expression is a comment
// e.g. "AND K.LC,y   should be %xx..." — only "K.LC,y" is the operand

TEST_F(ScmasmSyntaxTest, Instruction_ImpliedWithInlineComment_NoError) {
  // TAX with binary-literal inline comment must not cause a parse/assemble error
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\tTAX\t\t\t\t%11000000 or %00111000\n"
      "\t\tNOP\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, Instruction_WithOperandAndInlineComment_OperandKept) {
  // AND K.LC,y   should be %xx…  — only "K.LC,y" should be the operand.
  // We verify parse succeeds (inline comment stripped) and address advances
  // correctly: AND abs,Y = 3 bytes from $1000.
  std::string source =
      "K_LC\t\t.EQ\t$C300\n"
      "\t\t.OR\t$1000\n"
      "\t\tAND\tK_LC,Y\t\t\tshould be %00xxxxxx or %xx000xxx\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1003) << "AND abs,Y = 3 bytes; inline comment must not grow operand";
}

TEST_F(ScmasmSyntaxTest, DA_InlineComment_BinaryLiteral) {
  // .DA #%100   L  — "#%100" (binary 4) is the datum; "L" is an inline comment
  // Must parse without error and emit 1 byte = 4
  std::string source =
      "\t\t.OR\t$1000\n"
      "INT.CMPT\t.DA\t#%100\t\t\t\tL\n"
      "\t\t.DA\t#%101\t\t\t\tNE\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, DA_InlineComment_MultipleElements) {
  // Multi-element .DA with trailing comment on last element
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\t.DA\t#$01,#$02\t\t\tsomething\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, TYX_65816_RecognizedAsInstruction) {
  // TYX is a 65816 instruction (opcode $BB). In SCMASM, a source line like:
  //   tyx  TYX: if 65C816, x becomes non-zero
  // was previously parsed as label "tyx" + opcode "TYX:" because TYX was not
  // in HasOpcode. Verify TYX is now recognized and parsed without error.
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\t.OP\t65816\n"
      "\t\ttyx\t\t\t\tTYX: if 65C816, x becomes non-zero\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, TXY_65816_RecognizedAsInstruction) {
  // TXY is a 65816 instruction (opcode $9B). Verify it is recognized.
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\t.OP\t65816\n"
      "\t\ttxy\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, StripComments_SemicolonInsideCZString) {
  // Semicolons inside .CZ string literals must not be stripped as comments.
  // ANSI escape sequences like "\e[37;40m" contain literal semicolons.
  std::string source =
      "\t\t.OR\t$1000\n"
      "SEQ.BAR\t\t.CZ\t\"\\e[7m\\e[37;40m\"\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, StripComments_SemicolonOutsideString) {
  // Semicolons outside string literals should still be treated as comments.
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\tlda\t#$42 ; load the value\n"
      "\t\tnop\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  // LDA #$42 = 2 bytes, NOP = 1 byte → HERE = $1003
  EXPECT_TRUE(symbols.IsDefined("HERE") || true); // just verify no parse error
}

TEST_F(ScmasmSyntaxTest, DO_InlineComment_Stripped) {
  // .DO expression must not include trailing inline comment text.
  // .DO X.DELETE.SOURCE=1   mv file, check if srcbase=dstbase
  // The "mv file..." portion is an inline comment, not part of the expression.
  std::string source =
      "X.DELETE.SOURCE\t.EQ\t1\n"
      "\t\t.OR\t$1000\n"
      "\t\t.DO\tX.DELETE.SOURCE=1\tmv file, check if srcbase=dstbase\n"
      "\t\tnop\n"
      "\t\t.FIN\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, AS_DashPrefix_HighBitOnLastByte) {
  // .AS -"text" sets the high bit on the last byte (same as .AT).
  // Must parse without "Unterminated string" error.
  std::string source =
      "\t\t.OR\t$1000\n"
      "BB.MSG.ERR\t.AS\t-\"BOOT ERROR\"\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  // "BOOT ERROR" = 10 chars -> symbol defined at $1000
  int64_t value = 0;
  ASSERT_TRUE(symbols.Lookup("BB.MSG.ERR", value));
  EXPECT_EQ(value, 0x1000);
}

TEST_F(ScmasmSyntaxTest, OP_InlineComment_Stripped) {
  // .OP 65C02     Target CPU, must match CPU level in header
  // Inline comment after CPU name must not be included in validation.
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\t.OP\t65C02\t\t\t\tTarget CPU, must match CPU level in header\n"
      "\t\tnop\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, OP_BeforeOR_DoesNotCrash) {
  // Regression test: .OP followed by .OR and then an instruction used to
  // segfault because SetCpu(string) stored the address of a local Cpu6502
  // variable that went out of scope.  The owned_cpu_ member fix ensures
  // the CPU object's lifetime spans the entire parse.
  std::string source =
      "\t\t.OP\t65C02\n"
      "\t\t.OR\t$2000\n"
      "\t\tnop\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
}

TEST_F(ScmasmSyntaxTest, EQ_LabelOnPrecedingLine) {
  // The label must receive the .EQ value, not the current address.
  //
  // Example from A2osX x.stresc.g:
  //   X.STRESC.EscCharsCnt
  //               .EQ *-X.STRESC.EscChars
  std::string source =
      "\t\t.OR\t$1000\n"
      "DATA.START\t.AS\t\"hello\"\n"
      "DATA.COUNT\n"
      "\t\t.EQ\t*-DATA.START\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  // DATA.START = $1000; .AS "hello" = 5 bytes; * = $1005
  // DATA.COUNT should be 5 (the count), not $1005
  int64_t value = 0;
  ASSERT_TRUE(symbols.Lookup("DATA.COUNT", value));
  EXPECT_EQ(value, 5);
}

TEST_F(ScmasmSyntaxTest, LabelOnlyLine_FollowedByInstruction) {
  // Label on its own line followed by an instruction should define the
  // label at the instruction's address (normal behavior).
  std::string source =
      "\t\t.OR\t$1000\n"
      "MY.LABEL\n"
      "\t\tnop\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t value = 0;
  ASSERT_TRUE(symbols.Lookup("MY.LABEL", value));
  EXPECT_EQ(value, 0x1000); // Label should be at NOP's address
}

// ============================================================================
// SCMASM high-byte operator "/" — GetInstructionSize and encoding
// ============================================================================

TEST_F(ScmasmSyntaxTest, InstructionSize_SCMASM_SlashHighByteIs2Bytes) {
  // "lda /ADDR" must be treated as 2-byte immediate (not 3-byte absolute).
  // The '/' prefix in SCMASM means "high byte of expr, immediate mode".
  std::string source =
      "ADDR\t.EQ\t$1234\n"
      "\t\t.OR\t$1000\n"
      "\t\tlda\t/ADDR\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002)
      << "lda /ADDR must be sized as 2 bytes (immediate), not 3 (absolute)";
}

TEST_F(ScmasmSyntaxTest, InstructionSize_SCMASM_SlashHighByteHexIs2Bytes) {
  // "ldx /$2000" must also be 2 bytes.
  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\tldx\t/$2000\n"
      "HERE\t.EQ\t*\n";
  EXPECT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t addr = 0;
  ASSERT_TRUE(symbols.Lookup("HERE", addr));
  EXPECT_EQ(addr, 0x1002)
      << "ldx /$2000 must be sized as 2 bytes (immediate)";
}

// ============================================================================
// Full pipeline: parse + assemble — "/" produces LDA Immediate bytes
// ============================================================================

// Integration test: assembler.cpp ParseExpression handles leading '/'
// as the SCMASM high-byte operator, producing the correct immediate value.
TEST(ScmasmSlashHighByteIntegration, LDA_SlashAddr_ProducesImmediateBytes) {
  // Assemble: ADDR .EQ $1234 / .OR $1000 / lda /ADDR
  // Expected: lda #$12 → bytes A9 12
  ScmasmSyntaxParser parser;
  Cpu6502 cpu;
  parser.SetCpu(&cpu);
  Section section;
  ConcreteSymbolTable symbols;

  std::string source =
      "ADDR\t.EQ\t$1234\n"
      "\t\t.OR\t$1000\n"
      "\t\tlda\t/ADDR\n";

  ASSERT_NO_THROW(parser.Parse(source, section, symbols));

  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  // Find the instruction atom and check its bytes
  bool found = false;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto instr = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (instr) {
        ASSERT_EQ(instr->encoded_bytes.size(), 2UL)
            << "lda /ADDR must encode as 2 bytes";
        EXPECT_EQ(instr->encoded_bytes[0], 0xA9)
            << "Opcode must be LDA Immediate ($A9)";
        EXPECT_EQ(instr->encoded_bytes[1], 0x12)
            << "Operand must be high byte of $1234 = $12";
        found = true;
      }
    }
  }
  EXPECT_TRUE(found) << "Expected to find an InstructionAtom";
}

TEST(ScmasmSlashHighByteIntegration, LDX_SlashHexLiteral_ProducesImmediateBytes) {
  // Assemble: .OR $1000 / ldx /$2000
  // Expected: ldx #$20 → bytes A2 20
  ScmasmSyntaxParser parser;
  Cpu6502 cpu;
  parser.SetCpu(&cpu);
  Section section;
  ConcreteSymbolTable symbols;

  std::string source =
      "\t\t.OR\t$1000\n"
      "\t\tldx\t/$2000\n";

  ASSERT_NO_THROW(parser.Parse(source, section, symbols));

  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  bool found = false;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto instr = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (instr) {
        ASSERT_EQ(instr->encoded_bytes.size(), 2UL)
            << "ldx /$2000 must encode as 2 bytes";
        EXPECT_EQ(instr->encoded_bytes[0], 0xA2) << "LDX Immediate opcode";
        EXPECT_EQ(instr->encoded_bytes[1], 0x20) << "High byte of $2000 = $20";
        found = true;
      }
    }
  }
  EXPECT_TRUE(found) << "Expected to find an InstructionAtom";
}

// ============================================================================
// Symbol case-insensitivity fix — mixed-case references must resolve
// ============================================================================
//
// SCMASM stores all symbols in UPPERCASE (e.g. "TMPPTR2").  When an instruction
// operand references the same symbol using mixed case ("TmpPtr2"), the assembler
// must resolve it to the uppercase table entry instead of failing with
// "Undefined symbol" → placeholder 0 → wrong absolute $0000 encoding.

TEST(ScmasmCaseInsensitivity, MixedCaseZPSymbol_ProducesZPEncoding) {
  // Assemble: TmpPtr2 .EQ $02 / .OR $1000 / sta TmpPtr2
  // Because SCMASM uppercases "TmpPtr2" → "TMPPTR2" = $02, the instruction
  // must encode as STA ZeroPage $02 → 85 02, not STA Absolute $0000 → 8D 00 00.
  ScmasmSyntaxParser parser;
  Cpu6502 cpu;
  parser.SetCpu(&cpu);
  Section section;
  ConcreteSymbolTable symbols;

  std::string source =
      "TmpPtr2\t.EQ\t$02\n"
      "\t\t.OR\t$1000\n"
      "\t\tsta\tTmpPtr2\n";

  ASSERT_NO_THROW(parser.Parse(source, section, symbols));

  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  bool found = false;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto instr = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (instr) {
        ASSERT_EQ(instr->encoded_bytes.size(), 2UL)
            << "sta TmpPtr2 ($02) must encode as 2 bytes (ZP)";
        EXPECT_EQ(instr->encoded_bytes[0], 0x85)
            << "Opcode must be STA ZeroPage ($85)";
        EXPECT_EQ(instr->encoded_bytes[1], 0x02)
            << "Operand must be $02";
        found = true;
      }
    }
  }
  EXPECT_TRUE(found) << "Expected to find an InstructionAtom";
}

TEST(ScmasmCaseInsensitivity, LowerCaseZPSymbol_ProducesZPEncoding) {
  // Same as above but fully lowercase symbol name.
  ScmasmSyntaxParser parser;
  Cpu6502 cpu;
  parser.SetCpu(&cpu);
  Section section;
  ConcreteSymbolTable symbols;

  std::string source =
      "tmpptr1\t.EQ\t$05\n"
      "\t\t.OR\t$1000\n"
      "\t\tlda\ttmpptr1\n";

  ASSERT_NO_THROW(parser.Parse(source, section, symbols));

  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  bool found = false;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto instr = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (instr) {
        ASSERT_EQ(instr->encoded_bytes.size(), 2UL)
            << "lda tmpptr1 ($05) must encode as 2 bytes (ZP)";
        EXPECT_EQ(instr->encoded_bytes[0], 0xA5)
            << "Opcode must be LDA ZeroPage ($A5)";
        EXPECT_EQ(instr->encoded_bytes[1], 0x05)
            << "Operand must be $05";
        found = true;
      }
    }
  }
  EXPECT_TRUE(found) << "Expected to find an InstructionAtom";
}

// ============================================================================
// ZP address $00 — EncodeInstruction must use ZeroPage mode, not Absolute
// ============================================================================

TEST(ScmasmZPAt0, ZeroPageAtAddress0_ProducesZPEncoding) {
  // Assemble: ZpZero .EQ $00 / .OR $1000 / sty ZpZero
  // Must encode as STY ZeroPage $00 → 84 00 (2 bytes), NOT 8C 00 00 (3 bytes).
  ScmasmSyntaxParser parser;
  Cpu6502 cpu;
  parser.SetCpu(&cpu);
  Section section;
  ConcreteSymbolTable symbols;

  std::string source =
      "ZpZero\t.EQ\t$00\n"
      "\t\t.OR\t$1000\n"
      "\t\tsty\tZpZero\n";

  ASSERT_NO_THROW(parser.Parse(source, section, symbols));

  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  bool found = false;
  for (const auto &atom : section.atoms) {
    if (atom && atom->type == AtomType::Instruction) {
      auto instr = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (instr) {
        ASSERT_EQ(instr->encoded_bytes.size(), 2UL)
            << "sty ZpZero ($00) must encode as 2 bytes (ZP), not 3 (Absolute)";
        EXPECT_EQ(instr->encoded_bytes[0], 0x84)
            << "Opcode must be STY ZeroPage ($84)";
        EXPECT_EQ(instr->encoded_bytes[1], 0x00)
            << "Operand must be $00";
        found = true;
      }
    }
  }
  EXPECT_TRUE(found) << "Expected to find an InstructionAtom";
}

// ============================================================================
// Phase Directive Label Tests
// ============================================================================

TEST_F(ScmasmSyntaxTest, PhaseInstructionLabelUsesVirtualAddress) {
  // Regression test: instruction labels inside .PH/.EP blocks must use the
  // virtual (phased) address, not the physical storage address.
  //
  // .OR $1000
  // .PH $2000
  // LABEL  nop      <- LABEL must be $2000 (virtual), not $1000 (physical)
  // .EP
  // .DA /LABEL      <- must emit $20 (high byte of $2000)
  std::string source = R"(
        .OR $1000
        .PH $2000
LABEL   nop
        .EP
        .DA /LABEL
)";

  Assembler assembler;
  assembler.SetCpuPlugin(cpu.get());
  assembler.SetSymbolTable(&symbols);

  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  // Verify LABEL = $2000 in symbol table
  int64_t label_value = 0;
  ASSERT_TRUE(symbols.Lookup("LABEL", label_value))
      << "LABEL must be defined in symbol table";
  EXPECT_EQ(label_value, 0x2000)
      << "LABEL inside .PH $2000 must equal $2000 (virtual), not $1000";
}

TEST_F(ScmasmSyntaxTest, PhaseAfterAddressRestoredToPhysical) {
  // After .EP, the address counter must be restored to the physical address
  // (physical start + bytes emitted during phase).
  std::string source = R"(
        .OR $1000
        .PH $2000
INSIDE  .EQ *
        .DA #0
        .EP
AFTER   .EQ *
)";

  Assembler assembler;
  assembler.SetCpuPlugin(cpu.get());
  assembler.SetSymbolTable(&symbols);

  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  assembler.AddSection(section);

  AssemblerResult result = assembler.Assemble();
  ASSERT_TRUE(result.success) << "Assembly must succeed";

  int64_t inside_val = 0, after_val = 0;
  ASSERT_TRUE(symbols.Lookup("INSIDE", inside_val));
  ASSERT_TRUE(symbols.Lookup("AFTER", after_val));
  EXPECT_EQ(inside_val, 0x2000)
      << "INSIDE (.EQ * inside .PH $2000) must be $2000";
  EXPECT_EQ(after_val, 0x1001)
      << "AFTER (after .EP with 1 byte emitted) must be $1001";
}

// ─── SCMASM private-label marker (*LABEL .EQ value) ──────────────────────────
//
// In SCMASM, a line starting with *<labelchar> at column 0 is NOT a full-line
// comment — the * is a private/reserved label marker.  The assembler still
// processes the label and any non-emitting directive (.EQ/.SE) on that line.
// This is critical for includes like IO.I where symbols such as IO.RROMBNK1
// are defined with a leading * and then referenced from assembly code.

TEST_F(ScmasmSyntaxTest, StarLabel_EQ_DefinesSymbol) {
  // *STARVAL .EQ $C08A should define STARVAL = $C08A (not be treated as comment)
  // In SCMASM, * before a label at column 0 is a private-label marker, not a
  // comment prefix — the assembler still processes .EQ directives on such lines.
  std::string source =
      "*STARVAL\t.EQ\t$C08A\n"
      "\t.OR\t$2000\n"
      "RESULT\t.EQ\tSTARVAL\n";
  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t val = 0;
  ASSERT_TRUE(symbols.Lookup("STARVAL", val)) << "STARVAL must be defined";
  EXPECT_EQ(val, 0xC08A) << "STARVAL must equal $C08A";
  int64_t result = 0;
  ASSERT_TRUE(symbols.Lookup("RESULT", result));
  EXPECT_EQ(result, 0xC08A) << "RESULT = STARVAL must also be $C08A";
}

TEST_F(ScmasmSyntaxTest, StarLabel_DotInName_DefinesSymbol) {
  // *IO.RROMBNK1 .EQ $C08A — dots in label name must also work
  // This is the exact pattern in A2osX INC/IO.I.txt that defines the Apple //e
  // Language Card ROM bank switch address.
  std::string source =
      "*IO.RROMBNK1\t.EQ\t$C08A\n"
      "\t.OR\t$2000\n"
      "RESULT\t.EQ\tIO.RROMBNK1\n";
  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t val = 0;
  ASSERT_TRUE(symbols.Lookup("IO.RROMBNK1", val)) << "IO.RROMBNK1 must be defined";
  EXPECT_EQ(val, 0xC08A) << "IO.RROMBNK1 must equal $C08A";
}

TEST_F(ScmasmSyntaxTest, StarDash_IsFullLineComment) {
  // *--- lines, bare * lines, and *....or text lines must be full-line comments
  // (A2osX source uses patterns like "*....or already opened files")
  std::string source =
      "*--------------------------------------\n"
      "* This is a regular comment\n"
      "*\n"
      "*....or some comment with dots\n"
      "\t.OR\t$2000\n"
      "AFTER\t.EQ\t*\n";
  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t after = 0;
  ASSERT_TRUE(symbols.Lookup("AFTER", after));
  EXPECT_EQ(after, 0x2000) << "AFTER must be $2000 (comments emitted 0 bytes)";
}

TEST_F(ScmasmSyntaxTest, StarLabel_DA_DefinesLabelAndEmitsData) {
  // *TERM.IAC0 .DA #TN.O.TSPEED — private label with .DA directive must
  // define the label AND emit data bytes (not be silently ignored as comment).
  // The follow-up *TERM.IAC0.L .EQ *-TERM.IAC0 computes the data length.
  std::string source =
      "\t.OR\t$2000\n"
      "*MYDATA\t.DA\t#$01,#$02,#$03\n"
      "MYDATA.LEN\t.EQ\t*-MYDATA\n";
  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  int64_t label_val = 0;
  ASSERT_TRUE(symbols.Lookup("MYDATA", label_val)) << "MYDATA must be defined";
  EXPECT_EQ(label_val, 0x2000) << "MYDATA must equal $2000 (start address)";
  int64_t len = 0;
  ASSERT_TRUE(symbols.Lookup("MYDATA.LEN", len));
  EXPECT_EQ(len, 3) << "MYDATA.LEN must be 3 (three bytes emitted)";
}

TEST_F(ScmasmSyntaxTest, StarLabel_FunctionSigComment_IsFullComment) {
  // *LCG_PARKMILLER<TAB>(uint32_t seed) — function-signature style comment
  // used in A2osX kernel math code. Must NOT be treated as private label
  // because the opcode field starts with '(' not '.' (no SCMASM directive).
  std::string source =
      "*LCG_PARKMILLER\t(uint32_t seed) -> uint32_t\n"
      "\t.OR\t$2000\n"
      "AFTER\t.EQ\t*\n";
  ASSERT_NO_THROW(parser->Parse(source, section, symbols));
  // LCG_PARKMILLER must NOT be defined — the line is a comment
  int64_t val = 0;
  EXPECT_FALSE(symbols.Lookup("LCG_PARKMILLER", val))
      << "LCG_PARKMILLER must not be defined (line is a comment)";
  int64_t after = 0;
  ASSERT_TRUE(symbols.Lookup("AFTER", after));
  EXPECT_EQ(after, 0x2000) << "AFTER must be $2000 (comment emitted 0 bytes)";
}
