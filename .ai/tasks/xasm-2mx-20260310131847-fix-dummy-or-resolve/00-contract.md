# Contract: Fix .DUMMY/.OR Zero-Page Address Resolution

## Problem
Labels defined inside a SCMASM `.DUMMY`/`.OR` block resolve to the main section
address ($2000) instead of the correct ZP address (e.g., $E0). This causes every
binary that uses ZP variables to be wrong — 116 of 118 A2osX files are affected.

## Root Cause (from comparison report xasm-9wz)
`HandleOr()` in dummy section mode updates `*context.current_address = $E0` at
parse time. However, `ResolveSymbols()` re-walks the atom list and never sees the
$E0 origin (no OrgAtom is emitted in dummy mode), so it assigns all labels inside
the dummy section to the main-section address.

## SCMASM .DUMMY Semantics
```asm
.OR $2000          ; main section starts at $2000
.DUMMY             ; begin dummy (no-emit) section
.OR $E0            ; ZP origin — labels defined here get ZP addresses
ZS.START
ZPPtr1  .BS 2      ; ZPPtr1 = $E0, ArgIndex = $E2
ArgIndex .BS 1
ZS.END  .ED        ; end dummy; PC restored to main section ($2000)
```
After .ED: ZS.START=$E0, ZPPtr1=$E0, ArgIndex=$E2, ZS.END=$E3.
Main section PC is still $2000 — zero bytes emitted.

## Required Fix
In `ScmasmSyntaxParser::HandleOr()` (dummy section branch), emit a `DummyOrgAtom`
(or equivalent mechanism) that:
1. Is NOT processed by the code emitter (emits zero bytes, doesn't change output PC)
2. IS processed by `ResolveSymbols()` to update the address counter during resolve

This ensures ZP label addresses are correct when instructions are encoded.

## Key Files
- `src/syntax/scmasm_syntax.cpp` — HandleOr(), HandleDummy(), HandleEd()
- `src/syntax/scmasm_syntax.h` — atom types, dummy section state
- `src/core/assembler.cpp` — ResolveSymbols() pass
- `tests/unit/test_scmasm_syntax.cpp` — add regression tests

## Acceptance Criteria
- [ ] `cmake --build build` exits 0
- [ ] `ctest --output-on-failure` exits 0 (all tests pass)
- [ ] New test: .DUMMY/.OR labels get correct ZP addresses
- [ ] New test: Main section PC unchanged after .ED
- [ ] New test: .BS inside .DUMMY advances ZP address correctly
- [ ] A2osX kernel (`sys/kernel`) assembles and ZPPtr1=$E0 (not $2000)

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
