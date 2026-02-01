# Binary Compatibility Test Framework

**Status:** ✅ Framework Complete, ⚠️ Tests Partially Blocked

This directory contains the binary compatibility test framework for xasm++. The framework validates that xasm++ produces byte-for-byte identical output to reference assemblers (vasm-ext).

---

## Architecture

### Framework Components

```
binary_compat/
├── framework/
│   └── binary_compat_test.h       # Reusable base test class
├── test_sources/
│   ├── merlin/6502/               # Merlin syntax test files
│   └── scmasm/6502/               # SCMASM syntax test files
├── golden/
│   ├── merlin/6502/               # Reference binaries from vasm
│   └── scmasm/6502/               # Reference binaries from vasm
├── test_merlin_compat.cpp         # Merlin test suite
├── test_scmasm_compat.cpp         # SCMASM test suite
├── test_current_features.cpp      # Tests for working features only
└── CMakeLists.txt                 # Build configuration
```

### Test Parameterization

Tests use GTest's parameterized testing with `BinaryCompatTestCase`:

```cpp
struct BinaryCompatTestCase {
    std::string source_file;      // Path to .asm file
    std::string syntax;            // "merlin", "scmasm", etc.
    std::string cpu;               // "6502", "65c02", "65816", etc.
    std::string vasm_binary;       // Reference assembler name
    std::string vasm_extra_flags;  // CPU-specific flags (e.g., "-m65c02")
};
```

This enables testing **any syntax × CPU combination**.

---

## Running Tests

### Build and Run All Tests

```bash
# Build test suite
cmake --build build --target binary_compat_test

# Run all tests (includes failing tests)
./build/tests/integration/binary_compat/binary_compat_test

# Run only currently working tests
./build/tests/integration/binary_compat/binary_compat_test --gtest_filter="WorkingFeatures*"
```

### Run Specific Test Suite

```bash
# Merlin tests only
./build/tests/integration/binary_compat/binary_compat_test --gtest_filter="Merlin*"

# SCMASM tests only
./build/tests/integration/binary_compat/binary_compat_test --gtest_filter="Scmasm*"

# Specific test case
./build/tests/integration/binary_compat/binary_compat_test --gtest_filter="*01_basic_ops*"
```

---

## Current Status

### ✅ Passing Tests (3/13)

| Test Suite | Test Case | Syntax | CPU | Status |
|------------|-----------|--------|-----|--------|
| WorkingFeatures | 01_basic_ops | merlin | 6502 | ✅ PASS |
| Merlin6502 | 01_basic_ops | merlin | 6502 | ✅ PASS |
| Scmasm6502 | 01_basic_ops | merlin | 6502 | ✅ PASS (uses Merlin file) |

### ❌ Blocked Tests (10/13)

**BLOCKER 1: SCMASM Syntax Not Implemented**
- Impact: All SCMASM-specific tests fail
- Error: `--syntax: scmasm not in {simple,merlin}`
- Resolution: Implement SCMASM syntax parser/emitter

**BLOCKER 2: Missing Merlin Directives**
- Impact: 4 Merlin tests fail
- Missing: DA, DCI, INV, FLS, END
- Resolution: Implement these directives

**BLOCKER 3: Macro System Issues**
- Impact: Merlin macro test fails
- Error: `Undefined macro: INIT`
- Resolution: Debug/fix macro definition/expansion

See `BLOCKER_ANALYSIS.md` for detailed analysis.

---

## Test Categories

### 01_basic_ops.asm ✅
- Basic 6502 opcodes (LDA, STA, ADC, SBC, etc.)
- Immediate, zero-page, absolute addressing
- **Status:** PASSING

### 02_addressing_modes.asm ⚠️
- All 6502 addressing modes
- Indexed addressing (X, Y)
- Indirect addressing
- **Status:** BLOCKED (needs END directive)

### 03_data_directives.asm ⚠️
- DB (byte), DW (word), DA (address)
- DCI (inverted string)
- INV, FLS (text attributes)
- **Status:** BLOCKED (needs DA, DCI, INV, FLS, END)

### 04_macros.asm ⚠️
- Macro definition (>>> / <<<)
- Macro expansion
- Nested macros
- **Status:** BLOCKED (macro system issue)

### 05_conditionals.asm ⚠️
- DO/FIN conditionals
- IF/ELSE/ENDIF
- Assembly-time expressions
- **Status:** BLOCKED (needs END directive)

---

## Adding New Tests

### 1. Create Test Source File

```bash
# Create test file
tests/integration/binary_compat/test_sources/[syntax]/[cpu]/[name].asm
```

### 2. Generate Golden Binary

```bash
# Example: Generate Merlin 6502 golden binary
vasm6502_merlin -Fbin -dotdir \
  -o tests/integration/binary_compat/golden/merlin/6502/[name].bin \
  tests/integration/binary_compat/test_sources/merlin/6502/[name].asm
```

### 3. Add Test Case

```cpp
// In test_merlin_compat.cpp (or appropriate test file)
INSTANTIATE_TEST_SUITE_P(
    Merlin6502,
    BinaryCompatTest,
    ::testing::Values(
        // ... existing tests ...
        BinaryCompatTestCase{
            "test_sources/merlin/6502/[name].asm",
            "merlin",
            "6502",
            "vasm6502_merlin",
            ""
        }
    ),
    MerlinTestCaseName
);
```

