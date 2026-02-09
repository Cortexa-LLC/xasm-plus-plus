/**
 * @file intel_hex_writer.h
 * @brief Intel HEX format output writer
 *
 * This file defines the IntelHexWriter class, which writes assembled sections
 * to Intel HEX format. Intel HEX is a widely-used text format for representing
 * binary data, commonly used for ROM programmers and microcontroller uploads.
 *
 * @note Intel HEX format specification:
 *       https://en.wikipedia.org/wiki/Intel_HEX
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
 * @brief Writer for Intel HEX format
 *
 * IntelHexWriter converts assembled sections to Intel HEX format, which uses
 * ASCII text representation of binary data with checksums for error detection.
 *
 * @par Format Details
 * Intel HEX records have the format:
 * @code
 * :LLAAAATTDD...DDCC
 * @endcode
 *
 * Where:
 * - `:` = Start code (colon)
 * - `LL` = Byte count (2 hex digits)
 * - `AAAA` = Address (4 hex digits)
 * - `TT` = Record type (2 hex digits)
 * - `DD...DD` = Data bytes
 * - `CC` = Checksum (2 hex digits)
 *
 * @par Record Types
 * - `00` = Data record (contains program/data bytes)
 * - `01` = End of File record
 * - `04` = Extended Linear Address (for addresses > 64KB)
 * - `05` = Start Linear Address (entry point - not used by xasm++)
 *
 * @par Usage Example
 * @code
 * std::vector<Section> sections = assembler.GetSections();
 * IntelHexWriter writer;
 * writer.SetBytesPerLine(16);  // Optional: default is 16
 * std::ofstream output("program.hex");
 * writer.Write(sections, output);
 * @endcode
 */
class IntelHexWriter : public OutputWriter {
public:
  /**
   * @brief Construct Intel HEX writer with default settings
   *
   * Default bytes per line: 16
   */
  IntelHexWriter();

  /**
   * @brief Write sections to Intel HEX format
   *
   * Converts all sections to Intel HEX format and writes to the stream.
   * Automatically handles:
   * - Extended address records for addresses > 64KB
   * - Multiple sections with address gaps
   * - Checksum calculation
   * - Proper record formatting
   *
   * @param sections Vector of assembled sections to write
   * @param output Output stream to write formatted data to
   * @throws std::runtime_error if writing fails
   */
  void Write(const std::vector<Section> &sections,
             std::ostream &output) override;

  /**
   * @brief Get file extension for Intel HEX format
   * @return "hex"
   */
  std::string GetExtension() const override;

  /**
   * @brief Get format name
   * @return "Intel HEX"
   */
  std::string GetFormatName() const override;

  /**
   * @brief Set bytes per line for data records
   *
   * Configures how many data bytes to include in each data record.
   * Typical values: 8, 16, 32
   *
   * @param bytes Bytes per line (must be > 0 and <= 255)
   * @throws std::invalid_argument if bytes is 0 or > 255
   */
  void SetBytesPerLine(size_t bytes);

private:
  size_t bytes_per_line_; ///< Number of data bytes per record (default: 16)

  /**
   * @brief Write a single Intel HEX record
   *
   * @param output Output stream
   * @param byte_count Number of data bytes
   * @param address 16-bit address field
   * @param record_type Record type (00, 01, 04, etc.)
   * @param data Data bytes (may be empty)
   */
  void WriteRecord(std::ostream &output, uint8_t byte_count, uint16_t address,
                   uint8_t record_type, const std::vector<uint8_t> &data);

  /**
   * @brief Calculate Intel HEX checksum
   *
   * Checksum = two's complement of sum of all bytes in record
   * (excluding start code and checksum itself)
   *
   * @param byte_count Byte count field
   * @param address Address field (16-bit)
   * @param record_type Record type field
   * @param data Data bytes
   * @return Calculated checksum (8-bit)
   */
  uint8_t CalculateChecksum(uint8_t byte_count, uint16_t address,
                            uint8_t record_type,
                            const std::vector<uint8_t> &data);

  /**
   * @brief Write extended linear address record (type 04)
   *
   * Sets the upper 16 bits of the address for subsequent data records.
   *
   * @param output Output stream
   * @param upper_address Upper 16 bits of 32-bit address
   */
  void WriteExtendedLinearAddress(std::ostream &output, uint16_t upper_address);

  /**
   * @brief Write EOF record (type 01)
   *
   * @param output Output stream
   */
  void WriteEOF(std::ostream &output);

  /**
   * @brief Extract bytes from atoms for writing
   *
   * Collects all data/instruction bytes from a section's atoms.
   *
   * @param section Section to extract bytes from
   * @return Vector of (address, byte) pairs
   */
  std::vector<std::pair<uint64_t, uint8_t>>
  ExtractBytes(const Section &section);
};

} // namespace xasm
