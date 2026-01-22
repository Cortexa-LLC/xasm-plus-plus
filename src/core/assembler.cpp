// Assembler implementation

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/symbol.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xasm {

// Helper: Parse hex value ($1234 or $42)
static uint32_t ParseHex(const std::string& str) {
    if (str.empty() || str[0] != '$') {
        return 0;
    }
    return std::stoul(str.substr(1), nullptr, 16);
}

// Helper: Trim whitespace
static std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

// Helper: Determine addressing mode from operands string
static AddressingMode DetermineAddressingMode(const std::string& operands) {
    if (operands.empty()) {
        return AddressingMode::Implied;
    }

    std::string trimmed = Trim(operands);

    // Immediate: #$42
    if (trimmed[0] == '#') {
        return AddressingMode::Immediate;
    }

    // Absolute or zero page: $1234 or $80
    if (trimmed[0] == '$') {
        uint32_t value = ParseHex(trimmed);
        if (value <= 0xFF) {
            return AddressingMode::ZeroPage;
        }
        return AddressingMode::Absolute;
    }

    // Default to absolute (label reference)
    return AddressingMode::Absolute;
}

void Assembler::SetCpuPlugin(Cpu6502* cpu) {
    cpu_ = cpu;
}

void Assembler::SetSymbolTable(SymbolTable* symbols) {
    symbols_ = symbols;
}

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

    // Encode instructions using CPU plugin
    if (cpu_ != nullptr) {
        for (auto& section : sections_) {
            for (auto& atom : section.atoms) {
                if (atom->type == AtomType::Instruction) {
                    auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                    if (inst && inst->encoded_bytes.empty()) {
                        // Encode the instruction
                        std::string mnemonic = inst->mnemonic;
                        std::string operand = inst->operand;

                        // Parse operand to get addressing mode and value
                        AddressingMode mode = DetermineAddressingMode(operand);
                        uint16_t value = 0;

                        // Extract operand value
                        if (!operand.empty()) {
                            std::string trimmed = Trim(operand);
                            if (trimmed[0] == '#') {
                                // Immediate: #$42
                                value = static_cast<uint16_t>(ParseHex(trimmed.substr(1)));
                            } else if (trimmed[0] == '$') {
                                // Absolute/Zero Page: $1234
                                value = static_cast<uint16_t>(ParseHex(trimmed));
                            } else {
                                // Label reference - look up in symbol table
                                if (symbols_ != nullptr) {
                                    int64_t symbol_value;
                                    if (symbols_->Lookup(trimmed, symbol_value)) {
                                        value = static_cast<uint16_t>(symbol_value);
                                    }
                                }
                            }
                        }

                        // Call appropriate CPU encoding method
                        try {
                            if (mnemonic == "NOP") {
                                inst->encoded_bytes = cpu_->EncodeNOP();
                            } else if (mnemonic == "RTS") {
                                inst->encoded_bytes = cpu_->EncodeRTS();
                            } else if (mnemonic == "LDA") {
                                inst->encoded_bytes = cpu_->EncodeLDA(value, mode);
                            } else if (mnemonic == "STA") {
                                inst->encoded_bytes = cpu_->EncodeSTA(value, mode);
                            } else if (mnemonic == "JMP") {
                                inst->encoded_bytes = cpu_->EncodeJMP(value, mode);
                            } else {
                                // Unknown instruction
                                AssemblerError error;
                                error.message = "Unknown instruction: " + mnemonic;
                                result.errors.push_back(error);
                                result.success = false;
                            }
                        } catch (const std::exception& e) {
                            // Encoding error
                            AssemblerError error;
                            error.message = "Encoding error for " + mnemonic + ": " + e.what();
                            result.errors.push_back(error);
                            result.success = false;
                        }
                    }
                }
            }
        }
    }

    // For Phase 1, we do a simple single pass
    // Multi-pass logic will be enhanced in later phases
    result.pass_count = 1;

    return result;
}

} // namespace xasm
