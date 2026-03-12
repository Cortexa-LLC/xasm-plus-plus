# Opcode Header Implementation Guide

**Purpose:** Step-by-step guide for implementing opcode headers for Z80 and 6809
**Audience:** Developers assigned to Actions 1 and 2
**Estimated Time:** 10 hours total (4h Z80, 6h 6809)

---

## Quick Start: What You're Building

You're creating header files that define CPU opcodes as named constants, following the pattern established by `opcodes_6502.h`. This eliminates magic numbers and creates self-documenting code.

**Before (bad):**
```cpp
return {0x3E, value};  // ❌ What is 0x3E?
```

**After (good):**
```cpp
return {opcodes::LD_A_n, value};  // ✅ Clear: LD A, n instruction
```

---

## Part 1: Z80 Opcode Header (4 hours)

### Step 1: Create File Structure (15 minutes)

**Create:** `include/xasm++/cpu/opcodes_z80.h`

**Template:**
```cpp
/**
 * @file opcodes_z80.h
 * @brief Zilog Z80 instruction opcodes
 *
 * This file defines all Z80 opcodes as named constants, eliminating
 * magic numbers and providing a single source of truth for Z80 encoding.
 *
 * Organization:
 * - Basic opcodes in main namespace (opcodes::)
 * - Prefix bytes in opcodes::prefix::
 * - CB-prefixed opcodes in opcodes::cb::
 * - ED-prefixed opcodes in opcodes::ed::
 * - DD/FD-prefixed opcodes in opcodes::dd:: and opcodes::fd::
 *
 * Reference: Zilog Z80 CPU User Manual
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace opcodes {

// Your opcodes go here

} // namespace opcodes
} // namespace xasm
```

---

### Step 2: Document Basic Opcodes (1.5 hours)

**Categories to implement:**

#### A. 8-bit Load Instructions (20 opcodes)
```cpp
// ============================================================================
// 8-bit Load Instructions (LD r, n)
// ============================================================================
// Load immediate value into register
// Format: LD r, n → opcode + immediate byte

constexpr uint8_t LD_A_n = 0x3E;  // LD A, n
constexpr uint8_t LD_B_n = 0x06;  // LD B, n
constexpr uint8_t LD_C_n = 0x0E;  // LD C, n
constexpr uint8_t LD_D_n = 0x16;  // LD D, n
constexpr uint8_t LD_E_n = 0x1E;  // LD E, n
constexpr uint8_t LD_H_n = 0x26;  // LD H, n
constexpr uint8_t LD_L_n = 0x2E;  // LD L, n

// Add more: LD r, r' (register to register)
// LD A, (BC), LD A, (DE), LD (HL), n, etc.
```

**Reference:** Z80 User Manual, Section 7.1 (8-bit Load Group)

#### B. 16-bit Load Instructions (15 opcodes)
```cpp
// ============================================================================
// 16-bit Load Instructions (LD rr, nn)
// ============================================================================
// Load 16-bit immediate value into register pair
// Format: LD rr, nn → opcode + low byte + high byte

constexpr uint8_t LD_BC_nn = 0x01;  // LD BC, nn
constexpr uint8_t LD_DE_nn = 0x11;  // LD DE, nn
constexpr uint8_t LD_HL_nn = 0x21;  // LD HL, nn
constexpr uint8_t LD_SP_nn = 0x31;  // LD SP, nn

// Add more: LD (nn), HL; LD HL, (nn), etc.
```

**Reference:** Z80 User Manual, Section 7.2 (16-bit Load Group)

#### C. Arithmetic Instructions (20 opcodes)
```cpp
// ============================================================================
// 8-bit Arithmetic and Logic Instructions
// ============================================================================

// Add/Subtract
constexpr uint8_t ADD_A_n = 0xC6;   // ADD A, n
constexpr uint8_t ADC_A_n = 0xCE;   // ADC A, n (with carry)
constexpr uint8_t SUB_n = 0xD6;     // SUB n
constexpr uint8_t SBC_A_n = 0xDE;   // SBC A, n (with carry)

// Logical
constexpr uint8_t AND_n = 0xE6;     // AND n
constexpr uint8_t OR_n = 0xF6;      // OR n
constexpr uint8_t XOR_n = 0xEE;     // XOR n
constexpr uint8_t CP_n = 0xFE;      // CP n (compare)

// Increment/Decrement
constexpr uint8_t INC_A = 0x3C;     // INC A
constexpr uint8_t DEC_A = 0x3D;     // DEC A
// Add more: INC B, INC C, etc.
```

**Reference:** Z80 User Manual, Section 7.3 (Arithmetic Group)

#### D. Control Flow Instructions (15 opcodes)
```cpp
// ============================================================================
// Jump, Call, and Return Instructions
// ============================================================================

constexpr uint8_t JP_nn = 0xC3;     // JP nn (unconditional jump)
constexpr uint8_t JP_NZ_nn = 0xC2;  // JP NZ, nn (jump if not zero)
constexpr uint8_t JP_Z_nn = 0xCA;   // JP Z, nn (jump if zero)
constexpr uint8_t JP_NC_nn = 0xD2;  // JP NC, nn (jump if no carry)
constexpr uint8_t JP_C_nn = 0xDA;   // JP C, nn (jump if carry)

constexpr uint8_t CALL_nn = 0xCD;   // CALL nn
constexpr uint8_t RET = 0xC9;       // RET (return)
constexpr uint8_t RET_NZ = 0xC0;    // RET NZ
constexpr uint8_t RET_Z = 0xC8;     // RET Z

// Add more conditional jumps and calls
```

**Reference:** Z80 User Manual, Section 7.7 (Jump, Call, Return Group)

