# Task Contract

**Task ID:** 2026-02-12_edtasm-segments
**Beads Task:** xasm++-bysw
**Created:** 2026-02-12
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 3 EDTASM-M80++ segment directives for relocatable code support.

### Background and Context

EDTASM-M80++ supports relocatable code through segment directives. ASEG creates absolute (fixed address) segments, while CSEG and DSEG create relocatable code and data segments. This enables generation of object code that can be linked at different addresses.

### Current State

- Segment manager exists (include/xasm++/segment_manager.h)
- No EDTASM segment directives implemented
- Only absolute addressing currently supported

### Desired State

3 segment directives implemented:
- ASEG - Absolute segment (fixed address)
- CSEG - Code segment (relocatable code)
- DSEG - Data segment (relocatable data)

---

## Success Criteria

```
✓ All 3 segment directives implemented
✓ Segment switching mechanism working
✓ Current segment type tracked
✓ Relocatable vs absolute addressing distinguished
✓ All tests passing
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ ASEG creates absolute segment
□ CSEG creates code segment
□ DSEG creates data segment
□ Segment switching works correctly
□ Address resolution per segment type
□ Proper integration with segment_manager
```

### Quality Requirements
```
□ TDD process followed
□ Tests for all segment types
□ Tests for segment switching
□ No reduction in test pass rate
```

---

## Technical Approach

**Files to Modify:**
- 3 directive handlers
- Segment manager integration
- Tests

**Estimated:** 4-6 hours, 3-4 files, ~15K tokens

---

## Related Documents

- **Reference:** docs/planning/not-yet-implemented-items.md (Section 4)
- **Existing:** include/xasm++/segment_manager.h
