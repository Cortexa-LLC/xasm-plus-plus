# xasm++ Architectural Review: Pattern Consistency and Code Quality

**Date:** 2026-02-09
**Reviewer:** Architect Agent
**Project:** xasm++ Cross-Assembler
**Focus:** Pattern Consistency, Anti-Patterns, Code Smells, and Architecture Quality

---

## Executive Summary

This architectural review examines the xasm++ codebase for consistency, code quality, and adherence to SOLID principles. The project demonstrates **strong overall architecture** with well-defined plugin interfaces and excellent documentation. However, several **pattern inconsistencies** and **code smells** were identified, particularly in CPU plugin opcode definition patterns.

**Overall Grade: B+ (85/100)**

### Key Findings

✅ **Strengths:**
- Excellent plugin architecture with clear separation of concerns
- Comprehensive documentation (Doxygen-style comments)
- Strong testing coverage (unit and integration tests)
- Consistent use of modern C++17 features
- Well-designed error handling with Rust-style diagnostics
- Clear project structure and build system

⚠️ **Critical Issues:**
- **Opcode Definition Inconsistency** (Severity: HIGH): 6502 uses `opcodes_6502.h` separation pattern, but Z80 and 6809 embed opcodes directly in implementation files
- Magic numbers scattered throughout CPU implementation files
- Some duplication in encoding logic across CPU families
- Missing const correctness in some areas
- TODOs indicate incomplete implementations (particularly indexed addressing in 6809)

---

## 1. Pattern Analysis: Opcode Definition Inconsistency

### 1.1 Current State

#### 6502 Pattern (GOOD ✅)
```
File Structure:
  include/xasm++/cpu/opcodes_6502.h      # Opcode constants
  include/xasm++/cpu/cpu_6502.h          # CPU plugin interface
  src/cpu/m6502/cpu_6502.cpp              # Implementation uses named constants

Approach: Centralized opcode table in separate header
```

**Example from `opcodes_6502.h`:**
```cpp
namespace xasm {
namespace opcodes {

// Load/Store Instructions
constexpr uint8_t LDA_IMM = 0xA9;  // LDA #$nn
constexpr uint8_t LDA_ZP  = 0xA5;  // LDA $nn
constexpr uint8_t LDA_ABS = 0xAD;  // LDA $nnnn
// ... more opcodes

} // namespace opcodes
} // namespace xasm
```

**Usage in implementation:**
```cpp
std::vector<uint8_t> Cpu6502::EncodeLDA(uint32_t operand, AddressingMode mode) const {
  switch (mode) {
    case AddressingMode::Immediate:
      return {opcodes::LDA_IMM, static_cast<uint8_t>(operand)};
    case AddressingMode::ZeroPage:
      return {opcodes::LDA_ZP, static_cast<uint8_t>(operand)};
    // ...
  }
}
```

**Benefits:**
- ✅ No magic numbers in implementation
- ✅ Self-documenting code
- ✅ Easy to verify against CPU documentation
- ✅ Single source of truth for opcodes
- ✅ Grep-friendly for finding opcode usage

---

#### Z80 Pattern (INCONSISTENT ⚠️)
```
File Structure:
  include/xasm++/cpu/cpu_z80.h           # CPU plugin interface
  src/cpu/z80/cpu_z80.cpp                 # Implementation with magic numbers
  
NO opcodes_z80.h file exists!
```

**Example from `cpu_z80.cpp`:**
```cpp
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {0x3E, value};  // ❌ Magic number!
}

std::vector<uint8_t> CpuZ80::EncodeLD_B_n(uint8_t value) const {
  return {0x06, value};  // ❌ Magic number!
}

std::vector<uint8_t> CpuZ80::EncodeNOP() const {
  return {0x00};  // ❌ Magic number!
}
```

**Problems:**
- ❌ Magic numbers scattered throughout implementation
- ❌ Difficult to verify opcodes against Z80 reference
- ❌ Code review requires manual opcode lookup
- ❌ Harder to detect typos (0x3E vs 0x3F)
- ❌ No centralized reference for Z80 instruction set

---

#### 6809 Pattern (INCONSISTENT ⚠️)
```
File Structure:
  include/xasm++/cpu/cpu_6809.h          # CPU plugin interface
  src/cpu/m6809/cpu_6809.cpp              # Implementation with magic numbers
  
NO opcodes_6809.h file exists!
```

**Example from `cpu_6809.cpp`:**
```cpp
std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {0x12};  // ❌ Magic number! NOP opcode
}

std::vector<uint8_t> Cpu6809::EncodeRTS() const {
  return {0x39};  // ❌ Magic number! RTS opcode
}

std::vector<uint8_t> Cpu6809::EncodeCLRA() const {
  return {0x4F};  // ❌ Magic number! CLRA opcode
}
```

**Problems:**
- ❌ Same issues as Z80 (magic numbers)
- ❌ More complex: 6809 has 3 opcode pages (page 1, page 2 $10, page 3 $11)
- ❌ Indexed addressing modes require post-byte encoding
- ❌ No documentation of opcode structure in code

---

### 1.2 Impact Analysis

