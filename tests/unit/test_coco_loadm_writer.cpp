/**
 * @file test_coco_loadm_writer.cpp
 * @brief Unit tests for CoCo DOS (LOADM) binary format writer
 *
 * Tests for the CocoLoadmWriter class, covering:
 * - Preamble block (0x00)
 * - Data blocks with address and length
 * - Postamble block (0xFF)
 * - Entry point specification
 * - Multiple segment support
 *
 * @note CoCo DOS format used on TRS-80 Color Computer
 *       Commands: LOADM "filename" then EXEC &HXXXX
 */

#include "xasm++/atom.h"
#include "xasm++/output/coco_loadm_writer.h"
#include "xasm++/section.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

/**
 * @brief Test fixture for CoCo LOADM writer tests
 */
class CocoLoadmWriterTest : public ::testing::Test {
protected:
  CocoLoadmWriter writer;
  std::ostringstream output;

  /**
   * @brief Create a section with data atoms
   */
  Section CreateSectionWithData(const std::string &name, uint64_t org,
                                const std::vector<uint8_t> &data) {
    Section section(name, static_cast<uint32_t>(SectionAttributes::Data), org);
    auto data_atom = std::make_shared<DataAtom>(data);
    section.atoms.push_back(data_atom);
    return section;
  }

  /**
   * @brief Read bytes from output stream
   */
  std::vector<uint8_t> GetOutputBytes() {
    std::string str = output.str();
    return std::vector<uint8_t>(str.begin(), str.end());
  }

  /**
   * @brief Read 16-bit big-endian value from byte vector
   */
  uint16_t ReadBE16(const std::vector<uint8_t> &data, size_t offset) {
    return (data[offset] << 8) | data[offset + 1];
  }
};

// ============================================================================
// Basic Format Tests
// ============================================================================

/**
 * @brief Test empty output
 */
TEST_F(CocoLoadmWriterTest, EmptyOutput) {
  std::vector<Section> sections;
  writer.Write(sections, output);

  auto bytes = GetOutputBytes();
  // Empty should produce preamble + postamble
  ASSERT_EQ(bytes.size(), 7); // preamble (5) + postamble (2)

  // Check preamble
  EXPECT_EQ(bytes[0], 0x00);        // Preamble type
  EXPECT_EQ(ReadBE16(bytes, 1), 0); // Length
  EXPECT_EQ(ReadBE16(bytes, 3), 0); // Address

  // Check postamble
  EXPECT_EQ(bytes[5], 0xFF); // Postamble type
  EXPECT_EQ(bytes[6], 0x00); // Subtype (no entry)
}

/**
 * @brief Test single byte at address 0x2000
 */
TEST_F(CocoLoadmWriterTest, SingleByteAtAddress) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x2000, {0x42}));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // Format: preamble (5) + data block (5 + 1) + postamble (2) = 13
  ASSERT_EQ(bytes.size(), 13);

  // Check preamble
  EXPECT_EQ(bytes[0], 0x00);

  // Check data block
  size_t data_offset = 5;
  EXPECT_EQ(bytes[data_offset], 0x00);                 // Data block type
  EXPECT_EQ(ReadBE16(bytes, data_offset + 1), 1);      // Length
  EXPECT_EQ(ReadBE16(bytes, data_offset + 3), 0x2000); // Address
  EXPECT_EQ(bytes[data_offset + 5], 0x42);             // Data

  // Check postamble
  EXPECT_EQ(bytes[11], 0xFF);
  EXPECT_EQ(bytes[12], 0x00);
}

/**
 * @brief Test multiple bytes in single block
 */
TEST_F(CocoLoadmWriterTest, MultipleBytes) {
  std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x4000, test_data));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // preamble (5) + data block (5 + 5) + postamble (2) = 17
  ASSERT_EQ(bytes.size(), 17);

  // Check data block
  size_t data_offset = 5;
  EXPECT_EQ(bytes[data_offset], 0x00);
  EXPECT_EQ(ReadBE16(bytes, data_offset + 1), 5);
  EXPECT_EQ(ReadBE16(bytes, data_offset + 3), 0x4000);

  // Check data bytes
  for (size_t i = 0; i < test_data.size(); ++i) {
    EXPECT_EQ(bytes[data_offset + 5 + i], test_data[i]) << "Data byte " << i;
  }
}

