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

    // Create internal symbol table if needed
    // Always extract labels from atoms to get correct addresses
    ConcreteSymbolTable internal_symbols;
    ConcreteSymbolTable* label_table_ptr = nullptr;

    if (symbols_ != nullptr) {
        // If external symbol table is ConcreteSymbolTable, use it for label updates
        label_table_ptr = dynamic_cast<ConcreteSymbolTable*>(symbols_);
    } else {
        // Otherwise use internal table
        label_table_ptr = &internal_symbols;
    }

    // Multi-pass assembly loop
    bool converged = false;
    int pass = 0;
    std::vector<size_t> previous_sizes;

    while (!converged && pass < MAX_PASSES) {
        pass++;

        // Pass 1: Encode instructions using CPU plugin
        std::vector<size_t> current_sizes;
        if (cpu_ != nullptr) {
            for (auto& section : sections_) {
                for (auto& atom : section.atoms) {
                    if (atom->type == AtomType::Instruction) {
                        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                        if (inst) {
                            // Clear previous encoding
                            inst->encoded_bytes.clear();

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
                                    SymbolTable* lookup_table = symbols_ ? symbols_ : label_table_ptr;
                                    if (lookup_table != nullptr) {
                                        int64_t symbol_value;
                                        if (lookup_table->Lookup(trimmed, symbol_value)) {
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
                                } else if (mnemonic == "ADC") {
                                    inst->encoded_bytes = cpu_->EncodeADC(value, mode);
                                } else if (mnemonic == "SBC") {
                                    inst->encoded_bytes = cpu_->EncodeSBC(value, mode);
                                } else if (mnemonic == "AND") {
                                    inst->encoded_bytes = cpu_->EncodeAND(value, mode);
                                } else if (mnemonic == "ORA") {
                                    inst->encoded_bytes = cpu_->EncodeORA(value, mode);
                                } else if (mnemonic == "EOR") {
                                    inst->encoded_bytes = cpu_->EncodeEOR(value, mode);
                                } else if (mnemonic == "LDX") {
                                    inst->encoded_bytes = cpu_->EncodeLDX(value, mode);
                                } else if (mnemonic == "LDY") {
                                    inst->encoded_bytes = cpu_->EncodeLDY(value, mode);
                                } else if (mnemonic == "STX") {
                                    inst->encoded_bytes = cpu_->EncodeSTX(value, mode);
                                } else if (mnemonic == "STY") {
                                    inst->encoded_bytes = cpu_->EncodeSTY(value, mode);
                                } else if (mnemonic == "CMP") {
                                    inst->encoded_bytes = cpu_->EncodeCMP(value, mode);
                                } else if (mnemonic == "CPX") {
                                    inst->encoded_bytes = cpu_->EncodeCPX(value, mode);
                                } else if (mnemonic == "CPY") {
                                    inst->encoded_bytes = cpu_->EncodeCPY(value, mode);
                                } else if (mnemonic == "BEQ") {
                                    inst->encoded_bytes = cpu_->EncodeBEQ(value, mode);
                                } else if (mnemonic == "BNE") {
                                    inst->encoded_bytes = cpu_->EncodeBNE(value, mode);
                                } else if (mnemonic == "BCC") {
                                    inst->encoded_bytes = cpu_->EncodeBCC(value, mode);
                                } else if (mnemonic == "BCS") {
                                    inst->encoded_bytes = cpu_->EncodeBCS(value, mode);
                                } else if (mnemonic == "BMI") {
                                    inst->encoded_bytes = cpu_->EncodeBMI(value, mode);
                                } else if (mnemonic == "BPL") {
                                    inst->encoded_bytes = cpu_->EncodeBPL(value, mode);
                                } else if (mnemonic == "BVC") {
                                    inst->encoded_bytes = cpu_->EncodeBVC(value, mode);
                                } else if (mnemonic == "BVS") {
                                    inst->encoded_bytes = cpu_->EncodeBVS(value, mode);
                                } else if (mnemonic == "INX") {
                                    inst->encoded_bytes = cpu_->EncodeINX();
                                } else if (mnemonic == "INY") {
                                    inst->encoded_bytes = cpu_->EncodeINY();
                                } else if (mnemonic == "DEX") {
                                    inst->encoded_bytes = cpu_->EncodeDEX();
                                } else if (mnemonic == "DEY") {
                                    inst->encoded_bytes = cpu_->EncodeDEY();
                                } else if (mnemonic == "INC") {
                                    inst->encoded_bytes = cpu_->EncodeINC(value, mode);
                                } else if (mnemonic == "DEC") {
                                    inst->encoded_bytes = cpu_->EncodeDEC(value, mode);
                                } else if (mnemonic == "PHA") {
                                    inst->encoded_bytes = cpu_->EncodePHA();
                                } else if (mnemonic == "PLA") {
                                    inst->encoded_bytes = cpu_->EncodePLA();
                                } else if (mnemonic == "PHP") {
                                    inst->encoded_bytes = cpu_->EncodePHP();
                                } else if (mnemonic == "PLP") {
                                    inst->encoded_bytes = cpu_->EncodePLP();
                                } else if (mnemonic == "JSR") {
                                    inst->encoded_bytes = cpu_->EncodeJSR(value, mode);
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

                            // Record size for convergence check
                            current_sizes.push_back(inst->encoded_bytes.size());
                        }
                    }
                }
            }
        }

        // Pass 2: Extract labels from LabelAtoms
        // (Must happen AFTER encoding so encoded_bytes.size() is correct)
        // ALWAYS do this to ensure correct addresses, even with external symbol tables
        if (label_table_ptr != nullptr) {
            // Clear only labels (preserve other symbols like EQU/SET)
            // For now, just redefine - this will overwrite parser's placeholder addresses
            uint32_t current_address = 0;
            for (auto& section : sections_) {
                current_address = section.org;
                for (auto& atom : section.atoms) {
                    if (atom->type == AtomType::Org) {
                        // Handle .org directive - updates current address
                        auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
                        if (org) {
                            current_address = org->address;
                        }
                    } else if (atom->type == AtomType::Label) {
                        auto label = std::dynamic_pointer_cast<LabelAtom>(atom);
                        if (label) {
                            // Update label address
                            label->address = current_address;
                            // Define or redefine label in symbol table
                            label_table_ptr->Define(label->name, SymbolType::Label,
                                                   std::make_shared<LiteralExpr>(current_address));
                        }
                    } else if (atom->type == AtomType::Instruction) {
                        // Instructions consume bytes
                        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                        if (inst) {
                            current_address += inst->encoded_bytes.size();
                        }
                    }
                }
            }
        }

        // Check for convergence (sizes didn't change)
        if (pass > 1 && current_sizes == previous_sizes) {
            converged = true;
        }
        previous_sizes = current_sizes;
    }

    result.pass_count = pass;
    return result;
}

} // namespace xasm
