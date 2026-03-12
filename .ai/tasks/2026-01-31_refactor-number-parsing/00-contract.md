# Task Contract: Consolidate Duplicate ParseHex Implementations

**Beads Task:** xasm++-vgb
**Created:** 2026-01-31
**Status:** Open
**Priority:** P3 (Low)

## Context

There are currently **3 duplicate implementations** of hex number parsing logic scattered across the codebase:

1. **`src/utils/string_utils.cpp:25`** - `ParseHex()`
   - Public utility function
   - Handles optional `$` prefix
   - Strips addressing mode suffixes (`,X`, `,Y`, `,S`)
   - Throws exceptions on error

2. **`src/syntax/simple_syntax.cpp:12`** - `static ParseHex()`
   - File-scoped static function
   - Requires `$` prefix (returns 0 if missing)
   - Throws runtime_error on error
   - Duplicate validation logic

3. **`src/core/parse_utils.cpp:12`** - `ParseHexSafe()`
   - Safe variant with error reporting
   - Returns success flag + error message (no exceptions)
   - Most comprehensive validation
   - Already in utility namespace

This duplication leads to:
- Code maintenance overhead (bugs must be fixed in 3 places)
- Inconsistent error handling (exceptions vs. error codes)
- Inconsistent behavior (prefix handling, addressing modes)
- Larger binary size

## Requirements

### R1: Consolidate into Single Implementation

Create a unified `ParseHex` utility that:
- Handles all current use cases
- Provides both exception-throwing and safe (error-code) variants
- Maintains backward compatibility with existing callers
- Supports all current features:
  - Optional or required `$` prefix
  - Addressing mode suffix stripping (`,X`, `,Y`, `,S`)
  - Comprehensive validation

### R2: Migration Strategy

Replace all 3 implementations with calls to consolidated utility:
- Migrate `string_utils.cpp` callers (5+ call sites)
- Migrate `simple_syntax.cpp` callers (3 call sites)
- Migrate `parse_utils.cpp` callers (0 call sites - it IS the utility)
- Delete duplicate implementations

### R3: Target Location

Use existing **`src/core/parse_utils.cpp`** as the consolidated location:
- Already contains `ParseHexSafe()` with comprehensive validation
- Part of `xasm` namespace (proper utility organization)
- Header already exists (`include/xasm++/parse_utils.h`)

Extend with:
- `ParseHex()` - Exception-throwing variant (for current `string_utils` callers)
- Enhanced options for prefix/suffix handling

## Acceptance Criteria

### AC1: Single Implementation
- [ ] Only ONE `ParseHex` implementation exists in codebase
- [ ] Both exception-throwing and safe variants available
- [ ] All features preserved (prefix, suffix, validation)

### AC2: All Callers Migrated
- [ ] `src/core/assembler.cpp` uses consolidated utility (5+ call sites)
- [ ] `src/syntax/simple_syntax.cpp` uses consolidated utility (3 call sites)
- [ ] Duplicate implementations deleted:
  - [ ] `string_utils.cpp::ParseHex()` removed
  - [ ] `simple_syntax.cpp::ParseHex()` removed

### AC3: Tests Pass
- [ ] All existing unit tests pass (no regressions)
- [ ] Prince of Persia integration tests pass (100% compatibility maintained)
- [ ] New unit tests for consolidated utility cover all features

### AC4: Code Quality
- [ ] No increase in binary size (should decrease)
- [ ] No performance regression
- [ ] Clear documentation of function variants and options

## Out of Scope

- Changing error messages (maintain existing messages for compatibility)
- Adding new features beyond current capabilities
- Refactoring calling code (beyond the migration itself)

## Risks

| Risk | Mitigation |
|------|------------|
| Behavioral changes break existing code | Comprehensive test coverage before migration |
| Performance regression | Inline critical paths, benchmark |
| Missed call sites | Search entire codebase for all occurrences |

## Stakeholders

- **Maintainer:** Needs single source of truth for hex parsing
- **Users:** Need consistent behavior across assembler
- **CI/CD:** Must verify no regressions

## Dependencies

None - this is a pure refactoring task.

## Success Metrics

- Lines of code reduced by ~80 lines (duplicate implementations)
- Test coverage maintained at 100% for hex parsing
- Zero regressions in test suite
