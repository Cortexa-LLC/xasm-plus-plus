/**
 * @file test_directive_registry.cpp
 * @brief Unit tests for DirectiveRegistry
 */

#include "xasm++/syntax/directive_registry.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace xasm;

/**
 * @brief Test fixture for DirectiveRegistry tests
 */
class DirectiveRegistryTest : public ::testing::Test {
protected:
  DirectiveRegistry registry_;
};

/**
 * @test Verify that a directive handler can be registered and executed
 */
TEST_F(DirectiveRegistryTest, RegisterAndExecute) {
  // Track if handler was called
  bool handler_called = false;
  std::string received_label;
  std::string received_operand;

  // Create a simple handler
  auto handler = [&handler_called, &received_label, &received_operand](
                     const std::string &label, const std::string &operand,
                     DirectiveContext & /*context*/) {
    handler_called = true;
    received_label = label;
    received_operand = operand;
  };

  // Register the handler
  registry_.Register("TEST", handler);

  // Create a minimal context
  DirectiveContext context;

  // Execute the handler
  registry_.Execute("TEST", "MY_LABEL", "some_operand", context);

  // Verify handler was called with correct parameters
  EXPECT_TRUE(handler_called);
  EXPECT_EQ(received_label, "MY_LABEL");
  EXPECT_EQ(received_operand, "some_operand");
}

/**
 * @test Verify that directive lookup is case-insensitive
 */
TEST_F(DirectiveRegistryTest, CaseInsensitiveLookup) {
  bool handler_called = false;

  auto handler = [&handler_called](const std::string & /*label*/,
                                   const std::string & /*operand*/,
                                   DirectiveContext & /*context*/) {
    handler_called = true;
  };

  // Register with uppercase
  registry_.Register("ORG", handler);

  DirectiveContext context;

  // Execute with lowercase - should work
  registry_.Execute("org", "", "1000", context);
  EXPECT_TRUE(handler_called);

  // Reset flag
  handler_called = false;

  // Execute with mixed case - should work
  registry_.Execute("Org", "", "1000", context);
  EXPECT_TRUE(handler_called);
}

/**
 * @test Verify that executing an unregistered directive throws an exception
 */
TEST_F(DirectiveRegistryTest, UnknownDirectiveThrows) {
  DirectiveContext context;

  // Attempting to execute unknown directive should throw
  EXPECT_THROW(registry_.Execute("UNKNOWN", "", "operand", context),
               std::runtime_error);
}

/**
 * @test Verify that multiple directives can be registered
 */
TEST_F(DirectiveRegistryTest, MultipleDirectives) {
  unsigned int org_count = 0;
  unsigned int equ_count = 0;

  auto org_handler = [&org_count](const std::string & /*label*/,
                                  const std::string & /*operand*/,
                                  DirectiveContext & /*context*/) {
    org_count++;
  };

  auto equ_handler = [&equ_count](const std::string & /*label*/,
                                  const std::string & /*operand*/,
                                  DirectiveContext & /*context*/) {
    equ_count++;
  };

  // Register multiple handlers
  registry_.Register("ORG", org_handler);
  registry_.Register("EQU", equ_handler);

  DirectiveContext context;

  // Execute each
  registry_.Execute("ORG", "", "1000", context);
  registry_.Execute("EQU", "LABEL", "42", context);

  EXPECT_EQ(org_count, 1U);
  EXPECT_EQ(equ_count, 1U);
}

/**
 * @test Verify that directive can be checked for existence
 */
TEST_F(DirectiveRegistryTest, IsRegistered) {
  auto handler = [](const std::string & /*label*/,
                    const std::string & /*operand*/,
                    DirectiveContext & /*context*/) {};

  // Initially not registered
  EXPECT_FALSE(registry_.IsRegistered("ORG"));

  // Register it
  registry_.Register("ORG", handler);

  // Now it should be registered
  EXPECT_TRUE(registry_.IsRegistered("ORG"));

  // Case insensitive check
  EXPECT_TRUE(registry_.IsRegistered("org"));
  EXPECT_TRUE(registry_.IsRegistered("Org"));
}

/**
 * @test Verify that multiple aliases can point to same handler
 */
TEST_F(DirectiveRegistryTest, MultipleAliases) {
  unsigned int handler_count = 0;

  auto handler = [&handler_count](const std::string & /*label*/,
                                  const std::string & /*operand*/,
                                  DirectiveContext & /*context*/) {
    handler_count++;
  };

  // Register same handler with multiple names (DB aliases)
  registry_.Register("DB", handler);
  registry_.Register("DEFB", handler);
  registry_.Register("BYTE", handler);

  DirectiveContext context;

  // Execute with each alias
  registry_.Execute("DB", "", "42", context);
  registry_.Execute("DEFB", "", "42", context);
  registry_.Execute("BYTE", "", "42", context);

  // Handler should be called three times
  EXPECT_EQ(handler_count, 3U);
}
