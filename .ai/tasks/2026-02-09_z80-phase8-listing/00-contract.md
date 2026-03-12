# Task Contract: Z80 Phase 8 - Listing Control Directives

**Task ID:** 2026-02-09_z80-phase8-listing
**Beads Task:** xasm++-4bu
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement listing control directives for EDTASM-M80++ parser to control assembly listing output.

### Background and Context

This is Phase 8 of the Z80 parser feature implementation. Phase 7 (Conditionals) must complete first.

Listing control directives allow source code to control what appears in assembly listings - titles, subtitles, page breaks, blank lines, and enabling/disabling listing output.

### Current State

EDTASM-M80++ parser exists but lacks listing control directives. These directives are ignored or cause errors.

### Desired State

Parser handles all common listing control directives:
- .LIST/.XLIST - Toggle listing on/off
- *LIST ON/OFF - Z80ASM-style listing control
- .TITLE - Set listing title
- .SUBTTL - Set subtitle
- EJECT - Force page break
- SPACE - Insert blank lines
- NAME - Set module name

---

## Success Criteria

```
✓ All listing control directives implemented
✓ Directives affect listing output appropriately
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ .LIST/.XLIST toggle listing
□ *LIST ON/OFF Z80ASM-style control
□ .TITLE sets listing title
□ .SUBTTL sets subtitle
□ EJECT forces page break in listing
□ SPACE inserts blank lines
□ NAME sets module name
□ Directives integrated with listing output system
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

**Complexity:** Small

**Rationale:**
- Number of files: 2 (parser implementation, tests)
- Lines of code: ~150-250 LOC
- New patterns: Listing state management
- Integration: Low (isolated feature)
- Risk: Low

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 2 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 2 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
2 × 3,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 6,000 tokens → ✅ SAFE
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
- tests/unit/test_edtasm_m80_plusplus_syntax.cpp - Tests
- include/xasm++/output/listing_output.h - Listing output (may need updates)
```

### Documentation
```
- M80 Assembler Manual - Listing control directives
- ZMAC documentation - .LIST/.XLIST syntax
- Z80ASM reference - *LIST directives
```

---

## Dependencies

**Depends on:**
- Phase 7: Conditionals (xasm++-nkty) - MUST COMPLETE FIRST

**Blocks:**
- Phase 9: Special Features (xasm++-aho)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-09

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
