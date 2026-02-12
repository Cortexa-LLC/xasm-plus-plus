/**
 * @file test_srec_writer.cpp
 * @brief Unit tests for S-Record format writer
 *
 * Tests for the SRecordWriter class, covering:
 * - S19 format (16-bit addresses)
 * - S28 format (24-bit addresses)
 * - S37 format (32-bit addresses)
 * - Automatic format selection
 * - Checksum validation
 * - Header records (S0)
 * - Data records (S1/S2/S3)
 * - Count records (S5/S6)
 * - Start address records (S7/S8/S9)
 *
 * @note Motorola S-Record format specification:
 *       https://en.wikipedia.org/wiki/SREC_(file_format)
 */

#include "xasm++/output/srec_writer.h"
#include "xasm++/atom.h"
#include "xasm++/section.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

/**
 * @brief Test fixture for S-Record writer tests
 */
class SRecordWriterTest : public ::testing::Test {
protected:
  SRecordWriter writer;
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
   * @brief Parse and validate a single S-Record
   *
   * S-Record format: STLLAA...AADD...DDCC
   * S - Record type (S0-S9)
   * T - Record type digit
   * LL - Byte count (address + data + checksum)
   * AA...AA - Address (4/6/8 hex digits depending on type)
   * DD...DD - Data bytes
   * CC - Checksum (one's complement)
   */
  bool ValidateSRecord(const std::string &record, char &type_char,
                       int &type_num, uint8_t &byte_count, uint64_t &address,
                       std::vector<uint8_t> &data, uint8_t &checksum) {
    if (record.length() < 4 || record[0] != 'S') {
      return false;
    }

    type_char = record[0];
    type_num = record[1] - '0';

    // Extract byte count
    byte_count = std::stoi(record.substr(2, 2), nullptr, 16);

    // Determine address size based on record type
    size_t addr_size = 0;
    if (type_num == 0 || type_num == 1 || type_num == 5 || type_num == 9) {
      addr_size = 4; // 16-bit address
    } else if (type_num == 2 || type_num == 6 || type_num == 8) {
      addr_size = 6; // 24-bit address
    } else if (type_num == 3 || type_num == 7) {
      addr_size = 8; // 32-bit address
    } else {
      return false; // Invalid type
    }

    // Extract address
    address = std::stoull(record.substr(4, addr_size), nullptr, 16);

    // Extract data bytes
    data.clear();
    size_t data_start = 4 + addr_size;
    size_t data_bytes = byte_count - (addr_size / 2) - 1; // Subtract address and checksum
    
    if (record.length() != data_start + data_bytes * 2 + 2) {
      return false;
    }

    for (size_t i = 0; i < data_bytes; ++i) {
      data.push_back(std::stoi(record.substr(data_start + i * 2, 2), nullptr, 16));
    }

    // Extract checksum
    checksum = std::stoi(record.substr(data_start + data_bytes * 2, 2), nullptr, 16);

    // Verify checksum (one's complement of sum of count + address + data)
    uint8_t calculated_checksum = byte_count;
    
    // Add address bytes
    for (size_t i = 0; i < addr_size / 2; ++i) {
      calculated_checksum += (address >> (8 * (addr_size / 2 - 1 - i))) & 0xFF;
    }
    
    // Add data bytes
    for (uint8_t byte : data) {
      calculated_checksum += byte;
    }
    
    calculated_checksum = ~calculated_checksum & 0xFF;

    return calculated_checksum == checksum;
  }

  /**
   * @brief Split output into individual lines
   */
  std::vector<std::string> SplitLines(const std::string &text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
      if (!line.empty()) {
        lines.push_back(line);
      }
    }
    return lines;
  }
};

// ============================================================================
// Basic Format Tests - S19 (16-bit addresses)
// ============================================================================

/**
 * @brief Test empty output generates header and terminator only
 */
TEST_F(SRecordWriterTest, EmptyOutput) {
  std::vector<Section> sections;

  writer.Write(sections, output);

  std::string result = output.str();
  auto lines = SplitLines(result);

  ASSERT_EQ(lines.size(), 2); // Header (S0) + Terminator (S9)
  EXPECT_EQ(lines[0][0], 'S');
  EXPECT_EQ(lines[0][1], '0'); // Header record
  EXPECT_EQ(lines[1][0], 'S');
  EXPECT_EQ(lines[1][1], '9'); // Terminator for S19
}

/**
 * @brief Test single byte at address 0x0000 (S19 format)
 */
TEST_F(SRecordWriterTest, SingleByteAtZero_S19) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, {0x42}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 3); // Header + Data + Terminator

  // Validate data record (should be S1)
  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(ValidateSRecord(lines[1], type_char, type_num, byte_count,
                               address, data, checksum));
  EXPECT_EQ(type_num, 1); // S1 record
  EXPECT_EQ(address, 0x0000);
  EXPECT_EQ(data.size(), 1);
  EXPECT_EQ(data[0], 0x42);
}

/**
 * @brief Test multiple bytes in S19 format
 */
TEST_F(SRecordWriterTest, MultipleBytes_S19) {
  std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x1000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 3);

  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(ValidateSRecord(lines[1], type_char, type_num, byte_count,
                               address, data, checksum));
  EXPECT_EQ(type_num, 1);
  EXPECT_EQ(address, 0x1000);
  EXPECT_EQ(data, test_data);
}

// ============================================================================
// S28 Format Tests (24-bit addresses)
// ============================================================================

/**
 * @brief Test automatic S28 format selection for addresses >= 64KB
 */
