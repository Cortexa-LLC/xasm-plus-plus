/**
 * @file cpu_z80.h
 * @brief Zilog Z80 CPU Plugin for instruction encoding
 *
 * This file defines the CPU plugin for the Zilog Z80 processor,
 * used in systems like Game Boy, ZX Spectrum, TRS-80 Model I, and CP/M.
 * The Z80 extends the Intel 8080 with additional registers (IX, IY)
 * and new instructions.
 *
 * @note Key features:
 * - ~150 opcodes (8080 base + Z80 extensions)
 * - IX/IY index registers with displacement addressing
 * - Bit manipulation instructions (BIT, SET, RES)
 * - Block transfer and search instructions (LDIR, CPIR, etc.)
 * - Multiple prefix bytes (CB, DD, ED, FD)
 * - Little-endian byte order
 */

#pragma once

#include "xasm++/cpu/cpu_plugin.h"
#include <cstdint>
#include <string>
#include <vector>

namespace xasm {

/**
 * @brief Zilog Z80 CPU Plugin - encodes instructions for the Z80
 *
 * This class provides instruction encoding services for the Zilog Z80
 * processor. It converts assembly language mnemonics and operands into
 * machine code bytes.
 *
 * @par Key Features
 * - **~150 fundamental opcodes** (8080 + Z80 extensions)
 * - **Little-endian byte order** (LSB first, like 6502)
 * - **Prefix bytes**:
 *   - 0xCB: Bit manipulation instructions
 *   - 0xDD: IX index register operations
 *   - 0xED: Extended instructions
 *   - 0xFD: IY index register operations
 * - **Index registers**: IX and IY with 8-bit signed displacement
 * - **Alternate register set**: A', F', BC', DE', HL'
 *
 * @par Usage Example
 * @code
 * CpuZ80 cpu;
 *
 * // Encode "LD A, $42"
 * auto bytes = cpu.EncodeLD_A_n(0x42);
 * // Returns: {0x3E, 0x42}
 *
 * // Encode "LD BC, $1234" (little-endian)
 * auto bytes = cpu.EncodeLD_BC_nn(0x1234);
 * // Returns: {0x01, 0x34, 0x12}  // Note: LSB first (little-endian)
 *
 * // Encode "BIT 0, A"
 * auto bytes = cpu.EncodeBIT(0, 0x07); // Register A = 0x07
 * // Returns: {0xCB, 0x47}
 *
 * // Encode "LD IX, $4000"
 * auto bytes = cpu.EncodeLD_IX_nn(0x4000);
 * // Returns: {0xDD, 0x21, 0x00, 0x40}
 * @endcode
 */
class CpuZ80 : public CpuPlugin {
public:
  /**
   * @brief Default constructor - initializes Z80 mode
   */
  CpuZ80() = default;

  /**
   * @brief Virtual destructor
   */
  ~CpuZ80() override = default;

  // CpuPlugin interface implementation
  std::string GetCpuFamily() const override { return "Z80"; }
  std::vector<std::string> GetSupportedVariants() const override {
    return {"Z80", "GameBoy"};
  }
  bool HasOpcode(const std::string &mnemonic) const override;

  /**
   * @brief Get the CPU plugin name
   * @return "Z80"
   */
  static std::string GetName() { return "Z80"; }

  // CpuPlugin instruction encoding interface
  std::vector<uint8_t>
  EncodeInstruction(const std::string &mnemonic, uint32_t operand,
                    const std::string &operand_str) const override;

  /**
   * @name 8-bit Load Instructions
   * @{
   */

  /**
   * @brief Encode LD A, n instruction (load immediate into A)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x3E, n}
   */
  static std::vector<uint8_t> EncodeLD_A_n(uint8_t value);

  /**
   * @brief Encode LD B, n instruction (load immediate into B)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x06, n}
   */
  static std::vector<uint8_t> EncodeLD_B_n(uint8_t value);

