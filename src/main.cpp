// xasm++ - Cross-platform assembler
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "CLI/CLI.hpp"
#include "xasm++/assembler.h"
#include "xasm++/cli/command_line_options.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/core/error_formatter.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/cpu/cpu_constants.h"
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/output/binary_output.h"
#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/output/listing_output.h"
#include "xasm++/output/srec_writer.h"
#include "xasm++/output/symbol_output.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/syntax/edtasm_syntax.h"
#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include "xasm++/syntax/simple_syntax.h"
#include "xasm++/version.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

// Forward declaration (implemented in cli_parser.cpp)
namespace xasm {
CommandLineOptions ParseCommandLine(int argc, char **argv);
} // namespace xasm

using namespace xasm;

// ---------------------------------------------------------------------------
// Helper: resolve --color flag to formatter enum
// ---------------------------------------------------------------------------
static ErrorFormatter::ColorMode
ResolveColorMode(const std::string &color_flag) {
  if (color_flag == "always") {
    return ErrorFormatter::ColorMode::Enabled;
  }
  if (color_flag == "never") {
    return ErrorFormatter::ColorMode::Disabled;
  }
  return ErrorFormatter::ColorMode::Auto;
}

// ---------------------------------------------------------------------------
// Helper: inject -D/--define symbols into the symbol table
// ---------------------------------------------------------------------------
static void ApplyPreDefinitions(const std::vector<std::string> &defines,
                                ConcreteSymbolTable &symbols) {
  for (const auto &def : defines) {
    auto eq_pos = def.find('=');
    std::string name =
        (eq_pos != std::string::npos) ? def.substr(0, eq_pos) : def;
    int64_t value = (eq_pos != std::string::npos)
                        ? std::stoll(def.substr(eq_pos + 1), nullptr, 0)
                        : 1;
    symbols.DefineLabel(name, value);
  }
}

// ---------------------------------------------------------------------------
// Helper: select and wire up the CPU plugin.
// Returns nullptr on unknown CPU (caller should print error and exit).
// ---------------------------------------------------------------------------
static CpuPlugin *SelectCpu(const std::string &cpu_name, Cpu6502 &cpu6502,
                             Cpu6809 &cpu6809, CpuZ80 &cpu_z80) {
  static const std::unordered_map<std::string, CpuMode> k6502Modes = {
      {cpu::CPU_6502, CpuMode::Cpu6502},
      {cpu::CPU_65C02, CpuMode::Cpu65C02},
      {cpu::CPU_65C02_ROCK, CpuMode::Cpu65C02Rock},
      {cpu::CPU_65816, CpuMode::Cpu65816},
  };

  if (cpu_name == cpu::CPU_6809) {
    return &cpu6809;
  }
  if (cpu_name == cpu::CPU_Z80) {
    return &cpu_z80;
  }
  auto it = k6502Modes.find(cpu_name);
  if (it != k6502Modes.end()) {
    cpu6502.SetCpuMode(it->second);
    return &cpu6502;
  }
  return nullptr;
}

// ---------------------------------------------------------------------------
// Helper: run the Merlin-specific parse step.
// Returns false when the CPU/syntax combination is invalid.
// ---------------------------------------------------------------------------
static bool RunMerlinParse(const CommandLineOptions &opts,
                           std::optional<std::array<uint16_t, 4>> &rw18_header,
                           const std::string &source, Section &section,
                           ConcreteSymbolTable &symbols, Cpu6502 &cpu6502) {
  if (opts.cpu == cpu::CPU_6809 || opts.cpu == cpu::CPU_Z80) {
    std::cerr << "Error: Merlin syntax is only compatible with 6502 "
                 "family CPUs\n";
    std::cerr << "For " << cpu::CPU_6809
              << ", use --syntax edtasm or --syntax scmasm\n";
    std::cerr << "For " << cpu::CPU_Z80
              << ", use --syntax edtasm_m80_plusplus\n";
    return false;
  }
  MerlinSyntaxParser parser;
  parser.SetCpu(&cpu6502);
  if (opts.rw18) {
    parser.SetRw18Mode(true);
  }
  parser.Parse(source, section, symbols);
  if (opts.rw18 && parser.HasUsrArgs()) {
    rw18_header = parser.GetUsrArgs();
  }
  return true;
}

