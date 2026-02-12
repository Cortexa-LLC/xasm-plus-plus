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
#include <iostream>
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

  std::string page_title = "Assembly Listing";
  bool listing_enabled = true; // LIST/NOLIST control
  
  // Write header
  file << page_title << "\n";
  file << std::string(page_title.length(), '=') << "\n\n";
  file << "Line   Address  Bytes              Source\n";
  file << "-----  -------  -----------------  ------\n";

  // Process each section
  for (const auto *section : sections) {
    if (!section->atoms.empty()) {
      file << "\n; Section: " << section->name << "\n";
    }

    uint32_t current_address = section->org;

    // Process each atom in the section
    for (size_t i = 0; i < section->atoms.size(); ++i) {
      const auto &atom = section->atoms[i];
      // Handle listing control directives
      if (auto *ctrl = dynamic_cast<const ListingControlAtom *>(atom.get())) {
        bool output_source = false;
        
        switch (ctrl->control_type) {
        case ListingControlType::Title:
          // Update page title for future pages
          page_title = ctrl->value;
          output_source = true; // Show TITLE directive in listing
          break;
        case ListingControlType::Subtitle:
          // Subtitles could be appended to page title or displayed separately
          // For now, just show the directive in listing
          output_source = true;
          break;
        case ListingControlType::List:
          listing_enabled = true;
          output_source = true; // Show LIST directive in listing
          break;
        case ListingControlType::Nolist:
          listing_enabled = false;
          output_source = true; // Show NOLIST directive in listing
          break;
        case ListingControlType::Page:
          // Insert page break
          file << "\f"; // Form feed character
          file << page_title << "\n";
          file << std::string(page_title.length(), '=') << "\n\n";
          file << "Line   Address  Bytes              Source\n";
          file << "-----  -------  -----------------  ------\n";
          break;
        case ListingControlType::Space:
          // Insert blank lines
          for (int i = 0; i < ctrl->count; ++i) {
            file << "\n";
          }
          break;
        case ListingControlType::Lall:
        case ListingControlType::Sall:
          // TODO: Macro expansion control
          output_source = true;
          break;
        }
        
        // Output source line for directives if requested
        if (output_source && !ctrl->source_line.empty()) {
          std::string line_num = "     ";
          if (ctrl->location.line > 0) {
            std::ostringstream oss;
            oss << std::setw(5) << std::right << ctrl->location.line;
            line_num = oss.str();
          }
          file << line_num << "                            " << ctrl->source_line << "\n";
        }
        
        continue;
      }
      
      // Skip if listing is disabled
      if (!listing_enabled) {
        // Still need to track address for instructions/data
        if (auto *inst = dynamic_cast<const InstructionAtom *>(atom.get())) {
          current_address += inst->size;
        } else if (auto *data = dynamic_cast<const DataAtom *>(atom.get())) {
          current_address += data->data.size();
        }
        continue;
      }
      
      // Format line number (5 digits, right-aligned)
      std::string line_num = "     ";
      if (atom->location.line > 0) {
        std::ostringstream oss;
        oss << std::setw(5) << std::right << atom->location.line;
        line_num = oss.str();
      }
      
      // Use source_line if available, otherwise construct from atom data
      std::string source_text;
      if (!atom->source_line.empty()) {
        source_text = atom->source_line;
      } else {
        // Fallback to constructing from atom type
        if (auto *lbl = dynamic_cast<const LabelAtom *>(atom.get())) {
          source_text = lbl->name + ":";
        } else if (auto *inst = dynamic_cast<const InstructionAtom *>(atom.get())) {
          source_text = inst->mnemonic;
          if (!inst->operand.empty()) {
            source_text += " " + inst->operand;
          }
        }
      }
      
      // Handle different atom types
      if (auto *lbl = dynamic_cast<const LabelAtom *>(atom.get())) {
        // Label atom - check if next atom is on same line
        // If so, skip the label (it will be shown with the instruction)
        bool skip_label = false;
        if (i + 1 < section->atoms.size() && lbl->location.line > 0) {
          const auto &next_atom = section->atoms[i + 1];
          if (next_atom->location.line == lbl->location.line) {
            skip_label = true;
          }
        }
        
        if (!skip_label) {
          // Show standalone label
          file << line_num << "  " << FormatAddress(current_address)
               << "                        " << source_text << "\n";
        }
      } else if (auto *inst =
                     dynamic_cast<const InstructionAtom *>(atom.get())) {
        // Instruction atom - show address, bytes, and source
        std::string bytes_str = FormatBytes(inst->encoded_bytes);
        file << line_num << "  " << FormatAddress(current_address) << "     "
             << std::left << std::setw(17) << bytes_str << "  " << source_text
             << "\n";
        current_address += inst->encoded_bytes.size();
      } else if (auto *data = dynamic_cast<const DataAtom *>(atom.get())) {
        // Data atom - show address and bytes
        std::string bytes_str = FormatBytes(data->data);
        file << line_num << "  " << FormatAddress(current_address) << "     "
             << std::left << std::setw(17) << bytes_str << "  " << source_text
             << "\n";
        current_address += data->data.size();
      } else if (auto *org = dynamic_cast<const OrgAtom *>(atom.get())) {
        // Org directive - update current address BEFORE outputting the line
        current_address = org->address;
        file << line_num << "  " << FormatAddress(org->address)
             << "                        "
             << (source_text.empty() ? (".ORG $" + FormatAddress(org->address))
                                     : source_text)
             << "\n";
      }
      // Skip alignment, reserve, and other non-data atoms
    }
  }

  file << "\n; End of listing\n";
  file.close();
}

} // namespace xasm