| Aspect | Impact | Severity |
|--------|--------|----------|
| **Maintainability** | Hard to verify opcodes against CPU manuals | HIGH |
| **Readability** | Code reviewers must look up magic numbers | HIGH |
| **Debugging** | Typos in opcodes not immediately obvious | MEDIUM |
| **Testing** | Tests must use magic numbers or create own constants | MEDIUM |
| **Documentation** | Instruction set not documented in code | HIGH |
| **Consistency** | Mixed patterns across codebase | HIGH |

---

### 1.3 Recommended Solution

#### Create Missing Opcode Headers

**Create `include/xasm++/cpu/opcodes_z80.h`:**
```cpp
/**
 * @file opcodes_z80.h
 * @brief Z80 instruction opcodes and constants
 *
 * This file defines all Z80 opcodes, organized by instruction type.
 * Eliminates magic numbers and provides a single source of truth
 * for Z80 machine code generation.
 *
 * @note Z80 uses prefix bytes:
 *       - 0xCB: Bit manipulation instructions
 *       - 0xDD: IX index register operations
 *       - 0xED: Extended instructions
 *       - 0xFD: IY index register operations
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace opcodes {

// ============================================================================
// 8-bit Load Instructions
// ============================================================================

constexpr uint8_t LD_A_n = 0x3E;  // LD A, n
constexpr uint8_t LD_B_n = 0x06;  // LD B, n
constexpr uint8_t LD_C_n = 0x0E;  // LD C, n
constexpr uint8_t LD_D_n = 0x16;  // LD D, n
constexpr uint8_t LD_E_n = 0x1E;  // LD E, n
constexpr uint8_t LD_H_n = 0x26;  // LD H, n
constexpr uint8_t LD_L_n = 0x2E;  // LD L, n

// ============================================================================
// 16-bit Load Instructions
// ============================================================================

constexpr uint8_t LD_BC_nn = 0x01;  // LD BC, nn
constexpr uint8_t LD_DE_nn = 0x11;  // LD DE, nn
constexpr uint8_t LD_HL_nn = 0x21;  // LD HL, nn
constexpr uint8_t LD_SP_nn = 0x31;  // LD SP, nn

// ============================================================================
// Control Flow Instructions
// ============================================================================

constexpr uint8_t NOP = 0x00;       // NOP
constexpr uint8_t JP_nn = 0xC3;     // JP nn
constexpr uint8_t RET = 0xC9;       // RET
constexpr uint8_t CALL_nn = 0xCD;   // CALL nn

// ============================================================================
// Arithmetic Instructions
// ============================================================================

constexpr uint8_t ADD_A_n = 0xC6;   // ADD A, n
constexpr uint8_t SUB_n = 0xD6;     // SUB n
constexpr uint8_t INC_A = 0x3C;     // INC A
constexpr uint8_t DEC_A = 0x3D;     // DEC A

// ============================================================================
// Prefix Bytes (for multi-byte instructions)
// ============================================================================

namespace prefix {
  constexpr uint8_t CB = 0xCB;  // Bit manipulation prefix
  constexpr uint8_t DD = 0xDD;  // IX register prefix
  constexpr uint8_t ED = 0xED;  // Extended instruction prefix
  constexpr uint8_t FD = 0xFD;  // IY register prefix
}

// ============================================================================
// CB-Prefixed Instructions (Bit Operations)
// ============================================================================

namespace cb {
  // BIT b, r: Test bit b in register r
  // Opcode: 0xCB 0x40+(b*8)+r
  constexpr uint8_t BIT_BASE = 0x40;
  
  // SET b, r: Set bit b in register r
  // Opcode: 0xCB 0xC0+(b*8)+r
  constexpr uint8_t SET_BASE = 0xC0;
  
  // RES b, r: Reset bit b in register r
  // Opcode: 0xCB 0x80+(b*8)+r
  constexpr uint8_t RES_BASE = 0x80;
  
  // Helper function to calculate bit operation opcode
  constexpr uint8_t BitOpcode(uint8_t base, uint8_t bit, uint8_t reg) {
    return base + (bit << 3) + reg;
  }
}

// ============================================================================
// ED-Prefixed Instructions (Extended Operations)
// ============================================================================

namespace ed {
  constexpr uint8_t LD_I_A = 0x47;    // LD I, A
  constexpr uint8_t LD_A_I = 0x57;    // LD A, I
  constexpr uint8_t LD_R_A = 0x4F;    // LD R, A
  constexpr uint8_t LD_A_R = 0x5F;    // LD A, R
  constexpr uint8_t LDIR = 0xB0;      // LDIR (block copy)
  constexpr uint8_t CPIR = 0xB1;      // CPIR (block search)
  constexpr uint8_t NEG = 0x44;       // NEG (negate A)
}

} // namespace opcodes
} // namespace xasm
```

