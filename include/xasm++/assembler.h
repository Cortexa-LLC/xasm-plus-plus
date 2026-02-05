/**
 * @file assembler.h
 * @brief Multi-pass assembler engine for xasm++
 * 
 * This file contains the core assembler engine that handles multi-pass
 * assembly with symbol resolution and instruction encoding.
 * 
 * @note Phase 1: Minimal Viable Assembler - Assembler Engine
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "xasm++/atom.h"
#include "xasm++/section.h"
#include "xasm++/cpu/cpu_plugin.h"

namespace xasm {

// Forward declarations
class SymbolTable;
class ConcreteSymbolTable;
class Cpu6502;
enum class AddressingMode;

/**
 * @brief Represents an error that occurred during assembly
 * 
 * Contains the error message and the source location where the error
 * occurred for better debugging and user feedback.
 */
struct AssemblerError {
    std::string message;        ///< Human-readable error description
    SourceLocation location;    ///< Source file location of the error
};

/**
 * @brief Result of an assembly operation
 * 
 * Contains the success status, number of passes required, and any
 * errors that occurred during assembly.
 */
struct AssemblerResult {
    bool success = true;                        ///< True if assembly succeeded
    int pass_count = 0;                         ///< Number of passes performed
    std::vector<AssemblerError> errors;         ///< List of errors (if any)
};

/**
 * @brief Multi-pass assembler engine
 * 
 * The Assembler class is the core component that performs multi-pass assembly.
 * It coordinates symbol resolution, instruction encoding, and convergence checking
 * to handle forward references and branch relaxation.
 * 
 * @par Multi-Pass Assembly
 * The assembler uses multiple passes to resolve:
 * - Forward references (labels defined after use)
 * - Branch relaxation (short branches that need to expand to long form)
 * - Instruction sizing (sizes unknown until labels are resolved)
 * 
 * @par Usage Example
 * @code
 * Assembler asm;
 * Cpu6502 cpu;
 * ConcreteSymbolTable symbols;
 * 
 * asm.SetCpuPlugin(&cpu);
 * asm.SetSymbolTable(&symbols);
 * 
 * Section code_section("CODE", 0x01, 0x8000);
 * // ... add atoms to section
 * asm.AddSection(code_section);
 * 
 * AssemblerResult result = asm.Assemble();
 * if (result.success) {
 *     std::cout << "Assembly succeeded in " << result.pass_count << " passes\n";
 * }
 * @endcode
 */
class Assembler {
public:
    /// Maximum number of passes in fast phase before switching to slower convergence checking
    static constexpr int FAST_PHASE_LIMIT = 50;
    
    /// Absolute maximum number of passes (prevents infinite loops)
    static constexpr int MAX_PASSES = 500;

    /**
     * @brief Construct a new Assembler
     * 
     * Initializes the assembler with no CPU plugin, no symbol table, and no sections.
     */
    Assembler();

    /**
     * @brief Set the CPU plugin for instruction encoding
     *
     * The CPU plugin is responsible for encoding instructions into machine code.
     * Different CPU plugins can be used for different target architectures
     * (6502, 65C02, 65816, 6809, Z80, etc.).
     *
     * @param cpu Pointer to CPU plugin (must remain valid for lifetime of assembler)
     *
     * @note The assembler does not take ownership of the CPU plugin
     */
    void SetCpuPlugin(CpuPlugin* cpu);

    /**
     * @brief Set the symbol table for symbol resolution
     * 
     * The symbol table is used to resolve symbol references during assembly.
     * It must be populated with label definitions before calling Assemble().
     * 
     * @param symbols Pointer to symbol table (must remain valid for lifetime of assembler)
     * 
     * @note The assembler does not take ownership of the symbol table
     */
    void SetSymbolTable(SymbolTable* symbols);

    /**
     * @brief Add a section to assemble
     * 
     * Sections are independently assembled and can have different origin addresses
     * and attributes (code, data, bss, etc.).
     * 
     * @param section Section to add (copied internally)
     */
    void AddSection(const Section& section);

    /**
     * @brief Get the number of sections added
     * 
     * @return Number of sections that will be assembled
     */
    size_t GetSectionCount() const;

    /**
     * @brief Reset the assembler state
     * 
     * Clears all sections, allowing the assembler to be reused for a new
     * assembly operation. Does not reset the CPU plugin or symbol table.
     */
    void Reset();

    /**
     * @brief Assemble all sections with multi-pass resolution
     * 
     * Performs multi-pass assembly to resolve forward references and handle
     * branch relaxation. The assembly process continues until:
     * - All instruction sizes converge (no more changes)
     * - FAST_PHASE_LIMIT passes completed (switches to slower convergence)
     * - MAX_PASSES reached (prevents infinite loops)
     * 
     * @return AssemblerResult containing success status, pass count, and errors
     * 
     * @pre CPU plugin must be set via SetCpuPlugin()
     * @pre Symbol table must be set via SetSymbolTable()
     * @pre At least one section must be added via AddSection()
     * 
     * @note This method modifies the internal state of atoms (encoded bytes, sizes)
     */
    AssemblerResult Assemble();

private:
    // CPU plugin polymorphism - instruction encoding delegated to CpuPlugin::EncodeInstruction()

    /**
     * @brief Resolve symbols in a single pass
     * 
     * Extracts label addresses from label atoms and updates the symbol table.
     * This must be done before encoding instructions so that symbol references
     * can be resolved.
     * 
     * @param atoms List of atoms to process
     * @param symbols Symbol table to update with label addresses
     * @param org_address Current origin address
     * @param result Result object to append errors to
     */
    void ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                        ConcreteSymbolTable& symbols,
                        uint32_t org_address,
                        AssemblerResult& result);

    /**
     * @brief Encode all instructions in a single pass
     * 
     * Iterates through all instruction atoms and encodes them using the CPU
     * plugin. Returns the sizes of encoded instructions for convergence checking.
     * 
     * @param symbols Symbol table for resolving operand references
     * @param result Result object to append errors to
     * @return Vector of instruction sizes (one per instruction atom)
     */
    std::vector<size_t> EncodeInstructions(ConcreteSymbolTable& symbols,
                                            AssemblerResult& result);

    /**
     * @brief Check if instruction sizes have converged
     * 
     * Compares instruction sizes from the current pass with the previous pass.
     * If all sizes are identical, the assembly has converged and no more
     * passes are needed.
     * 
     * @param previous_sizes Instruction sizes from previous pass
     * @param current_sizes Instruction sizes from current pass
     * @return true if sizes are identical (converged), false otherwise
     */
    bool CheckConvergence(const std::vector<size_t>& previous_sizes,
                          const std::vector<size_t>& current_sizes) const;

    std::vector<Section> sections_;                                         ///< Sections to assemble
    CpuPlugin* cpu_ = nullptr;                                              ///< CPU plugin for instruction encoding
    SymbolTable* symbols_ = nullptr;                                        ///< Symbol table for symbol resolution
};

} // namespace xasm
