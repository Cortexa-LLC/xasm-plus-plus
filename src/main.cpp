// xasm++ - Cross-platform assembler
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "xasm++/cli/command_line_options.h"
#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/syntax/simple_syntax.h"
#include "xasm++/syntax/merlin_syntax.h"
#include "xasm++/syntax/scmasm_syntax.h"
#include "xasm++/output/binary_output.h"
#include "xasm++/output/listing_output.h"
#include "xasm++/output/symbol_output.h"
#include "xasm++/core/error_formatter.h"
#include "CLI/CLI.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// Forward declaration (implemented in cli_parser.cpp)
namespace xasm {
  CommandLineOptions ParseCommandLine(int argc, char** argv);
}

using namespace xasm;

// Main entry point
int main(int argc, char** argv) {
  try {
    CommandLineOptions opts = ParseCommandLine(argc, argv);

    // Create error formatter with configured color mode
    ErrorFormatter::ColorMode color_mode = ErrorFormatter::ColorMode::Auto;
    if (opts.color_mode == "always") {
      color_mode = ErrorFormatter::ColorMode::Enabled;
    } else if (opts.color_mode == "never") {
      color_mode = ErrorFormatter::ColorMode::Disabled;
    }
    ErrorFormatter error_formatter(color_mode);

    if (opts.show_help) {
      std::cout << opts.help_message;
      return 0;
    }

    if (opts.show_version) {
      std::cout << "xasm++ version 0.1.0\n";
      return 0;
    }

    // Phase 7: Full assembler pipeline integration

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

    // Step 2: Create section and symbol table
    Section section;
    ConcreteSymbolTable symbols;

    // Step 3: Parse source code
    // Change to source file's directory so PUT directives can find included files
    std::filesystem::path input_path = std::filesystem::absolute(opts.input_file);
    std::filesystem::path source_dir = input_path.parent_path();
    std::filesystem::path original_dir = std::filesystem::current_path();
    
    try {
      // Change to source directory for PUT directive resolution
      if (!source_dir.empty()) {
        std::filesystem::current_path(source_dir);
      }
      
      if (opts.syntax == "merlin") {
        MerlinSyntaxParser parser;
        parser.Parse(source, section, symbols);
      } else if (opts.syntax == "scmasm") {
        ScmasmSyntaxParser parser;
        parser.Parse(source, section, symbols);
      } else {
        SimpleSyntaxParser parser;
        parser.Parse(source, section, symbols);
      }
      
      // Restore original directory
      std::filesystem::current_path(original_dir);
    } catch (const std::filesystem::filesystem_error& e) {
      // Restore original directory on filesystem error
      std::filesystem::current_path(original_dir);
      std::cerr << "Filesystem error: " << e.what() << "\n";
      return 1;
    } catch (const std::runtime_error& e) {
      // Restore original directory on parse error
      std::filesystem::current_path(original_dir);
      std::cerr << "Parse error: " << e.what() << "\n";
      return 1;
    } catch (const std::invalid_argument& e) {
      // Restore original directory on invalid syntax
      std::filesystem::current_path(original_dir);
      std::cerr << "Invalid syntax: " << e.what() << "\n";
      return 1;
    }

    // Step 4: Create CPU plugin and assembler
    Cpu6502 cpu;
    Assembler assembler;
    assembler.SetCpuPlugin(&cpu);
    assembler.AddSection(section);

    // Step 5: Assemble (encode instructions, resolve symbols)
    AssemblerResult result = assembler.Assemble();
    if (!result.success) {
      for (const auto& error : result.errors) {
        std::cout << error_formatter.FormatError(error, &symbols) << "\n";
      }
      return 1;
    }

    // Step 6: Write output file
    // Note: Section was modified in-place by Assemble()
    std::vector<Section*> sections = {&section};
    BinaryOutput output;
    try {
      output.WriteOutput(opts.output, sections, symbols);
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "File I/O error: " << e.what() << "\n";
      return 1;
    } catch (const std::runtime_error& e) {
      std::cerr << "Output error: " << e.what() << "\n";
      return 1;
    }

    std::cout << "Assembly successful: " << opts.output << "\n";

    // Step 7: Generate listing file if requested
    if (!opts.listing_file.empty()) {
      ListingOutput listing;
      try {
        listing.WriteOutput(opts.listing_file, sections, symbols);
        std::cout << "Listing file generated: " << opts.listing_file << "\n";
      } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to generate listing file: " << e.what() << "\n";
      }
    }

    // Step 8: Generate symbol table if requested
    if (!opts.symbol_file.empty()) {
      SymbolOutput symbol_output;
      try {
        symbol_output.WriteOutput(opts.symbol_file, sections, symbols);
        std::cout << "Symbol table generated: " << opts.symbol_file << "\n";
      } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to generate symbol table: " << e.what() << "\n";
      }
    }

    return 0;
  } catch (const CLI::ParseError &e) {
    std::cerr << "Command-line error: " << e.what() << "\n";
    return 1;
  } catch (const std::bad_alloc& e) {
    std::cerr << "Out of memory: " << e.what() << "\n";
    return 1;
  } catch (const std::ios_base::failure& e) {
    std::cerr << "I/O error: " << e.what() << "\n";
    return 1;
  } catch (const std::runtime_error& e) {
    std::cerr << "Runtime error: " << e.what() << "\n";
    return 1;
  } catch (const std::logic_error& e) {
    std::cerr << "Logic error: " << e.what() << "\n";
    std::cerr << "This is likely an unhandled std::invalid_argument from stoul/stoi conversion.\n";
    std::cerr << "Please report this bug with the source file that caused it.\n";
    return 1;
  }
}
