// CLI argument parser
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "CLI/CLI.hpp"
#include "xasm++/cli/command_line_options.h"

namespace xasm {

// Parse command-line arguments and return options struct
CommandLineOptions ParseCommandLine(int argc, char** argv) {
  CommandLineOptions opts;

  CLI::App app{"xasm++ - Cross-platform assembler", "xasm++"};

  // Help and version flags (processed by CLI11 automatically)
  app.set_help_flag("--help,-h", "Show help message");
  app.set_version_flag("--version,-v", "0.1.0");

  // Positional argument: input file (required, but allow --help/--version without it)
  app.add_option("input", opts.input_file, "Input assembly file");

  // CPU architecture option
  app.add_option("--cpu", opts.cpu, "CPU architecture (default: 6502)")
      ->default_val("6502")
      ->check(CLI::IsMember({"6502"}));  // Extensible to 65c02, z80, etc.

  // Syntax parser option
  app.add_option("--syntax", opts.syntax, "Syntax parser (default: simple)")
      ->default_val("simple")
      ->check(CLI::IsMember({"simple", "merlin"}));  // simple and merlin supported

  // Output file option
  app.add_option("--output,-o", opts.output, "Output file (default: a.out)")
      ->default_val("a.out");

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    opts.show_help = true;
    return opts;
  } catch (const CLI::CallForVersion &e) {
    opts.show_version = true;
    return opts;
  } catch (const CLI::ParseError &e) {
    app.exit(e);
    throw;
  }

  // Validate input file is provided (unless help/version was requested)
  if (opts.input_file.empty()) {
    throw CLI::RequiredError("input");
  }

  return opts;
}

} // namespace xasm
