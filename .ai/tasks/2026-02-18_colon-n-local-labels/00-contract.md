# Task Contract: Support :N Local Labels in SCMASM

**Task ID:** 2026-02-18_colon-n-local-labels
**Beads Task:** xasm++-x997
**Created:** 2026-02-18
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Priority:** P1

---

## Problem

A2osX assembly fails with 16 errors for `:1` local labels. SCMASM historical syntax uses `:N` (`:1`, `:2`, etc.) for local labels, but xasm++ only recognizes `.N` syntax (`.1`, `.2`, etc.).

**Error Message:**
```
error: Invalid argument for :1: Unsupported instruction: :1
```

**Impact:** 16 errors in A2osX build (3% of remaining 504 errors)

---

## Root Cause

From Spelunker investigation (xasm++-yywb):

**Current Implementation** (`scmasm_syntax.cpp` lines 844-850):
```cpp
bool ScmasmSyntaxParser::IsLocalLabel(const std::string &label) {
  // Local labels are .0-.9
  if (label.length() == 2 && label[0] == '.' && label[1] >= '0' &&
      label[1] <= '9') {
    return true;
  }
  return false;
}
```

**Example from A2osX** (`inc/MACROS.I.txt`):
```scmasm
.MA	DEBUG
:1		bit $C000
		bpl :1        ; Branch to :1 label
		sta	$C010
.EM
```

**Issue:** `:N` syntax not recognized as local labels.

---

## Solution

Extend SCMASM parser to support both `.N` and `:N` local label syntaxes.

**Changes Required:**

1. **Update `IsLocalLabel()`** (`scmasm_syntax.cpp` ~line 844):
   - Accept `:0`-`:9` in addition to `.0`-`.9`
   - Both are valid SCMASM local label syntax

2. **Update `ParseLabel()`** (`scmasm_syntax.cpp` ~line 622):
   - Recognize `:` as valid label start character (currently only `.` and letters)
   - Line 889-890 checks: `(!std::isalpha(line[pos]) && line[pos] != '.')`
   - Add: `&& line[pos] != ':'`

---

## Acceptance Criteria

- [ ] `IsLocalLabel()` returns true for `:0` through `:9`
- [ ] `IsLocalLabel()` still returns true for `.0` through `.9` (backward compatibility)
- [ ] `ParseLabel()` recognizes `:` as valid label start
- [ ] Unit tests pass for both `:N` and `.N` syntax
- [ ] A2osX build reduces from 504 to 488 errors (16 errors fixed)
- [ ] All existing tests continue to pass

---

## Test Cases

**Unit Tests to Add** (`tests/unit/test_scmasm_syntax.cpp`):

```cpp
TEST_F(ScmasmSyntaxTest, ColonLocalLabels) {
  // :1 local label definition
  Parse(":1 LDA #$00");
  EXPECT_EQ(GetSymbol(":1"), current_address);

  // :1 local label reference
  Parse("BNE :1");
  // Verify branch target resolves to :1 label address
}

TEST_F(ScmasmSyntaxTest, ColonLocalLabelsInMacros) {
  Parse(".MA DEBUG");
  Parse(":1 BIT $C000");
  Parse("BPL :1");
  Parse(".EM");

  // Invoke macro
  Parse(">DEBUG");
  // Verify :1 label is macro-scoped
}

TEST_F(ScmasmSyntaxTest, BothLocalLabelSyntaxes) {
  Parse(".1 LDA #$00");  // Dot syntax
  Parse(":1 STA $00");   // Colon syntax

  // Both should be recognized as local labels
  EXPECT_TRUE(IsLocalLabel(".1"));
  EXPECT_TRUE(IsLocalLabel(":1"));
}
```

**Integration Test:**
```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make 2>&1 | grep -c "error:.*:1"
# Expected: 0 (down from 16)
```

---

## Files to Modify

1. `src/syntax/scmasm_syntax.cpp`
   - `IsLocalLabel()` function (~line 844)
   - `ParseLabel()` function (~line 622, specifically line 889-890)

2. `include/xasm++/syntax/scmasm_syntax.h`
   - No changes expected (function signatures remain the same)

3. `tests/unit/test_scmasm_syntax.cpp`
   - Add test cases for `:N` local label syntax

---

## Dependencies

**None** - Independent task

**Blocks:**
- None

**Blocked By:**
- None

---

## Complexity Estimate

**Size:** Small
- 2 functions to modify
- ~10 lines of code changes
- ~30 lines of test code

**Time Estimate:** 30-60 minutes
- Implementation: 15 minutes
- Testing: 15 minutes
- Integration verification: 15 minutes

---

## Success Metrics

**Error Reduction:**
- Before: 504 errors
- After: 488 errors
- Reduction: 16 errors (3%)

**Test Coverage:**
- Unit tests for `:N` syntax
- Integration test with A2osX

**Quality:**
- All existing tests pass
- No regressions in `.N` syntax support

---

## Notes

- Historical SCMASM assemblers supported both `.N` and `:N` syntax
- `:N` syntax is macro-scoped (different from global `.N` labels)
- A2osX uses `:N` extensively in `inc/MACROS.I.txt`
- This is the smaller of two P1 fixes (16 vs 488 errors)

---

## References

- Investigation: `.ai/tasks/2026-02-18_a2osx-remaining-errors/20-work-log.md`
- Source file: `/tmp/a2osx-build/inc/MACROS.I.txt`
- Test command: `cd ~/Projects/Vintage/Apple/A2osX && XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make`
