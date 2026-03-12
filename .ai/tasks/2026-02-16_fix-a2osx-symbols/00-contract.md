# Task Contract

**Task ID:** 2026-02-16_fix-a2osx-symbols
**Beads Task:** xasm++-18us
**Created:** 2026-02-16
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix symbol resolution issue preventing A2osX.S.txt from assembling.

### Current State

A2osX.S.txt fails at line 78 with:
```
Undefined symbol: MLIGETPREFIX01
```

**Progress:** Lines 1-77 assemble successfully (all directives working)

### Investigation Areas

1. **Macro Expansion:** MLICALL macro should generate MLIGETPREFIX01
2. **Include Files:** Check if mli.i or macros.i properly included
3. **Symbol Definition:** Verify symbol created at proper time
4. **.INB Processing:** Ensure recursive parsing works correctly

### Success Criteria

```
✓ A2osX.S.txt assembles 100% (no errors)
✓ All symbols resolve correctly
✓ Output binary generated
✓ No regressions in test suite
✓ Zero compiler warnings
```

---

## Resources

- **Test File:** ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt
- **Include Files:** ~/Projects/Vintage/Apple/A2osX/inc/
- **Binary:** /Users/bryanw/Projects/Vintage/tools/xasm++/build/bin/xasm++

**Estimated Effort:** 2-4 hours (investigation + fix)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
