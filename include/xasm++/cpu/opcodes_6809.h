/**
 * @file opcodes_6809.h
 * @brief Motorola 6809 opcode constants
 *
 * Replaces magic numbers with descriptive named constants.
 *
 * The 6809 uses a 3-page opcode space:
 * - Page 1: Standard opcodes (direct encoding)
 * - Page 2: Extended opcodes (prefix $10)
 * - Page 3: Extended opcodes (prefix $11)
 */

#pragma once

#include <cstdint>

namespace xasm {

namespace Opcodes6809 {

// ============================================================================
// Opcode Page Prefixes
// ============================================================================

constexpr uint8_t kPAGE2_PREFIX = 0x10;  // Page 2 prefix for extended opcodes
constexpr uint8_t kPAGE3_PREFIX = 0x11;  // Page 3 prefix for extended opcodes

// ============================================================================
// Page 1: Standard Opcodes
// ============================================================================

// ----------------------------------------------------------------------------
// Inherent Mode Instructions (no operand)
// ----------------------------------------------------------------------------

constexpr uint8_t kNOP = 0x12;   // kNOP          - No Operation
constexpr uint8_t kRTS = 0x39;   // kRTS          - Return from Subroutine
constexpr uint8_t kCLRA = 0x4F;  // kCLRA         - Clear Accumulator A
constexpr uint8_t kCLRB = 0x5F;  // kCLRB         - Clear Accumulator B

// Shift and Rotate Instructions (Inherent on A/B)
constexpr uint8_t kASLA = 0x48;  // kASLA         - Arithmetic Shift Left A
constexpr uint8_t kASLB = 0x58;  // kASLB         - Arithmetic Shift Left B
constexpr uint8_t kASRA = 0x47;  // kASRA         - Arithmetic Shift Right A
constexpr uint8_t kASRB = 0x57;  // kASRB         - Arithmetic Shift Right B
constexpr uint8_t kLSRA = 0x44;  // kLSRA         - Logical Shift Right A
constexpr uint8_t kLSRB = 0x54;  // kLSRB         - Logical Shift Right B
constexpr uint8_t kROLA = 0x49;  // kROLA         - Rotate Left A through Carry
constexpr uint8_t kROLB = 0x59;  // kROLB         - Rotate Left B through Carry
constexpr uint8_t kRORA = 0x46;  // kRORA         - Rotate Right A through Carry
constexpr uint8_t kRORB = 0x56;  // kRORB         - Rotate Right B through Carry

// Increment/Decrement Instructions (Inherent on A/B)
constexpr uint8_t kINCA = 0x4C;  // kINCA         - Increment A
constexpr uint8_t kINCB = 0x5C;  // kINCB         - Increment B
constexpr uint8_t kDECA = 0x4A;  // kDECA         - Decrement A
constexpr uint8_t kDECB = 0x5A;  // kDECB         - Decrement B

// Test/Compare/Negate Instructions (Inherent on A/B)
constexpr uint8_t kTSTA = 0x4D;  // kTSTA         - Test A (set flags)
constexpr uint8_t kTSTB = 0x5D;  // kTSTB         - Test B (set flags)
constexpr uint8_t kCOMA = 0x43;  // kCOMA         - Complement A (one's complement)
constexpr uint8_t kCOMB = 0x53;  // kCOMB         - Complement B (one's complement)
constexpr uint8_t kNEGA = 0x40;  // kNEGA         - Negate A (two's complement)
constexpr uint8_t kNEGB = 0x50;  // kNEGB         - Negate B (two's complement)

// ----------------------------------------------------------------------------
// Load/Store Instructions - 8-bit Accumulators (A, B)
// ----------------------------------------------------------------------------

// LDA - Load Accumulator A
constexpr uint8_t kLDA_IMM = 0x86;  // LDA #imm     - Immediate
constexpr uint8_t kLDA_DIR = 0x96;  // LDA <dp      - Direct Page
constexpr uint8_t kLDA_EXT = 0xB6;  // LDA addr     - Extended (absolute)
constexpr uint8_t kLDA_IDX = 0xA6;  // LDA ,X       - Indexed

// LDB - Load Accumulator B
constexpr uint8_t kLDB_IMM = 0xC6;  // LDB #imm     - Immediate
constexpr uint8_t kLDB_DIR = 0xD6;  // LDB <dp      - Direct Page
constexpr uint8_t kLDB_EXT = 0xF6;  // LDB addr     - Extended (absolute)
constexpr uint8_t kLDB_IDX = 0xE6;  // LDB ,X       - Indexed

// STA - Store Accumulator A
constexpr uint8_t kSTA_DIR = 0x97;  // STA <dp      - Direct Page
constexpr uint8_t kSTA_EXT = 0xB7;  // STA addr     - Extended (absolute)
constexpr uint8_t kSTA_IDX = 0xA7;  // STA ,X       - Indexed

// STB - Store Accumulator B
constexpr uint8_t kSTB_DIR = 0xD7;  // STB <dp      - Direct Page
constexpr uint8_t kSTB_EXT = 0xF7;  // STB addr     - Extended (absolute)
constexpr uint8_t kSTB_IDX = 0xE7;  // STB ,X       - Indexed

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit D Register (A:B concatenated)
// ----------------------------------------------------------------------------

// LDD - Load Accumulator D (16-bit: A=high, B=low)
constexpr uint8_t kLDD_IMM = 0xCC;  // LDD #imm     - Immediate (big-endian)
constexpr uint8_t kLDD_DIR = 0xDC;  // LDD <dp      - Direct Page
constexpr uint8_t kLDD_EXT = 0xFC;  // LDD addr     - Extended (absolute)
constexpr uint8_t kLDD_IDX = 0xEC;  // LDD ,X       - Indexed

// STD - Store Accumulator D (16-bit)
constexpr uint8_t kSTD_DIR = 0xDD;  // STD <dp      - Direct Page
constexpr uint8_t kSTD_EXT = 0xFD;  // STD addr     - Extended (absolute)
constexpr uint8_t kSTD_IDX = 0xED;  // STD ,X       - Indexed

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit Index Registers (X, Y)
// ----------------------------------------------------------------------------

// LDX - Load Index Register X
constexpr uint8_t kLDX_IMM = 0x8E;  // LDX #imm     - Immediate (big-endian)
constexpr uint8_t kLDX_DIR = 0x9E;  // LDX <dp      - Direct Page
constexpr uint8_t kLDX_EXT = 0xBE;  // LDX addr     - Extended (absolute)
constexpr uint8_t kLDX_IDX = 0xAE;  // LDX ,X       - Indexed

// STX - Store Index Register X
constexpr uint8_t kSTX_DIR = 0x9F;  // STX <dp      - Direct Page
constexpr uint8_t kSTX_EXT = 0xBF;  // STX addr     - Extended (absolute)
constexpr uint8_t kSTX_IDX = 0xAF;  // STX ,X       - Indexed

// LDY - Load Index Register Y (Page 2: $10 prefix)
// Note: Actual opcodes are kPAGE2_PREFIX followed by same codes as LDX

// STY - Store Index Register Y (Page 2: $10 prefix)
// Note: Actual opcodes are kPAGE2_PREFIX followed by same codes as STX

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit Stack Pointers (U, S)
// ----------------------------------------------------------------------------

// LDU - Load User Stack Pointer
constexpr uint8_t kLDU_IMM = 0xCE;  // LDU #imm     - Immediate (big-endian)
constexpr uint8_t kLDU_DIR = 0xDE;  // LDU <dp      - Direct Page
constexpr uint8_t kLDU_EXT = 0xFE;  // LDU addr     - Extended (absolute)
constexpr uint8_t kLDU_IDX = 0xEE;  // LDU ,X       - Indexed

// STU - Store User Stack Pointer
constexpr uint8_t kSTU_DIR = 0xDF;  // STU <dp      - Direct Page
constexpr uint8_t kSTU_EXT = 0xFF;  // STU addr     - Extended (absolute)
constexpr uint8_t kSTU_IDX = 0xEF;  // STU ,X       - Indexed

// LDS - Load System Stack Pointer (Page 2: $10 prefix)
// Note: Actual opcodes are kPAGE2_PREFIX followed by same codes as LDU

// STS - Store System Stack Pointer (Page 2: $10 prefix)
// Note: Actual opcodes are kPAGE2_PREFIX followed by same codes as STU

// ----------------------------------------------------------------------------
// Arithmetic Instructions
// ----------------------------------------------------------------------------

// ADDA - Add to Accumulator A
constexpr uint8_t kADDA_IMM = 0x8B;  // ADDA #imm    - Immediate
constexpr uint8_t kADDA_DIR = 0x9B;  // ADDA <dp     - Direct Page
constexpr uint8_t kADDA_EXT = 0xBB;  // ADDA addr    - Extended (absolute)
constexpr uint8_t kADDA_IDX = 0xAB;  // ADDA ,X      - Indexed

// ADDB - Add to Accumulator B
constexpr uint8_t kADDB_IMM = 0xCB;  // ADDB #imm    - Immediate
constexpr uint8_t kADDB_DIR = 0xDB;  // ADDB <dp     - Direct Page
constexpr uint8_t kADDB_EXT = 0xFB;  // ADDB addr    - Extended (absolute)
constexpr uint8_t kADDB_IDX = 0xEB;  // ADDB ,X      - Indexed

// ADCA - Add with Carry to Accumulator A
constexpr uint8_t kADCA_IMM = 0x89;  // ADCA #imm    - Immediate
constexpr uint8_t kADCA_DIR = 0x99;  // ADCA <dp     - Direct Page
constexpr uint8_t kADCA_EXT = 0xB9;  // ADCA addr    - Extended (absolute)
constexpr uint8_t kADCA_IDX = 0xA9;  // ADCA ,X      - Indexed

// ADCB - Add with Carry to Accumulator B
constexpr uint8_t kADCB_IMM = 0xC9;  // ADCB #imm    - Immediate
constexpr uint8_t kADCB_DIR = 0xD9;  // ADCB <dp     - Direct Page
constexpr uint8_t kADCB_EXT = 0xF9;  // ADCB addr    - Extended (absolute)
constexpr uint8_t kADCB_IDX = 0xE9;  // ADCB ,X      - Indexed

// SUBA - Subtract from Accumulator A
constexpr uint8_t kSUBA_IMM = 0x80;  // SUBA #imm    - Immediate
constexpr uint8_t kSUBA_DIR = 0x90;  // SUBA <dp     - Direct Page
constexpr uint8_t kSUBA_EXT = 0xB0;  // SUBA addr    - Extended (absolute)
constexpr uint8_t kSUBA_IDX = 0xA0;  // SUBA ,X      - Indexed

// SUBB - Subtract from Accumulator B
constexpr uint8_t kSUBB_IMM = 0xC0;  // SUBB #imm    - Immediate
constexpr uint8_t kSUBB_DIR = 0xD0;  // SUBB <dp     - Direct Page
constexpr uint8_t kSUBB_EXT = 0xF0;  // SUBB addr    - Extended (absolute)
constexpr uint8_t kSUBB_IDX = 0xE0;  // SUBB ,X      - Indexed

// SBCA - Subtract with Carry from Accumulator A
constexpr uint8_t kSBCA_IMM = 0x82;  // SBCA #imm    - Immediate
constexpr uint8_t kSBCA_DIR = 0x92;  // SBCA <dp     - Direct Page
constexpr uint8_t kSBCA_EXT = 0xB2;  // SBCA addr    - Extended (absolute)
constexpr uint8_t kSBCA_IDX = 0xA2;  // SBCA ,X      - Indexed

// SBCB - Subtract with Carry from Accumulator B
constexpr uint8_t kSBCB_IMM = 0xC2;  // SBCB #imm    - Immediate
constexpr uint8_t kSBCB_DIR = 0xD2;  // SBCB <dp     - Direct Page
constexpr uint8_t kSBCB_EXT = 0xF2;  // SBCB addr    - Extended (absolute)
constexpr uint8_t kSBCB_IDX = 0xE2;  // SBCB ,X      - Indexed

// ADDD - Add 16-bit immediate to D register
constexpr uint8_t kADDD_IMM = 0xC3;  // ADDD #imm16  - Immediate
constexpr uint8_t kADDD_DIR = 0xD3;  // ADDD <dp     - Direct Page
constexpr uint8_t kADDD_EXT = 0xF3;  // ADDD addr    - Extended (absolute)
constexpr uint8_t kADDD_IDX = 0xE3;  // ADDD ,X      - Indexed

// SUBD - Subtract 16-bit from D register
constexpr uint8_t kSUBD_IMM = 0x83;  // SUBD #imm16  - Immediate
constexpr uint8_t kSUBD_DIR = 0x93;  // SUBD <dp     - Direct Page
constexpr uint8_t kSUBD_EXT = 0xB3;  // SUBD addr    - Extended (absolute)
constexpr uint8_t kSUBD_IDX = 0xA3;  // SUBD ,X      - Indexed

// CMPD - Compare D with memory (16-bit) (page 1: prefix $10)
constexpr uint8_t kCMPD_IMM = 0x83;  // CMPD #imm16  - Immediate  (with 0x10 prefix)
constexpr uint8_t kCMPD_DIR = 0x93;  // CMPD <dp     - Direct Page (with 0x10 prefix)
constexpr uint8_t kCMPD_EXT = 0xB3;  // CMPD addr    - Extended   (with 0x10 prefix)
constexpr uint8_t kCMPD_IDX = 0xA3;  // CMPD ,X      - Indexed    (with 0x10 prefix)

// ----------------------------------------------------------------------------
// Compare Instructions
// ----------------------------------------------------------------------------

// CMPA - Compare A with memory
constexpr uint8_t kCMPA_IMM = 0x81;  // CMPA #imm    - Immediate
constexpr uint8_t kCMPA_DIR = 0x91;  // CMPA <dp     - Direct Page
constexpr uint8_t kCMPA_EXT = 0xB1;  // CMPA addr    - Extended (absolute)
constexpr uint8_t kCMPA_IDX = 0xA1;  // CMPA ,X      - Indexed

// CMPB - Compare B with memory
constexpr uint8_t kCMPB_IMM = 0xC1;  // CMPB #imm    - Immediate
constexpr uint8_t kCMPB_DIR = 0xD1;  // CMPB <dp     - Direct Page
constexpr uint8_t kCMPB_EXT = 0xF1;  // CMPB addr    - Extended (absolute)
constexpr uint8_t kCMPB_IDX = 0xE1;  // CMPB ,X      - Indexed

// CMPX - Compare X with memory (16-bit)
constexpr uint8_t kCMPX_IMM = 0x8C;  // CMPX #imm    - Immediate (big-endian)
constexpr uint8_t kCMPX_DIR = 0x9C;  // CMPX <dp     - Direct Page
constexpr uint8_t kCMPX_EXT = 0xBC;  // CMPX addr    - Extended (absolute)
constexpr uint8_t kCMPX_IDX = 0xAC;  // CMPX ,X      - Indexed

// CMPY - Compare Y with memory (Page 2: $10 prefix)
// Note: Actual opcodes are kPAGE2_PREFIX followed by same codes as CMPX

// ----------------------------------------------------------------------------
// Logical Operations
// ----------------------------------------------------------------------------

// ANDA - AND A with memory
constexpr uint8_t kANDA_IMM = 0x84;  // ANDA #imm    - Immediate
constexpr uint8_t kANDA_DIR = 0x94;  // ANDA <dp     - Direct Page
constexpr uint8_t kANDA_EXT = 0xB4;  // ANDA addr    - Extended (absolute)
constexpr uint8_t kANDA_IDX = 0xA4;  // ANDA ,X      - Indexed

// ANDB - AND B with memory
constexpr uint8_t kANDB_IMM = 0xC4;  // ANDB #imm    - Immediate
constexpr uint8_t kANDB_DIR = 0xD4;  // ANDB <dp     - Direct Page
constexpr uint8_t kANDB_EXT = 0xF4;  // ANDB addr    - Extended (absolute)
constexpr uint8_t kANDB_IDX = 0xE4;  // ANDB ,X      - Indexed

// ORA - OR A with memory
constexpr uint8_t kORA_IMM = 0x8A;  // ORA #imm     - Immediate
constexpr uint8_t kORA_DIR = 0x9A;  // ORA <dp      - Direct Page
constexpr uint8_t kORA_EXT = 0xBA;  // ORA addr     - Extended (absolute)
constexpr uint8_t kORA_IDX = 0xAA;  // ORA ,X       - Indexed

// ORB - OR B with memory
constexpr uint8_t kORB_IMM = 0xCA;  // ORB #imm     - Immediate
constexpr uint8_t kORB_DIR = 0xDA;  // ORB <dp      - Direct Page
constexpr uint8_t kORB_EXT = 0xFA;  // ORB addr     - Extended (absolute)
constexpr uint8_t kORB_IDX = 0xEA;  // ORB ,X       - Indexed

// EORA - Exclusive OR A with memory
constexpr uint8_t kEORA_IMM = 0x88;  // EORA #imm    - Immediate
constexpr uint8_t kEORA_DIR = 0x98;  // EORA <dp     - Direct Page
constexpr uint8_t kEORA_EXT = 0xB8;  // EORA addr    - Extended (absolute)
constexpr uint8_t kEORA_IDX = 0xA8;  // EORA ,X      - Indexed

// EORB - Exclusive OR B with memory
constexpr uint8_t kEORB_IMM = 0xC8;  // EORB #imm    - Immediate
constexpr uint8_t kEORB_DIR = 0xD8;  // EORB <dp     - Direct Page
constexpr uint8_t kEORB_EXT = 0xF8;  // EORB addr    - Extended (absolute)
constexpr uint8_t kEORB_IDX = 0xE8;  // EORB ,X      - Indexed

// ----------------------------------------------------------------------------
// Bit Test Operations
// ----------------------------------------------------------------------------

// BITA - Bit Test A with memory
constexpr uint8_t kBITA_IMM = 0x85;  // BITA #imm    - Immediate
constexpr uint8_t kBITA_DIR = 0x95;  // BITA <dp     - Direct Page
constexpr uint8_t kBITA_EXT = 0xB5;  // BITA addr    - Extended (absolute)
constexpr uint8_t kBITA_IDX = 0xA5;  // BITA ,X      - Indexed

// BITB - Bit Test B with memory
constexpr uint8_t kBITB_IMM = 0xC5;  // BITB #imm    - Immediate
constexpr uint8_t kBITB_DIR = 0xD5;  // BITB <dp     - Direct Page
constexpr uint8_t kBITB_EXT = 0xF5;  // BITB addr    - Extended (absolute)
constexpr uint8_t kBITB_IDX = 0xE5;  // BITB ,X      - Indexed

// ----------------------------------------------------------------------------
// Branch Instructions (8-bit relative offset)
// ----------------------------------------------------------------------------

constexpr uint8_t kBRA = 0x20;  // kBRA rel      - Branch Always
constexpr uint8_t kBRN = 0x21;  // kBRN rel      - Branch Never
constexpr uint8_t kBHI = 0x22;  // kBHI rel      - Branch if Higher (unsigned)
constexpr uint8_t kBLS = 0x23;  // kBLS rel      - Branch if Lower or Same (unsigned)
constexpr uint8_t kBCC = 0x24;  // kBCC/BHS rel  - Branch if Carry Clear
constexpr uint8_t kBCS = 0x25;  // kBCS/BLO rel  - Branch if Carry Set
constexpr uint8_t kBNE = 0x26;  // kBNE rel      - Branch if Not Equal
constexpr uint8_t kBEQ = 0x27;  // kBEQ rel      - Branch if Equal
constexpr uint8_t kBVC = 0x28;  // kBVC rel      - Branch if Overflow Clear
constexpr uint8_t kBVS = 0x29;  // kBVS rel      - Branch if Overflow Set
constexpr uint8_t kBPL = 0x2A;  // kBPL rel      - Branch if Plus
constexpr uint8_t kBMI = 0x2B;  // kBMI rel      - Branch if Minus
constexpr uint8_t kBGE = 0x2C;  // kBGE rel      - Branch if Greater or Equal (signed)
constexpr uint8_t kBLT = 0x2D;  // kBLT rel      - Branch if Less Than (signed)
constexpr uint8_t kBGT = 0x2E;  // kBGT rel      - Branch if Greater Than (signed)
constexpr uint8_t kBLE = 0x2F;  // kBLE rel      - Branch if Less or Equal (signed)
constexpr uint8_t kBSR = 0x8D;  // kBSR rel      - Branch to Subroutine

// ----------------------------------------------------------------------------
// Jump and Subroutine Instructions
// ----------------------------------------------------------------------------

constexpr uint8_t kJMP_DIR = 0x0E;  // JMP <dp      - Jump Direct
constexpr uint8_t kJMP_EXT = 0x7E;  // JMP addr     - Jump Extended
constexpr uint8_t kJMP_IDX = 0x6E;  // JMP ,X       - Jump Indexed

constexpr uint8_t kJSR_DIR = 0x9D;  // JSR <dp      - Jump to Subroutine Direct
constexpr uint8_t kJSR_EXT = 0xBD;  // JSR addr     - Jump to Subroutine Extended
constexpr uint8_t kJSR_IDX = 0xAD;  // JSR ,X       - Jump to Subroutine Indexed

// ----------------------------------------------------------------------------
// Load Effective Address Instructions
// ----------------------------------------------------------------------------

constexpr uint8_t kLEAX = 0x30;  // kLEAX ,X      - Load Effective Address into X
constexpr uint8_t kLEAY = 0x31;  // kLEAY ,X      - Load Effective Address into Y
constexpr uint8_t kLEAS = 0x32;  // kLEAS ,X      - Load Effective Address into S
constexpr uint8_t kLEAU = 0x33;  // kLEAU ,X      - Load Effective Address into U

// ----------------------------------------------------------------------------
// Stack Operations
// ----------------------------------------------------------------------------

constexpr uint8_t kPSHS = 0x34;  // kPSHS reglist - Push to System stack
constexpr uint8_t kPULS = 0x35;  // kPULS reglist - Pull from System stack
constexpr uint8_t kPSHU = 0x36;  // kPSHU reglist - Push to User stack
constexpr uint8_t kPULU = 0x37;  // kPULU reglist - Pull from User stack

// ----------------------------------------------------------------------------
// Register Transfer and Exchange
// ----------------------------------------------------------------------------

constexpr uint8_t kEXG = 0x1E;  // kEXG r1,r2    - Exchange Registers
constexpr uint8_t kTFR = 0x1F;  // kTFR r1,r2    - Transfer Register

// ----------------------------------------------------------------------------
// Condition Code Register Operations
// ----------------------------------------------------------------------------

constexpr uint8_t kANDCC = 0x1C;  // kANDCC #imm   - AND Condition Code Register
constexpr uint8_t kORCC = 0x1A;   // kORCC #imm    - OR Condition Code Register

// ----------------------------------------------------------------------------
// Inherent (no-operand) Instructions
// ----------------------------------------------------------------------------

constexpr uint8_t kMUL = 0x3D;   // kMUL          - Unsigned Multiply (A x B → D)
constexpr uint8_t kSEX = 0x1D;   // kSEX          - Sign Extend (B → A:B)
constexpr uint8_t kDAA = 0x19;   // kDAA          - Decimal Adjust A
constexpr uint8_t kRTI = 0x3B;   // kRTI          - Return from Interrupt
constexpr uint8_t kSYNC = 0x13;  // kSYNC         - Synchronize to Interrupt
constexpr uint8_t kSWI = 0x3F;   // kSWI          - Software Interrupt 1
constexpr uint8_t kCWAI = 0x3C;  // kCWAI #mask   - Clear CCs and Wait for Interrupt

// ============================================================================
// Page 2: Extended Opcodes (prefix $10)
// ============================================================================

namespace Page2 {

// Long Branch Instructions (16-bit relative offset)
constexpr uint8_t kLBRA = 0x16;  // kLBRA rel16   - Long Branch Always
constexpr uint8_t kLBSR = 0x17;  // kLBSR rel16   - Long Branch to Subroutine
constexpr uint8_t kLBRN = 0x21;  // kLBRN rel16   - Long Branch Never
constexpr uint8_t kLBHI = 0x22;  // kLBHI rel16   - Long Branch if Higher
constexpr uint8_t kLBLS = 0x23;  // kLBLS rel16   - Long Branch if Lower or Same
constexpr uint8_t kLBCC = 0x24;  // kLBCC rel16   - Long Branch if Carry Clear
constexpr uint8_t kLBCS = 0x25;  // kLBCS rel16   - Long Branch if Carry Set
constexpr uint8_t kLBNE = 0x26;  // kLBNE rel16   - Long Branch if Not Equal
constexpr uint8_t kLBEQ = 0x27;  // kLBEQ rel16   - Long Branch if Equal
constexpr uint8_t kLBVC = 0x28;  // kLBVC rel16   - Long Branch if Overflow Clear
constexpr uint8_t kLBVS = 0x29;  // kLBVS rel16   - Long Branch if Overflow Set
constexpr uint8_t kLBPL = 0x2A;  // kLBPL rel16   - Long Branch if Plus
constexpr uint8_t kLBMI = 0x2B;  // kLBMI rel16   - Long Branch if Minus
constexpr uint8_t kLBGE = 0x2C;  // kLBGE rel16   - Long Branch if Greater or Equal
constexpr uint8_t kLBLT = 0x2D;  // kLBLT rel16   - Long Branch if Less Than
constexpr uint8_t kLBGT = 0x2E;  // kLBGT rel16   - Long Branch if Greater Than
constexpr uint8_t kLBLE = 0x2F;  // kLBLE rel16   - Long Branch if Less or Equal

// Y Register Operations (same opcodes as X, but with $10 prefix)
constexpr uint8_t kLDY_IMM = 0x8E;  // LDY #imm     - Immediate (big-endian)
constexpr uint8_t kLDY_DIR = 0x9E;  // LDY <dp      - Direct Page
constexpr uint8_t kLDY_EXT = 0xBE;  // LDY addr     - Extended (absolute)
constexpr uint8_t kLDY_IDX = 0xAE;  // LDY ,X       - Indexed

constexpr uint8_t kSTY_DIR = 0x9F;  // STY <dp      - Direct Page
constexpr uint8_t kSTY_EXT = 0xBF;  // STY addr     - Extended (absolute)
constexpr uint8_t kSTY_IDX = 0xAF;  // STY ,X       - Indexed

constexpr uint8_t kCMPY_IMM = 0x8C;  // CMPY #imm    - Immediate (big-endian)
constexpr uint8_t kCMPY_DIR = 0x9C;  // CMPY <dp     - Direct Page
constexpr uint8_t kCMPY_EXT = 0xBC;  // CMPY addr    - Extended (absolute)
constexpr uint8_t kCMPY_IDX = 0xAC;  // CMPY ,X      - Indexed

// S Stack Pointer Operations (same opcodes as U, but with $10 prefix)
constexpr uint8_t kLDS_IMM = 0xCE;  // LDS #imm     - Immediate (big-endian)
constexpr uint8_t kLDS_DIR = 0xDE;  // LDS <dp      - Direct Page
constexpr uint8_t kLDS_EXT = 0xFE;  // LDS addr     - Extended (absolute)
constexpr uint8_t kLDS_IDX = 0xEE;  // LDS ,X       - Indexed

constexpr uint8_t kSTS_DIR = 0xDF;  // STS <dp      - Direct Page
constexpr uint8_t kSTS_EXT = 0xFF;  // STS addr     - Extended (absolute)
constexpr uint8_t kSTS_IDX = 0xEF;  // STS ,X       - Indexed

}  // namespace Page2

// ============================================================================
// Page 3: Extended Opcodes (prefix $11)
// ============================================================================

namespace Page3 {

// Compare U Stack Pointer with memory (16-bit)
constexpr uint8_t kCMPU_IMM = 0x83;  // CMPU #imm16  - Immediate
constexpr uint8_t kCMPU_DIR = 0x93;  // CMPU <dp     - Direct Page
constexpr uint8_t kCMPU_EXT = 0xB3;  // CMPU addr    - Extended (absolute)
constexpr uint8_t kCMPU_IDX = 0xA3;  // CMPU ,X      - Indexed

// Compare S Stack Pointer with memory (16-bit)
constexpr uint8_t kCMPS_IMM = 0x8C;  // CMPS #imm16  - Immediate
constexpr uint8_t kCMPS_DIR = 0x9C;  // CMPS <dp     - Direct Page
constexpr uint8_t kCMPS_EXT = 0xBC;  // CMPS addr    - Extended (absolute)
constexpr uint8_t kCMPS_IDX = 0xAC;  // CMPS ,X      - Indexed

// kSWI3 - Software Interrupt 3
constexpr uint8_t kSWI3 = 0x3F;  // kSWI3         - Software Interrupt 3 (with 0x11 prefix)

}  // namespace Page3

// ============================================================================
// Indexed Addressing Post-Byte Encoding
// ============================================================================

namespace Indexed {

// Post-byte bits for indexed addressing modes
// Format: [I RRRR AAAA]
//   I    = Indirect flag (bit 7)
//   RRRR = Register select (bits 5-6)
//   AAAA = Addressing mode (bits 0-4)

// Register Select Bits (bits 5-6 of post-byte)
constexpr uint8_t kREG_X = 0x00;  // 00 = X register
constexpr uint8_t kREG_Y = 0x20;  // 01 = Y register
constexpr uint8_t kREG_U = 0x40;  // 10 = U register
constexpr uint8_t kREG_S = 0x60;  // 11 = S register

// Indexed Mode Encoding (bits 0-4 of post-byte)
constexpr uint8_t kMODE_CONSTANT_OFFSET_5BIT = 0x00;   // ,R + 5-bit offset (0-31)
constexpr uint8_t kMODE_CONSTANT_OFFSET_8BIT = 0x88;   // ,R + 8-bit offset
constexpr uint8_t kMODE_CONSTANT_OFFSET_16BIT = 0x89;  // ,R + 16-bit offset
constexpr uint8_t kMODE_ACCUMULATOR_A = 0x86;          // A,R
constexpr uint8_t kMODE_ACCUMULATOR_B = 0x85;          // B,R
constexpr uint8_t kMODE_ACCUMULATOR_D = 0x8B;          // D,R
constexpr uint8_t kMODE_AUTO_INCREMENT_1 = 0x80;       // ,R+
constexpr uint8_t kMODE_AUTO_INCREMENT_2 = 0x81;       // ,R++
constexpr uint8_t kMODE_AUTO_DECREMENT_1 = 0x82;       // ,-R
constexpr uint8_t kMODE_AUTO_DECREMENT_2 = 0x83;       // ,--R
constexpr uint8_t kMODE_ZERO_OFFSET = 0x84;            // ,R (no offset)
constexpr uint8_t kMODE_PC_RELATIVE_8BIT = 0x8C;       // n,PCR (8-bit)
constexpr uint8_t kMODE_PC_RELATIVE_16BIT = 0x8D;      // n,PCR (16-bit)
constexpr uint8_t kMODE_EXTENDED_INDIRECT = 0x9F;      // [$addr]

// Indirect flag (OR with above modes for indirect addressing)
constexpr uint8_t kINDIRECT_FLAG = 0x10;  // [ ,R ] indirect

}  // namespace Indexed

// ============================================================================
// Register Encoding (for kTFR/kEXG instructions)
// ============================================================================

namespace Registers {

// Register codes for kTFR and kEXG instructions (4-bit codes)
constexpr uint8_t kD = 0x0;   // D register (A:B concatenated, 16-bit)
constexpr uint8_t kX = 0x1;   // X register (16-bit)
constexpr uint8_t kY = 0x2;   // Y register (16-bit)
constexpr uint8_t kU = 0x3;   // U stack pointer (16-bit)
constexpr uint8_t kS = 0x4;   // S stack pointer (16-bit)
constexpr uint8_t kPC = 0x5;  // kPC program counter (16-bit)
constexpr uint8_t kA = 0x8;   // A accumulator (8-bit)
constexpr uint8_t kB = 0x9;   // B accumulator (8-bit)
constexpr uint8_t kCC = 0xA;  // kCC condition codes (8-bit)
constexpr uint8_t kDP = 0xB;  // kDP direct page register (8-bit)

}  // namespace Registers

// ============================================================================
// Stack Register Mask Bits (for kPSHS/kPULS/kPSHU/kPULU)
// ============================================================================

namespace StackMask {

// Register mask bits for kPSHS/kPULS/kPSHU/kPULU instructions
constexpr uint8_t kCC = 0x01;      // Bit 0: kCC (Condition Codes)
constexpr uint8_t kA = 0x02;       // Bit 1: A
constexpr uint8_t kB = 0x04;       // Bit 2: B
constexpr uint8_t kDP = 0x08;      // Bit 3: kDP (Direct Page)
constexpr uint8_t kX = 0x10;       // Bit 4: X
constexpr uint8_t kY = 0x20;       // Bit 5: Y
constexpr uint8_t kU_OR_S = 0x40;  // Bit 6: U (for kPSHS/kPULS) or S (for kPSHU/kPULU)
constexpr uint8_t kPC = 0x80;      // Bit 7: kPC (Program Counter)

}  // namespace StackMask

}  // namespace Opcodes6809
}  // namespace xasm
