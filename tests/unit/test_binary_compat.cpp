// Binary Compatibility Tests - vasm parity for Prince of Persia
// Tests for three specific compatibility issues:
// 1. Equate-only files (should produce 0 bytes)
// 2. ASC high-bit mode (Apple II text format)
// 3. Zero padding removal (start at ORG, not $0000)

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/output/binary_output.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/merlin_syntax.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

// ============================================================================
// Phase 1: Equate-Only Files (should produce 0-byte output)
// ============================================================================

// Test 1: File with only labels and equates (no code/data) should produce 0
// bytes
TEST(BinaryCompatTest, EquateOnlyFileProducesZeroBytes) {
  BinaryOutput output;
  Section section("test", 0);

  // Simulate an equate-only file like EQ.S:
  // - ORG directives (set addresses)
  // - Label definitions (equates)
  // - NO code or data bytes

  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
  section.atoms.push_back(std::make_shared<LabelAtom>("buffer", 0x8100));
  section.atoms.push_back(std::make_shared<OrgAtom>(0x9000));
  section.atoms.push_back(std::make_shared<LabelAtom>("data", 0x9000));

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  // Write output
  output.WriteOutput("test_equate_only.bin", sections, symbols);

  // Verify: File should be 0 bytes (like vasm)
  std::ifstream file("test_equate_only.bin", std::ios::binary);
  ASSERT_TRUE(file.is_open());
  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.close();

  EXPECT_EQ(file_size, 0)
      << "Equate-only file should produce 0-byte output (vasm compatibility)";

  // Cleanup
  std::remove("test_equate_only.bin");
}

// Test 2: File with labels AND data should produce normal output
TEST(BinaryCompatTest, FileWithDataProducesNormalOutput) {
  BinaryOutput output;
  Section section("test", 0);

  // File with labels AND data (normal case)
  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0x01, 0x02}));
  section.atoms.push_back(std::make_shared<LabelAtom>("buffer", 0x8002));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0x03, 0x04}));

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  output.WriteOutput("test_with_data.bin", sections, symbols);

  // Verify: File should contain data bytes
  std::ifstream file("test_with_data.bin", std::ios::binary);
  ASSERT_TRUE(file.is_open());
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x02);
  EXPECT_EQ(result[2], 0x03);
  EXPECT_EQ(result[3], 0x04);

  std::remove("test_with_data.bin");
}

// Test 3: File with only SpaceAtom should produce output (it writes bytes)
TEST(BinaryCompatTest, FileWithSpaceProducesOutput) {
  BinaryOutput output;
  Section section("test", 0);

  // SpaceAtom writes zero bytes (different from equate-only)
  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<SpaceAtom>(10));

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  output.WriteOutput("test_with_space.bin", sections, symbols);

  // Verify: File should contain 10 zero bytes
  std::ifstream file("test_with_space.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 10);

  std::remove("test_with_space.bin");
}

// Test 4: File with instructions should produce output
TEST(BinaryCompatTest, FileWithInstructionsProducesOutput) {
  BinaryOutput output;
  Section section("test", 0);

  // Instructions write bytes
  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  auto inst = std::make_shared<InstructionAtom>("NOP", "");
  inst->encoded_bytes = {0xEA};
  inst->size = 1;
  section.atoms.push_back(inst);

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  output.WriteOutput("test_with_inst.bin", sections, symbols);

  // Verify: File should contain instruction byte
  std::ifstream file("test_with_inst.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0xEA); // NOP opcode

  std::remove("test_with_inst.bin");
}

// ============================================================================
// Phase 3: Zero Padding Removal (don't pad from $0000 to ORG)
// ============================================================================

// Test 5: ORG to high address should NOT pad from $0000 (vasm compatibility)
TEST(BinaryCompatTest, HighOrgNoZeroPadding) {
  BinaryOutput output;
  Section section("test", 0);

  // Simulate: ORG $3000, then data
  // vasm behavior: Output starts at first data byte (no padding from $0000)
  // Current xasm++ bug: Outputs $3000 zeros before data

  section.atoms.push_back(std::make_shared<OrgAtom>(0x3000));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xE9, 0x30}));

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  output.WriteOutput("test_high_org.bin", sections, symbols);

  // Verify: File should contain ONLY the 2 data bytes (no padding)
  std::ifstream file("test_high_org.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  // CRITICAL: Should be 2 bytes, NOT 0x3002 bytes!
  ASSERT_EQ(result.size(), 2)
      << "Should not pad from $0000 to ORG (vasm compatibility)";
  EXPECT_EQ(result[0], 0xE9);
  EXPECT_EQ(result[1], 0x30);

  std::remove("test_high_org.bin");
}

