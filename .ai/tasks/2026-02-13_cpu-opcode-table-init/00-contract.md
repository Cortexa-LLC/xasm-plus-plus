# Contract: Fix missing-field-initializers in cpu_6502.cpp

**Beads Task:** xasm++-8l2w
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer
**Priority:** P1 (CI blocker)
**Workflow:** bugfix.md

---

## Executive Summary

Ubuntu/Windows CI builds failing due to missing-field-initializers errors in cpu_6502.cpp. GCC/MSVC compilers require complete struct initialization when using designated initializers, but OpcodeTable structs have partial initialization (only 3-4 fields filled, missing 15-16 fields).

**Problem:** Partial designated initializers at lines 256, 416, 429, and similar patterns
**Solution:** Add explicit std::nullopt for all 19 OpcodeTable fields

---

## Background

### Current Issue

**CI Run:** https://github.com/Cortexa-LLC/xasm-plus-plus/actions/runs/21994137589

**Error Message:**
```
/home/runner/work/xasm-plus-plus/xasm-plus-plus/src/cpu/cpu_6502.cpp:256:3:
error: missing initializer for member 'xasm::Cpu6502::OpcodeTable::immediate'
[-Werror=missing-field-initializers]
```

**Compiler Behavior:**
- ✅ **macOS (Clang):** Permissive, partial initialization allowed
- ❌ **Ubuntu (GCC):** Strict, requires all fields initialized with -Werror
- ❌ **Windows (MSVC):** Strict, requires all fields initialized with /WX

### Root Cause

**OpcodeTable struct** (19 fields):
```cpp
struct OpcodeTable {
  std::optional<uint8_t> immediate;
  std::optional<uint8_t> zero_page;
  std::optional<uint8_t> zero_page_x;
  std::optional<uint8_t> zero_page_y;
  std::optional<uint8_t> absolute;
  std::optional<uint8_t> absolute_x;
  std::optional<uint8_t> absolute_y;
  std::optional<uint8_t> indirect;
  std::optional<uint8_t> indirect_x;
  std::optional<uint8_t> indirect_y;
  std::optional<uint8_t> accumulator;
  std::optional<uint8_t> relative;
  std::optional<uint8_t> indirect_zero_page;                // 65C02+
  std::optional<uint8_t> absolute_indexed_indirect;         // 65C02+
  std::optional<uint8_t> absolute_long;                     // 65816
  std::optional<uint8_t> indirect_long;                     // 65816
  std::optional<uint8_t> indirect_long_indexed_y;           // 65816
  std::optional<uint8_t> stack_relative;                    // 65816
  std::optional<uint8_t> stack_relative_indirect_indexed_y; // 65816
};
```

**Problematic initialization** (line 252-256):
```cpp
static const OpcodeTable JMP_TABLE = {
    .absolute = Opcodes::JMP_ABS,
    .indirect = Opcodes::JMP_IND,
    .absolute_indexed_indirect = Opcodes::JMP_AIX // 65C02+
};  // Missing 16 fields!
```

**Correct initialization pattern** (lines 271-290):
```cpp
static const OpcodeTable ADC_TABLE = {
    .immediate = Opcodes::ADC_IMM,
    .zero_page = Opcodes::ADC_ZP,
    // ... all 19 fields explicitly initialized
    .stack_relative_indirect_indexed_y = std::nullopt
};
```

---

## Requirements

### Functional Requirements

**FR-001: Complete all OpcodeTable initializations**
- Find all partial OpcodeTable initializations in cpu_6502.cpp
- Add explicit std::nullopt for every missing field
- Ensure all 19 fields present in every initialization

**FR-002: Maintain existing functionality**
- No behavior changes
- All tests must still pass (1564/1564)
- Zero new warnings

### Non-Functional Requirements

**NFR-001: Cross-platform compatibility**
- Must compile on macOS, Ubuntu, Windows
- Must pass CI format check and build jobs
- Zero warnings with -Werror/WX enabled

**NFR-002: Consistency**
- Follow existing pattern (see ADC_TABLE lines 271-290)
- Maintain alphabetical field order
- Use std::nullopt for unused addressing modes

---

## Acceptance Criteria

- [ ] All OpcodeTable structs have complete 19-field initialization
- [ ] Ubuntu CI build passes (GCC)
- [ ] Windows CI build passes (MSVC)
- [ ] macOS CI build passes (Clang)
- [ ] Format Check passes
- [ ] All 1564 tests passing
- [ ] Zero new compiler warnings
- [ ] Zero behavior changes (refactoring only)

---

## Scope

**In Scope:**
- Fix partial OpcodeTable initializations in cpu_6502.cpp
- Add missing std::nullopt fields

**Out of Scope:**
- Changes to OpcodeTable struct definition
- Changes to other CPU files (cpu_6809.cpp, cpu_z80.cpp)
- Logic changes (pure refactoring)
- New features

---

## Known Locations

**Files to modify:**
- `src/cpu/cpu_6502.cpp`

**Known problematic lines:**
- Line 256: JMP_TABLE (3 fields, missing 16)
- Line 416: (check for additional partial initializations)
- Line 429: (check for additional partial initializations)
- Search for all OpcodeTable initializations

**Pattern to fix:**
```cpp
// BEFORE (partial)
static const OpcodeTable FOO_TABLE = {
    .absolute = Opcodes::FOO_ABS,
    .zero_page = Opcodes::FOO_ZP
};

// AFTER (complete)
static const OpcodeTable FOO_TABLE = {
    .immediate = std::nullopt,
    .zero_page = Opcodes::FOO_ZP,
    .zero_page_x = std::nullopt,
    .zero_page_y = std::nullopt,
    .absolute = Opcodes::FOO_ABS,
    .absolute_x = std::nullopt,
    .absolute_y = std::nullopt,
    .indirect = std::nullopt,
    .indirect_x = std::nullopt,
    .indirect_y = std::nullopt,
    .accumulator = std::nullopt,
    .relative = std::nullopt,
    .indirect_zero_page = std::nullopt,
    .absolute_indexed_indirect = std::nullopt,
    .absolute_long = std::nullopt,
    .indirect_long = std::nullopt,
    .indirect_long_indexed_y = std::nullopt,
    .stack_relative = std::nullopt,
    .stack_relative_indirect_indexed_y = std::nullopt
};
```

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Miss some partial initializations | Med | Search entire file for "OpcodeTable" pattern |
| Introduce typos in field names | Low | Use reference table (ADC_TABLE) as template |
| Break functionality | Low | All tests must pass, zero behavior changes |

---

## Success Metrics

- Ubuntu/Windows CI builds passing
- Zero missing-field-initializers errors
- 1564/1564 tests passing

---

## References

- **CI Failure:** https://github.com/Cortexa-LLC/xasm-plus-plus/actions/runs/21994137589
- **OpcodeTable definition:** `include/xasm++/cpu/cpu_6502.h` lines ~137-157
- **Reference pattern:** `src/cpu/cpu_6502.cpp` lines 271-290 (ADC_TABLE)
- **Workflow:** `.ai-pack/workflows/bugfix.md`

---

**Contract approved by:** Orchestrator
**Ready for:** Engineer
