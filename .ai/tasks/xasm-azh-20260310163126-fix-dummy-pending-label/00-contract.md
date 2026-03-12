# Contract: Fix standalone labels in .DUMMY blocks getting wrong address

## Task ID
xasm-azh

## Problem

Standalone label-only lines inside `.DUMMY` blocks get the main-section address instead
of the ZP address set by the `.OR` inside the dummy block.

Example:
```asm
       .OR $2000
       .DUMMY
       .OR $E0
ZS.START           ; standalone label-only line -> gets $2000 (WRONG, should be $E0)
ZPTmpPtr .BS 2     ; label + directive -> correctly gets $E0
ZS.END  .ED
       .DA ZS.START        ; emits $00 $20 (WRONG, should be $E0 $00)
       .DA ZPTmpPtr        ; emits $E0 $00 (correct)
       .DA ZS.END-ZS.START ; emits $E0 $E2 (WRONG, should be $02 $00)
```

## Root Cause

In scmasm_syntax.cpp, when ZS.START (a label-only line) is encountered inside a
.DUMMY block:
1. pending_label_ is set to "ZS.START" and processing returns
2. On the next line (ZPTmpPtr .BS 2), the pending_label_ resolution code (lines
   762-782) defines ZS.START at current_address_ = $E0 AND emits a LabelAtom
3. During ResolveSymbols(), the LabelAtom is encountered and label->address is
   overwritten with the main-section current_address (e.g. $2000)

The same bug exists in the consecutive-label handling block (lines 703-720).

Data-emitting directives already handle this correctly via !in_dummy_section_ guard
(line 826-828): they don't emit LabelAtoms inside dummy sections.

## Fix Required

In src/syntax/scmasm_syntax.cpp:

1. pending_label_ resolution block (lines ~762-782):
   When resolving pending_label_ for a non-.EQ directive, check in_dummy_section_.
   If in_dummy_section_ is true: call symbols.Define() ONLY, do NOT emit LabelAtom.
   Apply to both local-label path and global-label path.

2. Consecutive label handling block (lines ~703-720):
   Same fix: if in_dummy_section_, don't emit LabelAtom.

3. Unit test in tests/unit/test_scmasm_syntax.cpp:
   Add DUMMY_StandaloneLabel_GetsZPAddress test:
   - .OR $2000 / .DUMMY / .OR $E0 / ZS.START / ZPTmpPtr .BS 2 / ZS.END .ED
   - Verify .DA ZS.START emits E0 00
   - Verify .DA ZS.END-ZS.START emits 02 00

## Acceptance Criteria

cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build  (exits 0)
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure  (exits 0)
New test DUMMY_StandaloneLabel_GetsZPAddress passes.
No regressions in existing 1762+ tests.

## Impact

Root cause of 107 files differing in A2osX binary comparison. All those files have
.DUMMY blocks with standalone label-only lines getting wrong ZP address.
