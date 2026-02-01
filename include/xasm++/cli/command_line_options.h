/**
 * @file command_line_options.h
 * @brief Command-line argument parsing result
 * 
 * This file defines the structure that holds parsed command-line options
 * for the xasm++ assembler.
 * 
 * @note Phase 1: Minimal Viable Assembler - Command-Line Interface
 */

#pragma once

#include <string>

namespace xasm {

/**
 * @brief Parsed command-line options
 * 
 * This structure holds the result of parsing command-line arguments.
 * It contains all options that control assembler behavior.
 * 
 * @par Example Usage
 * @code
 * xasm++ --cpu 65c02 --syntax merlin -o output.bin input.s
 * 
 * // Results in:
 * CommandLineOptions opts;
 * opts.input_file = "input.s";
 * opts.cpu = "65c02";
 * opts.syntax = "merlin";
 * opts.output = "output.bin";
 * @endcode
 */
struct CommandLineOptions {
  std::string input_file;       ///< Input assembly source file
  std::string cpu = "6502";     ///< Target CPU (6502, 65c02, 65816, etc.)
  std::string syntax = "simple"; ///< Syntax style (simple, merlin)
  std::string output = "a.out";  ///< Output filename
  std::string listing_file;      ///< Listing file (.lst) - optional
  std::string symbol_file;       ///< Symbol table file (.sym) - optional
  std::string color_mode = "auto"; ///< Color output mode (auto, always, never)
  bool show_help = false;        ///< True if --help was specified
  bool show_version = false;     ///< True if --version was specified
  std::string help_message;      ///< Help message text (populated when --help used)
};

} // namespace xasm
