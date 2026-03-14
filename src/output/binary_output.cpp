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
  // Header format (snapNcrackle RW18SavFileHeader):
  //   signature[4]: "USR\x1a"
  //   length:  actual binary code size (uint16_t LE)
  //   side:    USR arg 0 — bundle ID byte written into each RW18 sector
  //   track:   USR arg 1 — physical disk track
  //   offset:  USR arg 2 — intra-track byte offset
  // USR arg 3 (parse-time length) is discarded; actual size used instead.
  if (rw18_header) {
    // Compute actual binary code size first
    uint32_t code_size = 0;
    for (const auto *section : sections) {
      for (const auto &atom : section->atoms) {
        if (atom->type == AtomType::Data) {
          auto d = std::dynamic_pointer_cast<DataAtom>(atom);
          if (d) code_size += d->data.size();
        } else if (atom->type == AtomType::Instruction) {
          auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
          if (inst) code_size += inst->encoded_bytes.size();
        } else if (atom->type == AtomType::Space) {
          auto sp = std::dynamic_pointer_cast<SpaceAtom>(atom);
          if (sp) code_size += sp->count;
        }
      }
    }

    auto write_u16le = [&](uint16_t v) {
      uint8_t buf[2] = {static_cast<uint8_t>(v & 0xFF),
                        static_cast<uint8_t>((v >> 8) & 0xFF)};
      out.write(reinterpret_cast<const char *>(buf), 2);
    };

    const uint8_t magic[4] = {'U', 'S', 'R', 0x1a};
    out.write(reinterpret_cast<const char *>(magic), 4);
    write_u16le((*rw18_header)[0]);                // side   = bundle ID
    write_u16le((*rw18_header)[1]);                // track  = disk track
    write_u16le((*rw18_header)[2]);                // offset = intra-track offset
    write_u16le(static_cast<uint16_t>(code_size)); // length = actual code size (LAST)
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
      case AtomType::CpuMode:
        // CpuModeAtom: No bytes generated (mode change only)
        break;
      case AtomType::MxState:
        // MxAtom: No bytes generated (M/X state change only)
        break;
      }
    }
  }
}

} // namespace xasm
