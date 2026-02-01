/**
 * @file symbol_output.h
 * @brief Symbol table output plugin
 * 
 * This file defines the symbol table output plugin, which generates a
 * file containing all defined symbols and their values.
 * 
 * @note Phase 3.3: Output Format Plugins - Symbol Table Output
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
 * @brief Symbol table output plugin - writes symbol definition files
 * 
 * The SymbolOutput plugin generates a symbol table file that lists all
 * defined symbols and their values. This is useful for debugging,
 * linking, and understanding symbol definitions.
 * 
 * @par Output Format
 * The symbol file contains:
 * - Symbol name
 * - Symbol value (hexadecimal address or constant)
 * - Symbol type (label, equate, set)
 * - Sorted alphabetically
 * 
 * @par Example Symbol File Format
 * @code
 * Symbol Table
 * ============
 * 
 * CONST1     = $002A  (equate)
 * label1     = $8000  (label)
 * start      = $8000  (label)
 * var1       = $0064  (set)
 * @endcode
 * 
 * @par Compatibility
 * The format matches ca65 and Merlin assembler conventions.
 */
class SymbolOutput : public OutputPlugin {
public:
  /**
   * @brief Default constructor
   */
  SymbolOutput() = default;

  /**
   * @brief Get the plugin name
   * @return "symbol"
   */
  std::string GetName() const override;
  
  /**
   * @brief Get the default file extension
   * @return ".sym"
   */
  std::string GetFileExtension() const override;
  
  /**
   * @brief Write output to a symbol table file
   * 
   * Generates a symbol table file containing all defined symbols,
   * sorted alphabetically with their values and types.
   * 
   * @param filename Output filename
   * @param sections List of assembled sections (not used)
   * @param symbols Symbol table to write
   * 
   * @throws std::runtime_error if file cannot be written
   */
  void WriteOutput(const std::string& filename,
                   const std::vector<Section*>& sections,
                   const SymbolTable& symbols) override;
};

} // namespace xasm
