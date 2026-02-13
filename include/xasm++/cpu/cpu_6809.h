/**
 * @file cpu_6809.h
 * @brief Motorola 6809 CPU Plugin for instruction encoding
 *
 * This file defines the CPU plugin for the Motorola 6809 processor,
 * used in the TRS-80 Color Computer (CoCo), Dragon 32/64, and Vectrex.
 * The 6809 is one of the most sophisticated 8-bit CPUs ever designed,
 * featuring advanced indexed addressing modes and 16-bit operations.
 *
 * @note Key differences from 6502:
 * - Big-endian byte order (MSB first, vs 6502's little-endian)
 * - 16 indexed addressing sub-modes (most complex of any 8-bit CPU)
 * - Dual stack pointers (S for system, U for user)
 * - Direct page register (configurable, not fixed to $00)
 */

#pragma once

#include "xasm++/cpu/cpu_plugin.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Addressing modes supported by the Motorola 6809
 *
 * The 6809 has 13 primary addressing modes with 16 indexed sub-modes,
 * making it one of the most flexible 8-bit CPUs.
 */
enum class AddressingMode6809 {
  Inherent,    ///< No operand (e.g., NOP, CLRA, RTS)
  Immediate8,  ///< 8-bit immediate value (e.g., LDA #$42)
  Immediate16, ///< 16-bit immediate value (e.g., LDD #$1234)
  Direct,      ///< Direct page address (e.g., LDA <$80)
  Extended,    ///< 16-bit absolute address (e.g., LDA $1234)

  // Indexed modes (16 sub-modes encoded in post-byte)
  IndexedZeroOffset,       ///< Zero offset (e.g., ,X ,Y ,U ,S)
  Indexed5BitOffset,       ///< 5-bit constant offset (-16 to +15)
  Indexed8BitOffset,       ///< 8-bit constant offset (-128 to +127)
  Indexed16BitOffset,      ///< 16-bit constant offset (-32768 to +32767)
  IndexedAccumA,           ///< A accumulator offset (e.g., A,X)
  IndexedAccumB,           ///< B accumulator offset (e.g., B,Y)
  IndexedAccumD,           ///< D accumulator offset (e.g., D,U)
  IndexedAutoInc1,         ///< Auto-increment by 1 (e.g., ,X+)
  IndexedAutoInc2,         ///< Auto-increment by 2 (e.g., ,X++)
  IndexedAutoDec1,         ///< Auto-decrement by 1 (e.g., ,-X)
  IndexedAutoDec2,         ///< Auto-decrement by 2 (e.g., ,--X)
  IndexedPCRelative8,      ///< 8-bit PC relative (e.g., n,PCR)
  IndexedPCRelative16,     ///< 16-bit PC relative (e.g., label,PCR)
  IndexedIndirect,         ///< Indirect indexed (e.g., [,X] [10,Y])
  IndexedExtendedIndirect, ///< Extended indirect (e.g., [$1234])

  // Relative modes (branches only)
  Relative8,  ///< 8-bit relative (-128 to +127 bytes)
  Relative16, ///< 16-bit relative (-32768 to +32767 bytes)
};

/**
 * @brief Motorola 6809 CPU Plugin - encodes instructions for the 6809
 *
 * This class provides instruction encoding services for the Motorola 6809
 * processor. It converts assembly language mnemonics and operands into
 * machine code bytes.
 *
 * @par Key Features
 * - **59 fundamental opcodes** with multiple addressing modes
 * - **Big-endian byte order** (MSB first, unlike 6502)
 * - **Complex indexed addressing** (16 sub-modes with post-byte encoding)
 * - **Dual stack pointers** (S for system, U for user)
 * - **Direct page register** (configurable base for direct addressing)
 *
 * @par Multi-page Opcode Space
 * The 6809 uses three opcode pages:
 * - **Page 1**: Standard opcodes (1 byte)
 * - **Page 2**: Extended opcodes (prefix $10, 2 bytes)
 * - **Page 3**: Extended opcodes (prefix $11, 2 bytes)
 *
 * @par Usage Example
 * @code
 * Cpu6809 cpu;
 *
 * // Encode "LDA #$42" (8-bit immediate)
 * auto bytes = cpu.EncodeLDA(0x42, AddressingMode6809::Immediate8);
 * // Returns: {0x86, 0x42}
 *
 * // Encode "LDD #$1234" (16-bit immediate, big-endian)
 * auto bytes = cpu.EncodeLDD(0x1234, AddressingMode6809::Immediate16);
 * // Returns: {0xCC, 0x12, 0x34}  // Note: MSB first (big-endian)
 * @endcode
 */
