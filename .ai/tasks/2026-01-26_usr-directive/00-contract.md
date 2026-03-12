# Task Contract

**Task ID:** 2026-01-26_usr-directive
**Created:** 2026-01-26
**Beads Task:** xasm++-7gc
**Requestor:** PoP Compatibility Project
**Assigned Role:** Engineer
**Workflow:** Standard/Feature

---

## Task Description

Implement the USR directive for Merlin assembler syntax.

### Background and Context

The USR directive is used in Merlin assembly to call external subroutines (typically 6502 machine language routines loaded separately). The Prince of Persia source code uses this directive extensively to call graphics, sound, and game logic routines.

**Current Blocker:** 13 out of 29 PoP files (45%) fail with "Unknown instruction: USR" errors:
- AUTO.S, COLL.S, CTRL.S, CTRLSUBS.S, FRAMEADV.S
- GAMEBG.S, HIRES.S, MASTER.S, MISC.S, MOVER.S
- SUBS.S, TOPCTRL.S, VERSION.S

### Current State

- xasm++ recognizes Merlin syntax for most directives
- USR directive not implemented
- Files with USR fail to assemble

### Desired State

- USR directive recognized and processed
- Generates JSR (Jump to Subroutine) instruction with target address
- 13 additional PoP files assemble successfully
- All tests passing

---

## Success Criteria

```
✓ USR directive implemented and working
✓ Generates JSR opcode (0x20) + 16-bit address
✓ Supports immediate addresses: USR $C000
✓ Supports label references: USR MLRoutine
✓ All existing tests still pass (425/425)
✓ New tests for USR directive pass
✓ At least 13 PoP files now assemble (up from 7/29)
```

---

## Acceptance Criteria

### Functional Requirements
- [ ] USR directive recognized by Merlin syntax parser
- [ ] Generates JSR absolute addressing (opcode 0x20)
- [ ] Supports hexadecimal addresses (USR $C000)
- [ ] Supports decimal addresses (USR 49152)
- [ ] Supports label references (USR MyRoutine)
- [ ] Proper error handling for invalid addresses
- [ ] Address resolution via symbol table

### Quality Requirements
- [ ] TDD process followed (RED-GREEN-REFACTOR)
- [ ] Unit tests for USR directive parsing
- [ ] Integration tests with PoP files
- [ ] All 425 existing tests still pass
- [ ] Zero compiler warnings
- [ ] Code follows xasm++ patterns

### Validation Requirements
- [ ] Tester agent validates tests
- [ ] Reviewer agent validates code quality
- [ ] At least 13 PoP files assemble successfully

---

## Technical Specification

### Merlin USR Syntax

```asm
USR <address>
```

**Where:**
- `<address>` = absolute address (hex $NNNN, decimal NNNNN, or label)

**Encoding:**
```
JSR absolute: 0x20 <low-byte> <high-byte>
```

**Examples:**
```asm
USR $C000      ; JSR $C000 → 20 00 C0
USR 49152      ; JSR $C000 → 20 00 C0
USR DRAW       ; JSR <DRAW address>
```

---

## Implementation Approach

### Phase 1: Add Directive Recognition
1. Update `merlin_syntax.cpp` to recognize "USR" directive
2. Parse operand (address or label)

### Phase 2: Generate JSR Instruction
1. Resolve address from operand (immediate or symbol lookup)
2. Emit JSR opcode (0x20) + 16-bit address (little-endian)

### Phase 3: Testing
1. Unit tests: Parse USR directive with various operands
2. Unit tests: Generate correct JSR encoding
3. Integration test: PoP file with USR directive
4. Regression: All 425 existing tests still pass

---

## Dependencies

**Required:**
- Merlin syntax parser (`src/syntax/merlin/merlin_syntax.cpp`)
- Symbol table for label resolution
- Instruction encoder

**Blocks:**
- PoP compatibility milestone (13 files waiting)
- Phase C validation (cannot complete until all files assemble)

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Symbol resolution timing | Medium | USR uses existing symbol resolution, should be fine |
| Address range validation | Low | Validate 16-bit range (0x0000-0xFFFF) |
| Test coverage | Low | Follow TDD strictly |

---

## Reference

- **Merlin Manual:** USR directive documentation
- **6502 Reference:** JSR absolute (0x20) opcode specification
- **PoP Files:** Search for "USR" patterns to understand usage

---

## Stakeholders

- **PoP Compatibility Project:** Needs all 29 files assembling
- **xasm++ Users:** Future Merlin compatibility

---

## Estimated Effort

- Implementation: 30-45 minutes (straightforward directive)
- Testing: 15-20 minutes
- Validation (Tester/Reviewer): 15-30 minutes
- **Total:** ~1-2 hours

---

## Notes

- USR is functionally equivalent to JSR - it's just a Merlin-specific syntax
- This is similar to the branch relaxation work - delegate encoding to existing mechanisms
- After USR, next blockers are: 65816 mode (2 files), stoul errors (4 files), DS forward refs (3 files)
