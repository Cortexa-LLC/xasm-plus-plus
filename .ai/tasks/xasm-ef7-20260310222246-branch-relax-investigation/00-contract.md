# Branch Relaxation False-Positive Investigation

## Goal
Determine and fix why xasm++ falsely relaxes branches at the 127-byte limit.

## Status
- **drv/mkboard.drv** ✅ FIXED — ZP correction (dec/inc pStack overcounted as 3B, now 2B)
- **drv/dhgr.drv** ⏳ in xasm-oop
- **drv/pppssc.drv** ⏳ in xasm-oop
- **bin/sh, bin/mv, bin/cp, bin/du, bin/csh, bin/forth** ⏳ in xasm-oop

## Root Cause (confirmed for mkboard.drv)
ZP symbol operands: `GetInstructionSize` returns 3 (assumes absolute) for any symbol operand.
Instructions like `dec pStack` (pStack=$DC) are actually 2-byte ZP form.
Fix in `src/syntax/scmasm_syntax.cpp` ~line 1157: evaluate expression; if ≤$FF and not JSR/JMP, correct to 2.

## dhgr.drv Findings (for xasm-oop)
- False relaxation of `bne .1` at BITBLT.0 scope (DHGR.DRV.S.BLT.txt line 90)
- Correct distance is 9 bytes (tya+SYSCALL2+STA = 1+5+3), but overcounted >127
- JMP target resolves to $2483 (wrong) instead of $23EC (BITBLT.0.1)
- All CBX.Cache/CB.Cache symbol refs are +3 higher in built vs stable — shift caused by the false relaxation

## Blocking Task
- **xasm-oop**: Fix branch relaxation false-positives for dhgr.drv, pppssc.drv, bin files

## Key Paths
- Source: `/tmp/A2osX-335cd122/`
- Build: `/tmp/A2osX-335cd122-build/stage/`
- xasm++ source: `/Users/bryanw/Projects/Vintage/tools/xasm++/`
- ZP fix: `src/syntax/scmasm_syntax.cpp` ~line 1157
- Branch handler: `src/cpu/cpu_6502_branch_handler.cpp`
- Assembler: `src/core/assembler.cpp` lines 490–555

## Acceptance Criteria
- Root cause confirmed for all affected files (delegated to xasm-oop)
- All unit tests pass
- drv/mkboard.drv ✅, drv/dhgr.drv, drv/pppssc.drv byte-identical to stable (via xasm-oop)
