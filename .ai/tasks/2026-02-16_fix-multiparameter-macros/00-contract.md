# Task Contract

**Task ID:** 2026-02-16_fix-multiparameter-macros
**Beads Task:** xasm++-t77s
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix multi-parameter macro substitution in SCMASM syntax.

### Current Issue

Macros with multiple parameters are not substituting correctly. Only the first parameter (]1) is being substituted, while subsequent parameters (]2, ]3, etc.) are not expanded.

**Example:**
```
Macro definition (STYA):
.MA STYA
    sta ]1
    sty ]2
.EM

Invocation:
>STYA TmpPtr1,X

Expected expansion:
    sta TmpPtr1
    sty X

Actual expansion:
    sta TmpPtr1
    sty ]2    <-- ]2 not substituted!
```

**Debug Output:**
```
DEBUG InvokeMacro: macro STYA with 1 params
  Original line: [sta ]1,]2]
  Expanded line: [sta TmpPtr1,]
```

The debug shows only 1 parameter detected when 2 were passed.

### Root Cause

The parameter parsing logic is not correctly:
1. Splitting comma-separated parameters from macro invocation
2. Substituting all ]N placeholders in macro body

### Impact

**Blocks A2osX assembly at line 76:**
```
Error: Undefined symbol: MLIGETPREFIX01
Line 76: >LDYAI MLIGETPREFIX01
```

The LDYAI macro uses multiple parameters that aren't expanding correctly, causing undefined symbol errors.

### Tasks

1. **Debug parameter parsing:**
   - Examine how parameters are parsed from ">STYA TmpPtr1,X"
   - Verify parameter count detection (should be 2, not 1)
   - Check parameter splitting on commas

2. **Fix ]N substitution:**
   - Ensure ALL ]N placeholders (]1-]9) are replaced
   - Not just ]1

3. **Add test case:**
   - Multi-parameter macro test
   - Verify >STYA TmpPtr1,X expands correctly

4. **Validate A2osX:**
   - Test that line 76+ now assembles
   - Verify LDYAI and STYA macros work

### Success Criteria

```
✓ Multi-parameter macros expand correctly
✓ Debug shows correct parameter count
✓ Test for 2-parameter macro passes
✓ A2osX line 76+ assembles without errors
✓ All existing tests still pass (1,738/1,738)
✓ Zero warnings
```

**Estimated Effort:** 1-2 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
