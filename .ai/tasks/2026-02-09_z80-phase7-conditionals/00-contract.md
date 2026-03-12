# Task Contract: Z80 Phase 7 - Conditional Assembly

**Task ID:** 2026-02-09_z80-phase7-conditionals
**Beads Task:** xasm++-nkty
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement conditional assembly directives for EDTASM-M80++ parser to support M80/ZMAC/Z80ASM source code.

### Background and Context

This is Phase 7 of the Z80 parser feature implementation. Phase 6 (Macro System) is complete, and conditionals are the next major feature set needed for M80/ZMAC/Z80ASM compatibility.

Conditional assembly allows portions of source code to be conditionally included or excluded during assembly based on symbol definitions, pass number, or comparison results.

### Current State

EDTASM-M80++ parser exists but lacks conditional assembly support. Code using IF/ELSE/ENDIF will not assemble.

### Desired State

Parser handles all common conditional assembly directives:
- IF/ELSE/ENDIF (expression-based)
- IFDEF/IFNDEF (symbol existence)
- IFEQ/IFNE/IFLT/IFGT/IFLE/IFGE (comparisons)
- IF1/IF2 (pass-based)
- IFB/IFNB (blank/non-blank argument)
- IFIDN/IFDIF (string comparison)
- Nested conditionals (up to reasonable depth)

---

## Success Criteria

```
✓ All conditional directives implemented
✓ Nested conditionals working (depth 8+)
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ IF/ELSE/ENDIF with expression evaluation
□ IFDEF/IFNDEF symbol existence checks
□ IFEQ/IFNE/IFLT/IFGT/IFLE/IFGE comparisons
□ IF1/IF2 pass-based conditionals
□ IFB/IFNB blank argument checks
□ IFIDN/IFDIF string comparison
□ Nested conditionals (8+ levels)
□ Proper error handling for unmatched directives
```

### Quality Requirements
```
□ All tests passing
□ Coverage ≥80%
□ No compilation warnings
□ TDD process followed (RED-GREEN-REFACTOR)
```

---

## Estimated Complexity

**Complexity:** Small-Medium

**Rationale:**
- Number of files: 2-3 (parser implementation, tests)
- Lines of code: ~300-500 LOC
- New patterns: Conditional state stack
- Integration: Low (isolated feature)
- Risk: Low

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

Your Task: 3 files → ✅ IDEAL
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

Planned WIP: 1 agent (sequential execution due to dependencies)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

---

## Resources and References

### Relevant Files
```
- src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp - Parser implementation
- include/xasm++/syntax/edtasm_m80_plusplus_syntax.h - Parser header
- tests/unit/test_edtasm_m80_plusplus_syntax.cpp - Tests
```

### Documentation
```
- M80 Assembler Manual - Conditional assembly directives
- ZMAC documentation - IF/ELSE/ENDIF syntax
- Z80ASM reference - Conditional directives
```

---

## Dependencies

**Depends on:**
- ✅ Phase 6: Macro System (xasm++-7iau) - COMPLETE

**Blocks:**
- Phase 8: Listing Control (xasm++-4bu)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-09

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
