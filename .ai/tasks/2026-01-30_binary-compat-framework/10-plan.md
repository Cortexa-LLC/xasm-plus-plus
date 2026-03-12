# Implementation Plan

**Task ID:** 2026-01-30_binary-compat-framework
**Workflow:** Feature (standard.md)
**Role:** Engineer
**Execution:** Sequential (foundational dependencies)

---

## ⚠️ CRITICAL UPDATE: Current Status (40% Complete - 2026-01-30)

**COMPLETED (DO NOT REDO):**
- ✅ Framework infrastructure (BinaryCompatTest, golden comparison, GTest parameterization)
- ✅ Python regeneration script (scripts/regenerate_golden_binaries.py - 324 lines)
- ✅ Merlin 6502: 5/5 test sources + 5/5 golden binaries
- ✅ SCMASM 6502: 5/5 test sources (only 1/5 golden binary though)
- ⚠️ SCMASM 65C02: 2 test sources (0 golden binaries)
- ⚠️ SCMASM 65816: 2 test sources (0 golden binaries)

**REMAINING WORK (Execute in this EXACT order):**

### Step 1: Fix Test Files (CRITICAL - Enables Running Tests)
1. Update tests/integration/binary_compat/test_merlin_compat.cpp:
   - Currently only tests 1 case (01_basic_ops)
   - Add remaining 4 Merlin+6502 test cases to INSTANTIATE_TEST_SUITE_P
   - Files exist: 02_addressing_modes, 03_data_directives, 04_macros, 05_conditionals

2. Create tests/integration/binary_compat/test_scmasm_compat.cpp:
   - Copy structure from test_merlin_compat.cpp
   - Add SCMASM+6502 test case for 03_data_directives (only one with golden binary)
   - Will expand after generating more golden binaries

### Step 2: Generate Missing Golden Binaries (24 files)
3. Run: `python scripts/regenerate_golden_binaries.py`
   - This will generate golden binaries for all existing test sources
   - Should create: SCMASM 6502 (4 missing), SCMASM 65C02 (2), SCMASM 65816 (2)

### Step 3: Create Missing Test Sources (16 files)
4. Merlin 65C02: 5 test sources
   - 01_65c02_new_instructions.asm (BRA, PHX, PLY, PHZ)
   - 02_65c02_stz.asm (STZ zero page, absolute)
   - 03_65c02_bit_ops.asm (TRB, TSB)
   - 04_65c02_indirect_zp.asm ((zp) addressing mode)
   - 05_65c02_jmp_indirect.asm (JMP (abs,X))

5. Merlin 65816: 5 test sources
   - 01_65816_rep_sep.asm (REP, SEP, status register)
   - 02_65816_24bit.asm (24-bit addressing, long opcodes)
   - 03_65816_jsl_rtl.asm (JSL, JML, RTL)
   - 04_65816_mvn_mvp.asm (MVN, MVP block moves)
   - 05_65816_stack.asm (PEA, PEI, PER)

6. SCMASM 65C02: 3 more test sources (have 2, need 5 total)
   - 03_65c02_bit_ops.asm
   - 04_65c02_indirect.asm
   - 05_65c02_jmp.asm

7. SCMASM 65816: 3 more test sources (have 2, need 5 total)
   - 03_65816_jsl_rtl.asm
   - 04_65816_mvn_mvp.asm
   - 05_65816_stack.asm

8. After each set, run: `python scripts/regenerate_golden_binaries.py`

### Step 4: Update Test Files with New Cases
9. Update test_merlin_compat.cpp with Merlin 65C02/65816 test cases
10. Update test_scmasm_compat.cpp with all SCMASM test cases

### Step 5: CI Integration
11. Create .github/workflows/binary-compat.yml

### Step 6: Documentation
12. Create tests/integration/binary_compat/README.md

**DO NOT CLOSE TASK until ALL 6 combinations have passing tests (30/30 test sources, 30/30 golden binaries, both test files complete, CI added, README written).**

---

## Original Implementation Plan (For Reference)

## Phase 1: Build vasm-ext Reference Binaries (Foundation)

**Objective:** Compile reference assemblers for comparison

```bash
cd ~/Projects/Vintage/tools/vasm-ext

# Build Merlin syntax variant
make CPU=6502 SYNTAX=merlin
# Produces: vasmm6502_merlin

# Build SCMASM syntax variant  
make CPU=6502 SYNTAX=scmasm
# Produces: vasmm6502_scmasm
```

