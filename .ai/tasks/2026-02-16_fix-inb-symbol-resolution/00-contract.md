# Task Contract

**Task ID:** 2026-02-16_fix-inb-symbol-resolution
**Beads Task:** xasm++-ey96
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix the .INB directive so that symbols defined in included files are properly available in the main assembly.

### Problem

The .INB directive has been implemented to parse source files recursively, but symbols from included files aren't being resolved.

**Current behavior:**
- HandleInb() calls `parser->Parse(file_content, *context.section, *context.symbols)`
- Unit tests pass (because they check file structure, not symbol resolution)
- Real-world usage fails with "Undefined symbol" errors

**Minimal failing test case:**
```assembly
; test_include.i
TEST_VALUE .EQ $42

; test_main.asm
        .OR $0800
        .INB test_include.i
        .DA #TEST_VALUE    ; ERROR: Undefined symbol: TEST_VALUE
```

Running `xasm++ --cpu 6502 --syntax scmasm test_main.asm` fails with "Undefined symbol: TEST_VALUE"

### Root Cause Investigation Needed

The HandleInb implementation appears correct - it calls Parse() with the same symbol table. Need to debug:

1. Is HandleInb actually being called?
2. Is the included file being parsed?
3. Are symbols being added to the symbol table during Parse()?
4. Is there a scoping or visibility issue with the symbol table?
5. Is there a timing issue (symbols added after they're needed)?

---

## Success Criteria

```
✓ Minimal test case assembles successfully
✓ A2osX.S.txt assembles past line 77 (MLI.GETPREFIX symbol resolved)
✓ All 1679 existing tests still pass
✓ Symbols from included files available in main assembly
```

---

## Acceptance Criteria

### Functional Requirements
```
□ .INB directive includes and parses source files
□ Symbols defined via .EQ in included files are available
□ Forward references across includes work
□ Nested includes work correctly
□ A2osX.S.txt assembles successfully
```

### Quality Requirements
```
□ All 1679 tests passing
□ Minimal test case passes
□ A2osX test passes
□ No regressions in existing functionality
```

---

## Investigation Approach

1. **Add debug logging** to trace execution:
   - Log when HandleInb is called
   - Log when Parse() is called recursively
   - Log when symbols are added via HandleEq

2. **Check symbol table state**:
   - Verify symbols ARE being added during Parse()
   - Check if symbols disappear after Parse() returns
   - Verify symbol table pointer is correct

3. **Test incrementally**:
   - Start with minimal test case
   - Add debug output step by step
   - Verify each stage works before moving on

4. **Once root cause found**:
   - Implement fix
   - Remove debug output
   - Verify all tests pass

---

## Resources

**Test files to create:**
- `/tmp/test_include.i` - Simple .EQ directive
- `/tmp/test_main.asm` - Include and use symbol

**Real-world test:**
- `~/Projects/Vintage/Apple/A2osX/A2osX.S.txt` - Uses .INB extensively

**Key files:**
- `src/syntax/scmasm_directive_handlers.cpp` - HandleInb implementation (line 516)
- `src/syntax/scmasm_syntax.cpp` - Parse() method (line 252)
- `tests/unit/test_scmasm_syntax.cpp` - Unit tests

---

## Constraints

- Must not break existing 1679 tests
- Must maintain HandleInb's recursive parsing approach
- Must not modify symbol table API

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
