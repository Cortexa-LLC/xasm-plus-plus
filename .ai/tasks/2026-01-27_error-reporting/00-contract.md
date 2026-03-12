# Task Contract: Implement Error Reporting Strategy - M8 Refactoring

**Task ID:** xasm++-b1j  
**Created:** 2026-01-27  
**Status:** In Progress

## Objective

Add comprehensive error reporting with file, line, and context information to all error messages in xasm++.

## Current Problem

- Errors throw std::runtime_error with messages like: 'Invalid hex number'
- No file name, line number, or source context
- Hard to debug assembly errors
- SourceLocation infrastructure exists but unused (atom.h:28-32)

## Requirements

1. **Parser Line Tracking**: Track current line and file during parsing
2. **Atom Location Setting**: Set source location on all created atoms
3. **Error Helper Function**: Create FormatError() helper with consistent format
4. **Update Error Sites**: Update all ~20 throw sites to include location info
5. **Testing**: Add tests verifying error messages include file:line

## Acceptance Criteria

- [ ] All error messages include filename:line information
- [ ] Error messages are actionable (user knows where to fix)
- [ ] All 478 existing tests still passing
- [ ] 10+ new tests for error message quality
- [ ] Code builds with zero warnings
- [ ] TDD followed: tests before implementation

## Files to Modify

- `include/xasm++/syntax/merlin_syntax.h` (add line tracking)
- `src/syntax/merlin/merlin_syntax.cpp` (set locations, update errors)
- `include/xasm++/atom.h` (maybe add error helper)
- `src/core/assembler.cpp` (update error messages)
- `tests/unit/` (add error message tests)

## Out of Scope

- Complete Result<T> monad implementation (future work)
- Error recovery mechanisms
- Warnings vs errors distinction
