# Validation Run Report: ssc.drv, ssc.i.drv, libtcpip, libetalk

**Task ID:** xasm-oa9  
**Date:** 2026-03-11  
**xasm++ version:** 0.9.0-00c8f8e (HEAD, post-xasm-zp1 fixes)  
**A2osX source:** commit 335cd122  
**Reference:** `/tmp/stable_60b98d24/`  
**Staging area:** `/tmp/a2osx-stage-oa9/`  

---

## Assembly Results

All 4 files assembled successfully with no errors:

```
cd /tmp/a2osx-stage-oa9 && xasm++ --cpu 65c02 --syntax scmasm -I /tmp/a2osx-stage-oa9 usr/src/drv/ssc.drv.s
Assembly successful: drv/ssc.drv

xasm++ usr/src/drv/ssc.i.drv.s  → Assembly successful: drv/ssc.i.drv
xasm++ usr/src/lib/libtcpip.s   → Assembly successful: lib/libtcpip
xasm++ usr/src/lib/libetalk.s   → Assembly successful: lib/libetalk
```

---

## File Size Summary

| File | Stable (bytes) | Built (bytes) | Delta |
|------|---------------|---------------|-------|
| ssc.drv | 1187 | 1187 | 0 |
| ssc.i.drv | 1457 | 1457 | 0 |
| libtcpip | 6429 | 6429 | 0 |
| libetalk | 1884 | 1884 | 0 |

All four files match the stable in size.

---

## Diff Count Summary

| File | Diff count | First diff | Verdict |
|------|-----------|-----------|---------|
| ssc.drv | 39 | v$22C4 (stable=$01 built=$00) | **POSSIBLE_BUG** (multiple bugs) |
| ssc.i.drv | 39 | v$22D2 (stable=$01 built=$00) | **POSSIBLE_BUG** (multiple bugs) |
| libtcpip | 62 | v$20A6 (stable=$00 built=$53) | **SYSCALL** (all 31 pairs = $E200→$0153) |
| libetalk | 10 | v$2198 (stable=$00 built=$53) | **SYSCALL** (all 5 pairs = $E200→$0153) |

---

## libtcpip — SYSCALL Pattern (No Assembler Bugs)

**62 byte diffs = 31 × 2-byte pairs, all SYSCALL address substitution:**

| Position | Stable | Built | Pattern |
|----------|--------|-------|---------|
| v$20A6 | $E200 | $0153 | SYSCALL ✓ |
| v$211C | $E200 | $0153 | SYSCALL ✓ |
| v$21FE | $E200 | $0153 | SYSCALL ✓ |
| v$246C | $E200 | $0153 | SYSCALL ✓ |
| v$256E | $E200 | $0153 | SYSCALL ✓ |
| v$25ED | $E200 | $0153 | SYSCALL ✓ |
| v$2632 | $E200 | $0153 | SYSCALL ✓ |
| v$2640 | $E200 | $0153 | SYSCALL ✓ |
| ... (31 pairs total) | | | |

All 31 pairs are `stable=$E200 → built=$0153` which is the expected
A2osX.SYSCALL2 address change (A2osX.GP+19 = $0140+19 = $0153).
This is a **source code change** from the version used to build the stable,
not an assembler bug. **VERDICT: SYSCALL — no assembler issues.**

---

## libetalk — SYSCALL Pattern (No Assembler Bugs)

**10 byte diffs = 5 × 2-byte pairs, all SYSCALL address substitution:**

| Position | Stable | Built | Pattern |
|----------|--------|-------|---------|
| v$2198 | $E200 | $0153 | SYSCALL ✓ |
| v$23B7 | $E200 | $0153 | SYSCALL ✓ |
| v$247B | $E200 | $0153 | SYSCALL ✓ |
| v$2489 | $E200 | $0153 | SYSCALL ✓ |
| v$2498 | $E200 | $0153 | SYSCALL ✓ |

All 5 pairs are `stable=$E200 → built=$0153` (same SYSCALL pattern as libtcpip).
**VERDICT: SYSCALL — no assembler issues.**

---

## ssc.drv — Multiple Bugs (POSSIBLE_BUG)

### 39 diffs breakdown:

| Category | Count | Type |
|----------|-------|------|
| P1 baud table `.DA >expr` encoding | 26 | **POSSIBLE_BUG** |
| DIB address 8 bytes too low | 6 | **POSSIBLE_BUG** |
| DCB.BAUD `.DA >expr` encoding + `.DA #'N'` = 0x00 | 4 | **POSSIBLE_BUG** |
| BNE/BEQ branch target offsets | 3 | Source code change |

