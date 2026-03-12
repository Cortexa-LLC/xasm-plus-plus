# Task Contract

**Task ID:** 2026-02-01_lup-directive
**Created:** 2026-02-01
**Requestor:** Bryan Woodruff (Orchestrator)
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-01b

---

## Task Description

Implement the LUP (loop) directive for Merlin assembler syntax to enable repeated assembly of code blocks.

### Background and Context

The LUP directive is a Merlin assembler loop construct that repeats a block of assembly code N times. It's used extensively in TABLES.S to generate repetitive data tables efficiently.

**Current State:** TABLES.S fails to assemble with error:
```
Parse error: <stdin>:54: error: LUP directive not yet implemented (deferred)
```

**Blocking:** 1 of 29 Prince of Persia source files (TABLES.S)
**Current Progress:** 27/29 (93.1%)
**Target:** 28/29 (96.6%)

### Current State

LUP directive is recognized but not implemented - parser throws "not yet implemented" error.

**TABLES.S usage example (line 51-55):**
```asm
]byte = 0
 lup 36
 db ]byte,]byte,]byte,]byte,]byte,]byte,]byte
]byte = ]byte+1
 --^
```

This should:
1. Set loop variable `]byte` to 0
2. Repeat the DB line 36 times
3. Increment `]byte` after each iteration
4. End loop at `--^` terminator

### Desired State

LUP directive fully implemented:
- `lup N` starts a loop that repeats N times
- Loop body executes with variable substitution
- Variables prefixed with `]` are loop-local
- `--^` terminates the loop
- TABLES.S assembles successfully

---

## Success Criteria

```
✓ LUP directive parses and executes correctly
✓ Loop variables (]name) work properly
✓ Variable assignments evaluated each iteration
✓ --^ terminator ends loop correctly
✓ TABLES.S assembles without errors
✓ All existing tests still pass
✓ Binary compatibility maintained (14/14 reference files)
✓ Assembly success rate: 27/29 → 28/29 (96.6%)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ HandleLup() function implemented in MerlinSyntaxParser
□ Loop state tracked (iteration count, variables)
□ Loop variables (]name) scoped to loop
□ Variable assignments evaluated per iteration
□ --^ terminator recognized and handled
□ Nested loops supported (if needed by TABLES.S)
□ Error handling for malformed loops
```

### Quality Requirements
```
□ All existing tests passing
□ New unit tests for LUP directive
□ Code coverage maintained ≥80%
□ No regressions in binary compatibility
□ No new linting errors
□ Code follows existing patterns
```

### Non-Functional Requirements
```
□ Performance acceptable (negligible impact)
□ Error messages clear and helpful
□ Code maintainable and well-documented
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain backward compatibility
□ Must work within multi-pass assembly architecture
□ Cannot change Merlin syntax parser interface
□ Loop variables must use ] prefix convention
```

### Dependencies
```
□ Requires MerlinSyntaxParser (src/syntax/merlin/merlin_syntax.cpp)
□ May need symbol table extension for loop variables
□ Depends on existing expression evaluation
```

### Out of Scope
```
✗ Other loop directives (DO/FIN handle conditionals, not loops)
✗ Complex nested macro expansion
✗ Loop optimizations
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 2 (merlin_syntax.cpp, merlin_syntax.h)
- Lines of code estimate: ~150 lines (loop state + parsing)
- New concepts: Loop state management, variable scoping
- Integration complexity: Medium (fits into existing directive framework)
- Risk level: Medium (affects assembly flow)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 2-3 files
- src/syntax/merlin/merlin_syntax.cpp (HandleLup implementation)
- include/xasm++/syntax/merlin_syntax.h (declarations)
- tests/unit/test_merlin_syntax.cpp (tests)

**Batch Size:** 2-3 files → ✅ IDEAL

### Token Budget Estimation

```
Files × Average Tokens Per File = Estimated Total
3 × 2,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status: <20K tokens → ✅ SAFE
```

### Work In Progress

**Planned WIP:** 1 agent (Engineer) → ✅ IDEAL

---

## Resources and References

### Relevant Files
```
- src/syntax/merlin/merlin_syntax.cpp - Main parser, add HandleLup()
- include/xasm++/syntax/merlin_syntax.h - Add declarations
- tests/unit/test_merlin_syntax.cpp - Add LUP tests

Test file:
- TABLES.S (lines 51-58, 65-68) - Real-world usage examples
```

### LUP Syntax Reference

**From TABLES.S analysis:**
```asm
]variable = initial_value    ; Loop variable initialization
 lup count                    ; Start loop (count iterations)
 <assembly code>              ; Loop body (can use ]variables)
]variable = expression        ; Variable update each iteration
 --^                          ; Loop terminator
```

**Variables:**
- Variables prefixed with `]` are loop-local
- Updated each iteration
- Available in expressions within loop body

**Terminator:**
- `--^` marks end of loop
- Must appear after loop body
- Loop exits after N iterations

---

## Implementation Approach

### Suggested Strategy

1. **Add loop state to MerlinSyntaxParser:**
   - Track current loop depth
   - Store loop variables (]name → value mapping)
   - Store loop body lines for repetition

2. **HandleLup() implementation:**
   - Parse loop count from operand
   - Collect loop body until `--^` found
   - Execute loop body N times with variable substitution

3. **Variable handling:**
   - Detect `]variable` assignments
   - Store in loop-local symbol table
   - Substitute in expressions during each iteration

4. **Integration:**
   - Add "LUP" directive handler in ParseLine()
   - Add "--^" terminator detection
   - Ensure multi-pass assembly compatibility

---

## Risk Assessment

### Identified Risks

```
1. Breaking existing directive parsing
   - Probability: Low
   - Impact: High
   - Mitigation: TDD approach, comprehensive tests

2. Variable scoping conflicts
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Use separate namespace for ]variables

3. Multi-pass assembly interaction
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Test with forward references in loops
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (Orchestrator) (2026-02-01)
- [ ] Agent: Engineer [Date]

---

## Notes

This is one of the final 2 blockers for 100% Prince of Persia assembly success.

**Related tasks:**
- xasm++-8uc: stoul conversion - ✅ FIXED
- xasm++-g3g: DS program counter - ✅ FIXED
- xasm++-ly2: DO/FIN tracking - ✅ FIXED
- xasm++-4ay: Missing directives - ✅ FIXED
- xasm++-7f4: XC CPU mode - ✅ FIXED
- xasm++-q53: Arithmetic expressions - ✅ FIXED

**Current status:** 27/29 (93.1%)
**After LUP:** 28/29 (96.6%)
**Final goal:** 29/29 (100%)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-01
