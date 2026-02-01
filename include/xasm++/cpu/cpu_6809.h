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

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace xasm {

/**
 * @brief Addressing modes supported by the Motorola 6809
 * 
 * The 6809 has 13 primary addressing modes with 16 indexed sub-modes,
 * making it one of the most flexible 8-bit CPUs.
 */
enum class AddressingMode6809 {
    Inherent,           ///< No operand (e.g., NOP, CLRA, RTS)
    Immediate8,         ///< 8-bit immediate value (e.g., LDA #$42)
    Immediate16,        ///< 16-bit immediate value (e.g., LDD #$1234)
    Direct,             ///< Direct page address (e.g., LDA <$80)
    Extended,           ///< 16-bit absolute address (e.g., LDA $1234)
    
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
    Relative8,          ///< 8-bit relative (-128 to +127 bytes)
    Relative16,         ///< 16-bit relative (-32768 to +32767 bytes)
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
class Cpu6809 {
public:
    /**
     * @brief Default constructor - initializes to 6809 mode
     */
    Cpu6809() = default;

    /**
     * @brief Get the CPU plugin name
     * @return "6809"
     */
    std::string GetName() const { return "6809"; }

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
    std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode6809 mode) const;
    
    /**
     * @brief Encode LDB (Load Accumulator B) instruction
     * 
     * @param operand Operand value
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     */
    std::vector<uint8_t> EncodeLDB(uint32_t operand, AddressingMode6809 mode) const;
    
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
    std::vector<uint8_t> EncodeLDD(uint32_t operand, AddressingMode6809 mode) const;

    /**
     * @brief Encode STA (Store Accumulator A) instruction
     * 
     * @param operand Operand value (address)
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     * 
     * @note STA does not support Immediate mode
     */
    std::vector<uint8_t> EncodeSTA(uint32_t operand, AddressingMode6809 mode) const;
    
    /**
     * @brief Encode STB (Store Accumulator B) instruction
     * 
     * @param operand Operand value (address)
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     */
    std::vector<uint8_t> EncodeSTB(uint32_t operand, AddressingMode6809 mode) const;
    
    /**
     * @brief Encode STD (Store Accumulator D) instruction
     * 
     * @param operand Operand value (address)
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     * 
     * @note Uses big-endian byte order: stores A (high byte) first
     */
    std::vector<uint8_t> EncodeSTD(uint32_t operand, AddressingMode6809 mode) const;

    /** @} */  // End of Data Movement Instructions

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

    /** @} */  // End of Inherent Instructions

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
                                                 int32_t offset, 
                                                 uint8_t reg) const;

private:
    uint8_t direct_page_ = 0x00;  ///< Direct page register (default $00)
};

} // namespace xasm
