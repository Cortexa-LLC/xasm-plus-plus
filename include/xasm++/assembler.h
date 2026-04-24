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
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "xasm++/atom.h"
#include "xasm++/common/expression_parser.h"
#include "xasm++/cpu/cpu_plugin.h"
#include "xasm++/section.h"

namespace xasm {

// Forward declarations
class SymbolTable;
class ConcreteSymbolTable;
class Cpu6502;
enum class AddressingMode : std::uint8_t;
// Visitor pass classes (defined in assembler.cpp, declared here for friend).
class EncodePass;
class RefixupPass;

/**
 * @brief Represents an error that occurred during assembly
 *
 * Contains the error message and the source location where the error
 * occurred for better debugging and user feedback.
 */
struct AssemblerError {
  std::string message{};    ///< Human-readable error description
  SourceLocation location;  ///< Source file location of the error
};

/**
 * @brief Result of an assembly operation
 *
 * Contains the success status, number of passes required, and any
 * errors that occurred during assembly.
 */
struct AssemblerResult {
  bool success = true;                   ///< True if assembly succeeded
  unsigned int pass_count = 0;           ///< Number of passes performed
  std::vector<AssemblerError> errors{};  ///< List of errors (if any)
};

/**
 * @brief Multi-pass assembler engine
 *
 * The Assembler class is the core component that performs multi-pass assembly.
 * It coordinates symbol resolution, instruction encoding, and convergence
 * checking to handle forward references and branch relaxation.
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
 *     std::cout << "Assembly succeeded in " << result.pass_count << "
 * passes\n";
 * }
 * @endcode
 */

/**
 * @brief Instruction sizes from two consecutive passes for convergence checking
 */
struct ConvergenceSizes {
  const std::vector<size_t>& previous;  ///< Instruction sizes from the previous pass
  const std::vector<size_t>& current;   ///< Instruction sizes from the current pass
};

class Assembler {
 public:
  /// Maximum number of passes in fast phase before switching to slower
  /// convergence checking
  static constexpr int kFAST_PHASE_LIMIT = 50;

  /// Maximum assembly passes before reporting non-convergence error.
  /// Complex kernels with many cascading forward references may need 15-20
  /// passes; 50 is a safe upper bound that still catches infinite loops.
  static constexpr int kMAX_PASSES = 50;

  /**
   * @brief Construct a new Assembler
   *
   * Initializes the assembler with no CPU plugin, no symbol table, and no
   * sections.
   */
  Assembler();

  /**
   * @brief Set the CPU plugin for instruction encoding
   *
   * The CPU plugin is responsible for encoding instructions into machine code.
   * Different CPU plugins can be used for different target architectures
   * (6502, 65C02, 65816, 6809, Z80, etc.).
   *
   * @param cpu Pointer to CPU plugin (must remain valid for lifetime of
   * assembler)
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
   * @param symbols Pointer to symbol table (must remain valid for lifetime of
   * assembler)
   *
   * @note The assembler does not take ownership of the symbol table
   */
  void SetSymbolTable(SymbolTable* symbols);

  /**
   * @brief Add a section to assemble
   *
   * Sections are independently assembled and can have different origin
   * addresses and attributes (code, data, bss, etc.).
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
   * @brief Get the assembled sections
   *
   * Returns the sections after assembly. These sections contain the encoded
   * instructions and resolved data.
   *
   * @return Const reference to the vector of assembled sections
   *
   * @note Should only be called after Assemble() has been called
   */
  const std::vector<Section>& GetSections() const;

  /**
   * @brief Reset the assembler state
   *
   * Clears all sections, allowing the assembler to be reused for a new
   * assembly operation. Does not reset the CPU plugin or symbol table.
   */
  void Reset();

  /**
   * @brief Set the maximum number of assembly passes
   *
   * Overrides the default kMAX_PASSES limit. Useful when assembling code
   * with many forward references that require more convergence passes, or
   * when limiting passes for faster builds.
   *
   * @param max_passes Maximum number of assembly passes (must be >= 1)
   */
  void SetMaxPasses(int max_passes);

  /**
   * @brief Set expression parser features for dialect-specific operators
   *
   * Controls which expression parser features are active when evaluating
   * instruction operands.  For example, Merlin syntax uses `.` as bitwise OR
   * and `!` as bitwise XOR (ADR-005 V9).  Call this before Assemble() after
   * the syntax dialect is known.
   *
   * Default: ParserFeatures::Default() — no dialect extensions.
   *
   * @param features Feature flags to use when creating ExpressionParser
   *                 instances during instruction encoding.
   */
  void SetExpressionFeatures(ParserFeatures features);