  /**
   * @brief Encode LD C, n instruction (load immediate into C)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x0E, n}
   */
  static std::vector<uint8_t> EncodeLD_C_n(uint8_t value);

  /**
   * @brief Encode LD D, n instruction (load immediate into D)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x16, n}
   */
  static std::vector<uint8_t> EncodeLD_D_n(uint8_t value);

  /**
   * @brief Encode LD E, n instruction (load immediate into E)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x1E, n}
   */
  static std::vector<uint8_t> EncodeLD_E_n(uint8_t value);

  /**
   * @brief Encode LD H, n instruction (load immediate into H)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x26, n}
   */
  static std::vector<uint8_t> EncodeLD_H_n(uint8_t value);

  /**
   * @brief Encode LD L, n instruction (load immediate into L)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x2E, n}
   */
  static std::vector<uint8_t> EncodeLD_L_n(uint8_t value);

  /** @} */ // End of 8-bit Load Instructions

  /**
   * @name 16-bit Load Instructions
   * @{
   */

  /**
   * @brief Encode LD BC, nn instruction (load immediate into BC)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0x01, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_BC_nn(uint16_t value) ;

  /**
   * @brief Encode LD DE, nn instruction (load immediate into DE)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0x11, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_DE_nn(uint16_t value) ;

  /**
   * @brief Encode LD HL, nn instruction (load immediate into HL)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0x21, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_HL_nn(uint16_t value) ;

  /**
   * @brief Encode LD SP, nn instruction (load immediate into SP)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0x31, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_SP_nn(uint16_t value) ;

  /**
   * @brief Encode LD A, (BC) instruction (load A from address in BC)
   *
   * @return Vector of encoded bytes {0x0A}
   */
  static std::vector<uint8_t> EncodeLD_A_BC();

  /**
   * @brief Encode LD A, (DE) instruction (load A from address in DE)
   *
   * @return Vector of encoded bytes {0x1A}
   */
  static std::vector<uint8_t> EncodeLD_A_DE();

  /**
   * @brief Encode LD (BC), A instruction (store A to address in BC)
   *
   * @return Vector of encoded bytes {0x02}
   */
  static std::vector<uint8_t> EncodeLD_BC_A();

  /**
   * @brief Encode LD (DE), A instruction (store A to address in DE)
   *
   * @return Vector of encoded bytes {0x12}
   */
  static std::vector<uint8_t> EncodeLD_DE_A();

  /**
   * @brief Encode LD (HL), n instruction (store immediate to address in HL)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0x36, n}
   */
  static std::vector<uint8_t> EncodeLD_HL_n(uint8_t value);

  /**
   * @brief Encode LD HL, (nn) instruction (load HL from memory address)
   *
   * @param address 16-bit memory address
   * @return Vector of encoded bytes {0x2A, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_HL_addr(uint16_t address) ;

  /**
   * @brief Encode LD (nn), HL instruction (store HL to memory address)
   *
   * @param address 16-bit memory address
   * @return Vector of encoded bytes {0x22, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_addr_HL(uint16_t address) ;

  /**
   * @brief Encode LD SP, HL instruction (copy HL to stack pointer)
   *
   * @return Vector of encoded bytes {0xF9}
   */
  static std::vector<uint8_t> EncodeLD_SP_HL();

  /** @} */ // End of 16-bit Load Instructions

  /**
   * @name Arithmetic Instructions
   * @{
   */

  /**
   * @brief Encode ADD A, n instruction (add immediate to A)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xC6, n}
   */
  static std::vector<uint8_t> EncodeADD_A_n(uint8_t value);

  /**
   * @brief Encode SUB n instruction (subtract immediate from A)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xD6, n}
   */
  static std::vector<uint8_t> EncodeSUB_n(uint8_t value);

  /**
   * @brief Encode INC A instruction (increment A)
   *
   * @return Vector of encoded bytes {0x3C}
   */
  static std::vector<uint8_t> EncodeINC_A();

