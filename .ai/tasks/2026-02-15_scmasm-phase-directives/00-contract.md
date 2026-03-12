# Task Contract

**Task ID:** 2026-02-15_scmasm-phase-directives
**Beads Task:** xasm++-6isx
**Created:** 2026-02-15
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Implement S-C Macro Assembler phase directives (`.PH` and `.EP`) for temporary address ranges during assembly.

### Background and Context

Phase directives allow code to be assembled at one address but linked/stored at another. This is essential for position-independent code and runtime relocation. A2osX uses phase directives extensively for code that will be moved to different memory locations at runtime.

Example from A2osX.S.txt line 37:
```assembly
SYSTEM.START	.EQ *
				.PH	$1000
A2osX.Init0		>LDYAI $2000
				...
				.EP
```

The code between `.PH $1000` and `.EP` is assembled with addresses starting at $1000, but the actual bytes are placed at SYSTEM.START in the output file.

### Current State

- `.PH` directive not implemented - parse error
- `.EP` directive not implemented
- A2osX.S.txt fails to assemble at line 37
- No phase addressing support in scmasm parser

### Desired State

- `.PH <address>` implemented - sets temporary assembly address
- `.EP` implemented - restores previous assembly address
- A2osX.S.txt assembles past line 37
- Phase directives work like S-C Macro Assembler

---

## Success Criteria

```
✓ A2osX.S.txt assembles past line 37
✓ .PH directive sets phase address correctly
✓ .EP directive restores original address
✓ Code assembled correctly in phase
✓ All existing scmasm tests still pass
✓ New tests added for phase directives
✓ Code coverage ≥ 85%
```

---

## Acceptance Criteria

### Functional Requirements
```
□ .PH <address> implemented
□ .EP implemented
□ Phase address tracked separately from output address
□ Nested phases NOT supported (error if .PH inside .PH)
□ .EP without .PH produces error
□ Symbol values use phase address when in phase
□ Output bytes use original address
□ * (current location) returns phase address when in phase
```

### Quality Requirements
```
□ All existing tests passing (1538/1538)
□ New unit tests for phase directives (minimum 8 test cases)
□ Code coverage ≥ 85%
□ No regressions in existing scmasm functionality
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
- Must maintain backward compatibility
- Must not break other syntax parsers
- S-C Macro Assembler doesn't support nested phases (error if attempted)

### Dependencies
- Requires understanding of Section address vs phase address
- May need to modify Section class to track phase separately
- Directive handlers in scmasm_directive_registry.cpp

### Related Files
- `src/syntax/scmasm_syntax.cpp` - Parser
- `src/syntax/scmasm_directive_registry.cpp` - Directive handlers
- `src/core/section.h` - Section class (may need phase tracking)
- `tests/unit/test_scmasm_syntax.cpp` - Unit tests

---

## Test Cases

### Test Case 1: Basic phase
```assembly
	.OR $2000
	.PH $1000
	NOP
	.EP
```
Expected: NOP at output offset $2000, but * was $1000 during assembly

### Test Case 2: Symbol in phase
```assembly
	.OR $2000
	.PH $1000
LABEL
	NOP
	.EP
```
Expected: LABEL = $1000

### Test Case 3: Reference to phase symbol
```assembly
	.OR $2000
	.PH $1000
LABEL
	NOP
	.EP
	JMP LABEL
```
Expected: JMP $1000

### Test Case 4: Nested phase error
```assembly
	.OR $2000
	.PH $1000
	.PH $2000
```
Expected: Parse error: Cannot nest .PH directives

### Test Case 5: EP without PH error
```assembly
	.OR $2000
	.EP
```
Expected: Parse error: .EP without matching .PH

### Test Case 6: Current location in phase
```assembly
	.OR $2000
	.PH $1000
	LDA *
	.EP
```
Expected: LDA immediate, operand = $1000

### Test Case 7: Multiple phases
```assembly
	.OR $2000
	.PH $1000
	NOP
	.EP
	.PH $3000
	NOP
	.EP
```
Expected: Both NOPs work correctly

### Test Case 8: Phase with data
```assembly
	.OR $2000
	.PH $1000
	.DA $1234
	.EP
```
Expected: .DA outputs $34 $12 at $2000

---

## Stakeholders

- **Primary:** User (building A2osX with xasm++)
- **Secondary:** Anyone using scmasm phase directives

---

## Estimated Effort

- Investigation: 1 hour
- Implementation: 3-4 hours
- Testing: 1-2 hours
- **Total:** 5-7 hours

---

## References

- A2osX source: ~/Projects/Vintage/Apple/A2osX/
- Related issue: xasm++-6isx (Beads)
- Previous: xasm++-ijxa (inline comments - completed)
