/**
 * @file listing_output.cpp
 * @brief Listing output plugin implementation
 *
 * This file implements the listing output plugin, which generates a human-
 * readable assembly listing showing addresses, bytes, and source lines.
 *
 * @note Phase 3.3: Output Format Plugins - Listing Output
 */

#include "xasm++/output/listing_output.h"
#include "xasm++/atom.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace xasm {

std::string ListingOutput::GetName() const { return "listing"; }

std::string ListingOutput::GetFileExtension() const { return ".lst"; }

/**
 * @brief Format a hex address with padding
 * @param addr Address value
 * @param width Field width
 * @return Formatted string
 */
static std::string FormatAddress(uint32_t addr, int width = 4) {
  std::ostringstream oss;
  oss << std::uppercase << std::hex << std::setw(width) << std::setfill('0')
      << addr;
  return oss.str();
}

/**
 * @brief Format bytes as hex string
 * @param bytes Byte data
 * @param max_bytes Maximum bytes to show per line
 * @return Formatted string
 */
static std::string FormatBytes(const std::vector<uint8_t> &bytes,
                               size_t max_bytes = 8) {
  std::ostringstream oss;
  size_t count = std::min(bytes.size(), max_bytes);
  for (size_t i = 0; i < count; ++i) {
    if (i > 0)
      oss << ' ';
    oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

void ListingOutput::WriteOutput(const std::string &filename,
                                const std::vector<Section *> &sections,
                                const SymbolTable & /* symbols */) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open output file: " + filename);
  }

  // Write header
  file << "Assembly Listing\n";
  file << "================\n\n";
  file << "Address  Bytes              Source\n";
  file << "-------  -----------------  ------\n";

  // Process each section
  for (const auto *section : sections) {
    if (!section->atoms.empty()) {
      file << "\n; Section: " << section->name << "\n";
    }

    uint32_t current_address = section->org;

    // Process each atom in the section
    for (const auto &atom : section->atoms) {
      // Handle different atom types
      if (auto *label = dynamic_cast<const LabelAtom *>(atom.get())) {
        // Label atom - show label name
        file << FormatAddress(current_address) << "                        "
             << label->name << ":\n";
      } else if (auto *inst =
                     dynamic_cast<const InstructionAtom *>(atom.get())) {
        // Instruction atom - show address, bytes, and mnemonic
        std::string bytes_str = FormatBytes(inst->encoded_bytes);
        file << FormatAddress(current_address) << "     " << std::left
             << std::setw(17) << bytes_str << "  " << inst->mnemonic;
        if (!inst->operand.empty()) {
          file << " " << inst->operand;
        }
        file << "\n";
        current_address += inst->size;
      } else if (auto *data = dynamic_cast<const DataAtom *>(atom.get())) {
        // Data atom - show address and bytes
        std::string bytes_str = FormatBytes(data->data);
        file << FormatAddress(current_address) << "     " << std::left
             << std::setw(17) << bytes_str << "\n";
        current_address += data->data.size();
      } else if (auto *org = dynamic_cast<const OrgAtom *>(atom.get())) {
        // Org directive - update current address
        current_address = org->address;
        file << FormatAddress(current_address) << "                        "
             << ".ORG $" << FormatAddress(org->address) << "\n";
      }
      // Skip alignment, reserve, and other non-data atoms
    }
  }

  file << "\n; End of listing\n";
  file.close();
}

} // namespace xasm
