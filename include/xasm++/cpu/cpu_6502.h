/**
 * @file cpu_6502.h
 * @brief 6502 CPU Plugin for instruction encoding
 * 
 * This file defines the CPU plugin for the 6502 family of processors,
 * including the original 6502, 65C02, and 65816. The plugin is responsible
 * for encoding assembly language instructions into machine code.
 * 
 * @note Phase 1: Minimal Viable Assembler - CPU Plugin
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include "xasm++/cpu/cpu_6502_branch_handler.h"

namespace xasm {

/**
 * @brief Addressing modes supported by the 6502 family
 * 
 * Each addressing mode determines how the CPU interprets the operand
 * bytes following an instruction opcode.
 * 
 * @par 6502 Addressing Modes
 * The original 6502 supports 13 addressing modes. The 65C02 adds
 * additional modes, and the 65816 further extends the addressing
 * capabilities with 24-bit addressing and stack-relative modes.
 */
enum class AddressingMode {
    Implied,        ///< No operand (e.g., RTS, NOP)
    Accumulator,    ///< Operate on accumulator (e.g., ASL A)
    Immediate,      ///< Immediate value (e.g., LDA #$42)
    ZeroPage,       ///< Zero page address (e.g., LDA $80)
    ZeroPageX,      ///< Zero page indexed by X (e.g., LDA $80,X)
    ZeroPageY,      ///< Zero page indexed by Y (e.g., LDX $80,Y)
    Absolute,       ///< Absolute 16-bit address (e.g., LDA $1234)
    AbsoluteX,      ///< Absolute indexed by X (e.g., LDA $1234,X)
    AbsoluteY,      ///< Absolute indexed by Y (e.g., LDA $1234,Y)
    Indirect,       ///< Indirect jump (e.g., JMP ($1234))
    IndirectX,      ///< Indirect indexed by X (e.g., LDA ($80,X))
    IndirectY,      ///< Indirect indexed by Y (e.g., LDA ($80),Y)
    Relative,       ///< Relative branch (e.g., BEQ label)

    // 65C02 Enhanced Addressing Modes
    IndirectZeroPage,         ///< Indirect zero page - 65C02+ (e.g., LDA ($80))
    AbsoluteIndexedIndirect,  ///< Absolute indexed indirect - 65C02+ (e.g., JMP ($1234,X))

    // 65816 Long Addressing Modes (24-bit)
    AbsoluteLong,             ///< 24-bit absolute - 65816 only (e.g., LDA $123456)
    IndirectLong,             ///< 24-bit indirect - 65816 only (e.g., LDA [$80])
    IndirectLongIndexedY,     ///< 24-bit indirect indexed - 65816 only (e.g., LDA [$80],Y)

    // 65816 Stack Relative Addressing
    StackRelative,                    ///< Stack relative - 65816 only (e.g., LDA $03,S)
    StackRelativeIndirectIndexedY,    ///< Stack relative indirect indexed - 65816 only
};

/**
 * @brief CPU modes for the 6502 family
 * 
 * Different CPU modes enable different instruction sets and addressing modes.
 * The assembler needs to know which CPU mode is active to validate instructions
 * and generate correct opcodes.
 */
enum class CpuMode {
    Cpu6502,        ///< Original 6502 (default)
    Cpu65C02,       ///< 65C02 with enhanced opcodes
    Cpu65C02Rock,   ///< 65C02 with Rockwell extensions (BBR, BBS, etc.)
    Cpu65816        ///< 65816 with 16-bit support and enhanced addressing
};

