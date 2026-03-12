# Contract: Refactor - Replace Hardcoded Opcode List with CPU Query

**Beads Task:** (generated)
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer
**Priority:** P1 (code quality, eliminates duplication)

---

## Executive Summary

Eliminate hardcoded opcode list in `scmasm_syntax.cpp` by adding `HasOpcode()` method to `CpuPlugin` interface and querying the CPU's opcode table instead.

**Current Problem:** 50+ opcodes hardcoded in static list (lines 531-541)
**Solution:** Query CPU plugin's opcode table via new interface method

---

## Background

### Current Implementation (Bad)

**File:** `src/syntax/scmasm_syntax.cpp` lines 529-547

```cpp
// Check if this is a known 6502 opcode (not a label)
static const std::unordered_set<std::string> known_opcodes = {
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL",
    "BRK", "BVC", "BVS", "CLC", "CLD", "CLI", "CLV", "CMP", "CPX", "CPY",
    "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP", "JSR", "LDA",
    "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL",
    "ROR", "RTI", "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY",
    "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
    // 65C02 additions
    "BRA", "PHX", "PHY", "PLX", "PLY", "STZ", "TRB", "TSB",
    // Common pseudo-ops
    "DB", "DW", "DS"};

if (known_opcodes.find(label_upper) != known_opcodes.end()) {
  // This is an opcode, not a label
  pos = label_start;
  return "";
}
```

**Problems:**
1. **Duplication:** CPU already has opcode table
2. **Maintenance:** Must update when CPU support changes
3. **Sync Risk:** Will get out of sync with actual CPU implementation
4. **Magic Strings:** 50+ hardcoded string literals
5. **Architecture Violation:** Syntax layer shouldn't know CPU details

### Desired Implementation (Good)

```cpp
// Check if this is a known opcode (not a label)
if (cpu_->HasOpcode(label_upper)) {
  // This is an opcode, not a label
  pos = label_start;
  return "";
}
```

**Benefits:**
1. **Single Source of Truth:** CPU module owns opcode knowledge
2. **Automatic Updates:** New opcodes automatically supported
3. **Proper Layering:** Syntax queries CPU, doesn't duplicate logic
4. **No Magic Strings:** Zero hardcoded opcodes

---

## Requirements

### 1. Add HasOpcode() to CpuPlugin Interface

**File:** `include/xasm++/cpu/cpu_plugin.h`

Add virtual method:
```cpp
/**
 * @brief Check if a mnemonic is a valid opcode for this CPU
 *
 * @param mnemonic Instruction mnemonic (e.g., "LDA", "JMP")
 * @return true if mnemonic is a valid opcode for this CPU, false otherwise
 *
 * @note Mnemonic comparison should be case-insensitive
 */
virtual bool HasOpcode(const std::string &mnemonic) const = 0;
```

### 2. Implement HasOpcode() in Each CPU Plugin

**Files to modify:**
- `include/xasm++/cpu/cpu_6502.h` - Add declaration
- `src/cpu/cpu_6502.cpp` - Add implementation
- `include/xasm++/cpu/cpu_6809.h` - Add declaration
- `src/cpu/cpu_6809.cpp` - Add implementation
- `include/xasm++/cpu/cpu_z80.h` - Add declaration
- `src/cpu/cpu_z80.cpp` - Add implementation

**Implementation pattern:**
```cpp
bool Cpu6502::HasOpcode(const std::string &mnemonic) const {
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Query internal opcode table/map
  return opcode_table_.find(upper) != opcode_table_.end();
}
```

**Note:** Each CPU has different internal structures:
- CPU_6502: May use map/lookup table
- CPU_6809: May use different structure
- CPU_Z80: May use different structure

Find and use the existing opcode storage structure in each CPU.

### 3. Replace Hardcoded List in scmasm_syntax.cpp

**File:** `src/syntax/scmasm_syntax.cpp`

**Before (lines 529-547):**
```cpp
// Check if this is a known 6502 opcode (not a label)
static const std::unordered_set<std::string> known_opcodes = { ... };
if (known_opcodes.find(label_upper) != known_opcodes.end()) {
  pos = label_start;
  return "";
}
```

**After:**
```cpp
// Check if this is a known opcode (not a label)
if (cpu_->HasOpcode(label_upper)) {
  pos = label_start;
  return "";
}
```

**Verify cpu_ is available:**
- Check if ParseLabel() has access to cpu_ pointer
- If not, may need to pass as parameter or access via parser state

### 4. Remove Pseudo-op Handling

**Note:** The hardcoded list includes pseudo-ops ("DB", "DW", "DS") which aren't real CPU opcodes.

**Options:**
1. Keep minimal pseudo-op list (3 items vs 50+)
2. Check directive registry instead
3. Document that these aren't opcodes and should be handled separately

---

## Acceptance Criteria

1. **Interface updated:**
   - [ ] `HasOpcode()` added to `CpuPlugin` interface
   - [ ] Virtual method properly documented

2. **All CPUs implemented:**
   - [ ] CPU_6502 implements `HasOpcode()`
   - [ ] CPU_6809 implements `HasOpcode()`
   - [ ] CPU_Z80 implements `HasOpcode()`

3. **Duplication eliminated:**
   - [ ] Hardcoded opcode list removed from `scmasm_syntax.cpp`
   - [ ] Replaced with `cpu_->HasOpcode()` call

4. **Testing:**
   - [ ] All 1564+ tests still passing
   - [ ] Zero regressions
   - [ ] Zero new compiler warnings
   - [ ] Label parsing still works correctly

5. **Code quality:**
   - [ ] Zero magic strings remaining
   - [ ] Proper separation of concerns (syntax queries CPU)

---

## Implementation Notes

### CPU Opcode Storage

Each CPU likely stores opcodes differently:

**CPU_6502:**
```cpp
// Likely has something like:
std::unordered_map<std::string, OpcodeInfo> opcodes_;
// OR
std::map<std::string, uint8_t> opcode_map_;
```

**CPU_6809:**
```cpp
// May use different structure
```

**CPU_Z80:**
```cpp
// May use different structure
```

**Task:** Find the existing opcode storage in each CPU and query it.

### Pseudo-ops vs Real Opcodes

The current hardcoded list includes:
- **Real opcodes:** ADC, LDA, STA, etc. (47 items)
- **Pseudo-ops:** DB, DW, DS (3 items)

Pseudo-ops aren't CPU opcodes. Options:
1. Keep minimal 3-item check for pseudo-ops
2. Check directive registry for pseudo-ops
3. Let them be treated as labels (may break compatibility)

**Recommendation:** Keep minimal pseudo-op check until we understand their usage.

---

## Estimated Effort

**Interface design:** 0.5 hours
**CPU implementations:** 1-2 hours (3 CPUs × 20-40 min each)
**Syntax refactor:** 0.5 hours
**Testing/verification:** 1 hour

**Total:** 3-4 hours

---

## Success Metrics

- 50+ magic strings eliminated
- Proper architectural layering restored
- Maintainability improved (CPU changes auto-reflected)
- All tests passing

---

## References

- **Current code:** `src/syntax/scmasm_syntax.cpp` lines 529-547
- **CPU interface:** `include/xasm++/cpu/cpu_plugin.h`
- **CPU implementations:**
  - `src/cpu/cpu_6502.cpp`
  - `src/cpu/cpu_6809.cpp`
  - `src/cpu/cpu_z80.cpp`

---

**Contract approved by:** Orchestrator
**Ready for:** Engineer