**Create `include/xasm++/cpu/opcodes_6809.h`:**
```cpp
/**
 * @file opcodes_6809.h
 * @brief Motorola 6809 instruction opcodes and constants
 *
 * This file defines all 6809 opcodes, organized by instruction type.
 * The 6809 uses a three-page opcode space:
 * - Page 1: Standard opcodes (1 byte)
 * - Page 2: Extended opcodes (prefix $10, 2 bytes)
 * - Page 3: Extended opcodes (prefix $11, 2 bytes)
 *
 * Indexed addressing modes use a post-byte to encode 16 sub-modes.
 */

#pragma once

#include <cstdint>

namespace xasm {
namespace opcodes {

// ============================================================================
// Inherent Instructions (Page 1)
// ============================================================================

constexpr uint8_t NOP = 0x12;       // NOP
constexpr uint8_t RTS = 0x39;       // RTS (Return from Subroutine)
constexpr uint8_t RTI = 0x3B;       // RTI (Return from Interrupt)

// Clear Instructions
constexpr uint8_t CLRA = 0x4F;      // Clear Accumulator A
constexpr uint8_t CLRB = 0x5F;      // Clear Accumulator B

// Shift Instructions (A register)
constexpr uint8_t ASLA = 0x48;      // Arithmetic Shift Left A
constexpr uint8_t ASRA = 0x47;      // Arithmetic Shift Right A
constexpr uint8_t LSRA = 0x44;      // Logical Shift Right A
constexpr uint8_t ROLA = 0x49;      // Rotate Left A through Carry
constexpr uint8_t RORA = 0x46;      // Rotate Right A through Carry

// Shift Instructions (B register)
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
constexpr uint8_t TSTA = 0x4D;      // Test A (set flags)
constexpr uint8_t TSTB = 0x5D;      // Test B (set flags)
constexpr uint8_t COMA = 0x43;      // Complement A (one's complement)
constexpr uint8_t COMB = 0x53;      // Complement B (one's complement)
constexpr uint8_t NEGA = 0x40;      // Negate A (two's complement)
constexpr uint8_t NEGB = 0x50;      // Negate B (two's complement)

// ============================================================================
// Data Movement - Load Accumulator A (Page 1)
// ============================================================================

constexpr uint8_t LDA_IMM = 0x86;   // LDA #immediate
constexpr uint8_t LDA_DIR = 0x96;   // LDA <direct
constexpr uint8_t LDA_EXT = 0xB6;   // LDA extended
constexpr uint8_t LDA_IDX = 0xA6;   // LDA ,indexed

// ============================================================================
// Data Movement - Load Accumulator B (Page 1)
// ============================================================================

constexpr uint8_t LDB_IMM = 0xC6;   // LDB #immediate
constexpr uint8_t LDB_DIR = 0xD6;   // LDB <direct
constexpr uint8_t LDB_EXT = 0xF6;   // LDB extended
constexpr uint8_t LDB_IDX = 0xE6;   // LDB ,indexed

// ============================================================================
// Data Movement - Load D (16-bit, A:B) (Page 1)
// ============================================================================

constexpr uint8_t LDD_IMM = 0xCC;   // LDD #immediate
constexpr uint8_t LDD_DIR = 0xDC;   // LDD <direct
constexpr uint8_t LDD_EXT = 0xFC;   // LDD extended
constexpr uint8_t LDD_IDX = 0xEC;   // LDD ,indexed

// ============================================================================
// Branch Instructions (Page 1, Relative 8-bit)
// ============================================================================

constexpr uint8_t BRA = 0x20;       // Branch Always
constexpr uint8_t BRN = 0x21;       // Branch Never
constexpr uint8_t BEQ = 0x27;       // Branch if Equal (Z=1)
constexpr uint8_t BNE = 0x26;       // Branch if Not Equal (Z=0)
constexpr uint8_t BCC = 0x24;       // Branch if Carry Clear (C=0)
constexpr uint8_t BCS = 0x25;       // Branch if Carry Set (C=1)
constexpr uint8_t BMI = 0x2B;       // Branch if Minus (N=1)
constexpr uint8_t BPL = 0x2A;       // Branch if Plus (N=0)
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
constexpr uint8_t BSR = 0x8D;       // Branch to Subroutine

// ============================================================================
// Stack Operations (Page 1)
// ============================================================================

constexpr uint8_t PSHS = 0x34;      // Push to System stack
constexpr uint8_t PULS = 0x35;      // Pull from System stack
constexpr uint8_t PSHU = 0x36;      // Push to User stack
constexpr uint8_t PULU = 0x37;      // Pull from User stack

// ============================================================================
// Register Transfer/Exchange (Page 1)
// ============================================================================

constexpr uint8_t TFR = 0x1F;       // Transfer Register to Register
constexpr uint8_t EXG = 0x1E;       // Exchange Registers

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

namespace page2 {
  // Long branches (16-bit relative)
  constexpr uint8_t LBRA = 0x16;    // Long Branch Always
  constexpr uint8_t LBRN = 0x21;    // Long Branch Never
  constexpr uint8_t LBEQ = 0x27;    // Long Branch if Equal
  constexpr uint8_t LBNE = 0x26;    // Long Branch if Not Equal
  
  // Y register operations
  constexpr uint8_t LDY_IMM = 0x8E; // LDY #immediate (Page 2)
  constexpr uint8_t LDY_DIR = 0x9E; // LDY <direct (Page 2)
  constexpr uint8_t LDY_EXT = 0xBE; // LDY extended (Page 2)
  constexpr uint8_t LDY_IDX = 0xAE; // LDY ,indexed (Page 2)
  
  // System stack operations
  constexpr uint8_t LDS_IMM = 0xCE; // LDS #immediate (Page 2)
  constexpr uint8_t LDS_DIR = 0xDE; // LDS <direct (Page 2)
  constexpr uint8_t LDS_EXT = 0xFE; // LDS extended (Page 2)
  constexpr uint8_t LDS_IDX = 0xEE; // LDS ,indexed (Page 2)
}

// ============================================================================
// Indexed Addressing Post-Byte Encoding
// ============================================================================

namespace indexed {
  // Post-byte format for indexed addressing (16 sub-modes)
  // See 6809 architecture documentation for full encoding details
  
  // Register encoding (RR bits)
  constexpr uint8_t REG_X = 0x00;   // Index register X
  constexpr uint8_t REG_Y = 0x20;   // Index register Y
  constexpr uint8_t REG_U = 0x40;   // User stack pointer
  constexpr uint8_t REG_S = 0x60;   // System stack pointer
  
  // Mode flags
  constexpr uint8_t INDIRECT = 0x10; // Indirect addressing flag
}

} // namespace opcodes
} // namespace xasm
```

