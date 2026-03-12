# Task Contract: Magic String Elimination - Merlin + 6809

**Task ID:** 2026-02-11_magic-string-cleanup
**Beads Task:** xasm++-gtds
**Created:** 2026-02-11
**Requestor:** User (architectural standards)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Eliminate remaining magic strings in Merlin directives and 6809 CPU to complete magic string/number elimination mandate.

### Background

**User Mandate:** "as we go through the god-class cleanup, ensure we're also addressing the magic number/magic string problem"

**Progress So Far:**
- ✅ Core assembler: Clean
- ✅ CPU 6502: Clean (BLT/MVN/MVP constants added)
- ❌ Merlin directives: 13 magic strings remaining
- ❌ CPU 6809: Multiple magic mnemonic strings

### Current State

**File 1: `src/directives/merlin_directives.cpp`**
```cpp
// WRONG - Magic strings
registry.Register("HEX", ...);
registry.Register("ASC", ...);
registry.Register("DCI", ...);
// ... 13 total
```

**File 2: `src/cpu/m6809/cpu_6809.cpp`**
```cpp
// WRONG - Magic strings
if (mnemonic == "LDA") ...
if (mnemonic == "LDB") ...
if (mnemonic == "LDD") ...
// ... many more
```

### Desired State

**File 1: Use directive constants**
```cpp
// CORRECT
registry.Register(merlin::directives::HEX, ...);
registry.Register(merlin::directives::ASC, ...);
```

**File 2: Use mnemonic constants**
```cpp
// CORRECT
if (mnemonic == M6809Mnemonics::LDA) ...
if (mnemonic == M6809Mnemonics::LDB) ...
```

---

## Success Criteria

```
✓ Zero magic strings in merlin_directives.cpp registry calls
✓ Zero magic mnemonic strings in cpu_6809.cpp
✓ Constants defined in appropriate headers
✓ Build succeeds with 0 warnings
✓ All tests passing (no regressions)
```

---

## Implementation Strategy

### Part 1: Merlin Directives (1 hour)

**Step 1:** Verify `directive_constants.h` has Merlin constants
- Check if DIR_HEX, DIR_ASC, etc. exist
- Add missing constants if needed

**Step 2:** Update `merlin_directives.cpp`
- Replace 13 magic string Register() calls
- Use directive constants

**Step 3:** Test
```bash
cmake --build build
ctest --test-dir build -R MerlinSyntaxTest
```

### Part 2: 6809 Mnemonics (1-2 hours)

**Step 1:** Create `include/xasm++/cpu/mnemonics_6809.h`
```cpp
namespace M6809Mnemonics {
    constexpr const char* LDA = "LDA";
    constexpr const char* LDB = "LDB";
    constexpr const char* LDD = "LDD";
    // ... all 6809 mnemonics
}
```

**Step 2:** Update `cpu_6809.cpp`
- Include mnemonics_6809.h
- Replace all magic mnemonic strings
- Use M6809Mnemonics:: constants

**Step 3:** Test
```bash
cmake --build build
ctest --test-dir build -R Cpu6809Test
```

---

## Files Affected

```
include/xasm++/directives/directive_constants.h (verify/extend)
include/xasm++/cpu/mnemonics_6809.h (create)
src/directives/merlin_directives.cpp (13 changes)
src/cpu/m6809/cpu_6809.cpp (~20-30 changes)
```

**Estimated:** 4-5 files, ~15K tokens (SAFE)

---

## Estimated Complexity

**Complexity:** Small-Medium

**Rationale:**
- Mechanical changes (find/replace with constants)
- Pattern established (Phase 6b)
- Low risk (no logic changes)
- Time: 2-3 hours

---

## Approvals

**Contract Approved By:**
- [X] Requestor: User (2026-02-11)
- [ ] Agent: Engineer (TBD)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11
