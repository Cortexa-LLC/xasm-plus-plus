# Task Contract: Z80 Phase 9 - Special Features

**Task ID:** 2026-02-09_z80-phase9-special
**Beads Task:** xasm++-aho
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement special features for EDTASM-M80++ parser including radix control, date/time pseudo-ops, and undocumented Z-80 instructions.

### Background and Context

This is Phase 9 of the Z80 parser feature implementation. Phase 8 (Listing Control) must complete first.

Special features include:
- RADIX control for number base (binary, octal, decimal, hex)
- DATE/TIME pseudo-ops for build timestamps
- Undocumented Z-80 instructions (ixh/ixl/iyh/iyl registers, sl1 instruction)
- Optional: Cycle counting pseudo-ops

### Current State

EDTASM-M80++ parser exists but lacks these special features. Code using these features will not assemble correctly.

### Desired State

Parser handles all special features:
- *RADIX / .RADIX for number base control (2-16)
- DATE/TIME pseudo-ops returning current timestamp
- Undocumented Z-80 instructions:
  - ixh, ixl, iyh, iyl register access
  - sl1 (shift left with 1 fill)
- Optional: t, tilo, tihi, ocf cycle counting

---

## Success Criteria

```
✓ RADIX control implemented (bases 2-16)
✓ DATE/TIME pseudo-ops working
✓ Undocumented Z-80 instructions supported
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ *RADIX / .RADIX sets number base (2-16)
□ DATE pseudo-op returns current date string
□ TIME pseudo-op returns current time string
□ Undocumented instructions:
  □ ixh/ixl register access (e.g., LD A,ixh)
  □ iyh/iyl register access (e.g., LD B,iyl)
  □ sl1 instruction (shift left, fill with 1)
□ Optional: Cycle counting (t, tilo, tihi, ocf)
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
- Number of files: 2-3 (parser, CPU extensions, tests)
- Lines of code: ~250-400 LOC
- New patterns: Radix state management, undocumented opcodes
- Integration: Low-Medium (CPU instruction set extension)
- Risk: Low-Medium (undocumented opcodes may vary)

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
- src/cpu/z80/cpu_z80.cpp - CPU instruction set (for undocumented ops)
- tests/unit/test_edtasm_m80_plusplus_syntax.cpp - Tests
```

### Documentation
```
- M80 Assembler Manual - RADIX directive
- Z80 Undocumented Instructions Guide - ixh/ixl/iyh/iyl, sl1
- ZMAC documentation - Special features
- Z80ASM reference - DATE/TIME pseudo-ops
```

---

## Dependencies

**Depends on:**
- Phase 8: Listing Control (xasm++-4bu) - MUST COMPLETE FIRST

**Blocks:**
- Phase 10: Integration & Polish (xasm++-plrm)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-09

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
