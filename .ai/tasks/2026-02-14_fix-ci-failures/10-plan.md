# Implementation Plan: Fix CI Failures

**Workflow:** Bugfix
**Approach:** Apply format and fix type casting

## Steps

### 1. Fix Format Violations
```bash
clang-format-14 -i tests/unit/test_radix_parsing.cpp
```

### 2. Fix Integer Overflow (test_cpu_z80.cpp)

**Line 426:**
```cpp
// Before:
auto bytes = cpu.EncodeJR_C_e(0xFC); // Negative displacement

// After:
auto bytes = cpu.EncodeJR_C_e(static_cast<int8_t>(0xFC)); // -4
```

**Line 1022:**
```cpp
// Before:
auto bytes = cpu.EncodeJR_e(0xFF); // -1 in two's complement

// After:
auto bytes = cpu.EncodeJR_e(static_cast<int8_t>(0xFF)); // -1
```

**Line 1038:**
```cpp
// Before:
auto bytes = cpu.EncodeJR_e(0x80); // -128 in two's complement

// After:
auto bytes = cpu.EncodeJR_e(static_cast<int8_t>(0x80)); // -128
```

### 3. Verify Locally
```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

### 4. Commit and Push
```bash
git add tests/unit/test_radix_parsing.cpp tests/unit/test_cpu_z80.cpp
git commit -m "fix: apply clang-format and fix integer overflow in Z80 tests

- Format test_radix_parsing.cpp with clang-format-14
- Fix integer overflow warnings in test_cpu_z80.cpp (lines 426, 1022, 1038)
- Use explicit static_cast<int8_t> for hex literals

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
git push
```

### 5. Verify CI
```bash
gh run watch
```

## Expected Outcome

All CI checks passing (green).
