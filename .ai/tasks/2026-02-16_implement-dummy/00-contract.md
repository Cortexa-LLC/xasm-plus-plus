# Task Contract

**Task ID:** 2026-02-16_implement-dummy
**Beads Task:** xasm++-9cxt
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement .DUMMY directive for structure definitions (define symbols without emitting code).

### Problem

A2osX line 24 in inc/a2osx.i:
```assembly
.DUMMY
ZPPtr1          .BS 2
ZPPtr2          .BS 2
.ED
```

Error: `.DUMMY not yet implemented`

### What .DUMMY Does

- Enters "dummy mode" where directives define symbols but don't emit bytes
- Used for structure/memory layout definitions
- Paired with .ED (End Dummy) to exit dummy mode
- Inside .DUMMY: .BS, .DA, etc. only update address counter and define symbols

### Requirements

1. Add dummy mode flag to parser state
2. Modify .DUMMY to enter dummy mode
3. Modify .ED to exit dummy mode  
4. Modify data directives (.BS, .DA, .DW, etc.) to check dummy mode
5. In dummy mode: update address, define symbols, but DON'T emit bytes

### Success Criteria

```
✓ .DUMMY enters dummy mode
✓ .ED exits dummy mode
✓ Data directives respect dummy mode
✓ A2osX line 24+ assembles
✓ Tests pass
✓ Zero warnings
```

**Estimated Effort:** 2-3 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
