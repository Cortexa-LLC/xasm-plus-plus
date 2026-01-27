/**
 * @file binary_output.h
 * @brief Binary output plugin for raw binary files
 * 
 * This file defines the binary output plugin, which writes assembled
 * code and data directly to a raw binary file.
 * 
 * @note Phase 1: Minimal Viable Assembler - Binary Output Plugin
 */

#pragma once

#include "xasm++/output/output_plugin.h"
#include <string>
#include <vector>

namespace xasm {

// Forward declarations
class Section;
class SymbolTable;

/**
 * @brief Binary output plugin - writes raw binary files
 * 
 * The BinaryOutput plugin writes assembled sections directly to a binary
 * file with no headers or metadata. This is the simplest output format
 * and is suitable for ROM images, cartridges, and embedded systems.
 * 
 * @par Output Format
 * The output file contains the raw bytes from all sections concatenated
 * together. The file starts at the lowest section origin address and
 * contains all bytes up to the highest address.
 * 
 * @par Gap Handling
 * If there are gaps between sections (e.g., section at 0x8000 and another
 * at 0x9000), the gap is filled with zeros in the output file.
 * 
 * @par Example
 * @code
 * BinaryOutput plugin;
 * plugin.WriteOutput("program.bin", sections, symbols);
 * // Creates program.bin with raw binary data
 * @endcode
 */
class BinaryOutput : public OutputPlugin {
public:
  /**
   * @brief Default constructor
   */
  BinaryOutput() = default;

  /**
   * @brief Get the plugin name
   * @return "Binary"
   */
  std::string GetName() const override;
  
  /**
   * @brief Get the default file extension
   * @return ".bin"
   */
  std::string GetFileExtension() const override;
  
  /**
   * @brief Write output to a binary file
   * 
   * Writes all sections to a raw binary file. Sections are written in
   * address order, with gaps filled by zeros.
   * 
   * @param filename Output filename
   * @param sections List of assembled sections
   * @param symbols Symbol table (not used in binary output)
   * 
   * @throws std::runtime_error if file cannot be written
   */
  void WriteOutput(const std::string& filename,
                   const std::vector<Section*>& sections,
                   const SymbolTable& symbols) override;
};

} // namespace xasm