/**
 * @brief 6502 CPU Plugin - encodes instructions for the 6502 family
 * 
 * This class provides instruction encoding services for the 6502, 65C02,
 * and 65816 processors. It converts assembly language mnemonics and operands
 * into machine code bytes.
 * 
 * @par Supported Processors
 * - **6502**: Original MOS Technology 6502 (56 opcodes)
 * - **65C02**: WDC 65C02 with additional opcodes and addressing modes
 * - **65C02 Rockwell**: 65C02 with Rockwell bit manipulation extensions
 * - **65816**: WDC 65816 with 16-bit operations and 24-bit addressing
 * 
 * @par Branch Relaxation
 * The plugin automatically handles branch relaxation, converting out-of-range
 * short branches into complementary branch + JMP sequences:
 * @code
 * BEQ distant_label   ; If out of range (-128 to +127 bytes)
 * ; Becomes:
 * BNE *+5             ; Skip over JMP if condition not met
 * JMP distant_label   ; Unconditional jump to target
 * @endcode
 * 
 * @par Usage Example
 * @code
 * Cpu6502 cpu;
 * cpu.SetCpuMode(CpuMode::Cpu65C02);  // Enable 65C02 instructions
 * 
 * // Encode "LDA #$42"
 * auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
 * // Returns: {0xA9, 0x42}
 * @endcode
 */
class Cpu6502 {
public:
    /**
     * @brief Default constructor - initializes to 6502 mode
     */
    Cpu6502() = default;

    /**
     * @brief Get the CPU plugin name
     * @return "6502"
     */
    std::string GetName() const { return "6502"; }

    /**
     * @brief Set the CPU mode
     * 
     * Changes the active CPU mode, which affects which instructions and
     * addressing modes are available.
     * 
     * @param mode Target CPU mode
     */
    void SetCpuMode(CpuMode mode);
    
    /**
     * @brief Get the current CPU mode
     * @return Current CPU mode
     */
    CpuMode GetCpuMode() const;

    /**
     * @brief Set the M and X flags for 65816 register width control
     * 
     * On the 65816, the M flag controls accumulator width and the X flag
     * controls index register width. This affects instruction encoding sizes.
     * 
     * @param m_flag true = 8-bit accumulator, false = 16-bit accumulator
     * @param x_flag true = 8-bit index registers, false = 16-bit index registers
     * 
     * @note Only relevant for CpuMode::Cpu65816
     */
    void SetMX(bool m_flag, bool x_flag);
    
    /**
     * @brief Check if accumulator is 8-bit
     * @return true if accumulator is 8-bit (m_flag = 1)
     */
    bool IsAccumulator8Bit() const;
    
    /**
     * @brief Check if index registers are 8-bit
     * @return true if index registers are 8-bit (x_flag = 1)
     */
    bool IsIndex8Bit() const;

    /**
     * @name Instruction Encoding Methods
     * 
     * These methods encode individual 6502 family instructions into machine code.
     * Each method takes an operand value and addressing mode, and returns a
     * vector of bytes representing the encoded instruction.
     * 
     * @par Return Value
     * Returns a vector of bytes where:
     * - First byte is the opcode
     * - Following bytes are operand bytes (if any)
     * - Length varies by addressing mode (1-4 bytes)
     * 
     * @par Invalid Combinations
     * If an invalid instruction/addressing mode combination is specified,
     * the method returns an empty vector (this will be improved to throw
     * exceptions in future versions).
     * 
     * @{
     */

    /**
     * @brief Encode LDA (Load Accumulator) instruction
     * 
     * @param operand Operand value (address or immediate value)
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     * 
     * @par Supported Modes
     * Immediate, ZeroPage, ZeroPageX, Absolute, AbsoluteX, AbsoluteY,
     * IndirectX, IndirectY, IndirectZeroPage (65C02+), AbsoluteLong (65816)
     */
    std::vector<uint8_t> EncodeLDA(uint32_t operand, AddressingMode mode) const;
    
    /**
     * @brief Encode STA (Store Accumulator) instruction
     * 
     * @param operand Operand value (address)
     * @param mode Addressing mode
     * @return Vector of encoded bytes
     * 
     * @note STA does not support Immediate mode (cannot store to a constant)
     */
    std::vector<uint8_t> EncodeSTA(uint16_t operand, AddressingMode mode) const;
    
    /**
     * @brief Encode JMP (Jump) instruction
     * 
     * @param operand Target address
     * @param mode Addressing mode (Absolute or Indirect)
     * @return Vector of encoded bytes
     */
    std::vector<uint8_t> EncodeJMP(uint16_t operand, AddressingMode mode) const;
    
