// CLI argument parser
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "CLI/CLI.hpp"
#include "xasm++/cli/command_line_options.h"
#include "xasm++/cpu/cpu_constants.h"
#include "xasm++/version.h"

namespace xasm {

// Parse command-line arguments and return options struct
CommandLineOptions ParseCommandLine(int argc, char **argv) {
  CommandLineOptions opts;

  CLI::App app{"xasm++ - Cross-platform assembler", "xasm++"};

  // Help and version flags (processed by CLI11 automatically)
  app.set_help_flag("--help,-h", "Show help message");
  app.set_version_flag("--version,-v", xasm::version::version_full);

  // Positional argument: input file (required, but allow --help/--version
  // without it)
  app.add_option("input", opts.input_file, "Input assembly file");

  // CPU architecture option
  app.add_option("--cpu", opts.cpu, "CPU architecture (default: 6502)")
      ->default_val(cpu::CPU_6502)
      ->check(CLI::IsMember({cpu::CPU_6502, cpu::CPU_65C02, cpu::CPU_65C02_ROCK,
                             cpu::CPU_65816, cpu::CPU_6809}));

  // Syntax parser option
  app.add_option("--syntax", opts.syntax, "Syntax parser (default: simple)")
      ->default_val("simple")
      ->check(CLI::IsMember(
          {"simple", "merlin", "scmasm",
           "edtasm"})); // simple, merlin, scmasm, and edtasm supported

  // Output file option
  app.add_option("--output,-o", opts.output, "Output file (default: a.out)")
      ->default_val("a.out");

  // Listing file option (optional)
  app.add_option("--list", opts.listing_file, "Generate listing file (.lst)");

  // Symbol table option (optional)
  app.add_option("--symbols", opts.symbol_file,
                 "Generate symbol table file (.sym)");

  // Color output option
  app.add_option("--color", opts.color_mode,
                 "Color output (auto, always, never)")
      ->default_val("auto")
      ->check(CLI::IsMember({"auto", "always", "never"}));

  // Include path option (can be specified multiple times)
  app.add_option("--include-path,-I", opts.include_paths,
                 "Add directory to include search path (can be used multiple times)");
  
  // Path mapping option for .INB directive (A2osX compatibility)
  app.add_option("--path-map", opts.path_mappings,
                 "Map virtual paths to actual paths for .INB directive (format: virtual=actual, can be used multiple times)");

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    opts.show_help = true;
    opts.help_message = app.help();
    return opts;
  } catch (const CLI::CallForVersion &e) {
    opts.show_version = true;
    return opts;
  } catch (const CLI::ParseError &e) {
    app.exit(e);
    throw;
  }

  // If no input file provided, show help
  if (opts.input_file.empty()) {
    opts.show_help = true;
    opts.help_message = app.help();
  }

  return opts;
}

} // namespace xasm
