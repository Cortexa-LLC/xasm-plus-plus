# Implementation Plan

**Task ID:** 2026-01-31_bug-stoul-conversion
**Created:** 2026-01-31
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Investigate and fix the stoul conversion failure in expression evaluator. The bug occurs when std::stoul() is called on malformed or unresolved expression strings. Solution: add robust error handling, log problematic expressions, and ensure expressions are fully resolved before numeric conversion.

**TDD Approach:**
1. Create failing test with MASTER.S patterns (RED)
2. Add error handling and logging (GREEN)
3. Verify all 14 files assemble (VERIFY)

**Key Technical Decisions:**
- Add try-catch around stoul calls with detailed logging
- Check for empty/invalid strings before conversion
- Ensure symbol resolution completes before evaluation
- May need to defer evaluation for unresolved symbols

---

## Critical Files

### Files to Investigate
```
src/ - Locate all stoul usage (grep -rn "stoul" src/)
src/expression/ - Expression evaluation logic
src/assembler.cpp - Symbol resolution, multi-pass
src/syntax/merlin/merlin_syntax.cpp - Expression parsing
```

### Test File
```
~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/MASTER.S
  - Primary reproduction case
  - Use for verification
```

---

## Step-by-Step Plan

### Phase 1: Investigation (RED)
```
Step 1.1: Locate stoul calls
  - Action: grep -rn "stoul" src/
  - Identify all locations using std::stoul()

Step 1.2: Reproduce error
  - Action: Assemble MASTER.S with verbose logging
  - Capture exact expression causing failure

Step 1.3: Add test case
  - Action: Create failing test with problematic pattern
  - Verify test fails with stoul error
```

### Phase 2: Fix (GREEN)
```
Step 2.1: Add error handling
  - Action: Wrap stoul calls in try-catch
  - Log expression string before conversion
  - Return default value or throw descriptive error

Step 2.2: Validate input
  - Action: Check for empty/invalid strings before stoul
  - Ensure expression fully resolved

Step 2.3: Verify fix
  - Action: Assemble MASTER.S successfully
  - Test passes
```

### Phase 3: Full Verification
```
Step 3.1: Test all 14 affected files
  - Files: MASTER, HIRES, GRAFIX, CTRL, CTRLSUBS, TOPCTRL, MOVER, FRAMEADV, COLL, GAMEBG, SUBS, SOUND, MISC, UNPACK
  - All must assemble without stoul errors

Step 3.2: Run full test suite
  - Command: ctest
  - Verify: 739/739 pass (no regressions)

Step 3.3: Build POP with xasm++
  - Command: make ASM=xasm++ binaries
  - Target: 24/29 files (10 already working + 14 fixed)
```

---

## Success Metrics

```
✓ MASTER.S assembles successfully
✓ All 14 affected files assemble
✓ Full test suite passes (739/739)
✓ Error messages improved (log failing expressions)
✓ Build clean (0 warnings)
```

---

**Plan Status:** Ready for execution
**Reviewed By:** Orchestrator (2026-01-31)
**Plan Version:** 1.0
