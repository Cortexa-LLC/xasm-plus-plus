# Task Contract

**Task ID:** 2026-01-31_bug-stoul-conversion
**Beads Task:** xasm++-8uc
**Created:** 2026-01-31
**Priority:** P0 (CRITICAL)
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix critical expression evaluator bug causing "Logic error: stoul: no conversion" that prevents 14 POP game modules from assembling.

### Background and Context

During POP disk integration (xasm++-0fi), discovered xasm++ fails on all complex game code with stoul conversion errors. This is the highest priority bug blocking production use.

### Current State

- ❌ 14 of 29 POP modules fail: MASTER.S, HIRES.S, GRAFIX.S, CTRL.S, CTRLSUBS.S, TOPCTRL.S, MOVER.S, FRAMEADV.S, COLL.S, GAMEBG.S, SUBS.S, SOUND.S, MISC.S, UNPACK.S
- ❌ Error occurs during expression evaluation
- ❌ std::stoul() throws when given malformed/unresolved string
- ✅ Simple data files work (10 files verified)

### Desired State

- ✅ All 14 affected files assemble successfully
- ✅ Expression evaluator handles complex patterns
- ✅ Robust error handling prevents crashes
- ✅ Clear error messages when expressions fail

---

## Success Criteria

```
✓ All 14 affected POP files assemble without stoul errors
✓ MASTER.S builds successfully (primary test case)
✓ Expression evaluator logs failing expressions before throwing
✓ All existing tests pass (no regressions)
✓ New tests cover complex expression patterns
```

---

## Acceptance Criteria

### Functional Requirements
```
□ MASTER.S assembles successfully
□ All 14 affected files build
□ Forward symbol references work
□ Complex arithmetic expressions evaluate correctly
□ High/low byte operators (>, <) work
□ Label arithmetic (label+offset) works
```

### Quality Requirements
```
□ All tests passing (739/739 minimum)
□ Build clean (0 warnings)
□ Added error logging before stoul calls
□ Test cases for forward refs, complex expressions
□ Code review approved
```

---

## Constraints and Dependencies

### Constraints
```
□ Must not break existing functionality
□ Must preserve binary compatibility for working files
□ Must handle invalid expressions gracefully
```

### Dependencies
```
□ Expression parser (src/expression/)
□ Symbol resolution (src/assembler.cpp)
□ Multi-pass assembly architecture
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Investigation needed to find exact stoul call location
- Must understand expression evaluation flow
- May involve multiple fix locations
- Requires comprehensive testing

---

## Resources and References

### Bug Report
```
.ai/tasks/2026-01-31_pop-disk-integration/XASM-BUGS-REPORT.md
  - Section: Bug #2
  - Reproduction: All 14 affected files
  - Recommended fix: Add error handling, log expressions
```

### Test Files
```
~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/MASTER.S
  - Primary test case
  - First file to fail
```

### Code Locations
```
src/ - Search for stoul usage
src/expression/ - Expression evaluation
src/assembler.cpp - Multi-pass logic
```

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
