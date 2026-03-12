# Plan: Refactor - Replace Hardcoded Opcode List with CPU Query

**Task:** (generated)
**Workflow:** Refactoring
**Role:** Engineer
**Approach:** TDD with incremental implementation

---

## Execution Strategy

**Sequential implementation:**
1. Add interface method (breaks build - expected)
2. Implement for each CPU (fixes build incrementally)
3. Replace hardcoded list (uses new interface)
4. Test and verify (ensure no regressions)

---

## Implementation Steps

### Step 1: Add HasOpcode() to CpuPlugin Interface

**File:** `include/xasm++/cpu/cpu_plugin.h`

**Location:** After `GetSupportedVariants()` method (around line 59)

```cpp
/**
 * @brief Check if a mnemonic is a valid opcode for this CPU
 *
 * Determines whether the given mnemonic represents a valid instruction
 * for this CPU architecture. Used by syntax parsers to distinguish
 * between opcodes and labels in ambiguous contexts.
 *
 * @param mnemonic Instruction mnemonic (e.g., "LDA", "JMP", "ADD")
 * @return true if mnemonic is a valid opcode for this CPU, false otherwise
 *
 * @note Comparison should be case-insensitive (accepts "lda", "LDA", "Lda")
 * @note This does NOT check addressing mode validity, only mnemonic validity
 */
virtual bool HasOpcode(const std::string &mnemonic) const = 0;
```

**Verify:** Build will fail (expected - pure virtual not implemented)

---

### Step 2: Implement HasOpcode() for CPU_6502

**Files:**
- `include/xasm++/cpu/cpu_6502.h` - Add declaration
- `src/cpu/cpu_6502.cpp` - Add implementation

**2.1: Add declaration to header**

**Location:** `include/xasm++/cpu/cpu_6502.h` (after GetSupportedVariants)

```cpp
bool HasOpcode(const std::string &mnemonic) const override;
```

**2.2: Find opcode storage structure**

Search in `src/cpu/cpu_6502.cpp` for:
- `std::map` or `std::unordered_map` with opcode strings
- Arrays or tables of opcode info
- Likely named: `opcodes_`, `opcode_map_`, `instruction_table_`, etc.

**2.3: Implement HasOpcode()**

**Location:** `src/cpu/cpu_6502.cpp` (near other public methods)

```cpp
bool Cpu6502::HasOpcode(const std::string &mnemonic) const {
  // Convert to uppercase for case-insensitive comparison
  std::string upper = mnemonic;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // Query the opcode table
  // (Use whatever structure CPU_6502 actually uses)
  return /* check if upper exists in opcode table */;
}
```

**Note:** Exact implementation depends on CPU_6502's internal structure.

**Verify:** Build should still fail (other CPUs not implemented)

---

### Step 3: Implement HasOpcode() for CPU_6809

**Files:**
- `include/xasm++/cpu/cpu_6809.h` - Add declaration
- `src/cpu/cpu_6809.cpp` - Add implementation

**Same pattern as CPU_6502:**
1. Add declaration to header
2. Find opcode storage structure
3. Implement HasOpcode() using that structure

---

### Step 4: Implement HasOpcode() for CPU_Z80

**Files:**
- `include/xasm++/cpu/cpu_z80.h` - Add declaration
- `src/cpu/cpu_z80.cpp` - Add implementation

**Same pattern as CPU_6502:**
1. Add declaration to header
2. Find opcode storage structure
3. Implement HasOpcode() using that structure

**Verify:** Build should now succeed (all pure virtuals implemented)

---

### Step 5: Test CPU Implementations

**Before refactoring scmasm_syntax, verify HasOpcode() works:**

Create simple test:
```cpp
// In test file or temporary main
Cpu6502 cpu;
assert(cpu.HasOpcode("LDA") == true);
assert(cpu.HasOpcode("lda") == true);  // Case-insensitive
assert(cpu.HasOpcode("NOTREAL") == false);
```

**Run existing tests:**
```bash
cmake --build build && cd build && ctest --output-on-failure
```

