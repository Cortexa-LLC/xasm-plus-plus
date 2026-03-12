# Task Contract: God-Class Phase 6c - Full Handler Extraction

**Task ID:** 2026-02-10_god-class-phase6c-full
**Beads Task:** xasm++-ci14
**Created:** 2026-02-10
**Requestor:** Orchestrator (from decomposition of xasm++-3mwi)
**Assigned Role:** Orchestrator (DECOMPOSITION REQUIRED)
**Workflow:** Refactor

---

## Task Description

Extract remaining ~70 directive handlers across all parsers (Merlin, SCMASM, EDTASM-M80++), following the pattern established in Phase 6b. Eliminate all magic numbers and magic strings using named constants.

**CRITICAL:** This task is too large (estimated 15-20+ files) and MUST be decomposed into subtasks before execution.

### Background and Context

Phase 6b established the pattern:
- Extract handlers as free functions
- Create DirectiveHandler wrappers matching signature
- Register with DirectiveRegistry
- Eliminate magic numbers/strings using constants
- Comprehensive testing

Phase 6c applies this pattern to remaining ~70 directives across 3 parsers.

### Current State

- Phase 6b complete: Pattern proven with 5 directives
- Merlin: ~26 directives remaining (31 total - 5 extracted)
- SCMASM: ~25 directives remaining
- EDTASM-M80++: ~20 directives remaining
- Total: ~70 handlers to extract

### Desired State

All directives extracted, registered, and tested. Zero magic numbers/strings in directive code.

---

## Success Criteria

```
✓ All ~70 remaining directives extracted
✓ All handlers registered with DirectiveRegistry
✓ Zero magic numbers in directive code
✓ Zero magic strings in directive code
✓ All tests passing (97%+)
✓ God-Class Phase 6 complete
```

---

## MANDATORY DECOMPOSITION

### Batch Size Assessment

