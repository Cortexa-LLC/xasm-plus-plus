// CommandLineOptions - CLI argument parsing result
// Phase 1: Minimal Viable Assembler - Command-Line Interface

#pragma once

#include <string>

namespace xasm {

// Parsed command-line options
struct CommandLineOptions {
  std::string input_file;
  std::string cpu = "6502";
  std::string syntax = "simple";
  std::string output = "a.out";
  bool show_help = false;
  bool show_version = false;
};

} // namespace xasm