#### E. Miscellaneous (10 opcodes)
```cpp
// ============================================================================
// Miscellaneous Instructions
// ============================================================================

constexpr uint8_t NOP = 0x00;       // NOP (no operation)
constexpr uint8_t HALT = 0x76;      // HALT
constexpr uint8_t DI = 0xF3;        // DI (disable interrupts)
constexpr uint8_t EI = 0xFB;        // EI (enable interrupts)

constexpr uint8_t PUSH_BC = 0xC5;   // PUSH BC
constexpr uint8_t POP_BC = 0xC1;    // POP BC
// Add more: PUSH DE, PUSH HL, etc.
```

**Reference:** Z80 User Manual, Section 7.11 (Miscellaneous Group)

---

### Step 3: Document Prefix Bytes (30 minutes)

```cpp
// ============================================================================
// Prefix Bytes (for multi-byte instructions)
// ============================================================================
// Z80 uses prefix bytes to extend the instruction set:
// - 0xCB: Bit manipulation instructions (shifts, rotates, bit test/set/reset)
// - 0xED: Extended instructions (block operations, I/O, special ops)
// - 0xDD: IX index register operations (replaces HL with IX)
// - 0xFD: IY index register operations (replaces HL with IY)

namespace prefix {
  constexpr uint8_t CB = 0xCB;  // Bit manipulation prefix
  constexpr uint8_t ED = 0xED;  // Extended instruction prefix
  constexpr uint8_t DD = 0xDD;  // IX register prefix
  constexpr uint8_t FD = 0xFD;  // IY register prefix
}
```

---

### Step 4: Document CB-Prefixed Instructions (1 hour)

```cpp
// ============================================================================
// CB-Prefixed Instructions (Bit Operations)
// ============================================================================
// Format: 0xCB + opcode byte
// All these instructions take 2 bytes: prefix + operation

namespace cb {
  // Bit Test (BIT b, r): Test bit b in register r
  // Opcode calculation: 0x40 + (bit * 8) + register
  constexpr uint8_t BIT_BASE = 0x40;
  
  // Examples (can be calculated with BitOpcode helper):
  constexpr uint8_t BIT_0_A = 0x47;  // BIT 0, A
  constexpr uint8_t BIT_1_B = 0x48;  // BIT 1, B
  constexpr uint8_t BIT_7_H = 0x7C;  // BIT 7, H
  
  // Set Bit (SET b, r): Set bit b in register r
  // Opcode calculation: 0xC0 + (bit * 8) + register
  constexpr uint8_t SET_BASE = 0xC0;
  
  // Reset Bit (RES b, r): Reset bit b in register r
  // Opcode calculation: 0x80 + (bit * 8) + register
  constexpr uint8_t RES_BASE = 0x80;
  
  // Rotate and Shift Instructions
  constexpr uint8_t RLC_A = 0x07;    // Rotate Left Circular A
  constexpr uint8_t RRC_A = 0x0F;    // Rotate Right Circular A
  constexpr uint8_t RL_A = 0x17;     // Rotate Left A (through carry)
  constexpr uint8_t RR_A = 0x1F;     // Rotate Right A (through carry)
  constexpr uint8_t SLA_A = 0x27;    // Shift Left Arithmetic A
  constexpr uint8_t SRA_A = 0x2F;    // Shift Right Arithmetic A
  constexpr uint8_t SRL_A = 0x3F;    // Shift Right Logical A
  
  // Add more for B, C, D, E, H, L registers
  
  /**
   * @brief Calculate bit operation opcode
   * 
   * Helper function to calculate opcodes for BIT, SET, RES instructions.
   * 
   * @param base Base opcode (BIT_BASE, SET_BASE, or RES_BASE)
   * @param bit Bit number (0-7)
   * @param reg Register encoding (0=B, 1=C, 2=D, 3=E, 4=H, 5=L, 7=A)
   * @return Calculated opcode byte
   * 
   * Example:
   *   BitOpcode(BIT_BASE, 3, 7) → 0x5F (BIT 3, A)
   */
  constexpr uint8_t BitOpcode(uint8_t base, uint8_t bit, uint8_t reg) {
    return base + (bit << 3) + reg;
  }
}
```

**Reference:** Z80 User Manual, Section 7.4 (Rotate and Shift Group), 7.6 (Bit Manipulation Group)

---

### Step 5: Document ED-Prefixed Instructions (45 minutes)

```cpp
// ============================================================================
// ED-Prefixed Instructions (Extended Operations)
// ============================================================================
// Format: 0xED + opcode byte
// These are advanced Z80 instructions not available on 8080

namespace ed {
  // Register Load Instructions
  constexpr uint8_t LD_I_A = 0x47;    // LD I, A (load interrupt register)
  constexpr uint8_t LD_R_A = 0x4F;    // LD R, A (load refresh register)
  constexpr uint8_t LD_A_I = 0x57;    // LD A, I
  constexpr uint8_t LD_A_R = 0x5F;    // LD A, R
  
  // Block Transfer Instructions
  constexpr uint8_t LDI = 0xA0;       // LDI (load and increment)
  constexpr uint8_t LDIR = 0xB0;      // LDIR (load, increment, repeat)
  constexpr uint8_t LDD = 0xA8;       // LDD (load and decrement)
  constexpr uint8_t LDDR = 0xB8;      // LDDR (load, decrement, repeat)
  
  // Block Search Instructions
  constexpr uint8_t CPI = 0xA1;       // CPI (compare and increment)
  constexpr uint8_t CPIR = 0xB1;      // CPIR (compare, increment, repeat)
  constexpr uint8_t CPD = 0xA9;       // CPD (compare and decrement)
  constexpr uint8_t CPDR = 0xB9;      // CPDR (compare, decrement, repeat)
  
  // I/O Block Instructions
  constexpr uint8_t INI = 0xA2;       // INI (input and increment)
  constexpr uint8_t INIR = 0xB2;      // INIR (input, increment, repeat)
  constexpr uint8_t OUTI = 0xA3;      // OUTI (output and increment)
  constexpr uint8_t OTIR = 0xB3;      // OTIR (output, increment, repeat)
  
  // Special Instructions
  constexpr uint8_t NEG = 0x44;       // NEG (negate A, two's complement)
  constexpr uint8_t RETN = 0x45;      // RETN (return from non-maskable interrupt)
  constexpr uint8_t RETI = 0x4D;      // RETI (return from interrupt)
  constexpr uint8_t RRD = 0x67;       // RRD (rotate right BCD)
  constexpr uint8_t RLD = 0x6F;       // RLD (rotate left BCD)
  
  // 16-bit arithmetic
  constexpr uint8_t ADC_HL_BC = 0x4A; // ADC HL, BC
  constexpr uint8_t SBC_HL_BC = 0x42; // SBC HL, BC
  // Add more: ADC HL, DE; ADC HL, HL; ADC HL, SP
}
```

