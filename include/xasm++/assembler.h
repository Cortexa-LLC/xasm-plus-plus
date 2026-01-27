// Assembler - Multi-pass assembler engine
// Phase 1: Minimal Viable Assembler - Assembler Engine

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "xasm++/atom.h"
#include "xasm++/section.h"
#include "xasm++/cpu/cpu_6502.h"

namespace xasm {

// Forward declarations
class SymbolTable;
class ConcreteSymbolTable;

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

    Assembler();

    // Set CPU plugin for instruction encoding
    void SetCpuPlugin(Cpu6502* cpu);

    // Set symbol table for symbol resolution
    void SetSymbolTable(SymbolTable* symbols);

    // Add a section to assemble
    void AddSection(const Section& section);

    // Get number of sections
    size_t GetSectionCount() const;

    // Reset assembler (clear sections)
    void Reset();

    // Assemble all sections with multi-pass resolution
    AssemblerResult Assemble();

private:
    // Instruction handler function type
    using InstructionHandler = std::function<std::vector<uint8_t>(Cpu6502*, uint16_t, AddressingMode)>;
    
    // Initialize instruction handler map
    void InitializeInstructionHandlers();

    // Resolve symbols - extract label addresses from atoms
    void ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                        ConcreteSymbolTable& symbols,
                        uint32_t org_address,
                        AssemblerResult& result);

    // Encode instructions - encode all instruction atoms using CPU plugin
    // Returns vector of instruction sizes for convergence checking
    std::vector<size_t> EncodeInstructions(ConcreteSymbolTable& symbols,
                                            AssemblerResult& result);

    // Check convergence - compare instruction sizes from previous and current passes
    // Returns true if sizes are identical (converged), false otherwise
    bool CheckConvergence(const std::vector<size_t>& previous_sizes,
                          const std::vector<size_t>& current_sizes) const;

    std::vector<Section> sections_;
    Cpu6502* cpu_ = nullptr;
    SymbolTable* symbols_ = nullptr;
    std::unordered_map<std::string, InstructionHandler> instruction_handlers_;
};

} // namespace xasm