**Acceptance:** Both binaries exist and run successfully

## Phase 2: Test Framework Infrastructure

**Objective:** Create reusable test harness

### 2.1 Directory Structure
```
tests/integration/binary_compat/
├── framework/
│   ├── BinaryCompatTest.h
│   ├── BinaryCompatTest.cpp
│   └── reference_runner.sh
├── test_sources/
│   ├── merlin/
│   │   ├── 6502/
│   │   ├── 65c02/
│   │   └── 65816/
│   └── scmasm/
│       ├── 6502/
│       ├── 65c02/
│       └── 65816/
├── test_merlin_compat.cpp
├── test_scmasm_compat.cpp
├── CMakeLists.txt
└── README.md
```

### 2.2 Base Test Class (framework/BinaryCompatTest.h)
```cpp
#pragma once
#include <gtest/gtest.h>
#include <string>

namespace xasm {
namespace test {

struct TestCase {
    std::string source_file;
    std::string syntax;      // "merlin", "scmasm"
    std::string cpu;         // "6502", "65c02", "65816"
    std::string vasm_binary; // "vasmm6502_merlin" or "vasmm6502_scmasm"
    std::string vasm_flags;  // "", "-m65c02", "-m65816"
};

class BinaryCompatTest : public ::testing::TestWithParam<TestCase> {
protected:
    void SetUp() override;
    void TearDown() override;
    
    // Compare xasm++ output with vasm-ext reference
    void CompareWithReference(const TestCase& test);
    
private:
    std::string temp_dir_;
};

} // namespace test
} // namespace xasm
```

### 2.3 Comparison Script (framework/reference_runner.sh)
```bash
#!/bin/bash
# Compare xasm++ output with vasm-ext reference

SOURCE_FILE=$1
SYNTAX=$2
CPU=$3
VASM_BINARY=$4
VASM_FLAGS=$5

VASM_PATH=~/Projects/Vintage/tools/vasm-ext/$VASM_BINARY
XASM_PATH=./build/xasm++

# Assemble with vasm-ext
$VASM_PATH $VASM_FLAGS -Fbin -o /tmp/vasm.bin "$SOURCE_FILE" 2>&1 || exit 2

# Assemble with xasm++
$XASM_PATH --syntax $SYNTAX --cpu $CPU -o /tmp/xasm.bin "$SOURCE_FILE" 2>&1 || exit 3

# Compare binaries
if cmp -s /tmp/vasm.bin /tmp/xasm.bin; then
    exit 0  # Match
else
    # Generate diff for debugging
    hexdump -C /tmp/vasm.bin > /tmp/vasm.hex
    hexdump -C /tmp/xasm.bin > /tmp/xasm.hex
    diff -u /tmp/vasm.hex /tmp/xasm.hex > /tmp/binary.diff
    exit 1  # Mismatch
fi
```

## Phase 3: Test Source Files (6502 Family)

### 3.1 Merlin Syntax Tests

**merlin/6502/ (5 files):**
1. `01_basic_ops.asm` - LDA, STA, ADC, etc. (common instructions)
2. `02_addressing_modes.asm` - All 6502 addressing modes
3. `03_macros.asm` - PMC/MAC/EOM, >>>MACRO
4. `04_conditionals.asm` - DO/ELSE/FIN
5. `05_data.asm` - DFB, DA, ASC, HEX, DS

**merlin/65c02/ (5 files):**
1. `01_new_instructions.asm` - BRA, PHX, PLY, STZ, TRB, TSB
2. `02_zp_indirect.asm` - (zp) addressing mode
3. `03_jmp_indirect_fix.asm` - Fixed JMP (ind)
4. `04_macros_65c02.asm` - Macros using 65C02 ops
5. `05_combined.asm` - Real-world 65C02 code

**merlin/65816/ (5 files):**
1. `01_16bit_mode.asm` - REP/SEP mode switching
2. `02_long_addressing.asm` - 24-bit addresses ($HHLLLL)
3. `03_long_jumps.asm` - JML, JSL, RTL
4. `04_block_moves.asm` - MVN, MVP
5. `05_stack_ops.asm` - PEA, PEI, PER, PHB, PHD

### 3.2 SCMASM Syntax Tests

**scmasm/6502/ (5 files):**
1. `01_directives.asm` - .OR, .EQ, .SE, .DA, .HS
2. `02_strings.asm` - .AS, .AT, .AZ with delimiter rules
3. `03_macros.asm` - .MA/.EM with \0-\9 parameters
4. `04_conditionals.asm` - .DO/.ELSE/.FIN
5. `05_loops.asm` - .LU/.ENDU