// Test 6: ORG with labels then data (common pattern)
TEST(BinaryCompatTest, OrgLabelsDataNoPadding) {
  BinaryOutput output;
  Section section("test", 0);

  // Simulate typical pattern:
  // ORG $8000
  // start: label
  // data bytes

  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0x01, 0x02, 0x03}));

  ConcreteSymbolTable symbols;
  std::vector<Section *> sections = {&section};

  output.WriteOutput("test_org_label_data.bin", sections, symbols);

  // Verify: Should only contain data bytes, no padding
  std::ifstream file("test_org_label_data.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x02);
  EXPECT_EQ(result[2], 0x03);

  std::remove("test_org_label_data.bin");
}

// ============================================================================
// DUM Block Tests (Merlin syntax - dummy sections)
// ============================================================================

// Test 7: DS inside DUM block should NOT emit bytes (vasm compatibility)
TEST(BinaryCompatTest, DumBlockDsNoOutput) {
  // This tests the actual Merlin syntax parsing with DUM blocks
  // In Merlin assembler, DUM...DEND defines a "dummy section" where
  // DS directives only advance the address counter without emitting bytes

  std::string source = R"( org $f880
 dum $1000
label1 ds 10
label2 ds 20
 dend
)";

  // Parse using Merlin syntax
  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Write output
  BinaryOutput output;
  std::vector<Section *> sections = {&section};
  output.WriteOutput("test_dum_block.bin", sections, symbols);

  // Verify: File should be 0 bytes (DS in DUM block should not emit)
  std::ifstream file("test_dum_block.bin", std::ios::binary);
  ASSERT_TRUE(file.is_open());
  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.close();

  EXPECT_EQ(file_size, 0)
      << "DS inside DUM block should not emit bytes (vasm compatibility)";

  // Verify symbols were created
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("label1", value));
  EXPECT_EQ(value, 0x1000);
  EXPECT_TRUE(symbols.Lookup("label2", value));
  EXPECT_EQ(value, 0x100A); // 0x1000 + 10

  std::remove("test_dum_block.bin");
}

// Test 8: DS outside DUM block SHOULD emit bytes (normal behavior)
TEST(BinaryCompatTest, DsOutsideDumEmitsBytes) {
  std::string source = R"( org $1000
label1 ds 10
)";

  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Write output
  BinaryOutput output;
  std::vector<Section *> sections = {&section};
  output.WriteOutput("test_ds_normal.bin", sections, symbols);

  // Verify: File should contain 10 zero bytes
  std::ifstream file("test_ds_normal.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 10) << "DS outside DUM block should emit bytes";

  std::remove("test_ds_normal.bin");
}

// ============================================================================
// Phase 2: ASC High-Bit Mode (Apple II text format)
// ============================================================================

// Test 9: ASC directive should set high bit on all characters (Apple II
// standard)
TEST(BinaryCompatTest, AscSetsHighBit) {
  // Apple II text uses "high-bit ASCII" where all text characters have bit 7
  // set 'A' = 0x41 → 0xC1, 'B' = 0x42 → 0xC2, etc.

  std::string source = R"( org $1000
 asc "ABC"
)";

  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Write output
  BinaryOutput output;
  std::vector<Section *> sections = {&section};
  output.WriteOutput("test_asc_highbit.bin", sections, symbols);

  // Verify: Characters should have high bit set
  std::ifstream file("test_asc_highbit.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0xC1)
      << "ASCII 'A' (0x41) should become 0xC1 (high-bit set)";
  EXPECT_EQ(result[1], 0xC2)
      << "ASCII 'B' (0x42) should become 0xC2 (high-bit set)";
  EXPECT_EQ(result[2], 0xC3)
      << "ASCII 'C' (0x43) should become 0xC3 (high-bit set)";

  std::remove("test_asc_highbit.bin");
}

// Test 10: ASC with "Prince of Persia" (real-world example from VERSION.S)
TEST(BinaryCompatTest, AscPrinceOfPersiaHighBit) {
  // This is the actual string from VERSION.S that needs to match vasm output
  std::string source = R"( org $1000
 asc "Prince of Persia"
)";

  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Write output
  BinaryOutput output;
  std::vector<Section *> sections = {&section};
  output.WriteOutput("test_asc_pop.bin", sections, symbols);

  // Verify: First few characters match vasm output
  std::ifstream file("test_asc_pop.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_GE(result.size(), 4);
  // 'P' = 0x50 → 0xD0, 'r' = 0x72 → 0xF2, 'i' = 0x69 → 0xE9, 'n' = 0x6E → 0xEE
  EXPECT_EQ(result[0], 0xD0)
      << "Expected 0xD0 (not 0x50) for 'P' with high bit";
  EXPECT_EQ(result[1], 0xF2)
      << "Expected 0xF2 (not 0x72) for 'r' with high bit";
  EXPECT_EQ(result[2], 0xE9)
      << "Expected 0xE9 (not 0x69) for 'i' with high bit";
  EXPECT_EQ(result[3], 0xEE)
      << "Expected 0xEE (not 0x6E) for 'n' with high bit";

  std::remove("test_asc_pop.bin");
}

// ============================================================================
// Forward Reference Tests (Multi-pass assembly)
// ============================================================================

