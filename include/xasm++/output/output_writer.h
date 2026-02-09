/**
 * @file output_writer.h
 * @brief Base interface for output format writers
 *
 * This file defines the OutputWriter interface, which provides a common
 * contract for all output format writers (Intel HEX, S-Record, binary, etc.).
 *
 * @note Part of Phase 2: Output Format System
 */

#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "xasm++/section.h"

namespace xasm {

/**
 * @brief Base interface for output format writers
 *
 * OutputWriter provides a common interface for writing assembled sections
 * to various output formats (Intel HEX, Motorola S-Record, binary, etc.).
 *
 * Each format writer:
 * - Takes a list of assembled sections as input
 * - Converts the section data to the appropriate format
 * - Writes the formatted output to a stream
 *
 * @par Usage Example
 * @code
 * std::vector<Section> sections = assembler.GetSections();
 * IntelHexWriter writer;
 * std::ofstream output("program.hex");
 * writer.Write(sections, output);
 * @endcode
 */
class OutputWriter {
public:
  /**
   * @brief Virtual destructor for polymorphic deletion
   */
  virtual ~OutputWriter() = default;

  /**
   * @brief Write assembled sections to output stream
   *
   * Converts the assembled sections to the appropriate output format
   * and writes them to the provided stream.
   *
   * @param sections Vector of assembled sections containing atom data
   * @param output Output stream to write formatted data to
   * @throws std::runtime_error if writing fails or data is invalid
   */
  virtual void Write(const std::vector<Section> &sections,
                     std::ostream &output) = 0;

  /**
   * @brief Get the file extension for this output format
   *
   * Returns the typical file extension used for this format (without dot).
   *
   * @return File extension string (e.g., "hex", "s19", "bin")
   */
  virtual std::string GetExtension() const = 0;

  /**
   * @brief Get the format name
   *
   * Returns a human-readable name for this output format.
   *
   * @return Format name (e.g., "Intel HEX", "Motorola S-Record")
   */
  virtual std::string GetFormatName() const = 0;
};

} // namespace xasm
