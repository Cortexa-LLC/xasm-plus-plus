// SCMASM Syntax Parser Tests - Phase 1: Foundation
// TDD approach: Tests written first

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
  ASSERT_TRUE(symbols.Lookup("FPU.f", value));
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
  // .AS with delimiter " (0x22 < 0x27) → high bit SET
  parser->Parse("        .AS \"HELLO\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u);
  EXPECT_EQ(data_atom->data[0], 0xC8); // 'H' with high bit SET
  EXPECT_EQ(data_atom->data[1], 0xC5); // 'E' with high bit SET
  EXPECT_EQ(data_atom->data[2], 0xCC); // 'L' with high bit SET
  EXPECT_EQ(data_atom->data[3], 0xCC); // 'L' with high bit SET
  EXPECT_EQ(data_atom->data[4], 0xCF); // 'O' with high bit SET
}

TEST_F(ScmasmSyntaxTest, AsDirectiveHighBitRule) {
  // Delimiter < 0x27 should SET high bit for .AS
  parser->Parse("        .AS \"A\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 1u);
  EXPECT_EQ(data_atom->data[0], 0xC1); // 'A' with high bit SET (0x41 | 0x80)
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
  // .AT with delimiter < 0x27 should set high bit on ALL chars
  // Then set high bit on LAST char (which already has it)
  parser->Parse("        .AT \"AB\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 2u);
  EXPECT_EQ(data_atom->data[0], 0xC1); // 'A' with high bit from delimiter
  EXPECT_EQ(data_atom->data[1],
            0xC2); // 'B' with high bit (delimiter + .AT rule)
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
  // .AZ respects delimiter high-bit rule
  parser->Parse("        .AZ \"TEST\"\n", section, symbols);

  ASSERT_EQ(section.atoms.size(), 1u);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->data.size(), 5u); // 4 chars + null
  EXPECT_EQ(data_atom->data[0], 0xD4);   // 'T' with high bit
  EXPECT_EQ(data_atom->data[1], 0xC5);   // 'E' with high bit
  EXPECT_EQ(data_atom->data[2], 0xD3);   // 'S' with high bit
  EXPECT_EQ(data_atom->data[3], 0xD4);   // 'T' with high bit
  EXPECT_EQ(data_atom->data[4], 0x00);   // null terminator (no high bit)
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
  EXPECT_EQ(section.atoms.size(), 11u); // ORG + 10 NOPs
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
  std::remove(include_file.c_str());
  std::remove(main_file.c_str());
  std::filesystem::remove("test_subdir");
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
  std::string source = R"(
        .OR $0800
        .CS "Say \"Hi\""
)";

  parser->Parse(source, section, symbols);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // \" should become double quote
  std::vector<uint8_t> expected = {'S', 'a', 'y', ' ', '"', 'H', 'i', '"'};
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

  // Should have OrgAtom and only one DataAtom (from DATA .BS 3)
  ASSERT_EQ(section.atoms.size(), 2u);

  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[1]);
  ASSERT_NE(data_atom, nullptr);

  // Only DATA .BS 3 should emit bytes (3 bytes)
  EXPECT_EQ(data_atom->data.size(), 3u);
}

TEST_F(ScmasmSyntaxTest, DUMMY_AdvancesAddress) {
  // .DUMMY should advance address even though no bytes emitted
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

  // Check that START and END are defined
  int64_t start_addr, end_addr;
  ASSERT_TRUE(symbols.Lookup("START", start_addr));
  ASSERT_TRUE(symbols.Lookup("END", end_addr));

  // START should be $0800, END should be $0807 (7 bytes advanced)
  EXPECT_EQ(start_addr, 0x0800);
  EXPECT_EQ(end_addr, 0x0807);
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
