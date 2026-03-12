# Work Log: Fix A2osX Symbol Resolution

**Task ID:** 2026-02-16_fix-a2osx-symbols
**Beads Task:** xasm++-18us [CLOSED]
**Status:** ✅ Complete

---

## Session 2026-02-16

### Investigation Phase

**Step 1: Verify the problem**
- Testing A2osX.S.txt assembly
- Found: MLIGETPREFIX01 used at line 76, defined at line 629
- This is a **forward reference** problem
- Also discovered: .LIST directive not recognized

**Step 2: Understanding the issue**
- Symbol MLIGETPREFIX01 is a label for MLI parameter block
- Used before definition (forward reference)
- Assembler needs multi-pass or forward reference resolution

**Step 3: Current assembler behavior**
- xasm++ exists at build/bin/xasm++
- Fails on .LIST OFF directive (line 3)
- Directive IS registered: `LIST = ".LIST"` in scmasm_directive_constants.h
- Handler exists: HandleList() in scmasm_directive_handlers.cpp
- Need to run actual test to see current error

**Step 4: Running test**
- Test output shows .LIST directive is now working
- Multiple directive outputs present (*.EP, *.PH, etc.)
- But still failing with "Undefined symbol: MLIGETPREFIX01"

**Step 5: Deep dive into expression parsing**
- MLIGETPREFIX01 error was at line 78: `bcs *`
- The issue is NOT about MLIGETPREFIX01 symbol
- The issue is about `*` (current address operator)
- Line 77 is `bcs *` which should branch to current address
- Error message was misleading - said "Undefined symbol" but really expression parsing failed

**Step 6: Found the root cause**
- `ExpressionParser::Parse()` in expression_parser.cpp:
  - After parsing expression, checks for trailing characters
  - Line 32-34: Throws "Unexpected character after expression"
- When parsing `*`:
  - `*` is recognized as MULTIPLY operator in `ParseMulDiv()`
  - But `*` by itself is NOT a valid operand for multiply
  - Expression parser tries to parse it as symbol but fails
- Only `$` is recognized as current address in ExpressionParser
- SCMASM uses `*` for current address, not `$`

**Step 7: The fix**
- Modified `ScmasmSyntaxParser::EvaluateExpression()` to replace `*` with `$`
- Added logic to distinguish:
  - `*` at start of expression → current address (replace with `$`)
  - `*` after operator (+, -, (, etc.) → current address (replace with `$`)
  - `*` between operands → multiply operator (leave as-is)
- This handles expressions like `*`, `*+4`, `*-2`, etc.

**Step 8: Testing the fix**
```bash
cd ~/Projects/Vintage/Apple/A2osX
/Users/bryanw/Projects/Vintage/tools/xasm++/build/bin/xasm++ A2osX.S.test.txt --syntax scmasm --cpu 65c02 -o /tmp/a2osx.bin
```

**Results:**
✅ MLIGETPREFIX01 error is GONE!
✅ `bcs *` now parses correctly
✅ Forward references work as expected

**Remaining errors:**
- 66 errors: `>LDYAI` unsupported instruction
- 12 errors: `>STYA` unsupported instruction
- 3 errors: `>LDYA` unsupported instruction
- 3 errors: `>DEBUGOA` unsupported instruction

These are pseudo-instructions (macros), which is a DIFFERENT task.

---

## Completion Summary

✅ **Task Complete**

**What was fixed:**
- The "Undefined symbol: MLIGETPREFIX01" error at line 78
- Root cause: `*` (current address) operator not recognized
- Solution: Replace `*` with `$` in expression evaluation

**Implementation:**
- Modified `ScmasmSyntaxParser::EvaluateExpression()` in src/syntax/scmasm_syntax.cpp
- Added logic to distinguish current address `*` from multiply operator `*`
- Handles: `*`, `*+4`, `*-2`, etc.

**Verification:**
- Build: 0 warnings ✅
- Tests: All existing tests pass ✅
- Target file: A2osX.S.test.txt now assembles past line 78 ✅
- Beads task: xasm++-18us closed ✅

**Next Steps:**
- Pseudo-instruction support (>LDYAI, >STYA, etc.) should be separate task
- Consider adding more comprehensive tests for `*` operator edge cases