  /**
   * @brief Encode DEC A instruction (decrement A)
   *
   * @return Vector of encoded bytes {0x3D}
   */
  static std::vector<uint8_t> EncodeDEC_A();

  /** @} */ // End of Arithmetic Instructions

  /**
   * @name Control Flow Instructions
   * @{
   */

  /**
   * @brief Encode NOP instruction (no operation)
   *
   * @return Vector of encoded bytes {0x00}
   */
  static std::vector<uint8_t> EncodeNOP();

  /**
   * @brief Encode JP nn instruction (jump to absolute address)
   *
   * @param address 16-bit target address
   * @return Vector of encoded bytes {0xC3, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeJP_nn(uint16_t address) ;

  /**
   * @brief Encode RET instruction (return from subroutine)
   *
   * @return Vector of encoded bytes {0xC9}
   */
  static std::vector<uint8_t> EncodeRET();

  /** @} */ // End of Control Flow Instructions

  /**
   * @name Stack Operations
   * @{
   */

  /**
   * @brief Encode PUSH BC instruction (push BC onto stack)
   *
   * @return Vector of encoded bytes {0xC5}
   */
  static std::vector<uint8_t> EncodePUSH_BC();

  /**
   * @brief Encode POP BC instruction (pop BC from stack)
   *
   * @return Vector of encoded bytes {0xC1}
   */
  static std::vector<uint8_t> EncodePOP_BC();

  /** @} */ // End of Stack Operations

  /**
   * @name Bit Operations (CB Prefix)
   * @{
   */

  /**
   * @brief Encode BIT b, r instruction (test bit in register)
   *
   * @param bit Bit number (0-7)
   * @param reg Register code (0=B, 1=C, 2=D, 3=E, 4=H, 5=L, 6=(HL), 7=A)
   * @return Vector of encoded bytes {0xCB, opcode}
   *
   * @note Opcode = 0x40 + (bit << 3) + reg
   */
  static std::vector<uint8_t> EncodeBIT(uint8_t bit, uint8_t reg);

  /**
   * @brief Encode SET b, r instruction (set bit in register)
   *
   * @param bit Bit number (0-7)
   * @param reg Register code (0=B, 1=C, 2=D, 3=E, 4=H, 5=L, 6=(HL), 7=A)
   * @return Vector of encoded bytes {0xCB, opcode}
   *
   * @note Opcode = 0xC0 + (bit << 3) + reg
   */
  static std::vector<uint8_t> EncodeSET(uint8_t bit, uint8_t reg);

  /**
   * @brief Encode RES b, r instruction (reset bit in register)
   *
   * @param bit Bit number (0-7)
   * @param reg Register code (0=B, 1=C, 2=D, 3=E, 4=H, 5=L, 6=(HL), 7=A)
   * @return Vector of encoded bytes {0xCB, opcode}
   *
   * @note Opcode = 0x80 + (bit << 3) + reg
   */
  static std::vector<uint8_t> EncodeRES(uint8_t bit, uint8_t reg);

  /** @} */ // End of Bit Operations

  /**
   * @name IX Register Operations (DD Prefix)
   * @{
   */

  /**
   * @brief Encode LD IX, nn instruction (load immediate into IX)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0xDD, 0x21, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_IX_nn(uint16_t value) ;

  /**
   * @brief Encode LD A, (IX+d) instruction (load from indexed address)
   *
   * @param displacement Signed 8-bit displacement
   * @return Vector of encoded bytes {0xDD, 0x7E, displacement}
   */
  static std::vector<uint8_t> EncodeLD_A_IX_d(int8_t displacement);

  /** @} */ // End of IX Register Operations

  /**
   * @name IY Register Operations (FD Prefix)
   * @{
   */