TEST_F(SRecordWriterTest, AutoSelectS28Format) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x10000, {0xAA, 0xBB}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 3);

  // Data record should be S2
  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(ValidateSRecord(lines[1], type_char, type_num, byte_count,
                               address, data, checksum));
  EXPECT_EQ(type_num, 2); // S2 record
  EXPECT_EQ(address, 0x10000);
  EXPECT_EQ(data.size(), 2);
  EXPECT_EQ(data[0], 0xAA);
  EXPECT_EQ(data[1], 0xBB);

  // Terminator should be S8
  ASSERT_TRUE(ValidateSRecord(lines[lines.size() - 1], type_char, type_num,
                               byte_count, address, data, checksum));
  EXPECT_EQ(type_num, 8); // S8 terminator for S28
}

// ============================================================================
// S37 Format Tests (32-bit addresses)
// ============================================================================

/**
 * @brief Test automatic S37 format selection for addresses >= 16MB
 */
TEST_F(SRecordWriterTest, AutoSelectS37Format) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x1000000, {0xCC, 0xDD}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 3);

  // Data record should be S3
  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(ValidateSRecord(lines[1], type_char, type_num, byte_count,
                               address, data, checksum));
  EXPECT_EQ(type_num, 3); // S3 record
  EXPECT_EQ(address, 0x1000000);
  EXPECT_EQ(data.size(), 2);
  EXPECT_EQ(data[0], 0xCC);
  EXPECT_EQ(data[1], 0xDD);

  // Terminator should be S7
  ASSERT_TRUE(ValidateSRecord(lines[lines.size() - 1], type_char, type_num,
                               byte_count, address, data, checksum));
  EXPECT_EQ(type_num, 7); // S7 terminator for S37
}

// ============================================================================
// Header Record Tests
// ============================================================================

/**
 * @brief Test header record (S0) format
 */
TEST_F(SRecordWriterTest, HeaderRecord) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, {0x00}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 1);

  // First line should be S0 header
  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(ValidateSRecord(lines[0], type_char, type_num, byte_count,
                               address, data, checksum));
  EXPECT_EQ(type_num, 0); // S0 record
  EXPECT_EQ(address, 0x0000); // Typically zero for header
}

// ============================================================================
// Checksum Tests
// ============================================================================

/**
 * @brief Test checksum calculation for various records
 */
TEST_F(SRecordWriterTest, ChecksumValidation) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("TEST", 0xABCD, {0x12, 0x34, 0x56, 0x78}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());

  // All records should have valid checksums
  for (const auto &line : lines) {
    char type_char;
    int type_num;
    uint8_t byte_count, checksum;
    uint64_t address;
    std::vector<uint8_t> data;

    ASSERT_TRUE(ValidateSRecord(line, type_char, type_num, byte_count,
                                address, data, checksum))
        << "Invalid checksum in record: " << line;
  }
}

// ============================================================================
// Bytes Per Line Configuration Tests
// ============================================================================

/**
 * @brief Test custom bytes per line setting (16 bytes)
 */
TEST_F(SRecordWriterTest, CustomBytesPerLine_16) {
  writer.SetBytesPerLine(16);

  std::vector<uint8_t> test_data(48); // Should produce 3 records
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  // Header + 3 data records + Terminator
  ASSERT_EQ(lines.size(), 5);

  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  // Check data records
  for (size_t i = 1; i <= 3; ++i) {
    ASSERT_TRUE(ValidateSRecord(lines[i], type_char, type_num, byte_count,
                                address, data, checksum));
    EXPECT_EQ(data.size(), 16) << "Record " << i << " should have 16 bytes";
  }
}

// ============================================================================
// Format Information Tests
// ============================================================================

/**
 * @brief Test GetExtension returns correct extension
 */
TEST_F(SRecordWriterTest, GetExtension) {
  EXPECT_EQ(writer.GetExtension(), "s19");
}

/**
 * @brief Test GetFormatName returns descriptive name
 */
TEST_F(SRecordWriterTest, GetFormatName) {
  EXPECT_EQ(writer.GetFormatName(), "Motorola S-Record");
}

// ============================================================================
// Multiple Sections Tests
// ============================================================================

/**
 * @brief Test multiple sections with mixed address ranges
 */
TEST_F(SRecordWriterTest, MultipleSectionsWithMixedAddressRanges) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("LOW", 0x0000, {0x11}));
  sections.push_back(CreateSectionWithData("HIGH", 0xFFFF, {0x22}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 4); // Header + 2 data + Terminator

  char type_char;
  int type_num;
  uint8_t byte_count, checksum;
  uint64_t address;
  std::vector<uint8_t> data;

  // Verify all records are valid
  for (const auto &line : lines) {
    ASSERT_TRUE(ValidateSRecord(line, type_char, type_num, byte_count,
                                address, data, checksum))
        << "Invalid record: " << line;
  }
}

// ============================================================================
// Edge Cases
// ============================================================================

/**
 * @brief Test section with no atoms
 */
TEST_F(SRecordWriterTest, SectionWithNoAtoms) {
  std::vector<Section> sections;
  Section empty_section("EMPTY", 0, 0x1000);
  sections.push_back(empty_section);

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 2); // Header + Terminator only
}

/**
 * @brief Test section with space atom (should skip)
 */
TEST_F(SRecordWriterTest, SectionWithSpaceAtom) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x0000);

  auto space_atom = std::make_shared<SpaceAtom>(100);
  section.atoms.push_back(space_atom);

  sections.push_back(section);

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 2); // Header + Terminator only
}