// ---------------------------------------------------------------------------
// Helper: run the EDTASM-M80++ parse step.
// Returns false when the CPU is wrong.
// ---------------------------------------------------------------------------
static bool RunEdtasmM80Parse(const std::string &cpu_name,
                               const std::string &source, Section &section,
                               ConcreteSymbolTable &symbols, CpuZ80 &cpu_z80) {
  if (cpu_name != cpu::CPU_Z80) {
    std::cerr << "Error: EDTASM-M80++ syntax requires --cpu z80\n";
    return false;
  }
  EdtasmM80PlusPlusSyntaxParser parser;
  parser.SetCpu(&cpu_z80);
  parser.Parse(source, section, symbols);
  return true;
}

// ---------------------------------------------------------------------------
// Helper: build path-mapping map from CLI strings ("virtual=actual").
// Returns false and prints error on malformed entry.
// ---------------------------------------------------------------------------
static bool BuildPathMap(const std::vector<std::string> &mappings,
                         std::map<std::string, std::string> &path_map) {
  for (const auto &mapping : mappings) {
    size_t eq_pos = mapping.find('=');
    if (eq_pos == std::string::npos) {
      std::cerr << "Error: Invalid path mapping format: " << mapping
                << " (expected format: virtual=actual)\n";
      return false;
    }
    path_map[mapping.substr(0, eq_pos)] = mapping.substr(eq_pos + 1);
  }
  return true;
}

// ---------------------------------------------------------------------------
// Helper: run the SCMASM parse step (6502 or 6809).
// Returns false on invalid path-mapping syntax.
// ---------------------------------------------------------------------------
static bool RunScmasmParse(CommandLineOptions &opts, const std::string &source,
                            Section &section, ConcreteSymbolTable &symbols,
                            CpuPlugin *cpu) {
  ScmasmSyntaxParser parser;
  parser.SetCpu(cpu);
  if (!opts.include.empty()) {
    parser.SetIncludePaths(opts.include);
  }
  if (!opts.path_mappings.empty()) {
    std::map<std::string, std::string> path_map;
    if (!BuildPathMap(opts.path_mappings, path_map)) {
      return false;
    }
    parser.SetPathMappings(path_map);
  }
  parser.Parse(source, section, symbols);
  // .TF directive overrides -o when -o was not given explicitly.
  if (!parser.GetTfOutput().empty() && !opts.output_explicit) {
    opts.output = parser.GetTfOutput();
  }
  return true;
}

// ---------------------------------------------------------------------------
// Helper: dispatch source through the appropriate syntax parser.
// Returns false on any configuration error.
// ---------------------------------------------------------------------------
static bool ParseSource(CommandLineOptions &opts, const std::string &source,
                        Section &section, ConcreteSymbolTable &symbols,
                        CpuPlugin *cpu, Cpu6502 &cpu6502, CpuZ80 &cpu_z80,
                        std::optional<std::array<uint16_t, 4>> &rw18_header) {
  if (opts.syntax == "merlin") {
    return RunMerlinParse(opts, rw18_header, source, section, symbols, cpu6502);
  }
  if (opts.syntax == "edtasm_m80_plusplus") {
    return RunEdtasmM80Parse(opts.cpu, source, section, symbols, cpu_z80);
  }
  if (opts.syntax == "scmasm") {
    return RunScmasmParse(opts, source, section, symbols, cpu);
  }
  if (opts.syntax == "edtasm") {
    EdtasmSyntaxParser parser;
    parser.Parse(source, section, symbols);
    return true;
  }
  // Default: simple syntax
  SimpleSyntaxParser parser;
  parser.Parse(source, section, symbols);
  return true;
}

