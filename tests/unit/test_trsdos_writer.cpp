/**
 * @file test_trsdos_writer.cpp
 * @brief Unit tests for TRS-DOS binary format writer
 *
 * Tests for the TrsDosWriter class, covering:
 * - Header format (load address)
 * - Binary data output
 * - Entry point specification
 * - Multiple segment support
 * - Address gap handling
 *
 * @note TRS-DOS format used on TRS-80 Model I/III/4
 *       Command: SYSTEM "filename"
 */

#include "xasm++/atom.h"
#include "xasm++/output/trsdos_writer.h"
#include "xasm++/section.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

/**
 * @brief Test fixture for TRS-DOS writer tests
 */
class TrsDosWriterTest : public ::testing::Test {
protected:
  TrsDosWriter writer;
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
   * @brief Read 16-bit little-endian value from byte vector
   */
  uint16_t ReadLE16(const std::vector<uint8_t> &data, size_t offset) {
    return data[offset] | (data[offset + 1] << 8);
  }
};

// ============================================================================
// Basic Format Tests
// ============================================================================

/**
 * @brief Test empty output (should have header but no data)
 */
TEST_F(TrsDosWriterTest, EmptyOutput) {
  std::vector<Section> sections;
  writer.Write(sections, output);

  auto bytes = GetOutputBytes();
  // Empty sections should produce minimal header (if any)
  // TRS-DOS format may skip output for empty data
  EXPECT_EQ(bytes.size(), 0); // No data = no output
}

/**
 * @brief Test single byte at address 0x8000
 */
TEST_F(TrsDosWriterTest, SingleByteAtAddress) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x8000, {0x42}));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // Format: [segment_type] [load_addr_lo] [load_addr_hi] [length_lo]
  // [length_hi] [data...] Segment type: 0x01 for data segment
  ASSERT_GE(bytes.size(), 6); // header (5 bytes) + 1 byte data

  EXPECT_EQ(bytes[0], 0x01);             // Segment type
  EXPECT_EQ(ReadLE16(bytes, 1), 0x8000); // Load address
  EXPECT_EQ(ReadLE16(bytes, 3), 1);      // Length
  EXPECT_EQ(bytes[5], 0x42);             // Data
}

/**
 * @brief Test multiple bytes in single segment
 */
TEST_F(TrsDosWriterTest, MultipleBytes) {
  std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x4000, test_data));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  ASSERT_GE(bytes.size(), 10); // header (5) + data (5)

  EXPECT_EQ(bytes[0], 0x01);             // Segment type
  EXPECT_EQ(ReadLE16(bytes, 1), 0x4000); // Load address
  EXPECT_EQ(ReadLE16(bytes, 3), 5);      // Length

  // Check data bytes
  for (size_t i = 0; i < test_data.size(); ++i) {
    EXPECT_EQ(bytes[5 + i], test_data[i]) << "Data byte " << i;
  }
}

// ============================================================================
// Multiple Segment Tests
// ============================================================================

/**
 * @brief Test multiple sections (multiple segments)
 */
TEST_F(TrsDosWriterTest, MultipleSections) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x4000, {0xAA, 0xBB}));
  sections.push_back(CreateSectionWithData("DATA", 0x5000, {0xCC, 0xDD}));

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // Two segments: each with 5-byte header + 2 data bytes
  ASSERT_GE(bytes.size(), 14); // (5+2) + (5+2)

  // First segment
  EXPECT_EQ(bytes[0], 0x01);
  EXPECT_EQ(ReadLE16(bytes, 1), 0x4000);
  EXPECT_EQ(ReadLE16(bytes, 3), 2);
  EXPECT_EQ(bytes[5], 0xAA);
  EXPECT_EQ(bytes[6], 0xBB);

  // Second segment
  EXPECT_EQ(bytes[7], 0x01);
  EXPECT_EQ(ReadLE16(bytes, 8), 0x5000);
  EXPECT_EQ(ReadLE16(bytes, 10), 2);
  EXPECT_EQ(bytes[12], 0xCC);
  EXPECT_EQ(bytes[13], 0xDD);
}

// ============================================================================
// Entry Point Tests
// ============================================================================

/**
 * @brief Test with entry point address
 */
TEST_F(TrsDosWriterTest, WithEntryPoint) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("CODE", 0x8000, {0x01, 0x02}));

  writer.SetEntryPoint(0x8000);
  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // Data segment + entry point segment (type 0x02)
  ASSERT_GE(bytes.size(), 12); // (5+2) data + (5+0) entry

  // Find entry point segment (should be last, type 0x02)
  size_t entry_offset = bytes.size() - 5; // Entry segment is 5 bytes (no data)
  EXPECT_EQ(bytes[entry_offset], 0x02);   // Entry point segment type
  EXPECT_EQ(ReadLE16(bytes, entry_offset + 1), 0x8000); // Entry address
  EXPECT_EQ(ReadLE16(bytes, entry_offset + 3), 0);      // Length = 0
}

// ============================================================================
// Format Information Tests
// ============================================================================

/**
 * @brief Test GetExtension returns correct extension
 */
TEST_F(TrsDosWriterTest, GetExtension) {
  EXPECT_EQ(writer.GetExtension(), "cmd");
}

/**
 * @brief Test GetFormatName returns descriptive name
 */
TEST_F(TrsDosWriterTest, GetFormatName) {
  EXPECT_EQ(writer.GetFormatName(), "TRS-DOS Binary");
}

// ============================================================================
// Edge Cases
// ============================================================================

/**
 * @brief Test section with no atoms
 */
TEST_F(TrsDosWriterTest, SectionWithNoAtoms) {
  std::vector<Section> sections;
  Section empty_section("EMPTY", 0, 0x8000);
  sections.push_back(empty_section);

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();
  EXPECT_EQ(bytes.size(), 0); // No data = no output
}

/**
 * @brief Test section with space atom (should skip)
 */
TEST_F(TrsDosWriterTest, SectionWithSpaceAtom) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x8000);
  auto space_atom = std::make_shared<SpaceAtom>(100);
  section.atoms.push_back(space_atom);
  sections.push_back(section);

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();
  EXPECT_EQ(bytes.size(), 0); // Space atoms don't produce output
}

/**
 * @brief Test mixed atoms (data + space + data creates separate segments)
 */
TEST_F(TrsDosWriterTest, MixedAtomsWithSpace) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x8000);

  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));
  section.atoms.push_back(std::make_shared<SpaceAtom>(10));
  section.atoms.push_back(
      std::make_shared<DataAtom>(std::vector<uint8_t>{0xBB}));

  sections.push_back(section);

  writer.Write(sections, output);

  auto bytes = GetOutputBytes();

  // Should create two segments due to address gap
  ASSERT_GE(bytes.size(), 12); // (5+1) + (5+1)

  // First segment: 0xAA at 0x8000
  EXPECT_EQ(bytes[0], 0x01);
  EXPECT_EQ(ReadLE16(bytes, 1), 0x8000);
  EXPECT_EQ(ReadLE16(bytes, 3), 1);
  EXPECT_EQ(bytes[5], 0xAA);

  // Second segment: 0xBB at 0x800B (0x8000 + 1 + 10)
  EXPECT_EQ(bytes[6], 0x01);
  EXPECT_EQ(ReadLE16(bytes, 7), 0x800B);
  EXPECT_EQ(ReadLE16(bytes, 9), 1);
  EXPECT_EQ(bytes[11], 0xBB);
}
