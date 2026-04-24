/**
 * @file opcodes_6502.h
 * @brief 6502/65C02/65816 opcode constants
 *
 * Replaces magic numbers with descriptive named constants.
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace Opcodes {

// ============================================================================
// Load/Store Instructions
// ============================================================================

// LDA - Load Accumulator
constexpr uint8_t kLDA_IMM = 0xA9;  // kLDA #imm     - Immediate
constexpr uint8_t kLDA_ZP = 0xA5;   // kLDA zp       - Zero Page
constexpr uint8_t kLDA_ZPX = 0xB5;  // kLDA zp,X     - Zero Page,X
constexpr uint8_t kLDA_ABS = 0xAD;  // kLDA abs      - Absolute
constexpr uint8_t kLDA_ABX = 0xBD;  // kLDA abs,X    - Absolute,X
constexpr uint8_t kLDA_ABY = 0xB9;  // kLDA abs,Y    - Absolute,Y
constexpr uint8_t kLDA_INX = 0xA1;  // kLDA (zp,X)   - Indexed Indirect
constexpr uint8_t kLDA_INY = 0xB1;  // kLDA (zp),Y   - Indirect Indexed
constexpr uint8_t kLDA_IZP = 0xB2;  // kLDA (zp)     - Indirect Zero Page (65C02+)
constexpr uint8_t kLDA_ALG = 0xAF;  // kLDA long     - Absolute Long (65816)
constexpr uint8_t kLDA_ILG = 0xA7;  // kLDA [dp]     - Indirect Long (65816)
constexpr uint8_t kLDA_ILY = 0xB7;  // kLDA [dp],Y   - Indirect Long Indexed,Y (65816)
constexpr uint8_t kLDA_SR = 0xA3;   // kLDA sr,S     - Stack Relative (65816)
constexpr uint8_t kLDA_SRY = 0xB3;  // kLDA (sr,S),Y - Stack Relative Indirect Indexed,Y (65816)

// LDX - Load X Register
constexpr uint8_t kLDX_IMM = 0xA2;  // kLDX #imm     - Immediate
constexpr uint8_t kLDX_ZP = 0xA6;   // kLDX zp       - Zero Page
constexpr uint8_t kLDX_ZPY = 0xB6;  // kLDX zp,Y     - Zero Page,Y
constexpr uint8_t kLDX_ABS = 0xAE;  // kLDX abs      - Absolute
constexpr uint8_t kLDX_ABY = 0xBE;  // kLDX abs,Y    - Absolute,Y

// LDY - Load Y Register
constexpr uint8_t kLDY_IMM = 0xA0;  // kLDY #imm     - Immediate
constexpr uint8_t kLDY_ZP = 0xA4;   // kLDY zp       - Zero Page
constexpr uint8_t kLDY_ZPX = 0xB4;  // kLDY zp,X     - Zero Page,X
constexpr uint8_t kLDY_ABS = 0xAC;  // kLDY abs      - Absolute
constexpr uint8_t kLDY_ABX = 0xBC;  // kLDY abs,X    - Absolute,X

// STA - Store Accumulator
constexpr uint8_t kSTA_ZP = 0x85;   // kSTA zp       - Zero Page
constexpr uint8_t kSTA_ZPX = 0x95;  // kSTA zp,X     - Zero Page,X
constexpr uint8_t kSTA_ABS = 0x8D;  // kSTA abs      - Absolute
constexpr uint8_t kSTA_ABX = 0x9D;  // kSTA abs,X    - Absolute,X
constexpr uint8_t kSTA_ABY = 0x99;  // kSTA abs,Y    - Absolute,Y
constexpr uint8_t kSTA_INX = 0x81;  // kSTA (zp,X)   - Indexed Indirect
constexpr uint8_t kSTA_INY = 0x91;  // kSTA (zp),Y   - Indirect Indexed
constexpr uint8_t kSTA_IZP = 0x92;  // kSTA (zp)     - Indirect Zero Page (65C02+)

// STX - Store X Register
constexpr uint8_t kSTX_ZP = 0x86;   // kSTX zp       - Zero Page
constexpr uint8_t kSTX_ZPY = 0x96;  // kSTX zp,Y     - Zero Page,Y
constexpr uint8_t kSTX_ABS = 0x8E;  // kSTX abs      - Absolute

// STY - Store Y Register
constexpr uint8_t kSTY_ZP = 0x84;   // kSTY zp       - Zero Page
constexpr uint8_t kSTY_ZPX = 0x94;  // kSTY zp,X     - Zero Page,X
constexpr uint8_t kSTY_ABS = 0x8C;  // kSTY abs      - Absolute

// STZ - Store Zero (65C02+)
constexpr uint8_t kSTZ_ZP = 0x64;   // kSTZ zp       - Zero Page
constexpr uint8_t kSTZ_ZPX = 0x74;  // kSTZ zp,X     - Zero Page,X
constexpr uint8_t kSTZ_ABS = 0x9C;  // kSTZ abs      - Absolute
constexpr uint8_t kSTZ_ABX = 0x9E;  // kSTZ abs,X    - Absolute,X

// ============================================================================
// Arithmetic Instructions
// ============================================================================

// ADC - Add with Carry
constexpr uint8_t kADC_IMM = 0x69;  // kADC #imm     - Immediate
constexpr uint8_t kADC_ZP = 0x65;   // kADC zp       - Zero Page
constexpr uint8_t kADC_ZPX = 0x75;  // kADC zp,X     - Zero Page,X
constexpr uint8_t kADC_ABS = 0x6D;  // kADC abs      - Absolute
constexpr uint8_t kADC_ABX = 0x7D;  // kADC abs,X    - Absolute,X
constexpr uint8_t kADC_ABY = 0x79;  // kADC abs,Y    - Absolute,Y
constexpr uint8_t kADC_INX = 0x61;  // kADC (zp,X)   - Indexed Indirect
constexpr uint8_t kADC_INY = 0x71;  // kADC (zp),Y   - Indirect Indexed
constexpr uint8_t kADC_IZP = 0x72;  // kADC (zp)     - Indirect Zero Page (65C02+)

// SBC - Subtract with Carry
constexpr uint8_t kSBC_IMM = 0xE9;  // kSBC #imm     - Immediate
constexpr uint8_t kSBC_ZP = 0xE5;   // kSBC zp       - Zero Page
constexpr uint8_t kSBC_ZPX = 0xF5;  // kSBC zp,X     - Zero Page,X
constexpr uint8_t kSBC_ABS = 0xED;  // kSBC abs      - Absolute
constexpr uint8_t kSBC_ABX = 0xFD;  // kSBC abs,X    - Absolute,X
constexpr uint8_t kSBC_ABY = 0xF9;  // kSBC abs,Y    - Absolute,Y
constexpr uint8_t kSBC_INX = 0xE1;  // kSBC (zp,X)   - Indexed Indirect
constexpr uint8_t kSBC_INY = 0xF1;  // kSBC (zp),Y   - Indirect Indexed
constexpr uint8_t kSBC_IZP = 0xF2;  // kSBC (zp)     - Indirect Zero Page (65C02+)

// ============================================================================
// Logic Instructions
// ============================================================================

// AND - Logical AND
constexpr uint8_t kAND_IMM = 0x29;  // kAND #imm     - Immediate
constexpr uint8_t kAND_ZP = 0x25;   // kAND zp       - Zero Page
constexpr uint8_t kAND_ZPX = 0x35;  // kAND zp,X     - Zero Page,X
constexpr uint8_t kAND_ABS = 0x2D;  // kAND abs      - Absolute
constexpr uint8_t kAND_ABX = 0x3D;  // kAND abs,X    - Absolute,X
constexpr uint8_t kAND_ABY = 0x39;  // kAND abs,Y    - Absolute,Y
constexpr uint8_t kAND_INX = 0x21;  // kAND (zp,X)   - Indexed Indirect
constexpr uint8_t kAND_INY = 0x31;  // kAND (zp),Y   - Indirect Indexed
constexpr uint8_t kAND_IZP = 0x32;  // kAND (zp)     - Indirect Zero Page (65C02+)

// ORA - Logical OR
constexpr uint8_t kORA_IMM = 0x09;  // kORA #imm     - Immediate
constexpr uint8_t kORA_ZP = 0x05;   // kORA zp       - Zero Page
constexpr uint8_t kORA_ZPX = 0x15;  // kORA zp,X     - Zero Page,X
constexpr uint8_t kORA_ABS = 0x0D;  // kORA abs      - Absolute
constexpr uint8_t kORA_ABX = 0x1D;  // kORA abs,X    - Absolute,X
constexpr uint8_t kORA_ABY = 0x19;  // kORA abs,Y    - Absolute,Y
constexpr uint8_t kORA_INX = 0x01;  // kORA (zp,X)   - Indexed Indirect
constexpr uint8_t kORA_INY = 0x11;  // kORA (zp),Y   - Indirect Indexed
constexpr uint8_t kORA_IZP = 0x12;  // kORA (zp)     - Indirect Zero Page (65C02+)

// EOR - Exclusive OR
constexpr uint8_t kEOR_IMM = 0x49;  // kEOR #imm     - Immediate
constexpr uint8_t kEOR_ZP = 0x45;   // kEOR zp       - Zero Page
constexpr uint8_t kEOR_ZPX = 0x55;  // kEOR zp,X     - Zero Page,X
constexpr uint8_t kEOR_ABS = 0x4D;  // kEOR abs      - Absolute
constexpr uint8_t kEOR_ABX = 0x5D;  // kEOR abs,X    - Absolute,X
constexpr uint8_t kEOR_ABY = 0x59;  // kEOR abs,Y    - Absolute,Y
constexpr uint8_t kEOR_INX = 0x41;  // kEOR (zp,X)   - Indexed Indirect
constexpr uint8_t kEOR_INY = 0x51;  // kEOR (zp),Y   - Indirect Indexed
constexpr uint8_t kEOR_IZP = 0x52;  // kEOR (zp)     - Indirect Zero Page (65C02+)

// ============================================================================
// Shift and Rotate Instructions
// ============================================================================

// ASL - Arithmetic Shift Left
constexpr uint8_t kASL_ACC = 0x0A;  // kASL A        - Accumulator
constexpr uint8_t kASL_ZP = 0x06;   // kASL zp       - Zero Page
constexpr uint8_t kASL_ZPX = 0x16;  // kASL zp,X     - Zero Page,X
constexpr uint8_t kASL_ABS = 0x0E;  // kASL abs      - Absolute
constexpr uint8_t kASL_ABX = 0x1E;  // kASL abs,X    - Absolute,X

// LSR - Logical Shift Right
constexpr uint8_t kLSR_ACC = 0x4A;  // kLSR A        - Accumulator
constexpr uint8_t kLSR_ZP = 0x46;   // kLSR zp       - Zero Page
constexpr uint8_t kLSR_ZPX = 0x56;  // kLSR zp,X     - Zero Page,X
constexpr uint8_t kLSR_ABS = 0x4E;  // kLSR abs      - Absolute
constexpr uint8_t kLSR_ABX = 0x5E;  // kLSR abs,X    - Absolute,X

// ROL - Rotate Left
constexpr uint8_t kROL_ACC = 0x2A;  // kROL A        - Accumulator
constexpr uint8_t kROL_ZP = 0x26;   // kROL zp       - Zero Page
constexpr uint8_t kROL_ZPX = 0x36;  // kROL zp,X     - Zero Page,X
constexpr uint8_t kROL_ABS = 0x2E;  // kROL abs      - Absolute
constexpr uint8_t kROL_ABX = 0x3E;  // kROL abs,X    - Absolute,X

// ROR - Rotate Right
constexpr uint8_t kROR_ACC = 0x6A;  // kROR A        - Accumulator
constexpr uint8_t kROR_ZP = 0x66;   // kROR zp       - Zero Page
constexpr uint8_t kROR_ZPX = 0x76;  // kROR zp,X     - Zero Page,X
constexpr uint8_t kROR_ABS = 0x6E;  // kROR abs      - Absolute
constexpr uint8_t kROR_ABX = 0x7E;  // kROR abs,X    - Absolute,X

// ============================================================================
// Compare Instructions
// ============================================================================

// CMP - Compare Accumulator
constexpr uint8_t kCMP_IMM = 0xC9;  // kCMP #imm     - Immediate
constexpr uint8_t kCMP_ZP = 0xC5;   // kCMP zp       - Zero Page
constexpr uint8_t kCMP_ZPX = 0xD5;  // kCMP zp,X     - Zero Page,X
constexpr uint8_t kCMP_ABS = 0xCD;  // kCMP abs      - Absolute
constexpr uint8_t kCMP_ABX = 0xDD;  // kCMP abs,X    - Absolute,X
constexpr uint8_t kCMP_ABY = 0xD9;  // kCMP abs,Y    - Absolute,Y
constexpr uint8_t kCMP_INX = 0xC1;  // kCMP (zp,X)   - Indexed Indirect
constexpr uint8_t kCMP_INY = 0xD1;  // kCMP (zp),Y   - Indirect Indexed
constexpr uint8_t kCMP_IZP = 0xD2;  // kCMP (zp)     - Indirect Zero Page (65C02+)

// CPX - Compare X Register
constexpr uint8_t kCPX_IMM = 0xE0;  // kCPX #imm     - Immediate
constexpr uint8_t kCPX_ZP = 0xE4;   // kCPX zp       - Zero Page
constexpr uint8_t kCPX_ABS = 0xEC;  // kCPX abs      - Absolute

// CPY - Compare Y Register
constexpr uint8_t kCPY_IMM = 0xC0;  // kCPY #imm     - Immediate
constexpr uint8_t kCPY_ZP = 0xC4;   // kCPY zp       - Zero Page
constexpr uint8_t kCPY_ABS = 0xCC;  // kCPY abs      - Absolute

// ============================================================================
// Branch Instructions
// ============================================================================

constexpr uint8_t kBEQ = 0xF0;  // kBEQ rel      - Branch if Equal
constexpr uint8_t kBNE = 0xD0;  // kBNE rel      - Branch if Not Equal
constexpr uint8_t kBCC = 0x90;  // kBCC rel      - Branch if Carry Clear
constexpr uint8_t kBCS = 0xB0;  // kBCS rel      - Branch if Carry Set
constexpr uint8_t kBMI = 0x30;  // kBMI rel      - Branch if Minus
constexpr uint8_t kBPL = 0x10;  // kBPL rel      - Branch if Plus
constexpr uint8_t kBVC = 0x50;  // kBVC rel      - Branch if Overflow Clear
constexpr uint8_t kBVS = 0x70;  // kBVS rel      - Branch if Overflow Set
constexpr uint8_t kBRA = 0x80;  // kBRA rel      - Branch Always (65C02)

// ============================================================================
// Jump and Subroutine Instructions
// ============================================================================

constexpr uint8_t kJMP_ABS = 0x4C;  // kJMP abs      - Jump Absolute
constexpr uint8_t kJMP_IND = 0x6C;  // kJMP (ind)    - Jump Indirect
constexpr uint8_t kJMP_AIX = 0x7C;  // kJMP (abs,X)  - Jump Absolute Indexed Indirect (65C02+)

constexpr uint8_t kJSR = 0x20;  // kJSR abs      - Jump to Subroutine
constexpr uint8_t kRTS = 0x60;  // kRTS          - Return from Subroutine
constexpr uint8_t kRTI = 0x40;  // kRTI          - Return from Interrupt

// ============================================================================
// Stack Instructions
// ============================================================================

constexpr uint8_t kPHA = 0x48;  // kPHA          - Push Accumulator
constexpr uint8_t kPLA = 0x68;  // kPLA          - Pull Accumulator
constexpr uint8_t kPHP = 0x08;  // kPHP          - Push Processor Status
constexpr uint8_t kPLP = 0x28;  // kPLP          - Pull Processor Status
constexpr uint8_t kPHX = 0xDA;  // kPHX          - Push X Register (65C02+)
constexpr uint8_t kPLX = 0xFA;  // kPLX          - Pull X Register (65C02+)
constexpr uint8_t kPHY = 0x5A;  // kPHY          - Push Y Register (65C02+)
constexpr uint8_t kPLY = 0x7A;  // kPLY          - Pull Y Register (65C02+)
constexpr uint8_t kTSX = 0xBA;  // kTSX          - Transfer SP to X
constexpr uint8_t kTXS = 0x9A;  // kTXS          - Transfer X to SP

// ============================================================================
// Increment/Decrement Instructions
// ============================================================================

constexpr uint8_t kINX = 0xE8;  // kINX          - Increment X
constexpr uint8_t kINY = 0xC8;  // kINY          - Increment Y
constexpr uint8_t kDEX = 0xCA;  // kDEX          - Decrement X
constexpr uint8_t kDEY = 0x88;  // kDEY          - Decrement Y

// INC - Increment Memory
constexpr uint8_t kINC_ZP = 0xE6;   // kINC zp       - Zero Page
constexpr uint8_t kINC_ZPX = 0xF6;  // kINC zp,X     - Zero Page,X
constexpr uint8_t kINC_ABS = 0xEE;  // kINC abs      - Absolute
constexpr uint8_t kINC_ABX = 0xFE;  // kINC abs,X    - Absolute,X
constexpr uint8_t kINC_ACC = 0x1A;  // kINC / INA    - Accumulator (65C02+)

// DEC - Decrement Memory
constexpr uint8_t kDEC_ZP = 0xC6;   // kDEC zp       - Zero Page
constexpr uint8_t kDEC_ZPX = 0xD6;  // kDEC zp,X     - Zero Page,X
constexpr uint8_t kDEC_ABS = 0xCE;  // kDEC abs      - Absolute
constexpr uint8_t kDEC_ABX = 0xDE;  // kDEC abs,X    - Absolute,X
constexpr uint8_t kDEC_ACC = 0x3A;  // kDEC / DEA    - Accumulator (65C02+)

// ============================================================================
// Flag Operation Instructions
// ============================================================================

constexpr uint8_t kCLC = 0x18;  // kCLC          - Clear Carry
constexpr uint8_t kSEC = 0x38;  // kSEC          - Set Carry
constexpr uint8_t kCLD = 0xD8;  // kCLD          - Clear Decimal
constexpr uint8_t kSED = 0xF8;  // kSED          - Set Decimal
constexpr uint8_t kCLI = 0x58;  // kCLI          - Clear Interrupt Disable
constexpr uint8_t kSEI = 0x78;  // kSEI          - Set Interrupt Disable
constexpr uint8_t kCLV = 0xB8;  // kCLV          - Clear Overflow

// ============================================================================
// Transfer Instructions
// ============================================================================

constexpr uint8_t kTAX = 0xAA;  // kTAX          - Transfer A to X
constexpr uint8_t kTAY = 0xA8;  // kTAY          - Transfer A to Y
constexpr uint8_t kTXA = 0x8A;  // kTXA          - Transfer X to A
constexpr uint8_t kTYA = 0x98;  // kTYA          - Transfer Y to A

// ============================================================================
// Special Instructions
// ============================================================================

constexpr uint8_t kNOP = 0xEA;  // kNOP          - No Operation
constexpr uint8_t kBRK = 0x00;  // kBRK          - Break
constexpr uint8_t kWAI = 0xCB;  // kWAI          - Wait for Interrupt (65C02 Rockwell)
constexpr uint8_t kSTP = 0xDB;  // kSTP          - Stop Processor (65C02 Rockwell)

// BIT - Test Bits
constexpr uint8_t kBIT_ZP = 0x24;   // kBIT zp       - Zero Page
constexpr uint8_t kBIT_ABS = 0x2C;  // kBIT abs      - Absolute
constexpr uint8_t kBIT_IMM = 0x89;  // kBIT #imm     - Immediate (65C02+)
constexpr uint8_t kBIT_ZPX = 0x34;  // kBIT zp,X     - Zero Page,X (65C02+)
constexpr uint8_t kBIT_ABX = 0x3C;  // kBIT abs,X    - Absolute,X (65C02+)

// TRB - Test and Reset Bits (65C02+)
constexpr uint8_t kTRB_ZP = 0x14;   // kTRB zp       - Zero Page
constexpr uint8_t kTRB_ABS = 0x1C;  // kTRB abs      - Absolute

// TSB - Test and Set Bits (65C02+)
constexpr uint8_t kTSB_ZP = 0x04;   // kTSB zp       - Zero Page
constexpr uint8_t kTSB_ABS = 0x0C;  // kTSB abs      - Absolute

// ============================================================================
// 65816-Specific Instructions
// ============================================================================

// 65816 Stack Operations
constexpr uint8_t kPHB = 0x8B;  // kPHB          - Push Data Bank Register
constexpr uint8_t kPLB = 0xAB;  // kPLB          - Pull Data Bank Register
constexpr uint8_t kPHK = 0x4B;  // kPHK          - Push Program Bank Register
constexpr uint8_t kPHD = 0x0B;  // kPHD          - Push Direct Page Register
constexpr uint8_t kPLD = 0x2B;  // kPLD          - Pull Direct Page Register

// 65816 Transfer Operations
constexpr uint8_t kTCD = 0x5B;  // kTCD          - Transfer C to Direct Page
constexpr uint8_t kTDC = 0x7B;  // kTDC          - Transfer Direct Page to C
constexpr uint8_t kTCS = 0x1B;  // kTCS          - Transfer C to Stack Pointer
constexpr uint8_t kTSC = 0x3B;  // kTSC          - Transfer Stack Pointer to C
constexpr uint8_t kTXY = 0x9B;  // kTXY          - Transfer X to Y (65816)
constexpr uint8_t kTYX = 0xBB;  // kTYX          - Transfer Y to X (65816)

// 65816 Jump Operations
constexpr uint8_t kJML_ALG = 0x5C;  // kJML long     - Jump Long Absolute
constexpr uint8_t kJML_IND = 0xDC;  // kJML [ind]    - Jump Long Indirect
constexpr uint8_t kJSL_ALG = 0x22;  // kJSL long     - Jump Subroutine Long
constexpr uint8_t kRTL = 0x6B;      // kRTL          - Return from Subroutine Long

// 65816 Stack Addressing
constexpr uint8_t kPEA = 0xF4;  // kPEA abs      - Push Effective Absolute
constexpr uint8_t kPEI = 0xD4;  // kPEI zp       - Push Effective Indirect
constexpr uint8_t kPER = 0x62;  // kPER rel      - Push Effective PC Relative

// 65816 Block Move
constexpr uint8_t kMVN = 0x54;  // kMVN src,dst  - Block Move Negative
constexpr uint8_t kMVP = 0x44;  // kMVP src,dst  - Block Move Positive

// 65816 Special Operations
constexpr uint8_t kCOP = 0x02;  // kCOP #imm     - Coprocessor Enable
constexpr uint8_t kWDM = 0x42;  // kWDM #imm     - Reserved (WDC)
constexpr uint8_t kXBA = 0xEB;  // kXBA          - Exchange B and A
constexpr uint8_t kXCE = 0xFB;  // kXCE          - Exchange Carry and Emulation
constexpr uint8_t kSEP = 0xE2;  // kSEP #imm     - Set Processor Status Bits
constexpr uint8_t kREP = 0xC2;  // kREP #imm     - Reset Processor Status Bits

// ============================================================================
// Branch Relaxation Helper
// ============================================================================

// XOR mask to get complementary branch opcode for branch relaxation
// All branch opcodes can be inverted by XORing with 0x20
// Example: kBEQ (0xF0) XOR 0x20 = kBNE (0xD0)
constexpr uint8_t kBRANCH_COMPLEMENT_MASK = 0x20;

// Offset used in branch relaxation sequence: B!cc +3; JMP target
constexpr uint8_t kBRANCH_RELAXATION_OFFSET = 0x03;

// ============================================================================
// Numeric Radix Constants
// ============================================================================

// Radix values for std::stoul and similar parsing functions
constexpr int kRADIX_HEXADECIMAL = 16;  // Hexadecimal (base 16)
constexpr int kRADIX_DECIMAL = 10;      // Decimal (base 10)

}  // namespace Opcodes

// ============================================================================
// Mnemonic String Constants
// ============================================================================

namespace M6502Mnemonics {

// Load/Store Instructions
constexpr const char* kLDA = "LDA";
constexpr const char* kLDX = "LDX";
constexpr const char* kLDY = "LDY";
constexpr const char* kSTA = "STA";
constexpr const char* kSTX = "STX";
constexpr const char* kSTY = "STY";
constexpr const char* kSTZ = "STZ";  // 65C02+

// Arithmetic Instructions
constexpr const char* kADC = "ADC";
constexpr const char* kSBC = "SBC";
constexpr const char* kINC = "INC";
constexpr const char* kDEC = "DEC";
constexpr const char* kINX = "INX";
constexpr const char* kINY = "INY";
constexpr const char* kDEX = "DEX";
constexpr const char* kDEY = "DEY";

// Logical Instructions
constexpr const char* kAND = "AND";
constexpr const char* kORA = "ORA";
constexpr const char* kEOR = "EOR";
constexpr const char* kBIT = "BIT";

// Compare Instructions
constexpr const char* kCMP = "CMP";
constexpr const char* kCPX = "CPX";
constexpr const char* kCPY = "CPY";

// Branch Instructions
constexpr const char* kBEQ = "BEQ";
constexpr const char* kBNE = "BNE";
constexpr const char* kBCS = "BCS";
constexpr const char* kBCC = "BCC";
constexpr const char* kBMI = "BMI";
constexpr const char* kBPL = "BPL";
constexpr const char* kBVS = "BVS";
constexpr const char* kBVC = "BVC";
constexpr const char* kBRA = "BRA";  // 65C02+

// Pseudo Branch Instructions (aliases)
constexpr const char* kBLT = "BLT";  // Alias for kBCC (Branch if Less Than)

// Jump Instructions
constexpr const char* kJMP = "JMP";
constexpr const char* kJSR = "JSR";
constexpr const char* kRTS = "RTS";
constexpr const char* kRTI = "RTI";
constexpr const char* kJML = "JML";  // 65816
constexpr const char* kJSL = "JSL";  // 65816
constexpr const char* kRTL = "RTL";  // 65816

// Stack Instructions
constexpr const char* kPHA = "PHA";
constexpr const char* kPLA = "PLA";
constexpr const char* kPHP = "PHP";
constexpr const char* kPLP = "PLP";
constexpr const char* kPHX = "PHX";  // 65C02+
constexpr const char* kPLX = "PLX";  // 65C02+
constexpr const char* kPHY = "PHY";  // 65C02+
constexpr const char* kPLY = "PLY";  // 65C02+
constexpr const char* kPHB = "PHB";  // 65816
constexpr const char* kPLB = "PLB";  // 65816
constexpr const char* kPHD = "PHD";  // 65816
constexpr const char* kPLD = "PLD";  // 65816
constexpr const char* kPHK = "PHK";  // 65816

// Transfer Instructions
constexpr const char* kTAX = "TAX";
constexpr const char* kTXA = "TXA";
constexpr const char* kTAY = "TAY";
constexpr const char* kTYA = "TYA";
constexpr const char* kTSX = "TSX";
constexpr const char* kTXS = "TXS";
constexpr const char* kTCD = "TCD";  // 65816
constexpr const char* kTCS = "TCS";  // 65816
constexpr const char* kTDC = "TDC";  // 65816
constexpr const char* kTSC = "TSC";  // 65816
constexpr const char* kTXY = "TXY";  // 65816
constexpr const char* kTYX = "TYX";  // 65816

// Shift/Rotate Instructions
constexpr const char* kASL = "ASL";
constexpr const char* kLSR = "LSR";
constexpr const char* kROL = "ROL";
constexpr const char* kROR = "ROR";

// Flag Instructions
constexpr const char* kCLC = "CLC";
constexpr const char* kSEC = "SEC";
constexpr const char* kCLD = "CLD";
constexpr const char* kSED = "SED";
constexpr const char* kCLI = "CLI";
constexpr const char* kSEI = "SEI";
constexpr const char* kCLV = "CLV";
constexpr const char* kREP = "REP";  // 65816
constexpr const char* kSEP = "SEP";  // 65816
constexpr const char* kXCE = "XCE";  // 65816

// Test/Set Instructions
constexpr const char* kTRB = "TRB";  // 65C02+
constexpr const char* kTSB = "TSB";  // 65C02+

// Special Instructions
constexpr const char* kNOP = "NOP";
constexpr const char* kBRK = "BRK";
constexpr const char* kCOP = "COP";  // 65816
constexpr const char* kSTP = "STP";  // 65C02+
constexpr const char* kWAI = "WAI";  // 65C02+
constexpr const char* kWDM = "WDM";  // 65816
constexpr const char* kXBA = "XBA";  // 65816

// Stack Instructions - 65816
constexpr const char* kPEA = "PEA";  // 65816
constexpr const char* kPEI = "PEI";  // 65816
constexpr const char* kPER = "PER";  // 65816

// Block Move Instructions - 65816
constexpr const char* kMVN = "MVN";  // 65816 - Move Negative (Block Move)
constexpr const char* kMVP = "MVP";  // 65816 - Move Positive (Block Move)

}  // namespace M6502Mnemonics

// ============================================================================
// Rockwell 65C02 Mnemonic Constants
// ============================================================================

namespace RockwellMnemonics {

// RMB - Reset Memory Bit (Rockwell 65C02 Extensions)
constexpr const char* kRMB0 = "RMB0";
constexpr const char* kRMB1 = "RMB1";
constexpr const char* kRMB2 = "RMB2";
constexpr const char* kRMB3 = "RMB3";
constexpr const char* kRMB4 = "RMB4";
constexpr const char* kRMB5 = "RMB5";
constexpr const char* kRMB6 = "RMB6";
constexpr const char* kRMB7 = "RMB7";

// SMB - Set Memory Bit (Rockwell 65C02 Extensions)
constexpr const char* kSMB0 = "SMB0";
constexpr const char* kSMB1 = "SMB1";
constexpr const char* kSMB2 = "SMB2";
constexpr const char* kSMB3 = "SMB3";
constexpr const char* kSMB4 = "SMB4";
constexpr const char* kSMB5 = "SMB5";
constexpr const char* kSMB6 = "SMB6";
constexpr const char* kSMB7 = "SMB7";

// BBR - Branch if Bit Reset (Rockwell 65C02 Extensions)
constexpr const char* kBBR0 = "BBR0";
constexpr const char* kBBR1 = "BBR1";
constexpr const char* kBBR2 = "BBR2";
constexpr const char* kBBR3 = "BBR3";
constexpr const char* kBBR4 = "BBR4";
constexpr const char* kBBR5 = "BBR5";
constexpr const char* kBBR6 = "BBR6";
constexpr const char* kBBR7 = "BBR7";

// BBS - Branch if Bit Set (Rockwell 65C02 Extensions)
constexpr const char* kBBS0 = "BBS0";
constexpr const char* kBBS1 = "BBS1";
constexpr const char* kBBS2 = "BBS2";
constexpr const char* kBBS3 = "BBS3";
constexpr const char* kBBS4 = "BBS4";
constexpr const char* kBBS5 = "BBS5";
constexpr const char* kBBS6 = "BBS6";
constexpr const char* kBBS7 = "BBS7";

}  // namespace RockwellMnemonics

// ============================================================================
// Rockwell 65C02 Opcode Constants
// ============================================================================

namespace RockwellOpcodes {

// RMB - Reset Memory Bit (Rockwell 65C02 Extensions)
constexpr uint8_t kRMB0 = 0x07;  // kRMB0 zp      - Reset Memory Bit 0
constexpr uint8_t kRMB1 = 0x17;  // kRMB1 zp      - Reset Memory Bit 1
constexpr uint8_t kRMB2 = 0x27;  // kRMB2 zp      - Reset Memory Bit 2
constexpr uint8_t kRMB3 = 0x37;  // kRMB3 zp      - Reset Memory Bit 3
constexpr uint8_t kRMB4 = 0x47;  // kRMB4 zp      - Reset Memory Bit 4
constexpr uint8_t kRMB5 = 0x57;  // kRMB5 zp      - Reset Memory Bit 5
constexpr uint8_t kRMB6 = 0x67;  // kRMB6 zp      - Reset Memory Bit 6
constexpr uint8_t kRMB7 = 0x77;  // kRMB7 zp      - Reset Memory Bit 7

// SMB - Set Memory Bit (Rockwell 65C02 Extensions)
constexpr uint8_t kSMB0 = 0x87;  // kSMB0 zp      - Set Memory Bit 0
constexpr uint8_t kSMB1 = 0x97;  // kSMB1 zp      - Set Memory Bit 1
constexpr uint8_t kSMB2 = 0xA7;  // kSMB2 zp      - Set Memory Bit 2
constexpr uint8_t kSMB3 = 0xB7;  // kSMB3 zp      - Set Memory Bit 3
constexpr uint8_t kSMB4 = 0xC7;  // kSMB4 zp      - Set Memory Bit 4
constexpr uint8_t kSMB5 = 0xD7;  // kSMB5 zp      - Set Memory Bit 5
constexpr uint8_t kSMB6 = 0xE7;  // kSMB6 zp      - Set Memory Bit 6
constexpr uint8_t kSMB7 = 0xF7;  // kSMB7 zp      - Set Memory Bit 7

// BBR - Branch if Bit Reset (Rockwell 65C02 Extensions)
constexpr uint8_t kBBR0 = 0x0F;  // kBBR0 zp,rel  - Branch if Bit 0 Reset
constexpr uint8_t kBBR1 = 0x1F;  // kBBR1 zp,rel  - Branch if Bit 1 Reset
constexpr uint8_t kBBR2 = 0x2F;  // kBBR2 zp,rel  - Branch if Bit 2 Reset
constexpr uint8_t kBBR3 = 0x3F;  // kBBR3 zp,rel  - Branch if Bit 3 Reset
constexpr uint8_t kBBR4 = 0x4F;  // kBBR4 zp,rel  - Branch if Bit 4 Reset
constexpr uint8_t kBBR5 = 0x5F;  // kBBR5 zp,rel  - Branch if Bit 5 Reset
constexpr uint8_t kBBR6 = 0x6F;  // kBBR6 zp,rel  - Branch if Bit 6 Reset
constexpr uint8_t kBBR7 = 0x7F;  // kBBR7 zp,rel  - Branch if Bit 7 Reset

// BBS - Branch if Bit Set (Rockwell 65C02 Extensions)
constexpr uint8_t kBBS0 = 0x8F;  // kBBS0 zp,rel  - Branch if Bit 0 Set
constexpr uint8_t kBBS1 = 0x9F;  // kBBS1 zp,rel  - Branch if Bit 1 Set
constexpr uint8_t kBBS2 = 0xAF;  // kBBS2 zp,rel  - Branch if Bit 2 Set
constexpr uint8_t kBBS3 = 0xBF;  // kBBS3 zp,rel  - Branch if Bit 3 Set
constexpr uint8_t kBBS4 = 0xCF;  // kBBS4 zp,rel  - Branch if Bit 4 Set
constexpr uint8_t kBBS5 = 0xDF;  // kBBS5 zp,rel  - Branch if Bit 5 Set
constexpr uint8_t kBBS6 = 0xEF;  // kBBS6 zp,rel  - Branch if Bit 6 Set
constexpr uint8_t kBBS7 = 0xFF;  // kBBS7 zp,rel  - Branch if Bit 7 Set

}  // namespace RockwellOpcodes

}  // namespace xasm
