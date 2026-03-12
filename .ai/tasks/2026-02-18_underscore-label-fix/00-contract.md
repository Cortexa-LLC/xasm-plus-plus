# Task Contract: Fix Underscore Label Parsing

**Task ID:** 2026-02-18_underscore-label-fix
**Beads Task:** xasm++-ahrf
**Created:** 2026-02-18
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Priority:** P1

---

## Problem

**366 errors** in A2osX assembly because labels starting with underscore (`_WRITE`, `_STRLEN`, etc.) are rejected by the parser.

**Root Cause:** `scmasm_syntax.cpp` line 642 - `ParseLabel()` doesn't allow underscore as a starting character.

---

## The Fix (Trivial - 1 Line)

**File:** `src/syntax/scmasm_syntax.cpp`
**Line:** 642
**Function:** `ParseLabel()`

**Change from:**
```cpp
if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '.' && line[pos] != ':')) {
    return "";
}
```

**Change to:**
```cpp
if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '_' && line[pos] != '.' && line[pos] != ':')) {
    return "";
}
```

**Add:** `line[pos] != '_' &&` (4 characters)

---

## Why This Works

- Expression parser already allows underscore (line 613 in expression_parser.cpp)
- This aligns label parser with identifier parser
- Allows `_WRITE .EQ $1C` to be parsed as: label=`_WRITE`, opcode=`.EQ`, operand=`$1C`

---

## Acceptance Criteria

- [ ] Line 642 updated to allow underscore
- [ ] All existing tests pass
- [ ] A2osX build: 366 → 0 errors
- [ ] Test: Symbol starting with underscore can be defined and used

---

## Test

```bash
# Build
cmake --build build

# Run tests
ctest --output-on-failure

# Verify A2osX
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make
# Expected: 0 errors (down from 366)
```

---

## References

- **Investigation:** `.ai/tasks/2026-02-18_underscore-symbols/30-investigation-report.md`
- **Spelunker Task:** xasm++-nbs7 (completed)
- **Expected Impact:** 366 errors → 0 errors (100% A2osX compatibility!)
