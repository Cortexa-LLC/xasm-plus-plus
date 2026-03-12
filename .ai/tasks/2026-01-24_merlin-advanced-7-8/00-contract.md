# Task Contract

**Task ID:** 2026-01-24_merlin-advanced-7-8
**Created:** 2026-01-24
**Requestor:** Bryan Woodruff
**Assigned Role:** Orchestrator → Engineer
**Workflow:** Feature
**Beads Task:** xasm++-5y1

---

## Task Description

Implement Merlin Phases 7-8: Advanced features (Listing Control, String Directives, Macros) and Real-World Validation with Prince of Persia source files.

### Background and Context

**Why:** Final phases to complete Merlin syntax support. Phase 7 adds remaining directives used by PoP. Phase 8 validates everything works with real PoP source files.

**Problem:** While Phases 1-6 handle core functionality, PoP also uses:
- LST/TR listing control (69 uses)
- ASC string directive (2 uses)  
- Real-world validation needed

**Context:** Builds on Phases 1-6. This completes the Merlin feature.

### Current State

**What Exists (after Phases 1-6):**
- ✅ All core directives (ORG, EQU, DB, DW, HEX, DS, DUM, DEND)
- ✅ Label system (global, :local, ]variable)
- ✅ File inclusion (PUT)
- ✅ Data directives (DFB)
- ✅ Conditional assembly (DO/FIN)

**What's Missing:**
- ❌ LST/LSTDO/TR directives
- ❌ ASC string directive
- ❌ Real-world validation with PoP files

### Desired State

**After Completion:**
- ✅ LST directives implemented
- ✅ ASC string directive working
- ✅ Can assemble PoP EQ.S file
- ✅ Can assemble PoP SUBS.S file
- ✅ Full Merlin syntax support complete

---

## Success Criteria

```
✓ LST/LST OFF/LSTDO/TR directives parse correctly
✓ ASC directive outputs strings
✓ Can assemble PoP's EQ.S (equates file)
✓ Can assemble PoP's SUBS.S (subroutines file)
✓ All tests passing (100%)
✓ Code coverage ≥ 85%
✓ Tester validation: APPROVED
✓ Reviewer validation: APPROVED
```

---

## Dependencies

**BLOCKING:** Phases 1-6 must be complete before starting this task.

**Task IDs:**
- xasm++-vp5 (Phases 1-3)
- xasm++-m94 (Phases 4-6)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-24
