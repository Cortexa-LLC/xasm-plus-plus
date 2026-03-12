# Task Contract: Fix stoul HEX Parsing Errors

**Task ID:** 2026-01-26_stoul-hex-fix
**Beads Task:** xasm++-w02
**Created:** 2026-01-26
**Status:** In Progress
**Priority:** P1 (High - blocks 4 files)

---

## Problem Statement

Four Prince of Persia source files fail to assemble with "Parse error: stoul: no conversion" error:
- HRTABLES.S
- SEQTABLE.S
- SOUND.S
- SPECIALK.S

This error is DIFFERENT from the comma-separated HEX issue (already fixed in task xasm++-6gk).

---

## Acceptance Criteria

✅ **MUST**:
1. Identify root cause of stoul error in 4 affected files
2. Implement fix with proper error handling
3. All existing 429 tests continue passing (zero regressions)
4. New test(s) added to prevent regression
5. Zero compiler warnings
6. TDD process followed (RED-GREEN-REFACTOR)
7. All 4 files assemble successfully

✅ **SHOULD**:
1. Error messages indicate which line/value caused the error
2. Fix handles edge cases (empty strings, odd-length hex, etc.)

❌ **MUST NOT**:
1. Break existing HEX parsing (comma-separated or concatenated)
2. Break other directives (DB, DW, etc.)
3. Introduce new warnings or errors

---

## Context

### Current State
- **Files Passing:** 20/29 (69%)
- **Blocker:** stoul errors in 4 files (14% of total)
- **Expected After Fix:** 24/29 files (83%)

### Previous Work
- **xasm++-6gk:** Fixed comma-separated HEX parsing (e.g., "hex 01,02,03")
- **Current Issue:** Different error, possibly related to:
  - Expression parsing in operands
  - Number parsing (ParseNumber function)
  - Empty/malformed strings passed to stoul
  - HEX directive edge cases (odd-length, invalid characters)

### Error Locations
stoul is called in multiple places:
- `merlin_syntax.cpp:78` - ParseNumber (hex: $FFFF)
- `merlin_syntax.cpp:83` - ParseNumber (binary: %1010)
- `merlin_syntax.cpp:87` - ParseNumber (decimal)
- `merlin_syntax.cpp:327` - HandleHex (comma-separated tokens)
- `merlin_syntax.cpp:338` - HandleHex (concatenated pairs)

---

## Success Metrics

### Quantitative
- ✅ 4 additional files assembling (HRTABLES, SEQTABLE, SOUND, SPECIALK)
- ✅ Progress: 20/29 → 24/29 (69% → 83%)
- ✅ All 429+ tests passing
- ✅ Zero compiler warnings

### Qualitative
- ✅ Root cause identified and documented
- ✅ Fix is minimal and focused
- ✅ Error messages are helpful for debugging
- ✅ Code follows existing patterns

---

## Constraints

### Technical
- Must maintain backward compatibility with existing HEX parsing
- Must not break other number parsing (DB, DW, ORG, etc.)
- Must handle all Merlin number formats ($hex, %binary, decimal)

### Process
- TDD MANDATORY (tests first, implementation second)
- All changes must pass quality gates (Tester + Reviewer)
- Git commits must show RED-GREEN-REFACTOR cycle

### Timeline
- Estimated: 30-60 minutes
- Target: Complete within current session

---

## Dependencies

### Prerequisite Work
- ✅ xasm++-6gk (HEX comma-separated parsing) - COMPLETE
- ✅ Branch relaxation integration - COMPLETE
- ✅ USR directive - COMPLETE

### Blocks
- ❌ DS forward references (3 files)
- ❌ XC/65816 mode (2 files)
- ⚠️ **This task blocks 4 files (14% of total)**

---

## References

### Code Files
- `src/syntax/merlin/merlin_syntax.cpp` - HandleHex, ParseNumber
- `tests/unit/test_merlin_syntax.cpp` - HEX directive tests

### Documentation
- `.ai/tasks/MILESTONE-20-FILES-PASSING.md` - Current progress
- `.ai/tasks/2026-01-26_pop-hex-commas/` - Related HEX fix
- `docs/investigations/pop-compatibility-analysis.md` - Original analysis

### Source Files (Test Cases)
- `/Users/bryanw/Projects/Vintage/Apple/adamgreen-pop/01 POP Source/Source/HRTABLES.S`
- `/Users/bryanw/Projects/Vintage/Apple/adamgreen-pop/01 POP Source/Source/SEQTABLE.S`
- `/Users/bryanw/Projects/Vintage/Apple/adamgreen-pop/01 POP Source/Source/SOUND.S`
- `/Users/bryanw/Projects/Vintage/Apple/adamgreen-pop/01 POP Source/Source/SPECIALK.S`

---

## Definition of Done

- [ ] Root cause identified and documented in work log
- [ ] Failing test written (RED phase - TDD)
- [ ] Test fails with "stoul: no conversion" error
- [ ] Fix implemented (GREEN phase - TDD)
- [ ] All tests pass (including new test)
- [ ] Code refactored if needed (REFACTOR phase - TDD)
- [ ] All 4 affected files assemble successfully
- [ ] Zero compiler warnings
- [ ] Zero regressions (all 429 tests pass)
- [ ] Work log updated with findings and decisions
- [ ] Beads task closed (bd close xasm++-w02)
- [ ] Ready for quality gates (Tester + Reviewer)

---

**Contract Accepted By:** Engineer Agent
**Date:** 2026-01-26