**Reference:** Z80 User Manual, Section 7.8 (Block Transfer and Search Group)

---

### Step 6: Add Helper Functions (15 minutes)

```cpp
// ============================================================================
// Helper Functions and Constants
// ============================================================================

/**
 * @brief Register encoding for bit operations
 * 
 * Z80 uses 3-bit register encoding:
 * 0 = B, 1 = C, 2 = D, 3 = E, 4 = H, 5 = L, 6 = (HL), 7 = A
 */
namespace register_encoding {
  constexpr uint8_t B = 0;
  constexpr uint8_t C = 1;
  constexpr uint8_t D = 2;
  constexpr uint8_t E = 3;
  constexpr uint8_t H = 4;
  constexpr uint8_t L = 5;
  constexpr uint8_t HL_INDIRECT = 6;
  constexpr uint8_t A = 7;
}

/**
 * @brief Condition code encoding for conditional jumps/calls
 * 
 * Used in JP cc, nn and CALL cc, nn instructions
 */
namespace condition_codes {
  constexpr uint8_t NZ = 0x00;  // Not Zero (Z=0)
  constexpr uint8_t Z  = 0x08;  // Zero (Z=1)
  constexpr uint8_t NC = 0x10;  // No Carry (C=0)
  constexpr uint8_t C  = 0x18;  // Carry (C=1)
  constexpr uint8_t PO = 0x20;  // Parity Odd (P=0)
  constexpr uint8_t PE = 0x28;  // Parity Even (P=1)
  constexpr uint8_t P  = 0x30;  // Plus (S=0)
  constexpr uint8_t M  = 0x38;  // Minus (S=1)
}
```

---

### Step 7: Verification Checklist (30 minutes)

**Use this checklist to verify your header:**

- [ ] File header comment explains purpose
- [ ] All basic load instructions documented (~30 opcodes)
- [ ] All arithmetic instructions documented (~25 opcodes)
- [ ] All control flow instructions documented (~20 opcodes)
- [ ] All prefix bytes defined (CB, ED, DD, FD)
- [ ] CB-prefixed bit operations documented (~30 opcodes)
- [ ] ED-prefixed extended operations documented (~30 opcodes)
- [ ] Helper functions for calculated opcodes included
- [ ] Register encoding constants defined
- [ ] Each opcode has inline comment explaining instruction
- [ ] Instructions organized by category
- [ ] References to Z80 manual included
- [ ] No magic numbers remain (all are named constants)
- [ ] File compiles without errors
- [ ] Spot-check 10 opcodes against Z80 reference manual

**Total Opcodes:** Approximately 150-180 constants

---

## Part 2: 6809 Opcode Header (6 hours)

### Step 1: Create File Structure (15 minutes)

**Create:** `include/xasm++/cpu/opcodes_6809.h`

**Template:**
```cpp
/**
 * @file opcodes_6809.h
 * @brief Motorola 6809 instruction opcodes
 *
 * This file defines all 6809 opcodes as named constants.
 * 
 * The 6809 uses a three-page opcode space:
 * - Page 1: Standard single-byte opcodes (60+ instructions)
 * - Page 2: Prefix $10 + opcode (25+ instructions, mostly long branches and Y-register ops)
 * - Page 3: Prefix $11 + opcode (15+ instructions, mostly S-stack and U-register ops)
 *
 * Indexed addressing uses a post-byte to encode 16 sub-modes.
 *
 * Organization:
 * - Page 1 opcodes in main namespace (opcodes::)
 * - Page 2 opcodes in opcodes::page2::
 * - Page 3 opcodes in opcodes::page3::
 * - Indexed addressing constants in opcodes::indexed::
 *
 * Reference: Motorola MC6809 Programming Manual
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace opcodes {

// Your opcodes go here

} // namespace opcodes
} // namespace xasm
```

---

### Step 2: Document Page 1 Opcodes (2 hours)

**More complex than Z80 due to multiple addressing modes per instruction!**

