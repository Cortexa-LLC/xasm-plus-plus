/**
 * @file test_intel_hex_writer.cpp
 * @brief Unit tests for Intel HEX format writer
 *
 * Tests for the IntelHexWriter class, covering:
 * - Data records (type 00)
 * - EOF record (type 01)
 * - Extended linear address records (type 04)
 * - Extended segment address records (type 05)
 * - Checksum validation
 * - Various bytes-per-line configurations
 *
 * @note Intel HEX format specification:
 *       https://en.wikipedia.org/wiki/Intel_HEX
 */

#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/atom.h"
#include "xasm++/section.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace xasm;

/**
 * @brief Test fixture for Intel HEX writer tests
 */
class IntelHexWriterTest : public ::testing::Test {
protected:
  IntelHexWriter writer;
  std::ostringstream output;

  /**
   * @brief Create a section with data atoms
   *
   * Helper to create a section containing specified bytes at a given address.
   */
  Section CreateSectionWithData(const std::string &name, uint64_t org,
                                const std::vector<uint8_t> &data) {
    Section section(name, static_cast<uint32_t>(SectionAttributes::Data), org);

    // Create a data atom with the provided bytes
    auto data_atom = std::make_shared<DataAtom>(data);
    section.atoms.push_back(data_atom);

    return section;
  }

  /**
   * @brief Create a section with instruction atoms
   *
   * Helper to create a section with encoded instruction bytes.
   */
  Section CreateSectionWithInstructions(const std::string &name, uint64_t org,
                                        const std::vector<uint8_t> &bytes) {
    Section section(name,
                    static_cast<uint32_t>(SectionAttributes::Code) |
                        static_cast<uint32_t>(SectionAttributes::Execute),
                    org);

    // Create an instruction atom with encoded bytes
    auto inst_atom = std::make_shared<InstructionAtom>("LDA", "#$42");
    inst_atom->encoded_bytes = bytes;
    inst_atom->size = bytes.size();
    section.atoms.push_back(inst_atom);

    return section;
  }

  /**
   * @brief Parse and validate a single Intel HEX record
   *
   * Returns true if the record is valid, false otherwise.
   * Also extracts fields for verification.
   */
  bool ValidateIntelHexRecord(const std::string &record, uint8_t &byte_count,
                              uint16_t &address, uint8_t &record_type,
                              std::vector<uint8_t> &data,
                              uint8_t &checksum) {
    // Intel HEX format: :LLAAAATTDD...DDCC
    // : - start code
    // LL - byte count (2 hex digits)
    // AAAA - address (4 hex digits)
    // TT - record type (2 hex digits)
    // DD...DD - data bytes
    // CC - checksum (2 hex digits)

    if (record.empty() || record[0] != ':') {
      return false;
    }

    if (record.length() < 11) { // Minimum: :00000001FF (EOF record)
      return false;
    }

    // Extract fields
    byte_count = std::stoi(record.substr(1, 2), nullptr, 16);
    address = std::stoi(record.substr(3, 4), nullptr, 16);
    record_type = std::stoi(record.substr(7, 2), nullptr, 16);

    // Extract data bytes
    data.clear();
    size_t expected_length = 11 + byte_count * 2;
    if (record.length() != expected_length) {
      return false;
    }

    for (size_t i = 0; i < byte_count; ++i) {
      data.push_back(std::stoi(record.substr(9 + i * 2, 2), nullptr, 16));
    }

    // Extract checksum
    checksum = std::stoi(record.substr(9 + byte_count * 2, 2), nullptr, 16);

    // Verify checksum
    uint8_t calculated_checksum = byte_count;
    calculated_checksum += (address >> 8) & 0xFF;
    calculated_checksum += address & 0xFF;
    calculated_checksum += record_type;
    for (uint8_t byte : data) {
      calculated_checksum += byte;
    }
    calculated_checksum = (~calculated_checksum + 1) & 0xFF;

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
// Basic Format Tests
// ============================================================================

/**
 * @brief Test empty output generates only EOF record
 */
TEST_F(IntelHexWriterTest, EmptyOutput) {
  std::vector<Section> sections;

  writer.Write(sections, output);

  std::string result = output.str();
  auto lines = SplitLines(result);

  ASSERT_EQ(lines.size(), 1);
  EXPECT_EQ(lines[0], ":00000001FF"); // EOF record
}

/**
 * @brief Test single byte at address 0x0000
 */
TEST_F(IntelHexWriterTest, SingleByteAtZero) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, {0x42}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 2); // At least data + EOF

