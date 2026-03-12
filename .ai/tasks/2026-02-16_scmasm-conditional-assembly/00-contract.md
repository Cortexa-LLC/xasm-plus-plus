# Task Contract: SCMASM Conditional Assembly

**Task ID:** 2026-02-16_scmasm-conditional-assembly
**Beads Task:** xasm++-vuec
**Priority:** P0 (Critical - blocks A2osX validation)
**Type:** Feature Implementation

---

## Objective

Implement conditional assembly directives (.DO/.ELSE/.FIN) for S-C Macro Assembler (scmasm) syntax to enable assembly of A2osX.S.txt and other vintage Apple II source code.

---

## Requirements

### Functional Requirements

1. **Implement .DO directive:**
   - Syntax: `.DO <expression>`
   - Evaluates expression at assembly time
   - If expression is non-zero (true), includes following code
   - If expression is zero (false), excludes following code until .ELSE or .FIN
   - Supports complex expressions (comparisons, arithmetic, etc.)

2. **Implement .ELSE directive:**
   - Syntax: `.ELSE`
   - Must be within a .DO/.FIN block
   - Toggles inclusion state (if .DO was true, .ELSE is false, and vice versa)
   - Optional (can have .DO/.FIN without .ELSE)

3. **Implement .FIN directive:**
   - Syntax: `.FIN`
   - Ends the current conditional block
   - Resumes normal assembly
   - Must balance with preceding .DO

4. **Support nested conditionals:**
   - Allow .DO/.FIN blocks within .DO/.FIN blocks
   - Maintain separate state for each nesting level
   - Properly handle .ELSE at correct nesting level

5. **Error handling:**
   - Detect unbalanced conditionals (missing .FIN)
   - Detect .ELSE outside .DO block
   - Detect .FIN without matching .DO
   - Provide clear error messages with SCMASM-specific terminology

### Technical Requirements

1. **Integration approach:**
   - Use existing `ConditionalAssembler` shared component (same as Merlin)
   - Add ConditionalAssembler member to ScmasmSyntaxParser
   - Implement HandleDo(), HandleElse(), HandleFin() methods
   - Integrate with ParseLine() to control code emission

2. **Code emission control:**
   - ParseLine() must check `conditional_.ShouldEmit()` before emitting instructions
   - Directives (.DO, .ELSE, .FIN) always processed regardless of conditional state
   - Other directives and instructions only emitted when ShouldEmit() returns true

3. **Expression evaluation:**
   - Leverage existing EvaluateExpression() infrastructure
   - Support all expression types (constants, symbols, operators, comparisons)

### Quality Requirements

1. **Test coverage:**
   - Unit tests for all three directives
   - Test nested conditionals (2+ levels)
   - Test .ELSE behavior (both true and false DO)
   - Test error conditions (unbalanced, misplaced directives)
   - Test complex expressions
   - Target: ≥80% code coverage

2. **Code quality:**
   - Follow existing code style
   - Use SCMASM-specific terminology in error messages
   - Maintain consistency with Merlin implementation approach

3. **Documentation:**
   - Update work log with implementation details
   - Document any deviations from Merlin behavior
   - Clear comments in complex logic

---

## Success Criteria

### Acceptance Criteria (ALL MUST PASS)

1. ✅ `.DO <expr>` evaluates expression and conditionally includes code
2. ✅ `.ELSE` toggles conditional state appropriately
3. ✅ `.FIN` ends conditional block correctly
4. ✅ Nested conditionals (2+ levels) work correctly
5. ✅ Unbalanced conditionals detected and reported
6. ✅ All unit tests passing (≥80% coverage)
7. ✅ Build succeeds with zero warnings
8. ⚠️ A2osX.S.txt assembles successfully (if available)

### Verification Tests

1. **Basic conditional inclusion:**
   ```asm
   DEBUG .EQ 1
   .DO DEBUG=1
       lda #42    ; Should be included
   .FIN
   ```

2. **Basic conditional exclusion:**
   ```asm
   DEBUG .EQ 0
   .DO DEBUG=1
       lda #42    ; Should be excluded
   .FIN
   ```

3. **ELSE branch selection:**
   ```asm
   DEBUG .EQ 0
   .DO DEBUG=1
       lda #1     ; Excluded
   .ELSE
       lda #2     ; Included
   .FIN
   ```

4. **Nested conditionals:**
   ```asm
   OUTER .EQ 1
   INNER .EQ 1
   .DO OUTER=1
       lda #1     ; Included
       .DO INNER=1
           lda #2 ; Included
       .FIN
       lda #3     ; Included
   .FIN
   ```

5. **Complex expressions:**
   ```asm
   A .EQ 10
   B .EQ 5
   .DO A>B
       lda #42    ; Included (10 > 5 is true)
   .FIN
   ```

---

## Reference Materials

- **Merlin implementation:** `src/syntax/merlin_directive_handlers.cpp`
- **Merlin syntax parser:** `src/syntax/merlin_syntax.cpp`
- **ConditionalAssembler:** `include/xasm++/common/conditional_assembler.h`
- **Validation task:** `.ai/tasks/2026-02-16_validate-a2osx/30-review.md`
- **A2osX usage:** `inc/macros.i` (uses .DO/.FIN extensively)

---

## Constraints and Assumptions

### Constraints

- Must maintain compatibility with existing scmasm syntax
- Must not break existing directive handling
- Must use existing ConditionalAssembler infrastructure
- Must follow SCMASM-specific terminology (.DO/.FIN, not .IF/.ENDIF)

### Assumptions

- ConditionalAssembler component is correct (used by Merlin)
- Expression evaluation infrastructure is sufficient
- Test infrastructure (CTest, fixtures) is in place

---

## Risks and Mitigations

### Risk: Implementation complexity

- **Impact:** Medium
- **Likelihood:** Low
- **Mitigation:** Reference implementation exists in Merlin, well-understood problem

### Risk: Edge cases in nesting

- **Impact:** Medium
- **Likelihood:** Medium
- **Mitigation:** Comprehensive test suite, careful handling of state stack

### Risk: Expression evaluation issues

- **Impact:** Low
- **Likelihood:** Low
- **Mitigation:** Leverage existing expression evaluator, add specific tests

---

## Timeline

- **Estimated effort:** 2-4 hours
- **Critical path:** Blocks A2osX validation task (xasm++-bq87)
- **Priority:** P0 (implement immediately)

---

## Contract Agreement

This contract defines the scope, requirements, and success criteria for implementing .DO/.ELSE/.FIN conditional assembly directives for scmasm syntax.

**Prepared by:** AI Engineer Agent
**Date:** 2026-02-16
**Status:** ACTIVE

---

**Related Tasks:**
- **Blocks:** xasm++-bq87 (Validate A2osX assembly)
- **Reference:** 2026-02-16_validate-a2osx (identified need for this feature)

---

## Notes

- Implementation approach should mirror Merlin's use of ConditionalAssembler
- Error messages should use SCMASM terminology (.DO/.FIN not .IF/.ENDIF)
- A2osX.S.txt validation depends on this feature being complete and working
