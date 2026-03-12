# Task Contract

**Task ID:** 2026-01-31_phase1-equate-detection
**Beads Task:** xasm++-l0q
**Priority:** P2 (High - part of binary compatibility)
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++

---

## Task Description

Implement equate-only file detection to produce 0-byte output files (matching vasm behavior).

### Background

Some assembly files contain only equates (EQU directives) with no actual code or data. vasm outputs 0-byte .bin files for these, but xasm++ currently outputs the full ORG range filled with zeros.

### Current State

```
EQ.S:       xasm++ outputs 7592 bytes, vasm outputs 0 bytes
GAMEEQ.S:   xasm++ outputs 3401 bytes, vasm outputs 0 bytes
HRPARAMS.S: xasm++ outputs 2500 bytes, vasm outputs 0 bytes
```

### Root Cause

xasm++ doesn't track whether any actual bytes were emitted during assembly. It always outputs from $0000 to the highest address referenced.

---

## Success Criteria

```
✓ EQ.S produces 0-byte output
✓ GAMEEQ.S produces 0-byte output
✓ HRPARAMS.S produces 0-byte output
✓ Files with actual data still work (BGDATA.bin, HRTABLES.bin remain identical)
✓ All 729 existing tests still pass
✓ Binary compat test added to prevent regression
```

---

## Acceptance Criteria

### Functional Requirements

```
□ Add flag/counter tracking bytes emitted during assembly
□ Check flag after assembly completes
□ Output 0-byte file if no bytes were emitted
□ Preserve existing behavior for files with data
```

### Quality Requirements

```
□ All tests passing (729/729)
□ Binary compat test added
□ No linting errors
□ TDD approach (test first, then implement)
```

---

## Constraints and Dependencies

### Constraints

```
□ Must not break existing functionality
□ Must preserve exact output for files that already work
□ Should be simple flag check, not complex analysis
```

### Dependencies

```
□ None - this is Phase 1 (foundational)
```

### Out of Scope

```
✗ ASC high-bit mode (Phase 2)
✗ Padding removal (Phase 3)
✗ Other binary compatibility issues
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 3-4
- Lines of code estimate: ~50-100
- New concepts/patterns: Simple flag tracking
- Integration complexity: Low
- Risk level: Low

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 3-4 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 4 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
4 × 2,000 tokens = 8,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
└─ Your Task: 8K tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
Planned WIP: 1 agent (sequential phases)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

---

## Resources and References

### Relevant Files

```
- src/syntax/merlin/merlin_syntax.cpp - ASC directive and syntax handling
- include/xasm++/syntax/merlin_syntax.h - MerlinSyntax class
- tests/unit/test_binary_compat.cpp - New binary compat tests (to create)
- tests/e2e/apple2/prince_of_persia/src/EQ.S - Test input (equate-only)
```

### Documentation

```
- .ai/tasks/2026-01-31_binary-compatibility/00-contract.md - Parent task
- .ai/tasks/2026-01-31_binary-compatibility/10-plan.md - Overall plan
```

---

## Assumptions

```
1. Equate-only detection can be done with simple flag tracking
2. No bytes emitted = output 0-byte file
3. Existing binary output logic can be conditionally skipped
```

---

## Risk Assessment

### Identified Risks

```
1. False positives (detecting data files as equate-only)
   - Probability: Low
   - Impact: High (breaks working files)
   - Mitigation: Test with BGDATA.bin, HRTABLES.bin to ensure they still output

2. Breaking existing tests
   - Probability: Low
   - Impact: High (blocks completion)
   - Mitigation: Run full test suite, verify 729/729 pass
```

---

## Notes

- Part of 3-phase binary compatibility work
- Phase 1 must complete before Phase 2 can start
- Simple, focused scope (just equate detection)
- Low risk, high value (fixes 3 files immediately)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