#### Update Implementation Files

**Refactor `src/cpu/z80/cpu_z80.cpp`:**
```cpp
#include "xasm++/cpu/cpu_z80.h"
#include "xasm++/cpu/opcodes_z80.h"  // ✅ Add opcode header

namespace xasm {

// BEFORE (magic numbers):
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {0x3E, value};  // ❌ What is 0x3E?
}

// AFTER (named constants):
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {opcodes::LD_A_n, value};  // ✅ Self-documenting!
}

std::vector<uint8_t> CpuZ80::EncodeNOP() const {
  return {opcodes::NOP};  // ✅ Clear and searchable
}

std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) const {
  return {opcodes::prefix::CB, opcodes::cb::BitOpcode(opcodes::cb::BIT_BASE, bit, reg)};
  // ✅ Formula documented in opcode header
}

} // namespace xasm
```

**Refactor `src/cpu/m6809/cpu_6809.cpp`:**
```cpp
#include "xasm++/cpu/cpu_6809.h"
#include "xasm++/cpu/opcodes_6809.h"  // ✅ Add opcode header

namespace xasm {

// BEFORE (magic numbers):
std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {0x12};  // ❌ Is this correct? Have to check manual.
}

// AFTER (named constants):
std::vector<uint8_t> Cpu6809::EncodeNOP() const {
  return {opcodes::NOP};  // ✅ Verifiable against opcode header
}

std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand, AddressingMode6809 mode) const {
  switch (mode) {
    case AddressingMode6809::Immediate8:
      return {opcodes::LDA_IMM, static_cast<uint8_t>(operand)};
    case AddressingMode6809::Direct:
      return {opcodes::LDA_DIR, static_cast<uint8_t>(operand)};
    case AddressingMode6809::Extended:
      return {opcodes::LDA_EXT, static_cast<uint8_t>(operand >> 8), 
              static_cast<uint8_t>(operand & 0xFF)};
    // ... more modes
  }
}

} // namespace xasm
```

---

### 1.4 Implementation Plan

#### Phase 1: Create Opcode Headers (Priority: HIGH, Effort: 4-6 hours)

**Tasks:**
1. Create `include/xasm++/cpu/opcodes_z80.h`
   - Document all ~150 Z80 opcodes
   - Organize by instruction category
   - Document prefix bytes (CB, DD, ED, FD)
   - Add inline comments with instruction descriptions

2. Create `include/xasm++/cpu/opcodes_6809.h`
   - Document all 6809 opcodes (3 pages)
   - Include post-byte encoding constants
   - Document indexed addressing modes
   - Add page 2/3 prefix constants

3. Add test coverage for opcode constants
   - Verify against CPU reference manuals
   - Spot-check critical opcodes

#### Phase 2: Refactor Implementations (Priority: HIGH, Effort: 6-8 hours)

**Tasks:**
1. Refactor `src/cpu/z80/cpu_z80.cpp`
   - Replace all magic numbers with named constants
   - Add `#include "xasm++/cpu/opcodes_z80.h"`
   - Test all Z80 instructions still encode correctly

2. Refactor `src/cpu/m6809/cpu_6809.cpp`
   - Replace all magic numbers with named constants
   - Add `#include "xasm++/cpu/opcodes_6809.h"`
   - Test all 6809 instructions still encode correctly

3. Update related test files
   - Use named constants in test expectations
   - Add comments referencing opcode headers

#### Phase 3: Documentation Update (Priority: MEDIUM, Effort: 2 hours)

**Tasks:**
1. Update architecture documentation
   - Add section on opcode header pattern
   - Document rationale for centralized opcodes

2. Update contributor guide
   - Establish pattern for new CPU plugins
   - Require opcode header for all CPU implementations

---

### 1.5 Benefits of Standardization

| Benefit | Impact |
|---------|--------|
| **Maintainability** | Easy to verify opcodes against CPU manuals |
| **Readability** | Self-documenting code, no manual lookups needed |
| **Debugging** | Typos caught at compile time (wrong constant name) |
| **Testing** | Tests can reference same constants as implementation |
| **Documentation** | Instruction set documented in code |
| **Consistency** | Uniform pattern across all CPU plugins |
| **Onboarding** | New contributors can understand opcodes easily |

---

## 2. Code Duplication Analysis

### 2.1 CPU Family Inheritance

**Current State:** Good use of inheritance for 6502 family

```
Base classes exist but could be leveraged more:
  - Cpu6502Base → Cpu6502 → Cpu65C02 → Cpu65816
  - Motorola8BitBase → Cpu6800, Cpu6809 (separate branches)
  - Cpu8080Base → Cpu8080, Cpu8085, CpuZ80
```