### Bug A: `.DA >expr` emits wrong 32-bit value

**Location:** P1 baud rate table (v$22C2–v$2302), all 16 entries.

The source has `.DA >115200`, `.DA >50`, ..., `.DA >19200` (16 entries).
In SCMASM, `.DA >expr` is a **4-byte (32-bit little-endian) longword** directive.
The stable assembler correctly emits the full 32-bit value:

```
Stable: .DA >115200 → 00 C2 01 00  (115200 as 32-bit LE) ✓
Built:  .DA >115200 → 00 C2 00 C2  (wrong: high byte 0xC2 emitted as 16-bit word × 2)
```

**Pattern for all 16 entries:**
- Stable: `value_as_4bytes_LE`
- Built: `lo_byte hi_byte lo_byte hi_byte` (16-bit value duplicated)

This is confirmed by the `.DA > comment` in the source (`; BAUD (DWORD)` at line 864).

**All 26 diff positions (v$22C2–v$2302):** All are bytes 2–3 of each 4-byte baud entry where stable has the correct upper bytes and built has wrong duplicated bytes.

### Bug B: DIB label resolved to wrong address (8 bytes too low)

**Location:** 6 diffs at v$234E, v$236C, v$239B, v$23C2, v$23C8, v$23D9.

These are all LDA/TSB/TRB instructions that reference DIB in the driver code:
```
Stable: LDA $2480,Y  (DIB = $2480 in stable — CORRECT)
Built:  LDA $2478,Y  (DIB = $2478 in built — WRONG by 8 bytes)
```

The label `DIB` in source (x.ssc.drv.s line 850) appears on the same line as `.DO SSCIRQ=1`:
```
DIB     .DO SSCIRQ=1
        .DA #S.DIB.S.WRITE+S.DIB.S.READ+S.DIB.S.IRQ
        .ELSE
        .DA #S.DIB.S.WRITE+S.DIB.S.READ
        .FIN
```

xasm++ assigns DIB the address from within the conditional block context ($2478)
instead of the correct current PC at that line ($2480 = DRV.CS.END+4).

**Root cause:** Label on `.DO` directive line gets wrong address in xasm++.
The actual DIB DATA bytes are at the correct position in the file (offset $0480 in both
stable and built). Only the CODE REFERENCES use the wrong address.

### Bug C: `.DA #'N'` emits 0x00 instead of 0x4E

**Location:** DCB.PARITY and DCB.FLOW at v$24A1–v$24A2.

```asm
        .DA #'N'    ; PARITY
        .DA #'N'    ; FLOW
```

Verified with isolated test:
```
Stable: 4E 4E  ('N' = 0x4E, correct)
Built:  00 00  (character literal evaluates to 0x00, wrong)
```

This is distinct from Bug A: `.DA #8` (numeric literals) work correctly (emits $08).
Only character literals in `.DA #'X'` syntax produce $00 in xasm++.

Also at same positions: DCB.BAUD wrong values (same `.DA >expr` bug as Bug A but for v$249D–v$249E).

### Diffs from source code evolution (not assembler bugs)

**Location:** v$23EA (BNE offset), v$2433 (BNE offset), v$2437 (BEQ offset).

```
Stable: D0 04 = BNE +4 (forward to READ.2)
Built:  D0 FC = BNE -4 (backward to READ.10)
```

The source has `bne READ.11` where for SSCIRQ=0, READ.11 is inside the `.DO SSCIRQ=1` 
block making READ.11 = READ.10 (same address), creating a backward branch.
The stable was compiled from a version where READ.11 was defined outside the conditional,
placing it 4 bytes forward. **This is a source code change, not an assembler bug.**

---

## ssc.i.drv — Multiple Bugs (POSSIBLE_BUG)

**39 diffs — identical bug categories as ssc.drv:**

| Category | Count | Type |
|----------|-------|------|
| P1 baud table `.DA >expr` encoding | 26 | **POSSIBLE_BUG** |
| DIB address 8 bytes too low | 6 | **POSSIBLE_BUG** (stable=$248E, built=$2486) |
| DCB.BAUD wrong + `.DA #'N'` = 0x00 | 4 | **POSSIBLE_BUG** |
| BNE/BEQ branch target offsets | 3 | Source code change |

