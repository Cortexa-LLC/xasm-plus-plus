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
constexpr uint8_t LDA_IMM = 0xA9; // LDA #imm     - Immediate
constexpr uint8_t LDA_ZP = 0xA5;  // LDA zp       - Zero Page
constexpr uint8_t LDA_ZPX = 0xB5; // LDA zp,X     - Zero Page,X
constexpr uint8_t LDA_ABS = 0xAD; // LDA abs      - Absolute
constexpr uint8_t LDA_ABX = 0xBD; // LDA abs,X    - Absolute,X
constexpr uint8_t LDA_ABY = 0xB9; // LDA abs,Y    - Absolute,Y
constexpr uint8_t LDA_INX = 0xA1; // LDA (zp,X)   - Indexed Indirect
constexpr uint8_t LDA_INY = 0xB1; // LDA (zp),Y   - Indirect Indexed
constexpr uint8_t LDA_IZP = 0xB2; // LDA (zp)     - Indirect Zero Page (65C02+)
constexpr uint8_t LDA_ALG = 0xAF; // LDA long     - Absolute Long (65816)
constexpr uint8_t LDA_ILG = 0xA7; // LDA [dp]     - Indirect Long (65816)
constexpr uint8_t LDA_ILY =
    0xB7; // LDA [dp],Y   - Indirect Long Indexed,Y (65816)
constexpr uint8_t LDA_SR = 0xA3; // LDA sr,S     - Stack Relative (65816)
constexpr uint8_t LDA_SRY =
    0xB3; // LDA (sr,S),Y - Stack Relative Indirect Indexed,Y (65816)

// LDX - Load X Register
constexpr uint8_t LDX_IMM = 0xA2; // LDX #imm     - Immediate
constexpr uint8_t LDX_ZP = 0xA6;  // LDX zp       - Zero Page
constexpr uint8_t LDX_ZPY = 0xB6; // LDX zp,Y     - Zero Page,Y
constexpr uint8_t LDX_ABS = 0xAE; // LDX abs      - Absolute
constexpr uint8_t LDX_ABY = 0xBE; // LDX abs,Y    - Absolute,Y

// LDY - Load Y Register
constexpr uint8_t LDY_IMM = 0xA0; // LDY #imm     - Immediate
constexpr uint8_t LDY_ZP = 0xA4;  // LDY zp       - Zero Page
constexpr uint8_t LDY_ZPX = 0xB4; // LDY zp,X     - Zero Page,X
constexpr uint8_t LDY_ABS = 0xAC; // LDY abs      - Absolute
constexpr uint8_t LDY_ABX = 0xBC; // LDY abs,X    - Absolute,X

// STA - Store Accumulator
constexpr uint8_t STA_ZP = 0x85;  // STA zp       - Zero Page
constexpr uint8_t STA_ZPX = 0x95; // STA zp,X     - Zero Page,X
constexpr uint8_t STA_ABS = 0x8D; // STA abs      - Absolute
constexpr uint8_t STA_ABX = 0x9D; // STA abs,X    - Absolute,X
constexpr uint8_t STA_ABY = 0x99; // STA abs,Y    - Absolute,Y
constexpr uint8_t STA_INX = 0x81; // STA (zp,X)   - Indexed Indirect
constexpr uint8_t STA_INY = 0x91; // STA (zp),Y   - Indirect Indexed

// STX - Store X Register
constexpr uint8_t STX_ZP = 0x86;  // STX zp       - Zero Page
constexpr uint8_t STX_ZPY = 0x96; // STX zp,Y     - Zero Page,Y
constexpr uint8_t STX_ABS = 0x8E; // STX abs      - Absolute

// STY - Store Y Register
constexpr uint8_t STY_ZP = 0x84;  // STY zp       - Zero Page
constexpr uint8_t STY_ZPX = 0x94; // STY zp,X     - Zero Page,X
constexpr uint8_t STY_ABS = 0x8C; // STY abs      - Absolute

// ============================================================================
// Arithmetic Instructions
// ============================================================================

