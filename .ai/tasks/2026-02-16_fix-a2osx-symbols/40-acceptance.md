# Acceptance: Fix A2osX Symbol Resolution

**Task ID:** 2026-02-16_fix-a2osx-symbols
**Beads Task:** xasm++-18us
**Status:** ✅ Complete

---

## Acceptance Criteria

✅ **Primary Goal:** Fix MLIGETPREFIX01 undefined symbol error
- **Result:** FIXED - The error was actually about the `*` (current address) operator, not the symbol itself
- **Verification:** A2osX.S.test.txt assembles past line 78 without "Undefined symbol" errors

✅ **Root Cause Identified:** 
- Issue: `*` operator (current address in SCMASM) was not recognized by ExpressionParser
- Only `$` was recognized as current address operator
- When parser encountered `bcs *`, it tried to parse `*` as multiply operator, which failed

✅ **Solution Implemented:**
- Modified `ScmasmSyntaxParser::EvaluateExpression()` to replace `*` with `$`
- Added logic to distinguish:
  - `*` at start or after operators → current address (replace with `$`)
  - `*` between operands → multiply operator (keep as-is)
- Handles expressions: `*`, `*+4`, `*-2`, etc.

✅ **Testing:**
- Test file created and successfully assembled
- Verified `*` works in multiple contexts:
  - Bare `*` (current address)
  - `*+4` (current address + offset)
  - `.DA *` (store current address)
  - `2*3` (multiply operation) - still works correctly

✅ **Code Quality:**
- Clean implementation in src/syntax/scmasm_syntax.cpp
- No build warnings
- No test regressions

---

## Remaining Work (Outside Scope)

The following errors remain but are DIFFERENT issues (pseudo-instructions/macros):
- 66 errors: `>LDYAI` unsupported instruction
- 12 errors: `>STYA` unsupported instruction
- 3 errors: `>LDYA` unsupported instruction
- 3 errors: `>DEBUGOA` unsupported instruction

These are macro/pseudo-instruction features that should be handled in a separate task.

---

## Files Modified

- `src/syntax/scmasm_syntax.cpp` - Added `*` to `$` replacement logic in `EvaluateExpression()`

---

## Verification Steps

```bash
cd ~/Projects/Vintage/Apple/A2osX
/Users/bryanw/Projects/Vintage/tools/xasm++/build/bin/xasm++ A2osX.S.test.txt --syntax scmasm --cpu 65c02 -o /tmp/a2osx.bin 2>&1 | grep -i "MLIGETPREFIX01"
# Should return no results (error is gone)
```

---

**Completed:** 2026-02-16
**Next Steps:** Address pseudo-instruction support in separate task
