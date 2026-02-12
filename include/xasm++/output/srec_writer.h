/**
 * @file srec_writer.h
 * @brief Motorola S-Record format output writer
 *
 * This file defines the SRecordWriter class, which writes assembled sections
 * to Motorola S-Record format. S-Record is an industry-standard text format
 * for representing binary data, commonly used with Motorola processors and
 * embedded systems.
 *
 * @note S-Record format specification:
 *       https://en.wikipedia.org/wiki/SREC_(file_format)
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
 * @brief Writer for Motorola S-Record format
 *
 * SRecordWriter converts assembled sections to S-Record format, which uses
 * ASCII text representation with checksums for error detection. The format
 * automatically adapts based on the address range:
 * - S19: 16-bit addresses (max 64KB)
 * - S28: 24-bit addresses (max 16MB)
 * - S37: 32-bit addresses (max 4GB)
 *
 * @par Format Details
 * S-Record records have the format:
 * @code
 * STLLAA...AADD...DDCC
 * @endcode
 *
 * Where:
 * - `S` = Start code (letter 'S')
 * - `T` = Record type (0-9)
 * - `LL` = Byte count (2 hex digits) - address + data + checksum
 * - `AA...AA` = Address (4/6/8 hex digits depending on type)
 * - `DD...DD` = Data bytes
 * - `CC` = Checksum (one's complement of sum)
 *
 * @par Record Types
 * - `S0` = Header record (metadata/filename)
 * - `S1` = Data record with 16-bit address (S19)
 * - `S2` = Data record with 24-bit address (S28)
 * - `S3` = Data record with 32-bit address (S37)
 * - `S5` = Count record (16-bit count of S1/S2/S3 records)
 * - `S6` = Count record (24-bit count)
 * - `S7` = Start address record (32-bit) - terminator for S37
 * - `S8` = Start address record (24-bit) - terminator for S28
 * - `S9` = Start address record (16-bit) - terminator for S19
 *
 * @par Usage Example
 * @code
 * std::vector<Section> sections = assembler.GetSections();
 * SRecordWriter writer;
 * writer.SetBytesPerLine(32);  // Optional: default is 32
 * std::ofstream output("program.s19");
 * writer.Write(sections, output);
 * @endcode
 */
class SRecordWriter : public OutputWriter {
public:
  /**
   * @brief Construct S-Record writer with default settings
   *
   * Default bytes per line: 32
   */
  SRecordWriter();

  /**
   * @brief Write sections to S-Record format
   *
   * Converts all sections to S-Record format and writes to the stream.
   * Automatically:
   * - Selects appropriate format (S19/S28/S37) based on max address
   * - Writes header record (S0)
   * - Writes data records (S1/S2/S3)
   * - Writes optional count record (S5/S6)
   * - Writes terminator record (S7/S8/S9)
   * - Calculates checksums for all records
   *
   * @param sections Vector of assembled sections to write
   * @param output Output stream to write formatted data to
   * @throws std::runtime_error if writing fails
   */
  void Write(const std::vector<Section> &sections,
             std::ostream &output) override;

  /**
   * @brief Get file extension for S-Record format
   * @return "s19" (most common variant)
   */
  std::string GetExtension() const override;

  /**
   * @brief Get format name
   * @return "Motorola S-Record"
   */
  std::string GetFormatName() const override;

  /**
   * @brief Set bytes per line for data records
   *
   * Configures how many data bytes to include in each data record.
   * Typical values: 16, 32
   *
   * @param bytes Bytes per line (must be > 0 and <= 255)
   * @throws std::invalid_argument if bytes is 0 or > 255
   */
  void SetBytesPerLine(size_t bytes);

private:
  size_t bytes_per_line_; ///< Number of data bytes per record (default: 32)

  /**
   * @brief Determine which S-Record format to use based on max address
   *
   * @param max_address Highest address in all sections
   * @return Record type for data records (1=S1, 2=S2, 3=S3)
   */
  int DetermineFormat(uint64_t max_address);

  /**
   * @brief Write a single S-Record
   *
   * @param output Output stream
   * @param record_type Record type (0-9)
   * @param address Address field
   * @param data Data bytes (may be empty)
   */
  void WriteRecord(std::ostream &output, int record_type, uint64_t address,
                   const std::vector<uint8_t> &data);

  /**
   * @brief Calculate S-Record checksum
   *
   * Checksum = one's complement of sum of count + address + data bytes
   *
   * @param byte_count Byte count field (address bytes + data bytes + 1)
   * @param address Address field
   * @param addr_size Address size in bytes (2, 3, or 4)
   * @param data Data bytes
   * @return Calculated checksum (8-bit)
   */
  uint8_t CalculateChecksum(uint8_t byte_count, uint64_t address,
                            size_t addr_size,
                            const std::vector<uint8_t> &data);

  /**
   * @brief Write header record (S0)
   *
   * @param output Output stream
   * @param module_name Optional module name/metadata
   */
  void WriteHeader(std::ostream &output, const std::string &module_name = "");

  /**
   * @brief Write terminator record (S7/S8/S9)
   *
   * @param output Output stream
   * @param format Format type (1=S9, 2=S8, 3=S7)
   * @param start_address Optional start/entry address (default: 0)
   */
  void WriteTerminator(std::ostream &output, int format,
                       uint64_t start_address = 0);

  /**
   * @brief Write optional count record (S5/S6)
   *
   * @param output Output stream
   * @param record_count Number of data records written
   * @param format Format type (1=S5, 2=S6, 3=S6)
   */
  void WriteCountRecord(std::ostream &output, size_t record_count, int format);

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

  /**
   * @brief Get address size in bytes for format type
   *
   * @param format Format type (1=2 bytes, 2=3 bytes, 3=4 bytes)
   * @return Address size in bytes
   */
  size_t GetAddressSize(int format) const;
};

} // namespace xasm
