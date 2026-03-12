# Task Contract

**Task ID:** 2026-02-16_forward-references
**Beads Task:** xasm++-kbgs
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement forward reference resolution for `.DA` directive in SCMASM syntax.

### Problem Statement

A2osX source code uses forward references - symbols are used before they're defined:

```assembly
; Line 76: Use symbol (not yet defined)
.DA MLIGETPREFIX01

; ... 553 lines later ...

; Line 629: Define symbol
MLIGETPREFIX01
    ; code here
```

**Current behavior:** `Parse error: Undefined symbol: MLIGETPREFIX01`

**Expected behavior:** Symbol resolved on later pass

### Current Architecture

xasm++ has multi-pass support:
- **Pass 1:** Parse and build symbol table
- **Pass 2:** Resolve forward references
- **Pass 3+:** Continue until all references resolved

**Problem:** The `.DA` directive calls `EvaluateExpression()` immediately during parsing, throwing error on undefined symbols before later passes can resolve them.

### Solution Approach

**Option A: Defer .DA evaluation (RECOMMENDED)**

Modify `.DA` directive to:
1. Store expression as string during first pass
2. Evaluate on later passes after symbols defined
3. Emit bytes only when expression fully resolved

**Option B: Mark forward references**

When symbol undefined:
1. Mark as "forward reference pending"
2. Allow parsing to continue
3. Revisit on next pass

**Option C: Require forward declarations**

Document as limitation - users must declare labels before use. (NOT RECOMMENDED - breaks A2osX compatibility)

### Tasks

1. **Investigate current multi-pass implementation:**
   - How are passes triggered?
   - When is symbol table built?
   - How are forward references detected?

2. **Modify .DA directive handler:**
   - Store expression string on first pass
   - Check if all symbols defined before evaluating
   - Emit placeholder bytes if needed
   - Resolve on subsequent passes

3. **Test forward references:**
   - Simple forward reference
   - Forward reference in .DA directive
   - Multiple forward references
   - Circular references (should error)

4. **Validate A2osX:**
   - Line 76 should assemble successfully
   - All forward references resolved
   - Build complete A2osX.S.txt

### Success Criteria

```
✓ Forward references in .DA directives resolve correctly
✓ A2osX line 76+ assembles without errors
✓ Test case for forward references passes
✓ All existing tests still pass (1,738/1,738)
✓ No circular reference infinite loops
✓ Zero warnings
```

**Estimated Effort:** 3-4 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
