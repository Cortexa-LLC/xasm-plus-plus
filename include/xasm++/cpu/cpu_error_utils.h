/**
 * @file cpu_error_utils.h
 * @brief Common error handling utilities for CPU implementations
 *
 * This header provides inline helper functions for throwing consistent
 * error messages across different CPU implementations. Extracted from
 * duplicate error handling patterns found in cpu_6502.cpp and cpu_6809.cpp.
 *
 * Pattern Extraction: Phase 2.4 - CPU Error Handling Refactoring
 * Date: 2026-02-14
 */

#ifndef XASM_CPU_ERROR_UTILS_H
#define XASM_CPU_ERROR_UTILS_H

#include <stdexcept>
#include <string>

namespace xasm {
namespace cpu {

/**
 * @brief Throw error for unsupported instruction mnemonic
 *
 * Used in EncodeInstruction() when dispatcher doesn't recognize mnemonic.
 *
 * Pattern found in:
 * - cpu_6502.cpp:2253: throw std::invalid_argument("Unsupported instruction: " + mnemonic);
 * - cpu_6809.cpp:1907: throw std::invalid_argument("Unsupported instruction: " + mnemonic);
 *
 * @param mnemonic The unsupported instruction mnemonic (e.g., "XYZ")
 * @throws std::invalid_argument with message "Unsupported instruction: <mnemonic>"
 */
[[noreturn]] inline void ThrowUnsupportedInstruction(const std::string &mnemonic) {
  throw std::invalid_argument("Unsupported instruction: " + mnemonic);
}

/**
 * @brief Throw error for unsupported special encoding
 *
 * Used in EncodeInstructionSpecial() when instruction doesn't support
 * special encoding paths (branch relaxation, multi-operand instructions).
 *
 * Pattern found in:
 * - cpu_6502.cpp:2407: throw std::invalid_argument("Special encoding not supported for instruction: " + mnemonic);
 *
 * @param mnemonic The instruction mnemonic (e.g., "JMP")
 * @throws std::invalid_argument with message "Special encoding not supported for instruction: <mnemonic>"
 */
[[noreturn]] inline void ThrowSpecialEncodingNotSupported(const std::string &mnemonic) {
  throw std::invalid_argument("Special encoding not supported for instruction: " + mnemonic);
}

/**
 * @brief Throw error when hex value expected but not found
 *
 * Used when parsing operand strings that should start with $ for hex values.
 *
 * Pattern found in:
 * - cpu_6502.cpp:2321: throw std::runtime_error("Expected hex value starting with $");
 *
 * @throws std::runtime_error with message "Expected hex value starting with $"
 */
[[noreturn]] inline void ThrowExpectedHexValue() {
  throw std::runtime_error("Expected hex value starting with $");
}

/**
 * @brief Throw error when branch target is not a resolved address
 *
 * Used in EncodeInstructionSpecial() for branch instructions when target
 * address should be resolved but is still a label reference.
 *
 * Pattern found in:
 * - cpu_6502.cpp:2340: throw std::runtime_error("Branch target must be resolved address");
 *
 * @throws std::runtime_error with message "Branch target must be resolved address"
 */
[[noreturn]] inline void ThrowBranchTargetMustBeResolved() {
  throw std::runtime_error("Branch target must be resolved address");
}

/**
 * @brief Throw error when instruction requires two operands
 *
 * Used for instructions like MVN/MVP (65816) that require exactly two
 * operands in specific format.
 *
 * Pattern found in:
 * - cpu_6502.cpp:2376: throw std::runtime_error(mnemonic + " requires two operands: srcbank,destbank");
 *
 * @param mnemonic The instruction mnemonic (e.g., "MVN")
 * @param expected_format Description of expected operand format (e.g., "srcbank,destbank")
 * @throws std::runtime_error with message "<mnemonic> requires two operands: <expected_format>"
 */
[[noreturn]] inline void ThrowRequiresTwoOperands(
    const std::string &mnemonic,
    const std::string &expected_format) {
  throw std::runtime_error(mnemonic + " requires two operands: " + expected_format);
}

/**
 * @brief Throw error for invalid operand values
 *
 * Used when operand parsing fails or values are out of range. Includes
 * details about what went wrong (often from nested exception).
 *
 * Pattern found in:
 * - cpu_6502.cpp:2401: throw std::runtime_error("Invalid bank values for " + mnemonic + ": " + e.what());
 *
 * @param mnemonic The instruction mnemonic (e.g., "MVN")
 * @param reason Description of what's invalid (e.g., "out of range", exception message)
 * @throws std::runtime_error with message "Invalid bank values for <mnemonic>: <reason>"
 */
[[noreturn]] inline void ThrowInvalidValues(
    const std::string &mnemonic,
    const std::string &reason) {
  throw std::runtime_error("Invalid bank values for " + mnemonic + ": " + reason);
}

} // namespace cpu
} // namespace xasm

#endif // XASM_CPU_ERROR_UTILS_H
