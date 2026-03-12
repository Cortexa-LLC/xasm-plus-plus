# Implementation Plan

**Role:** Engineer
**Workflow:** Bugfix (.ai-pack/workflows/bugfix.md)

## Approach

Create platform-specific environment variable helpers using preprocessor conditionals.

## Steps

### 1. Create Cross-Platform Helper Functions

Add helper functions at the top of each affected test file (or in a shared test utility header if time permits):

```cpp
#ifdef _WIN32
#include <stdlib.h>
inline int setenv_portable(const char* name, const char* value, int) {
    return _putenv_s(name, value);
}
inline int unsetenv_portable(const char* name) {
    return _putenv_s(name, "");
}
#else
inline int setenv_portable(const char* name, const char* value, int overwrite) {
    return setenv(name, value, overwrite);
}
inline int unsetenv_portable(const char* name) {
    return unsetenv(name);
}
#endif
```

### 2. Update test_error_formatter.cpp

- Add helper functions before first test
- Replace `setenv()` call at line 107 with `setenv_portable()`
- Replace `unsetenv()` call at line 122 with `unsetenv_portable()`

### 3. Update test_error_reporting.cpp

- Add helper functions before first test
- Replace `setenv()` call at line 383 with `setenv_portable()`
- Replace `unsetenv()` call at line 399 with `unsetenv_portable()`

### 4. Build and Test

```bash
# Clean rebuild
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

### 5. Commit and Push

```bash
git add tests/unit/test_error_formatter.cpp tests/unit/test_error_reporting.cpp
git commit -m "fix: add Windows portability for environment variable functions

Replace POSIX-only setenv/unsetenv with cross-platform wrappers.
- Windows: Use _putenv_s
- POSIX (Linux/macOS): Use setenv/unsetenv

Fixes Windows CI build failures.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
git push
```

## Verification

After push, verify CI status:
```bash
gh run watch
```

Expected: 7/8 jobs passing (Windows Debug + Release now pass, coverage issue remains)

## Risks

**Low Risk:** Simple, localized changes. Standard cross-platform pattern.

## Execution Time

~15 minutes:
- Implementation: 5 min
- Build/test: 5 min
- Commit/push: 2 min
- CI verification: 3 min