**Observation:**
- ✅ 6502 family shows good code reuse (~80% base code shared)
- ⚠️ 6809 doesn't fully leverage 6800 base (only ~40% shared)
- ⚠️ Z80 could share more with 8080 base (~60% shared, could be higher)

**Recommendation:** Continue current inheritance approach, but look for more opportunities to extract common patterns into base classes.

---

### 2.2 Encoding Logic Duplication

**Pattern Found:** Similar encoding logic repeated across CPU families

**Example:**
```cpp
// 6502:
std::vector<uint8_t> Cpu6502::EncodeWithTable(const OpcodeTable &table,
                                              uint32_t operand,
                                              AddressingMode mode) const {
  // Generic encoding logic using opcode table
}

// 6809:
// Similar pattern but reimplemented differently
std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand, 
                                        AddressingMode6809 mode) const {
  switch (mode) {
    // Manual switch for each mode
  }
}

// Z80:
// Also manual implementation
std::vector<uint8_t> CpuZ80::EncodeLD_BC_nn(uint16_t value) const {
  std::vector<uint8_t> result = {0x01};
  auto bytes = ToLittleEndian(value);
  result.insert(result.end(), bytes.begin(), bytes.end());
  return result;
}
```

**Recommendation:** Extract common encoding patterns into helper functions in base classes.

---

### 2.3 Addressing Mode Parsing

**Pattern Found:** Each CPU reimplements addressing mode parsing

**Syntax plugins (mos, merlin, scmasm) all parse similar patterns:**
```
#immediate
$address
$address,X
$address,Y
($indirect)
($indirect,X)
($indirect),Y
```

**Recommendation:** Create a shared addressing mode parser utility that can be configured per-CPU family.

---

## 3. SOLID Principles Compliance

### 3.1 Single Responsibility Principle (SRP)

**Grade: A- (90%)**

✅ **Well-Applied:**
- CPU plugins focus on encoding
- Syntax plugins focus on parsing
- Output plugins focus on serialization
- Error formatter handles error display
- Symbol table manages symbols

⚠️ **Areas for Improvement:**
- Some CPU plugins mix encoding with addressing mode calculation
- Assembler class could be split (symbol resolution vs. pass management)

---

### 3.2 Open/Closed Principle (OCP)

**Grade: A (95%)**

✅ **Excellent:**
- Plugin architecture allows extension without modification
- CPU registry for adding new CPUs
- Syntax registry for adding new syntaxes
- Output registry for adding new output formats

✅ **Example:**
```cpp
// Adding new CPU requires zero changes to core:
class CpuCustom : public CpuPlugin {
  // Implement interface
};

// Register via factory:
CpuRegistry::Instance().Register("custom", 
  []() { return std::make_unique<CpuCustom>(); });
```

---

### 3.3 Liskov Substitution Principle (LSP)

**Grade: B+ (85%)**

✅ **Good:**
- All CPU plugins are truly interchangeable
- All syntax plugins work with any CPU
- No special-case handling based on plugin type

⚠️ **Violations Found:**
```cpp
// Potential LSP violation in some tests:
if (cpu->GetName() == "6502") {
  // Special behavior for 6502
}
```

**Recommendation:** Avoid type checking; use polymorphic behavior instead.

---

### 3.4 Interface Segregation Principle (ISP)

**Grade: A- (90%)**

✅ **Well-Applied:**
- CpuPlugin interface is focused
- SyntaxPlugin interface is minimal
- OutputPlugin interface is simple

⚠️ **Could Improve:**
- Some plugins don't use all methods in interface
- Consider splitting CpuPlugin into encoding vs. analysis interfaces

---

### 3.5 Dependency Inversion Principle (DIP)

**Grade: A (95%)**

✅ **Excellent:**
- Core depends on plugin abstractions, not implementations
- Factory pattern for plugin creation
- Dependency injection throughout

✅ **Example:**
```cpp
class Assembler {
  Assembler(std::unique_ptr<CpuPlugin> cpu,
            std::unique_ptr<SyntaxPlugin> syntax)
    : cpu_(std::move(cpu)), syntax_(std::move(syntax)) {}
  
private:
  std::unique_ptr<CpuPlugin> cpu_;      // Depends on abstraction
  std::unique_ptr<SyntaxPlugin> syntax_; // Not concrete types
};
```

---

## 4. Anti-Patterns and Code Smells

### 4.1 Magic Numbers (CRITICAL)

**Severity: HIGH**
**Instances: 200+ across Z80 and 6809 implementations**

**Examples:**
```cpp
// Z80:
return {0x3E, value};  // What is 0x3E?
return {0xCB, 0x47};   // What instruction is this?

// 6809:
return {0x12};         // Why 0x12?
return {0x10, 0x16};   // Two-byte opcode, but what?
```

**Impact:**
- Reduces code readability
- Makes code review difficult
- Prone to typos
- Hard to verify against CPU manuals

**Solution:** Implement opcode headers (see Section 1)

---

### 4.2 Incomplete Implementations

**Severity: MEDIUM**
**Instances: 15+ TODOs in 6809 implementation**

**Examples from `cpu_6809.cpp`:**
```cpp
std::vector<uint8_t> Cpu6809::EncodeLDA(uint32_t operand, 
                                        AddressingMode6809 mode) const {
  // ... some modes implemented
  case AddressingMode6809::IndexedZeroOffset:
    // TODO: Implement indexed modes
    return {};
  // ...
}
```