  /**
   * @brief Encode LD IY, nn instruction (load immediate into IY)
   *
   * @param value Immediate 16-bit value
   * @return Vector of encoded bytes {0xFD, 0x21, low_byte, high_byte}
   *
   * @note Uses little-endian byte order (LSB first)
   */
  static std::vector<uint8_t> EncodeLD_IY_nn(uint16_t value) ;

  /**
   * @brief Encode LD A, (IY+d) instruction (load from indexed address)
   *
   * @param displacement Signed 8-bit displacement
   * @return Vector of encoded bytes {0xFD, 0x7E, displacement}
   */
  static std::vector<uint8_t> EncodeLD_A_IY_d(int8_t displacement);

  /** @} */ // End of IY Register Operations

  /**
   * @name Register-to-Register Load Instructions
   * @{
   */

  /**
   * @brief Encode LD A, B instruction (load B into A)
   * @return Vector of encoded bytes {0x78}
   */
  static std::vector<uint8_t> EncodeLD_A_B();

  /**
   * @brief Encode LD A, C instruction (load C into A)
   * @return Vector of encoded bytes {0x79}
   */
  static std::vector<uint8_t> EncodeLD_A_C();

  /**
   * @brief Encode LD B, A instruction (load A into B)
   * @return Vector of encoded bytes {0x47}
   */
  static std::vector<uint8_t> EncodeLD_B_A();

  /**
   * @brief Encode LD C, A instruction (load A into C)
   * @return Vector of encoded bytes {0x4F}
   */
  static std::vector<uint8_t> EncodeLD_C_A();

  /** @} */ // End of Register-to-Register Load Instructions

  /**
   * @name Additional Arithmetic Instructions
   * @{
   */

  /**
   * @brief Encode ADC A, n instruction (add with carry)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xCE, n}
   */
  static std::vector<uint8_t> EncodeADC_A_n(uint8_t value);

  /**
   * @brief Encode SBC A, n instruction (subtract with carry)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xDE, n}
   */
  static std::vector<uint8_t> EncodeSBC_A_n(uint8_t value);

  /**
   * @brief Encode CP n instruction (compare immediate with A)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xFE, n}
   */
  static std::vector<uint8_t> EncodeCP_n(uint8_t value);

  /** @} */ // End of Additional Arithmetic Instructions

  /**
   * @name Logical Operations
   * @{
   */

  /**
   * @brief Encode AND n instruction (logical AND with immediate)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xE6, n}
   */
  static std::vector<uint8_t> EncodeAND_n(uint8_t value);

  /**
   * @brief Encode OR n instruction (logical OR with immediate)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xF6, n}
   */
  static std::vector<uint8_t> EncodeOR_n(uint8_t value);

  /**
   * @brief Encode XOR n instruction (logical XOR with immediate)
   *
   * @param value Immediate 8-bit value
   * @return Vector of encoded bytes {0xEE, n}
   */
  static std::vector<uint8_t> EncodeXOR_n(uint8_t value);

  /**
   * @brief Encode XOR A instruction (clear A register)
   *
   * @return Vector of encoded bytes {0xAF}
   *
   * @note This is a common idiom to clear the accumulator
   */
  static std::vector<uint8_t> EncodeXOR_A();

  /** @} */ // End of Logical Operations

  /**
   * @name Branch Instructions
   * @{
   */

  /**
   * @brief Encode JR e instruction (relative jump always)
   *
   * @param offset Signed 8-bit displacement (-128 to +127)
   * @return Vector of encoded bytes {0x18, e}
   */
  static std::vector<uint8_t> EncodeJR_e(int8_t offset);

  /**
   * @brief Encode JR NZ, e instruction (jump if not zero)
   *
   * @param offset Signed 8-bit displacement
   * @return Vector of encoded bytes {0x20, e}
   */
  static std::vector<uint8_t> EncodeJR_NZ_e(int8_t offset);

  /**
   * @brief Encode JR Z, e instruction (jump if zero)
   *
   * @param offset Signed 8-bit displacement
   * @return Vector of encoded bytes {0x28, e}
   */
  static std::vector<uint8_t> EncodeJR_Z_e(int8_t offset);

