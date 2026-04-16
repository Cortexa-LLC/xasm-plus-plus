/**
 * @file test_output_integration.cpp
 * @brief Integration tests for output format system
 *
 * Tests that output format writers can be instantiated and used correctly.
 *
 * @note Part of Output Formats Phase 3: CLI Integration
 */

#include <gtest/gtest.h>
#include <sstream>

#include "xasm++/atom.h"
#include "xasm++/output/binary_output.h"
#include "xasm++/output/coco_loadm_writer.h"
#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/output/listing_output.h"
#include "xasm++/output/srec_writer.h"
#include "xasm++/output/symbol_output.h"
#include "xasm++/output/trsdos_writer.h"
#include "xasm++/section.h"

using namespace xasm;

/**
 * @brief Test fixture for output format integration tests
 */
class OutputIntegrationTest : public ::testing::Test {
protected:
  /**
   * @brief Create a simple test section with data
   */
  Section CreateTestSection() {
    Section section("CODE", static_cast<uint32_t>(SectionAttributes::Code),
                    0x8000);
    std::vector<uint8_t> bytes = {0xA9, 0x42, 0x4C, 0x00, 0x80};
    section.atoms.push_back(std::make_shared<DataAtom>(bytes));
    return section;
  }
};

// ===========================================================================
// Output Format Writer Integration Tests
// ===========================================================================

/**
 * @test Intel HEX writer should be instantiable
 */
TEST_F(OutputIntegrationTest, IntelHexWriterInstantiation) {
  IntelHexWriter writer;
  EXPECT_FALSE(writer.GetExtension().empty());
}

/**
 * @test S-Record writer should be instantiable
 */
TEST_F(OutputIntegrationTest, SRecordWriterInstantiation) {
  SRecordWriter writer;
  EXPECT_FALSE(writer.GetExtension().empty());
}

/**
 * @test CoCo LOADM writer should be instantiable
 */
TEST_F(OutputIntegrationTest, CoCoLoadmWriterInstantiation) {
  CocoLoadmWriter writer;
  EXPECT_FALSE(writer.GetExtension().empty());
}

/**
 * @test TRS-DOS writer should be instantiable
 */
TEST_F(OutputIntegrationTest, TRSDOSWriterInstantiation) {
  TrsDosWriter writer;
  EXPECT_FALSE(writer.GetExtension().empty());
}

/**
 * @test Intel HEX writer should produce non-empty output
 */
TEST_F(OutputIntegrationTest, IntelHexWriterProducesOutput) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  IntelHexWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}

/**
 * @test S-Record writer should produce non-empty output
 */
TEST_F(OutputIntegrationTest, SRecordWriterProducesOutput) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  SRecordWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}

/**
 * @test CoCo LOADM writer should produce non-empty output
 */
TEST_F(OutputIntegrationTest, CoCoLoadmWriterProducesOutput) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  CocoLoadmWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}

/**
 * @test TRS-DOS writer should produce non-empty output
 */
TEST_F(OutputIntegrationTest, TrsDosWriterProducesOutput) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  TrsDosWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}

/**
 * @test Writers should handle empty section list gracefully
 */
TEST_F(OutputIntegrationTest, AllWritersHandleEmptySections) {
  std::vector<Section> empty_sections;

  {
    IntelHexWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(empty_sections, output));
  }

  {
    SRecordWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(empty_sections, output));
  }

  {
    CocoLoadmWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(empty_sections, output));
  }

  {
    TrsDosWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(empty_sections, output));
  }
}

/**
 * @test CoCo LOADM writer with entry point
 */
TEST_F(OutputIntegrationTest, CoCoLoadmWriterWithEntryPoint) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  CocoLoadmWriter writer;
  writer.SetEntryPoint(0x8000);
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}

/**
 * @test TRS-DOS writer with entry point
 */
TEST_F(OutputIntegrationTest, TrsDosWriterWithEntryPoint) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  TrsDosWriter writer;
  writer.SetEntryPoint(0x8000);
  std::ostringstream output;
  writer.Write(sections, output);
  EXPECT_FALSE(output.str().empty());
}