**Estimated Total Files:** 18-24 files
- 3 parser headers + implementations (6 files)
- 3-6 handler files (directives split by category)
- 2-3 constant files
- 3-6 test files
- Additional integration files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 18-24 files → TOO LARGE ❌
```

**Token Budget:**
```
18-24 files × 3,000 tokens = 54,000-72,000 tokens
Agent limit: 25K-32K tokens
Status: ❌ GUARANTEED FAILURE (2-3x over limit)
```

**DECOMPOSITION REQUIRED:** YES - MANDATORY

---

## Decomposition Plan

Break Phase 6c into 3 subtasks by parser (high cohesion, independent execution):

### Subtask 6c.1: Merlin Handler Extraction
**Task ID:** xasm++-ci14.1 (to be created)
**Scope:** Extract ~26 remaining Merlin directives
**Files:** 8-10 files
- merlin_syntax.h/cpp
- merlin_directive_handlers.h/cpp
- directive_constants.h (extend)
- test_merlin_directives.cpp
**Tokens:** ~27K tokens (⚠️ approaching limit, acceptable)
**Dependency:** Phase 6b complete (✓)

### Subtask 6c.2: SCMASM Handler Extraction
**Task ID:** xasm++-ci14.2 (to be created)
**Scope:** Extract ~25 SCMASM directives
**Files:** 8-10 files
- scmasm_syntax.h/cpp
- scmasm_directive_handlers.h/cpp
- directive_constants.h (extend)
- test_scmasm_directives.cpp
**Tokens:** ~27K tokens (⚠️ approaching limit, acceptable)
**Dependency:** Phase 6b complete (✓), can run in parallel with 6c.1

### Subtask 6c.3: EDTASM-M80++ Handler Extraction
**Task ID:** xasm++-ci14.3 (to be created)
**Scope:** Extract ~20 EDTASM-M80++ directives
**Files:** 8-10 files
- edtasm_m80_plusplus_syntax.h/cpp
- edtasm_directive_handlers.h/cpp
- directive_constants.h (extend)
- test_edtasm_directives.cpp
**Tokens:** ~27K tokens (⚠️ approaching limit, acceptable)
**Dependency:** Phase 6b complete (✓), can run in parallel with 6c.1 & 6c.2

**Execution Strategy:** Sequential OR Parallel (independent parsers)
**WIP Limit:** Maximum 3 concurrent agents (one per parser)

---

## Acceptance Criteria (Per Subtask)

### Functional Requirements
```
□ Extract all directives for assigned parser
□ Create handler functions following Phase 6b pattern
□ Create DirectiveHandler wrappers matching signature
□ Register all handlers with DirectiveRegistry
□ Verify all directives work identically to before
```

### Code Quality Requirements (CRITICAL)
```
□ NO magic numbers - use named constants from opcodes_*.h
□ NO magic strings - use constants from directive_constants.h
□ Extend directive_constants.h with new directive names
□ Use existing opcodes_z80.h, opcodes_6502.h, opcodes_6809.h
□ Create mnemonic_constants.h if needed
```

### Quality Requirements
```
□ All tests passing (97%+ target)
□ No warnings
□ TDD followed
□ Code formatted per project standards
□ Functions documented
```

---

## Estimated Complexity

**Complexity:** Large (requires decomposition)

**Total Effort:** 6-9 hours (2-3 hours per subtask)
**Per Subtask:** Medium (2-3 hours each)

---

## Dependencies

**Depends On:** xasm++-uqyd (Phase 6b - Core Handler Extraction) - ✓ COMPLETE
**Blocks:** Nothing (completes God-Class Phase 6)
**Reference:** `.ai/tasks/2026-02-09_god-class-phase6-directive/ESCALATION.md`

---

## Orchestrator Actions Required

**BEFORE SPAWNING AGENTS:**

1. **Create 3 subtasks in Beads:**
   ```bash
   bd create "Phase 6c.1: Merlin Handler Extraction" --priority 1
   bd create "Phase 6c.2: SCMASM Handler Extraction" --priority 1
   bd create "Phase 6c.3: EDTASM-M80++ Handler Extraction" --priority 1
   ```

2. **Create task packets for each subtask:**
   ```bash
   mkdir -p .ai/tasks/2026-02-11_god-class-phase6c1-merlin/
   mkdir -p .ai/tasks/2026-02-11_god-class-phase6c2-scmasm/
   mkdir -p .ai/tasks/2026-02-11_god-class-phase6c3-edtasm/
   ```

3. **Create contracts for each subtask** (copy this template, adjust scope)

4. **Set up dependencies:**
   ```bash
   # All depend on Phase 6b (already complete)
   bd dep add <6c.1-id> xasm++-uqyd
   bd dep add <6c.2-id> xasm++-uqyd
   bd dep add <6c.3-id> xasm++-uqyd
   ```

5. **Spawn agents:**
   ```bash
   # Option A: Sequential (safer, easier to review)
   agent engineer <6c.1-id> --stream  # Wait for completion
   agent engineer <6c.2-id> --stream  # Then this
   agent engineer <6c.3-id> --stream  # Then this

   # Option B: Parallel (faster, max WIP=3)
   agent engineer <6c.1-id>  # Spawn all
   agent engineer <6c.2-id>
   agent engineer <6c.3-id>
   # Then wait for all to complete
   ```

6. **Close parent task:**
   ```bash
   bd close xasm++-ci14 -r "Decomposed into 3 subtasks: 6c.1, 6c.2, 6c.3"
   ```

---

## Resources and References

### Pattern Reference (From Phase 6b)
```
.ai/tasks/2026-02-10_god-class-phase6b-core/ENGINEER-REPORT.md - Implementation pattern
```

### Existing Constant Files
```
include/xasm++/cpu/opcodes_z80.h - Z80 opcodes
include/xasm++/cpu/opcodes_6502.h - 6502 opcodes
include/xasm++/cpu/opcodes_6809.h - 6809 opcodes
include/xasm++/directives/directive_constants.h - Created in Phase 6b
```

---

## Special Instructions

**Magic Number/String Elimination (MANDATORY):**
- Follow Phase 6b pattern exactly
- Use directive_constants.h for directive names
- Use opcodes_*.h for CPU opcodes
- Create mnemonic_constants.h if mnemonics used
- No hardcoded strings or hex values in production code

**Testing Strategy:**
- Each subtask includes comprehensive tests
- Tests verify directives work identically to before
- Tests verify registration works correctly
- Aim for 95%+ coverage on directive handlers

---

**Contract Status:** ⚠️ REQUIRES DECOMPOSITION - Cannot execute directly
**Next Action:** Orchestrator must create subtasks before spawning agents

---

**Contract Approved:** ✓ Orchestrator 2026-02-11
