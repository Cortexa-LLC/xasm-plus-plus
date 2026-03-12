# Task Contract

**Task ID:** 2026-02-16_forward-refs-v2
**Beads Task:** xasm++-5z6k
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement forward reference resolution for .DA directive (re-implementation).

### Problem

.DA directive evaluates expressions immediately, causing errors for forward references:
```assembly
Line 76:  .DA MLIGETPREFIX01    ; Use (undefined yet)
Line 629: MLIGETPREFIX01:       ; Definition (553 lines later)
```

Error: `Undefined symbol: MLIGETPREFIX01`

### Solution

Use DataAtom expressions-based constructor instead of immediate evaluation:
- Store expression strings for deferred evaluation
- Let multi-pass assembler resolve during later passes
- Convert SCMASM byte operators (#, /) to generic operators (<, >)

### Previous Investigation (xasm++-kbgs)

From earlier agent work (never committed):
- DataAtom has two constructors: data-based (current) and expressions-based (needed)
- assembler.cpp already has multi-pass forward reference logic (lines 196-230)
- SCMASM uses # (low byte) and / (high byte), need conversion to < and >

### Success Criteria

```
✓ .DA uses expressions-based DataAtom
✓ Forward references resolve correctly
✓ A2osX line 76 assembles
✓ Tests pass
✓ Zero warnings
```

**Estimated Effort:** 2-3 hours

---

**Contract Version:** 1.0  
**Last Updated:** 2026-02-16
