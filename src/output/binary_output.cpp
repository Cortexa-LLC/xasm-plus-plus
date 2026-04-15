/**
 * @file binary_output.cpp
 * @brief Binary output plugin implementation
 *
 * Phase 1: Minimal Viable Assembler - Binary Output Plugin
 */

#include "xasm++/output/binary_output.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "xasm++/output/output_format_constants.h"
#include "xasm++/section.h"
#include "xasm++/symbol.h"

namespace xasm {

namespace {

// Compute the total byte count for sections (used for the RW18 header length).
static uint32_t ComputeCodeSize(const std::vector<Section*>& sections) {
  uint32_t code_size = 0;
  for (const auto* section : sections) {
    for (const auto& atom : section->atoms) {
      if (atom->type == AtomType::Data) {
        auto d = std::dynamic_pointer_cast<DataAtom>(atom);
        if (d) {
          code_size += static_cast<uint32_t>(d->data.size());
        }
      } else if (atom->type == AtomType::Instruction) {
        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
        if (inst) {
          code_size += static_cast<uint32_t>(inst->encoded_bytes.size());
        }
      } else if (atom->type == AtomType::Space) {
        auto sp = std::dynamic_pointer_cast<SpaceAtom>(atom);
        if (sp) {
          code_size += sp->count;
        }
      }
    }
  }
  return code_size;
}

// Write the 12-byte RW18 file header (USR signature + three 16-bit fields +
// actual code size).
static void WriteRw18Header(std::ofstream& out, const std::array<uint16_t, 4>& rw18_args,
                            uint32_t code_size) {
  auto write_u16le = [&](uint16_t v) {
    uint8_t buf[2] = {static_cast<uint8_t>(v & 0xFF), static_cast<uint8_t>((v >> 8) & 0xFF)};
    out.write(reinterpret_cast<const char*>(buf), 2);
  };
  const uint8_t kMagic[4] = {'U', 'S', 'R', 0x1a};
  out.write(reinterpret_cast<const char*>(kMagic), 4);
  write_u16le(rw18_args[0]);                      // side   = bundle ID
  write_u16le(rw18_args[1]);                      // track  = disk track
  write_u16le(rw18_args[2]);                      // offset = intra-track offset
  write_u16le(static_cast<uint16_t>(code_size));  // length = actual code size
}

// Write a single atom to the output stream. Updates position in place.
static void WriteAtom(std::ofstream& out, const std::shared_ptr<Atom>& atom, size_t& position) {
  switch (atom->type) {
    case AtomType::Data: {
      auto data_atom = std::dynamic_pointer_cast<DataAtom>(atom);
      if (data_atom) {
        out.write(reinterpret_cast<const char*>(data_atom->data.data()),
                  static_cast<std::streamsize>(data_atom->data.size()));
        position += data_atom->data.size();
      }
      break;
    }
    case AtomType::Space: {
      auto space_atom = std::dynamic_pointer_cast<SpaceAtom>(atom);
      if (space_atom) {
        std::vector<uint8_t> fill_buf(space_atom->count, space_atom->fill);
        out.write(reinterpret_cast<const char*>(fill_buf.data()),
                  static_cast<std::streamsize>(fill_buf.size()));
        position += space_atom->count;
      }
      break;
    }
    case AtomType::Align: {
      auto align_atom = std::dynamic_pointer_cast<AlignAtom>(atom);
      if (align_atom) {
        size_t alignment = align_atom->alignment;
        size_t padding = (alignment - (position % alignment)) % alignment;
        if (padding > 0) {
          std::vector<uint8_t> pad_bytes(padding, output_format::binary::ZERO_BYTE);
          out.write(reinterpret_cast<const char*>(pad_bytes.data()),
                    static_cast<std::streamsize>(pad_bytes.size()));
          position += padding;
        }
      }
      break;
    }
    case AtomType::Instruction: {
      auto inst_atom = std::dynamic_pointer_cast<InstructionAtom>(atom);
      if (inst_atom && !inst_atom->encoded_bytes.empty()) {
        out.write(reinterpret_cast<const char*>(inst_atom->encoded_bytes.data()),
                  static_cast<std::streamsize>(inst_atom->encoded_bytes.size()));
        position += inst_atom->encoded_bytes.size();
      }
      break;
    }
    case AtomType::Org:             // Track address — no bytes written.
    case AtomType::DummyOrg:        // .OR inside .DUMMY/.ED — no bytes.
    case AtomType::Label:           // LabelAtom — no bytes.
    case AtomType::ListingControl:  // Listing only — no bytes.
    case AtomType::Phase:           // Address tracking only.
    case AtomType::Equate:          // Symbol value only.
    case AtomType::CpuMode:         // Mode change only.
    case AtomType::MxState:         // M/X state change only.
      break;
  }
}

}  // anonymous namespace

std::string BinaryOutput::GetName() const {
  return "binary";
}

std::string BinaryOutput::GetFileExtension() const {
  return ".bin";
}

void BinaryOutput::WriteOutput(const std::string& filename, const std::vector<Section*>& sections,
                               const SymbolTable& symbols) {
  WriteOutputWithRw18(filename, sections, symbols, nullptr);
}

void BinaryOutput::WriteOutputWithRw18(const std::string& filename,
                                       const std::vector<Section*>& sections,
                                       const SymbolTable& symbols,
                                       const std::array<uint16_t, 4>* rw18_header) {
  (void)symbols;  // Unused for now

  std::ofstream out(filename, std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open output file: " + filename);
  }

  // Write RW18 header if provided (Merlin/Prince-of-Persia compatibility).
  if (rw18_header) {
    uint32_t code_size = ComputeCodeSize(sections);
    WriteRw18Header(out, *rw18_header, code_size);
  }

  // Write all atom bytes for every section.
  for (const auto* section : sections) {
    size_t position = 0;
    for (const auto& atom : section->atoms) {
      WriteAtom(out, atom, position);
    }
  }
}

}  // namespace xasm
