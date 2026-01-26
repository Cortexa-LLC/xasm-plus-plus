// MerlinSyntax Parser tests
// Phases 1-3: Foundation, Local Labels, DUM Blocks
// Phases 4-6: PUT, DFB, DO/FIN

#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>

using namespace xasm;

// ============================================================================
// Phase 1: Foundation - Comment Parsing
// ============================================================================

// RED: Test asterisk comment in column 1
TEST(MerlinSyntaxTest, CommentWithAsterisk) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("* This is a comment", section, symbols);

    // Comment lines should produce no atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test semicolon inline comment
TEST(MerlinSyntaxTest, CommentWithSemicolon) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("; This is a comment", section, symbols);

    // Comment lines should produce no atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test inline comment after directive
TEST(MerlinSyntaxTest, InlineComment) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ORG $8000  ; Set origin", section, symbols);

    // Should create ORG atom, ignoring comment
    ASSERT_EQ(section.atoms.size(), 1);
    auto atom = section.atoms[0];
    EXPECT_EQ(atom->type, AtomType::Org);
}

// ============================================================================
// Phase 1: Foundation - Global Label Parsing
// ============================================================================

// RED: Test global label definition
TEST(MerlinSyntaxTest, GlobalLabel) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("START", section, symbols);

    // Should create label symbol and LabelAtom
    EXPECT_TRUE(symbols.IsDefined("START"));

    ASSERT_EQ(section.atoms.size(), 1);
    auto atom = section.atoms[0];
    EXPECT_EQ(atom->type, AtomType::Label);

    auto label_atom = std::dynamic_pointer_cast<LabelAtom>(atom);
    ASSERT_NE(label_atom, nullptr);
    EXPECT_EQ(label_atom->name, "START");
}

// RED: Test global label with instruction
TEST(MerlinSyntaxTest, GlobalLabelWithInstruction) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("LOOP     LDA #$00", section, symbols);

    // Should create label and instruction atoms
    EXPECT_TRUE(symbols.IsDefined("LOOP"));
    ASSERT_EQ(section.atoms.size(), 2);

    EXPECT_EQ(section.atoms[0]->type, AtomType::Label);
    EXPECT_EQ(section.atoms[1]->type, AtomType::Instruction);
}

// ============================================================================
// Phase 1: Foundation - ORG Directive
// ============================================================================

// RED: Test ORG directive with hex value
TEST(MerlinSyntaxTest, OrgDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ORG $8000", section, symbols);

    // Should create OrgAtom
    ASSERT_EQ(section.atoms.size(), 1);

    auto atom = section.atoms[0];
    EXPECT_EQ(atom->type, AtomType::Org);

    auto org_atom = std::dynamic_pointer_cast<OrgAtom>(atom);
    ASSERT_NE(org_atom, nullptr);
    EXPECT_EQ(org_atom->address, 0x8000);
}

// RED: Test ORG with decimal value
TEST(MerlinSyntaxTest, OrgWithDecimal) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ORG 32768", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
    ASSERT_NE(org_atom, nullptr);
    EXPECT_EQ(org_atom->address, 32768);
}

// ============================================================================
// Phase 1: Foundation - Number Formats
// ============================================================================

// RED: Test hex number parsing
TEST(MerlinSyntaxTest, HexNumber) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DB $FF", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0xFF);
}

// RED: Test binary number parsing
TEST(MerlinSyntaxTest, BinaryNumber) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DB %11110000", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0xF0);
}

// RED: Test decimal number parsing
TEST(MerlinSyntaxTest, DecimalNumber) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DB 42", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 42);
}

// ============================================================================
// Phase 1: Foundation - EQU Directive
// ============================================================================

// RED: Test EQU directive
TEST(MerlinSyntaxTest, EquDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("CONST    EQU $FF", section, symbols);

    // Should define symbol, but create no atoms
    EXPECT_TRUE(symbols.IsDefined("CONST"));
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test EQU with expression
TEST(MerlinSyntaxTest, EquWithExpression) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("VALUE    EQU $100+$20", section, symbols);

    EXPECT_TRUE(symbols.IsDefined("VALUE"));
    int64_t value;
    EXPECT_TRUE(symbols.Lookup("VALUE", value));
}

