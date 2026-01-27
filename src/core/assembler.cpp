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

    // Group 1: Accumulator addressing mode
    // ASL A, LSR A, ROL A, ROR A
    if (trimmed == "A") {
        return AddressingMode::Accumulator;
    }

    // Group 3 & 4: Check for indirect addressing modes (parentheses)
    // Must check before indexed modes since these have parens
    if (trimmed[0] == '(') {
        size_t open_paren = trimmed.find('(');
        size_t close_paren = trimmed.find(')');

        if (close_paren != std::string::npos) {
            std::string inside_parens = trimmed.substr(open_paren + 1, close_paren - open_paren - 1);
            inside_parens = Trim(inside_parens);

            // Check if there's a comma inside the parens: ($80,X) - IndexedIndirect
            if (inside_parens.find(',') != std::string::npos) {
                return AddressingMode::IndirectX;
            }

            // Check if there's content after closing paren: ($80),Y - IndirectIndexed
            if (close_paren < trimmed.length() - 1) {
                std::string after_parens = Trim(trimmed.substr(close_paren + 1));
                if (after_parens == ",Y" || after_parens == ", Y") {
                    return AddressingMode::IndirectY;
                }
            }

            // Simple Indirect: ($1234) - only for JMP
            if (close_paren == trimmed.length() - 1) {
                return AddressingMode::Indirect;
            }
        }
    }

    // Group 2: Check for indexed addressing modes (,X or ,Y)
    // Need to check this before parsing hex values
    size_t comma_x = trimmed.find(",X");
    size_t comma_x_space = trimmed.find(", X");
    size_t comma_y = trimmed.find(",Y");
    size_t comma_y_space = trimmed.find(", Y");

    if (comma_x != std::string::npos || comma_x_space != std::string::npos) {
        // Extract address part (before comma)
        size_t comma_pos = (comma_x != std::string::npos) ? comma_x : comma_x_space;
        std::string addr_part = trimmed.substr(0, comma_pos);
        addr_part = Trim(addr_part);

        // Check if it's a hex address or label
        if (addr_part[0] == '$') {
            uint32_t value = ParseHex(addr_part);
            if (value <= 0xFF) {
                return AddressingMode::ZeroPageX;
            }
            return AddressingMode::AbsoluteX;
        } else {
            // Label with ,X indexing
            return AddressingMode::AbsoluteX;
        }
    }

    if (comma_y != std::string::npos || comma_y_space != std::string::npos) {
        // Extract address part (before comma)
        size_t comma_pos = (comma_y != std::string::npos) ? comma_y : comma_y_space;
        std::string addr_part = trimmed.substr(0, comma_pos);
        addr_part = Trim(addr_part);

        // Check if it's a hex address or label
        if (addr_part[0] == '$') {
            uint32_t value = ParseHex(addr_part);
            if (value <= 0xFF) {
                return AddressingMode::ZeroPageY;
            }
            return AddressingMode::AbsoluteY;
        } else {
            // Label with ,Y indexing
            return AddressingMode::AbsoluteY;
        }
    }

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
                // Track current address during encoding
                uint32_t current_address = section.org;

                for (auto& atom : section.atoms) {
                    if (atom->type == AtomType::Org) {
                        // Handle .org directive
                        auto org = std::dynamic_pointer_cast<OrgAtom>(atom);
                        if (!org) {
                            // Cast failed - this indicates a corrupted atom
                            AssemblerError error;
                            error.location = atom->location;
                            error.message = "Failed to cast to OrgAtom - atom corruption detected";
                            result.errors.push_back(error);
                            result.success = false;
                            continue;
                        }
                        current_address = org->address;
                    } else if (atom->type == AtomType::Label) {
                        // Labels don't advance address yet, but we track them
                        // (address will be finalized in Pass 2)
                    } else if (atom->type == AtomType::Instruction) {
                        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                        if (!inst) {
                            // Cast failed - this indicates a corrupted atom
                            AssemblerError error;
                            error.location = atom->location;
                            error.message = "Failed to cast to InstructionAtom - atom corruption detected";
                            result.errors.push_back(error);
                            result.success = false;
                            continue;
                        }
                        // Clear previous encoding
                        inst->encoded_bytes.clear();

                        // Encode the instruction
                        std::string mnemonic = inst->mnemonic;
                        std::string operand = inst->operand;

                        // Parse operand to get addressing mode and value
                        AddressingMode mode = DetermineAddressingMode(operand);
                        uint16_t value = 0;
                        std::string label_name = operand;  // Save for error messages

                        // Extract operand value
                        if (!operand.empty()) {
                            std::string trimmed = Trim(operand);

                            // Strip parentheses for indirect modes: ($1234) or ($80,X) or ($80),Y
                            std::string value_str = trimmed;
                            if (value_str[0] == '(') {
                                size_t close_paren = value_str.find(')');
                                if (close_paren != std::string::npos) {
                                    value_str = value_str.substr(1, close_paren - 1);
                                    value_str = Trim(value_str);
                                }
                            }

                            // Strip index registers (,X or ,Y) for value extraction
                            size_t comma_pos = value_str.find(',');
                            if (comma_pos != std::string::npos) {
                                value_str = Trim(value_str.substr(0, comma_pos));
                            }

                            if (value_str[0] == '#') {
                                // Immediate: #$42
                                value = static_cast<uint16_t>(ParseHex(value_str.substr(1)));
                            } else if (value_str[0] == '$') {
                                // Absolute/Zero Page: $1234 (or $1234,X after stripping)
                                value = static_cast<uint16_t>(ParseHex(value_str));
                            } else if (value_str != "A") {
                                // Label reference - look up in symbol table (skip if accumulator "A")
                                SymbolTable* lookup_table = symbols_ ? symbols_ : label_table_ptr;
                                if (lookup_table != nullptr) {
                                    int64_t symbol_value;
                                    if (lookup_table->Lookup(value_str, symbol_value)) {
                                        value = static_cast<uint16_t>(symbol_value);
                                    }
                                }
                            }
                        }

                        // Special handling for branch instructions (relative addressing)
                        bool is_branch = (mnemonic == "BEQ" || mnemonic == "BNE" ||
                                        mnemonic == "BCC" || mnemonic == "BCS" ||
                                        mnemonic == "BMI" || mnemonic == "BPL" ||
                                        mnemonic == "BVC" || mnemonic == "BVS");

                        if (is_branch && mode == AddressingMode::Absolute) {
                            // Branch instructions use relative addressing
                            // Use EncodeBranchWithRelaxation which handles both short and long branches
                            
                            // Get branch opcode for this mnemonic
                            uint8_t branch_opcode = 0;
                            if (mnemonic == "BEQ") branch_opcode = 0xF0;
                            else if (mnemonic == "BNE") branch_opcode = 0xD0;
                            else if (mnemonic == "BCC") branch_opcode = 0x90;
                            else if (mnemonic == "BCS") branch_opcode = 0xB0;
                            else if (mnemonic == "BMI") branch_opcode = 0x30;
                            else if (mnemonic == "BPL") branch_opcode = 0x10;
                            else if (mnemonic == "BVC") branch_opcode = 0x50;
                            else if (mnemonic == "BVS") branch_opcode = 0x70;
                            
                            // Use branch relaxation (handles both short and long branches)
                            inst->encoded_bytes = cpu_->EncodeBranchWithRelaxation(
                                branch_opcode,
                                static_cast<uint16_t>(current_address),
                                static_cast<uint16_t>(value)
                            );
                            
                            // Skip the normal encoding path for branches
                            // Advance current address past this instruction
                            current_address += inst->encoded_bytes.size();
                            current_sizes.push_back(inst->encoded_bytes.size());
                            continue;  // Skip to next atom
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
                            } else if (mnemonic == "BIT") {
                                inst->encoded_bytes = cpu_->EncodeBIT(value, mode);
                            } else if (mnemonic == "ASL") {
                                inst->encoded_bytes = cpu_->EncodeASL(value, mode);
                            } else if (mnemonic == "LSR") {
                                inst->encoded_bytes = cpu_->EncodeLSR(value, mode);
                            } else if (mnemonic == "ROL") {
                                inst->encoded_bytes = cpu_->EncodeROL(value, mode);
                            } else if (mnemonic == "ROR") {
                                inst->encoded_bytes = cpu_->EncodeROR(value, mode);
                            } else if (mnemonic == "RTI") {
                                inst->encoded_bytes = cpu_->EncodeRTI();
                            } else if (mnemonic == "BRK") {
                                inst->encoded_bytes = cpu_->EncodeBRK();
                            } else if (mnemonic == "CLC") {
                                inst->encoded_bytes = cpu_->EncodeCLC();
                            } else if (mnemonic == "SEC") {
                                inst->encoded_bytes = cpu_->EncodeSEC();
                            } else if (mnemonic == "CLD") {
                                inst->encoded_bytes = cpu_->EncodeCLD();
                            } else if (mnemonic == "SED") {
                                inst->encoded_bytes = cpu_->EncodeSED();
                            } else if (mnemonic == "CLI") {
                                inst->encoded_bytes = cpu_->EncodeCLI();
                            } else if (mnemonic == "SEI") {
                                inst->encoded_bytes = cpu_->EncodeSEI();
                            } else if (mnemonic == "CLV") {
                                inst->encoded_bytes = cpu_->EncodeCLV();
                            } else if (mnemonic == "TSX") {
                                inst->encoded_bytes = cpu_->EncodeTSX();
                            } else if (mnemonic == "TXS") {
                                inst->encoded_bytes = cpu_->EncodeTXS();
                            } else if (mnemonic == "TAX") {
                                inst->encoded_bytes = cpu_->EncodeTAX();
                            } else if (mnemonic == "TAY") {
                                inst->encoded_bytes = cpu_->EncodeTAY();
                            } else if (mnemonic == "TXA") {
                                inst->encoded_bytes = cpu_->EncodeTXA();
                            } else if (mnemonic == "TYA") {
                                inst->encoded_bytes = cpu_->EncodeTYA();
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

                        // Advance current address past this instruction
                        current_address += inst->encoded_bytes.size();
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
                        if (!org) {
                            // Cast failed - this indicates a corrupted atom
                            AssemblerError error;
                            error.location = atom->location;
                            error.message = "Failed to cast to OrgAtom - atom corruption detected";
                            result.errors.push_back(error);
                            result.success = false;
                            continue;
                        }
                        current_address = org->address;
                    } else if (atom->type == AtomType::Label) {
                        auto label = std::dynamic_pointer_cast<LabelAtom>(atom);
                        if (!label) {
                            // Cast failed - this indicates a corrupted atom
                            AssemblerError error;
                            error.location = atom->location;
                            error.message = "Failed to cast to LabelAtom - atom corruption detected";
                            result.errors.push_back(error);
                            result.success = false;
                            continue;
                        }
                        // Update label address
                        label->address = current_address;
                        // Define or redefine label in symbol table
                        label_table_ptr->Define(label->name, SymbolType::Label,
                                               std::make_shared<LiteralExpr>(current_address));
                    } else if (atom->type == AtomType::Instruction) {
                        // Instructions consume bytes
                        auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
                        if (!inst) {
                            // Cast failed - this indicates a corrupted atom
                            AssemblerError error;
                            error.location = atom->location;
                            error.message = "Failed to cast to InstructionAtom - atom corruption detected";
                            result.errors.push_back(error);
                            result.success = false;
                            continue;
                        }
                        current_address += inst->encoded_bytes.size();
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