    /**
     * @brief Encode NOP (No Operation) instruction
     * @return Vector containing single NOP opcode byte {0xEA}
     */
    std::vector<uint8_t> EncodeNOP() const;
    
    /**
     * @brief Encode RTS (Return from Subroutine) instruction
     * @return Vector containing single RTS opcode byte {0x60}
     */
    std::vector<uint8_t> EncodeRTS() const;

    // Phase 2.2: Arithmetic instructions
    std::vector<uint8_t> EncodeADC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSBC(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Logic instructions
    std::vector<uint8_t> EncodeAND(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeORA(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeEOR(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Additional loads/stores
    std::vector<uint8_t> EncodeLDX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeLDY(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSTX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeSTY(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Comparisons
    std::vector<uint8_t> EncodeCMP(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeCPX(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeCPY(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Branch instructions
    std::vector<uint8_t> EncodeBEQ(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBNE(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBCC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBCS(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBMI(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBPL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBVC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeBVS(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Inc/Dec instructions
    std::vector<uint8_t> EncodeINX() const;
    std::vector<uint8_t> EncodeINY() const;
    std::vector<uint8_t> EncodeDEX() const;
    std::vector<uint8_t> EncodeDEY() const;
    std::vector<uint8_t> EncodeINC(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeDEC(uint16_t operand, AddressingMode mode) const;

    // Phase 2.2: Stack operations
    std::vector<uint8_t> EncodePHA() const;
    std::vector<uint8_t> EncodePLA() const;
    std::vector<uint8_t> EncodePHP() const;
    std::vector<uint8_t> EncodePLP() const;

    // Phase 2.5 - Group 2: 65C02 Stack operations
    std::vector<uint8_t> EncodePHX() const;  // Push X (65C02+)
    std::vector<uint8_t> EncodePLX() const;  // Pull X (65C02+)
    std::vector<uint8_t> EncodePHY() const;  // Push Y (65C02+)
    std::vector<uint8_t> EncodePLY() const;  // Pull Y (65C02+)

    // Phase 2.5 - Group 3: 65C02 Store Zero
    std::vector<uint8_t> EncodeSTZ(uint16_t operand, AddressingMode mode) const;  // Store Zero (65C02+)

    // Phase 2.5 - Group 4: 65C02 Bit Test
    std::vector<uint8_t> EncodeTRB(uint16_t operand, AddressingMode mode) const;  // Test and Reset Bits (65C02+)
    std::vector<uint8_t> EncodeTSB(uint16_t operand, AddressingMode mode) const;  // Test and Set Bits (65C02+)

    // Phase 2.5 - Group 5: 65C02 Branch Always
    std::vector<uint8_t> EncodeBRA(uint16_t operand, AddressingMode mode) const;  // Branch Always (65C02+)

    // Phase 2.5 - Group 11: 65816 Bank Operations
    std::vector<uint8_t> EncodePHB() const;  // Push Data Bank Register (65816)
    std::vector<uint8_t> EncodePLB() const;  // Pull Data Bank Register (65816)
    std::vector<uint8_t> EncodePHK() const;  // Push Program Bank Register (65816)
    std::vector<uint8_t> EncodePHD() const;  // Push Direct Page Register (65816)
    std::vector<uint8_t> EncodePLD() const;  // Pull Direct Page Register (65816)

    // Phase 2.5 - Group 12: 65816 Transfer Operations
    std::vector<uint8_t> EncodeTCD() const;  // Transfer C to Direct Page (65816)
    std::vector<uint8_t> EncodeTDC() const;  // Transfer Direct Page to C (65816)
    std::vector<uint8_t> EncodeTCS() const;  // Transfer C to Stack Pointer (65816)
    std::vector<uint8_t> EncodeTSC() const;  // Transfer Stack Pointer to C (65816)

    // Phase 2.5 - Group 13: 65816 Long Jumps
    std::vector<uint8_t> EncodeJML(uint32_t operand, AddressingMode mode) const;  // Jump Long (65816)
    std::vector<uint8_t> EncodeJSL(uint32_t operand, AddressingMode mode) const;  // Jump Subroutine Long (65816)
    std::vector<uint8_t> EncodeRTL() const;  // Return from Subroutine Long (65816)

    // Phase 2.5 - Group 14: 65816 Miscellaneous Opcodes
    std::vector<uint8_t> EncodePEA(uint16_t operand, AddressingMode mode) const;  // Push Effective Address (65816)
    std::vector<uint8_t> EncodePEI(uint8_t operand, AddressingMode mode) const;   // Push Effective Indirect (65816)
    std::vector<uint8_t> EncodePER(uint16_t operand, AddressingMode mode) const;  // Push Effective PC Relative (65816)
    std::vector<uint8_t> EncodeMVN(uint8_t srcbank, uint8_t destbank) const;      // Block Move Negative (65816)
    std::vector<uint8_t> EncodeMVP(uint8_t srcbank, uint8_t destbank) const;      // Block Move Positive (65816)
    std::vector<uint8_t> EncodeCOP(uint8_t operand, AddressingMode mode) const;   // Coprocessor (65816)
    std::vector<uint8_t> EncodeWDM(uint8_t operand, AddressingMode mode) const;   // Reserved (65816)
    std::vector<uint8_t> EncodeXBA() const;  // Exchange B and A (65816)
    std::vector<uint8_t> EncodeXCE() const;  // Exchange Carry and Emulation (65816)

    // Phase 2.2: Subroutine
    std::vector<uint8_t> EncodeJSR(uint16_t operand, AddressingMode mode) const;

    // Phase 2.3: Complete 6502 Instruction Set
    // Group 1: BIT - Test Bits
    std::vector<uint8_t> EncodeBIT(uint16_t operand, AddressingMode mode) const;

    // Group 2: Shift Instructions
    std::vector<uint8_t> EncodeASL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeLSR(uint16_t operand, AddressingMode mode) const;

    // Group 3: Rotate Instructions
    std::vector<uint8_t> EncodeROL(uint16_t operand, AddressingMode mode) const;
    std::vector<uint8_t> EncodeROR(uint16_t operand, AddressingMode mode) const;

    // Group 4: Interrupt Instructions
    std::vector<uint8_t> EncodeRTI() const;
    std::vector<uint8_t> EncodeBRK() const;

    // Group 5: Flag Operations
    std::vector<uint8_t> EncodeCLC() const;
    std::vector<uint8_t> EncodeSEC() const;
    std::vector<uint8_t> EncodeCLD() const;
    std::vector<uint8_t> EncodeSED() const;
    std::vector<uint8_t> EncodeCLI() const;
    std::vector<uint8_t> EncodeSEI() const;
    std::vector<uint8_t> EncodeCLV() const;

    // Group 6: Transfer Instructions
    std::vector<uint8_t> EncodeTSX() const;
    std::vector<uint8_t> EncodeTXS() const;
    std::vector<uint8_t> EncodeTAX() const;
    std::vector<uint8_t> EncodeTAY() const;
    std::vector<uint8_t> EncodeTXA() const;
    std::vector<uint8_t> EncodeTYA() const;

    /** @} */  // End of Instruction Encoding Methods

    /**
     * @brief Calculate the size of an encoded instruction
     * 
     * Returns the number of bytes required to encode an instruction
     * with the given addressing mode.
     * 
     * @param mode Addressing mode
     * @return Instruction size in bytes (1-4 bytes)
     * 
     * @par Size by Mode
     * - Implied/Accumulator: 1 byte
     * - Immediate/ZeroPage/Relative: 2 bytes
     * - Absolute: 3 bytes
     * - AbsoluteLong: 4 bytes
     */
    size_t CalculateInstructionSize(AddressingMode mode) const;

    /**
     * @name Branch Relaxation Methods
     * 
     * These methods support automatic branch relaxation, which converts
     * out-of-range short branches into longer sequences that can reach
     * distant targets.
     * 
     * @{
     */

    /**
     * @brief Check if a branch needs relaxation
     * 
     * Determines if the target address is out of range for a short
     * relative branch (-128 to +127 bytes from the branch instruction).
     * 
     * @param current_addr Address of the branch instruction
     * @param target_addr Target address to branch to
     * @return true if branch is out of range and needs relaxation
     */
    bool NeedsBranchRelaxation(uint16_t current_addr, uint16_t target_addr) const;
    
    /**
     * @brief Get the complementary branch opcode
     * 
     * Returns the opposite branch condition opcode. Used for branch
     * relaxation sequences.
     * 
     * @param branch_opcode Original branch opcode (e.g., BEQ = 0xF0)
     * @return Complementary opcode (e.g., BNE = 0xD0)
     * 
     * @par Examples
     * - BEQ -> BNE
     * - BCS -> BCC
     * - BMI -> BPL
     */
    uint8_t GetComplementaryBranchOpcode(uint8_t branch_opcode) const;
    
    /**
     * @brief Encode a branch with relaxation
     * 
     * Encodes an out-of-range branch as a relaxed sequence:
     * @code
     * BEQ target    ; If target is out of range
     * ; Becomes:
     * BNE *+5       ; Skip over JMP if condition not met
     * JMP target    ; Unconditional jump to target
     * @endcode
     * 
     * @param branch_opcode Original branch opcode
     * @param current_addr Address of the branch instruction
     * @param target_addr Target address to branch to
     * @return Vector of encoded bytes for the relaxed sequence (5 bytes)
     */
    std::vector<uint8_t> EncodeBranchWithRelaxation(uint8_t branch_opcode, 
                                                      uint16_t current_addr, 
                                                      uint16_t target_addr) const;

    /** @} */  // End of Branch Relaxation Methods

private:
    // Opcode table structure for reducing duplication
    // Maps addressing modes to their corresponding opcodes for an instruction
    struct OpcodeTable {
        std::optional<uint8_t> immediate;
        std::optional<uint8_t> zero_page;
        std::optional<uint8_t> zero_page_x;
        std::optional<uint8_t> zero_page_y;
        std::optional<uint8_t> absolute;
        std::optional<uint8_t> absolute_x;
        std::optional<uint8_t> absolute_y;
        std::optional<uint8_t> indirect;
        std::optional<uint8_t> indirect_x;
        std::optional<uint8_t> indirect_y;
        std::optional<uint8_t> accumulator;
        std::optional<uint8_t> relative;
        std::optional<uint8_t> indirect_zero_page;           // 65C02+
        std::optional<uint8_t> absolute_indexed_indirect;    // 65C02+
        std::optional<uint8_t> absolute_long;                // 65816
        std::optional<uint8_t> indirect_long;                // 65816
        std::optional<uint8_t> indirect_long_indexed_y;      // 65816
        std::optional<uint8_t> stack_relative;               // 65816
        std::optional<uint8_t> stack_relative_indirect_indexed_y;  // 65816
    };

    // Generic encoding function using opcode table
    // Eliminates duplication across 50+ Encode methods
    std::vector<uint8_t> EncodeWithTable(const OpcodeTable& table, 
                                           uint32_t operand, 
                                           AddressingMode mode) const;

private:
    // Phase 2.5: CPU mode state
    CpuMode cpu_mode_ = CpuMode::Cpu6502;  // Default to 6502

    // Phase 2.5 - Group 8: 65816 MX state (register width control)
    // m flag: false = 16-bit accumulator, true = 8-bit accumulator
    // x flag: false = 16-bit index registers, true = 8-bit index registers
    bool m_flag_ = true;   // Default: 8-bit accumulator (emulation mode)
    bool x_flag_ = true;   // Default: 8-bit index registers (emulation mode)

    // Branch relaxation handler (Large class refactoring - improving SRP compliance M6.1)
    Cpu6502BranchHandler branch_handler_;
};

} // namespace xasm
