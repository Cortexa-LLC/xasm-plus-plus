# Work Log

**Task ID:** xasm-d7e
**Started:** 2026-03-11
**Status:** Completed

---

## Session 1: 2026-03-11

### Objectives
- Investigate sbin/bbsd (+16B), bin/httpget (+14B), bin/fnt2fon (+13B) deltas vs stable

### Work Completed

✓ Generated assembly listings for all three binaries  
✓ Binary diff analysis to locate first divergence point  
✓ Relocation table comparison for all three binaries  
✓ Code section diff analysis using Python difflib  
✓ Root cause identified for all three binaries  
✓ Updated bug tracker with findings  

### Investigation Summary

All three deltas are **source code changes** between the STABLE.800.po build commit
and the current A2osX source at commit 335cd122. These are NOT assembler bugs.

#### sbin/bbsd (+16 bytes)

Binary diff analysis:
- CS section: stable=$0370 (880B), built=$0380 (896B), delta=+16B
- CS.INIT and CS.RUN addresses identical ($2038, $2049) in both
- Extra bytes are distributed throughout the code section

Root causes (4 changes):
1. **+2B at $212B**: SYSCALL replaced by direct FDs table lookup
   - Stable: `LDX #$F4; JSR A2osX.SYSCALL` (5 bytes)
   - Built: `TAX; LDA FDs.pHI-1,X; LDY FDs.pLO-1,X` (7 bytes)
2. **-2B at $21A6**: Different push logic for connection data
   - Stable: `LDA $01; DEC pStack; STA (pStack); LDA $00` (8 bytes)
   - Built: `LDA #0; DEC pStack; STA (pStack)` (6 bytes)
3. **+15B at $2225**: New `_DEBUG` subroutine added (dead code, never called)
   - Source line 230: `_DEBUG >LDYA.G CmdBuf; >SYSCALL PutS; rts`
   - Stable had no `_DEBUG` - CS.DOEVENT followed immediately after CS.RUN.CLIENT1
4. **+1B at $22AF**: Extra `LDA (ZPLinePtr)` instruction added
   - Net delta: +2 - 2 + 15 + 1 = +16B ✓

#### bin/httpget (+14 bytes)

Binary diff analysis:
- CS section: stable=$05FA (1530B), built=$0608 (1544B), delta=+14B

Root causes:
1. **+2B**: New relocation table entry `L.MSG.Progress` added
   - Stable had 20 reloc table entries; built has 21
   - New entry at $2038: L.MSG.Progress → address of progress message string
   - This shifts CS.INIT from $203A to $203C
2. **-4B at $218C**: Code simplification
3. **+12B at $22F7**: New progress display code using `L.MSG.Progress`
   - Source: `>PUSHW L.MSG.Progress; >SYSCALL PrintF` (new feature)
4. **+4B at $2483**: Additional code change
   - Net delta: +2 - 4 + 12 + 4 = +14B ✓

#### bin/fnt2fon (+13 bytes)

Binary diff analysis:
- CS size: identical ($065F = 1631B) in both
- DS size: identical ($001D = 29B) in both
- First diff in relocation table at $201A: $279D → $27AA (delta=+13)
- All subsequent addresses shifted by +13 bytes

Root cause: MSG.USAGE string was changed/extended
- Stable (53 bytes): ends with `A0 BA A0 00` = ` : ` in Apple II high-bit encoding
  - Original source likely: `.AZ -" : "` (AZ with `-"` prefix = set high bit on all chars)
- Built (66 bytes): `N : No shrink\r\n"\0` (17 bytes)
  - Current source: `.CZ -N : No shrink\r\n"` (typo — missing opening `"`)
  - With `-` as delimiter, xasm++ reads rest of line as content
- Delta: 66 - 53 = +13B ✓

**Note**: The fnt2fon source at line 811 has a possible typo:
- Current: `.CZ -N : No shrink\r\n"`
- Intended (?) : `.CZ "-N : No shrink\r\n"`
- This is a SOURCE CODE issue, not an xasm++ bug

### Files Modified

- `.ai/docs/a2osx-bug-tracker.md` — Updated Remaining Differences table with investigation findings for bbsd, httpget, fnt2fon

### Decisions Made

1. **Not an assembler bug**: Confirmed all three deltas are legitimate source code changes.
   None require fixes to xasm++.

2. **fnt2fon typo noted but not fixed**: The missing `"` in `.CZ -N : No shrink\r\n"` is
   a source code issue in the A2osX repository, not in xasm++. Documented in tracker.

### Next Steps

None — investigation complete. These binaries can be removed from "potential assembler bugs" 
list and marked as "source code differences" in the tracker.