// ADC - Add with Carry
constexpr uint8_t ADC_IMM = 0x69; // ADC #imm     - Immediate
constexpr uint8_t ADC_ZP = 0x65;  // ADC zp       - Zero Page
constexpr uint8_t ADC_ZPX = 0x75; // ADC zp,X     - Zero Page,X
constexpr uint8_t ADC_ABS = 0x6D; // ADC abs      - Absolute
constexpr uint8_t ADC_ABX = 0x7D; // ADC abs,X    - Absolute,X
constexpr uint8_t ADC_ABY = 0x79; // ADC abs,Y    - Absolute,Y
constexpr uint8_t ADC_INX = 0x61; // ADC (zp,X)   - Indexed Indirect
constexpr uint8_t ADC_INY = 0x71; // ADC (zp),Y   - Indirect Indexed

// SBC - Subtract with Carry
constexpr uint8_t SBC_IMM = 0xE9; // SBC #imm     - Immediate
constexpr uint8_t SBC_ZP = 0xE5;  // SBC zp       - Zero Page
constexpr uint8_t SBC_ZPX = 0xF5; // SBC zp,X     - Zero Page,X
constexpr uint8_t SBC_ABS = 0xED; // SBC abs      - Absolute
constexpr uint8_t SBC_ABX = 0xFD; // SBC abs,X    - Absolute,X
constexpr uint8_t SBC_ABY = 0xF9; // SBC abs,Y    - Absolute,Y
constexpr uint8_t SBC_INX = 0xE1; // SBC (zp,X)   - Indexed Indirect
constexpr uint8_t SBC_INY = 0xF1; // SBC (zp),Y   - Indirect Indexed

// ============================================================================
// Logic Instructions
// ============================================================================

// AND - Logical AND
constexpr uint8_t AND_IMM = 0x29; // AND #imm     - Immediate
constexpr uint8_t AND_ZP = 0x25;  // AND zp       - Zero Page
constexpr uint8_t AND_ZPX = 0x35; // AND zp,X     - Zero Page,X
constexpr uint8_t AND_ABS = 0x2D; // AND abs      - Absolute
constexpr uint8_t AND_ABX = 0x3D; // AND abs,X    - Absolute,X
constexpr uint8_t AND_ABY = 0x39; // AND abs,Y    - Absolute,Y
constexpr uint8_t AND_INX = 0x21; // AND (zp,X)   - Indexed Indirect
constexpr uint8_t AND_INY = 0x31; // AND (zp),Y   - Indirect Indexed

// ORA - Logical OR
constexpr uint8_t ORA_IMM = 0x09; // ORA #imm     - Immediate
constexpr uint8_t ORA_ZP = 0x05;  // ORA zp       - Zero Page
constexpr uint8_t ORA_ZPX = 0x15; // ORA zp,X     - Zero Page,X
constexpr uint8_t ORA_ABS = 0x0D; // ORA abs      - Absolute
constexpr uint8_t ORA_ABX = 0x1D; // ORA abs,X    - Absolute,X
constexpr uint8_t ORA_ABY = 0x19; // ORA abs,Y    - Absolute,Y
constexpr uint8_t ORA_INX = 0x01; // ORA (zp,X)   - Indexed Indirect
constexpr uint8_t ORA_INY = 0x11; // ORA (zp),Y   - Indirect Indexed

// EOR - Exclusive OR
constexpr uint8_t EOR_IMM = 0x49; // EOR #imm     - Immediate
constexpr uint8_t EOR_ZP = 0x45;  // EOR zp       - Zero Page
constexpr uint8_t EOR_ZPX = 0x55; // EOR zp,X     - Zero Page,X
constexpr uint8_t EOR_ABS = 0x4D; // EOR abs      - Absolute
constexpr uint8_t EOR_ABX = 0x5D; // EOR abs,X    - Absolute,X
constexpr uint8_t EOR_ABY = 0x59; // EOR abs,Y    - Absolute,Y
constexpr uint8_t EOR_INX = 0x41; // EOR (zp,X)   - Indexed Indirect
constexpr uint8_t EOR_INY = 0x51; // EOR (zp),Y   - Indirect Indexed

