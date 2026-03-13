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
  // Call WriteOutputWithRw18 with no RW18 header
  WriteOutputWithRw18(filename, sections, symbols, nullptr);
}

void BinaryOutput::WriteOutputWithRw18(const std::string &filename,
                                       const std::vector<Section *> &sections,
                                       const SymbolTable &symbols,
                                       const std::array<uint16_t, 4> *rw18_header) {
  (void)symbols; // Unused for now

  // Create output file
  std::ofstream out(filename, std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open output file: " + filename);
  }

  // Write RW18 header if provided (Merlin/Prince of Persia compatibility)
  if (rw18_header) {
    // Magic: "USR\x1a" (4 bytes)
    const uint8_t magic[4] = {'U', 'S', 'R', 0x1a};
    out.write(reinterpret_cast<const char *>(magic), 4);

    // Write 4 uint16_t arguments in little-endian format
    for (int i = 0; i < 4; ++i) {
      uint16_t arg = (*rw18_header)[i];
      uint8_t lo = arg & 0xFF;
      uint8_t hi = (arg >> 8) & 0xFF;
      out.write(reinterpret_cast<const char *>(&lo), 1);
      out.write(reinterpret_cast<const char *>(&hi), 1);
    }
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
      case AtomType::DummyOrg:
        // DummyOrgAtom: .OR inside .DUMMY/.ED — no bytes, no PC change
        break;
      case AtomType::Label:
        // LabelAtom: No bytes generated
        break;
      case AtomType::ListingControl:
        // ListingControlAtom: No bytes generated (listing only)
        break;
      case AtomType::Phase:
        // PhaseAtom: No bytes generated (address tracking only)
        break;
      case AtomType::Equate:
        // EquateAtom: No bytes generated (symbol value only)
        break;
      }
    }
  }
}

} // namespace xasm
