/**
 * @file mnemonics_6809.h
 * @brief Mnemonic string constants for Motorola 6809 instructions
 *
 * Eliminates magic strings from 6809 CPU instruction encoding.
 * Part of magic string elimination initiative (Phase 6b).
 *
 * @section usage Usage Example
 * @code
 * // Instead of:
 * if (mnemonic == "LDA") { ... }  // Magic string
 *
 * // Use:
 * if (mnemonic == M6809Mnemonics::LDA) { ... }  // Named constant
 * @endcode
 */

#pragma once

namespace xasm {
namespace M6809Mnemonics {

// ============================================================================
// Data Movement Instructions - Load
// ============================================================================

/// LDA - Load Accumulator A
constexpr const char *LDA = "LDA";

/// LDB - Load Accumulator B
constexpr const char *LDB = "LDB";

/// LDD - Load Accumulator D (16-bit)
constexpr const char *LDD = "LDD";

/// LDX - Load Index Register X
constexpr const char *LDX = "LDX";

/// LDY - Load Index Register Y
constexpr const char *LDY = "LDY";

/// LDU - Load User Stack Pointer
constexpr const char *LDU = "LDU";

/// LDS - Load System Stack Pointer
constexpr const char *LDS = "LDS";

// ============================================================================
// Data Movement Instructions - Store
// ============================================================================

/// STA - Store Accumulator A
constexpr const char *STA = "STA";

/// STB - Store Accumulator B
constexpr const char *STB = "STB";

/// STD - Store Accumulator D (16-bit)
constexpr const char *STD = "STD";

/// STX - Store Index Register X
constexpr const char *STX = "STX";

/// STY - Store Index Register Y
constexpr const char *STY = "STY";

// ============================================================================
// Arithmetic Instructions
// ============================================================================

/// ADDA - Add to Accumulator A
constexpr const char *ADDA = "ADDA";

/// ADDB - Add to Accumulator B
constexpr const char *ADDB = "ADDB";

/// SUBA - Subtract from Accumulator A
constexpr const char *SUBA = "SUBA";

/// SUBB - Subtract from Accumulator B
constexpr const char *SUBB = "SUBB";

/// INCA - Increment Accumulator A
constexpr const char *INCA = "INCA";

/// INCB - Increment Accumulator B
constexpr const char *INCB = "INCB";

/// DECA - Decrement Accumulator A
constexpr const char *DECA = "DECA";

/// DECB - Decrement Accumulator B
constexpr const char *DECB = "DECB";

// ============================================================================
// Logical Operations
// ============================================================================

/// ANDA - AND with Accumulator A
constexpr const char *ANDA = "ANDA";

/// ANDB - AND with Accumulator B
constexpr const char *ANDB = "ANDB";

/// ORA - OR with Accumulator A
constexpr const char *ORA = "ORA";

/// ORB - OR with Accumulator B
constexpr const char *ORB = "ORB";

/// EORA - Exclusive OR with Accumulator A
constexpr const char *EORA = "EORA";

/// EORB - Exclusive OR with Accumulator B
constexpr const char *EORB = "EORB";

/// BITA - Bit Test Accumulator A
constexpr const char *BITA = "BITA";

/// BITB - Bit Test Accumulator B
constexpr const char *BITB = "BITB";

// ============================================================================
// Comparison Instructions
// ============================================================================

/// CMPA - Compare Accumulator A
constexpr const char *CMPA = "CMPA";

/// CMPB - Compare Accumulator B
constexpr const char *CMPB = "CMPB";

/// CMPX - Compare Index Register X
constexpr const char *CMPX = "CMPX";

/// CMPY - Compare Index Register Y
constexpr const char *CMPY = "CMPY";

/// CMPU - Compare User Stack Pointer
constexpr const char *CMPU = "CMPU";

/// CMPS - Compare System Stack Pointer
constexpr const char *CMPS = "CMPS";

// ============================================================================
// Shift and Rotate Instructions
// ============================================================================

/// ASLA - Arithmetic Shift Left Accumulator A
constexpr const char *ASLA = "ASLA";

/// ASLB - Arithmetic Shift Left Accumulator B
constexpr const char *ASLB = "ASLB";

/// ASRA - Arithmetic Shift Right Accumulator A
constexpr const char *ASRA = "ASRA";

/// ASRB - Arithmetic Shift Right Accumulator B
constexpr const char *ASRB = "ASRB";

/// LSRA - Logical Shift Right Accumulator A
constexpr const char *LSRA = "LSRA";

/// LSRB - Logical Shift Right Accumulator B
constexpr const char *LSRB = "LSRB";

/// ROLA - Rotate Left Accumulator A
constexpr const char *ROLA = "ROLA";

/// ROLB - Rotate Left Accumulator B
constexpr const char *ROLB = "ROLB";

/// RORA - Rotate Right Accumulator A
constexpr const char *RORA = "RORA";

/// RORB - Rotate Right Accumulator B
constexpr const char *RORB = "RORB";

// ============================================================================
// Test/Clear/Complement/Negate Instructions
// ============================================================================

/// TSTA - Test Accumulator A
constexpr const char *TSTA = "TSTA";

/// TSTB - Test Accumulator B
constexpr const char *TSTB = "TSTB";

/// CLRA - Clear Accumulator A
constexpr const char *CLRA = "CLRA";

/// CLRB - Clear Accumulator B
constexpr const char *CLRB = "CLRB";

/// COMA - Complement Accumulator A
constexpr const char *COMA = "COMA";

/// COMB - Complement Accumulator B
constexpr const char *COMB = "COMB";

/// NEGA - Negate Accumulator A
constexpr const char *NEGA = "NEGA";

/// NEGB - Negate Accumulator B
constexpr const char *NEGB = "NEGB";

// ============================================================================
// Short Branch Instructions (8-bit relative)
// ============================================================================

/// BRA - Branch Always
constexpr const char *BRA = "BRA";

/// BEQ - Branch if Equal
constexpr const char *BEQ = "BEQ";

/// BNE - Branch if Not Equal
constexpr const char *BNE = "BNE";

/// BCC - Branch if Carry Clear
constexpr const char *BCC = "BCC";

/// BHS - Branch if Higher or Same (alias for BCC)
constexpr const char *BHS = "BHS";

/// BCS - Branch if Carry Set
constexpr const char *BCS = "BCS";

/// BLO - Branch if Lower (alias for BCS)
constexpr const char *BLO = "BLO";

/// BMI - Branch if Minus
constexpr const char *BMI = "BMI";

/// BPL - Branch if Plus
constexpr const char *BPL = "BPL";

/// BVS - Branch if Overflow Set
constexpr const char *BVS = "BVS";

/// BVC - Branch if Overflow Clear
constexpr const char *BVC = "BVC";

/// BGE - Branch if Greater or Equal (signed)
constexpr const char *BGE = "BGE";

/// BLT - Branch if Less Than (signed)
constexpr const char *BLT = "BLT";

/// BGT - Branch if Greater Than (signed)
constexpr const char *BGT = "BGT";

/// BLE - Branch if Less or Equal (signed)
constexpr const char *BLE = "BLE";

/// BHI - Branch if Higher (unsigned)
constexpr const char *BHI = "BHI";

/// BLS - Branch if Lower or Same (unsigned)
constexpr const char *BLS = "BLS";

/// BSR - Branch to Subroutine
constexpr const char *BSR = "BSR";

// ============================================================================
// Long Branch Instructions (16-bit relative)
// ============================================================================

/// LBRA - Long Branch Always
constexpr const char *LBRA = "LBRA";

/// LBSR - Long Branch to Subroutine
constexpr const char *LBSR = "LBSR";

/// LBRN - Long Branch Never
constexpr const char *LBRN = "LBRN";

/// LBHI - Long Branch if Higher (unsigned)
constexpr const char *LBHI = "LBHI";

/// LBLS - Long Branch if Lower or Same (unsigned)
constexpr const char *LBLS = "LBLS";

/// LBCC - Long Branch if Carry Clear
constexpr const char *LBCC = "LBCC";

/// LBHS - Long Branch if Higher or Same (alias for LBCC)
constexpr const char *LBHS = "LBHS";

/// LBCS - Long Branch if Carry Set
constexpr const char *LBCS = "LBCS";

/// LBLO - Long Branch if Lower (alias for LBCS)
constexpr const char *LBLO = "LBLO";

/// LBNE - Long Branch if Not Equal
constexpr const char *LBNE = "LBNE";

/// LBEQ - Long Branch if Equal
constexpr const char *LBEQ = "LBEQ";

/// LBVC - Long Branch if Overflow Clear
constexpr const char *LBVC = "LBVC";

/// LBVS - Long Branch if Overflow Set
constexpr const char *LBVS = "LBVS";

/// LBPL - Long Branch if Plus
constexpr const char *LBPL = "LBPL";

/// LBMI - Long Branch if Minus
constexpr const char *LBMI = "LBMI";

/// LBGE - Long Branch if Greater or Equal (signed)
constexpr const char *LBGE = "LBGE";

/// LBLT - Long Branch if Less Than (signed)
constexpr const char *LBLT = "LBLT";

/// LBGT - Long Branch if Greater Than (signed)
constexpr const char *LBGT = "LBGT";

/// LBLE - Long Branch if Less or Equal (signed)
constexpr const char *LBLE = "LBLE";

// ============================================================================
// Control Flow Instructions
// ============================================================================

/// JMP - Jump
constexpr const char *JMP = "JMP";

/// JSR - Jump to Subroutine
constexpr const char *JSR = "JSR";

/// RTS - Return from Subroutine
constexpr const char *RTS = "RTS";

/// NOP - No Operation
constexpr const char *NOP = "NOP";

// ============================================================================
// Load Effective Address Instructions
// ============================================================================

/// LEAX - Load Effective Address into X
constexpr const char *LEAX = "LEAX";

/// LEAY - Load Effective Address into Y
constexpr const char *LEAY = "LEAY";

// ============================================================================
// Stack Operations
// ============================================================================

/// PSHS - Push registers onto System Stack
constexpr const char *PSHS = "PSHS";

/// PULS - Pull registers from System Stack
constexpr const char *PULS = "PULS";

/// PSHU - Push registers onto User Stack
constexpr const char *PSHU = "PSHU";

/// PULU - Pull registers from User Stack
constexpr const char *PULU = "PULU";

// ============================================================================
// Register Transfer and Exchange
// ============================================================================

/// TFR - Transfer between registers
constexpr const char *TFR = "TFR";

/// EXG - Exchange registers
constexpr const char *EXG = "EXG";

} // namespace M6809Mnemonics
} // namespace xasm