// ============================================================================
// Shift and Rotate Instructions
// ============================================================================

// ASL - Arithmetic Shift Left
constexpr uint8_t ASL_ACC = 0x0A; // ASL A        - Accumulator
constexpr uint8_t ASL_ZP = 0x06;  // ASL zp       - Zero Page
constexpr uint8_t ASL_ZPX = 0x16; // ASL zp,X     - Zero Page,X
constexpr uint8_t ASL_ABS = 0x0E; // ASL abs      - Absolute
constexpr uint8_t ASL_ABX = 0x1E; // ASL abs,X    - Absolute,X

// LSR - Logical Shift Right
constexpr uint8_t LSR_ACC = 0x4A; // LSR A        - Accumulator
constexpr uint8_t LSR_ZP = 0x46;  // LSR zp       - Zero Page
constexpr uint8_t LSR_ZPX = 0x56; // LSR zp,X     - Zero Page,X
constexpr uint8_t LSR_ABS = 0x4E; // LSR abs      - Absolute
constexpr uint8_t LSR_ABX = 0x5E; // LSR abs,X    - Absolute,X

// ROL - Rotate Left
constexpr uint8_t ROL_ACC = 0x2A; // ROL A        - Accumulator
constexpr uint8_t ROL_ZP = 0x26;  // ROL zp       - Zero Page
constexpr uint8_t ROL_ZPX = 0x36; // ROL zp,X     - Zero Page,X
constexpr uint8_t ROL_ABS = 0x2E; // ROL abs      - Absolute
constexpr uint8_t ROL_ABX = 0x3E; // ROL abs,X    - Absolute,X

// ROR - Rotate Right
constexpr uint8_t ROR_ACC = 0x6A; // ROR A        - Accumulator
constexpr uint8_t ROR_ZP = 0x66;  // ROR zp       - Zero Page
constexpr uint8_t ROR_ZPX = 0x76; // ROR zp,X     - Zero Page,X
constexpr uint8_t ROR_ABS = 0x6E; // ROR abs      - Absolute
constexpr uint8_t ROR_ABX = 0x7E; // ROR abs,X    - Absolute,X

// ============================================================================
// Compare Instructions
// ============================================================================

// CMP - Compare Accumulator
constexpr uint8_t CMP_IMM = 0xC9; // CMP #imm     - Immediate
constexpr uint8_t CMP_ZP = 0xC5;  // CMP zp       - Zero Page
constexpr uint8_t CMP_ZPX = 0xD5; // CMP zp,X     - Zero Page,X
constexpr uint8_t CMP_ABS = 0xCD; // CMP abs      - Absolute
constexpr uint8_t CMP_ABX = 0xDD; // CMP abs,X    - Absolute,X
constexpr uint8_t CMP_ABY = 0xD9; // CMP abs,Y    - Absolute,Y
constexpr uint8_t CMP_INX = 0xC1; // CMP (zp,X)   - Indexed Indirect
constexpr uint8_t CMP_INY = 0xD1; // CMP (zp),Y   - Indirect Indexed

// CPX - Compare X Register
constexpr uint8_t CPX_IMM = 0xE0; // CPX #imm     - Immediate
constexpr uint8_t CPX_ZP = 0xE4;  // CPX zp       - Zero Page
constexpr uint8_t CPX_ABS = 0xEC; // CPX abs      - Absolute

// CPY - Compare Y Register
constexpr uint8_t CPY_IMM = 0xC0; // CPY #imm     - Immediate
constexpr uint8_t CPY_ZP = 0xC4;  // CPY zp       - Zero Page
constexpr uint8_t CPY_ABS = 0xCC; // CPY abs      - Absolute

// ============================================================================
// Branch Instructions
// ============================================================================

