# Task Decomposition Required

**Date:** 2026-02-14
**Engineer:** [Spawned Agent]
**Task:** xasm++-uqzz (P2: Eliminate code duplication)

---

## Why Decomposition is Required

### Complexity Assessment Results

**Scope Analysis:**
```
Duplicate Patterns Identified:
- 20+ radix parsing duplicates
- 189+ error handling duplicates  
- 25+ expression evaluation duplicates
───────────────────────────────────
= 234+ duplication sites

Estimated Files: 30-50+ files affected
File Count: 27+ files → ❌ CRITICAL - MUST decompose into 3+ task packets
```

**Token Budget Risk:**
```
Conservative estimate: 30 files × 3,000 tokens = 90,000 tokens
Agent output limit: 25K-32K tokens
Status: ❌ GUARANTEED FAILURE (3x over limit)
```

**Lean Flow Violation:**
```
Batch Size: 30+ files → ❌ EXCEEDS MAXIMUM (max 14 files per batch)
WIP Limit: Would require 3+ concurrent agents → ❌ EXCEEDS LIMIT
Risk: Verification chaos, token limit failures
```

### Requirements Clarity

**Missing Specification:**
```
❌ No task packet contract (00-contract.md empty/template)
❌ No implementation plan (10-plan.md empty/template)
❌ No specific files identified
❌ No acceptance criteria defined
❌ No priority ordering (which duplicates first?)
```

---

## Recommended Decomposition

### Subtask 1: Radix Parsing Utilities (Small - Priority 1)
```
Estimated scope: 20+ duplicates across ~5-8 files
Estimated effort: 3-4 hours
Priority: High (foundational for other refactoring)

Files likely affected:
- src/parser/*.cpp (expression parsing)
- src/cpu/cpu_6502.cpp
- src/cpu/cpu_z80.cpp
- include/xasm++/parse_utils.h (new utility)

Deliverables:
- Extract common radix parsing utilities
- ParseHex, ParseDecimal, ParseBinary, ParseOctal functions
- Comprehensive test coverage
- Replace duplicates with utility calls

Token estimate: ~8 files × 3,000 = 24,000 tokens (✅ SAFE)
```

### Subtask 2: Expression Evaluation Patterns (Medium - Priority 2)
```
Estimated scope: 25+ duplicates across ~6-10 files
Estimated effort: 4-5 hours
Priority: Medium (depends on radix utilities)

Files likely affected:
- src/parser/expression_parser.cpp
- src/parser/operand_parser.cpp
- src/assembler/*.cpp (expression evaluation)
- include/xasm++/expression_utils.h (new utility)

Deliverables:
- Extract common expression evaluation patterns
- Utility functions for common operations
- Test coverage
- Replace duplicates

Token estimate: ~10 files × 3,000 = 30,000 tokens (⚠️ APPROACHING LIMIT)
Recommend: Split into 2 sub-batches if needed
```

### Subtask 3: Error Handling Patterns (Large - Priority 3)
```
Estimated scope: 189+ duplicates across ~20-30 files
Estimated effort: 6-8 hours
Priority: Lower (most pervasive, needs careful planning)

⚠️ THIS SUBTASK STILL TOO LARGE - needs further decomposition into:
  - Subtask 3a: Parser error handling (8-10 files)
  - Subtask 3b: CPU instruction error handling (6-8 files)
  - Subtask 3c: Directive error handling (6-8 files)

Each sub-subtask: ~8 files × 3,000 = 24,000 tokens (✅ SAFE)

Deliverables (per sub-subtask):
- Extract common error handling patterns
- Error utility functions or macros
- Consistent error messages
- Test coverage
- Replace duplicates in scope
```

---

## Execution Strategy

### Sequential Execution (Recommended)
```
SEQUENCE:
1. Subtask 1 (Radix) - Foundation for others
   ↓
2. Subtask 2 (Expression) - Depends on radix utilities
   ↓
3. Subtask 3a (Parser errors) - Independent
4. Subtask 3b (CPU errors) - Independent
5. Subtask 3c (Directive errors) - Independent

WIP Limit: Max 1 active task at a time
Reason: Sequential allows verification between phases
```

### Verification Between Subtasks
```
After each subtask:
✓ All tests passing
✓ Build succeeds with zero warnings
✓ No regressions introduced
✓ Coverage maintained/improved
✓ Document lessons learned
```

---

## Why This Can't Proceed As Single Task

**Engineering Principle Violations:**
1. ❌ Scope too large (234+ sites, 30+ files)
2. ❌ Token budget guaranteed failure (90K tokens vs 32K limit)
3. ❌ Lean Flow violation (batch size >14 files)
4. ❌ No clear specification or planning
5. ❌ High risk of thrashing (too many moving parts)

**Historical Evidence:**
```
Similar large refactorings that failed:
- 300+ turn debugging sessions when scope unclear
- Token limit hits causing incomplete refactors
- Verification chaos with too many concurrent changes
```

**Professional Standards:**
```
A 12-16 hour task with 234+ duplication sites across 30+ files
is not a single task - it's a REFACTORING INITIATIVE that
requires decomposition, planning, and phased execution.
```

---

## Request to Orchestrator

**Action Requested:**
```
1. Acknowledge this task is too large for single execution
2. Create decomposed task packets for Subtasks 1, 2, 3a, 3b, 3c
3. Provide proper contract and planning documents for each
4. Assign Subtask 1 (Radix) as starting point
5. Sequential execution with verification gates between subtasks
```

**Cannot Proceed Until:**
```
□ Decomposition approved
□ Task packets created for each subtask
□ Clear acceptance criteria defined
□ Proper planning completed
```

---

**Engineer Status:** Blocked (awaiting task decomposition)
**Beads Task:** xasm++-uqzz (marked as blocked)
**Recommendation:** Start with Subtask 1 (Radix Parsing) as proof of concept

---

## Reference

- Role Requirements: `.ai/agents/roles/engineer.md` Section 0.75
- Lean Flow Gate: `.ai/gates/05-lean-flow.md`
- Complexity Assessment: Engineer role Pre-Implementation section
