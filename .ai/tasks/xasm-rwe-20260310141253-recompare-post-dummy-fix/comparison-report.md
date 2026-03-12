# Binary Comparison Report: xasm++ Post .DUMMY/.OR Fix

**Date:** 2026-03-10  
**Task:** xasm-rwe  
**Source Commit:** A2osX 335cd122 ("A2osX.95:LATEST & LAST bugfixes")  
**Reference:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po`  
**Built With:** xasm++ v0.9.0-b850ca5 (commit b850ca5, ".DUMMY/.OR zero-page fix")  
**Prior Run:** xasm-9wz (built with b7b739c)

---

## Executive Summary

The b850ca5 commit (".DUMMY/.OR zero-page addresses now resolve correctly in ResolveSymbols")
introduced **two regressions** that prevented the binary comparison from improving:

| Metric | b7b739c (prior) | b850ca5 (current) | Change |
|--------|-----------------|-------------------|--------|
| **Identical** | 2 | 2 | 0 |
| **Different** | 116 | 113 | -3 (minor improvement) |
| **Not Built** | 15 | 17 | **+2 regressions** |

**Net result: No improvement. b850ca5 introduced critical regressions.**

---

## Regressions Introduced by b850ca5

### Regression 1: SYS/KERNEL fails to build

**Error:** `Parse error: kernel.s.dirent:516: Undefined symbol: K.FCLOSE`

**Root cause:** The star-label fix (commits e0b4425 + f2a45a6) now processes commented-out
`*LABEL .EQ value` lines as private label definitions. The line:
```
*K.CloseDir    .EQ K.FClose
```
is a documentation comment in `KERNEL.S.DIRENT.txt`. With the star-label fix, it is
processed as a symbol definition, but `K.FClose` is defined **later** in `KERNEL.S.STDIO.txt`
(included after DIRENT in the assembly order). Since `K.FClose` is a forward reference that
is never resolved at the point of the `.EQ`, assembly fails.

**Affected file:** `SYS/KERNEL.S.DIRENT.txt:516`  
**Was working in:** b7b739c (kernel built at 26,550 bytes)

### Regression 2: LIB/LIBBLKDEV fails to build

**Error:** `Parse error: inc/io.d2.i:34: Unexpected character after expression: N`

**Root cause:** Same star-label regression. The line:
```
*IO.D2.ReadSect    .EQ $Cn5C
```
is a SCMASM documentation comment using `n` as a slot-number placeholder. With the
star-label fix processing this as a label definition, the assembler attempts to evaluate
`$Cn5C` where `n` is not a valid hex digit, causing a parse error.

**Affected file:** `INC/IO.D2.I.txt:34`  
**Was working in:** b7b739c

### Regression 3: .DUMMY/.OR breaks JMP (.1,x) forward references (b850ca5 itself)

**Root cause:** The b850ca5 fix emits a `DummyOrgAtom` for `.OR` inside `.DUMMY` sections
to avoid advancing the main program counter. However, when a `DummyOrgAtom` appears
**before** a forward-reference label in the atom list, the label resolution pass
incorrectly drops the `jmp (.1,x)` instruction.

**Reproduction:**
```asm
ZPBIN .EQ $E0
              .OR $2000
              .DUMMY           ; <-- DummyOrgAtom appears BEFORE .1 label
              .OR ZPBIN
ZS.START
ZPTR          .BS 2
ZS.END        .ED
CS.START      cld
              jmp (.1,x)       ; <-- JMP DROPS OUT OF OUTPUT
              .DA #$61
