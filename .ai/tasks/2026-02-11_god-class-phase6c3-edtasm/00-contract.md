# Task Contract: Phase 6c.3 - EDTASM-M80++ Handler Extraction

**Task ID:** 2026-02-11_god-class-phase6c3-edtasm
**Beads Task:** xasm++-5j00
**Created:** 2026-02-11
**Requestor:** Orchestrator (from Phase 6c decomposition)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract ~20 remaining EDTASM-M80++ directive handlers following Phase 6b pattern. Eliminate all magic numbers and magic strings using named constants.

### Background and Context

- Phase 6b extracted 5 core directives (ORG, EQU, DB, DW, DS) and established the pattern
- EDTASM-M80++ has ~47 total directive registrations, but many share implementations
- This task applies the proven pattern to EDTASM-M80++ specific directives
- Focus on extracting unique handler logic, not duplicate registrations

### Current State

- EDTASM-M80++: ~47 directive registrations as lambda functions in edtasm_m80_plusplus_syntax.cpp
- Phase 6b pattern established and working
- directive_constants.h exists with 5 directive names
- File is 1797 lines long with all handlers inline

### Desired State

All EDTASM-M80++ directives extracted, registered, and tested. Zero magic numbers/strings.

---

## Success Criteria

```
✓ ~20 unique EDTASM-M80++ directive handlers extracted as free functions
✓ All handlers registered with DirectiveRegistry
✓ Zero magic numbers (use opcodes_z80.h)
✓ Zero magic strings (use directive_constants.h)
✓ All tests passing (97%+)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Extract all EDTASM-M80++ directives (focus on unique implementations)
□ Follow Phase 6b pattern exactly:
  - Free functions for logic
  - Wrapper functions matching DirectiveHandler signature
  - Registration with DirectiveRegistry
□ Verify directives work identically to before
□ Handle multiple directive names mapping to same handler
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use opcodes_z80.h constants
□ NO magic strings - use directive_constants.h
□ Add new directive names to directive_constants.h
□ Use constexpr for all constants
□ Use existing Z80Directives namespace constants
```

### Quality Requirements
```
□ All tests passing (97%+)
□ No warnings
□ TDD followed
□ Code documented
```

---

## Estimated Complexity

**Complexity:** Medium
**Files:** 8-10
**Tokens:** ~27K → ⚠️ Approaching limit, acceptable

### Affected Files (Estimated)
```
src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp - Reduce from 1797 lines
src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.h - Update declarations
src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.cpp - NEW: Handler implementations
src/syntax/edtasm_m80_plusplus/edtasm_directive_handlers.h - NEW: Handler declarations
include/xasm++/directives/directive_constants.h - Extend with EDTASM names
tests/test_edtasm_directives.cpp - Test handlers
```

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 8-10 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 8-10 files → ⚠️ ACCEPTABLE
```

### Batch Size Justification

Files: 8-10 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all files related to EDTASM-M80++ parser
- Single concern - extracting one parser's directive handlers
- Already minimal viable batch (can't split parser logic further)
- Files are tightly coupled (handlers reference parser state)

**Contingency for token limits:**
- If token limit hit, will decompose into: Core directives + Conditional directives

**Estimated tokens:** ~10 × 3,000 = ~30,000 tokens
**Status:** Within 25K-32K limit? YES (high end, but acceptable)

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
10 × 3,000 tokens = 30,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 30,000 tokens → ⚠️ APPROACHING LIMIT
```