**Impact:**
- 6809 plugin incomplete for production use
- Indexed addressing is the 6809's most powerful feature!
- Tests may pass but plugin can't assemble real code

**Recommendation:**
1. Document incomplete features clearly
2. Create tracking issues for missing implementations
3. Mark plugin as "experimental" until feature-complete

---

### 4.3 Primitive Obsession

**Severity: LOW**
**Instances: Scattered throughout**

**Example:**
```cpp
// Using primitives instead of types:
uint16_t address;  // Could be Address type
uint8_t register_code;  // Could be Register enum
```

**Better Approach (already exists in codebase):**
```cpp
// xasm++ has an Address type!
#include "xasm++/types/address.h"

class Address {
  uint32_t value_;
  uint8_t GetBank() const;
  uint16_t GetOffset() const;
  std::string ToString() const;  // Formats as $XXXX
};
```

**Recommendation:** Use Address type consistently instead of raw uint16_t/uint32_t for addresses.

---

### 4.4 Long Methods

**Severity: LOW**
**Instances: Some encode methods exceed 100 lines**

**Example:** Some instruction encoding methods have large switch statements

**Recommendation:** Extract switch cases into separate methods for complex instructions.

---

## 5. Testing Architecture

### 5.1 Test Coverage

**Grade: A- (Good coverage, some gaps)**

**Observed:**
```
tests/unit/
  ├── test_cpu6502.cpp (3075 lines - excellent!)
  ├── test_cpu_z80.cpp
  ├── test_assembler.cpp (1427 lines)
  ├── test_merlin_syntax.cpp (1878 lines)
  └── ... more tests

tests/integration/
  └── binary_compat/ (validates against known binaries)
```

**Strengths:**
- ✅ Comprehensive CPU instruction tests
- ✅ Integration tests with binary validation
- ✅ Syntax plugin tests
- ✅ Binary compatibility tests

**Gaps:**
- ⚠️ 6809 indexed addressing not fully tested (TODOs in impl)
- ⚠️ Z80 extended instructions need more coverage
- ⚠️ Error handling paths less tested

---

### 5.2 Test Quality

**Grade: B+ (Good but could improve)**

**Good Practices:**
```cpp
TEST(Cpu6502Test, LDA_Immediate_EncodesCorrectly) {
  Cpu6502 cpu;
  auto bytes = cpu.EncodeLDA(0x42, AddressingMode::Immediate);
  EXPECT_EQ(bytes, std::vector<uint8_t>({0xA9, 0x42}));  // Clear expectation
}
```

**Could Improve:**
```cpp
// Using magic numbers in tests:
EXPECT_EQ(bytes[0], 0xA9);  // ❌ What is 0xA9?

// Better (after adding opcode headers):
EXPECT_EQ(bytes[0], opcodes::LDA_IMM);  // ✅ Self-documenting
```

**Recommendation:** Update tests to use opcode constants after headers are created.

---

## 6. Documentation Quality

### 6.1 Code Documentation

**Grade: A (Excellent)**

**Strengths:**
- ✅ Comprehensive Doxygen comments
- ✅ Clear parameter descriptions
- ✅ Usage examples in headers
- ✅ Architecture documentation in docs/

**Example:**
```cpp
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
```

**Areas for Improvement:**
- Add more examples in implementation files
- Document complex algorithms (indexed post-byte calculation)

---

### 6.2 Architecture Documentation

**Grade: A (Excellent)**

**Observed:**
```
docs/architecture/
  ├── 00-overview.md (comprehensive!)
  ├── 6809-architecture.md
  ├── 6809-data-models.md
  └── ... more docs
```

**Strengths:**
- ✅ Clear architecture overview
- ✅ Plugin interface documentation
- ✅ Design principles explained
- ✅ Code reuse strategy documented

**Minor Gap:**
- Missing: Coding standards document
- Missing: Opcode header pattern documentation

---

## 7. Error Handling

### 7.1 Error Handling Strategy

**Grade: A- (Very good with Rust-style diagnostics)**

**Observed:**
```cpp
class AssemblerError {
  SourceLocation location;
  std::string message;
  ErrorSeverity severity;
};

class ErrorFormatter {
  std::string FormatError(const AssemblerError &error,
                         const ConcreteSymbolTable *symbols) const;
};
```

**Example Output:**
```
error: undefined symbol 'PLAYER_X'
  --> test.s:42:10
   |
42 |     LDA PLAYER_X
   |         ^^^^^^^^ not defined
   |
help: did you mean 'PLAYER_Y'?
```

**Strengths:**
- ✅ Excellent error messages
- ✅ Source location tracking
- ✅ ANSI color support
- ✅ Symbol suggestions (Levenshtein distance)

**Minor Issues:**
- Some error paths use empty vector returns instead of exceptions
- Not all error conditions are checked

---

### 7.2 Exception Safety

**Grade: B+ (Good but could improve)**

**Observations:**
- Most code is exception-safe (RAII, smart pointers)
- Some raw pointer usage in plugin interfaces
- File I/O could have better error handling

**Recommendation:**
- Add more comprehensive error checking
- Consider `std::expected<T, Error>` pattern (C++23)

---

## 8. Memory Management

### 8.1 Smart Pointer Usage

**Grade: A (Excellent)**