// ---------------------------------------------------------------------------
// Helper: write the main binary/ihex/srec output file.
// Returns 1 on I/O error, 0 on success.
// ---------------------------------------------------------------------------
static int WriteMainOutput(const CommandLineOptions &opts,
                            const std::vector<Section *> &sections,
                            const ConcreteSymbolTable &symbols,
                            const std::optional<std::array<uint16_t, 4>>
                                &rw18_header) {
  switch (opts.format) {
  case OutputFormat::IntelHex: {
    IntelHexWriter ihex_writer;
    std::ofstream ihex_out(opts.output);
    if (!ihex_out) {
      std::cerr << "Error: Cannot open output file: " << opts.output << "\n";
      return 1;
    }
    std::vector<Section> sec_copies;
    sec_copies.reserve(sections.size());
    for (const auto *sp : sections) {
      sec_copies.push_back(*sp);
    }
    ihex_writer.Write(sec_copies, ihex_out);
    break;
  }
  case OutputFormat::SRecord: {
    SRecordWriter srec_writer;
    std::ofstream srec_out(opts.output);
    if (!srec_out) {
      std::cerr << "Error: Cannot open output file: " << opts.output << "\n";
      return 1;
    }
    std::vector<Section> sec_copies;
    sec_copies.reserve(sections.size());
    for (const auto *sp : sections) {
      sec_copies.push_back(*sp);
    }
    srec_writer.Write(sec_copies, srec_out);
    break;
  }
  default: {
    // Default: binary format
    const std::array<uint16_t, 4> *rw18_ptr =
        rw18_header.has_value() ? &rw18_header.value() : nullptr;
    BinaryOutput::WriteOutputWithRw18(opts.output, sections, symbols, rw18_ptr);
    break;
  }
  }
  return 0;
}

// ---------------------------------------------------------------------------
// Helper: write all auxiliary output files (listing, symbol table, label map).
// ---------------------------------------------------------------------------
static void WriteAuxOutputFiles(const CommandLineOptions &opts,
                                 const std::vector<Section *> &sections,
                                 const ConcreteSymbolTable &symbols) {
  if (!opts.listing_file.empty()) {
    ListingOutput listing;
    try {
      listing.WriteOutput(opts.listing_file, sections, symbols);
      if (!opts.quiet) {
        std::cout << "Listing file generated: " << opts.listing_file << "\n";
      }
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
      std::cerr << "Warning: Failed to generate listing file: " << e.what()
                << "\n";
    }
  }

  if (!opts.symbol_file.empty()) {
    SymbolOutput symbol_output;
    try {
      symbol_output.WriteOutput(opts.symbol_file, sections, symbols);
      if (!opts.quiet) {
        std::cout << "Symbol table generated: " << opts.symbol_file << "\n";
      }
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
      std::cerr << "Warning: Failed to generate symbol table: " << e.what()
                << "\n";
    }
  }

  if (!opts.label_map.empty()) {
    SymbolOutput label_map_output;
    try {
      label_map_output.WriteOutput(opts.label_map, sections, symbols);
      if (!opts.quiet) {
        std::cout << "Label map generated: " << opts.label_map << "\n";
      }
    } catch (const std::exception &e) { // NOLINT(bugprone-empty-catch)
      std::cerr << "Warning: Failed to generate label map: " << e.what()
                << "\n";
    }
  }
}

