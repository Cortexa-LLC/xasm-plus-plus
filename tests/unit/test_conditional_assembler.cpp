/**
 * @file test_conditional_assembler.cpp
 * @brief Unit tests for ConditionalAssembler component
 *
 * Tests conditional assembly logic extracted from god-class parsers.
 * Part of god-class refactoring Phase 4.
 */

#include "xasm++/common/conditional_assembler.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace xasm;

/**
 * Test fixture for ConditionalAssembler tests
 */
class ConditionalAssemblerTest : public ::testing::Test {
protected:
  ConditionalAssembler assembler;
};

// ============================================================================
// Basic IF/ENDIF Tests
// ============================================================================

/**
 * @test Initially, should emit code (no conditionals active)
 */
TEST_F(ConditionalAssemblerTest, InitialState_ShouldEmit) {
  EXPECT_TRUE(assembler.ShouldEmit());
}

/**
 * @test IF with true condition should emit code
 */
TEST_F(ConditionalAssemblerTest, IfTrue_ShouldEmit) {
  assembler.BeginIf(true);
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.EndIf();
}

/**
 * @test IF with false condition should not emit code
 */
TEST_F(ConditionalAssemblerTest, IfFalse_ShouldNotEmit) {
  assembler.BeginIf(false);
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.EndIf();
}

/**
 * @test After ENDIF, should return to emitting
 */
TEST_F(ConditionalAssemblerTest, AfterEndIf_ShouldEmit) {
  assembler.BeginIf(false);
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.EndIf();
  EXPECT_TRUE(assembler.ShouldEmit());
}

// ============================================================================
// ELSE Tests
// ============================================================================

/**
 * @test ELSE after true IF should not emit
 */
TEST_F(ConditionalAssemblerTest, ElseAfterTrueIf_ShouldNotEmit) {
  assembler.BeginIf(true);
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.BeginElse();
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.EndIf();
}

/**
 * @test ELSE after false IF should emit
 */
TEST_F(ConditionalAssemblerTest, ElseAfterFalseIf_ShouldEmit) {
  assembler.BeginIf(false);
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.BeginElse();
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.EndIf();
}

/**
 * @test Multiple ELSE in same block should throw
 */
TEST_F(ConditionalAssemblerTest, MultipleElse_ShouldThrow) {
  assembler.BeginIf(true);
  assembler.BeginElse();
  EXPECT_THROW(assembler.BeginElse(), std::runtime_error);
}

/**
 * @test ELSE without IF should throw
 */
TEST_F(ConditionalAssemblerTest, ElseWithoutIf_ShouldThrow) {
  EXPECT_THROW(assembler.BeginElse(), std::runtime_error);
}

// ============================================================================
// Nested Conditional Tests
// ============================================================================

/**
 * @test Nested IF: true inside true
 */
TEST_F(ConditionalAssemblerTest, NestedIfTrueInTrue_ShouldEmit) {
  assembler.BeginIf(true); // Outer: true
  assembler.BeginIf(true); // Inner: true
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.EndIf(); // End inner
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.EndIf(); // End outer
}

/**
 * @test Nested IF: false inside true
 */
TEST_F(ConditionalAssemblerTest, NestedIfFalseInTrue_InnerShouldNotEmit) {
  assembler.BeginIf(true); // Outer: true, emit
  EXPECT_TRUE(assembler.ShouldEmit());
  assembler.BeginIf(false); // Inner: false, don't emit
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.EndIf();                   // End inner
  EXPECT_TRUE(assembler.ShouldEmit()); // Back to outer
  assembler.EndIf();                   // End outer
}

/**
 * @test Nested IF: true inside false (should not emit due to parent)
 */
TEST_F(ConditionalAssemblerTest, NestedIfTrueInFalse_ShouldNotEmit) {
  assembler.BeginIf(false); // Outer: false, don't emit
  EXPECT_FALSE(assembler.ShouldEmit());
  assembler.BeginIf(true);              // Inner: true, but parent is false
  EXPECT_FALSE(assembler.ShouldEmit()); // Still don't emit
  assembler.EndIf();                    // End inner
  EXPECT_FALSE(assembler.ShouldEmit()); // Still in false outer
  assembler.EndIf();                    // End outer
}

/**
 * @test Nested ELSE blocks
 */
TEST_F(ConditionalAssemblerTest, NestedElse_ComplexNesting) {
  assembler.BeginIf(true); // Outer: true, emit
  EXPECT_TRUE(assembler.ShouldEmit());

  assembler.BeginIf(false); // Inner: false, don't emit
  EXPECT_FALSE(assembler.ShouldEmit());

  assembler.BeginElse(); // Inner ELSE: emit (outer true, inner condition false)
  EXPECT_TRUE(assembler.ShouldEmit());

  assembler.EndIf(); // End inner
  EXPECT_TRUE(assembler.ShouldEmit());

  assembler.BeginElse(); // Outer ELSE: don't emit
  EXPECT_FALSE(assembler.ShouldEmit());

  assembler.EndIf(); // End outer
  EXPECT_TRUE(assembler.ShouldEmit());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

/**
 * @test ENDIF without IF should throw
 */
TEST_F(ConditionalAssemblerTest, EndIfWithoutIf_ShouldThrow) {
  EXPECT_THROW(assembler.EndIf(), std::runtime_error);
}

/**
 * @test Unmatched IF (missing ENDIF) detection
 */
TEST_F(ConditionalAssemblerTest, UnmatchedIf_IsDetectable) {
  assembler.BeginIf(true);
  EXPECT_FALSE(assembler.IsBalanced());
  assembler.EndIf();
  EXPECT_TRUE(assembler.IsBalanced());
}

// ============================================================================
// Stack Depth Tests
// ============================================================================

/**
 * @test Deep nesting works correctly
 */
TEST_F(ConditionalAssemblerTest, DeepNesting_Works) {
  // Level 1: true
  assembler.BeginIf(true);
  EXPECT_TRUE(assembler.ShouldEmit());

  // Level 2: true
  assembler.BeginIf(true);
  EXPECT_TRUE(assembler.ShouldEmit());

  // Level 3: false
  assembler.BeginIf(false);
  EXPECT_FALSE(assembler.ShouldEmit());

  // Level 4: true (but parent is false)
  assembler.BeginIf(true);
  EXPECT_FALSE(assembler.ShouldEmit());

  // Unwind
  assembler.EndIf(); // Level 3
  EXPECT_FALSE(assembler.ShouldEmit());

  assembler.EndIf(); // Level 2
  EXPECT_TRUE(assembler.ShouldEmit());

  assembler.EndIf(); // Level 1
  EXPECT_TRUE(assembler.ShouldEmit());

  assembler.EndIf(); // Level 0
  EXPECT_TRUE(assembler.ShouldEmit());
}

// ============================================================================
// Reset Tests
// ============================================================================

/**
 * @test Reset clears all conditional state
 */
TEST_F(ConditionalAssemblerTest, Reset_ClearsState) {
  assembler.BeginIf(false);
  assembler.BeginIf(true);
  EXPECT_FALSE(assembler.ShouldEmit());

  assembler.Reset();
  EXPECT_TRUE(assembler.ShouldEmit());
  EXPECT_TRUE(assembler.IsBalanced());
}