// ============================================================================
// Multiple Block Tests
// ============================================================================

/**
 * @brief Test multiple sections (multiple data blocks)
 */
TEST_F(CocoLoadmWriterTest, MultipleSections) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x2000, {0xAA, 0xBB}));
  sections.push_back(CreateSectionWithData("DATA", 0x3000, {0xCC, 0xDD}));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // preamble (5) + block1 (5+2) + block2 (5+2) + postamble (2) = 21
  ASSERT_EQ(bytes.size(), 21);

  // First block
  size_t offset1 = 5;
  EXPECT_EQ(bytes[offset1], 0x00);
  EXPECT_EQ(ReadBE16(bytes, offset1 + 1), 2);
  EXPECT_EQ(ReadBE16(bytes, offset1 + 3), 0x2000);
  EXPECT_EQ(bytes[offset1 + 5], 0xAA);
  EXPECT_EQ(bytes[offset1 + 6], 0xBB);

  // Second block
  size_t offset2 = offset1 + 7;
  EXPECT_EQ(bytes[offset2], 0x00);
  EXPECT_EQ(ReadBE16(bytes, offset2 + 1), 2);
  EXPECT_EQ(ReadBE16(bytes, offset2 + 3), 0x3000);
  EXPECT_EQ(bytes[offset2 + 5], 0xCC);
  EXPECT_EQ(bytes[offset2 + 6], 0xDD);
}

// ============================================================================
// Entry Point Tests
// ============================================================================

/**
 * @brief Test with entry point address
 */
TEST_F(CocoLoadmWriterTest, WithEntryPoint) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x2000, {0x01, 0x02}));

  writer.SetEntryPoint(0x2000);
  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // preamble (5) + data (5+2) + postamble_with_entry (5) = 17
  ASSERT_EQ(bytes.size(), 17);

  // Check postamble with entry
  size_t post_offset = bytes.size() - 5;
  EXPECT_EQ(bytes[post_offset], 0xFF);                 // Postamble
  EXPECT_EQ(bytes[post_offset + 1], 0x00);             // Subtype (entry)
  EXPECT_EQ(ReadBE16(bytes, post_offset + 3), 0x2000); // Entry address
}

// ============================================================================
// Format Information Tests
// ============================================================================

/**
 * @brief Test GetExtension returns correct extension
 */
TEST_F(CocoLoadmWriterTest, GetExtension) {
  EXPECT_EQ(writer.GetExtension(), "bin");
}

/**
 * @brief Test GetFormatName returns descriptive name
 */
TEST_F(CocoLoadmWriterTest, GetFormatName) {
  EXPECT_EQ(writer.GetFormatName(), "CoCo DOS (LOADM)");
}

// ============================================================================
// Edge Cases
// ============================================================================

/**
 * @brief Test section with space atom (creates separate blocks)
 */
TEST_F(CocoLoadmWriterTest, SectionWithSpaceAtom) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x2000);

  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));
  section.atoms.push_back(std::make_shared<SpaceAtom>(10));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xBB}));

  sections.push_back(section);

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // preamble (5) + block1 (5+1) + block2 (5+1) + postamble (2) = 19
  ASSERT_EQ(bytes.size(), 19);

  // First block: 0xAA at 0x2000
  size_t offset1 = 5;
  EXPECT_EQ(bytes[offset1], 0x00);
  EXPECT_EQ(ReadBE16(bytes, offset1 + 1), 1);
  EXPECT_EQ(ReadBE16(bytes, offset1 + 3), 0x2000);
  EXPECT_EQ(bytes[offset1 + 5], 0xAA);

  // Second block: 0xBB at 0x200B (0x2000 + 1 + 10)
  size_t offset2 = offset1 + 6;
  EXPECT_EQ(bytes[offset2], 0x00);
  EXPECT_EQ(ReadBE16(bytes, offset2 + 1), 1);
  EXPECT_EQ(ReadBE16(bytes, offset2 + 3), 0x200B);
  EXPECT_EQ(bytes[offset2 + 5], 0xBB);
}
