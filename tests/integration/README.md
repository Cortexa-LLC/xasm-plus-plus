# Integration Tests

End-to-end validation of the complete xasm++ assembly pipeline (CLI → Parser → Assembler → Output).

## Overview

Integration tests assemble real 6502 assembly programs and verify binary output correctness through byte-for-byte comparison with expected golden files.

## Test Programs

| Test | File | Purpose |
|------|------|---------|
| Test 1 | `01_hello.asm` | Basic instructions (LDA, STA, NOP, RTS) |
| Test 2 | `02_labels.asm` | JMP instruction, labels, absolute addressing |
| Test 3 | `03_data.asm` | Data directives (.db, .dw), little-endian encoding |
| Test 4 | `04_org.asm` | Origin directive (.org), address changes |
| Test 5 | `05_comments.asm` | Comment handling (full-line and inline) |

## Running Tests

```bash
# Run all tests (unit + integration)
cd build && ctest

# Run only integration tests
cd build && ctest -R integration

# Run specific integration test
cd build && ctest -R integration_hello

# Run with verbose output
cd build && ctest -R integration -V

# Run with failure details
cd build && ctest -R integration --output-on-failure
```

## Test Structure

Each test consists of:

1. **Source file** (`XX_name.asm`) - Assembly program
2. **Golden file** (`expected/XX_name.bin`) - Expected binary output
3. **CMake test** - Automated assembly and comparison

### Test Execution Flow

```
XX_name.asm
    ↓ (xasm++ assembles)
XX_name.bin (generated)
    ↓ (byte-for-byte comparison)
expected/XX_name.bin (golden)
    ↓
PASS or FAIL
```

## Adding New Integration Tests

### Step 1: Create Test Program

```bash
# Create assembly source file
vim tests/integration/06_new_feature.asm
```

Example:
```asm
; Test 6: New Feature
; Tests: Description of what this tests
    .org $8000
    ; Your test code here
    NOP
```

### Step 2: Assemble and Verify

```bash
# Assemble the test program
./build/src/xasm++ tests/integration/06_new_feature.asm \
    -o tests/integration/06_new_feature.bin

# Inspect binary output
xxd tests/integration/06_new_feature.bin

# Verify output is correct (check opcodes match expectations)
```

### Step 3: Create Golden File

```bash
# Once verified correct, save as golden file
cp tests/integration/06_new_feature.bin \
   tests/integration/expected/06_new_feature.bin
```

### Step 4: Add to CMake

Edit `tests/integration/CMakeLists.txt`:

```cmake
set(TEST_PROGRAMS
  01_hello
  02_labels
  03_data
  04_org
  05_comments
  06_new_feature  # Add your test here
)
```

### Step 5: Reconfigure and Test

```bash
# Reconfigure CMake to pick up new test
cd build && cmake ..

# Build and run new test
ctest -R integration_new_feature --output-on-failure
```

## Regenerating Golden Files

If you modify the assembler or fix a bug, you may need to regenerate golden files:

```bash
# WARNING: Only do this if you're SURE the new output is correct!

# Regenerate all golden files
for test in 01_hello 02_labels 03_data 04_org 05_comments; do
  ./build/src/xasm++ tests/integration/${test}.asm \
      -o tests/integration/${test}.bin
  cp tests/integration/${test}.bin \
     tests/integration/expected/${test}.bin
done

# Verify tests still pass
cd build && ctest -R integration
```

## Troubleshooting

### Test Fails: "Files differ"

```bash
# Run test with output
ctest -R integration_hello --output-on-failure

# Compare files manually
xxd tests/integration/01_hello.bin > /tmp/actual.hex
xxd tests/integration/expected/01_hello.bin > /tmp/expected.hex
diff /tmp/actual.hex /tmp/expected.hex
```

### Test Fails: Assembly Error

```bash
# Run xasm++ directly to see error message
./build/src/xasm++ tests/integration/01_hello.asm \
    -o /tmp/test.bin
```

### Golden File Missing

```bash
# Ensure golden file exists
ls -la tests/integration/expected/

# If missing, regenerate (see above)
```

## Test Coverage

Current integration test coverage:

- **Instructions**: LDA, STA, JMP, NOP, RTS
- **Addressing modes**: Immediate, Absolute, Implied
- **Directives**: .org, .db, .dw
- **Features**: Labels, Comments
- **Output format**: Raw binary

**NOT yet covered** (Phase 2+):
- Symbol resolution (labels as operands)
- All 6502 instructions
- All addressing modes
- Error handling
- Multi-pass assembly
- Cross-references

## Implementation Notes

### Phase 1 Limitations

1. **Symbol Resolution**: Labels defined but not yet resolved as operands
   - Workaround: Use hex addresses instead (e.g., `JMP $8007` vs `JMP forward`)

2. **.org Gaps**: Multiple .org directives don't create gaps in output
   - Limitation: Simple binary format writes atoms sequentially
   - Phase 2: Implement proper section-based output

3. **CPU Plugin**: Hardcoded to Cpu6502 in main.cpp
   - Phase 2: Make CPU plugin selectable via --cpu flag

### Test Philosophy

**Pragmatic TDD**: Write test programs, verify manually first, then automate comparison.

**Keep it simple**: Use CMake's built-in `compare_files` - no complex test framework needed.

**Fast execution**: All tests run in <2 seconds - suitable for CI/CD.

## References

- 6502 Instruction Set: http://www.6502.org/tutorials/6502opcodes.html
- CMake add_test: https://cmake.org/cmake/help/latest/command/add_test.html
- CTest: https://cmake.org/cmake/help/latest/manual/ctest.1.html

---

**Last Updated**: 2026-01-21
**Test Count**: 5 programs, 10 CMake tests (setup + comparison)
**Pass Rate**: 100% (111/111 total tests including unit tests)
