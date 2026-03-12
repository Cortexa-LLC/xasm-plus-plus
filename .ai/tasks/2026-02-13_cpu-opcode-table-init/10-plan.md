# Plan: Fix missing-field-initializers in cpu_6502.cpp

**Task:** xasm++-8l2w (generated)
**Workflow:** Bugfix
**Role:** Engineer
**Approach:** Search and replace with verification

---

## Execution Strategy

**Sequential implementation:**
1. Find all partial OpcodeTable initializations
2. Fix each one using complete 19-field pattern
3. Verify build passes on all platforms
4. Run all tests

---

## Implementation Steps

### Step 1: Find All Partial OpcodeTable Initializations

**Search pattern:**
```bash
grep -n "OpcodeTable.*=" src/cpu/cpu_6502.cpp
```

Expected locations:
- Line 252-256: JMP_TABLE (known from error)
- Line 416: (mentioned in CI log)
- Line 429: (mentioned in CI log)
- Additional partial initializations

**Identification criteria:**
- Count fields in initialization
- If < 19 fields → NEEDS FIX
- If = 19 fields → ALREADY CORRECT

### Step 2: Create Field Template

**All 19 fields** (from include/xasm++/cpu/cpu_6502.h):
```cpp
{
    .immediate = std::nullopt,
    .zero_page = std::nullopt,
    .zero_page_x = std::nullopt,
    .zero_page_y = std::nullopt,
    .absolute = std::nullopt,
    .absolute_x = std::nullopt,
    .absolute_y = std::nullopt,
    .indirect = std::nullopt,
    .indirect_x = std::nullopt,
    .indirect_y = std::nullopt,
    .accumulator = std::nullopt,
    .relative = std::nullopt,
    .indirect_zero_page = std::nullopt,
    .absolute_indexed_indirect = std::nullopt,
    .absolute_long = std::nullopt,
    .indirect_long = std::nullopt,
    .indirect_long_indexed_y = std::nullopt,
    .stack_relative = std::nullopt,
    .stack_relative_indirect_indexed_y = std::nullopt
};
```

### Step 3: Fix Each Partial Initialization

**For each partial OpcodeTable found:**

1. **Copy template** (all 19 fields with std::nullopt)
2. **Fill in actual opcodes** from original partial initialization
3. **Replace** partial initialization with complete one
4. **Verify field order** matches template (alphabetical)

**Example transformation:**

**BEFORE (line 252-256):**
```cpp
static const OpcodeTable JMP_TABLE = {
    .absolute = Opcodes::JMP_ABS,
    .indirect = Opcodes::JMP_IND,
    .absolute_indexed_indirect = Opcodes::JMP_AIX // 65C02+
};
```

**AFTER:**
```cpp
static const OpcodeTable JMP_TABLE = {
    .immediate = std::nullopt,
    .zero_page = std::nullopt,
    .zero_page_x = std::nullopt,
    .zero_page_y = std::nullopt,
    .absolute = Opcodes::JMP_ABS,
    .absolute_x = std::nullopt,
    .absolute_y = std::nullopt,
    .indirect = Opcodes::JMP_IND,
    .indirect_x = std::nullopt,
    .indirect_y = std::nullopt,
    .accumulator = std::nullopt,
    .relative = std::nullopt,
    .indirect_zero_page = std::nullopt,
    .absolute_indexed_indirect = Opcodes::JMP_AIX, // 65C02+
    .absolute_long = std::nullopt,
    .indirect_long = std::nullopt,
    .indirect_long_indexed_y = std::nullopt,
    .stack_relative = std::nullopt,
    .stack_relative_indirect_indexed_y = std::nullopt
};
```

### Step 4: Reference Existing Complete Initializations

**Good examples** (already complete, use as reference):
- Lines 271-290: ADC_TABLE (19 fields)
- Lines 221-244: STA_TABLE (19 fields)

