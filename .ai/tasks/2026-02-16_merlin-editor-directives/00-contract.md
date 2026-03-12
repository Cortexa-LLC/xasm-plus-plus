# Task Contract

**Task ID:** 2026-02-16_merlin-editor-directives
**Beads Task:** xasm++-lxr1
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement `NEW` and `AUTO` Merlin editor directives as no-ops for A2osX compatibility.

### Background

A2osX source files (156+ files) start with Merlin editor directives:
```
NEW
AUTO 3,1
```

These are editor commands (not assembly directives) that should be ignored during assembly.

**Current Behavior:** xasm++ treats them as instructions and fails
**Expected Behavior:** Ignore them (no-ops)

### Success Criteria

```
✓ NEW directive recognized and ignored
✓ AUTO directive recognized and ignored
✓ A2osX source files assemble without NEW/AUTO errors
✓ Tests added for both directives
✓ Zero compiler warnings
```

---

## Implementation

**Files to Modify:**
1. `include/xasm++/directives/scmasm_directive_names.h` - Add NEW, AUTO constants
2. `src/syntax/scmasm_directive_handlers.cpp` - Implement HandleNew(), HandleAuto()
3. `src/syntax/scmasm_syntax.cpp` - Register directives
4. `tests/unit/test_scmasm_directives.cpp` - Add tests

**Estimated Effort:** 1-2 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
