# Task Contract

**Task ID:** 2026-01-27_add-input-validation
**Created:** 2026-01-27
**Beads Task:** xasm++-462
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Add input validation and bounds checking before string operations to prevent buffer overflow vulnerabilities. This is [M4] No Input Validation from the comprehensive code quality review.

### Background and Context

The codebase contains string operations without bounds checking, creating potential buffer overflow vulnerabilities. String accesses should validate bounds before indexing, and std::string_view should be used where appropriate.

### Current State

- String operations without bounds checking throughout codebase
- Potential buffer overflow vulnerabilities
- No use of std::string_view for read-only string views
- All 451 tests passing

### Desired State

- All string accesses have bounds checking
- std::string_view used where appropriate
- Unit tests for edge cases (empty strings, overflow attempts)
- All 451 tests still passing
- Static analysis (clang-tidy) passes

---

## Success Criteria

```
✓ All string accesses have bounds checking
✓ std::string_view used for read-only string parameters
✓ Unit tests for edge cases added
✓ All 451 tests passing (zero regressions)
✓ Zero new compiler warnings
✓ clang-tidy or similar static analysis passes
```

---

## Technical Specification

### Common Patterns to Fix

1. **Array indexing without bounds check:**
```cpp
// BEFORE (unsafe):
char ch = str[0];

// AFTER (safe):
char ch = str.empty() ? '\0' : str[0];
// OR
if (!str.empty()) {
    char ch = str[0];
}
```

2. **String slicing without validation:**
```cpp
// BEFORE (unsafe):
std::string prefix = str.substr(0, 2);

// AFTER (safe):
std::string prefix = (str.length() >= 2) ? str.substr(0, 2) : str;
```

3. **Use string_view for read-only:**
```cpp
// BEFORE:
void Process(const std::string& str);

// AFTER:
void Process(std::string_view str);
```

### Scope

Find and fix unsafe string operations in:
- `src/syntax/merlin/merlin_syntax.cpp` (parsing)
- `src/core/assembler.cpp`
- Any other files with string manipulation

Add unit tests for edge cases:
- Empty strings
- Single character strings
- Overflow attempts

### Constraints

- ZERO behavior changes for valid input
- Add proper error handling for invalid input
- ZERO test regressions (all 451 tests must pass)
- C++17 compatible

---

## Lean Flow Analysis

**Files:** 3-5 files (merlin_syntax.cpp, assembler.cpp, possibly new validation utils)
**Tokens:** ~12K tokens
**Status:** ⚠️ APPROACHING LIMIT - May need decomposition if >3 files heavily modified

**Decomposition Trigger:** If >50 string operations need fixing, consider splitting into:
- Subtask 4A: Merlin syntax parser validation
- Subtask 4B: Assembler validation
- Subtask 4C: Utility functions validation

---

## Resources

- `src/syntax/merlin/merlin_syntax.cpp` - Main parsing logic
- `src/core/assembler.cpp` - Assembler logic
- `tests/unit/test_merlin_syntax.cpp` - Add edge case tests
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] All string indexing operations have bounds checking
- [ ] std::string_view used where appropriate
- [ ] Edge case unit tests added (empty strings, single char, overflow)
- [ ] All 451 tests passing
- [ ] Zero compiler warnings
- [ ] Static analysis clean (if available)
- [ ] Error messages for invalid input
