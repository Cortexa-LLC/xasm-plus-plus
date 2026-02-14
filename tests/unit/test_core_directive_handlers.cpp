/**
 * @file test_core_directive_handlers.cpp
 * @brief Unit tests for core directive handlers (ORG, EQU, DB, DW, DS)
 *
 * Tests extraction of common directive handlers into free functions
 * as part of God-Class Phase 6b refactoring.
 */

#include "xasm++/atom.h"
#include "xasm++/directives/directive_constants.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/core_directive_handlers.h"
#include "xasm++/syntax/directive_registry.h"
#include <gtest/gtest.h>
#include <memory>

using namespace xasm;

/**
 * @brief Test fixture for core directive handler tests
 */
class CoreDirectiveHandlersTest : public ::testing::Test {
protected:
  void SetUp() override {
    symbols_ = std::make_unique<ConcreteSymbolTable>();
    section_ = std::make_unique<Section>("test_section", 0x01);
    current_address_ = 0x1000;
    
    // Set up DirectiveContext
    context_.section = section_.get();
    context_.symbols = symbols_.get();
    context_.current_address = &current_address_;
  }

  std::unique_ptr<ConcreteSymbolTable> symbols_;
  std::unique_ptr<Section> section_;
  uint32_t current_address_;
  DirectiveContext context_;
};

// ============================================================================
// ORG Directive Tests
// ============================================================================

/**
 * @test ORG creates OrgAtom with correct decimal address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithDecimalAddress) {
  // Act
  HandleOrg("", "2048", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 2048U);
  EXPECT_EQ(current_address_, 2048U);
}

/**
 * @test ORG creates OrgAtom with correct hex address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithHexAddress) {
  // Act
  HandleOrg("", "$C000", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0xC000U);
  EXPECT_EQ(current_address_, 0xC000U);
}

/**
 * @test ORG creates OrgAtom with correct binary address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithBinaryAddress) {
  // Act
  HandleOrg("", "%11111111", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 255U);
  EXPECT_EQ(current_address_, 255U);
}

/**
 * @test ORG resolves symbol address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSymbolAddress) {
  // Arrange
  symbols_->Define("START", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000));

  // Act
  HandleOrg("", "START", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000U);
  EXPECT_EQ(current_address_, 0x8000U);
}

/**
 * @test ORG throws on empty operand
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithEmptyOperandThrows) {
  // Act & Assert
  EXPECT_THROW(HandleOrg("", "", context_),
               std::runtime_error);
}

/**
 * @test ORG handles whitespace in operand
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithWhitespace) {
  // Act
  HandleOrg("", "  $1000  ", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1000U);
}

// ============================================================================
// EQU Directive Tests
// ============================================================================

/**
 * @test EQU defines symbol with literal value
 */
TEST_F(CoreDirectiveHandlersTest, EquWithLiteralValue) {
  // Act
  HandleEqu("BUFFER_SIZE", "256", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("BUFFER_SIZE", value));
  EXPECT_EQ(value, 256);
}

/**
 * @test EQU defines symbol with hex value
 */
TEST_F(CoreDirectiveHandlersTest, EquWithHexValue) {
  // Act
  HandleEqu("IO_PORT", "$D000", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("IO_PORT", value));
  EXPECT_EQ(value, 0xD000);
}

/**
 * @test EQU defines symbol with expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithExpression) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x2000));

  // Act
  HandleEqu("OFFSET", "BASE+$100", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("OFFSET", value));
  EXPECT_EQ(value, 0x2100);
}

/**
 * @test EQU throws on empty label
 */
TEST_F(CoreDirectiveHandlersTest, EquWithEmptyLabelThrows) {
  // Act & Assert
  EXPECT_THROW(HandleEqu("", "100", context_), std::runtime_error);
}

/**
 * @test EQU does not create atoms
 */
TEST_F(CoreDirectiveHandlersTest, EquDoesNotCreateAtoms) {
  // Act
  HandleEqu("CONSTANT", "42", context_);

  // Assert
  EXPECT_EQ(section_->atoms.size(), 0UL);
}

// ============================================================================
// DB Directive Tests
// ============================================================================

/**
 * @test DB creates DataAtom with single byte
 */