  /**
   * @brief Assemble all sections with multi-pass resolution
   *
   * Performs multi-pass assembly to resolve forward references and handle
   * branch relaxation. The assembly process continues until:
   * - All instruction sizes converge (no more changes)
   * - kFAST_PHASE_LIMIT passes completed (switches to slower convergence)
   * - kMAX_PASSES reached (prevents infinite loops)
   *
   * @return AssemblerResult containing success status, pass count, and errors
   *
   * @pre CPU plugin must be set via SetCpuPlugin()
   * @pre Symbol table must be set via SetSymbolTable()
   * @pre At least one section must be added via AddSection()
   *
   * @note This method modifies the internal state of atoms (encoded bytes,
   * sizes)
   */
  AssemblerResult Assemble();

 private:
  // CPU plugin polymorphism - instruction encoding delegated to
  // CpuPlugin::EncodeInstruction()

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
  static void ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                             ConcreteSymbolTable& symbols, uint32_t org_address,
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
  std::vector<size_t> EncodeInstructions(ConcreteSymbolTable& symbols, AssemblerResult& result,
                                         int pass_number = 2);

  /// Mutable per-section state threaded through per-atom handlers.
  struct EncodeAtomState {
    uint32_t& current_address;
    uint32_t& virtual_address;
    uint32_t& phase_real_start;
    uint32_t& phase_virtual_start;
    ConcreteSymbolTable& symbols;
    AssemblerResult& result;
    std::vector<size_t>& current_sizes;
    int pass_number = 2;
  };

  /// Returns true when the caller should `continue` to the next atom.
  bool HandlePhaseAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);
  bool HandleOrgAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);
  bool HandleEquateAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);
  bool HandleDataAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);
  bool HandleInstructionAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);
  bool HandleSpaceAtom(const std::shared_ptr<Atom>& atom, EncodeAtomState& state);

  // Private helpers split from HandleDataAtom
  void EvaluateDataElement(const std::string& expr_str_raw, uint32_t virtual_address,
                           ConcreteSymbolTable& symbols, DataAtom& data);

  // Private helpers split from HandleInstructionAtom
  /// Attempt special-encoding branch (returns true + populates inst on success,
  /// or false when the caller should fall through to standard encoding).
  bool TrySpecialEncodeInstruction(InstructionAtom& inst, uint32_t current_address,
                                   uint32_t virtual_address, ConcreteSymbolTable& symbols,
                                   AssemblerResult& result, int pass_number);
  /// Parse the operand string into a uint16_t value for standard encoding.
  uint16_t ParseInstructionOperandValue(const std::string& operand, uint32_t virtual_address,
                                        ConcreteSymbolTable& symbols);

  // Private helpers split from HandleSpaceAtom
  /// Re-evaluate the PC-relative expression for a DS/BS directive and update
  /// space->count / space->size in place.
  void EvaluateSpaceExpression(SpaceAtom& space, uint32_t virtual_address,
                               ConcreteSymbolTable& symbols);

  /**
   * @brief Check if instruction sizes have converged
   *
   * Compares instruction sizes from the current pass with the previous pass.
   * If all sizes are identical, the assembly has converged and no more
   * passes are needed.
   *
   * @param sizes Instruction sizes from previous and current pass
   * @return true if sizes are identical (converged), false otherwise
   */
  static bool CheckConvergence(ConvergenceSizes sizes);

  /**
   * @brief Re-evaluate data and equate atoms using final converged symbols
   *
   * Called once after the convergence loop to propagate final symbol values
   * into DataAtoms that referenced forward-declared symbols (e.g. a `.DA
   * MAIN.P` entry that precedes the MAIN.P label in the atom list). Unlike
   * EncodeInstructions(), this method intentionally does NOT re-encode
   * InstructionAtoms, so branch sizes remain stable and no cascading address
   * shifts occur.
   *
   * @param symbols Converged symbol table with final values
   * @param result Result object to append errors to
   */
  void RefixupDataAtoms(ConcreteSymbolTable& symbols, AssemblerResult& result);

  /// Returns the expression parser features (used by visitor passes).
  ParserFeatures GetExpressionFeatures() const { return expression_features_; }

  std::vector<Section> sections_;   ///< Sections to assemble
  CpuPlugin* cpu_ = nullptr;        ///< CPU plugin for instruction encoding
  SymbolTable* symbols_ = nullptr;  ///< Symbol table for symbol resolution
  int max_passes_ = kMAX_PASSES;     ///< Maximum assembly passes (default: kMAX_PASSES)
  /// Expression parser features for instruction operand evaluation (ADR-005 V9)
  ParserFeatures expression_features_;

  // Visitor pass classes need access to private Handle* methods and cpu_.
  friend class EncodePass;
  friend class RefixupPass;
};

}  // namespace xasm
