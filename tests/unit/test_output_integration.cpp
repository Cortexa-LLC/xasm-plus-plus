/**
 * @file test_output_integration.cpp
 * @brief Integration tests for output format system with CLI
 *
 * Tests the integration between CLI options and output format writers,
 * ensuring all formats can be selected and instantiated correctly.
 *
 * @note Part of Output Formats Phase 3: CLI Integration
 */

#include <gtest/gtest.h>
#include <sstream>

#include "CLI/CLI.hpp"
#include "xasm++/cli/command_line_options.h"
#include "xasm++/output/binary_output.h"
#include "xasm++/output/coco_loadm_writer.h"
#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/output/listing_output.h"
#include "xasm++/output/output_writer.h"
#include "xasm++/output/srec_writer.h"
#include "xasm++/output/symbol_output.h"
#include "xasm++/output/trsdos_writer.h"
#include "xasm++/section.h"

// Forward declaration
namespace xasm {
CommandLineOptions ParseCommandLine(int argc, char **argv);
} // namespace xasm

using namespace xasm;

/**
 * @brief Test fixture for output format integration tests
 */
class OutputIntegrationTest : public ::testing::Test {
protected:
  /**
   * @brief Create a simple test section with data
   * @return Section with test data
   */
  Section CreateTestSection() {
    Section section;
    section.name = "CODE";
    section.org = 0x8000;

    // Create a simple instruction atom
    auto inst = std::make_unique<InstructionAtom>("LDA", "#$42");
    inst->encoded_bytes = {0xA9, 0x42};
    inst->size = 2;
    section.atoms.push_back(std::move(inst));

    return section;
  }
};

// ===========================================================================
// CLI Output Format Option Tests
// ===========================================================================

/**
 * @test Default output format should be binary
 */
TEST_F(OutputIntegrationTest, DefaultFormatIsBinary) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "bin");
}

/**
 * @test --format option should accept valid format names
 */
TEST_F(OutputIntegrationTest, FormatOption) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "hex"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "hex");
}

/**
 * @test --format should accept Intel HEX format
 */
TEST_F(OutputIntegrationTest, FormatIntelHex) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "hex"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "hex");
}

/**
 * @test --format should accept S-Record formats
 */
TEST_F(OutputIntegrationTest, FormatSRecord) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "srec"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "srec");
}

/**
 * @test --format should accept binary format
 */
TEST_F(OutputIntegrationTest, FormatBinary) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "bin"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "bin");
}

/**
 * @test --format should accept CoCo LOADM format
 */
TEST_F(OutputIntegrationTest, FormatCoCoLoadm) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "coco"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "coco");
}

/**
 * @test --format should accept TRS-DOS CMD format
 */
TEST_F(OutputIntegrationTest, FormatTRSDOS) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "trsdos"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "trsdos");
}

/**
 * @test Invalid format should throw exception
 */
TEST_F(OutputIntegrationTest, InvalidFormat) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "invalid"};
  int argc = 4;

  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::ValidationError);
}

// ===========================================================================
// Output Format Writer Integration Tests
// ===========================================================================

/**
 * @test Intel HEX writer should be instantiable
 */
TEST_F(OutputIntegrationTest, IntelHexWriterInstantiation) {
  IntelHexWriter writer;
  EXPECT_EQ(writer.GetFormatName(), "Intel HEX");
  EXPECT_EQ(writer.GetExtension(), "hex");
}

/**
 * @test S-Record writer should be instantiable
 */
TEST_F(OutputIntegrationTest, SRecordWriterInstantiation) {
  SRecordWriter writer;
  EXPECT_EQ(writer.GetFormatName(), "Motorola S-Record");
  EXPECT_EQ(writer.GetExtension(), "s19");
}

/**
 * @test CoCo LOADM writer should be instantiable
 */
TEST_F(OutputIntegrationTest, CoCoLoadmWriterInstantiation) {
  CocoLoadmWriter writer;
  EXPECT_EQ(writer.GetFormatName(), "CoCo DOS (LOADM)");
  EXPECT_EQ(writer.GetExtension(), "bin");
}

/**
 * @test TRS-DOS writer should be instantiable
 */