  // Validate data record
  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 1);
  EXPECT_EQ(address, 0x0000);
  EXPECT_EQ(record_type, 0x00); // Data record
  EXPECT_EQ(data.size(), 1);
  EXPECT_EQ(data[0], 0x42);

  // Validate EOF record
  EXPECT_EQ(lines[lines.size() - 1], ":00000001FF");
}

/**
 * @brief Test multiple bytes in a single record
 */
TEST_F(IntelHexWriterTest, MultipleBytes) {
  std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x1000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 2);

  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 5);
  EXPECT_EQ(address, 0x1000);
  EXPECT_EQ(record_type, 0x00);
  EXPECT_EQ(data, test_data);
}

/**
 * @brief Test data spanning multiple 16-byte records
 */
TEST_F(IntelHexWriterTest, MultipleRecords) {
  // Create 48 bytes of data (should produce 3 records of 16 bytes each)
  std::vector<uint8_t> test_data(48);
  for (size_t i = 0; i < test_data.size(); ++i) {
    test_data[i] = static_cast<uint8_t>(i & 0xFF);
  }

  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x2000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 4); // 3 data records + EOF

  // Verify first record
  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 16);
  EXPECT_EQ(address, 0x2000);

  // Verify second record
  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[1], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 16);
  EXPECT_EQ(address, 0x2010);

  // Verify third record
  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[2], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 16);
  EXPECT_EQ(address, 0x2020);
}

// ============================================================================
// Extended Address Tests
// ============================================================================

/**
 * @brief Test extended linear address record (type 04) for addresses > 64K
 */
TEST_F(IntelHexWriterTest, ExtendedLinearAddress) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x10000, {0xAA, 0xBB}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 3); // Extended address + data + EOF

  // First line should be extended linear address record
  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(byte_count, 2);
  EXPECT_EQ(address, 0x0000);
  EXPECT_EQ(record_type, 0x04); // Extended linear address
  EXPECT_EQ(data.size(), 2);
  EXPECT_EQ((data[0] << 8) | data[1], 0x0001); // Upper 16 bits

  // Second line should be data record
  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[1], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(record_type, 0x00); // Data record
  EXPECT_EQ(address, 0x0000);   // Lower 16 bits
}

/**
 * @brief Test multiple sections with different extended addresses
 */
TEST_F(IntelHexWriterTest, MultipleExtendedAddresses) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("LOW", 0x0000, {0x11}));
  sections.push_back(CreateSectionWithData("MED", 0x8000, {0x22}));
  sections.push_back(CreateSectionWithData("HIGH", 0x10000, {0x33}));
  sections.push_back(CreateSectionWithData("HIGHER", 0x20000, {0x44}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());

  // Should have: data (LOW) + data (MED) + ext addr + data (HIGH) + ext addr +
  // data (HIGHER) + EOF
  ASSERT_GE(lines.size(), 7);

  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  // Verify there are extended address records before HIGH and HIGHER sections
  int ext_addr_count = 0;
  for (const auto &line : lines) {
    if (ValidateIntelHexRecord(line, byte_count, address, record_type, data,
                                checksum)) {
      if (record_type == 0x04) {
        ext_addr_count++;
      }
    }
  }

  EXPECT_EQ(ext_addr_count, 2); // One for 0x10000, one for 0x20000
}

// ============================================================================
// Checksum Tests
// ============================================================================

/**
 * @brief Test checksum calculation for various records
 */
TEST_F(IntelHexWriterTest, ChecksumValidation) {
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("TEST", 0xABCD, {0x12, 0x34, 0x56, 0x78}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());

  // All records should have valid checksums
  for (const auto &line : lines) {
    uint8_t byte_count, record_type, checksum;
    uint16_t address;
    std::vector<uint8_t> data;

    ASSERT_TRUE(ValidateIntelHexRecord(line, byte_count, address, record_type,
                                       data, checksum))
        << "Invalid checksum in record: " << line;
  }
}

// ============================================================================
// Bytes Per Line Configuration Tests
// ============================================================================

/**
 * @brief Test custom bytes per line setting (8 bytes)
 */
TEST_F(IntelHexWriterTest, CustomBytesPerLine_8) {
  writer.SetBytesPerLine(8);

  std::vector<uint8_t> test_data(24); // Should produce 3 records
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 4); // 3 data records + EOF

  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  // Each data record should have 8 bytes
  for (size_t i = 0; i < 3; ++i) {
    ASSERT_TRUE(ValidateIntelHexRecord(lines[i], byte_count, address,
                                       record_type, data, checksum));
    EXPECT_EQ(byte_count, 8) << "Record " << i << " should have 8 bytes";
  }
}

/**
 * @brief Test custom bytes per line setting (32 bytes)
 */
