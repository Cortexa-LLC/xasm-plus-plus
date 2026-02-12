/**
 * @file cpu_plugin.h
 * @brief Base interface for CPU plugin architecture
 *
 * Defines the common interface that all CPU plugins must implement,
 * allowing the assembler to work with different target architectures.
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Base interface for CPU instruction encoders
 *
 * This abstract base class defines the interface that all CPU plugins
 * must implement. It allows the assembler to work with different CPU
 * architectures (6502, 6809, Z80, etc.) through polymorphism.
 *
 * Each CPU plugin is responsible for:
 * - Encoding instructions into machine code bytes
 * - Handling CPU-specific addressing modes
 * - Providing CPU identification
 *
 * @par Design Pattern
 * This is a Strategy pattern implementation, where different CPU
 * plugins provide different instruction encoding strategies.
 */
class CpuPlugin {
public:
  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~CpuPlugin() = default;

  /**
   * @brief Get the CPU family name
   *
   * Returns a string identifying the CPU family (e.g., "6502", "6809", "Z80").
   * This is used for diagnostics and error messages.
   *
   * @return CPU family identifier string
   */
  virtual std::string GetCpuFamily() const = 0;

  /**
   * @brief Get supported CPU variant names
   *
   * Returns a list of CPU variant names supported by this plugin
   * (e.g., ["6502", "65c02", "65816"] for the 6502 family).
   *
   * @return Vector of supported CPU variant names
   */
  virtual std::vector<std::string> GetSupportedVariants() const = 0;

  /**
   * @brief Encode an instruction into machine code
   *
   * This is the primary interface for instruction encoding. Each CPU plugin
   * implements this method to encode instructions according to its
   * architecture.
   *
   * @param mnemonic Instruction mnemonic (e.g., "LDA", "STA", "JMP")
   * @param operand Operand value (immediate value or address)
   * @param operand_str Original operand string for parsing addressing modes
   * @return Vector of encoded bytes, or empty vector if instruction not
   * supported
   *
   * @throws std::invalid_argument if instruction/addressing mode not supported
   * @throws std::out_of_range if operand value out of range
   *
   * @note Different CPUs may parse operand_str differently based on syntax
   */
  virtual std::vector<uint8_t>
  EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                    const std::string &operand_str) const = 0;

  /**
   * @brief Check if an instruction requires special encoding
   *
   * Some instructions (like branches with relaxation or multi-byte
   * instructions) need special handling beyond the standard EncodeInstruction
   * interface. This method allows the CPU plugin to identify such instructions.
   *
   * @param mnemonic Instruction mnemonic to check
   * @return true if the instruction requires special encoding
   *
   * @note Default implementation returns false (no special handling needed)
   */
  virtual bool RequiresSpecialEncoding(const std::string &mnemonic) const {
    (void)mnemonic; // Unused in default implementation
    return false;
  }

  /**
   * @brief Encode an instruction with special handling
   *
   * Handles instructions that require context beyond standard operand values,
   * such as:
   * - Branch instructions with relaxation (need current address and target)
   * - Multi-operand instructions (like MVN/MVP with two operands)
   * - Instructions with special parsing requirements
   *
   * @param mnemonic Instruction mnemonic
   * @param operand Operand string (unparsed, for special parsing)
   * @param current_address Current instruction address (for relative branches)
   * @return Vector of encoded bytes
   *
   * @throws std::invalid_argument if instruction not supported for special
   * encoding
   * @throws std::runtime_error if encoding fails
   *
   * @note Default implementation throws exception (not supported)
   */
  virtual std::vector<uint8_t>
  EncodeInstructionSpecial(const std::string &mnemonic,
                           const std::string &operand,
                           uint16_t current_address) const {
    (void)operand;         // Unused in default implementation
    (void)current_address; // Unused in default implementation
    throw std::invalid_argument(
        "Special encoding not supported for instruction: " + mnemonic);
  }

protected:
  /**
   * @brief Protected constructor (interface class)
   *
   * Prevents direct instantiation of the interface.
   */
  CpuPlugin() = default;

  // Disable copying and moving (plugins are typically unique)
  CpuPlugin(const CpuPlugin &) = delete;
  CpuPlugin &operator=(const CpuPlugin &) = delete;
  CpuPlugin(CpuPlugin &&) = delete;
  CpuPlugin &operator=(CpuPlugin &&) = delete;
};

} // namespace xasm