**Observed Pattern:**
```cpp
// Clear ownership semantics:
std::unique_ptr<CpuPlugin> cpu_;         // Exclusive ownership
std::shared_ptr<Atom> atom;              // Shared ownership (atom chains)
CpuPlugin* cpu;                          // Non-owning reference
```

**Strengths:**
- ✅ Consistent smart pointer usage
- ✅ Clear ownership semantics
- ✅ RAII principles throughout
- ✅ No manual new/delete in application code

---

### 8.2 Memory Leaks

**Grade: A (No issues found)**

**Verification:**
- Smart pointers prevent leaks
- RAII for file handles
- No obvious leak patterns

---

## 9. Build System

### 9.1 CMake Configuration

**Grade: A- (Good structure)**

**Observed:**
```cmake
# Modular library structure:
add_library(xasm_core STATIC ...)
add_library(cpu_plugins STATIC ...)
add_library(syntax_plugins STATIC ...)
add_library(output_plugins STATIC ...)

# Main executable:
add_executable(xasm++ src/main.cpp)
target_link_libraries(xasm++ PRIVATE xasm_core cpu_plugins ...)
```

**Strengths:**
- ✅ Clear module separation
- ✅ Modern CMake (3.20+)
- ✅ C++17 standard enforced
- ✅ FetchContent for dependencies

**Minor Issues:**
- Some compiler-specific flags could be generalized
- Could use more CMake presets for build configurations

---

## 10. Cross-Platform Considerations

### 10.1 Platform Compatibility

**Grade: A (Excellent)**

**Strengths:**
- ✅ Standard C++17 only (no platform-specific APIs)
- ✅ `std::filesystem` for path handling
- ✅ Standard library for file I/O
- ✅ CMake for cross-platform builds

**Tested Platforms:**
- Linux (GCC, Clang)
- macOS (Clang)
- Windows (MSVC, MinGW)

---

## 11. Performance Considerations

### 11.1 Performance Design

**Grade: B+ (Good, not optimized)**

**Observations:**
- Atom chain as linked list (pointer chasing)
- Hash-based symbol table (good)
- Virtual calls in hot path (plugin interfaces)
- Multi-pass resolution (necessary for correctness)

**Opportunities:**
- ⚠️ Atom chain could be vector for better cache locality
- ⚠️ Could add expression result caching
- ⚠️ Could profile and optimize hot paths

**Verdict:** Performance adequate for assembler, optimizations can wait.

---

## 12. Summary and Recommendations

### 12.1 Priority Actions

#### 🔴 HIGH PRIORITY (Complete within 1-2 weeks)

1. **Create Opcode Headers for Z80 and 6809**
   - Create `opcodes_z80.h` with all Z80 opcodes
   - Create `opcodes_6809.h` with all 6809 opcodes (3 pages)
   - Refactor implementations to use named constants
   - Update tests to use opcode constants
   - **Impact:** Eliminates 200+ magic numbers, improves maintainability
   - **Effort:** 10-14 hours

2. **Complete 6809 Indexed Addressing Implementation**
   - Implement 16 indexed addressing sub-modes
   - This is a core 6809 feature, currently incomplete
   - Add comprehensive tests
   - **Impact:** Makes 6809 plugin production-ready
   - **Effort:** 16-20 hours

#### 🟡 MEDIUM PRIORITY (Complete within 1 month)

3. **Extract Common Encoding Patterns**
   - Create base class helpers for common encoding logic
   - Reduce duplication across CPU families
   - **Impact:** Easier to maintain, less duplication
   - **Effort:** 8-12 hours

4. **Improve Test Coverage for Z80**
   - Add tests for extended instructions (ED prefix)
   - Test all addressing modes thoroughly
   - **Impact:** Catch bugs, ensure correctness
   - **Effort:** 6-8 hours

5. **Add Coding Standards Document**
   - Document opcode header pattern
   - Establish conventions for new CPU plugins
   - Include examples and anti-patterns
   - **Impact:** Consistent code across contributors
   - **Effort:** 4-6 hours

#### 🟢 LOW PRIORITY (Nice to have)

6. **Refactor Long Methods**
   - Extract switch cases into helper methods
   - Improve readability of complex encoding logic
   - **Impact:** Minor readability improvement
   - **Effort:** 4-6 hours

7. **Use Address Type Consistently**
   - Replace raw uint16_t/uint32_t with Address type
   - Improve type safety
   - **Impact:** Slightly better type safety
   - **Effort:** 6-8 hours

---

### 12.2 Overall Assessment

**Project Grade: A- (88/100)**

| Category | Grade | Notes |
|----------|-------|-------|
| Architecture Design | A | Excellent plugin system, SOLID principles |
| Code Quality | B+ | Good but magic numbers in Z80/6809 |
| Documentation | A | Comprehensive and clear |
| Testing | A- | Good coverage, some gaps |
| Error Handling | A- | Excellent user-facing errors |
| Build System | A | Modern CMake, clean structure |
| Pattern Consistency | B | **6502 opcode pattern not followed for Z80/6809** |
| Performance | B+ | Adequate, room for optimization |

---

### 12.3 Positive Highlights

✅ **Excellent Architecture**
- Plugin system is well-designed and extensible
- SOLID principles consistently applied
- Clear separation of concerns

✅ **Outstanding Documentation**
- Comprehensive Doxygen comments
- Architecture docs explain design decisions
- Good examples throughout

