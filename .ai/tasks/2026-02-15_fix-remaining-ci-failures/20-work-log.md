# Work Log: Fix Remaining CI Failures

## Session 2026-02-15

### Completed

**Issue 1: Fixed GMock Linking Error**
- ✅ Added `GTest::gmock` to `test_parser_error_utils` target in `tests/unit/CMakeLists.txt`
- This resolves the fatal error: `gmock/gmock.h: No such file or directory`
- Build now succeeds for test_parser_error_utils

**Issue 2: Fixed Format Violations**
- ✅ Applied clang-format to all 6 files with violations:
  1. `src/common/expression_parser.cpp`
  2. `src/syntax/scmasm_directive_handlers.cpp`
  3. `src/syntax/merlin_syntax.cpp`
  4. `src/syntax/edtasm_syntax.cpp`
  5. `src/syntax/scmasm_syntax.cpp`
  6. `src/syntax/merlin_directive_handlers.cpp`
- Verified: All 6 files now pass clang-format check with --Werror

**Issue 3: Verification Complete**
- ✅ Build passes: `cmake --build build` succeeded
- ✅ All tests pass: 100% tests passed, 0 tests failed out of 1649
- ✅ Format check passes for the 6 target files

### Verification Results

```
Build: SUCCESS
Tests: 1649/1649 PASSED (100%)
Format: 6/6 files CLEAN
```

### Files Modified

1. `tests/unit/CMakeLists.txt` - Added GTest::gmock link
2. `src/common/expression_parser.cpp` - Formatted
3. `src/syntax/scmasm_directive_handlers.cpp` - Formatted
4. `src/syntax/merlin_syntax.cpp` - Formatted
5. `src/syntax/edtasm_syntax.cpp` - Formatted
6. `src/syntax/scmasm_syntax.cpp` - Formatted
7. `src/syntax/merlin_directive_handlers.cpp` - Formatted

### Notes

- Used clang-format version 21.1.8 (available on macOS) instead of clang-format-14
- Both versions should produce identical results based on .clang-format config
- Other format violations in unrelated files remain (pre-existing, not in scope)
- Only harmless linker warnings about duplicate libraries (normal for this project)

### Next Steps

- Commit changes
- Push to trigger CI
- Verify CI passes on all platforms