// Test 11: Forward reference in DW directive should resolve correctly
TEST(BinaryCompatTest, ForwardReferenceInDW) {
  // This tests that forward references in data directives are resolved
  // during multi-pass assembly (not left as zeros)

  std::string source = R"(         org $3000
         dw forward_label
forward_label rts
)";

  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Run assembler (multi-pass) to encode instructions and resolve symbols
  Cpu6502 cpu;
  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);  // CRITICAL: Let assembler update symbol table
  assembler.AddSection(section);
  AssemblerResult asm_result = assembler.Assemble();

  if (!asm_result.success) {
    for (const auto &error : asm_result.errors) {
      std::cerr << "Assembly error: " << error.message << std::endl;
    }
  }

  ASSERT_TRUE(asm_result.success) << "Assembly should succeed";

  // Get the assembled sections (with encoded bytes)
  const std::vector<Section>& assembled_sections = assembler.GetSections();
  ASSERT_EQ(assembled_sections.size(), 1) << "Should have one section";

  // Write output using assembled section
  BinaryOutput output;
  std::vector<Section> sections_copy = assembled_sections; // Make mutable copy
  std::vector<Section *> section_ptrs;
  for (auto& s : sections_copy) {
    section_ptrs.push_back(&s);
  }
  output.WriteOutput("test_forward_ref.bin", section_ptrs, symbols);

  // Verify: DW should contain address of forward_label ($3002), not zeros
  std::ifstream file("test_forward_ref.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 3) << "Should have 2 bytes (dw) + 1 byte (rts)";

  // Little-endian: $3002 = 0x02 0x30
  EXPECT_EQ(result[0], 0x02) << "Low byte of $3002 should be 0x02 (not 0x00)";
  EXPECT_EQ(result[1], 0x30) << "High byte of $3002 should be 0x30 (not 0x00)";
  EXPECT_EQ(result[2], 0x60) << "RTS opcode should be 0x60";

  // Verify the symbol was resolved correctly
  int64_t value;
  EXPECT_TRUE(symbols.Lookup("forward_label", value));
  EXPECT_EQ(value, 0x3002) << "forward_label should be at $3002";

  std::remove("test_forward_ref.bin");
}

// Test 12: Multiple forward references in sequence (like SEQTABLE.S)
TEST(BinaryCompatTest, MultipleForwardReferencesInDW) {
  // SEQTABLE.S starts with a table of word pointers to sequence labels
  // All of these are forward references that must be resolved

  std::string source = R"(         org $3000
table    dw seq1,seq2,seq3
seq1     db $01
seq2     db $02
seq3     db $03
)";

  MerlinSyntaxParser parser;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  try {
    parser.Parse(source, section, symbols);
  } catch (const std::exception &e) {
    FAIL() << "Parse error: " << e.what();
  }

  // Run assembler (multi-pass) to encode instructions and resolve symbols
  Cpu6502 cpu;
  Assembler assembler;
  assembler.SetCpuPlugin(&cpu);
  assembler.SetSymbolTable(&symbols);  // CRITICAL: Let assembler update symbol table
  assembler.AddSection(section);
  AssemblerResult asm_result = assembler.Assemble();

  ASSERT_TRUE(asm_result.success) << "Assembly should succeed";

  // Get the assembled sections (with encoded bytes)
  const std::vector<Section>& assembled_sections = assembler.GetSections();
  ASSERT_EQ(assembled_sections.size(), 1) << "Should have one section";

  // Write output using assembled section
  BinaryOutput output;
  std::vector<Section> sections_copy = assembled_sections; // Make mutable copy
  std::vector<Section *> section_ptrs;
  for (auto& s : sections_copy) {
    section_ptrs.push_back(&s);
  }
  output.WriteOutput("test_multi_forward_ref.bin", section_ptrs, symbols);

  // Verify: Table should contain correct addresses (not zeros)
  std::ifstream file("test_multi_forward_ref.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 9)
      << "Should have 6 bytes (3 words) + 3 bytes (data)";

  // seq1 is at $3006 (after 3 words = 6 bytes)
  EXPECT_EQ(result[0], 0x06) << "seq1 low byte should be 0x06";
  EXPECT_EQ(result[1], 0x30) << "seq1 high byte should be 0x30";

  // seq2 is at $3007 (seq1 + 1 byte)
  EXPECT_EQ(result[2], 0x07) << "seq2 low byte should be 0x07";
  EXPECT_EQ(result[3], 0x30) << "seq2 high byte should be 0x30";

  // seq3 is at $3008 (seq2 + 1 byte)
  EXPECT_EQ(result[4], 0x08) << "seq3 low byte should be 0x08";
  EXPECT_EQ(result[5], 0x30) << "seq3 high byte should be 0x30";

  // Data bytes
  EXPECT_EQ(result[6], 0x01) << "seq1 data";
  EXPECT_EQ(result[7], 0x02) << "seq2 data";
  EXPECT_EQ(result[8], 0x03) << "seq3 data";

  std::remove("test_multi_forward_ref.bin");
}
