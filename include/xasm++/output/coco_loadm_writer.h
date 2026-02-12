/**
 * @file coco_loadm_writer.h
 * @brief CoCo DOS (LOADM) binary format output writer
 *
 * This file defines the CocoLoadmWriter class, which writes assembled sections
 * to CoCo DOS (LOADM) binary format. This format is used on the TRS-80 Color
 * Computer running Color BASIC and Disk Extended Color BASIC (DECB).
 *
 * @note CoCo DOS binary format specification:
 *       - Block-based format with preamble, data blocks, and postamble
 *       - Commands to load: LOADM "filename" then EXEC &HXXXX
 *       - Used for 6809 machine code
 * @note Part of Phase 2: Output Format System
 */

#pragma once

#include "xasm++/output/output_writer.h"
#include "xasm++/section.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Writer for CoCo DOS (LOADM) binary format
 *
 * CocoLoadmWriter converts assembled sections to CoCo DOS binary format,
 * which consists of a preamble, one or more data blocks, and a postamble.
 *
 * @par Format Details
 * Preamble block:
 * @code
 * 0x00 [len_hi] [len_lo] [addr_hi] [addr_lo]
 * @endcode
 *
 * Data block(s):
 * @code
 * 0x00 [len_hi] [len_lo] [addr_hi] [addr_lo] [data...]
 * @endcode
 *
 * Postamble (without entry point):
 * @code
 * 0xFF 0x00
 * @endcode
 *
 * Postamble (with entry point):
 * @code
 * 0xFF 0x00 0x00 [addr_hi] [addr_lo]
 * @endcode
 *
 * Where:
 * - All multi-byte values are big-endian (Motorola 6809 native)
 * - `len` = Data length (16-bit)
 * - `addr` = Load address (16-bit)
 * - `data` = Binary data bytes
 *
 * @par Usage Example
 * @code
 * std::vector<Section> sections = assembler.GetSections();
 * CocoLoadmWriter writer;
 * writer.SetEntryPoint(0x2000);  // Optional entry point
 * std::ofstream output("program.bin", std::ios::binary);
 * writer.Write(sections, output);
 * @endcode
 *
 * @par Platform
 * - Target: TRS-80 Color Computer
 * - OS: Color BASIC, Disk Extended Color BASIC (DECB)
 * - CPU: Motorola 6809
 * - Load command: `LOADM "filename"` then `EXEC &HXXXX`
 */
class CocoLoadmWriter : public OutputWriter {
public:
  /**
   * @brief Construct CoCo LOADM writer with default settings
   */
  CocoLoadmWriter();

  /**
   * @brief Write sections to CoCo DOS (LOADM) format
   *
   * Converts all sections to CoCo format and writes to the stream.
   * Automatically:
   * - Writes preamble block
   * - Creates data blocks for contiguous data
   * - Handles address gaps (creates separate blocks)
   * - Writes postamble (with entry point if specified)
   *
   * @param sections Vector of assembled sections to write
   * @param output Output stream to write binary data to
   * @throws std::runtime_error if writing fails
   */
  void Write(const std::vector<Section> &sections,
             std::ostream &output) override;

  /**
   * @brief Get file extension for CoCo DOS format
   * @return "bin" (standard CoCo binary extension)
   */
  std::string GetExtension() const override;

  /**
   * @brief Get format name
   * @return "CoCo DOS (LOADM)"
   */
  std::string GetFormatName() const override;

  /**
   * @brief Set entry point address
   *
   * Specifies the execution start address. If set, the postamble
   * will include the entry point address.
   *
   * @param address Entry point address (16-bit)
   */
  void SetEntryPoint(uint64_t address);

private:
  bool has_entry_point_;      ///< Whether entry point is specified
  uint64_t entry_point_addr_; ///< Entry point address

  /**
   * @brief Write preamble block
   *
   * @param output Output stream
   * @param first_address Address of first data block (0 if none)
   * @param total_length Total length of all data (0 if none)
   */
  void WritePreamble(std::ostream &output, uint64_t first_address,
                     size_t total_length);

  /**
   * @brief Write a data block
   *
   * @param output Output stream
   * @param address Load address
   * @param data Data bytes
   */
  void WriteDataBlock(std::ostream &output, uint64_t address,
                      const std::vector<uint8_t> &data);

  /**
   * @brief Write postamble block
   *
   * @param output Output stream
   */
  void WritePostamble(std::ostream &output);

  /**
   * @brief Write 16-bit value in big-endian format
   *
   * @param output Output stream
   * @param value 16-bit value to write
   */
  void WriteBE16(std::ostream &output, uint16_t value);

  /**
   * @brief Extract bytes from atoms for writing
   *
   * Collects all data/instruction bytes from a section's atoms.
   * Returns vector of (address, byte) pairs to detect address gaps.
   *
   * @param section Section to extract bytes from
   * @return Vector of (address, byte) pairs
   */
  std::vector<std::pair<uint64_t, uint8_t>>
  ExtractBytes(const Section &section);
};

} // namespace xasm