**Mitigation:** If token limit reached, engineer should complete core handlers first, then continue with remaining handlers in follow-up.

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (this task)
```

**Note:** This is subtask 6c.3 of 3 parallel subtasks (6c.1, 6c.2, 6c.3). Total WIP across all Phase 6c work = 3 agents maximum.

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:** 10 files, ~30K tokens, WIP=1. Within all limits but at high end. Acceptable to proceed with contingency plan if needed.

---

## Dependencies

**Depends On:** xasm++-uqyd (Phase 6b) - ✓ COMPLETE
**Parallel With:** 
- xasm++-kksh (Phase 6c.1 - Merlin) - May run concurrently
- xasm++-1ilq (Phase 6c.2 - SCMASM) - May run concurrently
**Reference:**
- `.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md` - Pattern to follow
- `.ai/tasks/2026-02-10_god-class-phase6c-full/00-contract.md` - Parent task

---

## Resources and References

### Relevant Files
```
src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp - Current implementation (1797 lines)
src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.h - Parser header
include/xasm++/directives/directive_constants.h - Existing constants (extend)
include/xasm++/cpu/opcodes_z80.h - Z80 opcodes
include/xasm++/directives/z80_directives.h - Z80 directive namespace
```

### Pattern Reference (From Phase 6b)
```
.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md - Implementation pattern
```

### Directive List (47 registrations, ~20 unique handlers)

**Segment Control:**
- ASEG, CSEG, DSEG (3 handlers)

**Conditional Assembly:**
- IF, ELSE, ENDIF (3 handlers)
- IFDEF, IFNDEF, IFEQ, IFNE, IFLT, IFGT, IFLE, IFGE (8 handlers)
- IF1, IF2, IFB, IFNB, IFIDN, IFDIF (6 handlers)

**Macro System:**
- MACRO, ENDM, LOCAL, EXITM (4 handlers)
- REPT, IRP, IRPC (3 handlers)

**Listing Control:**
- LIST, NOLIST/XLIST, *LIST (3 handlers)
- LALL, SALL (2 handlers)
- TITLE, SUBTTL, EJECT, SPACE (4 handlers)
- NAME, RADIX (2 handlers)

**Other:**
- ORG, EQU (already in core_directive_handlers.cpp)
- END (1 handler)

**Note:** Many directives share implementations (e.g., {LIST, ".LIST"} → same handler)

---

## Assumptions

```
1. Phase 6b pattern is correct and complete
2. Tests exist for EDTASM-M80++ directives
3. Z80Directives namespace already has needed constants
4. Parser state members (in_repeat_block_, etc.) remain accessible to handlers
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Token limit exceeded
   - Probability: Medium
   - Impact: Medium (blocks completion)
   - Mitigation: Extract core handlers first, follow-up for remainder

2. Handler state dependencies
   - Probability: Low
   - Impact: Medium (requires refactoring pattern)
   - Mitigation: Pass parser reference or state via DirectiveContext

3. Complex conditional/macro handlers
   - Probability: Medium
   - Impact: Low (more time needed)
   - Mitigation: Follow Phase 6b pattern, test incrementally
```

---

## Special Instructions

**Handler Grouping Strategy:**

Group handlers into logical categories for readability:
1. Segment control (ASEG, CSEG, DSEG)
2. Conditional assembly (IF family)
3. Macro system (MACRO, REPT, etc.)
4. Listing control (LIST family)
5. Miscellaneous (END, NAME, RADIX)

**State Access Pattern:**

Many EDTASM handlers access parser state (in_repeat_block_, macro_table_, etc.). Use one of these approaches:
1. Pass parser pointer via DirectiveContext (preferred)
2. Make handlers friend functions
3. Make state accessible via getters

Follow the approach established in Phase 6b.

**Testing Strategy:**

Test each category of handlers:
- Segment switching works correctly
- Conditionals evaluate properly
- Macros expand correctly
- Listing controls affect output

Aim for 95%+ coverage on directive handlers.

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Orchestrator [2026-02-11]
- [ ] Agent: Engineer [Date]

**Changes to Contract:**
[Document any contract changes here with date and rationale]

---

## Notes

**Key Differences from Merlin/SCMASM:**
- EDTASM-M80++ is Z80-specific (simpler opcodes)
- More conditional directives than other parsers
- Complex macro system with REPT, IRP, IRPC
- M80 compatibility means different listing directives

**Success Indicators:**
- edtasm_m80_plusplus_syntax.cpp reduced from 1797 lines to ~400-600 lines
- New edtasm_directive_handlers.cpp contains ~1000-1200 lines
- All 47 directive registrations still work
- Zero magic strings/numbers in handler code

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-11

---

**Contract Status:** ✅ READY FOR ENGINEER
**Next Action:** Spawn engineer agent with this task packet