TEST_F(IntelHexWriterTest, CustomBytesPerLine_32) {
  writer.SetBytesPerLine(32);

  std::vector<uint8_t> test_data(64); // Should produce 2 records
  std::vector<Section> sections;
  sections.push_back(CreateSectionWithData("DATA", 0x0000, test_data));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 3); // 2 data records + EOF

  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  // Each data record should have 32 bytes
  for (size_t i = 0; i < 2; ++i) {
    ASSERT_TRUE(ValidateIntelHexRecord(lines[i], byte_count, address,
                                       record_type, data, checksum));
    EXPECT_EQ(byte_count, 32) << "Record " << i << " should have 32 bytes";
  }
}

// ============================================================================
// Multiple Sections and Mixed Content Tests
// ============================================================================

/**
 * @brief Test multiple sections with different types
 */
TEST_F(IntelHexWriterTest, MultipleSectionsWithDifferentTypes) {
  std::vector<Section> sections;

  // Code section
  sections.push_back(
      CreateSectionWithInstructions("CODE", 0x8000, {0xA9, 0x42}));

  // Data section
  sections.push_back(CreateSectionWithData("DATA", 0x9000, {0x11, 0x22, 0x33}));

  // Another code section
  sections.push_back(CreateSectionWithInstructions("CODE2", 0xA000, {0x85, 0x00}));

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_GE(lines.size(), 4); // At least 3 data records + EOF

  // Verify we have data from all three sections
  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(address, 0x8000);

  // Note: Can't assume exact line order for other sections without knowing
  // implementation details, but all should have valid checksums and proper format
  for (size_t i = 1; i < lines.size() - 1; ++i) {
    ASSERT_TRUE(ValidateIntelHexRecord(lines[i], byte_count, address,
                                       record_type, data, checksum))
        << "Failed on line " << i << ": " << lines[i];
  }
}

// ============================================================================
// Format Information Tests
// ============================================================================

/**
 * @brief Test GetExtension returns correct extension
 */
TEST_F(IntelHexWriterTest, GetExtension) {
  EXPECT_EQ(writer.GetExtension(), "hex");
}

/**
 * @brief Test GetFormatName returns descriptive name
 */
TEST_F(IntelHexWriterTest, GetFormatName) {
  EXPECT_EQ(writer.GetFormatName(), "Intel HEX");
}

// ============================================================================
// Edge Cases
// ============================================================================

/**
 * @brief Test section with no atoms
 */
TEST_F(IntelHexWriterTest, SectionWithNoAtoms) {
  std::vector<Section> sections;
  Section empty_section("EMPTY", 0, 0x1000);
  sections.push_back(empty_section);

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 1); // Only EOF
  EXPECT_EQ(lines[0], ":00000001FF");
}

/**
 * @brief Test section with space atom (should skip)
 */
TEST_F(IntelHexWriterTest, SectionWithSpaceAtom) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x0000);

  // Add a space atom (uninitialized space - should not generate output)
  auto space_atom = std::make_shared<SpaceAtom>(100);
  section.atoms.push_back(space_atom);

  sections.push_back(section);

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());
  ASSERT_EQ(lines.size(), 1); // Only EOF
  EXPECT_EQ(lines[0], ":00000001FF");
}

/**
 * @brief Test mixed atoms (data + space + data)
 */
TEST_F(IntelHexWriterTest, MixedAtomsWithSpace) {
  std::vector<Section> sections;
  Section section("TEST", 0, 0x0000);

  // Data, space, more data
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xAA}));
  section.atoms.push_back(std::make_shared<SpaceAtom>(10));
  section.atoms.push_back(std::make_shared<DataAtom>(std::vector<uint8_t>{0xBB}));

  sections.push_back(section);

  writer.Write(sections, output);

  auto lines = SplitLines(output.str());

  // Should have at least 2 data records (potentially 3 if space causes address gap)
  // and EOF
  ASSERT_GE(lines.size(), 3);

  uint8_t byte_count, record_type, checksum;
  uint16_t address;
  std::vector<uint8_t> data;

  // First record should have 0xAA at address 0
  ASSERT_TRUE(
      ValidateIntelHexRecord(lines[0], byte_count, address, record_type, data, checksum));
  EXPECT_EQ(address, 0x0000);
  EXPECT_EQ(data[0], 0xAA);

  // Last data record should have 0xBB at address 11 (after 1 byte + 10 space)
  ASSERT_TRUE(ValidateIntelHexRecord(lines[lines.size() - 2], byte_count,
                                     address, record_type, data, checksum));
  EXPECT_EQ(address, 0x000B); // 11 decimal = 0x0B
  EXPECT_EQ(data[0], 0xBB);
}