constexpr uint8_t BEQ = 0xF0; // BEQ rel      - Branch if Equal
constexpr uint8_t BNE = 0xD0; // BNE rel      - Branch if Not Equal
constexpr uint8_t BCC = 0x90; // BCC rel      - Branch if Carry Clear
constexpr uint8_t BCS = 0xB0; // BCS rel      - Branch if Carry Set
constexpr uint8_t BMI = 0x30; // BMI rel      - Branch if Minus
constexpr uint8_t BPL = 0x10; // BPL rel      - Branch if Plus
constexpr uint8_t BVC = 0x50; // BVC rel      - Branch if Overflow Clear
constexpr uint8_t BVS = 0x70; // BVS rel      - Branch if Overflow Set
constexpr uint8_t BRA = 0x80; // BRA rel      - Branch Always (65C02)

// ============================================================================
// Jump and Subroutine Instructions
// ============================================================================

constexpr uint8_t JMP_ABS = 0x4C; // JMP abs      - Jump Absolute
constexpr uint8_t JMP_IND = 0x6C; // JMP (ind)    - Jump Indirect
constexpr uint8_t JMP_AIX =
    0x7C; // JMP (abs,X)  - Jump Absolute Indexed Indirect (65C02+)

constexpr uint8_t JSR = 0x20; // JSR abs      - Jump to Subroutine
constexpr uint8_t RTS = 0x60; // RTS          - Return from Subroutine
constexpr uint8_t RTI = 0x40; // RTI          - Return from Interrupt

// ============================================================================
// Stack Instructions
// ============================================================================

constexpr uint8_t PHA = 0x48; // PHA          - Push Accumulator
constexpr uint8_t PLA = 0x68; // PLA          - Pull Accumulator
constexpr uint8_t PHP = 0x08; // PHP          - Push Processor Status
constexpr uint8_t PLP = 0x28; // PLP          - Pull Processor Status
constexpr uint8_t TSX = 0xBA; // TSX          - Transfer SP to X
constexpr uint8_t TXS = 0x9A; // TXS          - Transfer X to SP

// ============================================================================
// Increment/Decrement Instructions
// ============================================================================

constexpr uint8_t INX = 0xE8; // INX          - Increment X
constexpr uint8_t INY = 0xC8; // INY          - Increment Y
constexpr uint8_t DEX = 0xCA; // DEX          - Decrement X
constexpr uint8_t DEY = 0x88; // DEY          - Decrement Y

// INC - Increment Memory
constexpr uint8_t INC_ZP = 0xE6;  // INC zp       - Zero Page
constexpr uint8_t INC_ZPX = 0xF6; // INC zp,X     - Zero Page,X
constexpr uint8_t INC_ABS = 0xEE; // INC abs      - Absolute
constexpr uint8_t INC_ABX = 0xFE; // INC abs,X    - Absolute,X

// DEC - Decrement Memory
constexpr uint8_t DEC_ZP = 0xC6;  // DEC zp       - Zero Page
constexpr uint8_t DEC_ZPX = 0xD6; // DEC zp,X     - Zero Page,X
constexpr uint8_t DEC_ABS = 0xCE; // DEC abs      - Absolute
constexpr uint8_t DEC_ABX = 0xDE; // DEC abs,X    - Absolute,X

// ============================================================================
// Flag Operation Instructions
// ============================================================================

constexpr uint8_t CLC = 0x18; // CLC          - Clear Carry
constexpr uint8_t SEC = 0x38; // SEC          - Set Carry
constexpr uint8_t CLD = 0xD8; // CLD          - Clear Decimal
constexpr uint8_t SED = 0xF8; // SED          - Set Decimal
constexpr uint8_t CLI = 0x58; // CLI          - Clear Interrupt Disable
constexpr uint8_t SEI = 0x78; // SEI          - Set Interrupt Disable
constexpr uint8_t CLV = 0xB8; // CLV          - Clear Overflow

// ============================================================================
// Transfer Instructions
// ============================================================================

constexpr uint8_t TAX = 0xAA; // TAX          - Transfer A to X
constexpr uint8_t TAY = 0xA8; // TAY          - Transfer A to Y
constexpr uint8_t TXA = 0x8A; // TXA          - Transfer X to A
constexpr uint8_t TYA = 0x98; // TYA          - Transfer Y to A

