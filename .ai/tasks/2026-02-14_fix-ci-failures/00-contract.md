# Task Contract: Fix CI Failures

**Beads Task:** xasm++-tzlg
**Priority:** P1 (BLOCKING)
**Created:** 2026-02-14

## Requirements

Fix 2 blocking CI failures identified by Tester.

## Issues

### Issue 1: Format Check Violations
- **File:** `tests/unit/test_radix_parsing.cpp`
- **Problem:** 40+ clang-format violations (lines 86-194)
- **Impact:** CI format check fails with exit code 123
- **Fix:** `clang-format-14 -i tests/unit/test_radix_parsing.cpp`

### Issue 2: Integer Overflow Build Errors
- **File:** `tests/unit/test_cpu_z80.cpp`
- **Lines:** 426, 1022, 1038
- **Problem:** Unsigned hex literals passed to signed int8_t parameters
- **Impact:** Build fails with `-Werror=overflow`
- **Fix:** Use signed literals or explicit casts

## Acceptance Criteria

- [ ] test_radix_parsing.cpp formatted (clang-format-14)
- [ ] test_cpu_z80.cpp integer overflow fixed (3 lines)
- [ ] Local build passes (no warnings)
- [ ] Local tests pass (1649/1649)
- [ ] Changes committed and pushed
- [ ] CI checks pass (verified via gh CLI)

## Stakeholders

- Owner: Bryan Woodruff
- Priority: P1 (BLOCKING release)
