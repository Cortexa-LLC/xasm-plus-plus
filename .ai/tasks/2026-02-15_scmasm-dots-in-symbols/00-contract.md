# Task Contract

**Task ID:** 2026-02-15_scmasm-dots-in-symbols
**Beads Task:** xasm++-786z
**Created:** 2026-02-15
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Support dots (`.`) in symbol names for S-C Macro Assembler compatibility.

### Background and Context

S-C Macro Assembler allows dots in symbol names, which are commonly used for namespace-like organization and hierarchical naming. A2osX uses this extensively for system symbols like `MLI.GETPREFIX`, `A2osX.Init0`, etc.

Example from A2osX.S.txt line 77:
```assembly
A2osX.Init1.128	jsr MLI
				.DA #MLI.GETPREFIX
				.DA MLIGETPREFIX01
```

The expression `#MLI.GETPREFIX` fails with "Unexpected character after expression: ."

### Current State

- Symbol names can only contain alphanumeric and underscore characters
- Dots in expressions trigger parse errors
- A2osX.S.txt fails at line 77
- Expression parser treats `.` as unknown operator

### Desired State

- Symbol names support dots (e.g., `MLI.GETPREFIX`, `A2osX.Init0`)
- Dots in symbol names distinguished from other uses of `.` (directives, current location)
- A2osX.S.txt assembles past line 77
- Compatible with S-C Macro Assembler behavior

---

## Success Criteria

```
✓ A2osX.S.txt assembles past line 77
✓ Symbols with dots defined correctly
✓ Symbol references with dots resolve correctly
✓ Dots in directives still work (.EQ, .OR, etc.)
✓ All existing tests still pass
✓ New tests added for dot symbols
✓ Code coverage ≥ 85%
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Symbol names support dots (e.g., SYS.CALL)
□ Labels with dots defined correctly
□ References to dotted symbols resolve correctly
□ Forward references with dots work
□ Dots in directive names still recognized (.EQ, .DA, etc.)
□ Current location (*) still works
□ Expression parser distinguishes symbol dots from operators
```

### Quality Requirements
```
□ All existing tests passing (1667/1667)
□ New unit tests for dotted symbols (minimum 6 test cases)
□ Code coverage ≥ 85%
□ No regressions in symbol handling
□ Documentation updated
```

### Non-Functional Requirements
```
□ Parser performance not degraded
□ Error messages clear and helpful
□ Consistent with S-C Macro Assembler behavior
```

---

## Constraints and Dependencies

### Constraints
- Dots in directives must continue to work (.EQ, .OR, .PH, etc.)
- Dot as directive prefix must be preserved
- Must not break other syntax parsers (Merlin, etc.)

### Dependencies
- Requires understanding of symbol parsing in expression evaluator
- May need to modify identifier tokenization
- Affects both scmasm_syntax and expression_parser

### Related Files
- `src/core/expression_parser.cpp` - Expression evaluation
- `src/core/expression_parser.h` - Parser interface
- `src/syntax/scmasm_syntax.cpp` - SCMASM parser
- `tests/unit/test_scmasm_syntax.cpp` - Unit tests
- `tests/unit/test_expression_parser.cpp` - Expression tests

---

## Technical Notes

### S-C Macro Assembler Symbol Rules

**Valid symbol names:**
- Start with letter or underscore
- Can contain letters, digits, underscores
- Can contain dots (.) anywhere except first character
- Case-sensitive in modern implementations

**Examples:**
- `MLI.GETPREFIX` ✅
- `A2osX.Init0` ✅
- `SYS.CALL.EXIT` ✅
- `.SYMBOL` ❌ (starts with dot - this is a directive)
- `123.ABC` ❌ (starts with digit)

### Implementation Approach

1. **Modify identifier tokenization**
   - Allow dots in identifier regex/parsing
   - Ensure first character is not a dot (that's a directive)

2. **Update expression parser**
   - When encountering a dot after an identifier, check if it's part of symbol name
   - Distinguish `SYMBOL.NAME` (one identifier) from `SYMBOL .DIRECTIVE` (symbol + directive)

3. **Test edge cases**
   - Symbol with dots vs directive with dots
   - Multiple dots in symbol name
   - Forward references with dots
   - Dot at end of symbol?

4. **Preserve existing behavior**
   - Directives still start with dot
   - Current location (*) still works
   - No breaks in other parsers

---

## Test Cases

### Test Case 1: Simple dotted symbol
```assembly
MLI.CALL .EQ $BF00
  LDA MLI.CALL
```
Expected: Symbol MLI.CALL = $BF00, LDA references it

### Test Case 2: Multiple dots
```assembly
SYS.CALL.EXIT .EQ $FF
  JMP SYS.CALL.EXIT
```
Expected: Symbol with 2 dots defined and referenced

### Test Case 3: Dotted label
```assembly
A2osX.Init0
  NOP
```
Expected: Label A2osX.Init0 defined at current address

### Test Case 4: Forward reference with dot
```assembly
  JMP FUTURE.SYMBOL
FUTURE.SYMBOL
  RTS
```
Expected: Forward reference resolves correctly

### Test Case 5: Directive after dotted symbol
```assembly
SYMBOL.NAME .EQ $1000
```
Expected: Symbol defined, .EQ recognized as directive

### Test Case 6: Expression with dotted symbols
```assembly
VALUE .EQ MLI.CALL + 3
```
Expected: Expression evaluates correctly

---

## Stakeholders

- **Primary:** User (building A2osX with xasm++)
- **Secondary:** Anyone using S-C Macro Assembler dotted symbols

---

## Estimated Effort

- Investigation: 30 minutes (tokenization and parsing)
- Implementation: 2-3 hours
- Testing: 1 hour
- **Total:** 3.5-4.5 hours

---

## References

- A2osX source: ~/Projects/Vintage/Apple/A2osX/
- Related issue: xasm++-786z (Beads)
- Previous: xasm++-ijxa (inline comments), xasm++-6isx (phase directives)
