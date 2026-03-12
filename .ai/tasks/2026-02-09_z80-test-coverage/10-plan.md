# Implementation Plan: Z80 Test Coverage Improvement

**Task:** 2026-02-09_z80-test-coverage
**Beads ID:** xasm++-37qd
**Workflow:** Testing enhancement

## Workflow Selection

Using: `.ai-pack/workflows/standard.md` with Tester focus

**Rationale:** This is primarily a testing task focused on coverage improvement.

## Implementation Approach

### Phase 1: Coverage Audit (1 hour)

**Goal:** Establish baseline metrics

**Steps:**

1. **Enable coverage in build:**
   ```bash
   # Configure with coverage flags
   cmake -B build -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_CXX_FLAGS="--coverage" \
     -DCMAKE_EXE_LINKER_FLAGS="--coverage"

   # Build
   cmake --build build
   ```

2. **Run existing tests:**
   ```bash
   cd build
   ctest -R test_cpu_z80 --output-on-failure
   cd ..
   ```

3. **Generate coverage report (try both methods):**
   ```bash
   # Method 1: lcov (detailed HTML report)
   lcov --capture --directory build --output-file coverage.info
   lcov --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage_filtered.info
   genhtml coverage_filtered.info --output-directory coverage_html
   open coverage_html/index.html

   # Method 2: gcovr (simpler, may be easier)
   gcovr --root . --html --html-details \
     --exclude '/usr/*' --exclude '*/tests/*' \
     -o coverage.html
   open coverage.html
   ```

4. **Document findings in work log:**
   - Current overall coverage %
   - Coverage per file (focus on cpu_z80.cpp)
   - List of untested functions
   - List of untested branches

**Expected Outcome:** Baseline report showing current coverage (likely 60-80%)

### Phase 2: Gap Analysis (1 hour)

**Goal:** Identify what needs testing

**Steps:**

1. **Analyze coverage report:**
   - Look for red/yellow highlighted code in HTML report
   - List all functions with <90% coverage
   - Identify completely untested functions

2. **Cross-reference with Z80 implementation:**
   ```bash
   # List all Encode* methods
   grep -n "^std::vector<uint8_t> CpuZ80::Encode" src/cpu/z80/cpu_z80.cpp
   ```

3. **Check existing test file:**
   ```bash
   # See what's already tested
   grep -n "TEST_CASE.*Z80" tests/unit/test_cpu_z80.cpp
   ```

4. **Create gap matrix:**
   | Instruction | Tested? | Coverage | Priority |
   |-------------|---------|----------|----------|
   | LD A,n | ✓ | 100% | - |
   | BIT | ✗ | 0% | P0 |
   | ... | ... | ... | ... |

5. **Prioritize gaps:**
   - **P0 (Critical):** Core instructions used frequently
   - **P1 (Important):** Advanced features, prefix instructions
   - **P2 (Nice-to-have):** Edge cases, rarely used instructions

**Expected Outcome:** Prioritized backlog of 50-100 test cases to write

### Phase 3: Test Implementation (4-6 hours)

**Goal:** Write tests to fill gaps

**TDD Approach: RED-GREEN-REFACTOR**

**3.1 Setup Test Structure:**
```cpp
// tests/unit/test_cpu_z80.cpp

TEST_CASE("Z80 - [Category] - [Instruction]", "[z80][category]") {
  xasm::CpuZ80 cpu;

  SECTION("[specific test case]") {
    // Arrange
    // Act
    auto result = cpu.EncodeXYZ(...);
    // Assert
    REQUIRE(result == std::vector<uint8_t>{...});
  }
}
```

**3.2 Batch Tests by Priority:**

**Batch 1: P0 Critical Gaps (2 hours)**
- Focus on untested core instructions
- Goal: Reach 80% coverage

**Batch 2: P1 Important Gaps (2 hours)**
- Prefix instructions (CB, ED, DD, FD)
- Advanced addressing modes
- Goal: Reach 90% coverage

**Batch 3: P2 Edge Cases (1-2 hours)**
- Boundary values
- Error conditions
- Goal: Reach 95% coverage

**3.3 Test Pattern:**

For each instruction group:
```cpp
TEST_CASE("Z80 - 8-bit Load - LD r,n", "[z80][load]") {
  xasm::CpuZ80 cpu;

  SECTION("LD A,n") {
    auto result = cpu.EncodeLD_A_n(0x42);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 0x3E);  // LD A,n opcode
    REQUIRE(result[1] == 0x42);  // Immediate value
  }

  SECTION("LD A,n - Edge case (0x00)") {
    auto result = cpu.EncodeLD_A_n(0x00);
    REQUIRE(result == std::vector<uint8_t>{0x3E, 0x00});
  }

  SECTION("LD A,n - Edge case (0xFF)") {
    auto result = cpu.EncodeLD_A_n(0xFF);
    REQUIRE(result == std::vector<uint8_t>{0x3E, 0xFF});
  }

  // Repeat for B, C, D, E, H, L
}
```

