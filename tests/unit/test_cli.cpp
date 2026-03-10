// CLI argument parsing tests
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "CLI/CLI.hpp"
#include "xasm++/assembler.h"
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

// Test 8: Missing input file sets show_help=true (not an exception)
TEST(CLITest, MissingInputFile) {
  const char *argv[] = {"xasm++"};
  int argc = 1;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  // When no input file is given, parser shows help rather than throwing
  EXPECT_TRUE(opts.show_help);
  EXPECT_EQ(opts.input_file, "");
}

// Test 9: Multiple errors (missing input + invalid CPU)
TEST(CLITest, MultipleErrors) {
  const char *argv[] = {"xasm++", "--cpu", "invalid"};
  int argc = 3;

  // CLI11 reports first error encountered (ValidationError for invalid CPU)
  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::ValidationError);
}

// ─── New option tests ────────────────────────────────────────────────────────

// Test 10: --include (-I) short option (renamed from --include-path)
TEST(CLITest, IncludeShortAlias) {
  const char *argv[] = {"xasm++", "test.asm", "-I", "/usr/include"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.include.size(), 1u);
  EXPECT_EQ(opts.include[0], "/usr/include");
}

// Test 11: --include long form
TEST(CLITest, IncludeLongOption) {
  const char *argv[] = {"xasm++", "test.asm", "--include", "/sys/inc"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.include.size(), 1u);
  EXPECT_EQ(opts.include[0], "/sys/inc");
}

// Test 12: Multiple --include paths
TEST(CLITest, MultipleIncludePaths) {
  const char *argv[] = {"xasm++",    "test.asm", "-I", "/inc1",
                        "--include", "/inc2",    "-I", "/inc3"};
  int argc = 8;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.include.size(), 3u);
  EXPECT_EQ(opts.include[0], "/inc1");
  EXPECT_EQ(opts.include[1], "/inc2");
  EXPECT_EQ(opts.include[2], "/inc3");
}

// Regression test: -I before input file must NOT consume the input filename.
// Previously, CLI11 treated -I as a greedy multi-value option and consumed
// the positional input file as a second include path, leaving no input file
// and causing xasm++ to print help and exit 0 silently (e.g. in A2osX cmake
// build: xasm++ --cpu 65c02 --syntax scmasm -I /stage usr/src/sys/kernel.s).
TEST(CLITest, IncludeDoesNotConsumeInputFile) {
  const char *argv[] = {"xasm++", "-I", "/stage", "usr/src/sys/kernel.s"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  // -I must have captured only the one explicit path
  ASSERT_EQ(opts.include.size(), 1u);
  EXPECT_EQ(opts.include[0], "/stage");
  // The input file must not have been consumed by -I
  EXPECT_EQ(opts.input_file, "usr/src/sys/kernel.s");
}


TEST(CLITest, DefineShortAlias) {
  const char *argv[] = {"xasm++", "test.asm", "-D", "DEBUG"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.define.size(), 1u);
  EXPECT_EQ(opts.define[0], "DEBUG");
}

// Test 14: --define long form with value
TEST(CLITest, DefineLongOptionWithValue) {
  const char *argv[] = {"xasm++", "test.asm", "--define", "VERSION=1"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.define.size(), 1u);
  EXPECT_EQ(opts.define[0], "VERSION=1");
}

// Test 15: Multiple -D defines
TEST(CLITest, MultipleDefines) {
  const char *argv[] = {"xasm++",   "test.asm", "-D",        "FOO",
                        "--define", "BAR=2",    "--define",  "BAZ"};
  int argc = 8;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  ASSERT_EQ(opts.define.size(), 3u);
  EXPECT_EQ(opts.define[0], "FOO");
  EXPECT_EQ(opts.define[1], "BAR=2");
  EXPECT_EQ(opts.define[2], "BAZ");
}

// Test 16: --warn defaults to level 1
TEST(CLITest, WarnDefaultLevel) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.warn, 1);
}

// Test 17: --no-warn sets warn level to 0
TEST(CLITest, NoWarnDisablesWarnings) {
  const char *argv[] = {"xasm++", "test.asm", "--no-warn"};
  int argc = 3;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.warn, 0);
}

// Test 18: --warn 2 sets extra warning level
TEST(CLITest, WarnLevelExtra) {
  const char *argv[] = {"xasm++", "test.asm", "--warn", "2"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.warn, 2);
}

// Test 19: --werror enables treat-warnings-as-errors
TEST(CLITest, WerrorOption) {
  const char *argv[] = {"xasm++", "test.asm", "--werror"};
  int argc = 3;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.werror);
  EXPECT_GT(opts.warn, 0); // werror doesn't disable warn
}

