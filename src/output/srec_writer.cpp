/**
 * @file srec_writer.cpp
 * @brief Implementation of Motorola S-Record format writer
 */

#include "xasm++/output/srec_writer.h"
#include "xasm++/atom.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace xasm {

SRecordWriter::SRecordWriter() : bytes_per_line_(32) {}

void SRecordWriter::Write(const std::vector<Section> &sections,
                          std::ostream &output) {
  // Extract all bytes from all sections
  std::vector<std::pair<uint64_t, uint8_t>> all_bytes;
  for (const auto &section : sections) {
    auto section_bytes = ExtractBytes(section);
    all_bytes.insert(all_bytes.end(), section_bytes.begin(),
                     section_bytes.end());
  }

  // Determine max address to select format
  uint64_t max_address = 0;
  for (const auto &[addr, byte] : all_bytes) {
    if (addr > max_address) {
      max_address = addr;
    }
  }

  int format = DetermineFormat(max_address);

  // Write header record
  WriteHeader(output);

  // Write data records
  if (!all_bytes.empty()) {
    size_t i = 0;
    while (i < all_bytes.size()) {
      uint64_t start_address = all_bytes[i].first;
      std::vector<uint8_t> data;

      // Collect bytes for this record (up to bytes_per_line_)
      while (i < all_bytes.size() && data.size() < bytes_per_line_) {
        uint64_t expected_addr = start_address + data.size();
        if (all_bytes[i].first != expected_addr) {
          // Address discontinuity - start new record
          break;
        }
        data.push_back(all_bytes[i].second);
        ++i;
      }

      // Write the data record
      WriteRecord(output, format, start_address, data);
    }
  }

  // Write terminator record
  WriteTerminator(output, format);
}

std::string SRecordWriter::GetExtension() const { return "s19"; }

std::string SRecordWriter::GetFormatName() const { return "Motorola S-Record"; }

void SRecordWriter::SetBytesPerLine(size_t bytes) {
  if (bytes == 0 || bytes > 255) {
    throw std::invalid_argument("Bytes per line must be > 0 and <= 255");
  }
  bytes_per_line_ = bytes;
}

int SRecordWriter::DetermineFormat(uint64_t max_address) {
  if (max_address >= 0x1000000) {
    return 3; // S3 (32-bit addresses)
  } else if (max_address >= 0x10000) {
    return 2; // S2 (24-bit addresses)
  } else {
    return 1; // S1 (16-bit addresses)
  }
}

void SRecordWriter::WriteRecord(std::ostream &output, int record_type,
                                uint64_t address,
                                const std::vector<uint8_t> &data) {
  size_t addr_size = GetAddressSize(record_type);
  uint8_t byte_count = addr_size + data.size() + 1; // address + data + checksum

  // Start with 'S' and record type
  output << 'S' << record_type;

  // Write byte count
  output << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
         << static_cast<int>(byte_count);

  // Write address
  output << std::setw(addr_size * 2) << address;

  // Write data bytes
  for (uint8_t byte : data) {
    output << std::setw(2) << static_cast<int>(byte);
  }

  // Calculate and write checksum
  uint8_t checksum = CalculateChecksum(byte_count, address, addr_size, data);
  output << std::setw(2) << static_cast<int>(checksum);

  output << '\n';
}

uint8_t SRecordWriter::CalculateChecksum(uint8_t byte_count, uint64_t address,
                                         size_t addr_size,
                                         const std::vector<uint8_t> &data) {
  uint8_t sum = byte_count;

  // Add address bytes
  for (size_t i = 0; i < addr_size; ++i) {
    sum += (address >> (8 * (addr_size - 1 - i))) & 0xFF;
  }

  // Add data bytes
  for (uint8_t byte : data) {
    sum += byte;
  }

  // Return one's complement
  return ~sum & 0xFF;
}

void SRecordWriter::WriteHeader(std::ostream &output,
                                const std::string &module_name) {
  // S0 record with optional module name
  std::vector<uint8_t> header_data;
  for (char c : module_name) {
    header_data.push_back(static_cast<uint8_t>(c));
  }

  WriteRecord(output, 0, 0x0000, header_data);
}

void SRecordWriter::WriteTerminator(std::ostream &output, int format,
                                    uint64_t start_address) {
  // S9 for S1, S8 for S2, S7 for S3
  int terminator_type = 10 - format; // S9=9, S8=8, S7=7
  WriteRecord(output, terminator_type, start_address, {});
}

void SRecordWriter::WriteCountRecord(std::ostream &output, size_t record_count,
                                     int /* format */) {
  // S5 for counts < 65536, S6 for larger counts
  int count_type = (record_count < 0x10000) ? 5 : 6;
  WriteRecord(output, count_type, record_count, {});
}

std::vector<std::pair<uint64_t, uint8_t>>
SRecordWriter::ExtractBytes(const Section &section) {
  std::vector<std::pair<uint64_t, uint8_t>> bytes;
  uint64_t current_address = section.org;

  for (const auto &atom : section.atoms) {
    if (auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom)) {
      // Data atom - extract bytes
      for (uint8_t byte : data_atom->data) {
        bytes.push_back({current_address, byte});
        ++current_address;
      }
    } else if (auto inst_atom =
                   std::dynamic_pointer_cast<InstructionAtom>(atom)) {
      // Instruction atom - extract encoded bytes
      for (uint8_t byte : inst_atom->encoded_bytes) {
        bytes.push_back({current_address, byte});
        ++current_address;
      }
    } else if (auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom)) {
      // Space atom - just advance address, don't output bytes
      current_address += space_atom->size;
    }
    // Labels and other atoms don't contribute bytes
  }

  return bytes;
}

size_t SRecordWriter::GetAddressSize(int format) const {
  switch (format) {
  case 0:
  case 1:
  case 5:
  case 9:
    return 2; // 16-bit address
  case 2:
  case 6:
  case 8:
    return 3; // 24-bit address
  case 3:
  case 7:
    return 4; // 32-bit address
  default:
    throw std::runtime_error("Invalid S-Record format type");
  }
}

} // namespace xasm