#### A. Inherent Instructions (20 opcodes)
```cpp
// ============================================================================
// Inherent Instructions (Page 1, no operand)
// ============================================================================

constexpr uint8_t NOP = 0x12;       // NOP
constexpr uint8_t RTS = 0x39;       // RTS (Return from Subroutine)
constexpr uint8_t RTI = 0x3B;       // RTI (Return from Interrupt)
constexpr uint8_t SWI = 0x3F;       // SWI (Software Interrupt)

// Clear Register
constexpr uint8_t CLRA = 0x4F;      // Clear Accumulator A
constexpr uint8_t CLRB = 0x5F;      // Clear Accumulator B

// Shift/Rotate A Register
constexpr uint8_t ASLA = 0x48;      // Arithmetic Shift Left A
constexpr uint8_t ASRA = 0x47;      // Arithmetic Shift Right A
constexpr uint8_t LSRA = 0x44;      // Logical Shift Right A
constexpr uint8_t ROLA = 0x49;      // Rotate Left A through Carry
constexpr uint8_t RORA = 0x46;      // Rotate Right A through Carry

// Shift/Rotate B Register
constexpr uint8_t ASLB = 0x58;      // Arithmetic Shift Left B
constexpr uint8_t ASRB = 0x57;      // Arithmetic Shift Right B
constexpr uint8_t LSRB = 0x54;      // Logical Shift Right B
constexpr uint8_t ROLB = 0x59;      // Rotate Left B through Carry
constexpr uint8_t RORB = 0x56;      // Rotate Right B through Carry

// Increment/Decrement
constexpr uint8_t INCA = 0x4C;      // Increment A
constexpr uint8_t INCB = 0x5C;      // Increment B
constexpr uint8_t DECA = 0x4A;      // Decrement A
constexpr uint8_t DECB = 0x5A;      // Decrement B

// Test/Compare/Negate
constexpr uint8_t TSTA = 0x4D;      // Test A (set condition codes)
constexpr uint8_t TSTB = 0x5D;      // Test B (set condition codes)
constexpr uint8_t COMA = 0x43;      // Complement A (one's complement)
constexpr uint8_t COMB = 0x53;      // Complement B (one's complement)
constexpr uint8_t NEGA = 0x40;      // Negate A (two's complement)
constexpr uint8_t NEGB = 0x50;      // Negate B (two's complement)

// More: Add DAA, SEX, MUL, etc.
```

#### B. Load/Store A Register (4 modes × 1 instruction = 4 opcodes)
```cpp
// ============================================================================
// Load Accumulator A (Page 1)
// ============================================================================
// 6809 has 4 addressing modes for most instructions:
// - Immediate: #value
// - Direct: <address (zero-page equivalent, uses direct page register)
// - Extended: address (16-bit absolute)
// - Indexed: various indexed modes (uses post-byte)

constexpr uint8_t LDA_IMM = 0x86;   // LDA #immediate
constexpr uint8_t LDA_DIR = 0x96;   // LDA <direct page
constexpr uint8_t LDA_IDX = 0xA6;   // LDA ,indexed (post-byte specifies mode)
constexpr uint8_t LDA_EXT = 0xB6;   // LDA extended (16-bit address)

// Store Accumulator A
constexpr uint8_t STA_DIR = 0x97;   // STA <direct page
constexpr uint8_t STA_IDX = 0xA7;   // STA ,indexed
constexpr uint8_t STA_EXT = 0xB7;   // STA extended
// Note: No STA immediate (can't store to a constant!)
```

#### C. Load/Store B Register (4 opcodes)
```cpp
// ============================================================================
// Load Accumulator B (Page 1)
// ============================================================================

constexpr uint8_t LDB_IMM = 0xC6;   // LDB #immediate
constexpr uint8_t LDB_DIR = 0xD6;   // LDB <direct page
constexpr uint8_t LDB_IDX = 0xE6;   // LDB ,indexed
constexpr uint8_t LDB_EXT = 0xF6;   // LDB extended

// Store Accumulator B
constexpr uint8_t STB_DIR = 0xD7;   // STB <direct page
constexpr uint8_t STB_IDX = 0xE7;   // STB ,indexed
constexpr uint8_t STB_EXT = 0xF7;   // STB extended
```

#### D. Load/Store D Register (16-bit A:B pair)
```cpp
// ============================================================================
// Load D Register (16-bit, A:B pair) (Page 1)
// ============================================================================

constexpr uint8_t LDD_IMM = 0xCC;   // LDD #immediate
constexpr uint8_t LDD_DIR = 0xDC;   // LDD <direct page
constexpr uint8_t LDD_IDX = 0xEC;   // LDD ,indexed
constexpr uint8_t LDD_EXT = 0xFC;   // LDD extended

constexpr uint8_t STD_DIR = 0xDD;   // STD <direct page
constexpr uint8_t STD_IDX = 0xED;   // STD ,indexed
constexpr uint8_t STD_EXT = 0xFD;   // STD extended
```

**Continue this pattern for:**
- Load/Store X register
- Load/Store U register
- Load/Store S register (system stack pointer)
- ADD, SUB, AND, OR, EOR instructions (each with 4 modes)
- CMP instructions (CMPA, CMPB, CMPD)

---

### Step 3: Document Branch Instructions (30 minutes)

