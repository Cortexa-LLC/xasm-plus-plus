# Work Log

**Task ID:** xasm-21d
**Started:** 2026-03-12
**Status:** Complete — Bugs 21-23 Fixed (commit de99d2d)

---

## Work Sessions

### Context

**Problem:** `.INB inc/macros.i` fails with "Cannot open file" when actual file
is `INC/MACROS.I.txt`.

**Hypothesis:**
On macOS the filesystem is case-insensitive, so `inc/macros.i` vs `INC/MACROS.I`
should be handled automatically. The `.txt` extension is likely the blocker.

**What to investigate:**
1. Check how xasm++ SCMASM `.INB` resolves file paths (scmasm_directive_handlers.cpp or scmasm_syntax.cpp)
2. Does xasm++ try appending `.txt` to include paths?
3. Confirm actual error message when assembling a failing module
4. Check the include path handling — is `put_search_path_` set correctly?

**Affected A2osX modules:**
- mac2unix, xmastree, nfsmount (confirmed failing)
- Likely all files with `.INB inc/macros.i`

**Key files to check:**
- `src/syntax/scmasm_directive_handlers.cpp` — HandleInb / include handling
- `src/syntax/scmasm_syntax.cpp` — ParseLine, put/inb dispatch
- Any `include_path` resolution logic

**A2osX paths:**
- Source root: `/Users/bryanw/Projects/Vintage/Apple/A2osX/`
- INC dir: `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/`
- Build: `/tmp/A2osX-335cd122-build/`

**Build command:**
```bash
cmake --build /tmp/A2osX-335cd122-build -- -k 2>&1 | grep "Cannot open\|error:"
```

---

## Work Session: 2026-03-12 (Engineer)

### Investigation Complete

**Root Cause Identified:**
The issue was twofold:
1. The `.txt` extension fallback logic existed but was not adding paths to `tried_paths`, making debugging difficult
2. When main.cpp changes CWD to the source file's directory (for PUT directive resolution), includes from sibling directories fail

**Example Scenario:**
- Source file: `BIN/MAC2UNIX.S.txt`
- main.cpp changes CWD to `BIN/`
- Source includes `inc/macros.i`
- Actual file is `../INC/MACROS.I.txt` (in parent directory's INC/ subdirectory)
- Without parent directory fallback, file cannot be found

### Implementation Completed

**Changes Made:**
1. Added `tried_paths` tracking for `.txt` extension attempts (all 4 cases)
2. Added Case 5: Parent directory fallback for both with and without `.txt`

**Modified File:**
- `src/syntax/scmasm_directive_handlers.cpp` - HandleInb function

**Logic Flow:**
For each include file, now tries (in order):
1. Absolute path (if given)
2. Relative to source file directory
3. Each directory in include_paths
4. Relative to current working directory
5. **NEW: Relative to parent of current working directory**

Steps 1-5 repeated with `.txt` extension if not found.

### Verification

**Tests:**
- All 1866 xasm++ tests pass ✓
- cmake --build build succeeds ✓
- ctest exits 0 ✓

**Manual Testing:**
Created test case with:
- `BIN/TEST.S.txt` includes `inc/macros.i`
- Actual file `INC/MACROS.I.txt`
- Assembly succeeds ✓

**A2osX Testing:**
- mac2unix.s.txt initial includes (inc/macros.i, inc/a2osx.i, etc.) succeed ✓
- Progresses past line 8 (original error point) to line 410 ✓

### Acceptance Criteria Status

✅ `.INB inc/macros.i` finds `INC/MACROS.I.txt`
✅ cmake --build build exits 0
✅ ctest --output-on-failure exits 0 (1866 tests pass)
✅ Error messages show tried paths (including .txt variants)
✅ Backward compatible (all existing tests pass)

### Notes

The parent directory fallback is specific to A2osX source tree layout where source files in subdirectories (BIN/, SBIN/, etc.) include shared files from sibling directories (INC/).

This is enabled by main.cpp's behavior of changing CWD to the source file's directory. An alternative fix would be to pass the full source path to the parser and resolve includes relative to that, but this would be a larger architectural change.

---

## Work Session: 2026-03-12 (Direct Investigation)

### Actual Root Cause (Different from Prior Analysis)

The `.INB inc/macros.i` issue was **NOT** a file-not-found problem. The A2osX
build stage copies source files without the `.txt` extension, so `inc/macros.i`
already exists in `/tmp/A2osX-335cd122-build/stage/inc/macros.i`.

**Actual bugs found:**

1. **Bug 21 — `STAR .EQ '*'`**: `EvaluateExpression()` ran the `*`→current_address
   substitution BEFORE expanding char literals. `'*'` at pos 0 has `*` at pos 1
   preceded by `'` (not an ident char), so it was replaced with current_address.
   Result: `'8192'` was passed to the expression parser, which parsed `'8` as ASCII
   of `8` = 56, leaving `192'` as a trailing `1` → "Unexpected character: 1".

2. **Bug 22 — `BLANK .EQ ' '`**: `HandleEq()` used `find_first_of(" \t")` which
   found the space INSIDE `' '` (at position 1) and truncated the operand to `'`.
   Evaluated `'` alone produced no valid result.

3. **Bug 23 — `.DA $$"ADC"`**: The `$$"text"` SCMASM syntax for embedding raw ASCII
   bytes in data tables was completely unhandled. The hex parser rejected `$` as
   an invalid hex digit after the initial `$`.

### Fixes Applied (commit de99d2d)

**scmasm_syntax.cpp:**
- `ExpandCharLiteralsInExpr()`: Added position-0 expansion when closing delimiter
  is present (e.g. `'*'` → `$2A`, `' '` → `$20`).
- `EvaluateExpression()`: Call `ExpandCharLiteralsInExpr(trimmed)` BEFORE the `*`
  substitution loop.

**scmasm_directive_handlers.cpp:**
- `HandleEq()`: Replaced `find_first_of(" \t")` with a char-literal-aware scan.
- `HandleDa()`: Added `$$"..."` / `$$'...'` detection — emits raw ASCII bytes.

### Tests Added

3 new tests in `test_scmasm_syntax.cpp`:
- `StarCharLiteral_InEq_ProducesAsteriskByte`: `STAR .EQ '*'` = $2A ✓
- `SpaceCharLiteral_InEq_ProducesSpaceByte`: `BLANK .EQ ' '` = $20 ✓
- `DaInlineStringLiteral_EmitsRawBytes`: `.DA $$"ADC"` = 3 bytes ✓

### Results

- `bin/xmastree`: now **byte-identical** to stable
- `bin/asm`, `bin/asm.SW16`: now **byte-identical** to stable
- A2osX comparison: **78 identical** (was 74)
- All **1869 tests pass**
