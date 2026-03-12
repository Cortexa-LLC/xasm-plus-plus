# Implementation Plan: Consolidate ParseHex

**Workflow:** `.ai-pack/workflows/refactor.md`
**Role:** Engineer
**Execution:** Sequential (shared utility module)

## Phase 0: Analysis

### Step 0.1: Inventory Current Implementations

**Done above in contract.**

### Step 0.2: Identify All Call Sites

```bash
# Find all ParseHex callers
grep -r "ParseHex" src/ --include="*.cpp" -n

# Expected call sites:
# - assembler.cpp: 5+ calls (via using xasm::util::ParseHex)
# - simple_syntax.cpp: 3 calls (local static)
# - parse_utils.cpp: 0 direct calls (it's the implementation)
```

### Step 0.3: Test Coverage Baseline

```bash
# Run tests to establish baseline
ctest -R unit
python3 tests/e2e/apple2/prince_of_persia/test_pop_complete.py

# Expected: All pass (100% PoP compatibility)
```

## Phase 1: Design Consolidated API

### Target API (in `parse_utils.h`/`parse_utils.cpp`)

```cpp
namespace xasm {

// Exception-throwing variant (replaces string_utils::ParseHex)
// - Optional '$' prefix (strips if present)
// - Strips addressing mode suffixes (,X ,Y ,S)
// - Throws std::invalid_argument on error
uint32_t ParseHex(const std::string& str);

// Safe variant (already exists, enhance if needed)
// - Requires '$' prefix
// - No exceptions, returns success flag + error message
// - Validates length (max 8 hex digits)
uint32_t ParseHexSafe(const std::string& str, bool& success, std::string& error_msg);

} // namespace xasm
```

### Design Decisions

1. **Keep both variants:** Some callers need exceptions, others need error codes
2. **Consolidate shared logic:** Both variants call common validation helper
3. **Maintain behavior:** Match existing semantics exactly (no surprises)

## Phase 2: Implement Consolidated Utility

### Step 2.1: Enhance `parse_utils.cpp`

**TDD Approach:**

1. **RED:** Write test for `ParseHex()` exception-throwing variant
   ```cpp
   TEST(ParseUtilsTest, ParseHexWithOptionalPrefix) {
       EXPECT_EQ(0x1234, xasm::ParseHex("$1234"));
       EXPECT_EQ(0x1234, xasm::ParseHex("1234"));  // No prefix
       EXPECT_EQ(0x10, xasm::ParseHex("$10,X"));   // Strip suffix
       EXPECT_THROW(xasm::ParseHex(""), std::invalid_argument);
       EXPECT_THROW(xasm::ParseHex("ZZZ"), std::invalid_argument);
   }
   ```

2. **GREEN:** Implement `ParseHex()` in `parse_utils.cpp`
   ```cpp
   uint32_t ParseHex(const std::string& str) {
       // Validation + parsing logic from string_utils.cpp
       // Reuse validation logic from ParseHexSafe where possible
   }
   ```

3. **REFACTOR:** Extract common validation helper
   ```cpp
   namespace {
       bool ValidateHexString(const std::string& hex_part, std::string& error_msg) {
           // Shared validation logic
       }
   }
   ```

### Step 2.2: Update Header

Add declaration to `include/xasm++/parse_utils.h`:
```cpp
namespace xasm {
    uint32_t ParseHex(const std::string& str);
    // ParseHexSafe already declared
}
```

### Step 2.3: Run Tests

```bash
# Verify new implementation works
ctest -R parse_utils
```

## Phase 3: Migrate Call Sites

**Sequential migration to verify each step:**

### Step 3.1: Migrate `assembler.cpp`

1. Change using declaration:
   ```cpp
   // OLD:
   using xasm::util::ParseHex;

   // NEW:
   using xasm::ParseHex;
   ```

2. Run tests:
   ```bash
   ctest -R assembler
   ```

### Step 3.2: Migrate `simple_syntax.cpp`

1. Remove local static `ParseHex()` implementation (lines 12-34)

2. Add include and using:
   ```cpp
   #include "xasm++/parse_utils.h"
   using xasm::ParseHex;
   ```

3. Update call sites (3 locations: lines 126, 138, 153)

4. Run tests:
   ```bash
   ctest -R simple_syntax
   ```

### Step 3.3: Remove Old Implementations

1. **Remove from `string_utils.cpp`:**
   - Delete `ParseHex()` function (lines 25-55)

2. **Update `string_utils.h`:**
   - Remove `ParseHex()` declaration

3. **Verify no orphaned references:**
   ```bash
   grep -r "util::ParseHex\|string_utils::ParseHex" src/ include/
   # Should find nothing
   ```

4. Run full test suite:
   ```bash
   ctest
   python3 tests/e2e/apple2/prince_of_persia/test_pop_complete.py
   ```

## Phase 4: Verification

### Step 4.1: Regression Testing

```bash
# Unit tests
ctest

# Integration tests
python3 tests/e2e/apple2/prince_of_persia/test_pop_complete.py

# Expected: 100% pass rate maintained
```

### Step 4.2: Binary Size Check

```bash
# Before refactor
ls -lh build/src/xasm++ > /tmp/before.txt

# After refactor (rebuild clean)
rm -rf build && mkdir build && cd build && cmake .. && make
ls -lh src/xasm++ > /tmp/after.txt

# Compare (should be smaller or same)
diff /tmp/before.txt /tmp/after.txt
```

### Step 4.3: Code Search Verification

```bash
# Verify only ONE implementation exists
grep -r "^uint32_t ParseHex" src/ --include="*.cpp"
# Should find: src/core/parse_utils.cpp only

# Verify no orphaned static implementations
grep -r "^static uint32_t ParseHex" src/ --include="*.cpp"
# Should find: nothing
```

## Phase 5: Documentation

### Step 5.1: Update Work Log

Document:
- Migration completed
- All tests passing
- Binary size impact (if measurable)

### Step 5.2: Update Code Comments

Add documentation to `parse_utils.h`:
```cpp
/// Parse hexadecimal number from string
/// @param str Hex string (e.g., "$1234", "1234", "$10,X")
/// @return Parsed value as uint32_t
/// @throws std::invalid_argument on invalid format
uint32_t ParseHex(const std::string& str);
```

## Rollback Plan

If any step fails:

1. **Revert changes:**
   ```bash
   git checkout -- src/ include/
   ```

2. **Identify issue:**
   - Which test failed?
   - What error message?
   - Which call site?

3. **Fix forward:**
   - Adjust consolidated implementation to handle edge case
   - Update test
   - Re-run migration

## Timeline Estimate

*Note: No time estimates per project guidelines. Work is done when acceptance criteria are met.*

## Success Criteria

✅ All acceptance criteria from contract met
✅ All tests pass (unit + integration)
✅ Code duplication eliminated
✅ Single source of truth established