// ---------------------------------------------------------------------------
// Main entry point
// ---------------------------------------------------------------------------
int main(int argc, char **argv) {
  try {
    CommandLineOptions opts = ParseCommandLine(argc, argv);

    ErrorFormatter error_formatter(ResolveColorMode(opts.color_mode));

    if (opts.show_help) {
      std::cout << opts.help_message;
      return 0;
    }
    if (opts.show_version) {
      std::cout << "xasm++ version " << xasm::version::version_full << "\n";
      std::cout << xasm::version::copyright << "\n";
      std::cout << xasm::version::license << "\n";
      return 0;
    }

    // Step 1: Read input file
    std::ifstream file(opts.input_file);
    if (!file.is_open()) {
      std::cerr << "Error: Cannot open input file: " << opts.input_file << "\n";
      return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Step 2: Initialise section, symbol table, and CPU plugin
    Section section;
    ConcreteSymbolTable symbols;

    if (opts.org != -1) {
      section.org = static_cast<uint64_t>(opts.org);
    }
    ApplyPreDefinitions(opts.define, symbols);

    Cpu6502 cpu6502;
    Cpu6809 cpu6809;
    CpuZ80 cpu_z80;
    CpuPlugin *cpu = SelectCpu(opts.cpu, cpu6502, cpu6809, cpu_z80);
    if (cpu == nullptr) {
      std::cerr << "Error: Unknown CPU type: " << opts.cpu << "\n";
      std::cerr << "Supported: " << cpu::CPU_6502 << ", " << cpu::CPU_65C02
                << ", " << cpu::CPU_65C02_ROCK << ", " << cpu::CPU_65816
                << ", " << cpu::CPU_6809 << ", " << cpu::CPU_Z80 << "\n";
      return 1;
    }

    // Branch relaxation defaults to off.
    cpu6502.SetRelaxBranches(opts.relax_branches);

    // Step 3: Parse source (change to source dir for PUT/include resolution)
    std::filesystem::path input_path =
        std::filesystem::absolute(opts.input_file);
    std::filesystem::path source_dir = input_path.parent_path();
    std::filesystem::path original_dir = std::filesystem::current_path();
    std::optional<std::array<uint16_t, 4>> rw18_header_args;

    try {
      if (!source_dir.empty()) {
        std::filesystem::current_path(source_dir);
      }
      if (!ParseSource(opts, source, section, symbols, cpu, cpu6502, cpu_z80,
                       rw18_header_args)) {
        std::filesystem::current_path(original_dir);
        return 1;
      }
      std::filesystem::current_path(original_dir);
    } catch (const std::runtime_error &e) {
      std::filesystem::current_path(original_dir);
      std::cerr << "Parse error: " << e.what() << "\n";
      return 1;
    } catch (const std::invalid_argument &e) {
      std::filesystem::current_path(original_dir);
      std::cerr << "Invalid syntax: " << e.what() << "\n";
      return 1;
    }

    // Step 4: Create assembler (CPU already created in Step 2)
    Assembler assembler;
    assembler.SetCpuPlugin(cpu);
    assembler.SetSymbolTable(
        &symbols); // CRITICAL: Link symbol table to assembler
    assembler.AddSection(section);

    assembler.SetMaxPasses(opts.max_passes);

    // Configure expression parser features for dialect-specific operators
    if (opts.syntax == "merlin") {
      assembler.SetExpressionFeatures(ParserFeatures::ForMerlin());
    }

    AssemblerResult result = assembler.Assemble();

    if (!result.success) {
      for (const auto &error : result.errors) {
        std::cout << error_formatter.FormatError(error, &symbols) << "\n";
      }
      return 1;
    }

    // Step 5: Write output file based on --format
    std::vector<Section *> sections = {&section};
    try {
      int rc = WriteMainOutput(opts, sections, symbols, rw18_header_args);
      if (rc != 0) {
        return rc;
      }
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "File I/O error: " << e.what() << "\n";
      return 1;
    } catch (const std::runtime_error &e) {
      std::cerr << "Output error: " << e.what() << "\n";
      return 1;
    }

    if (!opts.quiet) {
      std::cout << "Assembly successful: " << opts.output << "\n";
    }
    if (opts.verbose) {
      std::cout << "Passes completed: " << result.pass_count << "\n";
    }

    // Steps 6-8: Write listing, symbol table, and label map files
    WriteAuxOutputFiles(opts, sections, symbols);

    return 0;
  } catch (const CLI::ParseError &e) {
    std::cerr << "Command-line error: " << e.what() << "\n";
    return 1;
  } catch (const std::bad_alloc &e) {
    std::cerr << "Out of memory: " << e.what() << "\n";
    return 1;
  } catch (const std::ios_base::failure &e) {
    std::cerr << "I/O error: " << e.what() << "\n";
    return 1;
  } catch (const std::runtime_error &e) {
    std::cerr << "Runtime error: " << e.what() << "\n";
    return 1;
  } catch (const std::logic_error &e) {
    std::cerr << "Logic error: " << e.what() << "\n";
    std::cerr << "This is likely an unhandled std::invalid_argument from "
                 "stoul/stoi conversion.\n";
    std::cerr
        << "Please report this bug with the source file that caused it.\n";
    return 1;
  }
}
