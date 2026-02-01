// 6502/65C02/65816 Opcode Constants
// Replaces magic numbers with descriptive named constants

#pragma once

#include <cstdint>

namespace xasm {
namespace Opcodes {

// ============================================================================
// Load/Store Instructions
// ============================================================================

// LDA - Load Accumulator
constexpr uint8_t LDA_IMM = 0xA9;  // LDA #imm     - Immediate
constexpr uint8_t LDA_ZP  = 0xA5;  // LDA zp       - Zero Page
constexpr uint8_t LDA_ZPX = 0xB5;  // LDA zp,X     - Zero Page,X
constexpr uint8_t LDA_ABS = 0xAD;  // LDA abs      - Absolute
constexpr uint8_t LDA_ABX = 0xBD;  // LDA abs,X    - Absolute,X
constexpr uint8_t LDA_ABY = 0xB9;  // LDA abs,Y    - Absolute,Y
constexpr uint8_t LDA_INX = 0xA1;  // LDA (zp,X)   - Indexed Indirect
constexpr uint8_t LDA_INY = 0xB1;  // LDA (zp),Y   - Indirect Indexed
constexpr uint8_t LDA_IZP = 0xB2;  // LDA (zp)     - Indirect Zero Page (65C02+)
constexpr uint8_t LDA_ALG = 0xAF;  // LDA long     - Absolute Long (65816)
constexpr uint8_t LDA_ILG = 0xA7;  // LDA [dp]     - Indirect Long (65816)
constexpr uint8_t LDA_ILY = 0xB7;  // LDA [dp],Y   - Indirect Long Indexed,Y (65816)
constexpr uint8_t LDA_SR  = 0xA3;  // LDA sr,S     - Stack Relative (65816)
constexpr uint8_t LDA_SRY = 0xB3;  // LDA (sr,S),Y - Stack Relative Indirect Indexed,Y (65816)

// LDX - Load X Register
constexpr uint8_t LDX_IMM = 0xA2;  // LDX #imm     - Immediate
constexpr uint8_t LDX_ZP  = 0xA6;  // LDX zp       - Zero Page
constexpr uint8_t LDX_ZPY = 0xB6;  // LDX zp,Y     - Zero Page,Y
constexpr uint8_t LDX_ABS = 0xAE;  // LDX abs      - Absolute
constexpr uint8_t LDX_ABY = 0xBE;  // LDX abs,Y    - Absolute,Y

// LDY - Load Y Register
constexpr uint8_t LDY_IMM = 0xA0;  // LDY #imm     - Immediate
constexpr uint8_t LDY_ZP  = 0xA4;  // LDY zp       - Zero Page
constexpr uint8_t LDY_ZPX = 0xB4;  // LDY zp,X     - Zero Page,X
constexpr uint8_t LDY_ABS = 0xAC;  // LDY abs      - Absolute
constexpr uint8_t LDY_ABX = 0xBC;  // LDY abs,X    - Absolute,X

// STA - Store Accumulator
constexpr uint8_t STA_ZP  = 0x85;  // STA zp       - Zero Page
constexpr uint8_t STA_ZPX = 0x95;  // STA zp,X     - Zero Page,X
constexpr uint8_t STA_ABS = 0x8D;  // STA abs      - Absolute
constexpr uint8_t STA_ABX = 0x9D;  // STA abs,X    - Absolute,X
constexpr uint8_t STA_ABY = 0x99;  // STA abs,Y    - Absolute,Y
constexpr uint8_t STA_INX = 0x81;  // STA (zp,X)   - Indexed Indirect
constexpr uint8_t STA_INY = 0x91;  // STA (zp),Y   - Indirect Indexed

// STX - Store X Register
constexpr uint8_t STX_ZP  = 0x86;  // STX zp       - Zero Page
constexpr uint8_t STX_ZPY = 0x96;  // STX zp,Y     - Zero Page,Y
constexpr uint8_t STX_ABS = 0x8E;  // STX abs      - Absolute

// STY - Store Y Register
constexpr uint8_t STY_ZP  = 0x84;  // STY zp       - Zero Page
constexpr uint8_t STY_ZPX = 0x94;  // STY zp,X     - Zero Page,X
constexpr uint8_t STY_ABS = 0x8C;  // STY abs      - Absolute

// ============================================================================
// Arithmetic Instructions
// ============================================================================

// ADC - Add with Carry
constexpr uint8_t ADC_IMM = 0x69;  // ADC #imm     - Immediate
constexpr uint8_t ADC_ZP  = 0x65;  // ADC zp       - Zero Page
constexpr uint8_t ADC_ZPX = 0x75;  // ADC zp,X     - Zero Page,X
constexpr uint8_t ADC_ABS = 0x6D;  // ADC abs      - Absolute
constexpr uint8_t ADC_ABX = 0x7D;  // ADC abs,X    - Absolute,X
constexpr uint8_t ADC_ABY = 0x79;  // ADC abs,Y    - Absolute,Y
constexpr uint8_t ADC_INX = 0x61;  // ADC (zp,X)   - Indexed Indirect
constexpr uint8_t ADC_INY = 0x71;  // ADC (zp),Y   - Indirect Indexed

// SBC - Subtract with Carry
constexpr uint8_t SBC_IMM = 0xE9;  // SBC #imm     - Immediate
constexpr uint8_t SBC_ZP  = 0xE5;  // SBC zp       - Zero Page
constexpr uint8_t SBC_ZPX = 0xF5;  // SBC zp,X     - Zero Page,X
constexpr uint8_t SBC_ABS = 0xED;  // SBC abs      - Absolute
constexpr uint8_t SBC_ABX = 0xFD;  // SBC abs,X    - Absolute,X
constexpr uint8_t SBC_ABY = 0xF9;  // SBC abs,Y    - Absolute,Y
constexpr uint8_t SBC_INX = 0xE1;  // SBC (zp,X)   - Indexed Indirect
constexpr uint8_t SBC_INY = 0xF1;  // SBC (zp),Y   - Indirect Indexed

// ============================================================================
// Logic Instructions
// ============================================================================

// AND - Logical AND
constexpr uint8_t AND_IMM = 0x29;  // AND #imm     - Immediate
constexpr uint8_t AND_ZP  = 0x25;  // AND zp       - Zero Page
constexpr uint8_t AND_ZPX = 0x35;  // AND zp,X     - Zero Page,X
constexpr uint8_t AND_ABS = 0x2D;  // AND abs      - Absolute
constexpr uint8_t AND_ABX = 0x3D;  // AND abs,X    - Absolute,X
constexpr uint8_t AND_ABY = 0x39;  // AND abs,Y    - Absolute,Y
constexpr uint8_t AND_INX = 0x21;  // AND (zp,X)   - Indexed Indirect
constexpr uint8_t AND_INY = 0x31;  // AND (zp),Y   - Indirect Indexed

// ORA - Logical OR
constexpr uint8_t ORA_IMM = 0x09;  // ORA #imm     - Immediate
constexpr uint8_t ORA_ZP  = 0x05;  // ORA zp       - Zero Page
constexpr uint8_t ORA_ZPX = 0x15;  // ORA zp,X     - Zero Page,X
constexpr uint8_t ORA_ABS = 0x0D;  // ORA abs      - Absolute
constexpr uint8_t ORA_ABX = 0x1D;  // ORA abs,X    - Absolute,X
constexpr uint8_t ORA_ABY = 0x19;  // ORA abs,Y    - Absolute,Y
constexpr uint8_t ORA_INX = 0x01;  // ORA (zp,X)   - Indexed Indirect
constexpr uint8_t ORA_INY = 0x11;  // ORA (zp),Y   - Indirect Indexed

// EOR - Exclusive OR
constexpr uint8_t EOR_IMM = 0x49;  // EOR #imm     - Immediate
constexpr uint8_t EOR_ZP  = 0x45;  // EOR zp       - Zero Page
constexpr uint8_t EOR_ZPX = 0x55;  // EOR zp,X     - Zero Page,X
constexpr uint8_t EOR_ABS = 0x4D;  // EOR abs      - Absolute
constexpr uint8_t EOR_ABX = 0x5D;  // EOR abs,X    - Absolute,X
constexpr uint8_t EOR_ABY = 0x59;  // EOR abs,Y    - Absolute,Y
constexpr uint8_t EOR_INX = 0x41;  // EOR (zp,X)   - Indexed Indirect
constexpr uint8_t EOR_INY = 0x51;  // EOR (zp),Y   - Indirect Indexed

// ============================================================================
// Shift and Rotate Instructions
// ============================================================================

// ASL - Arithmetic Shift Left
constexpr uint8_t ASL_ACC = 0x0A;  // ASL A        - Accumulator
constexpr uint8_t ASL_ZP  = 0x06;  // ASL zp       - Zero Page
constexpr uint8_t ASL_ZPX = 0x16;  // ASL zp,X     - Zero Page,X
constexpr uint8_t ASL_ABS = 0x0E;  // ASL abs      - Absolute
constexpr uint8_t ASL_ABX = 0x1E;  // ASL abs,X    - Absolute,X

// LSR - Logical Shift Right
constexpr uint8_t LSR_ACC = 0x4A;  // LSR A        - Accumulator
constexpr uint8_t LSR_ZP  = 0x46;  // LSR zp       - Zero Page
constexpr uint8_t LSR_ZPX = 0x56;  // LSR zp,X     - Zero Page,X
constexpr uint8_t LSR_ABS = 0x4E;  // LSR abs      - Absolute
constexpr uint8_t LSR_ABX = 0x5E;  // LSR abs,X    - Absolute,X

// ROL - Rotate Left
constexpr uint8_t ROL_ACC = 0x2A;  // ROL A        - Accumulator
constexpr uint8_t ROL_ZP  = 0x26;  // ROL zp       - Zero Page
constexpr uint8_t ROL_ZPX = 0x36;  // ROL zp,X     - Zero Page,X
constexpr uint8_t ROL_ABS = 0x2E;  // ROL abs      - Absolute
constexpr uint8_t ROL_ABX = 0x3E;  // ROL abs,X    - Absolute,X

// ROR - Rotate Right
constexpr uint8_t ROR_ACC = 0x6A;  // ROR A        - Accumulator
constexpr uint8_t ROR_ZP  = 0x66;  // ROR zp       - Zero Page
constexpr uint8_t ROR_ZPX = 0x76;  // ROR zp,X     - Zero Page,X
constexpr uint8_t ROR_ABS = 0x6E;  // ROR abs      - Absolute
constexpr uint8_t ROR_ABX = 0x7E;  // ROR abs,X    - Absolute,X

// ============================================================================
// Compare Instructions
// ============================================================================

// CMP - Compare Accumulator
constexpr uint8_t CMP_IMM = 0xC9;  // CMP #imm     - Immediate
constexpr uint8_t CMP_ZP  = 0xC5;  // CMP zp       - Zero Page
constexpr uint8_t CMP_ZPX = 0xD5;  // CMP zp,X     - Zero Page,X
constexpr uint8_t CMP_ABS = 0xCD;  // CMP abs      - Absolute
constexpr uint8_t CMP_ABX = 0xDD;  // CMP abs,X    - Absolute,X
constexpr uint8_t CMP_ABY = 0xD9;  // CMP abs,Y    - Absolute,Y
constexpr uint8_t CMP_INX = 0xC1;  // CMP (zp,X)   - Indexed Indirect
constexpr uint8_t CMP_INY = 0xD1;  // CMP (zp),Y   - Indirect Indexed

// CPX - Compare X Register
constexpr uint8_t CPX_IMM = 0xE0;  // CPX #imm     - Immediate
constexpr uint8_t CPX_ZP  = 0xE4;  // CPX zp       - Zero Page
constexpr uint8_t CPX_ABS = 0xEC;  // CPX abs      - Absolute

// CPY - Compare Y Register
constexpr uint8_t CPY_IMM = 0xC0;  // CPY #imm     - Immediate
constexpr uint8_t CPY_ZP  = 0xC4;  // CPY zp       - Zero Page
constexpr uint8_t CPY_ABS = 0xCC;  // CPY abs      - Absolute

// ============================================================================
// Branch Instructions
// ============================================================================

constexpr uint8_t BEQ = 0xF0;  // BEQ rel      - Branch if Equal
constexpr uint8_t BNE = 0xD0;  // BNE rel      - Branch if Not Equal
constexpr uint8_t BCC = 0x90;  // BCC rel      - Branch if Carry Clear
constexpr uint8_t BCS = 0xB0;  // BCS rel      - Branch if Carry Set
constexpr uint8_t BMI = 0x30;  // BMI rel      - Branch if Minus
constexpr uint8_t BPL = 0x10;  // BPL rel      - Branch if Plus
constexpr uint8_t BVC = 0x50;  // BVC rel      - Branch if Overflow Clear
constexpr uint8_t BVS = 0x70;  // BVS rel      - Branch if Overflow Set
constexpr uint8_t BRA = 0x80;  // BRA rel      - Branch Always (65C02)

// ============================================================================
// Jump and Subroutine Instructions
// ============================================================================

constexpr uint8_t JMP_ABS = 0x4C;  // JMP abs      - Jump Absolute
constexpr uint8_t JMP_IND = 0x6C;  // JMP (ind)    - Jump Indirect
constexpr uint8_t JMP_AIX = 0x7C;  // JMP (abs,X)  - Jump Absolute Indexed Indirect (65C02+)

constexpr uint8_t JSR     = 0x20;  // JSR abs      - Jump to Subroutine
constexpr uint8_t RTS     = 0x60;  // RTS          - Return from Subroutine
constexpr uint8_t RTI     = 0x40;  // RTI          - Return from Interrupt

// ============================================================================
// Stack Instructions
// ============================================================================

constexpr uint8_t PHA = 0x48;  // PHA          - Push Accumulator
constexpr uint8_t PLA = 0x68;  // PLA          - Pull Accumulator
constexpr uint8_t PHP = 0x08;  // PHP          - Push Processor Status
constexpr uint8_t PLP = 0x28;  // PLP          - Pull Processor Status
constexpr uint8_t TSX = 0xBA;  // TSX          - Transfer SP to X
constexpr uint8_t TXS = 0x9A;  // TXS          - Transfer X to SP

// ============================================================================
// Increment/Decrement Instructions
// ============================================================================

constexpr uint8_t INX = 0xE8;  // INX          - Increment X
constexpr uint8_t INY = 0xC8;  // INY          - Increment Y
constexpr uint8_t DEX = 0xCA;  // DEX          - Decrement X
constexpr uint8_t DEY = 0x88;  // DEY          - Decrement Y

// INC - Increment Memory
constexpr uint8_t INC_ZP  = 0xE6;  // INC zp       - Zero Page
constexpr uint8_t INC_ZPX = 0xF6;  // INC zp,X     - Zero Page,X
constexpr uint8_t INC_ABS = 0xEE;  // INC abs      - Absolute
constexpr uint8_t INC_ABX = 0xFE;  // INC abs,X    - Absolute,X

// DEC - Decrement Memory
constexpr uint8_t DEC_ZP  = 0xC6;  // DEC zp       - Zero Page
constexpr uint8_t DEC_ZPX = 0xD6;  // DEC zp,X     - Zero Page,X
constexpr uint8_t DEC_ABS = 0xCE;  // DEC abs      - Absolute
constexpr uint8_t DEC_ABX = 0xDE;  // DEC abs,X    - Absolute,X

// ============================================================================
// Flag Operation Instructions
// ============================================================================

constexpr uint8_t CLC = 0x18;  // CLC          - Clear Carry
constexpr uint8_t SEC = 0x38;  // SEC          - Set Carry
constexpr uint8_t CLD = 0xD8;  // CLD          - Clear Decimal
constexpr uint8_t SED = 0xF8;  // SED          - Set Decimal
constexpr uint8_t CLI = 0x58;  // CLI          - Clear Interrupt Disable
constexpr uint8_t SEI = 0x78;  // SEI          - Set Interrupt Disable
constexpr uint8_t CLV = 0xB8;  // CLV          - Clear Overflow

// ============================================================================
// Transfer Instructions
// ============================================================================

constexpr uint8_t TAX = 0xAA;  // TAX          - Transfer A to X
constexpr uint8_t TAY = 0xA8;  // TAY          - Transfer A to Y
constexpr uint8_t TXA = 0x8A;  // TXA          - Transfer X to A
constexpr uint8_t TYA = 0x98;  // TYA          - Transfer Y to A

// ============================================================================
// Special Instructions
// ============================================================================

constexpr uint8_t NOP = 0xEA;  // NOP          - No Operation
constexpr uint8_t BRK = 0x00;  // BRK          - Break

// BIT - Test Bits
constexpr uint8_t BIT_ZP  = 0x24;  // BIT zp       - Zero Page
constexpr uint8_t BIT_ABS = 0x2C;  // BIT abs      - Absolute
constexpr uint8_t BIT_IMM = 0x89;  // BIT #imm     - Immediate (65C02+)
constexpr uint8_t BIT_ZPX = 0x34;  // BIT zp,X     - Zero Page,X (65C02+)
constexpr uint8_t BIT_ABX = 0x3C;  // BIT abs,X    - Absolute,X (65C02+)

// ============================================================================
// Branch Relaxation Helper
// ============================================================================

// XOR mask to get complementary branch opcode for branch relaxation
// All branch opcodes can be inverted by XORing with 0x20
// Example: BEQ (0xF0) XOR 0x20 = BNE (0xD0)
constexpr uint8_t BRANCH_COMPLEMENT_MASK = 0x20;

// Offset used in branch relaxation sequence: B!cc +3; JMP target
constexpr uint8_t BRANCH_RELAXATION_OFFSET = 0x03;

} // namespace Opcodes
} // namespace xasm
