# Task Contract: Z80 Phase 11 - Data Directives (DB/DW/DS)

**Task ID:** 2026-02-10_z80-phase11-data-directives
**Beads Task:** xasm++-jli3
**Created:** 2026-02-10
**Requestor:** User (discovered during Phase 10 integration testing)
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement fundamental data directives (DB/DW/DS) for EDTASM-M80++ parser with all variant names, string literal support, and escape sequences.

### Background and Context

During Phase 10 integration testing, it was discovered that DB/DW/DS data directives are not implemented. These are **fundamental Z80 directives** used extensively in real-world code:
- DB (Define Byte) - byte data
- DW (Define Word) - word data
- DS (Define Storage) - reserve space

The parser header claims support but implementation is missing. This blocks integration testing and prevents assembly of real Z80 code.

### Current State

- Parser exists with all other features (Phases 1-9) complete
- DB/DW/DS directives not registered in directive_registry_
- String literal parsing not implemented
- Integration tests failing due to missing directives

### Desired State

- DB/DW/DS directives fully implemented with all variant names
- String literal support (single/double quotes)
- Escape sequences (\n, \r, \t, \\, \', \")
- Mixed operands: `DB 'Hello',13,10,0`
- All integration tests passing

---

## Success Criteria

```
✓ All DB variants working (DB, DEFB, BYTE, DM, DEFM, TEXT, ASCII)
✓ All DW variants working (DW, DEFW, WORD)
✓ All DS variants working (DS, DEFS, DSPACE, BLOCK)
✓ String literals working (single/double quotes)
✓ Escape sequences working
✓ Mixed operands working
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ DB directive with byte values: DB $48,$65
□ DB directive with strings: DB 'Hello'
□ DB directive with mixed: DB 'Hi',13,10,0
□ DB aliases: DEFB, BYTE, DM, DEFM, TEXT, ASCII
□ DW directive with word values: DW $1234,$5678
□ DW aliases: DEFW, WORD
□ DS directive reserves space: DS 100
□ DS aliases: DEFS, DSPACE, BLOCK
□ Single quotes: DB 'text'
□ Double quotes: DB "text"
□ Escape sequences: DB '\n','\t','\\','\''
□ Empty strings handled: DB ''
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

**Complexity:** Medium

**Rationale:**
- Number of files: 3-4 (parser, tests, maybe string utils)
- Lines of code: ~400-600 LOC
- New patterns: String parsing, escape sequences
- Integration: Medium (extends directive system)
- Risk: Medium (string parsing can be tricky)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 4 files

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
4 × 3,000 tokens = 12,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 12,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (blocks Phase 10)
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
- src/syntax/scmasm/scmasm_syntax.cpp - Reference: ParseString() method
```

### Documentation
```
- M80 Assembler Manual - DB/DW/DS syntax
- ZMAC documentation - Data directive variants
- Z80ASM reference - String literal syntax
```

### Reference Implementation
```
- SCM ASM has ParseString() in src/syntax/scmasm/scmasm_syntax.cpp
- Can adapt for Z80 DB directive
- Handles any delimiter character
```

---

## Dependencies

**Depends on:**
- Nothing (can start immediately)

**Blocks:**
- Phase 10: Integration & Polish (xasm++-plrm)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