// ============================================================================
// Special Instructions
// ============================================================================

constexpr uint8_t NOP = 0xEA; // NOP          - No Operation
constexpr uint8_t BRK = 0x00; // BRK          - Break

// BIT - Test Bits
constexpr uint8_t BIT_ZP = 0x24;  // BIT zp       - Zero Page
constexpr uint8_t BIT_ABS = 0x2C; // BIT abs      - Absolute
constexpr uint8_t BIT_IMM = 0x89; // BIT #imm     - Immediate (65C02+)
constexpr uint8_t BIT_ZPX = 0x34; // BIT zp,X     - Zero Page,X (65C02+)
constexpr uint8_t BIT_ABX = 0x3C; // BIT abs,X    - Absolute,X (65C02+)

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

// ============================================================================
// Mnemonic String Constants
// ============================================================================

namespace M6502Mnemonics {

// Load/Store Instructions
constexpr const char *LDA = "LDA";
constexpr const char *LDX = "LDX";
constexpr const char *LDY = "LDY";
constexpr const char *STA = "STA";
constexpr const char *STX = "STX";
constexpr const char *STY = "STY";
constexpr const char *STZ = "STZ"; // 65C02+

// Arithmetic Instructions
constexpr const char *ADC = "ADC";
constexpr const char *SBC = "SBC";
constexpr const char *INC = "INC";
constexpr const char *DEC = "DEC";
constexpr const char *INX = "INX";
constexpr const char *INY = "INY";
constexpr const char *DEX = "DEX";
constexpr const char *DEY = "DEY";

// Logical Instructions
constexpr const char *AND = "AND";
constexpr const char *ORA = "ORA";
constexpr const char *EOR = "EOR";
constexpr const char *BIT = "BIT";

// Compare Instructions
constexpr const char *CMP = "CMP";
constexpr const char *CPX = "CPX";
constexpr const char *CPY = "CPY";

// Branch Instructions
constexpr const char *BEQ = "BEQ";
constexpr const char *BNE = "BNE";
constexpr const char *BCS = "BCS";
constexpr const char *BCC = "BCC";
constexpr const char *BMI = "BMI";
constexpr const char *BPL = "BPL";
constexpr const char *BVS = "BVS";
constexpr const char *BVC = "BVC";
constexpr const char *BRA = "BRA"; // 65C02+

// Pseudo Branch Instructions (aliases)
constexpr const char *BLT = "BLT"; // Alias for BCC (Branch if Less Than)

// Jump Instructions
constexpr const char *JMP = "JMP";
constexpr const char *JSR = "JSR";
constexpr const char *RTS = "RTS";
constexpr const char *RTI = "RTI";
constexpr const char *JML = "JML"; // 65816
constexpr const char *JSL = "JSL"; // 65816
constexpr const char *RTL = "RTL"; // 65816

// Stack Instructions
constexpr const char *PHA = "PHA";
constexpr const char *PLA = "PLA";
constexpr const char *PHP = "PHP";
constexpr const char *PLP = "PLP";
constexpr const char *PHX = "PHX"; // 65C02+
constexpr const char *PLX = "PLX"; // 65C02+
constexpr const char *PHY = "PHY"; // 65C02+
constexpr const char *PLY = "PLY"; // 65C02+
constexpr const char *PHB = "PHB"; // 65816
constexpr const char *PLB = "PLB"; // 65816
constexpr const char *PHD = "PHD"; // 65816
constexpr const char *PLD = "PLD"; // 65816
constexpr const char *PHK = "PHK"; // 65816

// Transfer Instructions
constexpr const char *TAX = "TAX";
constexpr const char *TXA = "TXA";
constexpr const char *TAY = "TAY";
constexpr const char *TYA = "TYA";
constexpr const char *TSX = "TSX";
constexpr const char *TXS = "TXS";
constexpr const char *TCD = "TCD"; // 65816
constexpr const char *TCS = "TCS"; // 65816
constexpr const char *TDC = "TDC"; // 65816
constexpr const char *TSC = "TSC"; // 65816

// Shift/Rotate Instructions
constexpr const char *ASL = "ASL";
constexpr const char *LSR = "LSR";
constexpr const char *ROL = "ROL";
constexpr const char *ROR = "ROR";

// Flag Instructions
constexpr const char *CLC = "CLC";
constexpr const char *SEC = "SEC";
constexpr const char *CLD = "CLD";
constexpr const char *SED = "SED";
constexpr const char *CLI = "CLI";
constexpr const char *SEI = "SEI";
constexpr const char *CLV = "CLV";
constexpr const char *REP = "REP"; // 65816
constexpr const char *SEP = "SEP"; // 65816
constexpr const char *XCE = "XCE"; // 65816

// Test/Set Instructions
constexpr const char *TRB = "TRB"; // 65C02+
constexpr const char *TSB = "TSB"; // 65C02+

// Special Instructions
constexpr const char *NOP = "NOP";
constexpr const char *BRK = "BRK";
constexpr const char *COP = "COP"; // 65816
constexpr const char *STP = "STP"; // 65C02+
constexpr const char *WAI = "WAI"; // 65C02+
constexpr const char *WDM = "WDM"; // 65816
constexpr const char *XBA = "XBA"; // 65816

// Stack Instructions - 65816
constexpr const char *PEA = "PEA"; // 65816
constexpr const char *PEI = "PEI"; // 65816
constexpr const char *PER = "PER"; // 65816

// Block Move Instructions - 65816
constexpr const char *MVN = "MVN"; // 65816 - Move Negative (Block Move)
constexpr const char *MVP = "MVP"; // 65816 - Move Positive (Block Move)

} // namespace M6502Mnemonics

