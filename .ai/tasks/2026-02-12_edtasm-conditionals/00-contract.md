# Task Contract

**Task ID:** 2026-02-12_edtasm-conditionals
**Beads Task:** xasm++-0x4e
**Created:** 2026-02-12
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement 13 EDTASM-M80++ conditional assembly directives for conditional code inclusion during assembly.

### Background and Context

Conditional assembly is fundamental for assemblers - allowing code to be conditionally included based on symbol definitions, expressions, and assembly pass. EDTASM-M80++ supports 13 conditional directives similar to C preprocessor conditionals. The implementation should support nested conditionals and track state per assembly pass.

### Current State

- No conditional directives implemented
- Conditional assembly infrastructure needed
- Similar implementation exists in Merlin syntax (IF/ELSE/ENDIF)

### Desired State

13 conditional directives fully implemented:
- Symbol-based: IFDEF, IFNDEF
- Comparison: IFEQ, IFNE, IFLT, IFGT, IFLE, IFGE
- Pass-based: IF1, IF2
- Text-based: IFB, IFNB, IFIDN, IFDIF

---

## Success Criteria

```
✓ All 13 conditional directives implemented
✓ Nested conditionals supported
✓ State tracked per assembly pass
✓ Expression evaluation for comparison operators
✓ All tests passing (maintain 1425/1426 minimum)
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ IFDEF/IFNDEF check symbol definition
□ IFEQ/IFNE/IFLT/IFGT/IFLE/IFGE evaluate expressions
□ IF1/IF2 check assembly pass
□ IFB/IFNB check blank operands
□ IFIDN/IFDIF compare strings
□ Nested conditionals work correctly
□ ELSE clause supported
□ ENDIF properly closes blocks
```

### Quality Requirements
```
□ TDD process followed
□ Comprehensive tests for all directives
□ Tests for nested conditionals
□ No reduction in test pass rate
□ Zero new compiler warnings
```

---

## Technical Approach

**Files to Create/Modify:**
- Conditional evaluation engine
- 13 directive handlers
- Directive registration
- Comprehensive tests

**Estimated:** 6-8 hours, 3-4 files, ~18K tokens

**Reference:** Similar to Merlin IF/ELSE/ENDIF (already implemented)

---

## Related Documents

- **Reference:** docs/planning/not-yet-implemented-items.md (Section 1)
- **Similar Implementation:** src/syntax/merlin_syntax.cpp (conditional handling)
