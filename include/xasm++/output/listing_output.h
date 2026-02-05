/**
 * @file listing_output.h
 * @brief Listing output plugin for assembly listings
 *
 * This file defines the listing output plugin, which generates a human-
 * readable assembly listing showing addresses, bytes, and source lines.
 *
 * @note Phase 3.3: Output Format Plugins - Listing Output
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
 * @brief Listing output plugin - writes assembly listing files
 *
 * The ListingOutput plugin generates a human-readable listing file that
 * shows the assembled code with addresses, byte values, and source
 * information. This format is useful for debugging and understanding
 * the assembled output.
 *
 * @par Output Format
 * The listing file contains:
 * - Address column (hexadecimal)
 * - Byte values (hexadecimal, up to 8 bytes per line)
 * - Source information (labels, mnemonics, operands)
 *
 * @par Example Listing Format
 * @code
 * Address  Bytes              Source
 * -------  -----------------  ------
 * 8000                        start:
 * 8000     A9 42              LDA #$42
 * 8002     8D 00 04           STA $0400
 * 8005     60                 RTS
 * @endcode
 *
 * @par Compatibility
 * The format matches ca65 and Merlin assembler conventions.
 */
class ListingOutput : public OutputPlugin {
public:
  /**
   * @brief Default constructor
   */
  ListingOutput() = default;

  /**
   * @brief Get the plugin name
   * @return "listing"
   */
  std::string GetName() const override;

  /**
   * @brief Get the default file extension
   * @return ".lst"
   */
  std::string GetFileExtension() const override;

  /**
   * @brief Write output to a listing file
   *
   * Generates a human-readable assembly listing showing addresses,
   * bytes, and source information.
   *
   * @param filename Output filename
   * @param sections List of assembled sections
   * @param symbols Symbol table (for reference information)
   *
   * @throws std::runtime_error if file cannot be written
   */
  void WriteOutput(const std::string &filename,
                   const std::vector<Section *> &sections,
                   const SymbolTable &symbols) override;
};

} // namespace xasm