TEST_F(OutputIntegrationTest, TRSDOSWriterInstantiation) {
  TrsDosWriter writer;
  EXPECT_EQ(writer.GetFormatName(), "TRS-DOS Binary");
  EXPECT_EQ(writer.GetExtension(), "cmd");
}

/**
 * @test All writers should write to output stream without errors
 */
TEST_F(OutputIntegrationTest, AllWritersCanWrite) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  // Test Intel HEX writer
  {
    IntelHexWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(sections, output));
    EXPECT_FALSE(output.str().empty());
  }

  // Test S-Record writer (S19)
  {
    SRecordWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(sections, output));
    EXPECT_FALSE(output.str().empty());
  }

  // Test CoCo LOADM writer
  {
    CocoLoadmWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(sections, output));
    EXPECT_FALSE(output.str().empty());
  }

  // Test TRS-DOS writer
  {
    TrsDosWriter writer;
    std::ostringstream output;
    EXPECT_NO_THROW(writer.Write(sections, output));
    EXPECT_FALSE(output.str().empty());
  }
}

// ===========================================================================
// Listing and Symbol Output Tests
// ===========================================================================

/**
 * @test Listing file option should be stored
 */
TEST_F(OutputIntegrationTest, ListingFileOption) {
  const char *argv[] = {"xasm++", "test.asm", "--list", "output.lst"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.listing_file, "output.lst");
}

/**
 * @test Symbol file option should be stored
 */
TEST_F(OutputIntegrationTest, SymbolFileOption) {
  const char *argv[] = {"xasm++", "test.asm", "--symbols", "output.sym"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.symbol_file, "output.sym");
}

/**
 * @test Multiple output options can be combined
 */
TEST_F(OutputIntegrationTest, MultipleOutputOptions) {
  const char *argv[] = {"xasm++",   "test.asm",   "--format",  "hex",
                        "--list",   "output.lst", "--symbols", "output.sym",
                        "--output", "output.hex"};
  int argc = 10;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.output_format, "hex");
  EXPECT_EQ(opts.listing_file, "output.lst");
  EXPECT_EQ(opts.symbol_file, "output.sym");
  EXPECT_EQ(opts.output, "output.hex");
}

// ===========================================================================
// Format-specific output tests
// ===========================================================================

/**
 * @test Intel HEX output should be properly formatted
 */
TEST_F(OutputIntegrationTest, IntelHexFormatted) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  IntelHexWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);

  std::string result = output.str();

  // Should start with :02 (2 bytes of data)
  EXPECT_TRUE(result.find(":02") != std::string::npos);
  // Should contain A942 (LDA #$42)
  EXPECT_TRUE(result.find("A942") != std::string::npos);
  // Should end with EOF record
  EXPECT_TRUE(result.find(":00000001FF") != std::string::npos);
}

/**
 * @test S-Record output should be properly formatted
 */
TEST_F(OutputIntegrationTest, SRecordFormatted) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  SRecordWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);

  std::string result = output.str();

  // Should contain S1 record (16-bit address)
  EXPECT_TRUE(result.find("S1") != std::string::npos);
  // Should contain 8000 address
  EXPECT_TRUE(result.find("8000") != std::string::npos);
  // Should end with S9 record
  EXPECT_TRUE(result.find("S9") != std::string::npos);
}

/**
 * @test CoCo LOADM output should contain header and data
 */
TEST_F(OutputIntegrationTest, CoCoLoadmFormatted) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  CocoLoadmWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);

  std::string result = output.str();

  // Should not be empty
  EXPECT_FALSE(result.empty());
  // Should contain binary data
  EXPECT_GT(result.size(), 5); // At least header + some data
}

/**
 * @test TRS-DOS CMD output should contain header and data
 */
TEST_F(OutputIntegrationTest, TRSDOSFormatted) {
  Section section = CreateTestSection();
  std::vector<Section> sections = {section};

  TrsDosWriter writer;
  std::ostringstream output;
  writer.Write(sections, output);

  std::string result = output.str();

  // Should not be empty
  EXPECT_FALSE(result.empty());
  // Should contain binary data with header
  EXPECT_GT(result.size(), 5);
}
