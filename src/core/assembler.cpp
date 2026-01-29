// Assembler implementation

#include "xasm++/assembler.h"
#include "xasm++/cpu/cpu_6502.h"
#include "xasm++/cpu/opcodes_6502.h"
#include "xasm++/symbol.h"
#include "xasm++/util/string_utils.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace xasm {

using xasm::util::Trim;
using xasm::util::ParseHex;

// Helper: Determine addressing mode from operands string
static AddressingMode DetermineAddressingMode(const std::string& operands) {
    if (operands.empty()) {
        return AddressingMode::Implied;
    }

    std::string trimmed = Trim(operands);
    
    // After trimming, check if empty (was all whitespace)
    if (trimmed.empty()) {
        return AddressingMode::Implied;
    }

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

        // Check for empty addr_part
        if (addr_part.empty()) {
            return AddressingMode::Absolute;  // Default fallback
        }

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

        // Check for empty addr_part
        if (addr_part.empty()) {
            return AddressingMode::Absolute;  // Default fallback
        }

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

Assembler::Assembler() {
    InitializeInstructionHandlers();
}

void Assembler::InitializeInstructionHandlers() {
    RegisterMemoryInstructions();
    RegisterArithmeticInstructions();
    RegisterBranchInstructions();
    RegisterStackInstructions();
    RegisterControlInstructions();
}

void Assembler::RegisterMemoryInstructions() {
    // Load instructions
    instruction_handlers_["LDA"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeLDA(value, mode);
    };
    instruction_handlers_["LDX"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeLDX(value, mode);
    };
    instruction_handlers_["LDY"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeLDY(value, mode);
    };
    
    // Store instructions
    instruction_handlers_["STA"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeSTA(value, mode);
    };
    instruction_handlers_["STX"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeSTX(value, mode);
    };
    instruction_handlers_["STY"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeSTY(value, mode);
    };
}

void Assembler::RegisterArithmeticInstructions() {
    // Arithmetic operations
    instruction_handlers_["ADC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeADC(value, mode);
    };
    instruction_handlers_["SBC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeSBC(value, mode);
    };
    
    // Increment/decrement memory
    instruction_handlers_["INC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeINC(value, mode);
    };
    instruction_handlers_["DEC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeDEC(value, mode);
    };
    
    // Increment/decrement registers (implied addressing)
    instruction_handlers_["INX"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeINX();
    };
    instruction_handlers_["INY"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeINY();
    };
    instruction_handlers_["DEX"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeDEX();
    };
    instruction_handlers_["DEY"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeDEY();
    };
}

void Assembler::RegisterBranchInstructions() {
    instruction_handlers_["BEQ"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBEQ(value, mode);
    };
    instruction_handlers_["BNE"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBNE(value, mode);
    };
    instruction_handlers_["BCC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBCC(value, mode);
    };
    instruction_handlers_["BCS"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBCS(value, mode);
    };
    instruction_handlers_["BPL"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBPL(value, mode);
    };
    instruction_handlers_["BMI"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBMI(value, mode);
    };
    instruction_handlers_["BVC"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBVC(value, mode);
    };
    instruction_handlers_["BVS"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBVS(value, mode);
    };
}

void Assembler::RegisterStackInstructions() {
    instruction_handlers_["PHA"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodePHA();
    };
    instruction_handlers_["PLA"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodePLA();
    };
    instruction_handlers_["PHP"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodePHP();
    };
    instruction_handlers_["PLP"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodePLP();
    };
}

void Assembler::RegisterControlInstructions() {
    // Control flow
    instruction_handlers_["NOP"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeNOP();
    };
    instruction_handlers_["BRK"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeBRK();
    };
    instruction_handlers_["RTI"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeRTI();
    };
    instruction_handlers_["RTS"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeRTS();
    };
    instruction_handlers_["JMP"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeJMP(value, mode);
    };
    instruction_handlers_["JSR"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeJSR(value, mode);
    };
    
    // Status flag operations
    instruction_handlers_["CLC"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeCLC();
    };
    instruction_handlers_["SEC"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeSEC();
    };
    instruction_handlers_["CLD"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeCLD();
    };
    instruction_handlers_["SED"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeSED();
    };
    instruction_handlers_["CLI"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeCLI();
    };
    instruction_handlers_["SEI"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeSEI();
    };
    instruction_handlers_["CLV"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeCLV();
    };
    
    // Register transfers
    instruction_handlers_["TSX"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTSX();
    };
    instruction_handlers_["TXS"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTXS();
    };
    instruction_handlers_["TAX"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTAX();
    };
    instruction_handlers_["TAY"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTAY();
    };
    instruction_handlers_["TXA"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTXA();
    };
    instruction_handlers_["TYA"] = [](Cpu6502* cpu, uint16_t, AddressingMode) {
        return cpu->EncodeTYA();
    };
    
    // Logical operations
    instruction_handlers_["AND"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeAND(value, mode);
    };
    instruction_handlers_["ORA"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeORA(value, mode);
    };
    instruction_handlers_["EOR"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeEOR(value, mode);
    };
    
    // Comparison operations
    instruction_handlers_["CMP"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeCMP(value, mode);
    };
    instruction_handlers_["CPX"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeCPX(value, mode);
    };
    instruction_handlers_["CPY"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeCPY(value, mode);
    };
    
    // Bit operations
    instruction_handlers_["BIT"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeBIT(value, mode);
    };
    
    // Shift/rotate operations
    instruction_handlers_["ASL"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeASL(value, mode);
    };
    instruction_handlers_["LSR"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeLSR(value, mode);
    };
    instruction_handlers_["ROL"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeROL(value, mode);
    };
    instruction_handlers_["ROR"] = [](Cpu6502* cpu, uint16_t value, AddressingMode mode) {
        return cpu->EncodeROR(value, mode);
    };
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

std::vector<size_t> Assembler::EncodeInstructions(ConcreteSymbolTable& symbols,
                                                    AssemblerResult& result) {
    // Encode instructions using CPU plugin
    std::vector<size_t> current_sizes;
    if (cpu_ != nullptr) {
        for (auto& section : sections_) {
            // Track current address during encoding
            uint32_t current_address = section.org;

            for (auto& atom : section.atoms) {
                // Skip null atoms gracefully
                if (!atom) {
                    AssemblerError error;
                    error.message = "Null atom encountered - skipping";
                    result.errors.push_back(error);
                    result.success = false;
                    continue;
                }
                
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
                            SymbolTable* lookup_table = symbols_ ? symbols_ : &symbols;
                            if (lookup_table != nullptr) {
                                int64_t symbol_value;
                                if (lookup_table->Lookup(value_str, symbol_value)) {
                                    value = static_cast<uint16_t>(symbol_value);
                                }
                            }
                        }
                    }

                    // Special handling for branch instructions (relative addressing)
                    //
                    // WHY BRANCH RELAXATION?
                    // ======================
                    // 6502 branch instructions (BEQ, BNE, etc.) use 8-bit signed relative offsets,
                    // limiting range to -128 to +127 bytes. If the target is farther away, we must
                    // "relax" the branch into a longer sequence:
                    //
                    //   Short form (2 bytes):     BEQ label
                    //   Long form (5 bytes):      BNE skip / JMP label / skip: ...
                    //
                    // This relaxation can trigger cascading changes:
                    // - Short branch becomes long (adds 3 bytes)
                    // - Subsequent instructions shift by 3 bytes
                    // - Other branches may now be out of range
                    // - Requires another pass to re-encode
                    //
                    // This is why multi-pass assembly is essential for correct branch generation.
                    bool is_branch = (mnemonic == "BEQ" || mnemonic == "BNE" ||
                                    mnemonic == "BCC" || mnemonic == "BCS" ||
                                    mnemonic == "BMI" || mnemonic == "BPL" ||
                                    mnemonic == "BVC" || mnemonic == "BVS");

                    if (is_branch && mode == AddressingMode::Absolute) {
                        // Branch instructions use relative addressing
                        // Use EncodeBranchWithRelaxation which handles both short and long branches
                        
                        // Get branch opcode for this mnemonic
                        uint8_t branch_opcode = 0;
                        if (mnemonic == "BEQ") branch_opcode = Opcodes::BEQ;
                        else if (mnemonic == "BNE") branch_opcode = Opcodes::BNE;
                        else if (mnemonic == "BCC") branch_opcode = Opcodes::BCC;
                        else if (mnemonic == "BCS") branch_opcode = Opcodes::BCS;
                        else if (mnemonic == "BMI") branch_opcode = Opcodes::BMI;
                        else if (mnemonic == "BPL") branch_opcode = Opcodes::BPL;
                        else if (mnemonic == "BVC") branch_opcode = Opcodes::BVC;
                        else if (mnemonic == "BVS") branch_opcode = Opcodes::BVS;
                        
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

                    // Use hash map lookup for O(1) instruction dispatch
                    try {
                        auto it = instruction_handlers_.find(mnemonic);
                        if (it != instruction_handlers_.end()) {
                            // Found handler - invoke it
                            inst->encoded_bytes = it->second(cpu_, value, mode);
                        } else {
                            // Unknown instruction
                            AssemblerError error;
                            error.message = "Unknown instruction: " + mnemonic;
                            result.errors.push_back(error);
                            result.success = false;
                        }
                    } catch (const std::invalid_argument& e) {
                        // Invalid argument (e.g., unsupported addressing mode)
                        AssemblerError error;
                        error.location = inst->location;
                        error.message = "Invalid argument for " + mnemonic + ": " + e.what();
                        result.errors.push_back(error);
                        result.success = false;
                    } catch (const std::out_of_range& e) {
                        // Value out of range (e.g., branch too far, value too large)
                        AssemblerError error;
                        error.location = inst->location;
                        error.message = "Value out of range for " + mnemonic + ": " + e.what();
                        result.errors.push_back(error);
                        result.success = false;
                    } catch (const std::runtime_error& e) {
                        // Runtime error (e.g., undefined behavior, internal error)
                        AssemblerError error;
                        error.location = inst->location;
                        error.message = "Runtime error encoding " + mnemonic + ": " + e.what();
                        result.errors.push_back(error);
                        result.success = false;
                    } catch (const std::logic_error& e) {
                        // Logic error (programming error, shouldn't happen in production)
                        AssemblerError error;
                        error.location = inst->location;
                        error.message = "Logic error encoding " + mnemonic + ": " + e.what();
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
    return current_sizes;
}

AssemblerResult Assembler::Assemble() {
    // WHY MULTI-PASS ASSEMBLY?
    // ========================
    // Multi-pass assembly is necessary because of three interdependent challenges:
    //
    // 1. FORWARD REFERENCES: Labels can be used before they're defined
    //    Example: JMP end_loop    ; 'end_loop' address unknown on first pass
    //             ...
    //             end_loop: RTS   ; Now we know the address
    //
    // 2. BRANCH RELAXATION: Branch instructions may change size between passes
    //    Example: BEQ label       ; Initially 2 bytes (short branch)
    //             [lots of code]
    //             label: NOP       ; If >127 bytes away, must expand to:
    //                              ; BNE +3 / JMP label (5 bytes total)
    //
    // 3. INSTRUCTION SIZING: We can't know final instruction sizes until symbols are resolved
    //    - Short branches (2 bytes) vs long branches (5 bytes)
    //    - Zero page addressing ($80) vs absolute addressing ($1234)
    //
    // CONVERGENCE ALGORITHM:
    // ======================
    // We repeat encoding passes until instruction sizes stabilize:
    // - Pass 1: Encode with best guesses, extract label addresses
    // - Pass 2: Re-encode with updated addresses, check if sizes changed
    // - Pass N: Repeat until sizes stop changing (converged)
    //
    // Typically converges in 2-3 passes. MAX_PASSES (500) prevents infinite loops
    // in pathological cases (though such cases are rare in practice).
    //
    // WHY 500 PASSES?
    // ===============
    // Empirically, real-world code converges quickly (<10 passes).
    // 500 is a safety limit chosen to be:
    // - Large enough for any realistic code
    // - Small enough to catch bugs (e.g., oscillating branches)
    // - Fast enough not to annoy users (even 500 passes takes <1 second)

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
        std::vector<size_t> current_sizes = EncodeInstructions(*label_table_ptr, result);

        // Pass 2: Extract labels from LabelAtoms
        // (Must happen AFTER encoding so encoded_bytes.size() is correct)
        // ALWAYS do this to ensure correct addresses, even with external symbol tables
        if (label_table_ptr != nullptr) {
            for (auto& section : sections_) {
                ResolveSymbols(section.atoms, *label_table_ptr, section.org, result);
            }
        }

        // Check for convergence
        // WHY: If instruction sizes are identical to previous pass, addresses won't change,
        // so we've reached a stable state and can stop.
        if (pass > 1) {
            converged = CheckConvergence(previous_sizes, current_sizes);
        }
        previous_sizes = current_sizes;
    }

    result.pass_count = pass;
    return result;
}

void Assembler::ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                                ConcreteSymbolTable& symbols,
                                uint32_t org_address,
                                AssemblerResult& result) {
    // Clear only labels (preserve other symbols like EQU/SET)
    // For now, just redefine - this will overwrite parser's placeholder addresses
    uint32_t current_address = org_address;
    
    // Process atoms to extract label addresses
    for (auto& atom : atoms) {
        // Skip null atoms gracefully
        if (!atom) {
            AssemblerError error;
            error.message = "Null atom encountered in symbol resolution - skipping";
            result.errors.push_back(error);
            result.success = false;
            continue;
        }
        
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
            symbols.Define(label->name, SymbolType::Label,
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

bool Assembler::CheckConvergence(const std::vector<size_t>& previous_sizes,
                                  const std::vector<size_t>& current_sizes) const {
    // Convergence achieved when instruction sizes are identical between passes
    //
    // WHY THIS WORKS:
    // ===============
    // If all instruction sizes are stable, then:
    // 1. All addresses are stable (no size changes = no address shifts)
    // 2. All symbol values are stable (addresses determine symbol values)
    // 3. All branches are correctly sized (no more relaxation needed)
    // 4. Further passes would produce identical output (fixed point reached)
    //
    // This is the mathematical definition of convergence: f(x) = x
    // where f is "encode one pass" and x is "instruction sizes vector"
    return current_sizes == previous_sizes;
}

} // namespace xasm
