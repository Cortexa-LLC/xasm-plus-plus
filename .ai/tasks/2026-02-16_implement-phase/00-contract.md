# Task Contract

**Task ID:** 2026-02-16_implement-phase
**Beads Task:** xasm++-psdb
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement .PH (Phase) and .EP (End Phase) directives for relocatable code assembly.

### What Phase Assembly Does

- `.PH address` - Set phase address (logical/execution address)
- Code assembled at current address but labeled with phase address
- `.EP` - End phase, return to normal addressing
- Used when code loads at one address but runs at another

### Success Criteria

```
✓ .PH directive implemented
✓ .EP directive implemented  
✓ A2osX line 35+ assembles
✓ Tests pass
```

**Estimated Effort:** 2-3 hours

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
**Beads Task:** 
