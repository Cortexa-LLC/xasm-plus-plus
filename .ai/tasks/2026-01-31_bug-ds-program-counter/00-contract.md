# Task Contract

**Task ID:** 2026-01-31_bug-ds-program-counter
**Created:** 2026-01-31
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Beads Task:** xasm++-g3g

---

## Task Description

Fix DS directive parser to correctly handle `*` (program counter symbol) in expressions.

### Background and Context

The Merlin assembler syntax uses `*` as the program counter symbol (equivalent to `.` in other assemblers). Expressions like `ds $900-*` mean "reserve space from current address to $900".

Currently, the parser incorrectly treats `*` as a multiplication operator, causing it to fail when parsing expressions like `$900-*`.

### Current State

Parser error on 3 POP files (BOOT.S:89, FRAMEDEF.S:275, TABLES.S:51):
```
Parse error: DS: Multiplication requires operands on both sides
```

**Root cause:**
- `src/syntax/merlin/merlin_syntax.cpp:475` checks `if (op.find('*') != std::string::npos)`
- Treats ANY asterisk as multiplication operator
- Splits `$900-*` on `*`: left=`$900-`, right=`` (empty)
- Throws error because right side is empty

### Desired State

- `ds $900-*` correctly evaluates to ($900 - current_address)
- `ds *+10` correctly evaluates to (current_address + 10)
- `ds 10*20` still works as multiplication
- All 3 affected POP files assemble successfully

---

## Success Criteria

```
✓ BOOT.S assembles without error
✓ FRAMEDEF.S assembles without error
✓ TABLES.S assembles without error
✓ All existing tests pass
✓ New tests added for * program counter usage
```

---

## Acceptance Criteria

### Functional Requirements
```
□ HandleDS function recognizes * as program counter symbol
□ Expressions with * evaluate correctly: $900-*, *+10, etc.
□ Multiplication still works: 10*20, 5*8
□ current_address_ is substituted for * in expressions
□ Arithmetic operators work after substitution: -, +
```

### Quality Requirements
```
□ All tests passing
□ Code coverage maintained
□ No linting errors
□ Code follows existing patterns
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain binary compatibility with vasm6502_merlin
□ Must not break existing DS directive functionality
□ Must handle all arithmetic operators: +, -, *
```

### Dependencies
```
□ Requires current_address_ member variable (already exists)
□ Depends on ParseNumber() function (already exists)
```

### Out of Scope
```
✗ Complex expression parsing (parentheses, etc.)
✗ Other directives using *
✗ Division or modulo operators
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 1 (merlin_syntax.cpp)
- Lines of code estimate: ~20 lines
- New concepts/patterns: No (similar to existing expression parsing)
- Integration complexity: Low
- Risk level: Low

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 1 file (merlin_syntax.cpp) + 1 test file

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

Planned WIP: 1 agent
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

---

## Resources and References

### Relevant Files
```
- src/syntax/merlin/merlin_syntax.cpp - HandleDS function (lines 463-539)
- include/xasm++/syntax/merlin_syntax.h - current_address_ member
- tests/unit/test_merlin_syntax.cpp - Add tests here
```

### Test Files
```
- BOOT.S line 89: ds $900-*
- FRAMEDEF.S line 275: ds $900-*
- TABLES.S line 51: ds $900-*
```

### Examples
```
Vasm behavior:
  ds $900-*  → Reserves space from current address to $900
  ds *+10    → Reserves (current address + 10) bytes
  ds 10*20   → Reserves 200 bytes (multiplication)
```

---

## Assumptions

```
1. current_address_ accurately reflects program counter during parsing
2. ParseNumber() correctly handles hex values like $900
3. Symbol table lookup works for symbol references
```

---

## Risk Assessment

### Identified Risks
```
1. Breaking multiplication functionality
   - Probability: Medium
   - Impact: High
   - Mitigation: Add test for multiplication first (TDD)

2. Incorrect address calculation
   - Probability: Low
   - Impact: High
   - Mitigation: Verify against vasm output
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-01-31)
- [ ] Agent: Engineer [Date]

---

## Notes

This is one of 3 critical bugs blocking full POP assembly. Priority P1.

**Related bugs:**
- xasm++-8uc (P0): stoul conversion - ✅ FIXED
- xasm++-ly2 (P2): DO/FIN tracking - Still open

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-31