// RED: Test = syntax (alternate EQU syntax)
TEST(MerlinSyntaxTest, EqualsDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("CONST = $FF", section, symbols);

    // Should define symbol using = syntax, but create no atoms
    EXPECT_TRUE(symbols.IsDefined("CONST"));
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test = syntax with hex value
TEST(MerlinSyntaxTest, EqualsWithHex) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("rw18 = $d000", section, symbols);

    EXPECT_TRUE(symbols.IsDefined("rw18"));
    int64_t value;
    EXPECT_TRUE(symbols.Lookup("rw18", value));
    EXPECT_EQ(value, 0xd000);
}

// RED: Test DUM with symbol reference
TEST(MerlinSyntaxTest, DumWithSymbol) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        "master = $f880\n"
        " dum master\n"
        "_firstboot ds 3\n"
        " dend\n";

    parser.Parse(source, section, symbols);

    // Should define master
    EXPECT_TRUE(symbols.IsDefined("master"));
    // Should define _firstboot in DUM block
    EXPECT_TRUE(symbols.IsDefined("_firstboot"));
}

// RED: Test ORG with symbol reference
TEST(MerlinSyntaxTest, OrgWithSymbol) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        "start = $8000\n"
        " org start\n"
        " nop\n";

    parser.Parse(source, section, symbols);

    // Should define start
    EXPECT_TRUE(symbols.IsDefined("start"));
    // Should have ORG atom with correct address
    ASSERT_GE(section.atoms.size(), 1);
    auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section.atoms[0]);
    ASSERT_NE(org_atom, nullptr);
    EXPECT_EQ(org_atom->address, 0x8000);
}

// ============================================================================
// Phase 1: Foundation - DB Directive
// ============================================================================

// RED: Test DB with single byte
TEST(MerlinSyntaxTest, DbSingleByte) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DB $42", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0x42);
}

// RED: Test DB with multiple bytes
TEST(MerlinSyntaxTest, DbMultipleBytes) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DB $01,$02,$03", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 3);
    EXPECT_EQ(data_atom->data[0], 0x01);
    EXPECT_EQ(data_atom->data[1], 0x02);
    EXPECT_EQ(data_atom->data[2], 0x03);
}

// ============================================================================
// Phase 1: Foundation - DW Directive
// ============================================================================

// RED: Test DW with single word
TEST(MerlinSyntaxTest, DwSingleWord) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DW $1234", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 2);
    EXPECT_EQ(data_atom->data[0], 0x34);  // Low byte (little-endian)
    EXPECT_EQ(data_atom->data[1], 0x12);  // High byte
}

// RED: Test DW with multiple words
TEST(MerlinSyntaxTest, DwMultipleWords) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DW $ABCD,$EF01", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 4);
    EXPECT_EQ(data_atom->data[0], 0xCD);  // Low byte of $ABCD
    EXPECT_EQ(data_atom->data[1], 0xAB);  // High byte of $ABCD
    EXPECT_EQ(data_atom->data[2], 0x01);  // Low byte of $EF01
    EXPECT_EQ(data_atom->data[3], 0xEF);  // High byte of $EF01
}

// ============================================================================
// Phase 1: Foundation - HEX Directive
// ============================================================================

// RED: Test HEX directive
TEST(MerlinSyntaxTest, HexDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         HEX 0102030405", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 5);
    EXPECT_EQ(data_atom->data[0], 0x01);
    EXPECT_EQ(data_atom->data[1], 0x02);
    EXPECT_EQ(data_atom->data[2], 0x03);
    EXPECT_EQ(data_atom->data[3], 0x04);
    EXPECT_EQ(data_atom->data[4], 0x05);
}

// RED: Test HEX with spaces
TEST(MerlinSyntaxTest, HexWithSpaces) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         HEX AB CD EF", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 3);
    EXPECT_EQ(data_atom->data[0], 0xAB);
    EXPECT_EQ(data_atom->data[1], 0xCD);
    EXPECT_EQ(data_atom->data[2], 0xEF);
}

// ============================================================================
// Phase 4: PUT Directive (File Inclusion)
// ============================================================================

// RED: Test basic PUT directive
TEST(MerlinSyntaxTest, PutDirectiveBasic) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Create a temporary test file
    std::string test_file = "/tmp/xasm_test_include.asm";
    std::ofstream out(test_file);
    out << "INCLUDED_LABEL\n";
    out << "         DB $42\n";
    out.close();

    // Parse PUT directive
    parser.Parse("         PUT /tmp/xasm_test_include.asm", section, symbols);

    // Should have included the label and DB directive
    EXPECT_TRUE(symbols.IsDefined("INCLUDED_LABEL"));
    ASSERT_GE(section.atoms.size(), 1);

    // Clean up
    std::remove(test_file.c_str());
}

