// ListingOutput Plugin tests
// Phase 3.3: Output Format Plugins - Listing Output

#include "xasm++/output/listing_output.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

// Test 1: Empty section produces minimal listing
TEST(ListingOutputTest, EmptySection) {
  ListingOutput output;
  Section section("test", 0, 0); // name, attributes, origin
  std::vector<Section *> sections = {&section};
  ConcreteSymbolTable symbols;

  output.WriteOutput("test.lst", sections, symbols);

  // Verify file exists
  std::ifstream file("test.lst");
  ASSERT_TRUE(file.is_open());

  // Should have header but no content
  std::string line;
  std::getline(file, line);
  EXPECT_FALSE(line.empty()); // Header exists
  file.close();

  // Cleanup
  std::remove("test.lst");
}

// Test 2: Simple data atom listing
TEST(ListingOutputTest, DataAtomListing) {
  ListingOutput output;
  Section section("test", 0, 0x8000); // name, attributes, origin
  std::vector<uint8_t> data = {0x01, 0x02, 0x03};
  section.atoms.push_back(std::make_shared<DataAtom>(data));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section}, symbols);

  // Verify listing contains address and bytes
  std::ifstream file("test.lst");
  ASSERT_TRUE(file.is_open());

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain hex address
  EXPECT_NE(content.find("8000"), std::string::npos);
  // Should contain hex bytes
  EXPECT_NE(content.find("01"), std::string::npos);
  EXPECT_NE(content.find("02"), std::string::npos);
  EXPECT_NE(content.find("03"), std::string::npos);

  std::remove("test.lst");
}

// Test 3: Instruction listing format
TEST(ListingOutputTest, InstructionListing) {
  ListingOutput output;
  Section section("test", 0, 0x8000); // name, attributes, origin

  auto inst = std::make_shared<InstructionAtom>("LDA", "#$42");
  inst->encoded_bytes = {0xA9, 0x42};
  inst->size = 2;
  section.atoms.push_back(inst);

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section}, symbols);

  std::ifstream file("test.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should show address, bytes, and mnemonic
  EXPECT_NE(content.find("8000"), std::string::npos);
  EXPECT_NE(content.find("A9 42"), std::string::npos);
  EXPECT_NE(content.find("LDA"), std::string::npos);

  std::remove("test.lst");
}

// Test 4: Label appears in listing
TEST(ListingOutputTest, LabelListing) {
  ListingOutput output;
  Section section("test", 0, 0x8000); // name, attributes, origin

  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xFF}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section}, symbols);

  std::ifstream file("test.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Label should appear
  EXPECT_NE(content.find("start"), std::string::npos);

  std::remove("test.lst");
}

// Test 5: GetName returns correct name
TEST(ListingOutputTest, PluginName) {
  ListingOutput output;
  EXPECT_EQ(output.GetName(), "listing");
}

// Test 6: GetFileExtension returns correct extension
TEST(ListingOutputTest, FileExtension) {
  ListingOutput output;
  EXPECT_EQ(output.GetFileExtension(), ".lst");
}

// Test 7: Multiple sections in listing
TEST(ListingOutputTest, MultipleSections) {
  ListingOutput output;

  Section section1("code", 0, 0x8000); // name, attributes, origin
  section1.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));

  Section section2("data", 0, 0x9000); // name, attributes, origin
  section2.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xBB}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section1, &section2}, symbols);

  std::ifstream file("test.lst");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Both sections should appear
  EXPECT_NE(content.find("8000"), std::string::npos);
  EXPECT_NE(content.find("9000"), std::string::npos);

  std::remove("test.lst");
}

// Test 8: Complete program listing
TEST(ListingOutputTest, CompleteProgram) {
  ListingOutput output;
  Section section("test", 0, 0x8000); // name, attributes, origin

  section.atoms.push_back(std::make_shared<OrgAtom>(0x8000));
  section.atoms.push_back(std::make_shared<LabelAtom>("start", 0x8000));

  auto lda = std::make_shared<InstructionAtom>("LDA", "#$42");
  lda->encoded_bytes = {0xA9, 0x42};
  lda->size = 2;
  section.atoms.push_back(lda);

  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0x01, 0x02}));

  ConcreteSymbolTable symbols;
  output.WriteOutput("test.lst", {&section}, symbols);

  std::ifstream file("test.lst");
  ASSERT_TRUE(file.is_open());
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Verify all elements present
  EXPECT_NE(content.find("start"), std::string::npos);
  EXPECT_NE(content.find("LDA"), std::string::npos);
  EXPECT_NE(content.find("8000"), std::string::npos);

  std::remove("test.lst");
}