### 4. Run Test

```bash
cmake --build build --target binary_compat_test
./build/tests/integration/binary_compat/binary_compat_test --gtest_filter="*[name]*"
```

---

## Adding New Syntax/CPU Combinations

The framework supports ANY syntax × CPU combination:

### Example: Add 65C02 Support

1. **Create test sources:**
   ```
   test_sources/merlin/65c02/01_basic_ops.asm
   ```

2. **Generate golden binary:**
   ```bash
   vasm6502_merlin -Fbin -dotdir -m65c02 \
     -o golden/merlin/65c02/01_basic_ops.bin \
     test_sources/merlin/65c02/01_basic_ops.asm
   ```

3. **Add test case:**
   ```cpp
   BinaryCompatTestCase{
       "test_sources/merlin/65c02/01_basic_ops.asm",
       "merlin",
       "65c02",      // Different CPU
       "vasm6502_merlin",
       "-m65c02"     // CPU-specific flag
   }
   ```

### Example: Add Z80 Support

1. **Create test sources:**
   ```
   test_sources/z80/basic/01_basic_ops.asm
   ```

2. **Generate golden binary:**
   ```bash
   vasmz80_std -Fbin \
     -o golden/z80/basic/01_basic_ops.bin \
     test_sources/z80/basic/01_basic_ops.asm
   ```

3. **Create test suite:**
   ```cpp
   // test_z80_compat.cpp
   INSTANTIATE_TEST_SUITE_P(
       Z80Basic,
       BinaryCompatTest,
       ::testing::Values(
           BinaryCompatTestCase{
               "test_sources/z80/basic/01_basic_ops.asm",
               "z80",
               "z80",
               "vasmz80_std",
               ""
           }
       ),
       Z80TestCaseName
   );
   ```

---

## CI/CD Integration

The framework is ready for CI/CD automation:

### GitHub Actions Workflow

```yaml
name: Binary Compatibility Tests

on: [push, pull_request]

jobs:
  binary-compat:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Build vasm-ext
        run: |
          git clone https://github.com/mist64/vasm-ext
          cd vasm-ext
          make CPU=6502 SYNTAX=merlin
          make CPU=6502 SYNTAX=scmasm
          sudo cp vasm6502_* /usr/local/bin/

      - name: Build xasm++
        run: |
          cmake -B build
          cmake --build build --target binary_compat_test

      - name: Run Binary Compatibility Tests
        run: |
          ./build/tests/integration/binary_compat/binary_compat_test
```

---

## Troubleshooting

### Test Fails: "Assembly failed"

**Cause:** xasm++ doesn't support feature used in test.

**Solution:** Check error message for specific unsupported instruction/directive. Either:
1. Implement the missing feature, or
2. Move test to "blocked" category

### Test Fails: "Binaries don't match"

**Cause:** xasm++ output differs from vasm reference.

**Solution:**
1. Generate hex dump comparison:
   ```bash
   xxd /tmp/xasm_test_output.bin > xasm.hex
   xxd golden/merlin/6502/test.bin > vasm.hex
   diff -u vasm.hex xasm.hex
   ```
2. Investigate byte-by-byte differences
3. Fix xasm++ code generation issue

### Test Fails: "Golden binary not found"

**Cause:** Reference binary not generated.

**Solution:**
```bash
# Generate missing golden binary
vasm6502_merlin -Fbin -dotdir \
  -o golden/merlin/6502/test.bin \
  test_sources/merlin/6502/test.asm
```

---

## Reference Documentation

- **Framework Design:** `.ai/tasks/2026-01-30_binary-compat-framework/10-plan.md`
- **Blocker Analysis:** `.ai/tasks/2026-01-30_binary-compat-framework/BLOCKER_ANALYSIS.md`
- **vasm Documentation:** https://github.com/mist64/vasm-ext
- **GTest Parameterized Tests:** https://google.github.io/googletest/advanced.html#value-parameterized-tests

---

## Future Expansion

The framework is designed for expansion:

### Planned Syntax/CPU Combinations

**Priority 1 (6502 Family):**
- [x] Merlin + 6502 ✅
- [ ] Merlin + 65C02
- [ ] Merlin + 65816
- [ ] SCMASM + 6502
- [ ] SCMASM + 65C02
- [ ] SCMASM + 65816

**Priority 2 (Additional CPUs):**
- [ ] 6809 (when implemented)
- [ ] Z80 (when implemented)
- [ ] 68000 (when implemented)

### Test Coverage Goals

- **Basic Opcodes:** All standard instructions
- **Addressing Modes:** All CPU-supported modes
- **Data Directives:** DB, DW, DA, DS, etc.
- **Macros:** Definition, expansion, nesting
- **Conditionals:** DO/FIN, IF/ELSE/ENDIF
- **Labels:** Global, local, anonymous
- **Expressions:** Arithmetic, logical, addresses
- **Sections:** ORG, multiple origins

---

**Last Updated:** 2026-01-30