// RED: Test PUT circular include detection (self-reference)
TEST(MerlinSyntaxTest, PutCircularIncludeSelf) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Create a file that includes itself
    std::string test_file = "/tmp/xasm_test_circular.asm";
    std::ofstream out(test_file);
    out << " PUT /tmp/xasm_test_circular.asm\n";
    out.close();

    // Should throw exception for circular include
    EXPECT_THROW(
        parser.Parse(" PUT /tmp/xasm_test_circular.asm", section, symbols),
        std::runtime_error
    );

    // Clean up
    std::remove(test_file.c_str());
}

// RED: Test PUT circular include detection (A includes B includes A)
TEST(MerlinSyntaxTest, PutCircularIncludeCycle) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Create file A that includes B
    std::string file_a = "/tmp/xasm_test_a.asm";
    std::ofstream out_a(file_a);
    out_a << " PUT /tmp/xasm_test_b.asm\n";
    out_a.close();

    // Create file B that includes A (creates cycle)
    std::string file_b = "/tmp/xasm_test_b.asm";
    std::ofstream out_b(file_b);
    out_b << " PUT /tmp/xasm_test_a.asm\n";
    out_b.close();

    // Should throw exception for circular include
    EXPECT_THROW(
        parser.Parse(" PUT /tmp/xasm_test_a.asm", section, symbols),
        std::runtime_error
    );

    // Clean up
    std::remove(file_a.c_str());
    std::remove(file_b.c_str());
}

// RED: Test PUT with nested includes (3 levels deep)
TEST(MerlinSyntaxTest, PutNestedIncludes3Levels) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Create level 3 file (innermost)
    std::string file_3 = "/tmp/xasm_test_level3.asm";
    std::ofstream out_3(file_3);
    out_3 << "LEVEL3_LABEL\n";
    out_3 << " DB $03\n";
    out_3.close();

    // Create level 2 file that includes level 3
    std::string file_2 = "/tmp/xasm_test_level2.asm";
    std::ofstream out_2(file_2);
    out_2 << "LEVEL2_LABEL\n";
    out_2 << " DB $02\n";
    out_2 << " PUT /tmp/xasm_test_level3.asm\n";
    out_2.close();

    // Create level 1 file that includes level 2
    std::string file_1 = "/tmp/xasm_test_level1.asm";
    std::ofstream out_1(file_1);
    out_1 << "LEVEL1_LABEL\n";
    out_1 << " DB $01\n";
    out_1 << " PUT /tmp/xasm_test_level2.asm\n";
    out_1.close();

    // Parse main file that includes level 1 (total 3 levels of nesting)
    parser.Parse(" PUT /tmp/xasm_test_level1.asm", section, symbols);

    // Should have all three labels defined
    EXPECT_TRUE(symbols.IsDefined("LEVEL1_LABEL"));
    EXPECT_TRUE(symbols.IsDefined("LEVEL2_LABEL"));
    EXPECT_TRUE(symbols.IsDefined("LEVEL3_LABEL"));

    // Clean up
    std::remove(file_1.c_str());
    std::remove(file_2.c_str());
    std::remove(file_3.c_str());
}

// RED: Test PUT file not found error
TEST(MerlinSyntaxTest, PutFileNotFound) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Try to include a non-existent file
    EXPECT_THROW(
        parser.Parse(" PUT /tmp/nonexistent_file_xasm.asm", section, symbols),
        std::runtime_error
    );
}

// RED: Test PUT with empty file
TEST(MerlinSyntaxTest, PutEmptyFile) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Create an empty file
    std::string test_file = "/tmp/xasm_test_empty.asm";
    std::ofstream out(test_file);
    out.close();

    // Should handle empty file gracefully
    parser.Parse(" PUT /tmp/xasm_test_empty.asm", section, symbols);

    // Should have no atoms added
    EXPECT_EQ(section.atoms.size(), 0);

    // Clean up
    std::remove(test_file.c_str());
}

// ============================================================================
// Phase 5: DFB Directive (Define Font Byte)
// ============================================================================

// RED: Test DFB with single byte
TEST(MerlinSyntaxTest, DfbSingleByte) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DFB $42", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0x42);
}

