/**
 * @file test_core_directive_handlers.cpp
 * @brief Unit tests for core directive handlers (kORG, kEQU, kDB, kDW, kDS)
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
// kORG Directive Tests
// ============================================================================

/**
 * @test kORG creates OrgAtom with correct decimal address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithDecimalAddress) {
  // Act
  context_.operand = "2048";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 2048U);
  EXPECT_EQ(current_address_, 2048U);
}

/**
 * @test kORG creates OrgAtom with correct hex address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithHexAddress) {
  // Act
  context_.operand = "$C000";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0xC000U);
  EXPECT_EQ(current_address_, 0xC000U);
}

/**
 * @test kORG creates OrgAtom with correct binary address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithBinaryAddress) {
  // Act
  context_.operand = "%11111111";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 255U);
  EXPECT_EQ(current_address_, 255U);
}

/**
 * @test kORG resolves symbol address
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSymbolAddress) {
  // Arrange
  symbols_->Define("START", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000));

  // Act
  context_.operand = "START";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8000U);
  EXPECT_EQ(current_address_, 0x8000U);
}

/**
 * @test kORG throws on empty operand
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithEmptyOperandThrows) {
  // Act & Assert
  do { context_.operand = ""; EXPECT_THROW(HandleOrg(context_), std::runtime_error); } while(0);
}

/**
 * @test kORG handles whitespace in operand
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithWhitespace) {
  // Act
  context_.operand = "  $1000  ";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1000U);
}

// ============================================================================
// kEQU Directive Tests
// ============================================================================

/**
 * @test kEQU defines symbol with literal value
 */
TEST_F(CoreDirectiveHandlersTest, EquWithLiteralValue) {
  // Act
  context_.label = "BUFFER_SIZE";
  context_.operand = "256";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("BUFFER_SIZE", value));
  EXPECT_EQ(value, 256);
}

/**
 * @test kEQU defines symbol with hex value
 */
TEST_F(CoreDirectiveHandlersTest, EquWithHexValue) {
  // Act
  context_.label = "IO_PORT";
  context_.operand = "$D000";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("IO_PORT", value));
  EXPECT_EQ(value, 0xD000);
}

/**
 * @test kEQU defines symbol with expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithExpression) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x2000));

  // Act
  context_.label = "OFFSET";
  context_.operand = "BASE+$100";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("OFFSET", value));
  EXPECT_EQ(value, 0x2100);
}

/**
 * @test kEQU throws on empty label
 */
TEST_F(CoreDirectiveHandlersTest, EquWithEmptyLabelThrows) {
  // Act & Assert
  do { context_.operand = "100"; EXPECT_THROW(HandleEqu(context_), std::runtime_error); } while(0);
}

/**
 * @test kEQU does not create atoms
 */
TEST_F(CoreDirectiveHandlersTest, EquDoesNotCreateAtoms) {
  // Act
  context_.label = "CONSTANT";
  context_.operand = "42";
  HandleEqu(context_);

  // Assert
  EXPECT_EQ(section_->atoms.size(), 0UL);
}

// ============================================================================
// kDB Directive Tests
// ============================================================================

/**
 * @test kDB creates DataAtom with single byte
 */