**3.4 Run coverage after each batch:**
```bash
cmake --build build
cd build && ctest -R test_cpu_z80
gcovr --root .. --html -o coverage_batch_X.html
```

**Expected Outcome:** 90%+ coverage with comprehensive test suite

### Phase 4: Verification (1 hour)

**Goal:** Confirm targets met

**Steps:**

1. **Generate final coverage report:**
   ```bash
   cmake --build build
   cd build && ctest
   gcovr --root .. --html --html-details -o ../coverage_final.html
   ```

2. **Verify metrics:**
   - [ ] Overall coverage ≥90%
   - [ ] cpu_z80.cpp coverage ≥90%
   - [ ] Critical functions coverage ≥95%
   - [ ] All tests passing

3. **Document results:**
   | Metric | Before | After | Target | Met? |
   |--------|--------|-------|--------|------|
   | Overall | TBD% | TBD% | 90% | ✓/✗ |
   | cpu_z80.cpp | TBD% | TBD% | 90% | ✓/✗ |
   | Critical | TBD% | TBD% | 95% | ✓/✗ |

4. **Test quality check:**
   - No duplicate tests
   - Clear test names
   - Good SECTION organization
   - Edge cases covered

**Expected Outcome:** All targets met, documented in acceptance

### Phase 5: CI Integration (Optional, 1 hour)

**Goal:** Prevent coverage regression

**Steps:**

1. **Add coverage target to CMake:**
   ```cmake
   # tests/unit/CMakeLists.txt
   if(CMAKE_BUILD_TYPE STREQUAL "Debug")
     option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
     if(ENABLE_COVERAGE)
       target_compile_options(test_cpu_z80 PRIVATE --coverage)
       target_link_options(test_cpu_z80 PRIVATE --coverage)
     endif()
   endif()
   ```

2. **Add coverage check script:**
   ```bash
   #!/bin/bash
   # scripts/check_coverage.sh
   MIN_COVERAGE=90
   ACTUAL=$(gcovr --root . | grep TOTAL | awk '{print $4}' | sed 's/%//')
   if (( $(echo "$ACTUAL < $MIN_COVERAGE" | bc -l) )); then
     echo "❌ Coverage $ACTUAL% < $MIN_COVERAGE%"
     exit 1
   fi
   echo "✅ Coverage $ACTUAL% ≥ $MIN_COVERAGE%"
   ```

3. **Document usage:**
   ```markdown
   # docs/testing/coverage.md

   ## Running Coverage

   \`\`\`bash
   cmake -B build -DENABLE_COVERAGE=ON
   cmake --build build
   cd build && ctest
   gcovr --root .. --html -o coverage.html
   \`\`\`

   ## Coverage Requirements

   - Overall: ≥90%
   - cpu_*.cpp: ≥90%
   - Critical functions: ≥95%
   ```

**Expected Outcome:** Coverage tracking automated

## Execution Strategy

**Sequential** - Each phase depends on previous:
1. Phase 1 (audit) must complete before Phase 2 (analysis)
2. Phase 2 must complete before Phase 3 (implementation)
3. Phase 3 can be batched internally (parallel test writing)
4. Phase 4 verifies all previous work
5. Phase 5 is optional enhancement

## TDD Approach

**For new tests:**
1. ❌ RED: Write test for untested instruction
2. ✅ GREEN: Verify test passes (code already exists)
3. ♻️ REFACTOR: Improve test clarity if needed

**Note:** Since we're testing existing code, tests should pass immediately (GREEN). If a test fails, it indicates either:
- Bug in implementation (fix required)
- Incorrect test (fix test)

## Risk Mitigation

**Risk:** Coverage tool not available on system
- **Mitigation:** Provide multiple options (lcov, gcovr), document installation

**Risk:** Existing code has bugs revealed by new tests
- **Mitigation:** This is actually good! Fix bugs as discovered

**Risk:** Tests become too slow
- **Mitigation:** Use Catch2 sections for organization, parallelize with ctest

## Success Metrics

**Before:**
- Coverage: Unknown
- Test count: ~88 tests
- Untested code: Unknown %

**After:**
- Coverage: ≥90% (target)
- Test count: ~150-200 tests (+60-120)
- Untested code: <10%
- All tests passing: ✓

## Tools and Commands

```bash
# Enable coverage
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"

# Build and test
cmake --build build && cd build && ctest

# Generate report (simple)
gcovr --root . --html -o coverage.html

# Generate report (detailed)
lcov --capture --directory . -o coverage.info
genhtml coverage.info -o coverage_html

# Check specific file
gcovr --root . --filter 'src/cpu/z80/cpu_z80.cpp'
```

## Notes

- Coverage is a means to an end (quality), not the end itself
- 90% is a good target, 100% is often unrealistic/wasteful
- Focus on critical paths first, nice-to-haves last
- Edge case coverage is more valuable than percentage points
