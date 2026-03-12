# Task Contract: Fix Remaining CI Failures

**Beads Task:** xasm++-czus
**Priority:** P1 (CRITICAL - All builds failing)
**Created:** 2026-02-15

## Requirements

Fix 3 blocking CI issues preventing all builds from passing.

## Critical Issues

### Issue 1: Missing GMock Include (CRITICAL)
- **File:** `tests/unit/test_parser_error_utils.cpp:9`
- **Error:** `fatal error: gmock/gmock.h: No such file or directory`
- **Impact:** ALL builds fail (Ubuntu, macOS, Windows - Debug & Release)
- **Root Cause:** test_parser_error_utils includes gmock but doesn't link it
- **Fix:** Add `GTest::gmock` to target in `tests/unit/CMakeLists.txt`

### Issue 2: Format Violations (29+ violations across 6 files)
1. `src/syntax/merlin_directive_handlers.cpp` (10+ violations)
2. `src/syntax/merlin_syntax.cpp` (9 violations)
3. `src/syntax/edtasm_syntax.cpp` (4 violations)
4. `src/syntax/scmasm_directive_handlers.cpp` (3 violations)
5. `src/common/expression_parser.cpp` (2 violations)
6. `src/syntax/scmasm_syntax.cpp` (1 violation)

### Issue 3: Verification Required
- Must verify build passes locally before push
- Must verify all tests pass (1649/1649)
- Must verify no format violations remain

## Acceptance Criteria

- [ ] GMock linked to test_parser_error_utils target
- [ ] All 6 files formatted with clang-format-14
- [ ] Local build passes with zero warnings
- [ ] All tests pass (1649/1649)
- [ ] Format check passes locally
- [ ] Changes committed and pushed
- [ ] CI verified passing

## Stakeholders

- Owner: Bryan Woodruff
- Priority: P1 (BLOCKING all development)