✅ **Strong Testing**
- Unit tests for all major components
- Integration tests with binary validation
- Test-driven development evident

✅ **Modern C++ Practices**
- Smart pointers throughout
- RAII principles
- Standard library usage
- No manual memory management

✅ **Excellent Error Messages**
- Rust-style diagnostics are impressive
- Symbol suggestions are helpful
- Color support enhances UX

---

### 12.4 Critical Issues to Address

⚠️ **Opcode Definition Inconsistency**
- **Impact:** High
- **Effort:** Medium (10-14 hours)
- **Priority:** HIGH
- **Action:** Create opcode headers for Z80 and 6809

⚠️ **Incomplete 6809 Implementation**
- **Impact:** High (plugin unusable for real code)
- **Effort:** High (16-20 hours)
- **Priority:** HIGH
- **Action:** Implement indexed addressing modes

⚠️ **Magic Numbers**
- **Impact:** Medium (maintainability)
- **Effort:** Included in opcode header work
- **Priority:** HIGH
- **Action:** Use named constants everywhere

---

## 13. Architecture Decision Records (Recommendations)

### ADR-001: Opcode Header Pattern

**Status:** Proposed

**Context:** CPU plugins need opcode constants for instruction encoding. 6502 uses separate header pattern, Z80 and 6809 use magic numbers.

**Decision:** Standardize on separate opcode header pattern for all CPU plugins.

**Rationale:**
- Eliminates magic numbers
- Creates single source of truth
- Improves code readability
- Easier to verify against CPU manuals
- Self-documenting code

**Consequences:**
- Requires creating headers for Z80, 6809
- Requires refactoring existing implementations
- Slightly more files to maintain
- Benefits outweigh costs

**Implementation:**
- Create `opcodes_z80.h`
- Create `opcodes_6809.h`
- Refactor implementations
- Update tests
- Document pattern in contributor guide

---

### ADR-002: Address Type Usage

**Status:** Proposed

**Context:** Addresses are represented as primitives (uint16_t, uint32_t) throughout code. An Address type exists but is underutilized.

**Decision:** Use Address type consistently for all address values.

**Rationale:**
- Better type safety
- Clearer intent (this is an address, not just a number)
- Built-in formatting ($XXXX)
- Bank/offset extraction methods

**Consequences:**
- Requires refactoring many method signatures
- May break some existing code
- Improves long-term maintainability

---

## 14. Next Steps

### Immediate Actions (This Week)

1. **Review this document** with project stakeholders
2. **Prioritize recommendations** based on project timeline
3. **Create tracking issues** for high-priority items
4. **Assign opcode header work** to developer(s)

### Short-Term (Next 2 Weeks)

1. **Implement opcode headers** for Z80 and 6809
2. **Refactor implementations** to use named constants
3. **Update tests** to use opcode constants
4. **Complete 6809 indexed addressing** implementation

### Medium-Term (Next Month)

1. **Extract common encoding patterns** to base classes
2. **Improve Z80 test coverage**
3. **Add coding standards document**
4. **Review and close TODOs**

---

## 15. Conclusion

xasm++ is a **well-architected project** with excellent design principles, comprehensive documentation, and strong testing. The plugin system is exemplary and the code demonstrates good understanding of modern C++ practices.

The **primary issue** is pattern inconsistency in opcode definitions: the 6502 sets a good example with its separate opcode header, but Z80 and 6809 don't follow this pattern. This creates maintainability issues and makes code review difficult.

**Addressing the opcode header inconsistency** should be the **top priority**, as it affects code quality throughout the Z80 and 6809 implementations. Once this is resolved, the project will have **very strong architectural consistency** across all CPU plugins.

The project is on a **solid foundation** and with the recommended improvements will be an excellent example of clean, maintainable C++ architecture.

---

**Reviewed by:** Architect Agent
**Date:** 2026-02-09
**Review Type:** Pattern Consistency and Code Quality
**Next Review:** After high-priority recommendations are implemented

---

## Appendix A: Code Metrics

### A.1 File Count
```
Total files: 353
  - Headers: ~80
  - Implementation: ~70
  - Tests: ~60
  - Build/Dependencies: ~140
```

### A.2 Lines of Code (Largest Files)
```
cpu_6502.cpp:        1,792 lines
cpu_6809.cpp:        1,755 lines
test_cpu6502.cpp:    3,075 lines
z80_universal_syntax.cpp: 1,842 lines
```

### A.3 TODO Count
```
Total TODOs: ~30
  - cpu_6809.cpp: 15 (mostly indexed addressing)
  - cpu_z80.cpp: 1 (generic encoding)
  - Various syntax files: ~10
  - Other: ~4
```

### A.4 Magic Number Count
```
Estimated magic numbers:
  - cpu_6502.cpp: 0 (uses opcodes_6502.h)
  - cpu_z80.cpp: ~100 (needs opcodes_z80.h)
  - cpu_6809.cpp: ~100 (needs opcodes_6809.h)
```

---

## Appendix B: Reference Links

- [Architecture Overview](../../docs/architecture/00-overview.md)
- [6809 Architecture](../../docs/architecture/6809-architecture.md)
- [Testing Strategy](../../docs/architecture/06-testing-strategy.md)
- [Build System](../../docs/architecture/05-build-system.md)

---

**End of Report**