// RED: Test DFB with multiple bytes
TEST(MerlinSyntaxTest, DfbMultipleBytes) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         DFB $01,$02,$03,$04", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 4);
    EXPECT_EQ(data_atom->data[0], 0x01);
    EXPECT_EQ(data_atom->data[1], 0x02);
    EXPECT_EQ(data_atom->data[2], 0x03);
    EXPECT_EQ(data_atom->data[3], 0x04);
}

// RED: Test DFB with signed negative byte
TEST(MerlinSyntaxTest, DfbSignedNegativeByte) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // -1 should be stored as $FF
    parser.Parse("         DFB -1", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0xFF);
}

// RED: Test DFB with high bit set (font mode)
TEST(MerlinSyntaxTest, DfbHighBit) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // $80 + value sets high bit
    parser.Parse("         DFB $80+$41", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0xC1);  // $80 | $41
}

// RED: Test DFB with expression
TEST(MerlinSyntaxTest, DfbWithExpression) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("VALUE    EQU $10", section, symbols);
    parser.Parse("         DFB VALUE+5", section, symbols);

    ASSERT_GE(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[section.atoms.size()-1]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 1);
    EXPECT_EQ(data_atom->data[0], 0x15);  // $10 + 5
}

// RED: Test DFB with label
TEST(MerlinSyntaxTest, DfbWithLabel) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("DATA     DFB $01,$02", section, symbols);

    EXPECT_TRUE(symbols.IsDefined("DATA"));
    ASSERT_GE(section.atoms.size(), 1);
}

// ============================================================================
// Phase 6: DO/ELSE/FIN Conditional Assembly
// ============================================================================

// RED: Test DO/FIN with true condition (code should be included)
TEST(MerlinSyntaxTest, DoFinTrue) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        " DO 1\n"
        " DB $42\n"
        " FIN\n";

    parser.Parse(source, section, symbols);

    // Code inside DO 1...FIN should be included
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data[0], 0x42);
}

// RED: Test DO/FIN with false condition (code should be excluded)
TEST(MerlinSyntaxTest, DoFinFalse) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        " DO 0\n"
        " DB $42\n"
        " FIN\n";

    parser.Parse(source, section, symbols);

    // Code inside DO 0...FIN should be excluded
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test DO/ELSE/FIN with true condition (ELSE branch excluded)
TEST(MerlinSyntaxTest, DoElseFinTrue) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        " DO 1\n"
        " DB $42\n"
        " ELSE\n"
        " DB $99\n"
        " FIN\n";

    parser.Parse(source, section, symbols);

    // DO branch included, ELSE branch excluded
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data[0], 0x42);
}

// RED: Test DO/ELSE/FIN with false condition (ELSE branch included)
TEST(MerlinSyntaxTest, DoElseFinFalse) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        " DO 0\n"
        " DB $42\n"
        " ELSE\n"
        " DB $99\n"
        " FIN\n";

    parser.Parse(source, section, symbols);

    // DO branch excluded, ELSE branch included
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data[0], 0x99);
}

// RED: Test nested DO/FIN blocks
TEST(MerlinSyntaxTest, DoFinNested) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        " DO 1\n"
        "  DO 1\n"
        "   DB $42\n"
        "  FIN\n"
        " FIN\n";

    parser.Parse(source, section, symbols);

    // Nested DO blocks both true - code should be included
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data[0], 0x42);
}

// ============================================================================
// Phase 7: Listing Control Directives (LST, LSTDO, TR)
// ============================================================================

