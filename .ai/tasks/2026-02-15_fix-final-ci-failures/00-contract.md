# Task Contract: Fix Final CI Failures

**Beads Task:** xasm++-wqzb
**Priority:** P1 (CRITICAL - 50% CI failure rate)
**Created:** 2026-02-15

## Requirements

Fix 3 critical CI issues causing 50% job failure rate (4/8 jobs failing).

## Critical Issues

### Issue 1: Format Check - 300+ Violations (CRITICAL)
- **Problem:** 300+ format violations remain across 20+ files
- **Previous Fix:** Only 6 files were formatted (incomplete)
- **Impact:** Format check job fails
- **Fix:** Format **ALL** C++ files in src/, include/, and tests/
- **Command:**
  ```bash
  find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format-14 -i
  ```

### Issue 2: Windows Build Broken (CRITICAL)
- **Error:** `'remove_if' is not a member of 'std'`
- **File:** `src/syntax/merlin_directive_handlers.cpp:245`
- **Cause:** Missing `#include <algorithm>` header
- **Impact:** ALL Windows builds fail (Debug + Release)
- **Fix:** Add `#include <algorithm>` at top of file

### Issue 3: Code Coverage Test Failure
- **Status:** Tests failed at "Generate coverage report" step
- **Action:** Investigate after fixing Issues 1 and 2
- **May resolve:** After other fixes are applied

## Current CI Status
- ❌ Windows Debug: FAILED
- ❌ Windows Release: FAILED
- ❌ Format Check: FAILED
- ❌ Code Coverage: FAILED
- ✅ Ubuntu Debug: PASSED
- ✅ Ubuntu Release: PASSED
- ✅ macOS Debug: PASSED
- ✅ macOS Release: PASSED

## Acceptance Criteria

- [ ] ALL C++ files formatted (no violations)
- [ ] #include <algorithm> added to merlin_directive_handlers.cpp
- [ ] Windows builds pass
- [ ] Format check passes
- [ ] Coverage test investigated and resolved
- [ ] Local build passes (0 warnings)
- [ ] All local tests pass (1649/1649)
- [ ] Changes committed and pushed
- [ ] CI verified: 8/8 jobs passing (100%)

## Stakeholders

- Owner: Bryan Woodruff
- Priority: P1 (BLOCKING all platforms)