**Pattern to follow:**
```cpp
static const OpcodeTable FOO_TABLE = {
    .immediate = Opcodes::FOO_IMM,          // or std::nullopt
    .zero_page = Opcodes::FOO_ZP,           // or std::nullopt
    .zero_page_x = Opcodes::FOO_ZPX,        // or std::nullopt
    .zero_page_y = std::nullopt,            // rarely used
    .absolute = Opcodes::FOO_ABS,           // or std::nullopt
    .absolute_x = Opcodes::FOO_ABX,         // or std::nullopt
    .absolute_y = Opcodes::FOO_ABY,         // or std::nullopt
    .indirect = std::nullopt,               // rarely used
    .indirect_x = Opcodes::FOO_INX,         // or std::nullopt
    .indirect_y = Opcodes::FOO_INY,         // or std::nullopt
    .accumulator = std::nullopt,            // rarely used
    .relative = std::nullopt,               // branch instructions only
    .indirect_zero_page = std::nullopt,     // 65C02+ only
    .absolute_indexed_indirect = std::nullopt, // 65C02+ only
    .absolute_long = std::nullopt,          // 65816 only
    .indirect_long = std::nullopt,          // 65816 only
    .indirect_long_indexed_y = std::nullopt,// 65816 only
    .stack_relative = std::nullopt,         // 65816 only
    .stack_relative_indirect_indexed_y = std::nullopt // 65816 only
};
```

### Step 5: Build and Verify

**Local build:**
```bash
cmake --build build
```

**Expected:** Zero warnings, clean build

**Run tests:**
```bash
cd build && ctest --output-on-failure
```

**Expected:** 1564/1564 tests passing

### Step 6: Verify Clang-Format

**Apply formatting:**
```bash
find src include -name "cpu_6502.*" | xargs clang-format -i
```

**Check diff:**
```bash
git diff src/cpu/cpu_6502.cpp
```

**Expected:** Minimal diff (only missing fields added)

### Step 7: Commit and Push

**Commit message:**
```
fix: Complete OpcodeTable initializations for GCC/MSVC compatibility

- Add explicit std::nullopt for all 19 fields in partial OpcodeTable structs
- Fixes missing-field-initializers errors on Ubuntu/Windows builds
- No behavior changes, pure refactoring for cross-platform compatibility

Fixes CI run: https://github.com/Cortexa-LLC/xasm-plus-plus/actions/runs/21994137589

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

**Commands:**
```bash
git add src/cpu/cpu_6502.cpp
git commit -m "$(cat <<'EOF'
fix: Complete OpcodeTable initializations for GCC/MSVC compatibility

- Add explicit std::nullopt for all 19 fields in partial OpcodeTable structs
- Fixes missing-field-initializers errors on Ubuntu/Windows builds
- No behavior changes, pure refactoring for cross-platform compatibility

Fixes CI run: https://github.com/Cortexa-LLC/xasm-plus-plus/actions/runs/21994137589

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
EOF
)"
git push origin main
```

### Step 8: Verify CI Passes

**Monitor CI run:**
```bash
# Wait for GitHub Actions to complete
# Check: https://github.com/Cortexa-LLC/xasm-plus-plus/actions
```

**Expected results:**
- ✅ Format Check passed
- ✅ Ubuntu build passed (GCC)
- ✅ Windows build passed (MSVC)
- ✅ macOS build passed (Clang)
- ✅ All tests passing

---

## Expected Partial Initializations

**Based on CI error log:**

1. **Line 252-256: JMP_TABLE** (3 fields → needs 16 more)
2. **Line 416: Unknown** (check during Step 1)
3. **Line 429: Unknown** (check during Step 1)

**Search entire file:**
```bash
grep -B2 -A10 "static const OpcodeTable" src/cpu/cpu_6502.cpp | grep -E "^\s+\." | wc -l
```

Compare field counts for each table.

---

## Verification Checklist

- [ ] All OpcodeTable initializations found
- [ ] Each has exactly 19 fields
- [ ] Field order matches template (alphabetical)
- [ ] Existing opcode values preserved
- [ ] Missing fields set to std::nullopt
- [ ] Local build passes (macOS)
- [ ] All 1564 tests passing
- [ ] Clang-format applied
- [ ] Changes committed and pushed
- [ ] CI builds passing (all platforms)

---

## Success Criteria

- All OpcodeTable structs have 19 fields
- Ubuntu/Windows CI builds passing
- Zero missing-field-initializers errors
- 1564/1564 tests passing
- Zero new warnings

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Engineer
**Estimated effort:** 30-60 minutes (straightforward search and replace)