  /**
   * @brief Encode JR NC, e instruction (jump if no carry)
   *
   * @param offset Signed 8-bit displacement
   * @return Vector of encoded bytes {0x30, e}
   */
  static std::vector<uint8_t> EncodeJR_NC_e(int8_t offset);

  /**
   * @brief Encode JR C, e instruction (jump if carry)
   *
   * @param offset Signed 8-bit displacement
   * @return Vector of encoded bytes {0x38, e}
   */
  static std::vector<uint8_t> EncodeJR_C_e(int8_t offset);

  /** @} */ // End of Branch Instructions

  /**
   * @name Memory Access Instructions
   * @{
   */

  /**
   * @brief Encode LD A, (nn) instruction (load from absolute address)
   *
   * @param address 16-bit address
   * @return Vector of encoded bytes {0x3A, low_byte, high_byte}
   */
  static std::vector<uint8_t> EncodeLD_A_addr(uint16_t address) ;

  /**
   * @brief Encode LD (nn), A instruction (store to absolute address)
   *
   * @param address 16-bit address
   * @return Vector of encoded bytes {0x32, low_byte, high_byte}
   */
  static std::vector<uint8_t> EncodeLD_addr_A(uint16_t address) ;

  /**
   * @brief Encode LD A, (HL) instruction (load from HL)
   * @return Vector of encoded bytes {0x7E}
   */
  static std::vector<uint8_t> EncodeLD_A_HL();

  /**
   * @brief Encode LD (HL), A instruction (store to HL)
   * @return Vector of encoded bytes {0x77}
   */
  static std::vector<uint8_t> EncodeLD_HL_A();

  /** @} */ // End of Memory Access Instructions

  /**
   * @name Rotate and Shift Instructions
   * @{
   */

  /**
   * @brief Encode RLCA instruction (rotate left circular accumulator)
   * @return Vector of encoded bytes {0x07}
   */
  static std::vector<uint8_t> EncodeRLCA();

  /**
   * @brief Encode RRCA instruction (rotate right circular accumulator)
   * @return Vector of encoded bytes {0x0F}
   */
  static std::vector<uint8_t> EncodeRRCA();

  /**
   * @brief Encode RLA instruction (rotate left through carry)
   * @return Vector of encoded bytes {0x17}
   */
  static std::vector<uint8_t> EncodeRLA();

  /**
   * @brief Encode RRA instruction (rotate right through carry)
   * @return Vector of encoded bytes {0x1F}
   */
  static std::vector<uint8_t> EncodeRRA();

  /**
   * @brief Encode RLD instruction (rotate left digit)
   *
   * Rotates the low 4 bits of (HL) into the low 4 bits of A,
   * and the low 4 bits of A into the high 4 bits of (HL).
   *
   * @return Vector of encoded bytes {0xED, 0x6F}
   */
  static std::vector<uint8_t> EncodeRLD();

  /**
   * @brief Encode RRD instruction (rotate right digit)
   *
   * Rotates the low 4 bits of A into the high 4 bits of (HL),
   * and the low 4 bits of (HL) into the low 4 bits of A.
   *
   * @return Vector of encoded bytes {0xED, 0x67}
   */
  static std::vector<uint8_t> EncodeRRD();

  /** @} */ // End of Rotate and Shift Instructions

  // ==========================================================================
  /**
   * @name Input/Output Instructions
   * @{
   */
  // ==========================================================================

  /**
   * @brief Encode IN A, (n) instruction
   *
   * Input from port n to accumulator A.
   *
   * @param port 8-bit port address
   * @return Vector of encoded bytes {0xDB, port}
   */
  static std::vector<uint8_t> EncodeIN_A_n(uint8_t port);

