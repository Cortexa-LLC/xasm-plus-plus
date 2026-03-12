# Pattern Consistency Diagram: xasm++ CPU Plugins

## Current State: Opcode Definition Patterns

### 6502 Family (CONSISTENT ✅)

```
┌─────────────────────────────────────────────────────────────────┐
│                          6502 Plugin                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  include/xasm++/cpu/                                            │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  opcodes_6502.h                                           │ │
│  │  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│ │
│  │  namespace opcodes {                                      │ │
│  │    constexpr uint8_t LDA_IMM = 0xA9;  // LDA #$nn        │ │
│  │    constexpr uint8_t LDA_ZP  = 0xA5;  // LDA $nn         │ │
│  │    constexpr uint8_t LDA_ABS = 0xAD;  // LDA $nnnn       │ │
│  │    constexpr uint8_t STA_ZP  = 0x85;  // STA $nn         │ │
│  │    constexpr uint8_t STA_ABS = 0x8D;  // STA $nnnn       │ │
│  │    // ... ~150 more opcodes                              │ │
│  │  }                                                        │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              ▲                                  │
│                              │                                  │
│                              │ #include                         │
│                              │                                  │
│  src/cpu/m6502/                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  cpu_6502.cpp                                             │ │
│  │  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│ │
│  │  #include "xasm++/cpu/opcodes_6502.h"                    │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> Cpu6502::EncodeLDA(...) {          │ │
│  │    switch (mode) {                                       │ │
│  │      case Immediate:                                     │ │
│  │        return {opcodes::LDA_IMM, operand};  ✅           │ │
│  │      case ZeroPage:                                      │ │
│  │        return {opcodes::LDA_ZP, operand};   ✅           │ │
│  │      case Absolute:                                      │ │
│  │        return {opcodes::LDA_ABS, lo, hi};   ✅           │ │
│  │    }                                                     │ │
│  │  }                                                       │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ✅ BENEFITS:                                                   │
│    • No magic numbers                                           │
│    • Self-documenting code                                      │
│    • Single source of truth                                     │
│    • Easy to verify against CPU manual                          │
│    • Grep-friendly                                              │
└─────────────────────────────────────────────────────────────────┘
```

---

### Z80 Plugin (INCONSISTENT ⚠️)

```
┌─────────────────────────────────────────────────────────────────┐
│                          Z80 Plugin                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  include/xasm++/cpu/                                            │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  ❌ opcodes_z80.h DOES NOT EXIST                          │ │
│  │                                                           │ │
│  │  PROBLEM: No centralized opcode definitions               │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              ✗ (missing)                        │
│                              │                                  │
│                              │                                  │
│  src/cpu/z80/                                                   │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  cpu_z80.cpp                                              │ │
│  │  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│ │
│  │  // No opcode header included                            │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> CpuZ80::EncodeLD_A_n(...) {        │ │
│  │    return {0x3E, value};  ❌ MAGIC NUMBER                │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> CpuZ80::EncodeLD_B_n(...) {        │ │
│  │    return {0x06, value};  ❌ MAGIC NUMBER                │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> CpuZ80::EncodeNOP() {              │ │
│  │    return {0x00};  ❌ MAGIC NUMBER                       │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> CpuZ80::EncodeBIT(...) {           │ │
│  │    return {0xCB, 0x40 + (bit << 3) + reg};  ❌ MAGIC    │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  // ~100 more methods with magic numbers...              │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ❌ PROBLEMS:                                                   │
│    • 100+ magic numbers scattered in code                       │
│    • Hard to verify opcodes (manual lookup required)            │
│    • Difficult code review                                      │
│    • Typos not caught (0x3E vs 0x3F looks similar)              │
│    • No documentation of Z80 instruction set                    │
└─────────────────────────────────────────────────────────────────┘
```

---

### 6809 Plugin (INCONSISTENT ⚠️)