// ============================================================================
// Rockwell 65C02 Mnemonic Constants
// ============================================================================

namespace RockwellMnemonics {

// RMB - Reset Memory Bit (Rockwell 65C02 Extensions)
constexpr const char *RMB0 = "RMB0";
constexpr const char *RMB1 = "RMB1";
constexpr const char *RMB2 = "RMB2";
constexpr const char *RMB3 = "RMB3";
constexpr const char *RMB4 = "RMB4";
constexpr const char *RMB5 = "RMB5";
constexpr const char *RMB6 = "RMB6";
constexpr const char *RMB7 = "RMB7";

// SMB - Set Memory Bit (Rockwell 65C02 Extensions)
constexpr const char *SMB0 = "SMB0";
constexpr const char *SMB1 = "SMB1";
constexpr const char *SMB2 = "SMB2";
constexpr const char *SMB3 = "SMB3";
constexpr const char *SMB4 = "SMB4";
constexpr const char *SMB5 = "SMB5";
constexpr const char *SMB6 = "SMB6";
constexpr const char *SMB7 = "SMB7";

// BBR - Branch if Bit Reset (Rockwell 65C02 Extensions)
constexpr const char *BBR0 = "BBR0";
constexpr const char *BBR1 = "BBR1";
constexpr const char *BBR2 = "BBR2";
constexpr const char *BBR3 = "BBR3";
constexpr const char *BBR4 = "BBR4";
constexpr const char *BBR5 = "BBR5";
constexpr const char *BBR6 = "BBR6";
constexpr const char *BBR7 = "BBR7";

// BBS - Branch if Bit Set (Rockwell 65C02 Extensions)
constexpr const char *BBS0 = "BBS0";
constexpr const char *BBS1 = "BBS1";
constexpr const char *BBS2 = "BBS2";
constexpr const char *BBS3 = "BBS3";
constexpr const char *BBS4 = "BBS4";
constexpr const char *BBS5 = "BBS5";
constexpr const char *BBS6 = "BBS6";
constexpr const char *BBS7 = "BBS7";

} // namespace RockwellMnemonics

// ============================================================================
// Rockwell 65C02 Opcode Constants
// ============================================================================

