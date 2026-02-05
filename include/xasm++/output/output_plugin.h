/**
 * @file output_plugin.h
 * @brief Base interface for output format plugins
 *
 * This file defines the abstract interface that all output format plugins
 * must implement. Plugins are responsible for generating output files in
 * various formats (binary, hex, S-record, ELF, etc.).
 *
 * @note Phase 1: Minimal Viable Assembler - Binary Output Plugin
 */

#pragma once

#include <string>
#include <vector>

namespace xasm {

// Forward declarations
class Section;
class SymbolTable;

/**
 * @brief Abstract base class for output format plugins
 *
 * Output plugins are responsible for writing assembled code and data
 * to files in various formats. Each plugin implements this interface
 * to support a specific output format.
 *
 * @par Supported Formats
 * - Binary: Raw binary output (.bin)
 * - Intel HEX: .hex files
 * - Motorola S-record: .s19, .s28, .s37
 * - ELF: Executable and Linkable Format
 * - More formats can be added as plugins
 *
 * @par Plugin Architecture
 * The assembler uses a plugin architecture for output formats, allowing
 * new formats to be added without modifying the core assembler code.
 *
 * @par Example Implementation
 * @code
 * class BinaryOutputPlugin : public OutputPlugin {
 * public:
 *     std::string GetName() const override { return "Binary"; }
 *     std::string GetFileExtension() const override { return ".bin"; }
 *     void WriteOutput(...) override {
 *         // Write raw binary data
 *     }
 * };
 * @endcode
 */
class OutputPlugin {
public:
  /**
   * @brief Virtual destructor for polymorphic deletion
   */
  virtual ~OutputPlugin() = default;

  /**
   * @brief Get the plugin name
   *
   * Returns a human-readable name for this output format plugin.
   *
   * @return Plugin name (e.g., "Binary", "Intel HEX", "S-Record")
   */
  virtual std::string GetName() const = 0;

  /**
   * @brief Get the default file extension for this format
   *
   * Returns the file extension typically used for this format,
   * including the leading dot.
   *
   * @return File extension (e.g., ".bin", ".hex", ".s19")
   */
  virtual std::string GetFileExtension() const = 0;

  /**
   * @brief Write output to a file
   *
   * Generates an output file in this plugin's format from the assembled
   * sections and symbol table. The plugin is responsible for all aspects
   * of file generation including formatting, error handling, and file I/O.
   *
   * @param filename Output filename (may be adjusted based on extension)
   * @param sections List of assembled sections to write
   * @param symbols Symbol table (for debug info, symbol files, etc.)
   *
   * @throws std::runtime_error if file cannot be written or format error
   *
   * @note Sections must be fully assembled before calling this method
   */
  virtual void WriteOutput(const std::string &filename,
                           const std::vector<Section *> &sections,
                           const SymbolTable &symbols) = 0;
};

} // namespace xasm