TEST_F(CoreDirectiveHandlersTest, DbWithSingleByte) {
  // Act
  context_.operand = "42";
  HandleDb(context_);

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
 * @test kDB creates DataAtom with multiple bytes
 */
TEST_F(CoreDirectiveHandlersTest, DbWithMultipleBytes) {
  // Act
  context_.operand = "1,2,3,4,5";
  HandleDb(context_);

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
 * @test kDB handles expressions
 */
TEST_F(CoreDirectiveHandlersTest, DbWithExpressions) {
  // Act
  context_.operand = "$kFF,1+2,%00001111";
  HandleDb(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(data_atom->expressions[0], "$kFF");
  EXPECT_EQ(data_atom->expressions[1], "1+2");
  EXPECT_EQ(data_atom->expressions[2], "%00001111");
}

/**
 * @test kDB handles whitespace in operands
 */
TEST_F(CoreDirectiveHandlersTest, DbWithWhitespace) {
  // Act
  context_.operand = "  1 ,  2  , 3  ";
  HandleDb(context_);

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
 * @test kDB handles empty operand
 */
TEST_F(CoreDirectiveHandlersTest, DbWithEmptyOperand) {
  // Act
  context_.operand = "";
  HandleDb(context_);

  // Assert - should create empty DataAtom
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->expressions.size(), 0UL);
  EXPECT_EQ(current_address_, 0x1000U); // Not advanced
}

// ============================================================================
// kDW Directive Tests
// ============================================================================

/**
 * @test kDW creates DataAtom with word size
 */
TEST_F(CoreDirectiveHandlersTest, DwWithSingleWord) {
  // Act
  context_.operand = "$1234";
  HandleDw(context_);

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
 * @test kDW creates DataAtom with multiple words
 */
TEST_F(CoreDirectiveHandlersTest, DwWithMultipleWords) {
  // Act
  context_.operand = "$1000,$2000,$3000";
  HandleDw(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  ASSERT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(current_address_, 0x1006U); // Advanced by 6 bytes (3 words)
}

/**
 * @test kDW handles whitespace
 */
TEST_F(CoreDirectiveHandlersTest, DwWithWhitespace) {
  // Act
  context_.operand = "  $1000  ,  $2000  ";
  HandleDw(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  ASSERT_EQ(data_atom->expressions.size(), 2UL);
  EXPECT_EQ(data_atom->expressions[0], "$1000");
  EXPECT_EQ(data_atom->expressions[1], "$2000");
}

// ============================================================================
// kDS Directive Tests
// ============================================================================

/**
 * @test kDS creates SpaceAtom with correct count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithLiteralCount) {
  // Act
  context_.operand = "100";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U); // Advanced by 100 bytes
}

/**
 * @test kDS handles hex count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithHexCount) {
  // Act
  context_.operand = "$100";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 256UL);
  EXPECT_EQ(current_address_, 0x1100U); // Advanced by 256 bytes
}

/**
 * @test kDS handles expression with symbol
 */
TEST_F(CoreDirectiveHandlersTest, DsWithExpression) {
  // Arrange
  symbols_->Define("SIZE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(50));

  // Act
  context_.operand = "SIZE*2";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U); // Advanced by 100 bytes
}

/**
 * @test kDS throws on negative count
 */
TEST_F(CoreDirectiveHandlersTest, DsWithNegativeCountThrows) {
  // Act & Assert
  do { context_.operand = "-10"; EXPECT_THROW(HandleDs(context_), std::runtime_error); } while(0);
}

/**
 * @test kDS handles whitespace
 */
TEST_F(CoreDirectiveHandlersTest, DsWithWhitespace) {
  // Act
  context_.operand = "  50  ";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 50UL);
}

/**
 * @test kDS with empty operand creates zero-size SpaceAtom
 */
TEST_F(CoreDirectiveHandlersTest, DsWithEmptyOperand) {
  // Act
  context_.operand = "";
  HandleDs(context_);

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
 * @test Registry kORG directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryOrgWorks) {
  // Act
  context_.mnemonic = directives::kORG;
  context_.label = "";
  context_.operand = "$C000";
  registry_.Execute(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0xC000U);
  EXPECT_EQ(current_address_, 0xC000U);
}

/**
 * @test Registry kEQU directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryEquWorks) {
  // Act
  context_.mnemonic = directives::kEQU;
  context_.label = "TEST_VALUE";
  context_.operand = "42";
  registry_.Execute(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("TEST_VALUE", value));
  EXPECT_EQ(value, 42);
}

/**
 * @test Registry kDB directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDbWorks) {
  // Act
  context_.mnemonic = directives::kDB;
  context_.label = "";
  context_.operand = "1,2,3";
  registry_.Execute(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Byte);
  EXPECT_EQ(data_atom->expressions.size(), 3UL);
  EXPECT_EQ(current_address_, 0x1003U);
}

/**
 * @test Registry kDW directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDwWorks) {
  // Act
  context_.mnemonic = directives::kDW;
  context_.label = "";
  context_.operand = "$1000,$2000";
  registry_.Execute(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto data_atom = std::dynamic_pointer_cast<DataAtom>(section_->atoms[0]);
  ASSERT_NE(data_atom, nullptr);
  EXPECT_EQ(data_atom->data_size, DataSize::Word);
  EXPECT_EQ(data_atom->expressions.size(), 2UL);
  EXPECT_EQ(current_address_, 0x1004U);
}

/**
 * @test Registry kDS directive works correctly
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDsWorks) {
  // Act
  context_.mnemonic = directives::kDS;
  context_.label = "";
  context_.operand = "100";
  registry_.Execute(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 100UL);
  EXPECT_EQ(current_address_, 0x1064U);
}

/**
 * @test Registry recognizes kDB aliases (kDEFB, kBYTE)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDbAliases) {
  // Test kDEFB
  context_.mnemonic = directives::kDEFB;
  context_.label = "";
  context_.operand = "42";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test kBYTE
  context_.mnemonic = directives::kBYTE;
  context_.label = "";
  context_.operand = "43";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);
}

/**
 * @test Registry recognizes kDW aliases (kDEFW, kWORD)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDwAliases) {
  // Test kDEFW
  context_.mnemonic = directives::kDEFW;
  context_.label = "";
  context_.operand = "$1234";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test kWORD
  context_.mnemonic = directives::kWORD;
  context_.label = "";
  context_.operand = "$5678";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);
}

/**
 * @test Registry recognizes kDS aliases (kDEFS, kBLOCK, kRMB)
 */
TEST_F(CoreDirectiveRegistryTest, RegistryDsAliases) {
  // Test kDEFS
  context_.mnemonic = directives::kDEFS;
  context_.label = "";
  context_.operand = "10";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 1UL);

  // Test kBLOCK
  context_.mnemonic = directives::kBLOCK;
  context_.label = "";
  context_.operand = "20";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 2UL);

  // Test kRMB
  context_.mnemonic = directives::kRMB;
  context_.label = "";
  context_.operand = "30";
  registry_.Execute(context_);
  ASSERT_EQ(section_->atoms.size(), 3UL);
}

/**
 * @test Registry IsRegistered works for all directives
 */
TEST_F(CoreDirectiveRegistryTest, RegistryIsRegistered) {
  EXPECT_TRUE(registry_.IsRegistered(directives::kORG));
  EXPECT_TRUE(registry_.IsRegistered(directives::kEQU));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDB));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDEFB));
  EXPECT_TRUE(registry_.IsRegistered(directives::kBYTE));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDW));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDEFW));
  EXPECT_TRUE(registry_.IsRegistered(directives::kWORD));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDS));
  EXPECT_TRUE(registry_.IsRegistered(directives::kDEFS));
  EXPECT_TRUE(registry_.IsRegistered(directives::kBLOCK));
  EXPECT_TRUE(registry_.IsRegistered(directives::kRMB));
  EXPECT_FALSE(registry_.IsRegistered("UNKNOWN"));
}

