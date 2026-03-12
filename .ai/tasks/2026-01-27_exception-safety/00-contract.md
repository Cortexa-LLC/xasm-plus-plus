# Task Contract

**Task ID:** 2026-01-27_exception-safety
**Created:** 2026-01-27
**Beads Task:** xasm++-e80
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (Code Quality Phase)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Replace catch-all exception handlers with specific exception types to preserve error context and improve debugging. This is [M5] Exception Safety from the comprehensive code quality review.

### Background and Context

The codebase uses catch-all handlers (catch(...)) that lose exception context, making debugging difficult. Exceptions should be caught by specific type, with context preserved via std::throw_with_nested or custom exception classes.

### Current State

- Catch-all handlers throughout codebase
- Lost exception context during re-throws
- Difficult to debug exceptions
- All 468 tests passing

### Desired State

- Specific exception types caught (std::invalid_argument, std::out_of_range, etc.)
- Context preserved when re-throwing
- Meaningful error messages with nested exceptions
- All 468 tests still passing
- Unit tests verify exception behavior

---

## Success Criteria

```
✓ Specific exception types caught instead of catch(...)
✓ Context preserved (std::throw_with_nested or custom exceptions)
✓ Meaningful error messages
✓ Unit tests verify exception behavior
✓ All 468 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Common Patterns to Fix

1. **Replace catch-all with specific types:**
```cpp
// BEFORE (loses context):
try {
    DoSomething();
} catch (...) {
    return error;
}

// AFTER (preserves context):
try {
    DoSomething();
} catch (const std::invalid_argument& e) {
    // Log with context
    result.errors.push_back(e.what());
} catch (const std::out_of_range& e) {
    result.errors.push_back(e.what());
}
```

2. **Preserve nested exceptions:**
```cpp
// AFTER (with nesting):
try {
    DoSomething();
} catch (const std::exception& e) {
    std::throw_with_nested(
        std::runtime_error("Higher-level context: " + std::string(e.what()))
    );
}
```

3. **Custom exception classes (optional):**
```cpp
class AssemblerException : public std::runtime_error {
public:
    AssemblerException(const std::string& msg, const SourceLocation& loc)
        : std::runtime_error(msg), location(loc) {}
    SourceLocation location;
};
```

### Scope

Find and fix catch-all handlers in:
- `src/core/assembler.cpp`
- `src/syntax/merlin/merlin_syntax.cpp`
- Any other files with exception handling

Add unit tests:
- Verify specific exceptions are thrown
- Verify exception messages are meaningful
- Verify nested exception context

### Constraints

- ZERO behavior changes for valid code paths
- Better error messages for exception paths
- ZERO test regressions (all 468 tests must pass)
- C++17 compatible

---

## Lean Flow Analysis

**Files:** 2-4 files (assembler.cpp, merlin_syntax.cpp, possibly new exception classes)
**Tokens:** ~10K tokens
**Status:** ✅ SAFE - Small batch, proceed

---

## Resources

- `src/core/assembler.cpp` - Main assembler
- `src/syntax/merlin/merlin_syntax.cpp` - Parser
- `tests/unit/` - Add exception tests
- C++ exception handling: https://en.cppreference.com/w/cpp/error/exception
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

- [ ] All catch(...) replaced with specific exception types
- [ ] Context preserved with throw_with_nested or custom exceptions
- [ ] Meaningful error messages
- [ ] Unit tests for exception behavior
- [ ] All 468 tests passing
- [ ] Zero compiler warnings
