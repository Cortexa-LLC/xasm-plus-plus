// Atom - Intermediate Representation (IR)
// Inspired by vasm-ext atom chain design

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace xasm {

// Atom types
enum class AtomType {
    Label,          // Symbol definition
    Instruction,    // CPU instruction
    Data,           // Raw data bytes
    Space,          // Reserved/uninitialized space
    Align,          // Alignment directive
    Org,            // Origin directive
};

// Source location (for error reporting)
struct SourceLocation {
    std::string filename;
    int line;
    int column;

    SourceLocation() : line(0), column(0) {}
    SourceLocation(const std::string& file, int l, int c)
        : filename(file), line(l), column(c) {}
};

// Base Atom class
class Atom {
public:
    AtomType type;
    SourceLocation location;
    size_t size;                        // Size in bytes
    size_t last_size;                   // Previous pass size
    uint32_t changes;                   // Size change count
    std::shared_ptr<Atom> next;         // Linked list

    explicit Atom(AtomType t)
        : type(t), size(0), last_size(0), changes(0), next(nullptr) {}

    virtual ~Atom() = default;
};

// Label atom - symbol definition
class LabelAtom : public Atom {
public:
    std::string name;
    uint32_t address;

    LabelAtom(const std::string& n, uint32_t addr)
        : Atom(AtomType::Label), name(n), address(addr) {
        size = 0;  // Labels don't take space
    }
};

// Instruction atom - CPU instruction
class InstructionAtom : public Atom {
public:
    std::string mnemonic;
    std::string operand;
    std::vector<uint8_t> encoded_bytes;  // Encoded instruction bytes (set by CPU plugin)
    // Will be enhanced with opcode info later

    InstructionAtom(const std::string& mnem, const std::string& oper)
        : Atom(AtomType::Instruction), mnemonic(mnem), operand(oper) {
        // Size determined during encoding phase
    }
};

// Data atom - raw bytes
class DataAtom : public Atom {
public:
    std::vector<uint8_t> data;

    explicit DataAtom(const std::vector<uint8_t>& d)
        : Atom(AtomType::Data), data(d) {
        size = data.size();
    }
};

// Space atom - reserved space
class SpaceAtom : public Atom {
public:
    size_t count;

    explicit SpaceAtom(size_t c)
        : Atom(AtomType::Space), count(c) {
        size = count;
    }
};

// Align atom - alignment directive
class AlignAtom : public Atom {
public:
    size_t alignment;

    explicit AlignAtom(size_t align)
        : Atom(AtomType::Align), alignment(align) {
        // Size determined during layout phase
    }
};

// Org atom - origin directive
class OrgAtom : public Atom {
public:
    uint32_t address;

    explicit OrgAtom(uint32_t addr)
        : Atom(AtomType::Org), address(addr) {
        size = 0;  // ORG doesn't generate bytes
    }
};

} // namespace xasm