// ============================================================================
// Expression Arithmetic Tests
// ============================================================================

/**
 * @test kORG with addition expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithAdditionExpression) {
  // Act
  context_.operand = "$1000+$100";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1100U);
  EXPECT_EQ(current_address_, 0x1100U);
}

/**
 * @test kORG with subtraction expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSubtractionExpression) {
  // Act
  context_.operand = "$2000-$100";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1F00U);
  EXPECT_EQ(current_address_, 0x1F00U);
}

/**
 * @test kORG with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithMultiplicationExpression) {
  // Act
  context_.operand = "$100*16";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1000U);
  EXPECT_EQ(current_address_, 0x1000U);
}

/**
 * @test kORG with division expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithDivisionExpression) {
  // Act
  context_.operand = "$4000/2";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x2000U);
  EXPECT_EQ(current_address_, 0x2000U);
}

/**
 * @test kORG with modulo expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithModuloExpression) {
  // Act
  context_.operand = "1000%256";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 232U);
  EXPECT_EQ(current_address_, 232U);
}

/**
 * @test kORG with nested expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithNestedExpression) {
  // Act
  context_.operand = "($1000+$100)*2";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x2200U);
  EXPECT_EQ(current_address_, 0x2200U);
}

/**
 * @test kORG with complex nested expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithComplexNestedExpression) {
  // Act
  context_.operand = "(100+50)*2-20";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 280U); // (100+50)*2-20 = 150*2-20 = 300-20 = 280
  EXPECT_EQ(current_address_, 280U);
}

/**
 * @test kORG with symbol in arithmetic expression
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithSymbolArithmetic) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x8000));

  // Act
  context_.operand = "BASE+$100";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x8100U);
  EXPECT_EQ(current_address_, 0x8100U);
}

/**
 * @test kEQU with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithMultiplicationExpression) {
  // Act
  context_.label = "SIZE";
  context_.operand = "64*1024";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("SIZE", value));
  EXPECT_EQ(value, 65536);
}

/**
 * @test kEQU with complex expression
 */
