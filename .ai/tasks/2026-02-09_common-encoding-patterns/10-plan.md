# Implementation Plan: Extract Common CPU Encoding Patterns

**Task:** 2026-02-09_common-encoding-patterns
**Beads ID:** xasm++-vray
**Workflow:** Refactor (DRY improvement)

## Workflow Selection

Using: `.ai-pack/workflows/refactor.md`

**Rationale:** This is a refactoring task to eliminate code duplication while preserving behavior.

## Implementation Approach

### Phase 1: Pattern Analysis (1 hour)

**Goal:** Identify all duplicated encoding patterns

**Steps:**
1. Read all CPU implementation files:
   ```bash
   src/cpu/m6502/cpu_6502.cpp
   src/cpu/z80/cpu_z80.cpp
   src/cpu/m6809/cpu_6809.cpp
   ```

2. Grep for common patterns:
   ```bash
   # Endianness conversion
   grep -n "ToLittleEndian\|ToBigEndian" src/cpu/*/*.cpp

   # Bit manipulation
   grep -n "& 0xFF\|>> 8\|<< " src/cpu/*/*.cpp

   # Vector construction
   grep -n "std::vector<uint8_t>" src/cpu/*/*.cpp
   ```

3. Document findings in work log with:
   - Pattern description
   - Current implementations (with line numbers)
   - Duplication count
   - Extraction priority

**Expected Outcome:** Comprehensive list of patterns to extract

### Phase 2: Utility Design (1 hour)

**Goal:** Design clean, type-safe utility API

**Steps:**
1. Create API design document in work log:
   ```cpp
   namespace xasm::encoding {
     // Endianness (constexpr for compile-time)
     constexpr std::array<uint8_t, 2> ToLittleEndian16(uint16_t value);
     constexpr std::array<uint8_t, 2> ToBigEndian16(uint16_t value);

     // Byte extraction
     constexpr uint8_t LowByte(uint16_t value);
     constexpr uint8_t HighByte(uint16_t value);
     constexpr uint8_t LowNibble(uint8_t value);
     constexpr uint8_t HighNibble(uint8_t value);

     // Range validation
     constexpr bool FitsIn8Bits(uint32_t value);
     constexpr bool FitsIn16Bits(uint32_t value);
     constexpr bool FitsInSignedByte(int32_t value);

     // Vector helpers
     inline std::vector<uint8_t> WithPrefix(uint8_t prefix, std::initializer_list<uint8_t> bytes);
     inline std::vector<uint8_t> WithPrefixes(std::initializer_list<uint8_t> prefixes, std::initializer_list<uint8_t> bytes);
   }
   ```

2. Document each function's:
   - Purpose
   - Parameters
   - Return value
   - Example usage
   - Why constexpr (if applicable)

**Expected Outcome:** Clear API design ready for implementation

### Phase 3: TDD Implementation (2-3 hours)

**Goal:** Implement utilities with comprehensive tests

**RED-GREEN-REFACTOR cycle for each utility:**

**3.1 Create Test File First**
```bash
touch tests/unit/test_encoding_utils.cpp
```

**3.2 For Each Utility (RED → GREEN → REFACTOR):**

**RED (Write failing test):**
```cpp
TEST_CASE("ToLittleEndian16 converts 16-bit value to little-endian bytes") {
  auto result = xasm::encoding::ToLittleEndian16(0x1234);
  REQUIRE(result[0] == 0x34);  // Low byte
  REQUIRE(result[1] == 0x12);  // High byte
}
// Test fails because function doesn't exist yet
```

**GREEN (Implement minimal code to pass):**
```cpp
namespace xasm::encoding {
  constexpr std::array<uint8_t, 2> ToLittleEndian16(uint16_t value) {
    return {static_cast<uint8_t>(value & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF)};
  }
}
// Test passes
```

**REFACTOR (Improve code quality):**
```cpp
// Already clean, no refactoring needed
```

**3.3 Test Edge Cases:**
```cpp
TEST_CASE("ToLittleEndian16 edge cases") {
  REQUIRE(ToLittleEndian16(0x0000) == std::array{0x00, 0x00});
  REQUIRE(ToLittleEndian16(0xFFFF) == std::array{0xFF, 0xFF});
  REQUIRE(ToLittleEndian16(0x0001) == std::array{0x01, 0x00});
  REQUIRE(ToLittleEndian16(0xFF00) == std::array{0x00, 0xFF});
}
```

