/**
 * @file trsdos_writer.h
 * @brief TRS-DOS binary format output writer
 *
 * This file defines the TrsDosWriter class, which writes assembled sections
 * to TRS-DOS binary format. This format is used on TRS-80 Model I, III, and 4
 * computers running TRS-DOS, NEWDOS/80, or compatible operating systems.
 *
 * @note TRS-DOS binary format specification:
 *       - Segment-based format with load address and length
 *       - Command to load: SYSTEM "filename"
 *       - Used primarily for Z80 machine code
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
 * @brief Writer for TRS-DOS binary format
 *
 * TrsDosWriter converts assembled sections to TRS-DOS binary format, which
 * consists of one or more segments with load addresses and binary data.
 * Each segment specifies where in memory the data should be loaded.
 *
 * @par Format Details
 * Each segment has the structure:
 * @code
 * [type] [addr_lo] [addr_hi] [len_lo] [len_hi] [data...]
 * @endcode
 *
 * Where:
 * - `type` = Segment type (1 byte)
 *   - 0x01 = Data segment
 *   - 0x02 = Entry point segment (length = 0)
 * - `addr_lo/addr_hi` = Load address (16-bit little-endian)
 * - `len_lo/len_hi` = Data length (16-bit little-endian)
 * - `data` = Binary data bytes
 *
 * @par Usage Example
 * @code
 * std::vector<Section> sections = assembler.GetSections();
 * TrsDosWriter writer;
 * writer.SetEntryPoint(0x8000);  // Optional entry point
 * std::ofstream output("program.cmd", std::ios::binary);
 * writer.Write(sections, output);
 * @endcode
 *
 * @par Platform
 * - Target: TRS-80 Model I/III/4
 * - OS: TRS-DOS, NEWDOS/80, compatible
 * - CPU: Z80
 * - Load command: `SYSTEM "filename"`
 */
class TrsDosWriter : public OutputWriter {
public:
  /**
   * @brief Construct TRS-DOS writer with default settings
   */
  TrsDosWriter();

  /**
   * @brief Write sections to TRS-DOS binary format
   *
   * Converts all sections to TRS-DOS format and writes to the stream.
   * Automatically:
   * - Creates segments for contiguous data
   * - Handles address gaps (creates separate segments)
   * - Appends entry point segment if specified
   *
   * @param sections Vector of assembled sections to write
   * @param output Output stream to write binary data to
   * @throws std::runtime_error if writing fails
   */
  void Write(const std::vector<Section> &sections,
             std::ostream &output) override;

  /**
   * @brief Get file extension for TRS-DOS format
   * @return "cmd" (standard TRS-DOS executable extension)
   */
  std::string GetExtension() const override;

  /**
   * @brief Get format name
   * @return "TRS-DOS Binary"
   */
  std::string GetFormatName() const override;

  /**
   * @brief Set entry point address
   *
   * Specifies the execution start address. If set, an entry point
   * segment (type 0x02) will be appended after all data segments.
   *
   * @param address Entry point address (16-bit)
   */
  void SetEntryPoint(uint64_t address);

private:
  bool has_entry_point_;      ///< Whether entry point is specified
  uint64_t entry_point_addr_; ///< Entry point address

  /**
   * @brief Write a single segment
   *
   * @param output Output stream
   * @param type Segment type (0x01=data, 0x02=entry)
   * @param address Load address
   * @param data Data bytes
   */
  void WriteSegment(std::ostream &output, uint8_t type, uint64_t address,
                    const std::vector<uint8_t> &data);

  /**
   * @brief Write 16-bit value in little-endian format
   *
   * @param output Output stream
   * @param value 16-bit value to write
   */
  void WriteLE16(std::ostream &output, uint16_t value);

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