class Cpu6809 : public CpuPlugin {
public:
  /**
   * @brief Default constructor - initializes to 6809 mode
   */
  Cpu6809() = default;

  /**
   * @brief Virtual destructor
   */
  ~Cpu6809() override = default;

  // CpuPlugin interface implementation
  std::string GetCpuFamily() const override { return "6809"; }
  std::vector<std::string> GetSupportedVariants() const override {
    return {"6809"};
  }
  bool HasOpcode(const std::string &mnemonic) const override;

  /**
   * @brief Get the CPU plugin name
   * @return "6809"
   */
  std::string GetName() const { return "6809"; }

  // CpuPlugin instruction encoding interface
  std::vector<uint8_t>
  EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                    const std::string &operand_str) const override;

  /**
   * @brief Set the direct page register
   *
   * The 6809 direct page register (DP) sets the high byte for direct
   * addressing. Unlike the 6502's fixed zero page ($00xx), the 6809
   * can use any 256-byte page in memory.
   *
   * @param dp Direct page value (0x00-0xFF)
   *
   * @par Example
   * @code
   * cpu.SetDirectPage(0x20);  // Direct page = $20xx
   * auto bytes = cpu.EncodeLDA(0x80, AddressingMode6809::Direct);
   * // Accesses address $2080, not $0080
   * @endcode
   */
  void SetDirectPage(uint8_t dp);

  /**
   * @brief Get the current direct page register value
   * @return Current DP value (0x00-0xFF)
   */
  uint8_t GetDirectPage() const;

  /**
   * @name Data Movement Instructions
   * @{
   */

  /**
   * @brief Encode LDA (Load Accumulator A) instruction
   *
   * @param operand Operand value (address or immediate value)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @par Supported Modes
   * Immediate8, Direct, Extended, all Indexed modes
   */
  std::vector<uint8_t> EncodeLDA(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode LDB (Load Accumulator B) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeLDB(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode LDD (Load Accumulator D) instruction
   *
   * D is the 16-bit concatenation of A:B (A=high byte, B=low byte).
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses big-endian byte order: LDD #$1234 loads A=$12, B=$34
   */
  std::vector<uint8_t> EncodeLDD(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode STA (Store Accumulator A) instruction
   *
   * @param operand Operand value (address)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note STA does not support Immediate mode
   */
  std::vector<uint8_t> EncodeSTA(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode STB (Store Accumulator B) instruction
   *
   * @param operand Operand value (address)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeSTB(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode STD (Store Accumulator D) instruction
   *
   * @param operand Operand value (address)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses big-endian byte order: stores A (high byte) first
   */
  std::vector<uint8_t> EncodeSTD(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode LDX (Load Index Register X) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses big-endian byte order for 16-bit values
   */
  std::vector<uint8_t> EncodeLDX(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode LDY (Load Index Register Y) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses page 2 prefix ($10), big-endian byte order
   */
  std::vector<uint8_t> EncodeLDY(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode STX (Store Index Register X) instruction
   *
   * @param operand Operand value (address)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeSTX(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode STY (Store Index Register Y) instruction
   *
   * @param operand Operand value (address)
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses page 2 prefix ($10)
   */
  std::vector<uint8_t> EncodeSTY(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /** @} */ // End of Data Movement Instructions

  /**
   * @defgroup arithmetic Arithmetic Instructions
   * @{
   */

  /**
   * @brief Encode ADDA (Add to Accumulator A) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeADDA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode ADDB (Add to Accumulator B) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeADDB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode SUBA (Subtract from Accumulator A) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeSUBA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode SUBB (Subtract from Accumulator B) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeSUBB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode CMPA (Compare A with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeCMPA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode CMPB (Compare B with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeCMPB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode CMPX (Compare X with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeCMPX(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode CMPY (Compare Y with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   *
   * @note Uses page 2 prefix ($10)
   */
  std::vector<uint8_t> EncodeCMPY(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /** @} */ // End of Arithmetic Instructions

  /**
   * @defgroup logical Logical Operations
   * @{
   */

  /**
   * @brief Encode ANDA (AND A with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeANDA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode ANDB (AND B with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeANDB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode ORA (OR A with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeORA(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode ORB (OR B with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeORB(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode EORA (Exclusive OR A with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeEORA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode EORB (Exclusive OR B with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeEORB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /** @} */ // End of Logical Operations

  /**
   * @defgroup control Control Flow Instructions
   * @{
   */

  /**
   * @brief Encode JSR (Jump to Subroutine) instruction
   *
   * @param operand Target address
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeJSR(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode JMP (Jump) instruction
   *
   * @param operand Target address
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeJMP(uint32_t operand,
                                 AddressingMode6809 mode) const;

  /**
   * @brief Encode LEAX (Load Effective Address into X) instruction
   *
   * @param operand Address or offset
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeLEAX(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode LEAY (Load Effective Address into Y) instruction
   *
   * @param operand Address or offset
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeLEAY(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /** @} */ // End of Control Flow Instructions

  /**
   * @defgroup branch Branch Instructions
   * @{
   */

  /**
   * @brief Encode BRA (Branch Always) instruction
   *
   * @param offset Signed offset from PC (-128 to +127 for 8-bit)
   * @param mode Addressing mode (Relative8 or Relative16)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBRA(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BEQ (Branch if Equal) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBEQ(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BNE (Branch if Not Equal) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBNE(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BCC (Branch if Carry Clear) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBCC(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BCS (Branch if Carry Set) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBCS(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BMI (Branch if Minus) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBMI(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BPL (Branch if Plus) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBPL(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BVS (Branch if Overflow Set) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBVS(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BVC (Branch if Overflow Clear) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBVC(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BGE (Branch if Greater or Equal - signed) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBGE(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BLT (Branch if Less Than - signed) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBLT(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BGT (Branch if Greater Than - signed) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBGT(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BLE (Branch if Less or Equal - signed) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBLE(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BHI (Branch if Higher - unsigned) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBHI(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BLS (Branch if Lower or Same - unsigned) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBLS(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode BSR (Branch to Subroutine) instruction
   *
   * @param offset Signed offset from PC
   * @param mode Addressing mode (Relative8)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBSR(int32_t offset, AddressingMode6809 mode) const;

  /**
   * @brief Encode LBRA (Long Branch Always) instruction
   *
   * @param offset Signed 16-bit offset from PC (-32768 to +32767)
   * @return Vector of encoded bytes (4 bytes: 0x10 0x16 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBRA(int16_t offset) const;

  /**
   * @brief Encode LBSR (Long Branch to Subroutine) instruction
   *
   * @param offset Signed 16-bit offset from PC (-32768 to +32767)
   * @return Vector of encoded bytes (4 bytes: 0x10 0x17 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBSR(int16_t offset) const;

  /**
   * @brief Encode LBRN (Long Branch Never) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x21 offset_msb offset_lsb)
   * @note This instruction never branches but takes 4 bytes and time
   */
  std::vector<uint8_t> EncodeLBRN(int16_t offset) const;

  /**
   * @brief Encode LBHI (Long Branch if Higher - unsigned) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x22 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBHI(int16_t offset) const;

  /**
   * @brief Encode LBLS (Long Branch if Lower or Same - unsigned) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x23 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBLS(int16_t offset) const;

  /**
   * @brief Encode LBCC/LBHS (Long Branch if Carry Clear) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x24 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBCC(int16_t offset) const;

  /**
   * @brief Encode LBCS/LBLO (Long Branch if Carry Set) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x25 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBCS(int16_t offset) const;

  /**
   * @brief Encode LBNE (Long Branch if Not Equal) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x26 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBNE(int16_t offset) const;

  /**
   * @brief Encode LBEQ (Long Branch if Equal) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x27 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBEQ(int16_t offset) const;

  /**
   * @brief Encode LBVC (Long Branch if Overflow Clear) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x28 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBVC(int16_t offset) const;

  /**
   * @brief Encode LBVS (Long Branch if Overflow Set) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x29 offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBVS(int16_t offset) const;

  /**
   * @brief Encode LBPL (Long Branch if Plus) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2A offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBPL(int16_t offset) const;

  /**
   * @brief Encode LBMI (Long Branch if Minus) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2B offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBMI(int16_t offset) const;

  /**
   * @brief Encode LBGE (Long Branch if Greater or Equal - signed) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2C offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBGE(int16_t offset) const;

  /**
   * @brief Encode LBLT (Long Branch if Less Than - signed) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2D offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBLT(int16_t offset) const;

  /**
   * @brief Encode LBGT (Long Branch if Greater Than - signed) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2E offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBGT(int16_t offset) const;

  /**
   * @brief Encode LBLE (Long Branch if Less or Equal - signed) instruction
   *
   * @param offset Signed 16-bit offset from PC
   * @return Vector of encoded bytes (4 bytes: 0x10 0x2F offset_msb offset_lsb)
   */
  std::vector<uint8_t> EncodeLBLE(int16_t offset) const;

  /** @} */ // End of Branch Instructions

  /**
   * @defgroup stack Stack Operations
   * @{
   */

  /**
   * @brief Encode PSHS (Push registers to System stack) instruction
   *
   * @param mask Register mask (bit 0=CC, 1=A, 2=B, 3=DP, 4=X, 5=Y, 6=U, 7=PC)
   * @return Vector of encoded bytes
   *
   * @par Register Mask Bits
   * - Bit 0 (0x01): CC (Condition Codes)
   * - Bit 1 (0x02): A
   * - Bit 2 (0x04): B
   * - Bit 3 (0x08): DP (Direct Page)
   * - Bit 4 (0x10): X
   * - Bit 5 (0x20): Y
   * - Bit 6 (0x40): U (User stack)
   * - Bit 7 (0x80): PC (Program Counter)
   */
  std::vector<uint8_t> EncodePSHS(uint8_t mask) const;

  /**
   * @brief Encode PULS (Pull registers from System stack) instruction
   *
   * @param mask Register mask (same as PSHS)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodePULS(uint8_t mask) const;

  /**
   * @brief Encode PSHU (Push registers to User stack) instruction
   *
   * @param mask Register mask (bit 0=CC, 1=A, 2=B, 3=DP, 4=X, 5=Y, 6=S, 7=PC)
   * @return Vector of encoded bytes
   *
   * @note For PSHU/PULU, bit 6 is S (System stack) instead of U
   */
  std::vector<uint8_t> EncodePSHU(uint8_t mask) const;

  /**
   * @brief Encode PULU (Pull registers from User stack) instruction
   *
   * @param mask Register mask (same as PSHU)
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodePULU(uint8_t mask) const;

  /** @} */ // End of Stack Operations

  /**
   * @defgroup transfer Register Transfer and Exchange
   * @{
   */

  /**
   * @brief Encode TFR (Transfer Register) instruction
   *
   * @param src Source register (0-15)
   * @param dst Destination register (0-15)
   * @return Vector of encoded bytes
   *
   * @par Register Encoding
   * - 0: D (16-bit)
   * - 1: X (16-bit)
   * - 2: Y (16-bit)
   * - 3: U (16-bit)
   * - 4: S (16-bit)
   * - 5: PC (16-bit)
   * - 8: A (8-bit)
   * - 9: B (8-bit)
   * - 10: CC (8-bit)
   * - 11: DP (8-bit)
   */
  std::vector<uint8_t> EncodeTFR(uint8_t src, uint8_t dst) const;

  /**
   * @brief Encode EXG (Exchange Registers) instruction
   *
   * @param reg1 First register (0-15)
   * @param reg2 Second register (0-15)
   * @return Vector of encoded bytes
   *
   * @note Register encoding same as TFR
   */
  std::vector<uint8_t> EncodeEXG(uint8_t reg1, uint8_t reg2) const;

  /** @} */ // End of Register Transfer and Exchange

  /**
   * @name Inherent Instructions
   * @{
   */

  /**
   * @brief Encode NOP (No Operation) instruction
   * @return Vector containing single NOP opcode byte {0x12}
   */
  std::vector<uint8_t> EncodeNOP() const;

  /**
   * @brief Encode RTS (Return from Subroutine) instruction
   * @return Vector containing single RTS opcode byte {0x39}
   */
  std::vector<uint8_t> EncodeRTS() const;

  /**
   * @brief Encode CLRA (Clear Accumulator A) instruction
   * @return Vector containing single CLRA opcode byte {0x4F}
   */
  std::vector<uint8_t> EncodeCLRA() const;

  /**
   * @brief Encode CLRB (Clear Accumulator B) instruction
   * @return Vector containing single CLRB opcode byte {0x5F}
   */
  std::vector<uint8_t> EncodeCLRB() const;

  // Shift and Rotate Instructions
  std::vector<uint8_t> EncodeASLA() const; ///< Arithmetic Shift Left A
  std::vector<uint8_t> EncodeASLB() const; ///< Arithmetic Shift Left B
  std::vector<uint8_t> EncodeASRA() const; ///< Arithmetic Shift Right A
  std::vector<uint8_t> EncodeASRB() const; ///< Arithmetic Shift Right B
  std::vector<uint8_t> EncodeLSRA() const; ///< Logical Shift Right A
  std::vector<uint8_t> EncodeLSRB() const; ///< Logical Shift Right B
  std::vector<uint8_t> EncodeROLA() const; ///< Rotate Left A through Carry
  std::vector<uint8_t> EncodeROLB() const; ///< Rotate Left B through Carry
  std::vector<uint8_t> EncodeRORA() const; ///< Rotate Right A through Carry
  std::vector<uint8_t> EncodeRORB() const; ///< Rotate Right B through Carry

  // Increment/Decrement Instructions
  std::vector<uint8_t> EncodeINCA() const; ///< Increment A
  std::vector<uint8_t> EncodeINCB() const; ///< Increment B
  std::vector<uint8_t> EncodeDECA() const; ///< Decrement A
  std::vector<uint8_t> EncodeDECB() const; ///< Decrement B

  // Test/Compare/Negate Instructions
  std::vector<uint8_t> EncodeTSTA() const; ///< Test A (set flags)
  std::vector<uint8_t> EncodeTSTB() const; ///< Test B (set flags)
  std::vector<uint8_t> EncodeCOMA() const; ///< Complement A (one's complement)
  std::vector<uint8_t> EncodeCOMB() const; ///< Complement B (one's complement)
  std::vector<uint8_t> EncodeNEGA() const; ///< Negate A (two's complement)
  std::vector<uint8_t> EncodeNEGB() const; ///< Negate B (two's complement)

  /** @} */ // End of Inherent Instructions

  /**
   * @defgroup bitops Bit Test Operations
   * @{
   */

  /**
   * @brief Encode BITA (Bit Test A with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBITA(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /**
   * @brief Encode BITB (Bit Test B with memory) instruction
   *
   * @param operand Operand value
   * @param mode Addressing mode
   * @return Vector of encoded bytes
   */
  std::vector<uint8_t> EncodeBITB(uint32_t operand,
                                  AddressingMode6809 mode) const;

  /** @} */ // End of Bit Test Operations

  /**
   * @brief Calculate the size of an encoded instruction
   *
   * Returns the number of bytes required to encode an instruction
   * with the given addressing mode.
   *
   * @param mode Addressing mode
   * @return Instruction size in bytes (1-5 bytes)
   *
   * @par Size by Mode
   * - Inherent: 1 byte (or 2 for page 2/3 opcodes)
   * - Immediate8/Direct/Indexed simple: 2-3 bytes
   * - Extended: 3 bytes
   * - Indexed complex: 2-5 bytes (depends on offset size)
   */
  size_t CalculateInstructionSize(AddressingMode6809 mode) const;

private:
  /**
   * @brief Encode 16-bit value in big-endian byte order
   *
   * The 6809 uses big-endian byte order (MSB first), unlike the 6502
   * which uses little-endian (LSB first).
   *
   * @param value 16-bit value to encode
   * @return Vector containing {high_byte, low_byte}
   *
   * @par Example
   * @code
   * ToBigEndian(0x1234) -> {0x12, 0x34}  // MSB first
   * @endcode
   */
  std::vector<uint8_t> ToBigEndian(uint16_t value) const;

  /**
   * @brief Encode indexed addressing post-byte
   *
   * The 6809's indexed addressing uses a post-byte to encode 16 different
   * sub-modes. This is the most complex feature of the 6809.
   *
   * @param mode Indexed addressing mode
   * @param offset Offset value (if applicable)
   * @param reg Index register (0=X, 1=Y, 2=U, 3=S)
   * @return Vector of post-byte and any additional offset bytes
   *
   * @note This is a complex encoding - see 6809-ARCHITECTURE.md for details
   */
  std::vector<uint8_t> EncodeIndexedPostByte(AddressingMode6809 mode,
                                             int32_t offset, uint8_t reg) const;

private:
  uint8_t direct_page_ = 0x00; ///< Direct page register (default $00)
};

} // namespace xasm
