# Task Contract: Z80 Phase 3 - Expression Evaluation

**Task ID:** 2026-02-09_z80-phase3-expressions
**Created:** 2026-02-09
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-ek2

---

## Task Description

Implement complete expression evaluation system for Z80 Universal Syntax with arithmetic, bitwise, logical, and comparison operators following C-style precedence.

### Background and Context

Z80 Phase 2 (PUBLIC/EXTERN) is complete. Phase 3 adds full expression evaluation capabilities needed for:
- Complex address calculations
- Conditional assembly (Phase 7)
- Macro parameter expressions (Phase 6)
- Symbolic constants and computed values

### Current State

- Basic number parsing exists
- Symbol table with value lookup
- Limited expression support (numbers + symbols only)
- No operator support beyond simple addition

### Desired State

- Full expression evaluator with operator precedence
- All arithmetic operators: +, -, *, /, % (modulo)
- All bitwise operators: &, |, ^, ~, <<, >>
- All logical operators: &&, ||, !
- All comparison operators: ==, !=, <, >, <=, >=
- Unary operators: HIGH, LOW (byte extraction), -, ~, !
- Parentheses for grouping
- C-style precedence rules

---

## Success Criteria

```
✓ Expression evaluator integrated into Z80 syntax parser
✓ All 15+ operator types supported
✓ C-style operator precedence correct
✓ HIGH/LOW byte extraction working
✓ Comprehensive test suite (30+ tests)
✓ Zero compiler warnings
✓ Documentation complete
```

---

## Acceptance Criteria

### Functional Requirements

**Arithmetic Operators:**
```
□ + (addition)
□ - (subtraction, unary negation)
□ * (multiplication)
□ / (division)
□ % (modulo)
```

**Bitwise Operators:**
```
□ & (AND)
□ | (OR)
□ ^ (XOR)
□ ~ (NOT/complement)
□ << (left shift)
□ >> (right shift)
```

**Logical Operators:**
```
□ && (logical AND)
□ || (logical OR)
□ ! (logical NOT)
```

**Comparison Operators:**
```
□ == (equal)
□ != (not equal)
□ < (less than)
□ > (greater than)
□ <= (less than or equal)
□ >= (greater than or equal)
```

**Unary Operators:**
```
□ HIGH(expr) - Extract high byte
□ LOW(expr) - Extract low byte
□ - (unary negation)
□ ~ (bitwise complement)
□ ! (logical NOT)
```

**Operator Precedence (C-style, highest to lowest):**
```
1. HIGH(), LOW(), unary -, ~, !
2. *, /, %
3. +, -
4. <<, >>
5. <, >, <=, >=
6. ==, !=
7. &
8. ^
9. |
10. &&
11. ||
```

**Grouping:**
```
□ Parentheses () for explicit precedence
```

### Quality Requirements

```
□ All tests passing (30+ tests)
□ Code coverage ≥ 90% for expression evaluator
□ Zero compiler warnings
□ Google C++ Style Guide compliance
□ TDD methodology (RED-GREEN-REFACTOR)
```

---

## Constraints and Dependencies

### Constraints

```
□ Must integrate with existing Z80UniversalSyntaxParser
□ Must not break existing Phase 1/2 functionality
□ Must handle integer arithmetic only (no floating point)
□ Expression values are 32-bit signed integers
```

### Dependencies

```
✓ Z80 Phase 2 (PUBLIC/EXTERN) - COMPLETE
✓ Symbol table with value lookup - EXISTS
✓ Google Test framework - AVAILABLE
```

### Out of Scope

```
✗ Floating point arithmetic
✗ String manipulation
✗ Complex number support
✗ Runtime evaluation (compile-time only)
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 2-3 files
- Lines of code estimate: ~400-600 lines (expression evaluator + tests)
- New concepts/patterns: Recursive descent parser, operator precedence
- Integration complexity: Medium (extends existing parser)
- Risk level: Medium (operator precedence tricky to get right)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 3 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
└─ Your Task: 3 files → ✅ IDEAL
```

### Token Budget Estimation

```
Files × Average Tokens Per File = Estimated Total
3 × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status: <20K tokens → ✅ SAFE
```

### WIP Planning

```
Planned WIP: 1 agent (Engineer)
```

### Decomposition Decision

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

---

## Resources and References

### Relevant Files

```
- include/xasm++/syntax/z80_universal_syntax.h - Parser interface
- src/syntax/z80_universal/z80_universal_syntax.cpp - Parser implementation
- tests/unit/test_z80_universal_syntax.cpp - Test suite
```

### Documentation

```
- C operator precedence reference
- M80/ZMAC/Z80ASM expression syntax documentation
```

---

## Assumptions

```
1. Expression evaluation is compile-time only
2. All values fit in 32-bit signed integers
3. Division by zero should be caught and reported
4. Expressions used in DB, DW, ORG directives and labels
```

---

## Risk Assessment

```
1. Operator precedence complexity
   - Probability: Medium
   - Impact: High (wrong results if precedence incorrect)
   - Mitigation: Comprehensive tests, follow C precedence exactly

2. Integration with existing parser
   - Probability: Low
   - Impact: Medium
   - Mitigation: Use existing ParseExpression() as starting point
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Bryan Woodruff (Orchestrator) 2026-02-09
- [ ] Agent: Engineer [Pending]

---

## Notes

**Implementation Strategy:**

Use recursive descent parser with precedence climbing for clean operator precedence handling.

**Example expressions to support:**
```assembly
LD A, (LABEL + 5)           ; Arithmetic
LD B, HIGH(ADDRESS)         ; HIGH byte extraction
LD C, LOW(ADDRESS)          ; LOW byte extraction
DB (VALUE & $FF)            ; Bitwise AND
DW (START << 8) | END       ; Bitwise shift and OR
ORG $ + ((SIZE + 15) / 16) * 16  ; Complex arithmetic
```

**Key Design Points:**
1. Integrate into existing ParseExpression() method
2. Use operator precedence table
3. Handle unary operators specially (HIGH, LOW, -, ~, !)
4. Return 32-bit signed integer result
5. Report errors for undefined symbols, division by zero

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