  /**
   * @brief Encode OUT (n), A instruction
   *
   * Output from accumulator A to port n.
   *
   * @param port 8-bit port address
   * @return Vector of encoded bytes {0xD3, port}
   */
  static std::vector<uint8_t> EncodeOUT_n_A(uint8_t port);

  /**
   * @brief Encode IN B, (C) instruction
   *
   * Input from port C to register B.
   *
   * @return Vector of encoded bytes {0xED, 0x40}
   */
  static std::vector<uint8_t> EncodeIN_B_C();

  /**
   * @brief Encode IN C, (C) instruction
   *
   * Input from port C to register C.
   *
   * @return Vector of encoded bytes {0xED, 0x48}
   */
  static std::vector<uint8_t> EncodeIN_C_C();

  /**
   * @brief Encode IN D, (C) instruction
   *
   * Input from port C to register D.
   *
   * @return Vector of encoded bytes {0xED, 0x50}
   */
  static std::vector<uint8_t> EncodeIN_D_C();

  /**
   * @brief Encode IN E, (C) instruction
   *
   * Input from port C to register E.
   *
   * @return Vector of encoded bytes {0xED, 0x58}
   */
  static std::vector<uint8_t> EncodeIN_E_C();

  /**
   * @brief Encode IN H, (C) instruction
   *
   * Input from port C to register H.
   *
   * @return Vector of encoded bytes {0xED, 0x60}
   */
  static std::vector<uint8_t> EncodeIN_H_C();

  /**
   * @brief Encode IN L, (C) instruction
   *
   * Input from port C to register L.
   *
   * @return Vector of encoded bytes {0xED, 0x68}
   */
  static std::vector<uint8_t> EncodeIN_L_C();

  /**
   * @brief Encode IN A, (C) instruction
   *
   * Input from port C to register A.
   *
   * @return Vector of encoded bytes {0xED, 0x78}
   */
  static std::vector<uint8_t> EncodeIN_A_C();

  /**
   * @brief Encode OUT (C), B instruction
   *
   * Output from register B to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x41}
   */
  static std::vector<uint8_t> EncodeOUT_C_B();

  /**
   * @brief Encode OUT (C), C instruction
   *
   * Output from register C to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x49}
   */
  static std::vector<uint8_t> EncodeOUT_C_C();

  /**
   * @brief Encode OUT (C), D instruction
   *
   * Output from register D to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x51}
   */
  static std::vector<uint8_t> EncodeOUT_C_D();

  /**
   * @brief Encode OUT (C), E instruction
   *
   * Output from register E to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x59}
   */
  static std::vector<uint8_t> EncodeOUT_C_E();

  /**
   * @brief Encode OUT (C), H instruction
   *
   * Output from register H to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x61}
   */
  static std::vector<uint8_t> EncodeOUT_C_H();

  /**
   * @brief Encode OUT (C), L instruction
   *
   * Output from register L to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x69}
   */
  static std::vector<uint8_t> EncodeOUT_C_L();

  /**
   * @brief Encode OUT (C), A instruction
   *
   * Output from register A to port C.
   *
   * @return Vector of encoded bytes {0xED, 0x79}
   */
  static std::vector<uint8_t> EncodeOUT_C_A();

  /** @} */ // End of Input/Output Instructions

  /**
   * @name Extended Instructions (ED Prefix)
   * @{
   */

  /**
   * @brief Encode LD I, A instruction (load interrupt vector register)
   * @return Vector of encoded bytes {0xED, 0x47}
   */
  static std::vector<uint8_t> EncodeLD_I_A();

  /**
   * @brief Encode LD A, I instruction (load from interrupt vector)
   * @return Vector of encoded bytes {0xED, 0x57}
   */
  static std::vector<uint8_t> EncodeLD_A_I();

  /**
   * @brief Encode LD R, A instruction (load refresh register)
   * @return Vector of encoded bytes {0xED, 0x4F}
   */
  static std::vector<uint8_t> EncodeLD_R_A();

