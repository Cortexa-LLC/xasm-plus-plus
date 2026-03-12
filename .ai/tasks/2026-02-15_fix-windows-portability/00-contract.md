# Fix Windows Portability: setenv/unsetenv

**Beads Task:** xasm++-5arm
**Priority:** P1
**Type:** Bugfix

## Problem

Windows CI builds failing because `setenv()` and `unsetenv()` are POSIX-only functions not available on Windows/MinGW.

**Error:**
```
test_error_formatter.cpp:107:3: error: 'setenv' was not declared in this scope; did you mean 'getenv'?
test_error_formatter.cpp:122:3: error: 'unsetenv' was not declared in this scope; did you mean 'getenv'?
test_error_reporting.cpp:383:3: error: 'setenv' was not declared in this scope; did you mean 'getenv'?
test_error_reporting.cpp:399:3: error: 'unsetenv' was not declared in this scope; did you mean 'getenv'?
```

## Requirements

Fix Windows portability by creating cross-platform environment variable wrappers.

**Affected Files:**
- `tests/unit/test_error_formatter.cpp` (lines 107, 122)
- `tests/unit/test_error_reporting.cpp` (lines 383, 399)

## Solution

Create cross-platform wrapper functions:
- Use `_putenv_s()` for Windows
- Use `setenv()`/`unsetenv()` for POSIX (Linux/macOS)

## Acceptance Criteria

- [ ] Windows CI builds pass (both Debug and Release)
- [ ] Linux CI builds still pass
- [ ] macOS CI builds still pass
- [ ] All 1649 tests pass on all platforms
- [ ] No code quality regressions

## Scope

**IN SCOPE:**
- Fix Windows portability ONLY
- Minimal changes to affected test files

**OUT OF SCOPE:**
- Format violations (already fixed in commit e37a16f)
- Code coverage issues (separate issue)
- Any other CI failures

## Constraints

- Must maintain test behavior across all platforms
- Must not introduce new dependencies
- Must use standard C++ and platform APIs only
