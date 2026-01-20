// Assembler implementation

#include "xasm++/assembler.h"

namespace xasm {

void Assembler::AddSection(const Section& section) {
    sections_.push_back(section);
}

size_t Assembler::GetSectionCount() const {
    return sections_.size();
}

void Assembler::Reset() {
    sections_.clear();
}

AssemblerResult Assembler::Assemble() {
    AssemblerResult result;

    // Empty assembly
    if (sections_.empty()) {
        result.success = true;
        result.pass_count = 0;
        return result;
    }

    // For Phase 1, we do a simple single pass
    // Multi-pass logic will be enhanced in later phases with CPU plugins
    result.success = true;
    result.pass_count = 1;

    return result;
}

} // namespace xasm
