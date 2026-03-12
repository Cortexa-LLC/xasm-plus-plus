# Task Contract: Z80 Phase 13 - Listing File Generation

**Task ID:** 2026-02-10_z80-phase13-listing
**Beads Task:** xasm++-55ok
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement listing file (.lst) output for EDTASM-M80++ parser and make listing control directives functional.

### Background and Context

Phase 10 identified listing output as missing. Listing directives (TITLE, PAGE, LIST, NOLIST, etc.) are recognized but produce no output. Need to:
1. Generate .lst file format
2. Make listing control directives functional
3. Only active when --list/-l flag specified

### Current State

Listing directives are no-ops. No .lst file generated.

### Desired State

.lst file generated showing addresses, bytes, and source lines with proper formatting controlled by listing directives.

---

## Success Criteria

```
✓ .lst file generated when --list flag used
✓ LIST/NOLIST directives control output
✓ TITLE/PAGE/SPACE/EJECT format properly
✓ Shows address, bytes, source line
✓ All tests passing (97%+)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Generate .lst alongside binary output
□ LIST/NOLIST toggle listing on/off
□ LALL/SALL control macro expansion visibility
□ TITLE sets page title
□ PAGE/EJECT force page breaks
□ SPACE inserts blank lines
□ Format: address | bytes | source line
□ Line numbering
```

### Quality Requirements
```
□ All tests passing
□ No warnings
□ TDD followed
□ Documentation updated
```

---

## Estimated Complexity

**Complexity:** Medium

**Files:** 4 (listing output, parser integration, CLI, tests)
**Tokens:** 12K → ✅ SAFE

---

## Dependencies

**Depends on:** Nothing
**Blocks:** Nothing

---

**Contract Approved:** ✓ Orchestrator 2026-02-10