TEST_F(CoreDirectiveHandlersTest, EquWithComplexExpression) {
  // Arrange
  symbols_->Define("kWIDTH", SymbolType::Label,
                   std::make_shared<LiteralExpr>(40));
  symbols_->Define("HEIGHT", SymbolType::Label,
                   std::make_shared<LiteralExpr>(25));

  // Act
  context_.label = "SCREEN_SIZE";
  context_.operand = "kWIDTH*HEIGHT";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("SCREEN_SIZE", value));
  EXPECT_EQ(value, 1000);
}

/**
 * @test kEQU with nested expression and symbols
 */
TEST_F(CoreDirectiveHandlersTest, EquWithNestedExpressionAndSymbols) {
  // Arrange
  symbols_->Define("BASE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x1000));
  symbols_->Define("OFFSET", SymbolType::Label,
                   std::make_shared<LiteralExpr>(0x10));

  // Act
  context_.label = "ADDR";
  context_.operand = "(BASE+OFFSET)*2";
  HandleEqu(context_);

  // Assert
  int64_t value = 0;
  ASSERT_TRUE(symbols_->Lookup("ADDR", value));
  EXPECT_EQ(value, 0x2020);
}

/**
 * @test kDS with multiplication expression
 */
TEST_F(CoreDirectiveHandlersTest, DsWithMultiplicationExpression) {
  // Act
  context_.operand = "10*16";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 160UL);
  EXPECT_EQ(current_address_, 0x1000U + 160U);
}

/**
 * @test kDS with symbol and arithmetic
 */
TEST_F(CoreDirectiveHandlersTest, DsWithSymbolAndArithmetic) {
  // Arrange
  symbols_->Define("PAGE_SIZE", SymbolType::Label,
                   std::make_shared<LiteralExpr>(256));

  // Act
  context_.operand = "PAGE_SIZE*4";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 1024UL);
  EXPECT_EQ(current_address_, 0x1000U + 1024U);
}

/**
 * @test kDS with division expression
 */
TEST_F(CoreDirectiveHandlersTest, DsWithDivisionExpression) {
  // Act
  context_.operand = "1024/4";
  HandleDs(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(section_->atoms[0]);
  ASSERT_NE(space_atom, nullptr);
  EXPECT_EQ(space_atom->count, 256UL);
  EXPECT_EQ(current_address_, 0x1000U + 256U);
}

/**
 * @test kORG with operator precedence (multiplication before addition)
 */
TEST_F(CoreDirectiveHandlersTest, OrgWithOperatorPrecedence) {
  // Act - should evaluate as $1000 + ($10 * 2) = $1000 + $20 = $1020
  context_.operand = "$1000+$10*2";
  HandleOrg(context_);

  // Assert
  ASSERT_EQ(section_->atoms.size(), 1UL);
  auto org_atom = std::dynamic_pointer_cast<OrgAtom>(section_->atoms[0]);
  ASSERT_NE(org_atom, nullptr);
  EXPECT_EQ(org_atom->address, 0x1020U);
  EXPECT_EQ(current_address_, 0x1020U);
}
