# Task Contract

**Task ID:** 2026-02-16_validate-a2osx
**Beads Task:** xasm++-bq87
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Tester
**Workflow:** Standard

---

## Task Description

Rebuild xasm++, install, and validate that A2osX.S.txt assembles successfully with new .ED/.FI/.EL directives.

### Current State

- All directives implemented (.ED, .FI, .EL, .PH, .EP)
- All 1,685 tests passing (100%)
- Code built but not installed

### Desired State

- xasm++ rebuilt and installed to /usr/local/bin/
- A2osX.S.txt assembles successfully (past line 23)
- Binary validation complete

---

## Success Criteria

```
✓ xasm++ rebuilt successfully
✓ xasm++ installed to /usr/local/bin/
✓ A2osX.S.txt assembles without errors
✓ Output binary generated
✓ Assembly proceeds past line 23 (previous failure point)
```

---

## Acceptance Criteria

### Build Requirements
```
□ cmake --build build succeeds
□ Zero compiler warnings
□ sudo cmake --install build succeeds
```

### Validation Requirements
```
□ A2osX.S.txt assembles successfully
□ No "not yet implemented" errors
□ Output binary created
□ File size reasonable (>0 bytes)
```

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
