// xasm++ - Cross-platform assembler
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#include "xasm++/cli/command_line_options.h"
#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/syntax/simple_syntax.h"
#include "xasm++/output/binary_output.h"
#include "CLI/CLI.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// Forward declaration (implemented in cli_parser.cpp)
namespace xasm {
  CommandLineOptions ParseCommandLine(int argc, char** argv);
}

using namespace xasm;

// Main entry point
int main(int argc, char** argv) {
  try {
    CommandLineOptions opts = ParseCommandLine(argc, argv);

    if (opts.show_help) {
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
    SimpleSyntaxParser parser;
    try {
      parser.Parse(source, section, symbols);
    } catch (const std::exception& e) {
      std::cerr << "Parse error: " << e.what() << "\n";
      return 1;
    }

    // Step 4: Create assembler and add section
    Assembler assembler;
    assembler.AddSection(section);

    // Step 5: Assemble (resolve symbols, finalize code)
    AssemblerResult result = assembler.Assemble();
    if (!result.success) {
      std::cerr << "Assembly failed:\n";
      for (const auto& error : result.errors) {
        std::cerr << "  " << error.message << "\n";
      }
      return 1;
    }

    // Step 6: Write output file
    // Note: Section was modified in-place by Assemble()
    std::vector<Section*> sections = {&section};
    BinaryOutput output;
    try {
      output.WriteOutput(opts.output, sections, symbols);
    } catch (const std::exception& e) {
      std::cerr << "Output error: " << e.what() << "\n";
      return 1;
    }

    std::cout << "Assembly successful: " << opts.output << "\n";
    return 0;
  } catch (const CLI::ParseError &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
