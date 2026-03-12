# Task Contract

**Task ID:** 2026-02-09_z80-phase2-symbols
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-4n2

---

## Task Description

Implement advanced symbol directives for Z80 Universal Syntax Phase 2, enabling symbol visibility control and linking support (PUBLIC/GLOBAL/ENTRY for exports, EXTERN/EXTRN/EXT for imports).

### Background and Context

This is Phase 2 of the Z80 Universal Syntax implementation. Phase 1 (xasm++-1fn) established the foundation with:
- Basic data directives (DB, DW, DS)
- All label styles (LABEL:, LABEL::, $LOCAL:, .local)
- Number formats ($FF, 0xFF, 0FFH, binB, octO)
- Symbol definitions (EQU, SET, DEFL)

Phase 2 adds symbol visibility control needed for multi-file assembly and linking:
- PUBLIC/GLOBAL/ENTRY - Mark symbols for export
- EXTERN/EXTRN/EXT - Declare imported symbols
- Symbol visibility tracking

### Current State

- Z80 Universal Syntax Phase 1 complete (26/26 tests passing)
- Basic symbol definitions working (EQU, SET, DEFL)
- No symbol visibility control
- No import/export support

### Desired State

- PUBLIC/GLOBAL/ENTRY directives implemented
- EXTERN/EXTRN/EXT directives implemented
- Symbol table tracks visibility (private, public, extern)
- Integration with existing z80_universal_syntax.cpp
- Comprehensive tests for all visibility scenarios

---

## Success Criteria

```
✓ PUBLIC directive marks symbols as exportable
✓ GLOBAL directive (synonym for PUBLIC) works
✓ ENTRY directive (synonym for PUBLIC) works
✓ EXTERN directive declares imported symbols
✓ EXTRN directive (synonym for EXTERN) works
✓ EXT directive (synonym for EXTERN) works
✓ Symbol table tracks visibility correctly
✓ Tests verify all directive variations
✓ Integration with Phase 1 code complete
✓ All tests passing (existing + new)
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements - PUBLIC/GLOBAL/ENTRY
```
□ PUBLIC label - mark symbol as public/exportable
□ PUBLIC label1, label2, label3 - multiple symbols per directive
□ GLOBAL synonym supported
□ ENTRY synonym supported
□ Public symbols visible in symbol table
□ Error if marking undefined symbol as public
□ Interaction with label definitions (:/::/etc)
```

### Functional Requirements - EXTERN/EXTRN/EXT
```
□ EXTERN label - declare imported symbol
□ EXTERN label1, label2, label3 - multiple symbols per directive
□ EXTRN synonym supported
□ EXT synonym supported
□ Extern symbols visible in symbol table
□ Extern symbols can be used in expressions
□ No error if extern symbol never defined (linker's job)
□ Interaction with symbol definitions (cannot redefine)
```

### Functional Requirements - Symbol Table
```
□ Symbol visibility tracked (private, public, extern)
□ Symbol.h extended with visibility enum/flags
□ Symbol::SetVisibility() method
□ Symbol::GetVisibility() method
□ Symbol table queries by visibility
```

### Functional Requirements - Integration
```
□ HandleDirective() extended for PUBLIC/EXTERN
□ Parse comma-separated symbol lists
□ Work with existing label parsing
□ Work with existing symbol table
□ No disruption to Phase 1 functionality
```

### Quality Requirements
```
□ All tests passing (Phase 1 + Phase 2)
□ Code coverage ≥ 90% for new code
□ No compiler warnings
□ TDD methodology (RED-GREEN-REFACTOR)
□ Code review standards met
```

### Non-Functional Requirements
```
□ Performance: Minimal overhead for visibility tracking
□ Error messages: Clear and actionable
□ Code style: Google C++ Style Guide
□ Documentation: Doxygen comments complete
```

---

## Constraints and Dependencies

### Constraints
```
□ Must integrate with existing z80_universal_syntax.cpp
□ Must not break Phase 1 functionality (26 tests must still pass)
□ Must follow existing symbol table architecture
□ Must support all three synonym variants per directive
```

### Dependencies
```
□ Z80 Universal Syntax Phase 1 (xasm++-1fn) - COMPLETE ✓
□ Symbol class (include/xasm++/assembler/symbol.h)
□ Symbol table (part of assembler core)
□ z80_universal_syntax.h/.cpp (modify)
```

### Out of Scope
```
✗ Actual linking (that's a separate linker component)
✗ Object file generation (Phase 10 integration)
✗ Cross-reference generation (Phase 8 listing control)
✗ Expression evaluation (Phase 3)
✗ Macros (Phase 6)
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 3 (z80_universal_syntax.h/.cpp, test_z80_universal_syntax.cpp)
- Lines of code estimate: ~300-400 (directive handlers, symbol table extension, tests)
- New concepts/patterns: Symbol visibility enum
- Integration complexity: Low (extends existing code)
- Risk level: Low (well-defined directives, clear semantics)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 3 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 3 files → ✅ IDEAL (small batch, proceed)
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
3 × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 9,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

**Status:** ✅ SAFE to proceed as single task

**Reference:** `gates/05-lean-flow.md` for enforcement details

---

## Resources and References

### Relevant Files
```
- include/xasm++/syntax/z80_universal_syntax.h - Extend with PUBLIC/EXTERN handlers
- src/syntax/z80_universal/z80_universal_syntax.cpp - Implement directive logic
- tests/unit/test_z80_universal_syntax.cpp - Add test cases
- include/xasm++/assembler/symbol.h - Extend with visibility tracking
```

### Documentation
```
- M80 Manual: PUBLIC and EXTRN directives
- ZMAC: Symbol visibility conventions
- Z80ASM: GLOBAL and EXT directives
- Phase 1 contract: .ai/tasks/2026-02-09_z80-universal-syntax/00-contract.md
```

### Examples
```
- FLEX ASM: PUBLIC/EXTERN implementation (for reference patterns)
- Phase 1 implementation: HandleDirective() pattern
```

---

## Assumptions

```
1. Symbol class can be extended with visibility enum
2. Comma-separated parsing follows standard conventions
3. PUBLIC before definition or after definition both allowed
4. EXTERN symbols don't need definitions in current file
5. Symbol table already supports querying by properties
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Symbol table API limitations
   - Probability: Low
   - Impact: Medium
   - Mitigation: Check Symbol class API early, extend if needed

2. Phase 1 regression
   - Probability: Low
   - Impact: High
   - Mitigation: Run Phase 1 tests after changes, no modifications to existing handlers

3. Directive synonym conflicts
   - Probability: Low
   - Impact: Low
   - Mitigation: Test all synonym variants, check existing directive names
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User [2026-02-09]
- [X] Agent: Orchestrator [2026-02-09]

**Changes to Contract:**
[None yet]

---

## Notes

This is Phase 2 of 10 for Z80 Universal Syntax. Dependencies chain:
- Phase 1 (xasm++-1fn): ✅ COMPLETE
- Phase 2 (xasm++-4n2): 👈 CURRENT
- Phase 3 (xasm++-ek2): Depends on Phase 2
- Phase 4 (xasm++-a5g): Depends on Phase 3
- ... etc through Phase 10

Phase 2 is a small, focused task (3 files, 9K tokens) with low risk.

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