```cpp
// ============================================================================
// Branch Instructions (Page 1, 8-bit relative)
// ============================================================================
// Format: opcode + signed 8-bit offset (-128 to +127 bytes)

constexpr uint8_t BRA = 0x20;       // Branch Always
constexpr uint8_t BRN = 0x21;       // Branch Never (used for timing delay)

// Test Z flag (Zero)
constexpr uint8_t BEQ = 0x27;       // Branch if Equal (Z=1)
constexpr uint8_t BNE = 0x26;       // Branch if Not Equal (Z=0)

// Test C flag (Carry)
constexpr uint8_t BCC = 0x24;       // Branch if Carry Clear (C=0)
constexpr uint8_t BCS = 0x25;       // Branch if Carry Set (C=1)
// Aliases:
constexpr uint8_t BHS = BCC;        // Branch if Higher or Same (unsigned)
constexpr uint8_t BLO = BCS;        // Branch if Lower (unsigned)

// Test N flag (Negative)
constexpr uint8_t BMI = 0x2B;       // Branch if Minus (N=1)
constexpr uint8_t BPL = 0x2A;       // Branch if Plus (N=0)

// Test V flag (Overflow)
constexpr uint8_t BVS = 0x29;       // Branch if Overflow Set (V=1)
constexpr uint8_t BVC = 0x28;       // Branch if Overflow Clear (V=0)

// Signed comparisons
constexpr uint8_t BGE = 0x2C;       // Branch if Greater or Equal (N⊕V=0)
constexpr uint8_t BLT = 0x2D;       // Branch if Less Than (N⊕V=1)
constexpr uint8_t BGT = 0x2E;       // Branch if Greater Than (Z|(N⊕V)=0)
constexpr uint8_t BLE = 0x2F;       // Branch if Less or Equal (Z|(N⊕V)=1)

// Unsigned comparisons
constexpr uint8_t BHI = 0x22;       // Branch if Higher (C|Z=0)
constexpr uint8_t BLS = 0x23;       // Branch if Lower or Same (C|Z=1)

// Subroutine
constexpr uint8_t BSR = 0x8D;       // Branch to Subroutine (8-bit relative)
```

**Reference:** 6809 Programming Manual, Section 4.3 (Branch Instructions)

---

### Step 4: Document Page 2 Opcodes (1.5 hours)

```cpp
// ============================================================================
// Prefix Bytes (Multi-page opcodes)
// ============================================================================

namespace prefix {
  constexpr uint8_t PAGE2 = 0x10;   // Page 2 prefix
  constexpr uint8_t PAGE3 = 0x11;   // Page 3 prefix
}

// ============================================================================
// Page 2 Instructions (Prefix $10)
// ============================================================================
// Format: $10 + opcode [+ operand bytes]

namespace page2 {
  // Long Branches (16-bit relative, extended range)
  constexpr uint8_t LBRA = 0x16;    // Long Branch Always
  constexpr uint8_t LBRN = 0x21;    // Long Branch Never
  constexpr uint8_t LBEQ = 0x27;    // Long Branch if Equal
  constexpr uint8_t LBNE = 0x26;    // Long Branch if Not Equal
  constexpr uint8_t LBCC = 0x24;    // Long Branch if Carry Clear
  constexpr uint8_t LBCS = 0x25;    // Long Branch if Carry Set
  constexpr uint8_t LBMI = 0x2B;    // Long Branch if Minus
  constexpr uint8_t LBPL = 0x2A;    // Long Branch if Plus
  constexpr uint8_t LBVS = 0x29;    // Long Branch if Overflow Set
  constexpr uint8_t LBVC = 0x28;    // Long Branch if Overflow Clear
  constexpr uint8_t LBGE = 0x2C;    // Long Branch if Greater or Equal
  constexpr uint8_t LBLT = 0x2D;    // Long Branch if Less Than
  constexpr uint8_t LBGT = 0x2E;    // Long Branch if Greater Than
  constexpr uint8_t LBLE = 0x2F;    // Long Branch if Less or Equal
  constexpr uint8_t LBHI = 0x22;    // Long Branch if Higher
  constexpr uint8_t LBLS = 0x23;    // Long Branch if Lower or Same
  
  // Long Subroutine
  constexpr uint8_t LBSR = 0x17;    // Long Branch to Subroutine
  
  // Y Register Operations (16-bit index register)
  constexpr uint8_t LDY_IMM = 0x8E; // LDY #immediate
  constexpr uint8_t LDY_DIR = 0x9E; // LDY <direct page
  constexpr uint8_t LDY_IDX = 0xAE; // LDY ,indexed
  constexpr uint8_t LDY_EXT = 0xBE; // LDY extended
  
  constexpr uint8_t STY_DIR = 0x9F; // STY <direct page
  constexpr uint8_t STY_IDX = 0xAF; // STY ,indexed
  constexpr uint8_t STY_EXT = 0xBF; // STY extended
  
  // System Stack Pointer Operations
  constexpr uint8_t LDS_IMM = 0xCE; // LDS #immediate (system stack)
  constexpr uint8_t LDS_DIR = 0xDE; // LDS <direct page
  constexpr uint8_t LDS_IDX = 0xEE; // LDS ,indexed
  constexpr uint8_t LDS_EXT = 0xFE; // LDS extended
  
  constexpr uint8_t STS_DIR = 0xDF; // STS <direct page
  constexpr uint8_t STS_IDX = 0xEF; // STS ,indexed
  constexpr uint8_t STS_EXT = 0xFF; // STS extended
  
  // More: CMPY, SWI2, etc.
}
```

**Reference:** 6809 Programming Manual, Section 4.4 (Page 2 Instructions)

---

### Step 5: Document Page 3 Opcodes (30 minutes)

```cpp
// ============================================================================
// Page 3 Instructions (Prefix $11)
// ============================================================================
// Format: $11 + opcode [+ operand bytes]

namespace page3 {
  // U Register Operations (16-bit user stack pointer)
  constexpr uint8_t LDU_IMM = 0x8E; // LDU #immediate
  constexpr uint8_t LDU_DIR = 0x9E; // LDU <direct page
  constexpr uint8_t LDU_IDX = 0xAE; // LDU ,indexed
  constexpr uint8_t LDU_EXT = 0xBE; // LDU extended
  
  constexpr uint8_t STU_DIR = 0x9F; // STU <direct page
  constexpr uint8_t STU_IDX = 0xAF; // STU ,indexed
  constexpr uint8_t STU_EXT = 0xBF; // STU extended
  
  // Compare Instructions
  constexpr uint8_t CMPU_IMM = 0x83; // CMPU #immediate
  constexpr uint8_t CMPU_DIR = 0x93; // CMPU <direct page
  constexpr uint8_t CMPU_IDX = 0xA3; // CMPU ,indexed
  constexpr uint8_t CMPU_EXT = 0xB3; // CMPU extended
  
  constexpr uint8_t CMPS_IMM = 0x8C; // CMPS #immediate (compare S)
  constexpr uint8_t CMPS_DIR = 0x9C; // CMPS <direct page
  constexpr uint8_t CMPS_IDX = 0xAC; // CMPS ,indexed
  constexpr uint8_t CMPS_EXT = 0xBC; // CMPS extended
  
  // Software Interrupt 3
  constexpr uint8_t SWI3 = 0x3F;    // SWI3
}
```

