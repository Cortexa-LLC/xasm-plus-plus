# Branch Relaxation Implementation Quick Guide

**For:** xasm++ developers
**Objective:** Implement automatic long branch handling (Merlin compatibility)
**Blocks:** 14 PoP source files

---

## The Problem in 30 Seconds

6502 branches have 8-bit signed offsets (-128 to +127 bytes). PoP has branches spanning 132 to 3017 bytes. Current xasm++ errors on these.

---

## The Solution in 30 Seconds

**Automatic Branch Relaxation** - When branch is out of range, automatically emit:

```assembly
; Source:
BEQ FARAWAY

; Emit:
BNE *+5        ; Complementary branch (opcode XOR 0x20)
JMP FARAWAY    ; 3-byte absolute jump
```

**Size:** 5 bytes (2 for B!cc, 3 for JMP)

---

## Implementation Pseudocode

### 1. Opcode Complement
```cpp
uint8_t complement_branch(uint8_t opcode) {
    return opcode ^ 0x20;  // XOR with 0x20
}

// BEQ (0xF0) → BNE (0xD0)
// BCC (0x90) → BCS (0xB0)
// etc.
```

### 2. Detection
```cpp
bool needs_relaxation(uint16_t current, uint16_t target) {
    int16_t offset = target - (current + 2);
    return (offset < -128 || offset > 127);
}
```

### 3. Size Calculation
```cpp
size_t get_instruction_size(Instruction* inst, uint16_t addr) {
    if (inst->is_branch()) {
        return needs_relaxation(addr, inst->target) ? 5 : 2;
    }
    // ... other instructions
}
```

### 4. Code Emission
```cpp
void emit_branch(uint8_t opcode, uint16_t target, uint16_t current) {
    int16_t offset = target - (current + 2);
    
    if (offset >= -128 && offset <= 127) {
        // Normal branch
        emit_byte(opcode);
        emit_byte(static_cast<int8_t>(offset));
    } else {
        // Relaxed branch
        emit_byte(complement_branch(opcode));
        emit_byte(3);        // Skip JMP
        emit_byte(0x4C);     // JMP opcode
        emit_word(target);   // Target address (little-endian)
    }
}
```

---

## Branch Opcodes

| Instruction | Opcode | Complement | Opcode |
|-------------|--------|------------|--------|
| BPL | 0x10 | BMI | 0x30 |
| BMI | 0x30 | BPL | 0x10 |
| BVC | 0x50 | BVS | 0x70 |
| BVS | 0x70 | BVC | 0x50 |
| BCC | 0x90 | BCS | 0xB0 |
| BCS | 0xB0 | BCC | 0x90 |
| BNE | 0xD0 | BEQ | 0xF0 |
| BEQ | 0xF0 | BNE | 0xD0 |

---

## Testing Requirements

### Edge Cases
```cpp
TEST(BranchRelaxation, EdgeCases) {
    EXPECT_NO_RELAX(127);   // +127 bytes - fits
    EXPECT_RELAX(128);      // +128 bytes - too far
    EXPECT_NO_RELAX(-128);  // -128 bytes - fits
    EXPECT_RELAX(-129);     // -129 bytes - too far
}
```

### All Branch Types
```cpp
TEST(BranchRelaxation, AllBranches) {
    test_branch(0x10);  // BPL
    test_branch(0x30);  // BMI
    test_branch(0x50);  // BVC
    test_branch(0x70);  // BVS
    test_branch(0x90);  // BCC
    test_branch(0xB0);  // BCS
    test_branch(0xD0);  // BNE
    test_branch(0xF0);  // BEQ
}
```

### Real-World
```cpp
TEST(BranchRelaxation, PoPFiles) {
    // These 14 files should now assemble:
    test_assemble("AUTO.S");
    test_assemble("COLL.S");
    // ... all 14 files
}
```

---

## Integration Points

**File:** `src/cpus/cpu_6502.cpp` (or equivalent)

**Functions:**
1. `is_branch_instruction()` - Check if opcode is a branch
2. `calculate_instruction_size()` - Return 2 or 5 bytes
3. `encode_instruction()` - Emit normal or relaxed branch

**Two-pass assembly:**
- Pass 1: Calculate sizes (may need iteration)
- Pass 2: Emit code

---

## Example Output

**Input:**
```assembly
       ORG $1000
START: LDA #$00
       BEQ TARGET
       ; ... 200 bytes ...
TARGET: RTS
```

**Output bytes:**
```
$1000: A9 00           ; LDA #$00
$1002: D0 03           ; BNE *+5 (complement of BEQ)
$1004: 4C D0 10        ; JMP $10D0 (TARGET)
...
$10D0: 60              ; RTS
```

---

## Performance

**Size:** +3 bytes per relaxed branch (5 vs 2)
**Speed:** +2 cycles when condition true (5 vs 3)
**Impact:** Negligible (~0.08% size, 2 microseconds)

---

## Reference Implementation

**vasm-ext:** `cpus/6502/cpu.c`
- Lines 560-575: Detection
- Line 707: Complement (XOR 0x20)
- Lines 1008-1013: Emission

---

## Success Criteria

```
✓ All 8 branch types handled
✓ Edge cases tested (±127, ±128, ±129)
✓ 14 PoP files assemble successfully
✓ No regressions in working files
✓ Code coverage ≥ 90%
```

---

## Questions?

See full documentation: `docs/investigations/merlin-branch-relaxation.md`

---

**Quick Start:** Copy pseudocode, add to 6502 plugin, test, done!