Same patterns as ssc.drv. For SSCIRQ=1, the DIB address is $248E (stable)
vs $2486 (built) — still 8 bytes too low. DCB.BAUD uses `.DA >9600` (wrong duplicate
encoding). DCB.PARITY and DCB.FLOW are 0x00 instead of 'N' (same `.DA #'N'` bug).

---

## Summary of Assembler Bugs Found

| Bug | Affects | Description |
|-----|---------|-------------|
| **Bug A: `.DA >expr` wrong 32-bit encoding** | ssc.drv (26+2), ssc.i.drv (26+2) | `.DA >value` should emit 32-bit LE, instead emits high-byte as 16-bit word repeated twice |
| **Bug B: DIB label on `.DO` line gets wrong address** | ssc.drv (6), ssc.i.drv (6) | Label on `.DO conditional` line resolves to wrong PC (8 bytes too low) |
| **Bug C: `.DA #'char'` emits 0x00** | ssc.drv (2), ssc.i.drv (2) | Character literal in `.DA` immediate form ignored, emits 0x00 |

## Comparison with Pre-Fix Run (xasm-zp1)

The xasm-zp1 investigation ran against commit d05fd40 and found:
- ssc.drv: 39 diffs, first at v$22B4
- ssc.i.drv: 39 diffs
- libtcpip: 65 diffs
- libetalk: 11 diffs

Current run (post xasm-zp1 / Bug 9+10 fixes, commit 00c8f8e):
- ssc.drv: 39 diffs (unchanged — xasm-zp1 fixes did not affect these)
- ssc.i.drv: 39 diffs (unchanged)
- libtcpip: **62 diffs** (3 fewer — confirms some xasm-zp1 fix applied)
- libetalk: **10 diffs** (1 fewer — confirms some xasm-zp1 fix applied)

The libtcpip/libetalk improvements confirm the ZP/DUMMY fix was effective.
The ssc.drv/ssc.i.drv count unchanged: their 39 diffs include 3 source-change
branch diffs + 36 xasm++ bugs (Bug A×28, Bug B×6, Bug C×2).

---

## All Diff Positions

### ssc.drv (39 diffs)
```
v$22C4 stable=$01 built=$00  │ P1[0] BAUD+2 (Bug A)
v$22C5 stable=$00 built=$C2  │ P1[0] BAUD+3 (Bug A)
v$22C8 stable=$00 built=$32  │ P1[1] BAUD+2 (Bug A)
v$22CC stable=$00 built=$4B  │ P1[2] BAUD+2 (Bug A)
v$22D0 stable=$00 built=$6E  │ P1[3] BAUD+2 (Bug A)
v$22D4 stable=$00 built=$87  │ P1[4] BAUD+2 (Bug A)
v$22D8 stable=$00 built=$96  │ P1[5] BAUD+2 (Bug A)
v$22DC stable=$00 built=$2C  │ P1[6] BAUD+2 (Bug A)
v$22DD stable=$00 built=$01  │ P1[6] BAUD+3 (Bug A)
v$22E0 stable=$00 built=$58  │ P1[7] BAUD+2 (Bug A)
v$22E1 stable=$00 built=$02  │ P1[7] BAUD+3 (Bug A)
v$22E4 stable=$00 built=$B0  │ P1[8] BAUD+2 (Bug A)
v$22E5 stable=$00 built=$04  │ P1[8] BAUD+3 (Bug A)
v$22E8 stable=$00 built=$08  │ P1[9] BAUD+2 (Bug A)
v$22E9 stable=$00 built=$07  │ P1[9] BAUD+3 (Bug A)
v$22EC stable=$00 built=$60  │ P1[10] BAUD+2 (Bug A)
v$22ED stable=$00 built=$09  │ P1[10] BAUD+3 (Bug A)
v$22F0 stable=$00 built=$10  │ P1[11] BAUD+2 (Bug A)
v$22F1 stable=$00 built=$0E  │ P1[11] BAUD+3 (Bug A)
v$22F4 stable=$00 built=$C0  │ P1[12] BAUD+2 (Bug A)
v$22F5 stable=$00 built=$12  │ P1[12] BAUD+3 (Bug A)
v$22F8 stable=$00 built=$20  │ P1[13] BAUD+2 (Bug A)
v$22F9 stable=$00 built=$1C  │ P1[13] BAUD+3 (Bug A)
v$22FC stable=$00 built=$80  │ P1[14] BAUD+2 (Bug A)
v$22FD stable=$00 built=$25  │ P1[14] BAUD+3 (Bug A)
v$2300 stable=$00 built=$4B  │ P1[15] BAUD+2 (Bug A)
v$234E stable=$80 built=$78  │ LDA DIB,Y → DIB addr hi byte (Bug B)
v$236C stable=$80 built=$78  │ LDA DIB,Y → DIB addr hi byte (Bug B)
v$239B stable=$80 built=$78  │ TSB DIB → DIB addr hi byte (Bug B)
v$23C2 stable=$80 built=$78  │ TSB DIB → DIB addr hi byte (Bug B)
v$23C8 stable=$80 built=$78  │ TRB DIB → DIB addr hi byte (Bug B)
v$23D9 stable=$80 built=$78  │ TSB DIB → DIB addr hi byte (Bug B)
v$23EA stable=$04 built=$FC  │ BNE READ.11 offset (source change)
v$2433 stable=$04 built=$FC  │ BNE WRITE.11 offset (source change)
v$2437 stable=$18 built=$10  │ BEQ offset (source change)
v$249D stable=$01 built=$00  │ DCB.BAUD+2 (Bug A)
v$249E stable=$00 built=$C2  │ DCB.BAUD+3 (Bug A)
v$24A1 stable=$4E built=$00  │ DCB.PARITY .DA #'N' → 0 (Bug C)
v$24A2 stable=$4E built=$00  │ DCB.FLOW .DA #'N' → 0 (Bug C)
```