---

### Step 6: Document Indexed Addressing Post-Bytes (1.5 hours)

**This is the most complex part of the 6809!**

```cpp
// ============================================================================
// Indexed Addressing Post-Byte Encoding
// ============================================================================
// The 6809's indexed addressing uses a post-byte to encode 16 sub-modes.
// This is the most sophisticated feature of the 6809.
//
// Post-byte format: RRxxxMMM
//   RR = register selection (00=X, 01=Y, 10=U, 11=S)
//   xxx = mode-specific encoding
//   MMM = additional mode bits
//
// Reference: 6809 Programming Manual, Section 3.3.3 (Indexed Addressing)

namespace indexed {
  // ========================================================================
  // Register Encoding (RR bits, bits 5-6 of post-byte)
  // ========================================================================
  
  constexpr uint8_t REG_X = 0x00;   // Index register X (bits 5-6 = 00)
  constexpr uint8_t REG_Y = 0x20;   // Index register Y (bits 5-6 = 01)
  constexpr uint8_t REG_U = 0x40;   // User stack pointer (bits 5-6 = 10)
  constexpr uint8_t REG_S = 0x60;   // System stack pointer (bits 5-6 = 11)
  
  // ========================================================================
  // Mode Flags
  // ========================================================================
  
  constexpr uint8_t INDIRECT = 0x10; // Indirect addressing flag (bit 4)
  
  // ========================================================================
  // 5-bit Offset Mode (no extra bytes)
  // ========================================================================
  // Format: 0RR0nnnn (n = signed 5-bit offset, -16 to +15)
  // Post-byte only, no extra bytes
  // Example: LDA 5,X → opcode + (REG_X | offset_value)
  
  constexpr uint8_t OFFSET_5BIT_MASK = 0x1F;  // Extract 5-bit offset
  constexpr uint8_t OFFSET_5BIT_FLAG = 0x00;  // Bit 7 = 0 indicates 5-bit mode
  
  // ========================================================================
  // Extended Modes (bit 7 = 1)
  // ========================================================================
  // When bit 7 = 1, bits 0-3 encode the mode:
  
  // Constant Offset (with extra bytes)
  constexpr uint8_t OFFSET_8BIT = 0x88;   // 8-bit offset (1 extra byte)
  constexpr uint8_t OFFSET_16BIT = 0x89;  // 16-bit offset (2 extra bytes)
  
  // Auto-increment/decrement
  constexpr uint8_t AUTO_INC_1 = 0x80;    // ,R+ (increment by 1)
  constexpr uint8_t AUTO_INC_2 = 0x81;    // ,R++ (increment by 2)
  constexpr uint8_t AUTO_DEC_1 = 0x82;    // ,-R (decrement by 1)
  constexpr uint8_t AUTO_DEC_2 = 0x83;    // ,--R (decrement by 2)
  
  // Zero Offset (no extra bytes)
  constexpr uint8_t ZERO_OFFSET = 0x84;   // ,R (zero offset, post-byte only)
  
  // Accumulator Offset
  constexpr uint8_t OFFSET_A = 0x86;      // A,R (A register offset)
  constexpr uint8_t OFFSET_B = 0x85;      // B,R (B register offset)
  constexpr uint8_t OFFSET_D = 0x8B;      // D,R (D register offset, 16-bit)
  
  // Indirect Modes (add INDIRECT flag to above)
  constexpr uint8_t INDIRECT_8BIT = OFFSET_8BIT | INDIRECT;    // [n,R]
  constexpr uint8_t INDIRECT_16BIT = OFFSET_16BIT | INDIRECT;  // [nn,R]
  constexpr uint8_t INDIRECT_ZERO = ZERO_OFFSET | INDIRECT;    // [,R]
  constexpr uint8_t INDIRECT_AUTO_INC_2 = AUTO_INC_2 | INDIRECT;  // [,R++]
  constexpr uint8_t INDIRECT_AUTO_DEC_2 = AUTO_DEC_2 | INDIRECT;  // [,--R]
  constexpr uint8_t INDIRECT_A = OFFSET_A | INDIRECT;          // [A,R]
  constexpr uint8_t INDIRECT_B = OFFSET_B | INDIRECT;          // [B,R]
  constexpr uint8_t INDIRECT_D = OFFSET_D | INDIRECT;          // [D,R]
  
  // PC-Relative
  constexpr uint8_t PC_OFFSET_8BIT = 0x8C;   // n,PCR (8-bit PC-relative)
  constexpr uint8_t PC_OFFSET_16BIT = 0x8D;  // nn,PCR (16-bit PC-relative)
  constexpr uint8_t PC_INDIRECT_16BIT = PC_OFFSET_16BIT | INDIRECT; // [nn,PCR]
  
  // Extended Indirect (16-bit address in next 2 bytes)
  constexpr uint8_t EXTENDED_INDIRECT = 0x9F; // [address]
  
  /**
   * @brief Build indexed post-byte
   * 
   * Helper function to construct post-byte for indexed addressing.
   * 
   * @param reg Register (REG_X, REG_Y, REG_U, REG_S)
   * @param mode Mode bits (OFFSET_8BIT, AUTO_INC_1, etc.)
   * @return Complete post-byte value
   * 
   * Example:
   *   BuildPostByte(REG_X, OFFSET_8BIT) → 0x88
   *   BuildPostByte(REG_Y, AUTO_INC_2) → 0xA1
   */
  constexpr uint8_t BuildPostByte(uint8_t reg, uint8_t mode) {
    return reg | mode;
  }
  
  /**
   * @brief Build 5-bit offset post-byte
   * 
   * @param reg Register (REG_X, REG_Y, REG_U, REG_S)
   * @param offset Signed offset (-16 to +15)
   * @return Post-byte with 5-bit offset encoded
   * 
   * Example:
   *   Build5BitOffset(REG_X, 5) → 0x05 (LDA 5,X)
   *   Build5BitOffset(REG_Y, -3) → 0x3D (LDA -3,Y)
   */
  constexpr uint8_t Build5BitOffset(uint8_t reg, int8_t offset) {
    return reg | (offset & OFFSET_5BIT_MASK);
  }
}
```

