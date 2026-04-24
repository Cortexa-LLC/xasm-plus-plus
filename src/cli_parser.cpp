/**
 * @file cli_parser.cpp
 * @brief CLI argument parser
 *
 * Phase 1: Minimal Viable Assembler - Command-Line Interface
 */

#include "CLI/CLI.hpp"
#include "xasm++/assembler.h"
#include "xasm++/cli/command_line_options.h"
#include "xasm++/cpu/cpu_constants.h"
#include "xasm++/version.h"

namespace xasm {

// Parse command-line arguments and return options struct
CommandLineOptions ParseCommandLine(int argc, char** argv) {
  CommandLineOptions opts;

  CLI::App app{"xasm++ - Cross-platform assembler", "xasm++"};

  // Help and version flags (processed by CLI11 automatically)
  app.set_help_flag("--help,-h", "Show help message");
  app.set_version_flag("--version,-v", xasm::version::kVersionFull);

  // Positional argument: input file (required, but allow --help/--version
  // without it)
  app.add_option("input", opts.input_file, "Input assembly file");

  // kCPU architecture option
  app.add_option("--cpu", opts.cpu, "CPU architecture (default: 6502)")
      ->default_val(cpu::kCPU_6502)
      ->check(CLI::IsMember(
          {cpu::kCPU_6502, cpu::kCPU_65C02, cpu::kCPU_65C02_ROCK, cpu::kCPU_65816, cpu::kCPU_6809}));

  // Syntax parser option
  app.add_option("--syntax", opts.syntax, "Syntax parser (default: simple)")
      ->default_val("simple")
      ->check(CLI::IsMember({"simple", "merlin", "scmasm",
                             "edtasm"}));  // simple, merlin, scmasm, and edtasm supported

  // Output file option
  app.add_option("--output,-o", opts.output, "Output file (default: a.out)")
      ->default_val("a.out")
      ->each([&opts](const std::string&) { opts.output_explicit = true; });

  // Listing file option (optional)
  app.add_option("--list", opts.listing_file, "Generate listing file (.lst)");

  // Symbol table option (optional)
  app.add_option("--symbols", opts.symbol_file, "Generate symbol table file (.sym)");

  // Color output option
  app.add_option("--color", opts.color_mode, "Color output (auto, always, never)")
      ->default_val("auto")
      ->check(CLI::IsMember({"auto", "always", "never"}));

  // Include path option (renamed from --include-path to --include, keeping -I alias)
  // allow_extra_args(false) prevents CLI11 from greedily consuming the positional
  // input filename when -I is followed by the source file (e.g. -I /stage src/foo.s)
  app.add_option("--include,-I", opts.include,
                 "Add directory to include search path (for .INB directive)")
      ->allow_extra_args(false);

  // Path mapping option for .INB directive (A2osX compatibility)
  app.add_option("--path-map", opts.path_mappings,
                 "Map virtual paths to actual paths for .INB directive "
                 "(format: virtual=actual, can be used multiple times)");

  // Define macro option (can be specified multiple times)
  app.add_option("-D,--define", opts.define,
                 "Define macro (format: NAME=VALUE, can be used multiple times)")
      ->allow_extra_args(false);

  // Maximum assembly passes
  app.add_option("--max-passes", opts.max_passes,
                 "Maximum assembly passes (default: " + std::to_string(Assembler::kMAX_PASSES) + ")")
      ->default_val(Assembler::kMAX_PASSES);

  // Label map file for debugging
  app.add_option("--label-map", opts.label_map, "Generate label map file for debugging");

  // Warning control options
  // --warn LEVEL sets warning level (0=none,1=default,2=extra,3=all)
  // --no-warn is shorthand for --warn 0
  app.add_option("--warn", opts.warn, "Warning level: 0=none, 1=default, 2=extra, 3=all")
      ->default_val(1)
      ->check(CLI::Range(0, 3));
  app.add_flag(
      "--no-warn", [&opts](int64_t /*count*/) { opts.warn = 0; },
      "Suppress all warnings (equivalent to --warn 0)");
  app.add_flag("--werror", opts.werror, "Treat warnings as errors");

  // Branch relaxation (off by default - original assemblers error on out-of-range)
  app.add_flag("--relax-branches", opts.relax_branches,
               "Expand out-of-range branches to B!cc+3/JMP sequences (default: error)");

  app.add_flag("--rw18", opts.rw18,
               "Output raw binary with no header "
               "(vasm -rw18 compatibility for Prince of Persia modules)");

  // Quiet mode
  app.add_flag("-q,--quiet", opts.quiet, "Suppress non-essential output");

  // Verbose mode
  app.add_flag("-V,--verbose", opts.verbose, "Enable verbose output");

  // Origin address override
  app.add_option("--org", opts.org, "Origin address (override default)");

  // Output format
  // First, parse as string, then convert to enum
  std::string format_str;
  app.add_option("--format", format_str, "Output format (binary, ihex, srec)")
      ->default_val("binary")
      ->check(CLI::IsMember({"binary", "ihex", "srec"}));

  // After parsing, convert string to enum
  app.callback([&opts, &format_str]() {
    if (format_str == "binary") {
      opts.format = OutputFormat::Binary;
    } else if (format_str == "ihex") {
      opts.format = OutputFormat::IntelHex;
    } else if (format_str == "srec") {
      opts.format = OutputFormat::SRecord;
    }
  });

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp& e) {
    opts.show_help = true;
    opts.help_message = app.help();
    return opts;
  } catch (const CLI::CallForVersion& e) {
    opts.show_version = true;
    return opts;
  } catch (const CLI::ParseError& e) {
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

}  // namespace xasm
