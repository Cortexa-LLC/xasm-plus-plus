# Task Contract: Fix A2oSX Critical Bugs

**Task ID:** 2026-02-13_a2osx-critical-bugs
**Beads Task:** xasm++-zmna
**Created:** 2026-02-13
**Requestor:** A2oSX Validation Results
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix 3 critical bugs blocking A2oSX assembly: symbol resolution failure (P0), .BS directive misimplementation (P1), and missing expression operators (P1).

### Background and Context

A2oSX validation testing revealed that xasm++ cannot successfully assemble A2oSX Operating System source files (190 files) due to fundamental issues with symbol handling, directive implementation, and expression parsing. These bugs prevent xasm++ from being production-ready for A2oSX.

**Discovery:** A2oSX validation test suite (tests/integration/a2osx/)
**Impact:** Blocks 99%+ of A2oSX assembly
**Reference:** BUG-001.md, BUG-002.md, BUG-003.md

### Current State

1. **Symbol Resolution**: Symbols defined with .EQ work in immediate mode (#VALUE) but fail in all other addressing modes
2. **.BS Directive**: Treats argument as binary digits instead of byte count for space reservation
3. **Expression Operators**: Missing <, >, +, - operators essential for address manipulation

### Desired State

All three bugs fixed:
- Symbols work in all addressing modes
- .BS correctly reserves N bytes
- Expression operators <, >, +, - fully functional
- A2oSX validation tests pass (at least 6/6 blocked tests)

---

## Success Criteria

```
✓ BUG-001 fixed: Symbols resolve in all addressing modes
✓ BUG-002 fixed: .BS reserves N bytes (not binary parsing)
✓ BUG-003 fixed: Expression operators <, >, +, - working
✓ A2oSX validation tests passing (11/11 or 5/5 + 6/6)
✓ All existing 1564 tests still passing
✓ Zero regressions introduced
```

---

## Acceptance Criteria

### Functional Requirements

**BUG-001: Symbol Resolution (P0 CRITICAL)**
```
□ Symbols defined with .EQ resolve in zero page addressing
□ Symbols defined with .EQ resolve in absolute addressing
□ Symbols defined with .EQ resolve in indexed addressing (,X and ,Y)
□ Symbols defined with .EQ resolve in indirect addressing
□ Test case: test_symbols.S passes
```

**BUG-002: .BS Directive (P1 HIGH)**
```
□ .BS N reserves N bytes of space
□ .BS accepts decimal numbers (e.g., .BS 256)
□ .BS accepts hex numbers (e.g., .BS $100)
□ .BS no longer treats argument as binary
□ Test case: test_bs.S passes
```

**BUG-003: Expression Support (P1 HIGH)**
```
□ < operator extracts low byte of 16-bit address
□ > operator extracts high byte of 16-bit address
□ + operator performs addition (for offsets)
□ - operator performs subtraction
□ Test cases: test_expr_lo.S, test_expr_hi.S, test_expr_offset.S pass
```

### Quality Requirements
```
□ All existing 1564 tests passing
□ New tests added for each bug fix
□ Code coverage maintained or improved
□ Zero new compiler warnings
□ Zero regressions in other SCMASM features
```

### Documentation
```
□ Bug fix documented in commit messages
□ Complex changes have code comments
□ A2oSX validation README updated with success status
```

---

## Constraints and Dependencies

### Constraints
```
□ Must not break existing SCMASM functionality
□ Must maintain backward compatibility
□ Symbol resolution must work with all CPU variants
□ Expression operators must follow standard 6502 conventions
```

### Dependencies
```
✅ A2oSX validation test suite exists (tests/integration/a2osx/)
✅ Bug reports with minimal reproductions available
✅ All SCMASM directives implemented (100% coverage)
□ Understanding of symbol table implementation
□ Understanding of expression parser
□ Understanding of directive handlers
```

### Out of Scope
```
✗ Binary-exact matching with Merlin assembler
✗ Full A2oSX OS build validation (separate follow-up task)
✗ Performance optimization
✗ Other expression operators beyond <, >, +, -
✗ Advanced macro features
```

---

## Estimated Complexity

**Complexity:** Medium-Large

**Rationale:**
- Number of files affected: ~7-10 files
  - Symbol table/operand parsing: 2-3 files
  - .BS directive handler: 1-2 files
  - Expression parser: 2-3 files
  - Tests: 2-3 files
- Lines of code estimate: ~300-500
- New concepts/patterns: No (uses existing infrastructure)
- Integration complexity: Medium (touches core parsing logic)
- Risk level: Medium (affects fundamental assembly behavior)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 7-10 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 7-10 files → ⚠️ ACCEPTABLE (requires justification)
```

### Batch Size Justification

Files: 7-10 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all 3 bugs affect symbol/expression parsing subsystem
- Interdependencies - BUG-003 (expressions) affects BUG-001 (symbols)
- Single concern - all relate to operand parsing and symbol resolution
- Already minimal viable batch - splitting would create artificial boundaries

**Contingency for token limits:**
- If token limit hit, will decompose into:
  - Subtask 1: BUG-001 (symbol resolution) - MUST fix first
  - Subtask 2: BUG-002 + BUG-003 (directives + expressions)

**Estimated tokens:** ~10 × 3,000 = 30,000 tokens
**Status:** Within 25K-32K limit? Marginal (30K is at upper limit)

**Risk mitigation:** Monitor token usage, decompose if approaching 25K

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

Your Task: 30,000 tokens → ❌ HIGH RISK
```

**⚠️ WARNING: Token budget risk**

**REQUIRED ACTION:** Consider decomposition if agent struggles

**Recommended approach:**
1. Attempt as single task (bugs are tightly coupled)
2. If agent hits token limit, decompose into:
   - Phase 1: BUG-001 only (P0 critical)
   - Phase 2: BUG-002 + BUG-003 (P1 high)

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent → ✅ IDEAL
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget marginal AND WIP ≤3
- [ ] NO - MUST decompose

**Justification:**
- Bugs are tightly coupled (all affect operand/symbol parsing)
- Splitting would create artificial boundaries
- Can decompose mid-flight if needed
- Estimated 30K tokens is at upper limit but acceptable for first attempt

**Contingency:** If token limit hit, decompose into BUG-001 (Phase 1) + BUG-002/003 (Phase 2)

---

## Resources and References

### Relevant Files

**Symbol Resolution (BUG-001):**
```
- src/symbol_table.cpp - Symbol storage and lookup
- src/syntax/scmasm_syntax.cpp - Operand parsing
- src/expression_parser.cpp - Expression evaluation (if used)
```

**BS Directive (BUG-002):**
```
- src/syntax/scmasm_directive_handlers.cpp - HandleBs function
```

**Expression Operators (BUG-003):**
```
- src/expression_parser.cpp - Expression parsing
- include/xasm++/expression_parser.h - Parser interface
```

**Tests:**
```
- tests/integration/a2osx/test_symbols.S
- tests/integration/a2osx/test_bs.S
- tests/integration/a2osx/test_expr_*.S
```

### Documentation
```
- tests/integration/a2osx/BUG-001-symbol-addressing.md - Detailed analysis
- tests/integration/a2osx/BUG-002-bs-directive.md - Detailed analysis
- tests/integration/a2osx/BUG-003-expression-support.md - Detailed analysis
- tests/integration/a2osx/NEXT_STEPS.md - Implementation guide
```

### Examples
```
- Merlin syntax (similar): src/syntax/merlin_syntax.cpp
- Expression parsing (reference): existing operators (+, *, /, etc.)
```

---

## Assumptions

```
1. Symbol table infrastructure exists and is functional
2. Expression parser exists with basic operator support
3. .BS directive handler exists (just needs correction)
4. A2oSX validation tests accurately represent required behavior
5. Fixing these 3 bugs will enable >95% of A2oSX assembly
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Token limit exceeded during implementation
   - Probability: Medium
   - Impact: High
   - Mitigation: Decompose into Phase 1 (BUG-001) + Phase 2 (BUG-002/003)

2. Symbol resolution more complex than expected
   - Probability: Medium
   - Impact: High
   - Mitigation: Focus on BUG-001 first, document complexity

3. Expression operators affect other parsers
   - Probability: Low
   - Impact: Medium
   - Mitigation: Thorough testing of all syntax parsers

4. .BS fix breaks existing usage
   - Probability: Low
   - Impact: Low
   - Mitigation: Check for existing .BS usage in test suite

5. Interdependencies between bugs
   - Probability: High
   - Impact: Medium
   - Mitigation: Fix in order (BUG-001, BUG-002, BUG-003)
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Orchestrator [2026-02-13]
- [ ] Engineer [TBD]

**Changes to Contract:**
[None yet]

---

## Notes

- **Priority Order:** Fix BUG-001 first (blocks everything), then BUG-002/003
- **Test-Driven:** Use existing test_*.S files from validation suite
- **Incremental:** Verify each bug fix before proceeding to next
- **Token Budget:** Monitor closely, decompose if approaching 25K tokens

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-13
