// Assembler - Multi-pass assembler engine
// Phase 1: Minimal Viable Assembler - Assembler Engine

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "xasm++/atom.h"
#include "xasm++/section.h"

namespace xasm {

// Assembler error
struct AssemblerError {
    std::string message;
    SourceLocation location;
};

// Assembler result
struct AssemblerResult {
    bool success = true;
    int pass_count = 0;
    std::vector<AssemblerError> errors;
};

// Assembler engine - handles multi-pass resolution
class Assembler {
public:
    // Pass limits (from vasm-ext architecture)
    static constexpr int FAST_PHASE_LIMIT = 50;
    static constexpr int MAX_PASSES = 500;

    Assembler() = default;

    // Add a section to assemble
    void AddSection(const Section& section);

    // Get number of sections
    size_t GetSectionCount() const;

    // Reset assembler (clear sections)
    void Reset();

    // Assemble all sections with multi-pass resolution
    AssemblerResult Assemble();

private:
    std::vector<Section> sections_;
};

} // namespace xasm