**scmasm/65c02/ (5 files):**
1. `01_new_instructions.asm` - 65C02 opcodes
2. `02_zp_indirect.asm` - (zp) mode
3. `03_macros_65c02.asm` - Macros with 65C02
4. `04_strings_advanced.asm` - Complex string directives
5. `05_combined.asm` - Real-world 65C02 code

**scmasm/65816/ (5 files):**
1. `01_16bit_mode.asm` - REP/SEP
2. `02_long_addressing.asm` - 24-bit
3. `03_long_jumps.asm` - JML/JSL/RTL
4. `04_macros_65816.asm` - 65816-specific macros
5. `05_combined.asm` - Real-world 65816 code

## Phase 4: GTest Integration

### 4.1 Merlin Test Suite (test_merlin_compat.cpp)
```cpp
#include "framework/BinaryCompatTest.h"

using namespace xasm::test;

INSTANTIATE_TEST_SUITE_P(
    Merlin6502,
    BinaryCompatTest,
    ::testing::Values(
        TestCase{"merlin/6502/01_basic_ops.asm", "merlin", "6502", "vasmm6502_merlin", ""},
        TestCase{"merlin/6502/02_addressing_modes.asm", "merlin", "6502", "vasmm6502_merlin", ""},
        // ... all 15 Merlin tests
    )
);
```

### 4.2 SCMASM Test Suite (test_scmasm_compat.cpp)
```cpp
INSTANTIATE_TEST_SUITE_P(
    Scmasm6502Family,
    BinaryCompatTest,
    ::testing::Values(
        TestCase{"scmasm/6502/01_directives.asm", "scmasm", "6502", "vasmm6502_scmasm", ""},
        // ... all 15 SCMASM tests
    )
);
```

### 4.3 CMake Integration
```cmake
# tests/integration/binary_compat/CMakeLists.txt
add_executable(test_binary_compat
    framework/BinaryCompatTest.cpp
    test_merlin_compat.cpp
    test_scmasm_compat.cpp
)

target_link_libraries(test_binary_compat
    gtest
    gtest_main
    xasm_core
)

gtest_discover_tests(test_binary_compat)
```

## Phase 5: CI/CD Integration

### 5.1 GitHub Actions Workflow
```yaml
# .github/workflows/binary-compat.yml
name: Binary Compatibility Tests

on: [push, pull_request]

jobs:
  binary-compat:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: true
      
      - name: Build vasm-ext
        run: |
          cd ~/Projects/Vintage/tools/vasm-ext
          make CPU=6502 SYNTAX=merlin
          make CPU=6502 SYNTAX=scmasm
      
      - name: Build xasm++
        run: |
          cmake -B build
          cmake --build build
      
      - name: Run compatibility tests
        run: |
          cd build
          ctest -R BinaryCompat --output-on-failure
```

## Phase 6: Documentation

### 6.1 README.md
- Framework architecture
- How to run tests
- How to add new combinations
- Troubleshooting guide

### 6.2 Test Source Comments
- Each .asm file has header explaining what it tests
- Expected behavior documented

## Dependencies

### Sequential Phases
1. Phase 1 must complete before Phase 2 (need vasm binaries)
2. Phase 2 must complete before Phase 3 (need framework)
3. Phase 3 must complete before Phase 4 (need test sources)
4. Phase 4 can run in parallel with Phase 5

### External Dependencies
- vasm-ext source code available
- CMake 3.15+
- GTest framework
- GitHub Actions (for CI)

## Risk Mitigation

**Risk:** vasm-ext build failures
**Mitigation:** Document build process, test on multiple platforms

**Risk:** Binary mismatches due to vasm bugs
**Mitigation:** Document known issues, verify against vintage hardware when possible

**Risk:** 65816 complexity
**Mitigation:** Start with basic tests, expand incrementally

## Success Criteria

- All 30 test files created
- All 6 syntax/CPU combinations tested
- 100% binary match with vasm-ext (or documented discrepancies)
- Tests integrated into CI
- Documentation complete

## Estimated Timeline

- Phase 1: 30 minutes
- Phase 2: 4 hours
- Phase 3: 6 hours
- Phase 4: 2 hours
- Phase 5: 2 hours
- Phase 6: 1 hour
- **Total: ~15.5 hours**