**Reference:** 6809 Programming Manual, Section 3.3.3 (Indexed Addressing) - **READ THIS CAREFULLY!**

---

### Step 7: Add Stack and Register Constants (15 minutes)

```cpp
// ============================================================================
// Stack Operation Bit Masks
// ============================================================================
// For PSHS, PULS, PSHU, PULU instructions
// Post-byte: which registers to push/pull

namespace stack_bits {
  constexpr uint8_t CC = 0x01;  // Condition Code register
  constexpr uint8_t A  = 0x02;  // Accumulator A
  constexpr uint8_t B  = 0x04;  // Accumulator B
  constexpr uint8_t DP = 0x08;  // Direct Page register
  constexpr uint8_t X  = 0x10;  // Index register X
  constexpr uint8_t Y  = 0x20;  // Index register Y
  constexpr uint8_t U  = 0x40;  // User stack pointer (for S stack)
  constexpr uint8_t PC = 0x80;  // Program Counter
  constexpr uint8_t S  = 0x40;  // System stack pointer (for U stack)
}

// ============================================================================
// Register Encoding for TFR and EXG Instructions
// ============================================================================
// Post-byte: source register (high nibble), dest register (low nibble)

namespace tfr_exg_encoding {
  constexpr uint8_t D  = 0x0;   // D (A:B pair)
  constexpr uint8_t X  = 0x1;   // X register
  constexpr uint8_t Y  = 0x2;   // Y register
  constexpr uint8_t U  = 0x3;   // U register
  constexpr uint8_t S  = 0x4;   // S register
  constexpr uint8_t PC = 0x5;   // PC (program counter)
  constexpr uint8_t A  = 0x8;   // A (8-bit)
  constexpr uint8_t B  = 0x9;   // B (8-bit)
  constexpr uint8_t CC = 0xA;   // CC (condition codes)
  constexpr uint8_t DP = 0xB;   // DP (direct page)
  
  /**
   * @brief Build TFR/EXG post-byte
   * 
   * @param src Source register
   * @param dst Destination register
   * @return Post-byte for TFR/EXG instruction
   * 
   * Example:
   *   BuildTfrByte(D, X) → 0x01 (TFR D,X)
   */
  constexpr uint8_t BuildTfrByte(uint8_t src, uint8_t dst) {
    return (src << 4) | dst;
  }
}
```

---

### Step 8: Verification Checklist (30 minutes)

**Use this checklist to verify your header:**

Page 1:
- [ ] All inherent instructions documented (~25 opcodes)
- [ ] All LDA/STA variants (immediate, direct, indexed, extended)
- [ ] All LDB/STB variants
- [ ] All LDD/STD variants
- [ ] All LDX/STX variants
- [ ] All branch instructions (short, 8-bit relative) (~15 opcodes)
- [ ] Arithmetic instructions (ADD, SUB, AND, OR, EOR) for A, B, D
- [ ] Compare instructions (CMPA, CMPB, CMPD, CMPX)
- [ ] Stack operations (PSHS, PULS, PSHU, PULU)
- [ ] Register transfer (TFR, EXG)

Page 2 (prefix $10):
- [ ] All long branch instructions (~16 opcodes)
- [ ] Long BSR
- [ ] Y register operations (LDY, STY, CMPY)
- [ ] S stack operations (LDS, STS, CMPS)
- [ ] SWI2

Page 3 (prefix $11):
- [ ] U register operations (LDU, STU, CMPU)
- [ ] S compare (CMPS)
- [ ] SWI3

Indexed Addressing:
- [ ] Register encoding constants
- [ ] 5-bit offset mode constant
- [ ] 8-bit offset constant
- [ ] 16-bit offset constant
- [ ] Auto-increment/decrement constants
- [ ] Accumulator offset constants
- [ ] PC-relative constants
- [ ] Indirect mode flag
- [ ] Helper functions (BuildPostByte, Build5BitOffset)

Stack/Register:
- [ ] Stack bit masks for PSHS/PULS
- [ ] TFR/EXG register encoding
- [ ] Helper function for TFR/EXG post-byte

Verification:
- [ ] Spot-check 20 opcodes against 6809 reference manual
- [ ] Verify all 3 pages represented
- [ ] Verify indexed modes match manual
- [ ] File compiles without errors
- [ ] All opcodes have inline comments
- [ ] Instructions organized by category
- [ ] References to 6809 manual included

**Total Opcodes:** Approximately 100-120 constants + helper functions

