# Task Contract

**Task ID:** 2026-02-16_fix-ph-tests
**Beads Task:** xasm++-xmwf
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

6 tests are failing for .PH directive after implementing .ED/.FI/.EL directives.

**Critical:** ALL tests must pass. Leaving failing tests is unacceptable.

### Investigation Required

1. **Are these pre-existing failures?**
   - Check git history
   - Verify if .PH was implemented before

2. **Or are these regressions?**
   - Did .ED/.FI/.EL changes break .PH?
   - Check for conflicts in directive handling

### Current State

- 141/147 tests passing
- 6 failures related to .PH directive
- .ED/.FI/.EL directives implemented and working

### Desired State

- **147/147 tests passing**
- .PH directive functional (if not implemented)
- No regressions in .PH (if already implemented)
- Zero compiler warnings

---

## Success Criteria

```
✓ All 147 tests passing
✓ .PH directive working correctly
✓ No regressions introduced
✓ Zero build warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Root cause identified (pre-existing vs regression)
□ .PH directive implemented or fixed
□ All 6 failing tests now pass
□ All previously passing tests still pass
```

### Quality Requirements
```
□ 147/147 tests passing
□ Zero compiler warnings
□ Code review quality
□ TDD approach if implementing new functionality
```

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