Expected: All tests still pass (no behavior changes yet)

---

### Step 6: Access CPU in scmasm_syntax ParseLabel()

**Check if cpu_ pointer available in ParseLabel():**

**File:** `src/syntax/scmasm_syntax.cpp`

Look at ParseLabel() signature and available members:
```cpp
std::string ScmasmSyntaxParser::ParseLabel(const std::string &line, size_t &pos,
                                           Section &section,
                                           ConcreteSymbolTable &symbols)
```

**Options:**
1. If `cpu_` is a member of `ScmasmSyntaxParser` - use directly
2. If not available - need to pass as parameter or get from parser state
3. Check other methods for pattern

**Search for cpu_ usage:**
```bash
grep "cpu_" src/syntax/scmasm_syntax.cpp
```

**If cpu_ not available:**
- Add to ParseLabel() parameters
- Or add as member to ScmasmSyntaxParser class
- Or access via section/symbols (if they provide access)

---

### Step 7: Replace Hardcoded List

**File:** `src/syntax/scmasm_syntax.cpp` lines 529-547

**Remove:**
```cpp
// Check if this is a known 6502 opcode (not a label)
// Common 6502/65C02 opcodes that might appear at start of line
static const std::unordered_set<std::string> known_opcodes = {
    "ADC", "AND", "ASL", // ... 50+ opcodes
};

if (known_opcodes.find(label_upper) != known_opcodes.end()) {
  // This is an opcode, not a label
  pos = label_start;
  return "";
}
```

**Replace with:**
```cpp
// Check if this is a known CPU opcode (not a label)
if (cpu_->HasOpcode(label_upper)) {
  pos = label_start;
  return "";
}

// Handle pseudo-ops that aren't CPU opcodes
static const std::unordered_set<std::string> pseudo_ops = {"DB", "DW", "DS"};
if (pseudo_ops.find(label_upper) != pseudo_ops.end()) {
  pos = label_start;
  return "";
}
```

**Rationale:** Keep minimal 3-item pseudo-op check since those aren't real CPU opcodes.

---

### Step 8: Build and Test

**Full rebuild:**
```bash
cmake --build build --clean-first
```

**Run all tests:**
```bash
cd build && ctest --output-on-failure
```

**Expected:** All 1564+ tests pass

**If tests fail:**
- Check if label parsing is too strict/loose
- Verify HasOpcode() implementations are correct
- Check if pseudo-ops need different handling

---

### Step 9: Verify No Regressions

**Specific checks:**

1. **SCMASM label parsing still works:**
   ```bash
   ctest -R SCMASM -V
   ```

2. **Other syntax parsers unaffected:**
   ```bash
   ctest -R Merlin -V
   ctest -R Simple -V
   ```

3. **Zero new warnings:**
   ```bash
   cmake --build build 2>&1 | grep -i warning
   ```

---

### Step 10: Update Documentation

**Work log:** Document implementation decisions
- Which opcode storage structure each CPU uses
- Why pseudo-ops kept separate
- Any edge cases discovered

**Code comments:** Update comment where hardcoded list was removed:
```cpp
// Check if this is a known CPU opcode (not a label)
// Delegates to CPU plugin to avoid duplicating opcode knowledge
if (cpu_->HasOpcode(label_upper)) {
```

---

## Verification Checklist

- [ ] HasOpcode() added to CpuPlugin interface
- [ ] Cpu6502::HasOpcode() implemented and tested
- [ ] Cpu6809::HasOpcode() implemented and tested
- [ ] CpuZ80::HasOpcode() implemented and tested
- [ ] Hardcoded 50+ opcode list removed from scmasm_syntax.cpp
- [ ] Replaced with cpu_->HasOpcode() call
- [ ] Pseudo-ops handled separately (DB, DW, DS)
- [ ] All 1564+ tests passing
- [ ] Zero new warnings
- [ ] Zero regressions

---

## Success Criteria

- 50+ magic strings eliminated
- Single source of truth for opcode knowledge
- Proper architectural layering restored
- All tests passing

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Engineer
**Estimated effort:** 3-4 hours