### ssc.i.drv (39 diffs)
```
v$22D2 stable=$01 built=$00  │ P1[0] BAUD+2 (Bug A)
... [26 P1 table entries — same pattern]
v$235E stable=$8E built=$86  │ DIB addr hi byte (Bug B, DIB=$248E vs $2486)
v$237C stable=$8E built=$86  │ DIB addr hi byte (Bug B)
v$23AB stable=$8E built=$86  │ DIB addr hi byte (Bug B)
v$2412 stable=$8E built=$86  │ DIB addr hi byte (Bug B)
v$2425 stable=$8E built=$86  │ DIB addr hi byte (Bug B)
v$2446 stable=$8E built=$86  │ DIB addr hi byte (Bug B)
v$245A stable=$04 built=$FC  │ BNE READ.11 offset (source change)
v$24CA stable=$04 built=$FC  │ BNE WRITE.11 offset (source change)
v$24CE stable=$3D built=$35  │ BEQ offset (source change)
v$25AB stable=$00 built=$80  │ DCB.BAUD+2 (Bug A)
v$25AC stable=$00 built=$25  │ DCB.BAUD+3 (Bug A)
v$25AF stable=$4E built=$00  │ DCB.PARITY .DA #'N' → 0 (Bug C)
v$25B0 stable=$4E built=$00  │ DCB.FLOW .DA #'N' → 0 (Bug C)
```

### libtcpip (62 diffs = 31 SYSCALL pairs)
```
v$20A6 stable=$E200 built=$0153  │ SYSCALL ($E200→$0153)
v$211C stable=$E200 built=$0153  │ SYSCALL
v$21FE stable=$E200 built=$0153  │ SYSCALL
v$246C stable=$E200 built=$0153  │ SYSCALL
v$256E stable=$E200 built=$0153  │ SYSCALL
v$25ED stable=$E200 built=$0153  │ SYSCALL
v$2632 stable=$E200 built=$0153  │ SYSCALL
v$2640 stable=$E200 built=$0153  │ SYSCALL
... [31 pairs total, all $E200→$0153]
```

### libetalk (10 diffs = 5 SYSCALL pairs)
```
v$2198 stable=$E200 built=$0153  │ SYSCALL ($E200→$0153)
v$23B7 stable=$E200 built=$0153  │ SYSCALL
v$247B stable=$E200 built=$0153  │ SYSCALL
v$2489 stable=$E200 built=$0153  │ SYSCALL
v$2498 stable=$E200 built=$0153  │ SYSCALL
```

---

## Recommended Next Steps

1. **Bug A (`.DA >expr` 32-bit encoding):** Implement proper 32-bit (DWORD) handling when
   the `>` prefix is used with `.DA` — emit the full 32-bit little-endian value of the
   expression, not the high-byte as a 16-bit word.

2. **Bug B (DIB label on `.DO` line):** Fix label assignment for labels that appear on the
   same source line as `.DO` conditional directives. The label should receive the current
   PC value from AFTER all preceding data in the current section, not from within
   conditional block evaluation.

3. **Bug C (`.DA #'char'` emits 0x00):** Fix character literal parsing in `.DA #'X'` form.
   The ASCII value of the character should be emitted as a 1-byte immediate.
