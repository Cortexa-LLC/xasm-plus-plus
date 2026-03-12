# Work Log: Validation Run — ssc.drv, ssc.i.drv, libtcpip, libetalk

**Task ID:** xasm-oa9  
**Date:** 2026-03-11  
**Engineer:** AI Engineer

---

## Session 2026-03-11

### Setup

- Confirmed source tree at `/tmp/A2osX-335cd122/` (A2osX commit 335cd122)
- Confirmed stable reference at `/tmp/stable_60b98d24/`
- Built xasm++ at HEAD (00c8f8e) — post xasm-zp1 fixes (Bug 9 DUMMY/ZP fix, Bug 10 JSR/JMP absolute fix, also Bug 11-15)
- Created staging area at `/tmp/a2osx-stage-oa9/` with lowercase symlinks for all source files

### Assembly

All 4 files assembled successfully:
```
ssc.drv:   1187 bytes
ssc.i.drv: 1457 bytes  
libtcpip:  6429 bytes
libetalk:  1884 bytes
```

All sizes match stable exactly.

### Diff Analysis

**libtcpip (62 diffs):** All 31 pairs are SYSCALL pattern `$E200→$0153` — expected source change, no assembler bugs.

**libetalk (10 diffs):** All 5 pairs are SYSCALL pattern `$E200→$0153` — expected source change, no assembler bugs.

**ssc.drv and ssc.i.drv (39 diffs each):** Three assembler bugs identified:

1. **Bug A: `.DA >expr` emits wrong 32-bit value**
   - In SCMASM, `.DA >expr` should emit a 4-byte (32-bit LE) value
   - xasm++ emits the high-byte of expr as a 16-bit word, then repeats it
   - Affects: P1 baud table (26 bytes) + DCB.BAUD (2 bytes) = 28 diffs per file
   
2. **Bug B: DIB label on `.DO` directive line gets wrong address**
   - Source: `DIB .DO SSCIRQ=1` — label on the same line as conditional
   - xasm++ assigns DIB = $2478 (ssc.drv) or $2486 (ssc.i.drv)
   - Correct values: $2480 and $248E respectively (8 bytes higher)
   - Affects: 6 address bytes per file (instructions referencing DIB)
   
3. **Bug C: `.DA #'char'` emits 0x00 instead of ASCII value**
   - `.DA #'N'` should emit 0x4E but xasm++ emits 0x00
   - Confirmed with isolated test: `DA #8 DA #1 DA #'N' DA #'N'` → `08 01 00 00`
   - Affects: DCB.PARITY and DCB.FLOW = 2 bytes per file

3 remaining diffs per file are from source code evolution (BNE branch direction in READ/WRITE loops when SSCIRQ=0 — READ.11 label placement changed between stable's source and current source).

### Comparison with Pre-Fix Counts

| File | Pre-fix | Post-fix | Change |
|------|---------|---------|--------|
| ssc.drv | 39 | 39 | unchanged |
| ssc.i.drv | 39 | 39 | unchanged |
| libtcpip | 65 | 62 | -3 |
| libetalk | 11 | 10 | -1 |

libtcpip/libetalk improved by xasm-zp1 fixes. ssc.drv/ssc.i.drv unchanged because their bugs are different (Bug A/B/C not Bug 9/10).

### Status: COMPLETE

Full report written to `30-validation-report.md`.
Three new bugs identified (Bug A, Bug B, Bug C) needing separate fix tasks.
