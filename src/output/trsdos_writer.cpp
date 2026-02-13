/**
 * @file trsdos_writer.cpp
 * @brief Implementation of TRS-DOS binary format writer
 */

#include "xasm++/output/trsdos_writer.h"
#include "xasm++/atom.h"

#include <stdexcept>

namespace xasm {

TrsDosWriter::TrsDosWriter() : has_entry_point_(false), entry_point_addr_(0) {}

void TrsDosWriter::Write(const std::vector<Section> &sections,
                         std::ostream &output) {
  // Extract all bytes from all sections
  std::vector<std::pair<uint64_t, uint8_t>> all_bytes;
  for (const auto &section : sections) {
    auto section_bytes = ExtractBytes(section);
    all_bytes.insert(all_bytes.end(), section_bytes.begin(),
                     section_bytes.end());
  }

  if (all_bytes.empty() && !has_entry_point_) {
    return; // Nothing to output
  }

  // Group bytes into contiguous segments
  size_t i = 0;
  while (i < all_bytes.size()) {
    uint64_t start_address = all_bytes[i].first;
    std::vector<uint8_t> segment_data;

    // Collect contiguous bytes
    while (i < all_bytes.size()) {
      uint64_t expected_addr = start_address + segment_data.size();
      if (all_bytes[i].first != expected_addr) {
        // Address discontinuity - start new segment
        break;
      }
      segment_data.push_back(all_bytes[i].second);
      ++i;
    }

    // Write data segment
    WriteSegment(output, 0x01, start_address, segment_data);
  }

  // Write entry point segment if specified
  if (has_entry_point_) {
    WriteSegment(output, 0x02, entry_point_addr_, {});
  }
}

std::string TrsDosWriter::GetExtension() const { return "cmd"; }

std::string TrsDosWriter::GetFormatName() const { return "TRS-DOS Binary"; }

void TrsDosWriter::SetEntryPoint(uint64_t address) {
  has_entry_point_ = true;
  entry_point_addr_ = address;
}

void TrsDosWriter::WriteSegment(std::ostream &output, uint8_t type,
                                uint64_t address,
                                const std::vector<uint8_t> &data) {
  // Validate address fits in 16-bit
  if (address > 0xFFFF) {
    throw std::runtime_error("TRS-DOS format: Address exceeds 16-bit limit");
  }

  // Write segment header
  output.put(static_cast<char>(type));
  WriteLE16(output, static_cast<uint16_t>(address));
  WriteLE16(output, static_cast<uint16_t>(data.size()));

  // Write data bytes
  for (uint8_t byte : data) {
    output.put(static_cast<char>(byte));
  }
}

void TrsDosWriter::WriteLE16(std::ostream &output, uint16_t value) {
  output.put(static_cast<char>(value & 0xFF));        // Low byte
  output.put(static_cast<char>((value >> 8) & 0xFF)); // High byte
}

std::vector<std::pair<uint64_t, uint8_t>>
TrsDosWriter::ExtractBytes(const Section &section) {
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