**3.4 Repeat for All Utilities**

**Expected Outcome:**
- `encoding_utils.h` with all utilities
- `test_encoding_utils.cpp` with 100% coverage
- All tests passing

### Phase 4: Integration Refactoring (2-3 hours)

**Goal:** Replace duplicated code with utilities in all CPU implementations

**Strategy:** One CPU at a time, test after each

**4.1 Refactor Cpu6502 (Iteration 1)**

1. Add include:
   ```cpp
   #include "xasm++/cpu/encoding_utils.h"
   ```

2. Replace ToLittleEndian calls:
   ```cpp
   // Before:
   auto bytes = ToLittleEndian(address);
   result.insert(result.end(), bytes.begin(), bytes.end());

   // After:
   auto bytes = xasm::encoding::ToLittleEndian16(address);
   result.insert(result.end(), bytes.begin(), bytes.end());
   ```

3. Remove member function ToLittleEndian from header and implementation

4. Test:
   ```bash
   cmake --build build
   cd build && ctest -R test_cpu6502 --output-on-failure
   ```

5. Commit:
   ```bash
   git add -A
   git commit -m "refactor: use encoding utils in Cpu6502"
   ```

**4.2 Refactor CpuZ80 (Iteration 2)**

Repeat same pattern as 4.1

**4.3 Refactor Cpu6809 (Iteration 3)**

Repeat same pattern as 4.1 (but use ToBigEndian16)

**Expected Outcome:** All CPU implementations using shared utilities, all tests passing

### Phase 5: Verification (1 hour)

**Goal:** Confirm refactoring success

**Steps:**
1. Full clean build:
   ```bash
   rm -rf build
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

2. Run all tests:
   ```bash
   cd build && ctest --output-on-failure
   ```

3. Check for duplication:
   ```bash
   # Should find ZERO instances in CPU implementations
   grep -r "& 0xFF.*>> 8" src/cpu/
   grep -r ">> 8.*& 0xFF" src/cpu/
   ```

4. Verify no warnings:
   ```bash
   cmake --build build 2>&1 | grep -i warning
   # Should be empty
   ```

5. Document metrics:
   - LOC removed
   - Duplication percentage before/after
   - Test coverage

**Expected Outcome:** All verification checks passing, metrics documented

## Execution Strategy

**Sequential within phases, but phases can overlap:**
- Phase 1-2: Can run concurrently (analysis + design)
- Phase 3: Must complete before Phase 4
- Phase 4: Must be sequential (one CPU at a time)
- Phase 5: Must be last

## TDD Approach

**Strict RED-GREEN-REFACTOR for Phase 3:**
1. ❌ RED: Write test, verify it fails
2. ✅ GREEN: Write minimal code to pass
3. ♻️ REFACTOR: Clean up code
4. Repeat for next utility

**For Phase 4 (Integration):**
- Tests already exist (CPU tests)
- Refactor implementation
- Verify tests still pass (behavior unchanged)

## Risk Mitigation

**Risk:** Breaking CPU encoding behavior
- **Mitigation:**
  - Comprehensive tests for utilities
  - Incremental integration (one CPU at a time)
  - Git commit after each successful CPU refactoring
  - Can rollback individual commits if needed

**Risk:** Performance regression
- **Mitigation:**
  - Use `constexpr` for compile-time evaluation
  - Use `inline` for vector helpers
  - Profile if concerns arise

**Risk:** API design doesn't fit all use cases
- **Mitigation:**
  - Start with most common patterns
  - Add utilities incrementally
  - Can extend API in future if needed

## Success Metrics

**Before:**
- Endianness logic: Duplicated 3 times
- Bit manipulation: Scattered across files
- Code duplication: ~15-20%

**After:**
- Endianness logic: Single implementation (encoding_utils.h)
- Bit manipulation: Centralized and tested
- Code duplication: <5%
- Test coverage: 100% for utilities
- All CPU tests: Passing

## Tools and Commands

```bash
# Pattern search
grep -r "pattern" src/cpu/

# Build and test
cmake --build build && cd build && ctest -R test_encoding

# Duplication check
grep -r "& 0xFF" src/cpu/

# Coverage (if available)
lcov --capture --directory . --output-file coverage.info
```

## Notes

- This refactoring sets precedent for future utility extraction
- Can be extended with additional utilities as patterns emerge
- Consider moving utilities to separate namespace for clarity