```
┌─────────────────────────────────────────────────────────────────┐
│                         6809 Plugin                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  include/xasm++/cpu/                                            │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  ❌ opcodes_6809.h DOES NOT EXIST                         │ │
│  │                                                           │ │
│  │  PROBLEM: 6809 has 3 opcode pages!                       │ │
│  │    - Page 1: Standard opcodes                            │ │
│  │    - Page 2: Prefix $10 + opcode                         │ │
│  │    - Page 3: Prefix $11 + opcode                         │ │
│  │                                                           │ │
│  │  IMPACT: Even MORE complex than Z80, needs documentation │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              ✗ (missing)                        │
│                              │                                  │
│                              │                                  │
│  src/cpu/m6809/                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  cpu_6809.cpp                                             │ │
│  │  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│ │
│  │  // No opcode header included                            │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> Cpu6809::EncodeNOP() {             │ │
│  │    return {0x12};  ❌ MAGIC NUMBER                       │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> Cpu6809::EncodeRTS() {             │ │
│  │    return {0x39};  ❌ MAGIC NUMBER                       │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> Cpu6809::EncodeCLRA() {            │ │
│  │    return {0x4F};  ❌ MAGIC NUMBER                       │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  std::vector<uint8_t> Cpu6809::EncodeLDA(...) {          │ │
│  │    switch (mode) {                                       │ │
│  │      case Immediate:                                     │ │
│  │        return {0x86, operand};  ❌ MAGIC                 │ │
│  │      case Direct:                                        │ │
│  │        return {0x96, operand};  ❌ MAGIC                 │ │
│  │      case Extended:                                      │ │
│  │        return {0xB6, hi, lo};   ❌ MAGIC                 │ │
│  │      // ... 15 TODOs for indexed modes                  │ │
│  │    }                                                     │ │
│  │  }                                                       │ │
│  │                                                           │ │
│  │  // Page 2 opcodes (prefix $10):                        │ │
│  │  std::vector<uint8_t> Cpu6809::EncodeLBRA(...) {         │ │
│  │    return {0x10, 0x16, hi, lo};  ❌ MAGIC NUMBERS       │ │
│  │  }                                                        │ │
│  │                                                           │ │
│  │  // ~100 more methods with magic numbers...              │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ❌ PROBLEMS:                                                   │
│    • 100+ magic numbers (including multi-byte prefixes)         │
│    • 3-page opcode structure not documented                     │
│    • Indexed post-byte calculation buried in code               │
│    • Hard to verify against 6809 reference manual               │
│    • 15+ TODOs indicate incomplete implementation               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Target State: Consistent Pattern Across All CPUs

```
┌─────────────────────────────────────────────────────────────────┐
│                 ALL CPU PLUGINS (Consistent)                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  include/xasm++/cpu/                                            │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  opcodes_6502.h    ✅ Already exists                     │ │
│  │  opcodes_z80.h     📝 CREATE THIS                        │ │
│  │  opcodes_6809.h    📝 CREATE THIS                        │ │
│  │  opcodes_6800.h    📝 Future (for consistency)           │ │
│  │  opcodes_68k.h     📝 Future (for consistency)           │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              ▲                                  │
│                              │ #include (all implementations)   │
│                              │                                  │
│  src/cpu/*/                                                     │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  All CPU implementations use:                             │ │
│  │    • Named opcode constants (no magic numbers)            │ │
│  │    • Consistent pattern across all CPUs                   │ │
│  │    • Self-documenting code                                │ │
│  │    • Easy verification against CPU manuals                │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ✅ BENEFITS OF CONSISTENCY:                                    │
│    • Uniform codebase across all CPU families                   │
│    • New contributors easily understand pattern                 │
│    • Code reviews faster (no manual opcode lookup)              │
│    • Less error-prone (typos caught by wrong constant name)     │
│    • Instruction sets documented in code                        │
│    • Tests can reference same constants                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## Impact Analysis: Magic Number Problem

### Example: Code Review Difficulty

**WITHOUT OPCODE HEADER (Current Z80):**
```cpp
// Reviewer must manually look up what 0xCB 0x47 means:
std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) const {
  return {0xCB, 0x47};  // ❓ What instruction is this?
}

// Reviewer checks Z80 manual:
// 0xCB = bit operation prefix
// 0x47 = BIT 0,A instruction
// (5 minutes wasted per opcode during review)
```

**WITH OPCODE HEADER (Target):**
```cpp
// Reviewer immediately understands:
std::vector<uint8_t> CpuZ80::EncodeBIT(uint8_t bit, uint8_t reg) const {
  return {opcodes::prefix::CB, 
          opcodes::cb::BitOpcode(opcodes::cb::BIT_BASE, bit, reg)};
  // ✅ Clear: This is a CB-prefixed bit operation
  // ✅ Formula documented in opcodes_z80.h
}
```

---

### Example: Bug Detection

**WITHOUT OPCODE HEADER:**
```cpp
// Typo not caught (0x3E vs 0x3F look similar):
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {0x3F, value};  // ❌ BUG! Should be 0x3E
  // 0x3F is actually CCF (Complement Carry Flag)
  // Bug only found during runtime testing
}
```

**WITH OPCODE HEADER:**
```cpp
// Typo caught at compile time:
std::vector<uint8_t> CpuZ80::EncodeLD_A_n(uint8_t value) const {
  return {opcodes::LD_A_M, value};  // ❌ Compiler error: LD_A_M undefined
  // Should be: opcodes::LD_A_n
  // Typo caught immediately!
}
```

---

### Example: Documentation Value

**6809 Opcode Structure (Not Documented in Code):**

```
Current code (no context):
  return {0x10, 0x16, hi, lo};  // ❓ Why two bytes before operand?

With opcode header:
  return {opcodes::prefix::PAGE2, opcodes::page2::LBRA, hi, lo};
  // ✅ Clear: This is a Page 2 instruction (prefix $10)
  // ✅ LBRA = Long Branch Always (16-bit relative)
```

---

## Refactoring Impact Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│  PHASE 1: CREATE OPCODE HEADERS                                  │
│  Effort: 10-14 hours                                             │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. Create opcodes_z80.h        (~4 hours)                       │
│     • Document ~150 opcodes                                      │
│     • Organize by category                                       │
│     • Document prefix bytes (CB, DD, ED, FD)                     │
│     • Add comments with instruction descriptions                 │
│                                                                  │
│  2. Create opcodes_6809.h       (~6 hours)                       │
│     • Document ~100 page 1 opcodes                               │
│     • Document page 2 opcodes (prefix $10)                       │
│     • Document page 3 opcodes (prefix $11)                       │
│     • Document indexed post-byte encoding                        │
│     • More complex than Z80!                                     │
│                                                                  │
│  3. Add test coverage           (~2 hours)                       │
│     • Verify opcodes against CPU manuals                         │
│     • Spot-check critical instructions                           │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────────┐
│  PHASE 2: REFACTOR IMPLEMENTATIONS                               │
│  Effort: 6-8 hours                                               │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. Refactor cpu_z80.cpp        (~3 hours)                       │
│     • Add #include "xasm++/cpu/opcodes_z80.h"                   │
│     • Replace ~100 magic numbers                                 │
│     • Test all Z80 instructions                                  │
│                                                                  │
│  2. Refactor cpu_6809.cpp       (~3 hours)                       │
│     • Add #include "xasm++/cpu/opcodes_6809.h"                  │
│     • Replace ~100 magic numbers                                 │
│     • Test all 6809 instructions                                 │
│                                                                  │
│  3. Update test files           (~2 hours)                       │
│     • Use named constants in test expectations                   │
│     • Add comments referencing opcode headers                    │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────────┐
│  RESULT: CONSISTENT PATTERN ACROSS ALL CPUs                      │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ✅ No magic numbers in any CPU implementation                   │
│  ✅ Uniform pattern across 6502, Z80, 6809                       │
│  ✅ Self-documenting code                                        │
│  ✅ Easy code reviews                                            │
│  ✅ Instruction sets documented in code                          │
│  ✅ Better maintainability                                       │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Comparison: Before and After

### BEFORE (Inconsistent)

| CPU Plugin | Pattern | Magic Numbers | Maintainability |
|------------|---------|---------------|-----------------|
| 6502       | Opcode header | 0 ✅ | Excellent ✅ |
| Z80        | Magic numbers | ~100 ❌ | Poor ❌ |
| 6809       | Magic numbers | ~100 ❌ | Poor ❌ |

**Problem:** Mixed patterns, hard to maintain Z80/6809

---

### AFTER (Consistent)

| CPU Plugin | Pattern | Magic Numbers | Maintainability |
|------------|---------|---------------|-----------------|
| 6502       | Opcode header | 0 ✅ | Excellent ✅ |
| Z80        | Opcode header | 0 ✅ | Excellent ✅ |
| 6809       | Opcode header | 0 ✅ | Excellent ✅ |

**Result:** Uniform pattern, easy to maintain all CPUs

---

## Conclusion

The **opcode definition inconsistency** is the most significant architectural issue in the xasm++ codebase. While the 6502 family demonstrates excellent practices with its opcode header pattern, the Z80 and 6809 plugins diverge from this pattern, introducing over 200 magic numbers that harm code quality.

**Recommendation:** Prioritize creating `opcodes_z80.h` and `opcodes_6809.h` as the highest-priority architectural improvement.

**Impact:**
- ✅ Eliminates 200+ magic numbers
- ✅ Achieves pattern consistency across all CPUs
- ✅ Improves maintainability significantly
- ✅ Reduces code review burden
- ✅ Makes codebase more welcoming to contributors

**Effort:** 16-22 hours total (reasonable for the benefit gained)

---

**Diagram Created:** 2026-02-09
**Purpose:** Visualize pattern inconsistency in CPU plugin opcode definitions
**Next Step:** Implement opcode headers for Z80 and 6809