// Test 20: --quiet / -q suppresses non-essential output
TEST(CLITest, QuietShortAlias) {
  const char *argv[] = {"xasm++", "test.asm", "-q"};
  int argc = 3;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.quiet);
}

// Test 21: --quiet long form
TEST(CLITest, QuietLongOption) {
  const char *argv[] = {"xasm++", "test.asm", "--quiet"};
  int argc = 3;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.quiet);
}

// Test 22: quiet defaults to false
TEST(CLITest, QuietDefaultFalse) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_FALSE(opts.quiet);
}

// Test 23: --verbose enables verbose output
TEST(CLITest, VerboseOption) {
  const char *argv[] = {"xasm++", "test.asm", "--verbose"};
  int argc = 3;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.verbose);
}

// Test 24: verbose defaults to false
TEST(CLITest, VerboseDefaultFalse) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_FALSE(opts.verbose);
}

// Test 25: --org sets origin address
TEST(CLITest, OrgOption) {
  const char *argv[] = {"xasm++", "test.asm", "--org", "0x0800"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.org, 0x0800u);
}

// Test 26: --org with decimal value
TEST(CLITest, OrgDecimalValue) {
  const char *argv[] = {"xasm++", "test.asm", "--org", "2048"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.org, 2048u);
}

// Test 27: --org defaults to -1 (sentinel meaning "not set")
TEST(CLITest, OrgDefaultNotSet) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.org, -1);
}

// Test 28: --format binary
TEST(CLITest, FormatBinary) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "binary"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.format, OutputFormat::Binary);
}

// Test 29: --format ihex
TEST(CLITest, FormatIHex) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "ihex"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.format, OutputFormat::IntelHex);
}

// Test 30: --format srec
TEST(CLITest, FormatSRec) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "srec"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.format, OutputFormat::SRecord);
}

// Test 31: --format defaults to binary
TEST(CLITest, FormatDefaultBinary) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.format, OutputFormat::Binary);
}

// Test 32: Invalid --format value should throw
TEST(CLITest, InvalidFormat) {
  const char *argv[] = {"xasm++", "test.asm", "--format", "elf"};
  int argc = 4;

  EXPECT_THROW(ParseCommandLine(argc, const_cast<char **>(argv)),
               CLI::ValidationError);
}

// Test 33: --max-passes sets the maximum number of assembly passes
TEST(CLITest, MaxPassesOption) {
  const char *argv[] = {"xasm++", "test.asm", "--max-passes", "5"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.max_passes, 5);
}

// Test 34: --max-passes defaults to 10
TEST(CLITest, MaxPassesDefault) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.max_passes, Assembler::MAX_PASSES);
}

// Test 35: --label-map specifies output file for label map
TEST(CLITest, LabelMapOption) {
  const char *argv[] = {"xasm++", "test.asm", "--label-map", "labels.sym"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.label_map, "labels.sym");
}

// Test 36: --label-map defaults to empty string
TEST(CLITest, LabelMapDefault) {
  const char *argv[] = {"xasm++", "test.asm"};
  int argc = 2;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.label_map, "");
}

// ─── Combined option tests ───────────────────────────────────────────────────

// Test 37: Combined options: quiet + define + include
TEST(CLITest, CombinedOptions) {
  const char *argv[] = {"xasm++", "test.asm", "-q", "-D", "DEBUG",
                        "-I",     "/inc",      "--format", "ihex"};
  int argc = 9;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.quiet);
  ASSERT_EQ(opts.define.size(), 1u);
  EXPECT_EQ(opts.define[0], "DEBUG");
  ASSERT_EQ(opts.include.size(), 1u);
  EXPECT_EQ(opts.include[0], "/inc");
  EXPECT_EQ(opts.format, OutputFormat::IntelHex);
}

// Test 38: werror and no-warn can coexist
TEST(CLITest, WerrorAndNoWarn) {
  const char *argv[] = {"xasm++", "test.asm", "--no-warn", "--werror"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.warn, 0);
  EXPECT_TRUE(opts.werror);
}

// Test 39: --org with zero value stays zero
TEST(CLITest, OrgZeroValue) {
  const char *argv[] = {"xasm++", "test.asm", "--org", "0"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_EQ(opts.org, 0u);
}

// Test 40: --verbose and --quiet can coexist (behavior controlled by main.cpp)
TEST(CLITest, VerboseAndQuiet) {
  const char *argv[] = {"xasm++", "test.asm", "--verbose", "--quiet"};
  int argc = 4;

  CommandLineOptions opts = ParseCommandLine(argc, const_cast<char **>(argv));

  EXPECT_TRUE(opts.verbose);
  EXPECT_TRUE(opts.quiet);
}