namespace RockwellOpcodes {

// RMB - Reset Memory Bit (Rockwell 65C02 Extensions)
constexpr uint8_t RMB0 = 0x07; // RMB0 zp      - Reset Memory Bit 0
constexpr uint8_t RMB1 = 0x17; // RMB1 zp      - Reset Memory Bit 1
constexpr uint8_t RMB2 = 0x27; // RMB2 zp      - Reset Memory Bit 2
constexpr uint8_t RMB3 = 0x37; // RMB3 zp      - Reset Memory Bit 3
constexpr uint8_t RMB4 = 0x47; // RMB4 zp      - Reset Memory Bit 4
constexpr uint8_t RMB5 = 0x57; // RMB5 zp      - Reset Memory Bit 5
constexpr uint8_t RMB6 = 0x67; // RMB6 zp      - Reset Memory Bit 6
constexpr uint8_t RMB7 = 0x77; // RMB7 zp      - Reset Memory Bit 7

// SMB - Set Memory Bit (Rockwell 65C02 Extensions)
constexpr uint8_t SMB0 = 0x87; // SMB0 zp      - Set Memory Bit 0
constexpr uint8_t SMB1 = 0x97; // SMB1 zp      - Set Memory Bit 1
constexpr uint8_t SMB2 = 0xA7; // SMB2 zp      - Set Memory Bit 2
constexpr uint8_t SMB3 = 0xB7; // SMB3 zp      - Set Memory Bit 3
constexpr uint8_t SMB4 = 0xC7; // SMB4 zp      - Set Memory Bit 4
constexpr uint8_t SMB5 = 0xD7; // SMB5 zp      - Set Memory Bit 5
constexpr uint8_t SMB6 = 0xE7; // SMB6 zp      - Set Memory Bit 6
constexpr uint8_t SMB7 = 0xF7; // SMB7 zp      - Set Memory Bit 7

// BBR - Branch if Bit Reset (Rockwell 65C02 Extensions)
constexpr uint8_t BBR0 = 0x0F; // BBR0 zp,rel  - Branch if Bit 0 Reset
constexpr uint8_t BBR1 = 0x1F; // BBR1 zp,rel  - Branch if Bit 1 Reset
constexpr uint8_t BBR2 = 0x2F; // BBR2 zp,rel  - Branch if Bit 2 Reset
constexpr uint8_t BBR3 = 0x3F; // BBR3 zp,rel  - Branch if Bit 3 Reset
constexpr uint8_t BBR4 = 0x4F; // BBR4 zp,rel  - Branch if Bit 4 Reset
constexpr uint8_t BBR5 = 0x5F; // BBR5 zp,rel  - Branch if Bit 5 Reset
constexpr uint8_t BBR6 = 0x6F; // BBR6 zp,rel  - Branch if Bit 6 Reset
constexpr uint8_t BBR7 = 0x7F; // BBR7 zp,rel  - Branch if Bit 7 Reset

// BBS - Branch if Bit Set (Rockwell 65C02 Extensions)
constexpr uint8_t BBS0 = 0x8F; // BBS0 zp,rel  - Branch if Bit 0 Set
constexpr uint8_t BBS1 = 0x9F; // BBS1 zp,rel  - Branch if Bit 1 Set
constexpr uint8_t BBS2 = 0xAF; // BBS2 zp,rel  - Branch if Bit 2 Set
constexpr uint8_t BBS3 = 0xBF; // BBS3 zp,rel  - Branch if Bit 3 Set
constexpr uint8_t BBS4 = 0xCF; // BBS4 zp,rel  - Branch if Bit 4 Set
constexpr uint8_t BBS5 = 0xDF; // BBS5 zp,rel  - Branch if Bit 5 Set
constexpr uint8_t BBS6 = 0xEF; // BBS6 zp,rel  - Branch if Bit 6 Set
constexpr uint8_t BBS7 = 0xFF; // BBS7 zp,rel  - Branch if Bit 7 Set

} // namespace RockwellOpcodes

} // namespace xasm