  /**
   * @brief Encode LD A, R instruction (load from refresh register)
   * @return Vector of encoded bytes {0xED, 0x5F}
   */
  static std::vector<uint8_t> EncodeLD_A_R();

  /**
   * @brief Encode LDIR instruction (load, increment, repeat)
   *
   * Block copy: (DE) <- (HL), HL++, DE++, BC--, repeat until BC=0
   *
   * @return Vector of encoded bytes {0xED, 0xB0}
   */
  static std::vector<uint8_t> EncodeLDIR();

  /**
   * @brief Encode CPIR instruction (compare, increment, repeat)
   *
   * Block search: compare A with (HL), HL++, BC--, repeat until A=(HL) or BC=0
   *
   * @return Vector of encoded bytes {0xED, 0xB1}
   */
  static std::vector<uint8_t> EncodeCPIR();

  /**
   * @brief Encode NEG instruction (negate accumulator - two's complement)
   * @return Vector of encoded bytes {0xED, 0x44}
   */
  static std::vector<uint8_t> EncodeNEG();

  /** @} */ // End of Extended Instructions

  /**
   * @name Additional Stack Operations
   * @{
   */

  /**
   * @brief Encode PUSH DE instruction
   * @return Vector of encoded bytes {0xD5}
   */
  static std::vector<uint8_t> EncodePUSH_DE();

  /**
   * @brief Encode PUSH HL instruction
   * @return Vector of encoded bytes {0xE5}
   */
  static std::vector<uint8_t> EncodePUSH_HL();

  /**
   * @brief Encode PUSH AF instruction
   * @return Vector of encoded bytes {0xF5}
   */
  static std::vector<uint8_t> EncodePUSH_AF();

  /**
   * @brief Encode POP DE instruction
   * @return Vector of encoded bytes {0xD1}
   */
  static std::vector<uint8_t> EncodePOP_DE();

  /**
   * @brief Encode POP HL instruction
   * @return Vector of encoded bytes {0xE1}
   */
  static std::vector<uint8_t> EncodePOP_HL();

  /**
   * @brief Encode POP AF instruction
   * @return Vector of encoded bytes {0xF1}
   */
  static std::vector<uint8_t> EncodePOP_AF();

  /** @} */ // End of Additional Stack Operations

  /**
   * @name Conditional Call and Return Instructions
   * @{
   */

  /**
   * @brief Encode CALL nn instruction (call subroutine)
   *
   * @param address 16-bit target address
   * @return Vector of encoded bytes {0xCD, low_byte, high_byte}
   */
  static std::vector<uint8_t> EncodeCALL_nn(uint16_t address) ;

  /**
   * @brief Encode CALL NZ, nn instruction (call if not zero)
   *
   * @param address 16-bit target address
   * @return Vector of encoded bytes {0xC4, low_byte, high_byte}
   */
  static std::vector<uint8_t> EncodeCALL_NZ_nn(uint16_t address) ;

  /**
   * @brief Encode RET Z instruction (return if zero)
   * @return Vector of encoded bytes {0xC8}
   */
  static std::vector<uint8_t> EncodeRET_Z();

  /**
   * @brief Encode RET NZ instruction (return if not zero)
   * @return Vector of encoded bytes {0xC0}
   */
  static std::vector<uint8_t> EncodeRET_NZ();

  /** @} */ // End of Conditional Call and Return Instructions

private:
  /**
   * @brief Encode 16-bit value in little-endian byte order
   *
   * The Z80 uses little-endian byte order (LSB first), like the 6502
   * and unlike the 6809 (which uses big-endian).
   *
   * @param value 16-bit value to encode
   * @return Vector containing {low_byte, high_byte}
   *
   * @par Example
   * @code
   * ToLittleEndian(0x1234) -> {0x34, 0x12}  // LSB first
   * @endcode
   */
  static std::vector<uint8_t> ToLittleEndian(uint16_t value);
};

} // namespace xasm
