# Task Contract

**Task ID:** 2026-02-15_scmasm-inline-comments
**Beads Task:** xasm++-ijxa
**Created:** 2026-02-15
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix S-C Macro Assembler (scmasm) syntax parser to support inline comments after directives and instructions.

### Background and Context

The A2osX operating system uses S-C Macro Assembler syntax with inline comments throughout the source code. xasm++'s scmasm parser currently fails to parse these files because it treats inline comments as part of the expression/operand, causing parse errors.

Example from A2osX.S.txt line 17:
```assembly
pRWReg			.EQ 0					x.printf.s
```

The parser sees `x.printf.s` as part of the expression after `0`, when it should be treated as a comment.

### Current State

- scmasm parser supports line comments starting with `*`
- Inline comments (after directives/instructions) are NOT supported
- A2osX source files fail to assemble with error: "Unexpected character after expression"

### Desired State

- scmasm parser handles inline comments correctly
- A2osX source files assemble successfully
- S-C Macro Assembler compatibility improved

---

## Success Criteria

```
✓ A2osX.S.txt assembles without parse errors
✓ All existing scmasm tests still pass
✓ New tests added for inline comment scenarios
✓ Code coverage ≥ 85%
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Inline comments supported after .EQ directives
□ Inline comments supported after instructions
□ Inline comments supported after all directives
□ Comments can contain any characters (dots, slashes, etc.)
□ Whitespace before comment is handled correctly
```

### Quality Requirements
```
□ All existing tests passing (1538/1538)
□ New unit tests for inline comments (minimum 5 test cases)
□ Code coverage ≥ 85%
□ No regressions in existing scmasm functionality
□ Documentation updated
```

### Non-Functional Requirements
```
□ Parser performance not degraded
□ Error messages remain clear and helpful
□ Consistent with S-C Macro Assembler behavior
```

---

## Constraints and Dependencies

### Constraints
- Must maintain backward compatibility with existing scmasm code
- Must not break Merlin or other syntax parsers
- Parser changes should be minimal and localized

### Dependencies
- Requires understanding of scmasm_syntax.cpp parser implementation
- May need to update scmasm_directive_registry if directives handle comments specially

### Related Files
- `src/syntax/scmasm_syntax.cpp` - Main parser
- `src/syntax/scmasm_syntax.h` - Parser interface
- `tests/unit/test_scmasm_syntax.cpp` - Unit tests
- `src/syntax/scmasm_directive_registry.cpp` - Directive handlers

---

## Test Cases

### Test Case 1: .EQ with inline comment
```assembly
SYMBOL  .EQ $1234  This is a comment
```
Expected: SYMBOL = 0x1234

### Test Case 2: Instruction with inline comment
```assembly
  LDA #$00  Load zero into accumulator
```
Expected: Assembles to opcode A9 00

### Test Case 3: Comment with dots and slashes
```assembly
pRWReg  .EQ 0  x.printf.s
```
Expected: pRWReg = 0

### Test Case 4: Multiple spaces before comment
```assembly
VALUE   .EQ 42       comment here
```
Expected: VALUE = 42

### Test Case 5: Empty comment
```assembly
DATA    .EQ $FF
```
Expected: DATA = 0xFF (works today, should continue working)

---

## Technical Notes

### S-C Macro Assembler Comment Rules
- `*` at start of line = line comment
- Whitespace after operand = start of inline comment
- Comments extend to end of line
- Comments can contain any characters

### Implementation Approach
1. Identify where expression/operand parsing ends
2. After operand is parsed, skip whitespace
3. Treat remaining characters on line as comment
4. Do NOT try to parse comment content

---

## Stakeholders

- **Primary:** User (building A2osX with xasm++)
- **Secondary:** Anyone using scmasm syntax with inline comments

---

## Estimated Effort

- Investigation: 30 minutes
- Implementation: 1-2 hours
- Testing: 1 hour
- **Total:** 2.5-3.5 hours

---

## References

- A2osX source: ~/Projects/Vintage/Apple/A2osX/
- S-C Macro Assembler documentation: .ai-pack/docs/scmasm-spec.md (if exists)
- Related issue: xasm++-ijxa (Beads)
