// Symbol tests
// Phase 1: Minimal Viable Assembler - Core Abstractions

#include "xasm++/expression.h"
#include "xasm++/symbol.h"
#include <gtest/gtest.h>

using namespace xasm;

// Test 1: SymbolType enum
TEST(SymbolTest, SymbolTypeEnum) {
  // Just verify the enum values exist
  SymbolType t1 = SymbolType::Label;
  SymbolType t2 = SymbolType::Equate;
  SymbolType t3 = SymbolType::Set;

  EXPECT_NE(t1, t2);
  EXPECT_NE(t2, t3);
}

// Test 2: Symbol creation with label
TEST(SymbolTest, SymbolLabelCreation) {
  auto value = std::make_shared<LiteralExpr>(0x1000);
  Symbol sym("start", SymbolType::Label, value);
  ConcreteSymbolTable dummy_table; // Need a valid table reference

  EXPECT_EQ(sym.name, "start");
  EXPECT_EQ(sym.type, SymbolType::Label);
  EXPECT_NE(sym.value, nullptr);
  EXPECT_EQ(sym.value->Evaluate(dummy_table), 0x1000);
  EXPECT_FALSE(sym.is_exported);
  EXPECT_FALSE(sym.is_imported);
}

// Test 3: Symbol creation with equate
TEST(SymbolTest, SymbolEquateCreation) {
  auto value = std::make_shared<LiteralExpr>(42);
  Symbol sym("CONSTANT", SymbolType::Equate, value);
  ConcreteSymbolTable dummy_table; // Need a valid table reference

  EXPECT_EQ(sym.name, "CONSTANT");
  EXPECT_EQ(sym.type, SymbolType::Equate);
  EXPECT_EQ(sym.value->Evaluate(dummy_table), 42);
}

// Test 4: Symbol with export flag
TEST(SymbolTest, SymbolExportFlag) {
  auto value = std::make_shared<LiteralExpr>(0x2000);
  Symbol sym("exported_label", SymbolType::Label, value);
  sym.is_exported = true;

  EXPECT_TRUE(sym.is_exported);
  EXPECT_FALSE(sym.is_imported);
}

// Test 5: SymbolTable creation
TEST(SymbolTest, SymbolTableCreation) {
  ConcreteSymbolTable table;

  // Table should be empty initially
  int64_t value;
  EXPECT_FALSE(table.Lookup("undefined", value));
}

// Test 6: SymbolTable define and lookup
TEST(SymbolTest, SymbolTableDefineAndLookup) {
  ConcreteSymbolTable table;

  auto expr = std::make_shared<LiteralExpr>(0x1000);
  table.Define("label1", SymbolType::Label, expr);

  int64_t value;
  EXPECT_TRUE(table.Lookup("label1", value));
  EXPECT_EQ(value, 0x1000);
}

// Test 7: SymbolTable multiple symbols
TEST(SymbolTest, SymbolTableMultipleSymbols) {
  ConcreteSymbolTable table;

  table.Define("label1", SymbolType::Label,
               std::make_shared<LiteralExpr>(0x1000));
  table.Define("label2", SymbolType::Label,
               std::make_shared<LiteralExpr>(0x2000));
  table.Define("CONST", SymbolType::Equate, std::make_shared<LiteralExpr>(42));

  int64_t val1, val2, val3;
  EXPECT_TRUE(table.Lookup("label1", val1));
  EXPECT_TRUE(table.Lookup("label2", val2));
  EXPECT_TRUE(table.Lookup("CONST", val3));

  EXPECT_EQ(val1, 0x1000);
  EXPECT_EQ(val2, 0x2000);
  EXPECT_EQ(val3, 42);
}

// Test 8: SymbolTable undefined lookup
TEST(SymbolTest, SymbolTableUndefinedLookup) {
  ConcreteSymbolTable table;

  table.Define("label1", SymbolType::Label,
               std::make_shared<LiteralExpr>(0x1000));

  int64_t value;
  EXPECT_FALSE(table.Lookup("undefined", value));
}

// Test 9: SymbolTable redefinition (should replace)
TEST(SymbolTest, SymbolTableRedefinition) {
  ConcreteSymbolTable table;

  table.Define("var", SymbolType::Set, std::make_shared<LiteralExpr>(10));
  table.Define("var", SymbolType::Set, std::make_shared<LiteralExpr>(20));

  int64_t value;
  EXPECT_TRUE(table.Lookup("var", value));
  EXPECT_EQ(value, 20); // Should use the new value
}

// Test 10: SymbolTable GetSymbol
TEST(SymbolTest, SymbolTableGetSymbol) {
  ConcreteSymbolTable table;

  auto expr = std::make_shared<LiteralExpr>(0x1000);
  table.Define("label1", SymbolType::Label, expr);

  const Symbol *sym = table.GetSymbol("label1");
  ASSERT_NE(sym, nullptr);
  EXPECT_EQ(sym->name, "label1");
  EXPECT_EQ(sym->type, SymbolType::Label);
  EXPECT_EQ(sym->value->Evaluate(table), 0x1000);
}

// Test 11: SymbolTable GetSymbol undefined
TEST(SymbolTest, SymbolTableGetSymbolUndefined) {
  ConcreteSymbolTable table;

  const Symbol *sym = table.GetSymbol("undefined");
  EXPECT_EQ(sym, nullptr);
}

// Test 12: SymbolTable iteration
TEST(SymbolTest, SymbolTableIteration) {
  ConcreteSymbolTable table;

  table.Define("label1", SymbolType::Label,
               std::make_shared<LiteralExpr>(0x1000));
  table.Define("label2", SymbolType::Label,
               std::make_shared<LiteralExpr>(0x2000));
  table.Define("CONST", SymbolType::Equate, std::make_shared<LiteralExpr>(42));

  auto symbols = table.GetAllSymbols();
  EXPECT_EQ(symbols.size(), 3UL);

  // Verify all symbols are present
  bool found_label1 = false, found_label2 = false, found_const = false;
  for (const auto &[name, sym] : symbols) {
    if (name == "label1" && sym.type == SymbolType::Label)
      found_label1 = true;
    if (name == "label2" && sym.type == SymbolType::Label)
      found_label2 = true;
    if (name == "CONST" && sym.type == SymbolType::Equate)
      found_const = true;
  }

  EXPECT_TRUE(found_label1);
  EXPECT_TRUE(found_label2);
  EXPECT_TRUE(found_const);
}
