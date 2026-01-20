// Section - Section management for multi-section assembly
// Phase 1: Minimal Viable Assembler - Core Abstractions

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "xasm++/atom.h"

namespace xasm {

// Section attribute flags (can be combined with bitwise OR)
enum class SectionAttributes {
    Code    = 0x01,    // Code section
    Data    = 0x02,    // Data section
    Bss     = 0x04,    // Uninitialized data
    Read    = 0x08,    // Readable
    Write   = 0x10,    // Writable
    Execute = 0x20,    // Executable
};

// Section (represents a single output section)
class Section {
public:
    std::string name;
    uint32_t attributes = 0;
    uint64_t org = 0;                           // Origin address
    std::vector<std::shared_ptr<Atom>> atoms;
    size_t current_offset = 0;

    Section() = default;

    Section(const std::string& n, uint32_t attr, uint64_t origin = 0)
        : name(n), attributes(attr), org(origin), current_offset(0) {}
};

} // namespace xasm
