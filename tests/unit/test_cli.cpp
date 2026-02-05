// CLI argument parsing tests
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "CLI/CLI.hpp"
#include "xasm++/cli/command_line_options.h"
#include <gtest/gtest.h>

// Forward declaration of function to test (in xasm namespace)
namespace xasm {
CommandLineOptions ParseCommandLine(int argc, char **argv);
}

using namespace xasm;

// Test 1: Valid input file with defaults
TEST(CLITest, ValidInputFile) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.input_file, "test.asm");
  EXPECT_EQ(opts.cpu, "6502");
  EXPECT_EQ(opts.syntax, "simple");
  EXPECT_EQ(opts.output, "a.out");
  EXPECT_FALSE(opts.show_help);
  EXPECT_FALSE(opts.show_version);
}

// Test 2: --cpu option
TEST(CLITest, CPUOption) {
  const char *argv[] = {"xasm++", "test.asm", "--cpu", "6502"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.input_file, "test.asm");
  EXPECT_EQ(opts.cpu, "6502");
}

// Test 3: Invalid CPU value should throw exception
TEST(CLITest, InvalidCPU) {
  const char *argv[] = {"xasm++", "test.asm", "--cpu", "invalid"};
  int argc = 4;

  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::ValidationError);
}

// Test 4: --syntax option
TEST(CLITest, SyntaxOption) {
  const char *argv[] = {"xasm++", "test.asm", "--syntax", "simple"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.input_file, "test.asm");
  EXPECT_EQ(opts.syntax, "simple");
}

// Test 5: --output option
TEST(CLITest, OutputOption) {
  const char *argv[] = {"xasm++", "test.asm", "--output", "out.bin"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.input_file, "test.asm");
  EXPECT_EQ(opts.output, "out.bin");
}

// Test 6: --help flag
TEST(CLITest, HelpFlag) {
  const char *argv[] = {"xasm++", "--help"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.show_help);
}

// Test 7: --version flag
TEST(CLITest, VersionFlag) {
  const char *argv[] = {"xasm++", "--version"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.show_version);
}

// Test 8: Missing input file should throw exception
TEST(CLITest, MissingInputFile) {
  const char *argv[] = {"xasm++"};
  int argc = 1;

  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::RequiredError);
}

// Test 9: Multiple errors (missing input + invalid CPU)
TEST(CLITest, MultipleErrors) {
  const char *argv[] = {"xasm++", "--cpu", "invalid"};
  int argc = 3;

  // CLI11 reports first error encountered (ValidationError for invalid CPU)
  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::ValidationError);
}
