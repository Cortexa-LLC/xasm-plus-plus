# Task Contract: Fix Final 3 CI Issues

**Beads Task:** xasm++-qf3l
**Priority:** P1 (CRITICAL - Windows still broken)
**Created:** 2026-02-15

## CRITICAL NOTE

⚠️ **Previous attempt added `#include <algorithm>` to WRONG file!**
- Added to: test_z80_forward_refs.cpp (WRONG)
- Should be: **tests/unit/test_z80_listing_directives.cpp** (CORRECT)

## Requirements

Fix 3 remaining CI issues causing 50% failure rate (4/8 jobs failing).

## Critical Issues

### Issue 1: Include Added to WRONG File (CRITICAL)
- **Error Location:** `tests/unit/test_z80_listing_directives.cpp:225`
- **Error:** `'count' is not a member of 'std'`
- **Code:** `size_t newline_count = std::count(between.begin(), between.end(), '\n');`
- **Wrong Fix Applied:** Added include to test_z80_forward_refs.cpp
- **Correct Fix:** Add `#include <algorithm>` to **tests/unit/test_z80_listing_directives.cpp**

### Issue 2: File Missed in Formatting
- **File:** `tests/unit/test_cpu_error_utils.cpp`
- **Problem:** 8 format violations in EXPECT_THROW macros
- **Lines:** 16, 17, 50, 51, 150, 151, 187, 188
- **Fix:** `clang-format-14 -i tests/unit/test_cpu_error_utils.cpp`

### Issue 3: Coverage Generation Failure
- **Error:** `mismatched end line for test function at test_z80_forward_refs.cpp:24`
- **Cause:** Line ranges changed after formatting
- **Fix:** Clean rebuild with coverage enabled

## Current CI Status
- ❌ Windows Debug: FAILED (missing algorithm)
- ❌ Windows Release: FAILED (missing algorithm)
- ❌ Format Check: FAILED (8 violations)
- ❌ Code Coverage: FAILED (generation error)
- ✅ Ubuntu Debug: PASSED
- ✅ Ubuntu Release: PASSED
- ✅ macOS Debug: PASSED
- ✅ macOS Release: PASSED

## Acceptance Criteria

- [ ] `#include <algorithm>` added to tests/unit/test_z80_listing_directives.cpp
- [ ] tests/unit/test_cpu_error_utils.cpp formatted (no violations)
- [ ] Coverage generation fixed
- [ ] Local build passes (0 warnings)
- [ ] All local tests pass (1649/1649)
- [ ] Changes committed and pushed
- [ ] CI verified: 8/8 jobs passing (100%)

## Stakeholders

- Owner: Bryan Woodruff
- Priority: P1 (BLOCKING - Windows broken)
