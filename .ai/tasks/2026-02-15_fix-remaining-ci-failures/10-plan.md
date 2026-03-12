# Implementation Plan: Fix Remaining CI Failures

**Workflow:** Bugfix (Critical)
**Approach:** Fix GMock link + apply format

## Steps

### 1. Fix GMock Linking Issue

**File:** `tests/unit/CMakeLists.txt`

Find the `test_parser_error_utils` target:
```cmake
add_executable(test_parser_error_utils
    test_parser_error_utils.cpp
)

target_link_libraries(test_parser_error_utils
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)
```

Add `GTest::gmock` to the link libraries:
```cmake
target_link_libraries(test_parser_error_utils
    xasm_syntax
    xasm_core
    gtest_main
    gtest
    GTest::gmock  # ADD THIS LINE
)
```

### 2. Apply Format to All 6 Files

```bash
clang-format-14 -i \
  src/common/expression_parser.cpp \
  src/syntax/scmasm_directive_handlers.cpp \
  src/syntax/merlin_syntax.cpp \
  src/syntax/edtasm_syntax.cpp \
  src/syntax/scmasm_syntax.cpp \
  src/syntax/merlin_directive_handlers.cpp
```

### 3. Verify Format Fixed

```bash
find src include tests -name '*.cpp' -o -name '*.h' | \
  xargs clang-format-14 --dry-run --Werror
```

Expected: No output (all files formatted)

### 4. Verify Build Passes

```bash
cmake --build build --clean-first
```

Expected: Build succeeds with zero warnings

### 5. Verify Tests Pass

```bash
ctest --test-dir build --output-on-failure
```

Expected: 100% tests passed, 0 tests failed out of 1649

### 6. Commit and Push

```bash
git add tests/unit/CMakeLists.txt \
  src/common/expression_parser.cpp \
  src/syntax/scmasm_directive_handlers.cpp \
  src/syntax/merlin_syntax.cpp \
  src/syntax/edtasm_syntax.cpp \
  src/syntax/scmasm_syntax.cpp \
  src/syntax/merlin_directive_handlers.cpp

git commit -m "fix: add GMock link and apply clang-format to 6 files

- Add GTest::gmock to test_parser_error_utils target
- Fix 29+ format violations across 6 files:
  - merlin_directive_handlers.cpp (10+ violations)
  - merlin_syntax.cpp (9 violations)
  - edtasm_syntax.cpp (4 violations)
  - scmasm_directive_handlers.cpp (3 violations)
  - expression_parser.cpp (2 violations)
  - scmasm_syntax.cpp (1 violation)

All builds now pass on all platforms.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

git push
```

### 7. Verify CI Passes

Wait for CI to complete and verify all checks green.

## Critical Requirements

- **MUST** verify locally before push
- **MUST** ensure all tests pass
- **MUST** ensure no format violations remain
- **MUST** verify build succeeds with zero warnings

## Expected Outcome

All CI checks passing (green) on all platforms.