TEST_F(CoreDirectiveHandlersTest, DbWithSingleByte) {
  // Act
  HandleDb("", "42", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Byte);
  ASSERT_EQ(data_atom->expressions.size(), 1UL);
  EXPECT_EQ(data_atom->expressions[0], "42");
  EXPECT_EQ(current_address_, 0x1001U); // Advanced by 1 byte
}

/**
 * @test DB creates DataAtom with multiple bytes
 */
TEST_F(CoreDirectiveHandlersTest, DbWithMultipleBytes) {
  // Act
  HandleDb("", "1,2,3,4,5", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Byte);
  ASSERT_EQ(data_atom->expressions.size(), 5UL);
  EXPECT_EQ(data_atom->expressions[0], "1");
  EXPECT_EQ(data_atom->expressions[4], "5");
  EXPECT_EQ(current_address_, 0x1005U); // Advanced by 5 bytes
}

/**
 * @test DB handles expressions
 */
TEST_F(CoreDirectiveHandlersTest, DbWithExpressions) {
  // Act
  HandleDb("", "$FF,1+2,%00001111", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(data_atom->expressions[0], "$FF");
  EXPECT_EQ(data_atom->expressions[1], "1+2");
  EXPECT_EQ(data_atom->expressions[2], "%00001111");
}

/**
 * @test DB handles whitespace in operands
 */
TEST_F(CoreDirectiveHandlersTest, DbWithWhitespace) {
  // Act
  HandleDb("", "  1 ,  2  , 3  ", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(data_atom->expressions[0], "1");
  EXPECT_EQ(data_atom->expressions[1], "2");
  EXPECT_EQ(data_atom->expressions[2], "3");
}

/**
 * @test DB handles empty operand
 */
TEST_F(CoreDirectiveHandlersTest, DbWithEmptyOperand) {
  // Act
  HandleDb("", "", context_);

  // Assert - should create empty DataAtom
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->expressions.size(), 0UL);
  EXPECT_EQ(current_address_, 0x1000U); // Not advanced
}

// ============================================================================
// DW Directive Tests
// ============================================================================

/**
 * @test DW creates DataAtom with word size
 */
TEST_F(CoreDirectiveHandlersTest, DwWithSingleWord) {
  // Act
  HandleDw("", "$1234", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  ASSERT_EQ(data_atom->expressions.size(), 1UL);
  EXPECT_EQ(data_atom->expressions[0], "$1234");
  EXPECT_EQ(current_address_, 0x1002U); // Advanced by 2 bytes
}

/**
 * @test DW creates DataAtom with multiple words
 */
TEST_F(CoreDirectiveHandlersTest, DwWithMultipleWords) {
  // Act
  HandleDw("", "$1000,$2000,$3000", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  ASSERT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(current_address_, 0x1006U); // Advanced by 6 bytes (3 words)
}

/**
 * @test DW handles whitespace
 */
TEST_F(CoreDirectiveHandlersTest, DwWithWhitespace) {
  // Act
  HandleDw("", "  $1000  ,  $2000  ", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->expressions.size(), 2UL);
  EXPECT_EQ(data_atom->expressions[0], "$1000");
  EXPECT_EQ(data_atom->expressions[1], "$2000");
}

// ============================================================================
// DS Directive Tests
// ============================================================================

/**
 * @test DS creates SpaceAtom with correct count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithLiteralCount) {
  // Act
  HandleDs("", "100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U); // Advanced by 100 bytes
}

/**
 * @test DS handles hex count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithHexCount) {
  // Act
  HandleDs("", "$100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 256UL);
  EXPECT_EQ(current_address_, 0x1100U); // Advanced by 256 bytes
}

/**
 * @test DS handles expression with symbol
 */
TEST_F(CoreDirectiveHandlersTest, DsWithExpression) {
  // Arrange
  symbols_->Define("SIZE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(50));

  // Act
  HandleDs("", "SIZE*2", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U); // Advanced by 100 bytes
}

/**
 * @test DS throws on negative count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithNegativeCountThrows) {
  // Act & Assert
  EXPECT_THROW(HandleDs("", "-10", context_),
               std::runtime_error);
}

/**
 * @test DS handles whitespace
 */
TEST_F(CoreDirectiveHandlersTest, DsWithWhitespace) {
  // Act
  HandleDs("", "  50  ", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 50UL);
}

/**
 * @test DS with empty operand creates zero-size SpaceAtom
 */
TEST_F(CoreDirectiveHandlersTest, DsWithEmptyOperand) {
  // Act
  HandleDs("", "", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 0UL);
  EXPECT_EQ(current_address_, 0x1000U); // Not advanced
}

// ============================================================================
// DirectiveRegistry Integration Tests
// ============================================================================

/**
 * @brief Test fixture for DirectiveRegistry integration tests
 */
class CoreDirectiveRegistryTest : public ::testing::Test {
protected:
  void SetUp() override {
    symbols_ = std::make_unique<ConcreteSymbolTable>();
    section_ = std::make_unique<Section>("test_section", 0x01);
    current_address_ = 0x1000;

    // Set up DirectiveContext
    context_.section = section_.get();
    context_.symbols = symbols_.get();
    context_.current_address = &current_address_;

    // Register handlers
    RegisterCoreDirectiveHandlers(registry_);
  }

  std::unique_ptr<ConcreteSymbolTable> symbols_;
  std::unique_ptr<Section> section_;
  uint32_t current_address_;
  DirectiveContext context_;
  DirectiveRegistry registry_;
};

/**
 * @test Registry ORG directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryOrgWorks) {
  // Act
  registry_.Execute(directives::ORG, "", "$C000", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0xC000U);
  EXPECT_EQ(current_address_, 0xC000U);
}

/**
 * @test Registry EQU directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryEquWorks) {
  // Act
  registry_.Execute(directives::EQU, "TEST_VALUE", "42", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("TEST_VALUE", value));
  EXPECT_EQ(value, 42);
}

/**
 * @test Registry DB directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDbWorks) {
  // Act
  registry_.Execute(directives::DB, "", "1,2,3", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Byte);
  EXPECT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(current_address_, 0x1003U);
}

/**
 * @test Registry DW directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDwWorks) {
  // Act
  registry_.Execute(directives::DW, "", "$1000,$2000", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  EXPECT_EQ(data_atom->expressions.size(), 2UL);
  EXPECT_EQ(current_address_, 0x1004U);
}

/**
 * @test Registry DS directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDsWorks) {
  // Act
  registry_.Execute(directives::DS, "", "100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U);
}

/**
 * @test Registry recognizes DB aliases (DEFB, BYTE)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDbAliases) {
  // Test DEFB
  registry_.Execute(directives::DEFB, "", "42", context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test BYTE
  registry_.Execute(directives::BYTE, "", "43", context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);
}

/**
 * @test Registry recognizes DW aliases (DEFW, WORD)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDwAliases) {
  // Test DEFW
  registry_.Execute(directives::DEFW, "", "$1234", context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test WORD
  registry_.Execute(directives::WORD, "", "$5678", context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);
}

/**
 * @test Registry recognizes DS aliases (DEFS, BLOCK, RMB)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDsAliases) {
  // Test DEFS
  registry_.Execute(directives::DEFS, "", "10", context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test BLOCK
  registry_.Execute(directives::BLOCK, "", "20", context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);

  // Test RMB
  registry_.Execute(directives::RMB, "", "30", context_);
  ASSERT_EQ(section_->atoms.size(), 3UL);
}

/**
 * @test Registry IsRegistered works for all directives
 */
TEST_F(CoreDirectiveRegistryTest, RegistryIsRegistered) {
  EXPECT_TRUE(registry_.IsRegistered(directives::ORG));
  EXPECT_TRUE(registry_.IsRegistered(directives::EQU));
  EXPECT_TRUE(registry_.IsRegistered(directives::DB));
  EXPECT_TRUE(registry_.IsRegistered(directives::DEFB));
  EXPECT_TRUE(registry_.IsRegistered(directives::BYTE));
  EXPECT_TRUE(registry_.IsRegistered(directives::DW));
  EXPECT_TRUE(registry_.IsRegistered(directives::DEFW));
  EXPECT_TRUE(registry_.IsRegistered(directives::WORD));
  EXPECT_TRUE(registry_.IsRegistered(directives::DS));
  EXPECT_TRUE(registry_.IsRegistered(directives::DEFS));
  EXPECT_TRUE(registry_.IsRegistered(directives::BLOCK));
  EXPECT_TRUE(registry_.IsRegistered(directives::RMB));
  EXPECT_FALSE(registry_.IsRegistered("UNKNOWN"));
}

// ============================================================================
// Expression Arithmetic Tests
// ============================================================================

/**
 * @test ORG with addition expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithAdditionExpression) {
  // Act
  HandleOrg("", "$1000+$100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1100U);
  EXPECT_EQ(current_address_, 0x1100U);
}

/**
 * @test ORG with subtraction expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSubtractionExpression) {
  // Act
  HandleOrg("", "$2000-$100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1F00U);
  EXPECT_EQ(current_address_, 0x1F00U);
}

/**
 * @test ORG with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithMultiplicationExpression) {
  // Act
  HandleOrg("", "$100*16", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1000U);
  EXPECT_EQ(current_address_, 0x1000U);
}

/**
 * @test ORG with division expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithDivisionExpression) {
  // Act
  HandleOrg("", "$4000/2", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x2000U);
  EXPECT_EQ(current_address_, 0x2000U);
}

/**
 * @test ORG with modulo expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithModuloExpression) {
  // Act
  HandleOrg("", "1000%256", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 232U);
  EXPECT_EQ(current_address_, 232U);
}

/**
 * @test ORG with nested expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithNestedExpression) {
  // Act
  HandleOrg("", "($1000+$100)*2", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x2200U);
  EXPECT_EQ(current_address_, 0x2200U);
}

/**
 * @test ORG with complex nested expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithComplexNestedExpression) {
  // Act
  HandleOrg("", "(100+50)*2-20", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 280U); // (100+50)*2-20 = 150*2-20 = 300-20 = 280
  EXPECT_EQ(current_address_, 280U);
}

/**
 * @test ORG with symbol in arithmetic expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSymbolArithmetic) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000));

  // Act
  HandleOrg("", "BASE+$100", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8100U);
  EXPECT_EQ(current_address_, 0x8100U);
}

/**
 * @test EQU with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithMultiplicationExpression) {
  // Act
  HandleEqu("SIZE", "64*1024", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("SIZE", value));
  EXPECT_EQ(value, 65536);
}

/**
 * @test EQU with complex expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithComplexExpression) {
  // Arrange
  symbols_->Define("WIDTH", SymbolType::Label,
                   std::make_shared<LiteralExpr>(40));
  symbols_->Define("HEIGHT", SymbolType::Label,
                   std::make_shared<LiteralExpr>(25));

  // Act
  HandleEqu("SCREEN_SIZE", "WIDTH*HEIGHT", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("SCREEN_SIZE", value));
  EXPECT_EQ(value, 1000);
}

/**
 * @test EQU with nested expression and symbols
 */
TEST_F(CoreDirectiveHandlersTest, EquWithNestedExpressionAndSymbols) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x1000));
  symbols_->Define("OFFSET", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x10));

  // Act
  HandleEqu("ADDR", "(BASE+OFFSET)*2", context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("ADDR", value));
  EXPECT_EQ(value, 0x2020);
}

/**
 * @test DS with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, DsWithMultiplicationExpression) {
  // Act
  HandleDs("", "10*16", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 160UL);
  EXPECT_EQ(current_address_, 0x1000U + 160U);
}

/**
 * @test DS with symbol and arithmetic
 */
TEST_F(CoreDirectiveHandlersTest, DsWithSymbolAndArithmetic) {
  // Arrange
  symbols_->Define("PAGE_SIZE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(256));

  // Act
  HandleDs("", "PAGE_SIZE*4", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 1024UL);
  EXPECT_EQ(current_address_, 0x1000U + 1024U);
}

/**
 * @test DS with division expression
 */
TEST_F(CoreDirectiveHandlersTest, DsWithDivisionExpression) {
  // Act
  HandleDs("", "1024/4", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 256UL);
  EXPECT_EQ(current_address_, 0x1000U + 256U);
}

/**
 * @test ORG with operator precedence (multiplication before addition)
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithOperatorPrecedence) {
  // Act - should evaluate as $1000 + ($10 * 2) = $1000 + $20 = $1020
  HandleOrg("", "$1000+$10*2", context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1020U);
  EXPECT_EQ(current_address_, 0x1020U);
}
