# Task Contract

**Task ID:** 2026-01-24_merlin-directives-4-6
**Created:** 2026-01-24
**Requestor:** Bryan Woodruff
**Assigned Role:** Orchestrator → Engineer
**Workflow:** Feature
**Beads Task:** xasm++-m94

---

## Task Description

Implement Merlin Phases 4-6: File Inclusion (PUT), Data Directives (DFB, etc.), and Conditional Assembly (DO/FIN).

### Background and Context

**Why:** This is Phase 4-6 of the Merlin Syntax Support feature. These directives are essential for assembling real Prince of Persia code:
- PUT directive: Used 63 times in PoP for file inclusion
- DFB data directive: Used 44 times for special data formats
- DO/FIN conditionals: Used 49 times for compile-time configuration

**Problem:** After Phases 1-3 provide the foundation, we need these features to handle PoP's multi-file structure and conditional compilation.

**Context:** Builds on Phases 1-3 (Foundation, Local Labels, DUM Blocks) which must be complete first.

### Current State

**What Exists (after Phase 1-3):**
- ✅ MerlinSyntax class with basic parsing
- ✅ Comment parsing (* and ;)
- ✅ Label system (global, :local, ]variable)
- ✅ Core directives (ORG, EQU, DB, DW, HEX, DS, DUM, DEND)

**What's Missing:**
- ❌ PUT directive (file inclusion)
- ❌ DFB directive (signed/font bytes)
- ❌ DO/ELSE/FIN conditional assembly
- ❌ Nested conditional handling
- ❌ Include path resolution

### Desired State

**After Completion:**
- ✅ PUT directive working with relative paths
- ✅ Circular include detection
- ✅ DFB and other data directive variants
- ✅ DO/ELSE/FIN conditional blocks
- ✅ Nested conditionals (up to 8 levels deep)
- ✅ Can assemble multi-file PoP projects
- ✅ Comprehensive test coverage

---

## Success Criteria

```
✓ Can parse and process PUT directive
✓ Can detect circular includes
✓ Can handle nested includes (at least 3 deep)
✓ DFB directive works with multiple values
✓ DO/FIN conditionals evaluate correctly
✓ ELSE clause supported
✓ Nested conditionals work (at least 3 deep)
✓ All tests passing (100%)
✓ Code coverage ≥ 85% overall, 95%+ for new code
```

---

## Dependencies

**BLOCKING:** Phases 1-3 must be complete before starting this task.

**Task ID:** xasm++-vp5 (Merlin Phases 1-3)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-24
