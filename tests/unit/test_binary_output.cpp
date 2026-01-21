// BinaryOutput Plugin tests
// Phase 1: Minimal Viable Assembler - Binary Output Plugin

#include "xasm++/output/binary_output.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>
#include <fstream>
#include <vector>

using namespace xasm;

// Test 1: Empty section produces empty file
TEST(BinaryOutputTest, EmptySection) {
  BinaryOutput output;
  Section section("test", 0);
  std::vector<Section*> sections = {&section};
  ConcreteSymbolTable symbols;

  output.WriteOutput("test.bin", sections, symbols);

  // Verify file exists and is empty
  std::ifstream file("test.bin", std::ios::binary);
  ASSERT_TRUE(file.is_open());
  file.seekg(0, std::ios::end);
  EXPECT_EQ(file.tellg(), 0);
  file.close();

  // Cleanup
  std::remove("test.bin");
}

// Test 2: DataAtom serialization
TEST(BinaryOutputTest, DataAtomSerialization) {
  BinaryOutput output;
  Section section("test", 0);
  std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
  section.atoms.push_back(std::make_shared<DataAtom>(data));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // Verify binary content
  std::ifstream file("test.bin", std::ios::binary);
  ASSERT_TRUE(file.is_open());
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x02);
  EXPECT_EQ(result[2], 0x03);
  EXPECT_EQ(result[3], 0x04);

  // Cleanup
  std::remove("test.bin");
}

// Test 3: OrgAtom handling (doesn't write bytes)
TEST(BinaryOutputTest, OrgDirective) {
  BinaryOutput output;
  Section section("test", 0);
  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // ORG doesn't write bytes, just the data after it
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0xAA);

  std::remove("test.bin");
}

// Test 4: LabelAtom handling (doesn't write bytes)
TEST(BinaryOutputTest, LabelHandling) {
  BinaryOutput output;
  Section section("test", 0);
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xFF}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // Label produces no bytes
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0xFF);

  std::remove("test.bin");
}

// Test 5: SpaceAtom serialization (writes zeros)
TEST(BinaryOutputTest, SpaceAtomSerialization) {
  BinaryOutput output;
  Section section("test", 0);
  section.atoms.push_back(std::make_shared<SpaceAtom>(4));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // Space writes zeros
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x00);
  EXPECT_EQ(result[2], 0x00);
  EXPECT_EQ(result[3], 0x00);

  std::remove("test.bin");
}

// Test 6: AlignAtom handling (writes padding)
TEST(BinaryOutputTest, AlignAtomHandling) {
  BinaryOutput output;
  Section section("test", 0);
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0x01}));
  section.atoms.push_back(std::make_shared<AlignAtom>(4));
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0x02}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // 1 byte data + 3 bytes padding + 1 byte data = 5 bytes
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 5);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x00);  // Padding
  EXPECT_EQ(result[2], 0x00);
  EXPECT_EQ(result[3], 0x00);
  EXPECT_EQ(result[4], 0x02);

  std::remove("test.bin");
}

// Test 7: Multiple sections
TEST(BinaryOutputTest, MultipleSections) {
  BinaryOutput output;
  Section section1("code", 0);
  section1.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));

  Section section2("data", 0);
  section2.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xBB}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section1, &section2}, symbols);

  // Sections written sequentially
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0xAA);
  EXPECT_EQ(result[1], 0xBB);

  std::remove("test.bin");
}

// Test 8: InstructionAtom serialization
TEST(BinaryOutputTest, InstructionSerialization) {
  BinaryOutput output;
  Section section("test", 0);

  // Create instruction atom with encoded bytes
  auto inst = std::make_shared<InstructionAtom>("LDA", "#$42");
  inst->encoded_bytes = {0xA9, 0x42};  // LDA immediate
  inst->size = 2;
  section.atoms.push_back(inst);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0xA9);
  EXPECT_EQ(result[1], 0x42);

  std::remove("test.bin");
}

// Test 9: Error handling - invalid path
TEST(BinaryOutputTest, FileWriteError) {
  BinaryOutput output;
  Section section("test", 0);
  ConcreteSymbolTable symbols;

  // Try to write to invalid path
  EXPECT_THROW(
    output.WriteOutput("/invalid/nonexistent/path/test.bin", {&section}, symbols),
    std::runtime_error
  );
}

// Test 10: Complete program (all atom types)
TEST(BinaryOutputTest, CompleteProgram) {
  BinaryOutput output;
  Section section("test", 0);

  // OrgAtom
  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));

  // LabelAtom
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));

  // InstructionAtom
  auto lda = std::make_shared<InstructionAtom>("LDA", "#$42");
  lda->encoded_bytes = {0xA9, 0x42};
  lda->size = 2;
  section.atoms.push_back(lda);

  // DataAtom
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0x01, 0x02}));

  // SpaceAtom
  section.atoms.push_back(std::make_shared<SpaceAtom>(2));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.bin", {&section}, symbols);

  // Expected: A9 42 01 02 00 00
  std::ifstream file("test.bin", std::ios::binary);
  std::vector<uint8_t> result((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
  file.close();

  ASSERT_EQ(result.size(), 6);
  EXPECT_EQ(result[0], 0xA9);  // LDA opcode
  EXPECT_EQ(result[1], 0x42);  // LDA operand
  EXPECT_EQ(result[2], 0x01);  // Data
  EXPECT_EQ(result[3], 0x02);  // Data
  EXPECT_EQ(result[4], 0x00);  // Space
  EXPECT_EQ(result[5], 0x00);  // Space

  std::remove("test.bin");
}
