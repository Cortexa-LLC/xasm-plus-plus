// 6502 Branch Relaxation Handler
// Extracted from Cpu6502 Large class refactoring - improving SRP compliance
// (M6.1)

#pragma once

#include <cstdint>
#include <vector>

namespace xasm {

/**
 * @brief Handles 6502 branch relaxation logic
 *
 * This class extracts branch-related functionality from Cpu6502,
 * specifically handling out-of-range branch instructions by converting
 * them to equivalent longer sequences.
 *
 * Branch relaxation converts:
 *   BEQ far_label (out of range)
 * Into:
 *   BNE *+5       (inverted condition, skip JMP)
 *   JMP far_label (absolute jump to target)
 */
class Cpu6502BranchHandler {
public:
  Cpu6502BranchHandler() = default;

  /**
   * @brief Check if branch needs relaxation (out of range)
   *
   * Branches can only jump -128 to +127 bytes relative to PC+2.
   * If target is outside this range, branch relaxation is needed.
   *
   * @param current_addr Address where branch instruction will be located
   * @param target_addr Target address to branch to
   * @return true if branch is out of range and needs relaxation
   */
  bool NeedsBranchRelaxation(uint16_t current_addr, uint16_t target_addr) const;

  /**
   * @brief Get complementary (inverted) branch opcode
   *
   * All 6502 branch opcodes can be inverted by XORing with 0x20.
   * This is used for branch relaxation: BEQ far → BNE *+5; JMP far
   *
   * Examples:
   *   BEQ (0xF0) → BNE (0xD0)  [0xF0 XOR 0x20 = 0xD0]
   *   BCC (0x90) → BCS (0xB0)  [0x90 XOR 0x20 = 0xB0]
   *
   * @param branch_opcode Original branch opcode
   * @return Complementary branch opcode
   */
  uint8_t GetComplementaryBranchOpcode(uint8_t branch_opcode) const;

  /**
   * @brief Encode branch instruction with automatic relaxation if needed
   *
   * If branch target is in range (-128 to +127 bytes):
   *   Emits normal 2-byte branch: [opcode] [offset]
   *
   * If branch target is out of range:
   *   Emits relaxed 5-byte sequence: [B!cc] [0x03] [JMP] [target_lo]
   * [target_hi]
   *
   * Example: BEQ $1200 from $1000 (offset = +510, out of range)
   *   Normal:  F0 7E (fails - offset too large)
   *   Relaxed: D0 03 4C 00 12 (BNE *+5; JMP $1200)
   *
   * @param branch_opcode Branch opcode (BEQ, BNE, BCC, BCS, etc.)
   * @param current_addr Address where branch instruction will be located
   * @param target_addr Target address to branch to
   * @return Encoded bytes (2 bytes if in range, 5 bytes if relaxed)
   */
  std::vector<uint8_t> EncodeBranchWithRelaxation(uint8_t branch_opcode,
                                                  uint16_t current_addr,
                                                  uint16_t target_addr) const;
};

} // namespace xasm