// RED: Test LST directive (listing on)
TEST(MerlinSyntaxTest, LstDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         LST", section, symbols);

    // LST is a listing control directive - doesn't generate atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test LST OFF directive (listing off)
TEST(MerlinSyntaxTest, LstOffDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         LST OFF", section, symbols);

    // LST OFF is a listing control directive - doesn't generate atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test LSTDO directive (list during DO blocks)
TEST(MerlinSyntaxTest, LstdoDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         LSTDO", section, symbols);

    // LSTDO is a listing control directive - doesn't generate atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test TR directive (truncate listing)
TEST(MerlinSyntaxTest, TrDirective) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         TR", section, symbols);

    // TR is a listing control directive - doesn't generate atoms
    EXPECT_EQ(section.atoms.size(), 0);
}

// RED: Test TR with ADR/ON/OFF parameters
TEST(MerlinSyntaxTest, TrWithParameters) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         TR ADR", section, symbols);
    EXPECT_EQ(section.atoms.size(), 0);

    Section section2("test", 0);
    parser.Parse("         TR ON", section2, symbols);
    EXPECT_EQ(section2.atoms.size(), 0);

    Section section3("test", 0);
    parser.Parse("         TR OFF", section3, symbols);
    EXPECT_EQ(section3.atoms.size(), 0);
}

// ============================================================================
// Phase 7: ASC Directive (ASCII String)
// ============================================================================

// RED: Test ASC with simple string
TEST(MerlinSyntaxTest, AscSimpleString) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ASC 'HELLO'", section, symbols);

    // Should create DataAtom with ASCII bytes
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 5);
    EXPECT_EQ(data_atom->data[0], 'H');
    EXPECT_EQ(data_atom->data[1], 'E');
    EXPECT_EQ(data_atom->data[2], 'L');
    EXPECT_EQ(data_atom->data[3], 'L');
    EXPECT_EQ(data_atom->data[4], 'O');
}

// RED: Test ASC with double quotes
TEST(MerlinSyntaxTest, AscDoubleQuotes) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ASC \"TEST\"", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 4);
    EXPECT_EQ(data_atom->data[0], 'T');
    EXPECT_EQ(data_atom->data[1], 'E');
    EXPECT_EQ(data_atom->data[2], 'S');
    EXPECT_EQ(data_atom->data[3], 'T');
}

// RED: Test ASC with high bit set (used for inverse/flashing text)
TEST(MerlinSyntaxTest, AscHighBit) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    // Merlin allows 'text'80 to set high bit on last character
    parser.Parse("         ASC 'HI'80", section, symbols);

    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    ASSERT_EQ(data_atom->data.size(), 2);
    EXPECT_EQ(data_atom->data[0], 'H');
    EXPECT_EQ(data_atom->data[1], 'I' | 0x80);  // High bit set on last char
}

// RED: Test ASC with empty string
TEST(MerlinSyntaxTest, AscEmptyString) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    parser.Parse("         ASC ''", section, symbols);

    // Empty string should create DataAtom with no bytes
    ASSERT_EQ(section.atoms.size(), 1);
    auto data_atom = std::dynamic_pointer_cast<DataAtom>(section.atoms[0]);
    ASSERT_NE(data_atom, nullptr);
    EXPECT_EQ(data_atom->data.size(), 0);
}

// ============================================================================
// DS Directive with Expressions
// ============================================================================

// RED: Test DS with symbol reference (from PoP EQ.S)
TEST(MerlinSyntaxTest, DsWithSymbolReference) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        "maxback = 200\n"
        "bgX ds maxback\n";

    parser.Parse(source, section, symbols);

    // Should define maxback
    EXPECT_TRUE(symbols.IsDefined("maxback"));
    int64_t maxback_value;
    EXPECT_TRUE(symbols.Lookup("maxback", maxback_value));
    EXPECT_EQ(maxback_value, 200);
    
    // Should define bgX and create SpaceAtom with 200 bytes
    EXPECT_TRUE(symbols.IsDefined("bgX"));
    
    // Find the SpaceAtom
    std::shared_ptr<SpaceAtom> space_atom = nullptr;
    for (const auto& atom : section.atoms) {
        if (atom->type == AtomType::Space) {
            space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
            break;
        }
    }
    ASSERT_NE(space_atom, nullptr);
    EXPECT_EQ(space_atom->size, 200);
}

// RED: Test DS with expression (multiplication)
TEST(MerlinSyntaxTest, DsWithExpression) {
    MerlinSyntaxParser parser;
    ConcreteSymbolTable symbols;
    Section section("test", 0);

    std::string source = 
        "BLUETYPE ds 24*30\n";

    parser.Parse(source, section, symbols);

    // Should define BLUETYPE and create SpaceAtom with 720 bytes
    EXPECT_TRUE(symbols.IsDefined("BLUETYPE"));
    
    // Find the SpaceAtom
    std::shared_ptr<SpaceAtom> space_atom = nullptr;
    for (const auto& atom : section.atoms) {
        if (atom->type == AtomType::Space) {
            space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
            break;
        }
    }
    ASSERT_NE(space_atom, nullptr);
    EXPECT_EQ(space_atom->size, 720);
}
