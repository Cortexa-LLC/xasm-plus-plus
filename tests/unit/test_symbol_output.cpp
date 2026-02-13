// SymbolOutput Plugin tests
// Phase 3.3: Output Format Plugins - Symbol Table Output

#include "xasm++/expression.h"
#include "xasm++/output/symbol_output.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

using namespace xasm;

// Test 1: Empty symbol table produces minimal file
TEST(SymbolOutputTest, EmptySymbolTable) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};
  ConcreteSymbolTable symbols;

  output.WriteOutput("test.sym", sections, symbols);

  // Verify file exists
  std::ifstream file("test.sym");
  ASSERT_TRUE(file.is_open());

  // Should have header but no symbols
  std::string line;
  std::getline(file, line);
  EXPECT_FALSE(line.empty()); // Header exists
  file.close();

  // Cleanup
  std::remove("test.sym");
}

// Test 2: Single symbol in table
TEST(SymbolOutputTest, SingleSymbol) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};

  ConcreteSymbolTable symbols;
  symbols.Define("start", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));

  output.WriteOutput("test.sym", {&section}, symbols);

  std::ifstream file("test.sym");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain symbol name and value
  EXPECT_NE(content.find("start"), std::string::npos);
  EXPECT_NE(content.find("8000"), std::string::npos);

  std::remove("test.sym");
}

// Test 3: Multiple symbols - alphabetically sorted
TEST(SymbolOutputTest, MultipleSymbolsSorted) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};

  ConcreteSymbolTable symbols;
  symbols.Define("zebra", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x9000));
  symbols.Define("apple", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));
  symbols.Define("middle", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8500));

  output.WriteOutput("test.sym", {&section}, symbols);

  std::ifstream file("test.sym");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Find positions of each symbol
  size_t apple_pos = content.find("apple");
  size_t middle_pos = content.find("middle");
  size_t zebra_pos = content.find("zebra");

  // Verify alphabetical order
  EXPECT_LT(apple_pos, middle_pos);
  EXPECT_LT(middle_pos, zebra_pos);

  std::remove("test.sym");
}

// Test 4: Different symbol types
TEST(SymbolOutputTest, DifferentSymbolTypes) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};

  ConcreteSymbolTable symbols;
  symbols.Define("label1", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0x8000));
  symbols.Define("CONST1", SymbolType::Equate,
                 std::make_shared<LiteralExpr>(42));
  symbols.Define("var1", SymbolType::Set, std::make_shared<LiteralExpr>(100));

  output.WriteOutput("test.sym", {&section}, symbols);

  std::ifstream file("test.sym");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // All symbols should appear
  EXPECT_NE(content.find("label1"), std::string::npos);
  EXPECT_NE(content.find("CONST1"), std::string::npos);
  EXPECT_NE(content.find("var1"), std::string::npos);

  std::remove("test.sym");
}

// Test 5: GetName returns correct name
TEST(SymbolOutputTest, PluginName) {
  SymbolOutput output;
  EXPECT_EQ(output.GetName(), "symbol");
}

// Test 6: GetFileExtension returns correct extension
TEST(SymbolOutputTest, FileExtension) {
  SymbolOutput output;
  EXPECT_EQ(output.GetFileExtension(), ".sym");
}

// Test 7: Hex values formatted correctly
TEST(SymbolOutputTest, HexValueFormat) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};

  ConcreteSymbolTable symbols;
  symbols.Define("addr", SymbolType::Label,
                 std::make_shared<LiteralExpr>(0xABCD));

  output.WriteOutput("test.sym", {&section}, symbols);

  std::ifstream file("test.sym");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // Should contain hex value (case may vary)
  EXPECT_TRUE(content.find("ABCD") != std::string::npos ||
              content.find("abcd") != std::string::npos);

  std::remove("test.sym");
}

// Test 8: Large symbol table
TEST(SymbolOutputTest, LargeSymbolTable) {
  SymbolOutput output;
  Section section("test", 0);
  std::vector<Section *> sections = {&section};

  ConcreteSymbolTable symbols;
  for (int i = 0; i < 100; ++i) {
    std::string name = "sym" + std::to_string(i);
    symbols.Define(name, SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000 + i));
  }

  output.WriteOutput("test.sym", {&section}, symbols);

  std::ifstream file("test.sym");
  ASSERT_TRUE(file.is_open());

  // Count lines
  unsigned int line_count = 0;
  std::string line;
  while (std::getline(file, line)) {
    ++line_count;
  }
  file.close();

  // Should have at least 100 symbols
  EXPECT_GE(line_count, 100U);

  std::remove("test.sym");
}
