/**
 * @file coco_loadm_writer.cpp
 * @brief Implementation of CoCo DOS (LOADM) binary format writer
 */

#include "xasm++/output/coco_loadm_writer.h"
#include "xasm++/atom.h"

#include <stdexcept>

namespace xasm {

CocoLoadmWriter::CocoLoadmWriter()
    : has_entry_point_(false), entry_point_addr_(0) {}

void CocoLoadmWriter::Write(const std::vector<Section> &sections,
                            std::ostream &output) {
  // Extract all bytes from all sections
  std::vector<std::pair<uint64_t, uint8_t>> all_bytes;
  for (const auto &section : sections) {
    auto section_bytes = ExtractBytes(section);
    all_bytes.insert(all_bytes.end(), section_bytes.begin(),
                     section_bytes.end());
  }

  // Calculate first address and total length for preamble
  uint64_t first_address = all_bytes.empty() ? 0 : all_bytes.front().first;
  size_t total_length = all_bytes.size();

  // Write preamble
  WritePreamble(output, first_address, total_length);

  // Group bytes into contiguous data blocks
  size_t i = 0;
  while (i < all_bytes.size()) {
    uint64_t start_address = all_bytes[i].first;
    std::vector<uint8_t> block_data;

    // Collect contiguous bytes
    while (i < all_bytes.size()) {
      uint64_t expected_addr = start_address + block_data.size();
      if (all_bytes[i].first != expected_addr) {
        // Address discontinuity - start new block
        break;
      }
      block_data.push_back(all_bytes[i].second);
      ++i;
    }

    // Write data block
    WriteDataBlock(output, start_address, block_data);
  }

  // Write postamble
  WritePostamble(output);
}

std::string CocoLoadmWriter::GetExtension() const { return "bin"; }

std::string CocoLoadmWriter::GetFormatName() const {
  return "CoCo DOS (LOADM)";
}

void CocoLoadmWriter::SetEntryPoint(uint64_t address) {
  has_entry_point_ = true;
  entry_point_addr_ = address;
}

void CocoLoadmWriter::WritePreamble(std::ostream &output,
                                    uint64_t first_address,
                                    size_t total_length) {
  // Validate address fits in 16-bit
  if (first_address > 0xFFFF) {
    throw std::runtime_error("CoCo DOS format: Address exceeds 16-bit limit");
  }

  output.put(0x00); // Preamble type
  WriteBE16(output, static_cast<uint16_t>(total_length));
  WriteBE16(output, static_cast<uint16_t>(first_address));
}

void CocoLoadmWriter::WriteDataBlock(std::ostream &output, uint64_t address,
                                     const std::vector<uint8_t> &data) {
  // Validate address fits in 16-bit
  if (address > 0xFFFF) {
    throw std::runtime_error("CoCo DOS format: Address exceeds 16-bit limit");
  }

  output.put(0x00); // Data block type
  WriteBE16(output, static_cast<uint16_t>(data.size()));
  WriteBE16(output, static_cast<uint16_t>(address));

  // Write data bytes
  for (uint8_t byte : data) {
    output.put(static_cast<char>(byte));
  }
}

void CocoLoadmWriter::WritePostamble(std::ostream &output) {
  output.put(static_cast<char>(0xFF)); // Postamble type
  output.put(0x00);                    // Subtype

  if (has_entry_point_) {
    // Validate address fits in 16-bit
    if (entry_point_addr_ > 0xFFFF) {
      throw std::runtime_error(
          "CoCo DOS format: Entry point exceeds 16-bit limit");
    }

    output.put(0x00); // Padding
    WriteBE16(output, static_cast<uint16_t>(entry_point_addr_));
  }
}

void CocoLoadmWriter::WriteBE16(std::ostream &output, uint16_t value) {
  output.put(static_cast<char>((value >> 8) & 0xFF)); // High byte
  output.put(static_cast<char>(value & 0xFF));        // Low byte
}

std::vector<std::pair<uint64_t, uint8_t>>
CocoLoadmWriter::ExtractBytes(const Section &section) {
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

} // namespace xasm
