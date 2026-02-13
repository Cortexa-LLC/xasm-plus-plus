// Motorola 6809 Opcode Constants
// Replaces magic numbers with descriptive named constants
//
// The 6809 uses a 3-page opcode space:
// - Page 1: Standard opcodes (direct encoding)
// - Page 2: Extended opcodes (prefix $10)
// - Page 3: Extended opcodes (prefix $11)

#pragma once

#include <cstdint>

namespace xasm {

// ============================================================================
// Motorola 6809 Mnemonic String Constants
// ============================================================================
// Eliminates magic strings code smell by providing named constants for
// instruction mnemonics used in string comparisons throughout parsers.
namespace M6809Mnemonics {

// Load/Store Instructions
constexpr const char *LDA = "LDA";
constexpr const char *LDB = "LDB";
constexpr const char *LDD = "LDD";
constexpr const char *LDX = "LDX";
constexpr const char *LDY = "LDY";
constexpr const char *LDU = "LDU";
constexpr const char *LDS = "LDS";
constexpr const char *STA = "STA";
constexpr const char *STB = "STB";
constexpr const char *STD = "STD";
constexpr const char *STX = "STX";
constexpr const char *STY = "STY";
constexpr const char *STU = "STU";
constexpr const char *STS = "STS";

// Arithmetic Instructions
constexpr const char *ADDA = "ADDA";
constexpr const char *ADDB = "ADDB";
constexpr const char *ADDD = "ADDD";
constexpr const char *SUBA = "SUBA";
constexpr const char *SUBB = "SUBB";
constexpr const char *SUBD = "SUBD";
constexpr const char *INCA = "INCA";
constexpr const char *INCB = "INCB";
constexpr const char *INC = "INC";
constexpr const char *DECA = "DECA";
constexpr const char *DECB = "DECB";
constexpr const char *DEC = "DEC";
constexpr const char *NEGA = "NEGA";
constexpr const char *NEGB = "NEGB";
constexpr const char *NEG = "NEG";

// Logical Instructions
constexpr const char *ANDA = "ANDA";
constexpr const char *ANDB = "ANDB";
constexpr const char *ORA = "ORA";
constexpr const char *ORB = "ORB";
constexpr const char *EORA = "EORA";
constexpr const char *EORB = "EORB";
constexpr const char *BITA = "BITA";
constexpr const char *BITB = "BITB";
constexpr const char *COMA = "COMA";
constexpr const char *COMB = "COMB";
constexpr const char *COM = "COM";

// Compare Instructions
constexpr const char *CMPA = "CMPA";
constexpr const char *CMPB = "CMPB";
constexpr const char *CMPD = "CMPD";
constexpr const char *CMPX = "CMPX";
constexpr const char *CMPY = "CMPY";
constexpr const char *CMPU = "CMPU";
constexpr const char *CMPS = "CMPS";

// Shift/Rotate Instructions
constexpr const char *ASLA = "ASLA";
constexpr const char *ASLB = "ASLB";
constexpr const char *ASL = "ASL";
constexpr const char *ASRA = "ASRA";
constexpr const char *ASRB = "ASRB";
constexpr const char *ASR = "ASR";
constexpr const char *LSRA = "LSRA";
constexpr const char *LSRB = "LSRB";
constexpr const char *LSR = "LSR";
constexpr const char *ROLA = "ROLA";
constexpr const char *ROLB = "ROLB";
constexpr const char *ROL = "ROL";
constexpr const char *RORA = "RORA";
constexpr const char *RORB = "RORB";
constexpr const char *ROR = "ROR";

// Test/Clear Instructions
constexpr const char *TSTA = "TSTA";
constexpr const char *TSTB = "TSTB";
constexpr const char *TST = "TST";
constexpr const char *CLRA = "CLRA";
constexpr const char *CLRB = "CLRB";
constexpr const char *CLR = "CLR";

// Branch Instructions
constexpr const char *BRA = "BRA";
constexpr const char *BRN = "BRN";
constexpr const char *BHI = "BHI";
constexpr const char *BLS = "BLS";
constexpr const char *BCC = "BCC";
constexpr const char *BHS = "BHS"; // Alternate for BCC
constexpr const char *BCS = "BCS";
constexpr const char *BLO = "BLO"; // Alternate for BCS
constexpr const char *BNE = "BNE";
constexpr const char *BEQ = "BEQ";
constexpr const char *BVC = "BVC";
constexpr const char *BVS = "BVS";
constexpr const char *BPL = "BPL";
constexpr const char *BMI = "BMI";
constexpr const char *BGE = "BGE";
constexpr const char *BLT = "BLT";
constexpr const char *BGT = "BGT";
constexpr const char *BLE = "BLE";
constexpr const char *BSR = "BSR";

// Long Branch Instructions (16-bit offset)
constexpr const char *LBRA = "LBRA";
constexpr const char *LBRN = "LBRN";
constexpr const char *LBHI = "LBHI";
constexpr const char *LBLS = "LBLS";
constexpr const char *LBCC = "LBCC";
constexpr const char *LBHS = "LBHS"; // Alternate for LBCC
constexpr const char *LBCS = "LBCS";
constexpr const char *LBLO = "LBLO"; // Alternate for LBCS
constexpr const char *LBNE = "LBNE";
constexpr const char *LBEQ = "LBEQ";
constexpr const char *LBVC = "LBVC";
constexpr const char *LBVS = "LBVS";
constexpr const char *LBPL = "LBPL";
constexpr const char *LBMI = "LBMI";
constexpr const char *LBGE = "LBGE";
constexpr const char *LBLT = "LBLT";
constexpr const char *LBGT = "LBGT";
constexpr const char *LBLE = "LBLE";

// Jump/Subroutine Instructions
constexpr const char *JMP = "JMP";
constexpr const char *JSR = "JSR";
constexpr const char *RTS = "RTS";

// Load Effective Address Instructions
constexpr const char *LEAX = "LEAX";
constexpr const char *LEAY = "LEAY";
constexpr const char *LEAS = "LEAS";
constexpr const char *LEAU = "LEAU";

// Stack Instructions
constexpr const char *PSHS = "PSHS";
constexpr const char *PULS = "PULS";
constexpr const char *PSHU = "PSHU";
constexpr const char *PULU = "PULU";

// Register Transfer/Exchange Instructions
constexpr const char *TFR = "TFR";
constexpr const char *EXG = "EXG";

// Special Instructions
constexpr const char *NOP = "NOP";
constexpr const char *SWI = "SWI";
constexpr const char *SWI2 = "SWI2";
constexpr const char *SWI3 = "SWI3";
constexpr const char *CWAI = "CWAI";
constexpr const char *SYNC = "SYNC";
constexpr const char *RTI = "RTI";

// Multiply Instructions
constexpr const char *MUL = "MUL";

// Sign Extend Instructions
constexpr const char *SEX = "SEX";

// Decimal Adjust Accumulator
constexpr const char *DAA = "DAA";

// Alternate Mnemonics (for compatibility)
constexpr const char *ASLD = "ASLD"; // Same as LSLD
constexpr const char *LSLD = "LSLD";
constexpr const char *CLRD = "CLRD";
constexpr const char *TSTD = "TSTD";

} // namespace M6809Mnemonics

namespace Opcodes6809 {

// ============================================================================
// Opcode Page Prefixes
// ============================================================================

constexpr uint8_t PAGE2_PREFIX = 0x10; // Page 2 prefix for extended opcodes
constexpr uint8_t PAGE3_PREFIX = 0x11; // Page 3 prefix for extended opcodes

// ============================================================================
// Page 1: Standard Opcodes
// ============================================================================

// ----------------------------------------------------------------------------
// Inherent Mode Instructions (no operand)
// ----------------------------------------------------------------------------

constexpr uint8_t NOP = 0x12;  // NOP          - No Operation
constexpr uint8_t RTS = 0x39;  // RTS          - Return from Subroutine
constexpr uint8_t CLRA = 0x4F; // CLRA         - Clear Accumulator A
constexpr uint8_t CLRB = 0x5F; // CLRB         - Clear Accumulator B

// Shift and Rotate Instructions (Inherent on A/B)
constexpr uint8_t ASLA = 0x48; // ASLA         - Arithmetic Shift Left A
constexpr uint8_t ASLB = 0x58; // ASLB         - Arithmetic Shift Left B
constexpr uint8_t ASRA = 0x47; // ASRA         - Arithmetic Shift Right A
constexpr uint8_t ASRB = 0x57; // ASRB         - Arithmetic Shift Right B
constexpr uint8_t LSRA = 0x44; // LSRA         - Logical Shift Right A
constexpr uint8_t LSRB = 0x54; // LSRB         - Logical Shift Right B
constexpr uint8_t ROLA = 0x49; // ROLA         - Rotate Left A through Carry
constexpr uint8_t ROLB = 0x59; // ROLB         - Rotate Left B through Carry
constexpr uint8_t RORA = 0x46; // RORA         - Rotate Right A through Carry
constexpr uint8_t RORB = 0x56; // RORB         - Rotate Right B through Carry

// Increment/Decrement Instructions (Inherent on A/B)
constexpr uint8_t INCA = 0x4C; // INCA         - Increment A
constexpr uint8_t INCB = 0x5C; // INCB         - Increment B
constexpr uint8_t DECA = 0x4A; // DECA         - Decrement A
constexpr uint8_t DECB = 0x5A; // DECB         - Decrement B

// Test/Compare/Negate Instructions (Inherent on A/B)
constexpr uint8_t TSTA = 0x4D; // TSTA         - Test A (set flags)
constexpr uint8_t TSTB = 0x5D; // TSTB         - Test B (set flags)
constexpr uint8_t COMA = 0x43; // COMA         - Complement A (one's complement)
constexpr uint8_t COMB = 0x53; // COMB         - Complement B (one's complement)
constexpr uint8_t NEGA = 0x40; // NEGA         - Negate A (two's complement)
constexpr uint8_t NEGB = 0x50; // NEGB         - Negate B (two's complement)

// ----------------------------------------------------------------------------
// Load/Store Instructions - 8-bit Accumulators (A, B)
// ----------------------------------------------------------------------------

// LDA - Load Accumulator A
constexpr uint8_t LDA_IMM = 0x86; // LDA #imm     - Immediate
constexpr uint8_t LDA_DIR = 0x96; // LDA <dp      - Direct Page
constexpr uint8_t LDA_EXT = 0xB6; // LDA addr     - Extended (absolute)
constexpr uint8_t LDA_IDX = 0xA6; // LDA ,X       - Indexed

// LDB - Load Accumulator B
constexpr uint8_t LDB_IMM = 0xC6; // LDB #imm     - Immediate
constexpr uint8_t LDB_DIR = 0xD6; // LDB <dp      - Direct Page
constexpr uint8_t LDB_EXT = 0xF6; // LDB addr     - Extended (absolute)
constexpr uint8_t LDB_IDX = 0xE6; // LDB ,X       - Indexed

// STA - Store Accumulator A
constexpr uint8_t STA_DIR = 0x97; // STA <dp      - Direct Page
constexpr uint8_t STA_EXT = 0xB7; // STA addr     - Extended (absolute)
constexpr uint8_t STA_IDX = 0xA7; // STA ,X       - Indexed

// STB - Store Accumulator B
constexpr uint8_t STB_DIR = 0xD7; // STB <dp      - Direct Page
constexpr uint8_t STB_EXT = 0xF7; // STB addr     - Extended (absolute)
constexpr uint8_t STB_IDX = 0xE7; // STB ,X       - Indexed

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit D Register (A:B concatenated)
// ----------------------------------------------------------------------------

// LDD - Load Accumulator D (16-bit: A=high, B=low)
constexpr uint8_t LDD_IMM = 0xCC; // LDD #imm     - Immediate (big-endian)
constexpr uint8_t LDD_DIR = 0xDC; // LDD <dp      - Direct Page
constexpr uint8_t LDD_EXT = 0xFC; // LDD addr     - Extended (absolute)
constexpr uint8_t LDD_IDX = 0xEC; // LDD ,X       - Indexed

// STD - Store Accumulator D (16-bit)
constexpr uint8_t STD_DIR = 0xDD; // STD <dp      - Direct Page
constexpr uint8_t STD_EXT = 0xFD; // STD addr     - Extended (absolute)
constexpr uint8_t STD_IDX = 0xED; // STD ,X       - Indexed

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit Index Registers (X, Y)
// ----------------------------------------------------------------------------

// LDX - Load Index Register X
constexpr uint8_t LDX_IMM = 0x8E; // LDX #imm     - Immediate (big-endian)
constexpr uint8_t LDX_DIR = 0x9E; // LDX <dp      - Direct Page
constexpr uint8_t LDX_EXT = 0xBE; // LDX addr     - Extended (absolute)
constexpr uint8_t LDX_IDX = 0xAE; // LDX ,X       - Indexed

// STX - Store Index Register X
constexpr uint8_t STX_DIR = 0x9F; // STX <dp      - Direct Page
constexpr uint8_t STX_EXT = 0xBF; // STX addr     - Extended (absolute)
constexpr uint8_t STX_IDX = 0xAF; // STX ,X       - Indexed

// LDY - Load Index Register Y (Page 2: $10 prefix)
// Note: Actual opcodes are PAGE2_PREFIX followed by same codes as LDX

// STY - Store Index Register Y (Page 2: $10 prefix)
// Note: Actual opcodes are PAGE2_PREFIX followed by same codes as STX

// ----------------------------------------------------------------------------
// Load/Store Instructions - 16-bit Stack Pointers (U, S)
// ----------------------------------------------------------------------------

// LDU - Load User Stack Pointer
constexpr uint8_t LDU_IMM = 0xCE; // LDU #imm     - Immediate (big-endian)
constexpr uint8_t LDU_DIR = 0xDE; // LDU <dp      - Direct Page
constexpr uint8_t LDU_EXT = 0xFE; // LDU addr     - Extended (absolute)
constexpr uint8_t LDU_IDX = 0xEE; // LDU ,X       - Indexed

// STU - Store User Stack Pointer
constexpr uint8_t STU_DIR = 0xDF; // STU <dp      - Direct Page
constexpr uint8_t STU_EXT = 0xFF; // STU addr     - Extended (absolute)
constexpr uint8_t STU_IDX = 0xEF; // STU ,X       - Indexed

// LDS - Load System Stack Pointer (Page 2: $10 prefix)
// Note: Actual opcodes are PAGE2_PREFIX followed by same codes as LDU

// STS - Store System Stack Pointer (Page 2: $10 prefix)
// Note: Actual opcodes are PAGE2_PREFIX followed by same codes as STU

// ----------------------------------------------------------------------------
// Arithmetic Instructions
// ----------------------------------------------------------------------------

// ADDA - Add to Accumulator A
constexpr uint8_t ADDA_IMM = 0x8B; // ADDA #imm    - Immediate
constexpr uint8_t ADDA_DIR = 0x9B; // ADDA <dp     - Direct Page
constexpr uint8_t ADDA_EXT = 0xBB; // ADDA addr    - Extended (absolute)
constexpr uint8_t ADDA_IDX = 0xAB; // ADDA ,X      - Indexed

// ADDB - Add to Accumulator B
constexpr uint8_t ADDB_IMM = 0xCB; // ADDB #imm    - Immediate
constexpr uint8_t ADDB_DIR = 0xDB; // ADDB <dp     - Direct Page
constexpr uint8_t ADDB_EXT = 0xFB; // ADDB addr    - Extended (absolute)
constexpr uint8_t ADDB_IDX = 0xEB; // ADDB ,X      - Indexed

// SUBA - Subtract from Accumulator A
constexpr uint8_t SUBA_IMM = 0x80; // SUBA #imm    - Immediate
constexpr uint8_t SUBA_DIR = 0x90; // SUBA <dp     - Direct Page
constexpr uint8_t SUBA_EXT = 0xB0; // SUBA addr    - Extended (absolute)
constexpr uint8_t SUBA_IDX = 0xA0; // SUBA ,X      - Indexed

// SUBB - Subtract from Accumulator B
constexpr uint8_t SUBB_IMM = 0xC0; // SUBB #imm    - Immediate
constexpr uint8_t SUBB_DIR = 0xD0; // SUBB <dp     - Direct Page
constexpr uint8_t SUBB_EXT = 0xF0; // SUBB addr    - Extended (absolute)
constexpr uint8_t SUBB_IDX = 0xE0; // SUBB ,X      - Indexed

// ----------------------------------------------------------------------------
// Compare Instructions
// ----------------------------------------------------------------------------

// CMPA - Compare A with memory
constexpr uint8_t CMPA_IMM = 0x81; // CMPA #imm    - Immediate
constexpr uint8_t CMPA_DIR = 0x91; // CMPA <dp     - Direct Page
constexpr uint8_t CMPA_EXT = 0xB1; // CMPA addr    - Extended (absolute)
constexpr uint8_t CMPA_IDX = 0xA1; // CMPA ,X      - Indexed

// CMPB - Compare B with memory
constexpr uint8_t CMPB_IMM = 0xC1; // CMPB #imm    - Immediate
constexpr uint8_t CMPB_DIR = 0xD1; // CMPB <dp     - Direct Page
constexpr uint8_t CMPB_EXT = 0xF1; // CMPB addr    - Extended (absolute)
constexpr uint8_t CMPB_IDX = 0xE1; // CMPB ,X      - Indexed

// CMPX - Compare X with memory (16-bit)
constexpr uint8_t CMPX_IMM = 0x8C; // CMPX #imm    - Immediate (big-endian)
constexpr uint8_t CMPX_DIR = 0x9C; // CMPX <dp     - Direct Page
constexpr uint8_t CMPX_EXT = 0xBC; // CMPX addr    - Extended (absolute)
constexpr uint8_t CMPX_IDX = 0xAC; // CMPX ,X      - Indexed

// CMPY - Compare Y with memory (Page 2: $10 prefix)
// Note: Actual opcodes are PAGE2_PREFIX followed by same codes as CMPX

// ----------------------------------------------------------------------------
// Logical Operations
// ----------------------------------------------------------------------------

// ANDA - AND A with memory
constexpr uint8_t ANDA_IMM = 0x84; // ANDA #imm    - Immediate
constexpr uint8_t ANDA_DIR = 0x94; // ANDA <dp     - Direct Page
constexpr uint8_t ANDA_EXT = 0xB4; // ANDA addr    - Extended (absolute)
constexpr uint8_t ANDA_IDX = 0xA4; // ANDA ,X      - Indexed

// ANDB - AND B with memory
constexpr uint8_t ANDB_IMM = 0xC4; // ANDB #imm    - Immediate
constexpr uint8_t ANDB_DIR = 0xD4; // ANDB <dp     - Direct Page
constexpr uint8_t ANDB_EXT = 0xF4; // ANDB addr    - Extended (absolute)
constexpr uint8_t ANDB_IDX = 0xE4; // ANDB ,X      - Indexed

// ORA - OR A with memory
constexpr uint8_t ORA_IMM = 0x8A; // ORA #imm     - Immediate
constexpr uint8_t ORA_DIR = 0x9A; // ORA <dp      - Direct Page
constexpr uint8_t ORA_EXT = 0xBA; // ORA addr     - Extended (absolute)
constexpr uint8_t ORA_IDX = 0xAA; // ORA ,X       - Indexed

// ORB - OR B with memory
constexpr uint8_t ORB_IMM = 0xCA; // ORB #imm     - Immediate
constexpr uint8_t ORB_DIR = 0xDA; // ORB <dp      - Direct Page
constexpr uint8_t ORB_EXT = 0xFA; // ORB addr     - Extended (absolute)
constexpr uint8_t ORB_IDX = 0xEA; // ORB ,X       - Indexed

// EORA - Exclusive OR A with memory
constexpr uint8_t EORA_IMM = 0x88; // EORA #imm    - Immediate
constexpr uint8_t EORA_DIR = 0x98; // EORA <dp     - Direct Page
constexpr uint8_t EORA_EXT = 0xB8; // EORA addr    - Extended (absolute)
constexpr uint8_t EORA_IDX = 0xA8; // EORA ,X      - Indexed

// EORB - Exclusive OR B with memory
constexpr uint8_t EORB_IMM = 0xC8; // EORB #imm    - Immediate
constexpr uint8_t EORB_DIR = 0xD8; // EORB <dp     - Direct Page
constexpr uint8_t EORB_EXT = 0xF8; // EORB addr    - Extended (absolute)
constexpr uint8_t EORB_IDX = 0xE8; // EORB ,X      - Indexed

// ----------------------------------------------------------------------------
// Bit Test Operations
// ----------------------------------------------------------------------------

// BITA - Bit Test A with memory
constexpr uint8_t BITA_IMM = 0x85; // BITA #imm    - Immediate
constexpr uint8_t BITA_DIR = 0x95; // BITA <dp     - Direct Page
constexpr uint8_t BITA_EXT = 0xB5; // BITA addr    - Extended (absolute)
constexpr uint8_t BITA_IDX = 0xA5; // BITA ,X      - Indexed

// BITB - Bit Test B with memory
constexpr uint8_t BITB_IMM = 0xC5; // BITB #imm    - Immediate
constexpr uint8_t BITB_DIR = 0xD5; // BITB <dp     - Direct Page
constexpr uint8_t BITB_EXT = 0xF5; // BITB addr    - Extended (absolute)
constexpr uint8_t BITB_IDX = 0xE5; // BITB ,X      - Indexed

// ----------------------------------------------------------------------------
// Branch Instructions (8-bit relative offset)
// ----------------------------------------------------------------------------

constexpr uint8_t BRA = 0x20; // BRA rel      - Branch Always
constexpr uint8_t BRN = 0x21; // BRN rel      - Branch Never
constexpr uint8_t BHI = 0x22; // BHI rel      - Branch if Higher (unsigned)
constexpr uint8_t BLS =
    0x23; // BLS rel      - Branch if Lower or Same (unsigned)
constexpr uint8_t BCC = 0x24; // BCC/BHS rel  - Branch if Carry Clear
constexpr uint8_t BCS = 0x25; // BCS/BLO rel  - Branch if Carry Set
constexpr uint8_t BNE = 0x26; // BNE rel      - Branch if Not Equal
constexpr uint8_t BEQ = 0x27; // BEQ rel      - Branch if Equal
constexpr uint8_t BVC = 0x28; // BVC rel      - Branch if Overflow Clear
constexpr uint8_t BVS = 0x29; // BVS rel      - Branch if Overflow Set
constexpr uint8_t BPL = 0x2A; // BPL rel      - Branch if Plus
constexpr uint8_t BMI = 0x2B; // BMI rel      - Branch if Minus
constexpr uint8_t BGE =
    0x2C; // BGE rel      - Branch if Greater or Equal (signed)
constexpr uint8_t BLT = 0x2D; // BLT rel      - Branch if Less Than (signed)
constexpr uint8_t BGT = 0x2E; // BGT rel      - Branch if Greater Than (signed)
constexpr uint8_t BLE = 0x2F; // BLE rel      - Branch if Less or Equal (signed)
constexpr uint8_t BSR = 0x8D; // BSR rel      - Branch to Subroutine

// ----------------------------------------------------------------------------
// Jump and Subroutine Instructions
// ----------------------------------------------------------------------------

constexpr uint8_t JMP_DIR = 0x0E; // JMP <dp      - Jump Direct
constexpr uint8_t JMP_EXT = 0x7E; // JMP addr     - Jump Extended
constexpr uint8_t JMP_IDX = 0x6E; // JMP ,X       - Jump Indexed

constexpr uint8_t JSR_DIR = 0x9D; // JSR <dp      - Jump to Subroutine Direct
constexpr uint8_t JSR_EXT = 0xBD; // JSR addr     - Jump to Subroutine Extended
constexpr uint8_t JSR_IDX = 0xAD; // JSR ,X       - Jump to Subroutine Indexed

// ----------------------------------------------------------------------------
// Load Effective Address Instructions
// ----------------------------------------------------------------------------

constexpr uint8_t LEAX = 0x30; // LEAX ,X      - Load Effective Address into X
constexpr uint8_t LEAY = 0x31; // LEAY ,X      - Load Effective Address into Y
constexpr uint8_t LEAS = 0x32; // LEAS ,X      - Load Effective Address into S
constexpr uint8_t LEAU = 0x33; // LEAU ,X      - Load Effective Address into U

// ----------------------------------------------------------------------------
// Stack Operations
// ----------------------------------------------------------------------------

constexpr uint8_t PSHS = 0x34; // PSHS reglist - Push to System stack
constexpr uint8_t PULS = 0x35; // PULS reglist - Pull from System stack
constexpr uint8_t PSHU = 0x36; // PSHU reglist - Push to User stack
constexpr uint8_t PULU = 0x37; // PULU reglist - Pull from User stack

// ----------------------------------------------------------------------------
// Register Transfer and Exchange
// ----------------------------------------------------------------------------

constexpr uint8_t EXG = 0x1E; // EXG r1,r2    - Exchange Registers
constexpr uint8_t TFR = 0x1F; // TFR r1,r2    - Transfer Register

// ============================================================================
// Page 2: Extended Opcodes (prefix $10)
// ============================================================================

namespace Page2 {

// Long Branch Instructions (16-bit relative offset)
constexpr uint8_t LBRA = 0x16; // LBRA rel16   - Long Branch Always
constexpr uint8_t LBRN = 0x21; // LBRN rel16   - Long Branch Never
constexpr uint8_t LBHI = 0x22; // LBHI rel16   - Long Branch if Higher
constexpr uint8_t LBLS = 0x23; // LBLS rel16   - Long Branch if Lower or Same
constexpr uint8_t LBCC = 0x24; // LBCC rel16   - Long Branch if Carry Clear
constexpr uint8_t LBCS = 0x25; // LBCS rel16   - Long Branch if Carry Set
constexpr uint8_t LBNE = 0x26; // LBNE rel16   - Long Branch if Not Equal
constexpr uint8_t LBEQ = 0x27; // LBEQ rel16   - Long Branch if Equal
constexpr uint8_t LBVC = 0x28; // LBVC rel16   - Long Branch if Overflow Clear
constexpr uint8_t LBVS = 0x29; // LBVS rel16   - Long Branch if Overflow Set
constexpr uint8_t LBPL = 0x2A; // LBPL rel16   - Long Branch if Plus
constexpr uint8_t LBMI = 0x2B; // LBMI rel16   - Long Branch if Minus
constexpr uint8_t LBGE = 0x2C; // LBGE rel16   - Long Branch if Greater or Equal
constexpr uint8_t LBLT = 0x2D; // LBLT rel16   - Long Branch if Less Than
constexpr uint8_t LBGT = 0x2E; // LBGT rel16   - Long Branch if Greater Than
constexpr uint8_t LBLE = 0x2F; // LBLE rel16   - Long Branch if Less or Equal

// Y Register Operations (same opcodes as X, but with $10 prefix)
constexpr uint8_t LDY_IMM = 0x8E; // LDY #imm     - Immediate (big-endian)
constexpr uint8_t LDY_DIR = 0x9E; // LDY <dp      - Direct Page
constexpr uint8_t LDY_EXT = 0xBE; // LDY addr     - Extended (absolute)
constexpr uint8_t LDY_IDX = 0xAE; // LDY ,X       - Indexed

constexpr uint8_t STY_DIR = 0x9F; // STY <dp      - Direct Page
constexpr uint8_t STY_EXT = 0xBF; // STY addr     - Extended (absolute)
constexpr uint8_t STY_IDX = 0xAF; // STY ,X       - Indexed

constexpr uint8_t CMPY_IMM = 0x8C; // CMPY #imm    - Immediate (big-endian)
constexpr uint8_t CMPY_DIR = 0x9C; // CMPY <dp     - Direct Page
constexpr uint8_t CMPY_EXT = 0xBC; // CMPY addr    - Extended (absolute)
constexpr uint8_t CMPY_IDX = 0xAC; // CMPY ,X      - Indexed

// S Stack Pointer Operations (same opcodes as U, but with $10 prefix)
constexpr uint8_t LDS_IMM = 0xCE; // LDS #imm     - Immediate (big-endian)
constexpr uint8_t LDS_DIR = 0xDE; // LDS <dp      - Direct Page
constexpr uint8_t LDS_EXT = 0xFE; // LDS addr     - Extended (absolute)
constexpr uint8_t LDS_IDX = 0xEE; // LDS ,X       - Indexed

constexpr uint8_t STS_DIR = 0xDF; // STS <dp      - Direct Page
constexpr uint8_t STS_EXT = 0xFF; // STS addr     - Extended (absolute)
constexpr uint8_t STS_IDX = 0xEF; // STS ,X       - Indexed

} // namespace Page2

// ============================================================================
// Page 3: Extended Opcodes (prefix $11)
// ============================================================================

namespace Page3 {

// Additional U Stack Pointer Operations
// (Currently reserved/unused in standard 6809)

} // namespace Page3

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
constexpr uint8_t REG_X = 0x00; // 00 = X register
constexpr uint8_t REG_Y = 0x20; // 01 = Y register
constexpr uint8_t REG_U = 0x40; // 10 = U register
constexpr uint8_t REG_S = 0x60; // 11 = S register

// Indexed Mode Encoding (bits 0-4 of post-byte)
constexpr uint8_t MODE_CONSTANT_OFFSET_5BIT = 0x00;  // ,R + 5-bit offset (0-31)
constexpr uint8_t MODE_CONSTANT_OFFSET_8BIT = 0x88;  // ,R + 8-bit offset
constexpr uint8_t MODE_CONSTANT_OFFSET_16BIT = 0x89; // ,R + 16-bit offset
constexpr uint8_t MODE_ACCUMULATOR_A = 0x86;         // A,R
constexpr uint8_t MODE_ACCUMULATOR_B = 0x85;         // B,R
constexpr uint8_t MODE_ACCUMULATOR_D = 0x8B;         // D,R
constexpr uint8_t MODE_AUTO_INCREMENT_1 = 0x80;      // ,R+
constexpr uint8_t MODE_AUTO_INCREMENT_2 = 0x81;      // ,R++
constexpr uint8_t MODE_AUTO_DECREMENT_1 = 0x82;      // ,-R
constexpr uint8_t MODE_AUTO_DECREMENT_2 = 0x83;      // ,--R
constexpr uint8_t MODE_ZERO_OFFSET = 0x84;           // ,R (no offset)
constexpr uint8_t MODE_PC_RELATIVE_8BIT = 0x8C;      // n,PCR (8-bit)
constexpr uint8_t MODE_PC_RELATIVE_16BIT = 0x8D;     // n,PCR (16-bit)
constexpr uint8_t MODE_EXTENDED_INDIRECT = 0x9F;     // [$addr]

// Indirect flag (OR with above modes for indirect addressing)
constexpr uint8_t INDIRECT_FLAG = 0x10; // [ ,R ] indirect

} // namespace Indexed

// ============================================================================
// Register Encoding (for TFR/EXG instructions)
// ============================================================================

namespace Registers {

// Register codes for TFR and EXG instructions (4-bit codes)
constexpr uint8_t D = 0x0;  // D register (A:B concatenated, 16-bit)
constexpr uint8_t X = 0x1;  // X register (16-bit)
constexpr uint8_t Y = 0x2;  // Y register (16-bit)
constexpr uint8_t U = 0x3;  // U stack pointer (16-bit)
constexpr uint8_t S = 0x4;  // S stack pointer (16-bit)
constexpr uint8_t PC = 0x5; // PC program counter (16-bit)
constexpr uint8_t A = 0x8;  // A accumulator (8-bit)
constexpr uint8_t B = 0x9;  // B accumulator (8-bit)
constexpr uint8_t CC = 0xA; // CC condition codes (8-bit)
constexpr uint8_t DP = 0xB; // DP direct page register (8-bit)

} // namespace Registers

// ============================================================================
// Stack Register Mask Bits (for PSHS/PULS/PSHU/PULU)
// ============================================================================

namespace StackMask {

// Register mask bits for PSHS/PULS/PSHU/PULU instructions
constexpr uint8_t CC = 0x01; // Bit 0: CC (Condition Codes)
constexpr uint8_t A = 0x02;  // Bit 1: A
constexpr uint8_t B = 0x04;  // Bit 2: B
constexpr uint8_t DP = 0x08; // Bit 3: DP (Direct Page)
constexpr uint8_t X = 0x10;  // Bit 4: X
constexpr uint8_t Y = 0x20;  // Bit 5: Y
constexpr uint8_t U_OR_S =
    0x40;                    // Bit 6: U (for PSHS/PULS) or S (for PSHU/PULU)
constexpr uint8_t PC = 0x80; // Bit 7: PC (Program Counter)

} // namespace StackMask

} // namespace Opcodes6809
} // namespace xasm