---

## Part 3: Implementation Refactoring (After Headers Complete)

### Z80 Refactoring Checklist

**File:** `src/cpu/z80/cpu_z80.cpp`

- [ ] Add `#include "xasm++/cpu/opcodes_z80.h"` at top
- [ ] Replace all `0x[0-9A-F][0-9A-F]` with `opcodes::*` constants
- [ ] Update CB-prefixed instructions to use `opcodes::prefix::CB`
- [ ] Update ED-prefixed instructions to use `opcodes::prefix::ED`
- [ ] Use helper functions for calculated opcodes (bit operations)
- [ ] Add comments for complex multi-byte instructions
- [ ] Verify no magic numbers remain: `grep -E "0x[0-9A-Fa-f]{2}" src/cpu/z80/*.cpp`
- [ ] Run all Z80 tests: `ctest -R z80`
- [ ] All tests pass

### 6809 Refactoring Checklist

**File:** `src/cpu/m6809/cpu_6809.cpp`

- [ ] Add `#include "xasm++/cpu/opcodes_6809.h"` at top
- [ ] Replace all single-byte opcodes with `opcodes::*` constants
- [ ] Replace page 2 instructions with `opcodes::prefix::PAGE2, opcodes::page2::*`
- [ ] Replace page 3 instructions with `opcodes::prefix::PAGE3, opcodes::page3::*`
- [ ] Update indexed addressing to use `opcodes::indexed::*` constants
- [ ] Use helper functions for post-byte construction
- [ ] Add comments explaining complex indexed modes
- [ ] Verify no magic numbers remain: `grep -E "0x[0-9A-Fa-f]{2}" src/cpu/m6809/*.cpp`
- [ ] Run all 6809 tests: `ctest -R 6809`
- [ ] All tests pass

---

## Common Pitfalls to Avoid

### Pitfall 1: Forgetting Inline Comments
```cpp
// ❌ BAD: No explanation
constexpr uint8_t BIT_0_A = 0x47;

// ✅ GOOD: Clear explanation
constexpr uint8_t BIT_0_A = 0x47;  // BIT 0, A (test bit 0 of accumulator A)
```

### Pitfall 2: Inconsistent Organization
```cpp
// ❌ BAD: Random order
constexpr uint8_t LD_A_n = 0x3E;
constexpr uint8_t ADD_A_n = 0xC6;
constexpr uint8_t LD_B_n = 0x06;  // Should be with other loads!

// ✅ GOOD: Grouped by category
// 8-bit Load Instructions
constexpr uint8_t LD_A_n = 0x3E;
constexpr uint8_t LD_B_n = 0x06;
// ... all loads together

// Arithmetic Instructions
constexpr uint8_t ADD_A_n = 0xC6;
// ... all arithmetic together
```

### Pitfall 3: Not Verifying Against Manual
```cpp
// ❌ BAD: Opcode wrong, not verified
constexpr uint8_t LD_A_n = 0x3F;  // WRONG! (0x3F is actually CCF)

// ✅ GOOD: Verified against Z80 manual page 123
constexpr uint8_t LD_A_n = 0x3E;  // LD A, n (verified: Z80 manual p.123)
```

### Pitfall 4: Missing Helper Functions
```cpp
// ❌ BAD: Magic formula in implementation
return {0xCB, 0x40 + (bit << 3) + reg};  // What is this calculating?

// ✅ GOOD: Use documented helper
return {opcodes::prefix::CB, 
        opcodes::cb::BitOpcode(opcodes::cb::BIT_BASE, bit, reg)};
// Formula explained in opcodes_z80.h
```

---

## Success Criteria

Your opcode headers are complete when:

✅ **All opcodes defined** as named constants
✅ **No magic numbers** in header (all are named constants)
✅ **Organized by category** (loads, arithmetic, branches, etc.)
✅ **Inline comments** explain each instruction
✅ **Helper functions** for calculated opcodes
✅ **References** to CPU manual included
✅ **File compiles** without errors or warnings
✅ **Verified** against official CPU reference manual (spot-check 20+ opcodes)
✅ **Follows established pattern** from opcodes_6502.h
✅ **Namespaces** used appropriately (prefix::, cb::, ed::, page2::, etc.)

---

## Time Estimates Summary

| Task | Z80 | 6809 | Total |
|------|-----|------|-------|
| File structure | 15m | 15m | 30m |
| Basic opcodes | 1.5h | 2h | 3.5h |
| Prefix/page opcodes | 2h | 2h | 4h |
| Helper functions | 15m | 30m | 45m |
| Verification | 30m | 30m | 1h |
| **TOTAL** | **4h** | **6h** | **10h** |

---

## Resources

### Z80 References
- **Zilog Z80 CPU User Manual** (UM008011-0816)
- Section 7: Instruction Set
- Appendix A: Opcode Map

### 6809 References
- **Motorola MC6809 Programming Manual** (M6809PM/AD)
- Section 3: Addressing Modes
- Section 4: Instruction Set
- Appendix A: Opcode Map

### Existing Code
- `include/xasm++/cpu/opcodes_6502.h` - Use as template
- `src/cpu/m6502/cpu_6502.cpp` - See how opcodes are used

---

## Questions?

If you encounter issues:

1. **Pattern unclear?** → Check `opcodes_6502.h` for examples
2. **Opcode uncertain?** → Verify against CPU reference manual
3. **Helper function needed?** → Look at existing helpers in `opcodes_6502.h`
4. **Organization question?** → Follow category structure from 6502

**Remember:** The goal is to eliminate magic numbers and create self-documenting code. When in doubt, add a comment!

---

**Good luck! This is high-impact work that will significantly improve code quality.**
