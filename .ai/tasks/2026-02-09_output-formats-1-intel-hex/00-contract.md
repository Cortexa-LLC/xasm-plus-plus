# Task Contract

**Task ID:** 2026-02-09_output-formats-1-intel-hex
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-dxgm
**Parent Task:** xasm++-hze (decomposed) → .ai/tasks/2026-02-09_output-formats/

---

## Task Description

Implement OutputWriter interface and Intel HEX format writer. This is **Subtask 1 of 3** from the decomposed Output Formats task.

### Background and Context

This is the first phase of implementing multiple output formats for xasm++. The parent task was identified as HIGH RISK (36K tokens) and decomposed into 3 sequential subtasks for safety.

**Parent Task:** `.ai/tasks/2026-02-09_output-formats/00-contract.md`
**Parent Plan:** `.ai/tasks/2026-02-09_output-formats/10-plan.md`

### Current State

- No output format infrastructure exists
- Section/Atom framework provides assembled binary data
- This is Phase 1 (foundation)

### Desired State

- OutputWriter base interface defined
- Intel HEX format writer fully implemented
- Comprehensive tests for Intel HEX format
- Foundation ready for Subtasks 2 & 3

---

## Success Criteria

```
✓ OutputWriter interface defined
✓ Intel HEX writer implemented
✓ All Intel HEX tests passing
✓ Record type 00, 01, 04/05 working
✓ Checksum validation working
✓ Format validated against standards
✓ Zero compiler warnings
✓ Code coverage ≥ 95%
```

---

## Acceptance Criteria

### Functional Requirements - OutputWriter Interface
```
□ Base class/interface defined
□ Write() method signature
□ Common utilities (if any)
□ Ready for inheritance by format writers
```

### Functional Requirements - Intel HEX Format
```
□ Record type 00 (data records)
□ Record type 01 (EOF)
□ Record type 04/05 (extended address for >64K)
□ Checksum calculation correct
□ Configurable bytes-per-line (16-32 typical)
□ Proper address handling
□ Multiple segments supported
```

### Quality Requirements
```
□ All tests passing
□ Code coverage ≥ 95%
□ No compiler warnings
□ TDD methodology followed
□ Doxygen documentation complete
```

---

## Constraints and Dependencies

### Constraints
```
□ Must work with existing Section/Atom framework
□ Google C++ Style Guide compliance
```

### Dependencies
```
✓ Section/Atom framework (exists)
```

### Out of Scope
```
✗ Other output formats (Subtask 2)
✗ CLI integration (Subtask 3)
✗ Listing output (Subtask 3)
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files: 5
- Lines of code estimate: ~600-800
- Token estimate: ~12K (SAFE)
- New concepts: OutputWriter interface, Intel HEX format
- Integration complexity: Low (standalone)
- Risk level: Low (well-defined format)

---

## Lean Flow Analysis

**Estimated Files:** 5 files
**Batch Size Evaluation:** ✅ IDEAL (1-5 files)
**Token Budget:** 12K tokens → ✅ SAFE

---

## Resources and References

### Relevant Files
```
- include/xasm++/assembler/section.h - Section/Atom framework
```

### Documentation
```
- Intel HEX: https://en.wikipedia.org/wiki/Intel_HEX
- Parent contract: .ai/tasks/2026-02-09_output-formats/00-contract.md
- Parent plan: .ai/tasks/2026-02-09_output-formats/10-plan.md
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User [2026-02-09]
- [X] Agent: Orchestrator [2026-02-09]

---

## Notes

**Subtask 1 of 3** - Foundation phase
- **Next:** Subtask 2 (xasm++-lyai) - S-Record + Binary formats
- **Then:** Subtask 3 (xasm++-qttk) - Listing + CLI integration

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
