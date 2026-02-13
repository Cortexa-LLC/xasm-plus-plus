/**
 * @file test_segment_directives.cpp
 * @brief Unit tests for EDTASM segment directives (ASEG/CSEG/DSEG)
 *
 * Tests segment directive handlers for relocatable code support.
 */

#include "xasm++/section.h"
#include "xasm++/segment_manager.h"
#include "xasm++/symbol.h"
#include "xasm++/syntax/directive_registry.h"
#include "xasm++/syntax/edtasm_directive_handlers.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include <gtest/gtest.h>
#include <memory>

using namespace xasm;

/**
 * @brief Test fixture for segment directive tests
 */
class SegmentDirectivesTest : public ::testing::Test {
protected:
  void SetUp() override {
    symbols_ = std::make_unique<ConcreteSymbolTable>();
    section_ = std::make_unique<Section>("test_section", 0x01);
    parser_ = std::make_unique<EdtasmM80PlusPlusSyntaxParser>();
    current_address_ = 0x1000;

    // Initialize context
    ctx_.section = section_.get();
    ctx_.symbols = symbols_.get();
    ctx_.current_address = &current_address_;
    ctx_.parser_state = parser_.get();
  }

  std::unique_ptr<ConcreteSymbolTable> symbols_;
  std::unique_ptr<Section> section_;
  std::unique_ptr<EdtasmM80PlusPlusSyntaxParser> parser_;
  uint32_t current_address_;
  DirectiveContext ctx_;
};

// ============================================================================
// ASEG Directive Tests
// ============================================================================

/**
 * @test ASEG switches to absolute segment
 */
TEST_F(SegmentDirectivesTest, AsegSwitchesToAbsoluteSegment) {
  // Act
  HandleAsegDirective("", "", ctx_);

  // Assert
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentSegmentType(),
            SegmentType::Absolute);
}

/**
 * @test ASEG with no operand starts at address 0
 */
TEST_F(SegmentDirectivesTest, AsegDefaultsToZero) {
  // Act
  HandleAsegDirective("", "", ctx_);

  // Assert - should be at 0 (default for new segment)
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0);
}

// ============================================================================
// CSEG Directive Tests
// ============================================================================

/**
 * @test CSEG switches to code segment
 */
TEST_F(SegmentDirectivesTest, CsegSwitchesToCodeSegment) {
  // Act
  HandleCsegDirective("", "", ctx_);

  // Assert
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentSegmentType(),
            SegmentType::Code);
}

/**
 * @test CSEG with no operand starts at address 0
 */
TEST_F(SegmentDirectivesTest, CsegDefaultsToZero) {
  // Act
  HandleCsegDirective("", "", ctx_);

  // Assert
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0);
}

// ============================================================================
// DSEG Directive Tests
// ============================================================================

/**
 * @test DSEG switches to data segment
 */
TEST_F(SegmentDirectivesTest, DsegSwitchesToDataSegment) {
  // Act
  HandleDsegDirective("", "", ctx_);

  // Assert
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentSegmentType(),
            SegmentType::Data);
}

/**
 * @test DSEG with no operand starts at address 0
 */
TEST_F(SegmentDirectivesTest, DsegDefaultsToZero) {
  // Act
  HandleDsegDirective("", "", ctx_);

  // Assert
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0);
}

// ============================================================================
// Segment Switching Tests
// ============================================================================

/**
 * @test Switching segments preserves individual address counters
 */
TEST_F(SegmentDirectivesTest, SegmentSwitchingPreservesAddresses) {
  // Arrange - Start in CSEG, advance to 0x1000
  HandleCsegDirective("", "", ctx_);
  parser_->GetSegmentManager().SetOrigin(0x1000);
  parser_->GetSegmentManager().Advance(10);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0x100A);

  // Act - Switch to DSEG, set origin to 0x2000
  HandleDsegDirective("", "", ctx_);
  parser_->GetSegmentManager().SetOrigin(0x2000);
  parser_->GetSegmentManager().Advance(5);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0x2005);

  // Act - Switch back to CSEG
  HandleCsegDirective("", "", ctx_);

  // Assert - CSEG address should be preserved at 0x100A
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(), 0x100A);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentSegmentType(),
            SegmentType::Code);
}

/**
 * @test Multiple segment switches maintain independent address spaces
 */
TEST_F(SegmentDirectivesTest, MultipleSegmentSwitches) {
  // CSEG @ 0x1000
  HandleCsegDirective("", "", ctx_);
  parser_->GetSegmentManager().SetOrigin(0x1000);
  parser_->GetSegmentManager().Advance(20);

  // DSEG @ 0x2000
  HandleDsegDirective("", "", ctx_);
  parser_->GetSegmentManager().SetOrigin(0x2000);
  parser_->GetSegmentManager().Advance(30);

  // ASEG @ 0x3000
  HandleAsegDirective("", "", ctx_);
  parser_->GetSegmentManager().SetOrigin(0x3000);
  parser_->GetSegmentManager().Advance(40);

  // Verify each segment preserved its address
  HandleCsegDirective("", "", ctx_);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(),
            0x1014); // 0x1000 + 20

  HandleDsegDirective("", "", ctx_);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(),
            0x201E); // 0x2000 + 30

  HandleAsegDirective("", "", ctx_);
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentAddress(),
            0x3028); // 0x3000 + 40
}

/**
 * @test Initial segment type is Code
 */
TEST_F(SegmentDirectivesTest, InitialSegmentIsCode) {
  // Assert - Default should be Code segment
  EXPECT_EQ(parser_->GetSegmentManager().GetCurrentSegmentType(),
            SegmentType::Code);
}
