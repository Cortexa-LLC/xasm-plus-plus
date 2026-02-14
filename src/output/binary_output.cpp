// BinaryOutput - Binary output plugin implementation
// Phase 1: Minimal Viable Assembler - Binary Output Plugin

#include "xasm++/output/binary_output.h"
#include "xasm++/output/output_format_constants.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace xasm {

std::string BinaryOutput::GetName() const { return "binary"; }

std::string BinaryOutput::GetFileExtension() const { return ".bin"; }

void BinaryOutput::WriteOutput(const std::string &filename,
                               const std::vector<Section *> &sections,
                               const SymbolTable &symbols) {
  (void)symbols; // Unused for now

  // Create output file
  std::ofstream out(filename, std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open output file: " + filename);
  }

  // Process each section
  for (const auto *section : sections) {
    size_t position = 0; // Track position for alignment

    // Process atoms in section
    for (const auto &atom : section->atoms) {
      switch (atom->type) {
      case AtomType::Data: {
        // Serialize DataAtom
        auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
        if (data_atom) {
          out.write(reinterpret_cast<const char *>(data_atom->data.data()),
                    data_atom->data.size());
          position += data_atom->data.size();
        }
        break;
      }

      case AtomType::Space: {
        // Serialize SpaceAtom (write zeros)
        auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
        if (space_atom) {
          std::vector<uint8_t> zeros(space_atom->count,
                                      output_format::binary::ZERO_BYTE);
          out.write(reinterpret_cast<const char *>(zeros.data()), zeros.size());
          position += space_atom->count;
        }
        break;
      }

      case AtomType::Align: {
        // Serialize AlignAtom (write padding)
        auto align_atom = std::dynamic_pointer_cast<AlignAtom>(atom);
        if (align_atom) {
          size_t alignment = align_atom->alignment;
          size_t padding = (alignment - (position % alignment)) % alignment;
          if (padding > 0) {
            std::vector<uint8_t> pad_bytes(padding,
                                            output_format::binary::ZERO_BYTE);
            out.write(reinterpret_cast<const char *>(pad_bytes.data()),
                      pad_bytes.size());
            position += padding;
          }
        }
        break;
      }

      case AtomType::Instruction: {
        // Serialize InstructionAtom (if CPU plugin has encoded it)
        auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(atom);
        if (inst_atom && !inst_atom->encoded_bytes.empty()) {
          out.write(
              reinterpret_cast<const char *>(inst_atom->encoded_bytes.data()),
              inst_atom->encoded_bytes.size());
          position += inst_atom->encoded_bytes.size();
        }
        break;
      }

      case AtomType::Org:
        // OrgAtom: Track address but don't write bytes
        break;
      case AtomType::Label:
        // LabelAtom: No bytes generated
        break;
      case AtomType::ListingControl:
        // ListingControlAtom: No bytes generated (listing only)
        break;
      }
    }
  }
}

} // namespace xasm
