/**
 * @file intel_hex_writer.cpp
 * @brief Implementation of Intel HEX format writer
 *
 * This file implements the IntelHexWriter class for generating Intel HEX
 * format output from assembled sections.
 *
 * @note Part of Phase 2: Output Format System
 */

#include "xasm++/output/intel_hex_writer.h"
#include "xasm++/atom.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace xasm {

IntelHexWriter::IntelHexWriter() : bytes_per_line_(16) {}

void IntelHexWriter::Write(const std::vector<Section> &sections,
                           std::ostream &output) {
  // Track current extended address (upper 16 bits for > 64KB addresses)
  // Start with 0 so we don't emit unnecessary extended address record for low addresses
  uint16_t current_extended_address = 0;
  bool extended_address_written = false;

  // Process each section
  for (const auto &section : sections) {
    // Extract all bytes from this section
    auto bytes = ExtractBytes(section);

    if (bytes.empty()) {
      continue; // Skip empty sections
    }

    // Write bytes in chunks
    size_t i = 0;
    while (i < bytes.size()) {
      uint64_t address = bytes[i].first;

      // Check if we need an extended address record
      uint16_t extended_address = (address >> 16) & 0xFFFF;
      if (extended_address != current_extended_address || 
          (extended_address != 0 && !extended_address_written)) {
        // Only write extended address if non-zero or we've already written one
        if (extended_address != 0 || extended_address_written) {
          WriteExtendedLinearAddress(output, extended_address);
          extended_address_written = true;
        }
        current_extended_address = extended_address;
      }

      // Collect bytes for this record
      std::vector<uint8_t> record_data;
      uint64_t record_address = address;
      uint16_t record_address_low = address & 0xFFFF;

      // Gather consecutive bytes up to bytes_per_line
      while (i < bytes.size() && record_data.size() < bytes_per_line_) {
        uint64_t byte_address = bytes[i].first;

        // Check if byte is consecutive (same extended address, consecutive low
        // address)
        uint16_t byte_extended = (byte_address >> 16) & 0xFFFF;

        if (byte_extended != extended_address) {
          break; // Different extended address - need new record
        }

        uint64_t expected_address = record_address + record_data.size();
        if (byte_address != expected_address) {
          break; // Non-consecutive - need new record
        }

        record_data.push_back(bytes[i].second);
        i++;
      }

      // Write data record
      WriteRecord(output, static_cast<uint8_t>(record_data.size()),
                  record_address_low, 0x00, record_data);
    }
  }

  // Write EOF record
  WriteEOF(output);
}

std::string IntelHexWriter::GetExtension() const { return "hex"; }

std::string IntelHexWriter::GetFormatName() const { return "Intel HEX"; }

void IntelHexWriter::SetBytesPerLine(size_t bytes) {
  if (bytes == 0 || bytes > 255) {
    throw std::invalid_argument("Bytes per line must be between 1 and 255");
  }
  bytes_per_line_ = bytes;
}

void IntelHexWriter::WriteRecord(std::ostream &output, uint8_t byte_count,
                                 uint16_t address, uint8_t record_type,
                                 const std::vector<uint8_t> &data) {
  // Calculate checksum
  uint8_t checksum = CalculateChecksum(byte_count, address, record_type, data);

  // Write record
  output << ":";
  output << std::uppercase << std::hex << std::setfill('0');
  output << std::setw(2) << static_cast<int>(byte_count);
  output << std::setw(4) << static_cast<int>(address);
  output << std::setw(2) << static_cast<int>(record_type);

  for (uint8_t byte : data) {
    output << std::setw(2) << static_cast<int>(byte);
  }

  output << std::setw(2) << static_cast<int>(checksum);
  output << "\n";
}

uint8_t IntelHexWriter::CalculateChecksum(uint8_t byte_count, uint16_t address,
                                          uint8_t record_type,
                                          const std::vector<uint8_t> &data) {
  // Sum all bytes
  uint8_t sum = byte_count;
  sum += (address >> 8) & 0xFF;   // High byte of address
  sum += address & 0xFF;          // Low byte of address
  sum += record_type;

  for (uint8_t byte : data) {
    sum += byte;
  }

  // Two's complement
  return static_cast<uint8_t>((~sum + 1) & 0xFF);
}

void IntelHexWriter::WriteExtendedLinearAddress(std::ostream &output,
                                                uint16_t upper_address) {
  std::vector<uint8_t> data;
  data.push_back((upper_address >> 8) & 0xFF); // High byte
  data.push_back(upper_address & 0xFF);        // Low byte

  WriteRecord(output, 2, 0x0000, 0x04, data);
}

void IntelHexWriter::WriteEOF(std::ostream &output) {
  std::vector<uint8_t> empty_data;
  WriteRecord(output, 0, 0x0000, 0x01, empty_data);
}

std::vector<std::pair<uint64_t, uint8_t>>
IntelHexWriter::ExtractBytes(const Section &section) {
  std::vector<std::pair<uint64_t, uint8_t>> result;
  uint64_t current_address = section.org;

  for (const auto &atom : section.atoms) {
    switch (atom->type) {
    case AtomType::Data: {
      // Data atom - extract bytes
      auto data_atom = std::static_pointer_cast<DataAtom>(atom);
      for (uint8_t byte : data_atom->data) {
        result.push_back({current_address, byte});
        current_address++;
      }
      break;
    }

    case AtomType::Instruction: {
      // Instruction atom - extract encoded bytes
      auto inst_atom = std::static_pointer_cast<InstructionAtom>(atom);
      for (uint8_t byte : inst_atom->encoded_bytes) {
        result.push_back({current_address, byte});
        current_address++;
      }
      break;
    }

    case AtomType::Space: {
      // Space atom - skip (uninitialized memory)
      auto space_atom = std::static_pointer_cast<SpaceAtom>(atom);
      current_address += space_atom->count;
      break;
    }

    case AtomType::Align: {
      // Align atom - calculate padding and skip
      auto align_atom = std::static_pointer_cast<AlignAtom>(atom);
      uint64_t alignment = align_atom->alignment;
      uint64_t remainder = current_address % alignment;
      if (remainder != 0) {
        current_address += alignment - remainder;
      }
      break;
    }

    case AtomType::Org: {
      // Org atom - change address
      auto org_atom = std::static_pointer_cast<OrgAtom>(atom);
      current_address = org_atom->address;
      break;
    }

    case AtomType::Label:
      // Label atom - no bytes, no address change
      break;

    case AtomType::ListingControl:
      // Listing control atom - no bytes, no address change
      break;
    }
  }

  return result;
}

} // namespace xasm