.1            .DA 0            ; <-- forward ref .1 is after DummyOrgAtom
```

**Expected output:** `d8 7c 0b 20 61 01 00` (CLD + JMP indirect + .DA bytes)  
**Actual output:**   `d8 61 01 00 00` (CLD + .DA bytes, no JMP!)

**Impact:** All files that declare a `.DUMMY .OR <symbol>` section **before** the main
code header (the A2osX standard binary layout) produce broken output with the critical
`jmp (.1,x)` relocation-table jump missing. This affects ~90+ files.

---

## Detailed Comparison Results

### Files Identical (byte-for-byte): 2

| File | Bytes |
|------|-------|
| `LIB/LIBBLKDEV.O` | 229 |
| `LIB/LIBGUI.O` | 135 |

These two stub/relocation-table files match between xasm++ and SCMASM output. These were
also identical in the prior run.

### Files Not Built: 17

**Pre-existing failures (present in both runs):**
- `BIN/ADM` — depends on 65816 SMB/RMB instructions
- `BIN/ASM.65816` — SMB/RMB instructions unsupported
- `BIN/ASM.65R02` — reduced instruction set
- `BIN/BASIC` — BASIC.FX dependency
- `BIN/BEEP`, `BIN/FIND`, `BIN/HELP`, `BIN/MKTEMP`, `BIN/WHICH`, `BIN/XMASTREE` — various errors
- `BIN/FNT2FON` — host-tool dependency
- `LIB/LIBTUI.O` — depends on TUI assembly order
- `SYS/PM.RAMWORKS` — legacy driver
- `SYS/VEDRIVE.CONF` — config data, not assembled
- `A2OSX.LOGO` — type $CB logo file, not an assembled binary

**New failures (b850ca5 regressions):**
- `SYS/KERNEL` — K.FCLOSE undefined (star-label regression)
- `LIB/LIBBLKDEV` — $Cn5C invalid hex (star-label regression)

**Now builds (was failing in b7b739c):**
- ~~`SYS/PM/PM.NSC`~~ — fixed (assembly now succeeds)

### Files Different (still): 113

All 113 remaining files differ from STABLE.800.po. The nature of the differences
changed significantly due to the b850ca5 `.DUMMY/.OR` regression:

| First-diff offset | Count | Explanation |
|-------------------|-------|-------------|
| `0x0001` | 89 | JMP (.1,x) instruction dropped; remaining bytes shifted |
| `0x0000` | 5 | Binary format differences (ASM.6502, ASM.65C02, etc.) |
| `0x0002–0x0003` | 4 | Minor header differences |
| `0x0008` | 14 | Files that work correctly (differ only in code content) |
| `0x002F` | 1 | A2OSX.SYSTEM (minor diff) |

The 89 files differing at `0x0001` represent the `.DUMMY`-regression cases where
the JMP instruction was dropped and all subsequent bytes shifted by 3.

**Comparison with prior run (b7b739c):**
- Prior: 107 files with first diff at `0x0008` (header matched, code differed)
- Current: only 14 files at `0x0008` (most now broken at `0x0001`)
- The b850ca5 fix made most files **worse**, not better

---

## Root Cause Analysis

The b850ca5 fix correctly identified that `.OR` inside `.DUMMY` sections should not
advance the main program counter — this is architecturally correct.

However, the implementation broke the **assembly of forward-reference indirect JMP
instructions** (`jmp (.1,x)`) when a `.DUMMY` section appears before the referenced
label in the source. The `DummyOrgAtom` injected by the fix disrupts the multi-pass
symbol resolution for the local label `.1`.

This is separate from (but compounded by) the earlier star-label fix regression,
which processes documented-but-commented-out `*LABEL .EQ value` lines as actual
symbol definitions, breaking kernel and libblkdev assembly.

---

## Categorization of Remaining Differences

Even after fixing both regressions, the 113 differing binaries would remain different
from STABLE.800.po. These fall into several categories:

### Category 1: Code Generation Differences (~100 files)
The assembled code differs in size/content from SCMASM output. Likely causes:
- Different instruction encoding choices (BRA vs JMP, ZP vs absolute addressing)
- Different macro expansion output
- Different ordering of constant data sections
- Different ZP variable layout  

**Example:** `BIN/AARP` — stable=964 bytes, xasm++ built=873 bytes (91B less code)

### Category 2: Binary Format / Load Address Differences (5 files)
- `BIN/ASM.6502`, `BIN/ASM.65C02`, `BIN/ASM.SW16`, `BIN/ASM.Z80` — no standard $2000 load address; 
  format differences from offset 0

### Category 3: Pre-existing Build Failures (17 files)
See "Files Not Built" section above.

---

## Before/After Comparison Table

| | **b7b739c** (xasm-9wz) | **b850ca5** (this run) | **Change** |
|---|---|---|---|
| Identical | 2 | 2 | ±0 |
| Different | 116 | 113 | -3 |
| Not Built | 15 | 17 | **+2** |
| Files differing at 0x0008 (best case) | 107 | 14 | **-93** |
| Files differing at 0x0001 (regression) | 0 | 89 | **+89** |

---

## Recommendations

### Fix 1: Revert or narrow the star-label fix (e0b4425/f2a45a6)

The `*LABEL .EQ value` processing should only apply when the `.EQ` value is a
constant expression (a numeric literal or already-defined symbol). It must **not**
process lines that use unresolved forward references as the `.EQ` value.

Additionally, consider whether lines like `*IO.D2.ReadSect .EQ $Cn5C` with
non-hex-digit slot placeholders should simply be treated as regular comments.

### Fix 2: Fix DummyOrgAtom forward-reference interaction (b850ca5 regression)

The `DummyOrgAtom` must not interfere with the resolution of local label forward
references (`jmp (.1,x)`). The fix should ensure that the presence of a DummyOrgAtom
in the atom list does not suppress or drop indirect JMP instructions.

**Suggested test case to add:**
```asm
; .DUMMY section BEFORE forward-reference .1 label must not drop JMP
ZPBIN .EQ $E0
       .OR $2000
       .DUMMY
       .OR ZPBIN
ZS.START
ZPTR   .BS 2
ZS.END .ED
CS.START cld
       jmp (.1,x)  ; must assemble to 7c <lo> <hi>
       .DA #$61
.1     .DA 0
```
Expected: first byte is `d8` (CLD), second byte is `7c` (JMP abs,X opcode).

---

## Files Modified / Artifacts

- `/tmp/A2osX-335cd122-build/` — rebuild with b850ca5 binary
- `/tmp/comparison_results_v2/results_v2.json` — full comparison data
- `/.ai/tasks/xasm-rwe-20260310141253-recompare-post-dummy-fix/comparison-report.md` — this report
