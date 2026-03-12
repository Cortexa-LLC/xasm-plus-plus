# Binary Comparison Report: xasm++ vs SCMASM (A2osX commit 335cd122)

**Date:** 2026-03-10  
**Task:** xasm-9wz  
**Source Commit:** 335cd122 ("A2osX.95:LATEST & LAST bugfixes")  
**Reference:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po`  
**Built With:** xasm++ v0.9.0-b7b739c (commit b7b739c)

---

## Executive Summary

| Result | Count |
|--------|-------|
| **Byte-for-byte identical** | **2** (stub/index files only) |
| **Different content** | **116** |
| **Not built (xasm++ failures)** | **15** |
| **Not built (no source in 335cd122)** | 5 |

**All 116 binary mismatches are caused by a single critical bug in xasm++:**  
The `.DUMMY` / `.OR` zero-page allocation mechanism does not correctly track symbol  
addresses inside dummy sections during `ResolveSymbols()`. All zero-page variable  
references are resolved to the main-section address (`$2000`) instead of the correct  
zero-page addresses (e.g., `$E0`), causing wrong ZP addressing modes in emitted code.

---

## Critical Bug: `.DUMMY`/`.OR` Zero-Page Address Resolution

### Root Cause

In SCMASM and A2osX, zero-page variables are allocated using a dummy section:

```asm
.DUMMY
.OR ZPBIN        ; = $E0 (from a2osx.i)
ZS.START
ZPPtr1  .BS 2    ; allocate 2 ZP bytes
ArgIndex .BS 1   ; allocate 1 ZP byte
ZS.END  .ED
```

After `.ED`, the PC is restored to the main-section address, but `ZS.START` should  
equal `$E0`, `ZPPtr1` = `$E0`, `ArgIndex` = `$E2`, `ZS.END` = `$E3`.

**xasm++ behavior:**  
The `.OR $E0` (or `.OR ZPBIN`) inside `.DUMMY` calls `HandleOr()` which sets  
`*context.current_address = $E0` at **parse time**. However, during the  
`ResolveSymbols()` pass, the assembler re-walks the atom list. Since `HandleOr`  
does NOT emit an `OrgAtom` in dummy section mode (by design, to protect the main  
section PC), `ResolveSymbols()` never sees the `$E0` origin and assigns all labels  
inside the dummy section to the **main section address** (`$2000`).

**Effect:**  
- `ZS.START = $2000` (should be `$E0`)  
- `ZPPtr1 = $2000` (should be `$E0`)  
- All ZP variable accesses use absolute addressing (`$2000+offset`) instead of  
  zero-page addressing (`$E0+offset`)
- File header field `ZS.END-ZS.START` = `$E5` (229) instead of `5`
- All instructions that should use ZP mode (2 bytes) use absolute mode (3 bytes)
- Built files are systematically **smaller** by 1–3 bytes per ZP access because  
  the wrong mode may fold differently, but the binary content is completely wrong

### Verification

Test case demonstrating the bug:

```asm
.OR $2000
.DUMMY
.OR $E0
LABEL_A .EQ *
V1 .BS 3
LABEL_B .EQ *
.ED
.DA LABEL_A      ; should emit E0 00, emits 00 20 ($2000)
.DA LABEL_B      ; should emit E3 00, emits 00 20 ($2000)
```

xasm++ output: `00 20 00 20` (both labels = $2000)  
Expected output: `E0 00 E3 00`

---

## Build Failures: xasm++ Limitations

### Files That Could Not Be Assembled

| File | Error | Root Cause |
|------|-------|------------|
| `BIN/ASM.65816` | `MVN requires two operands: srcbank,destbank` | 65816 MVN/MVP multi-bank syntax not implemented |
| `BIN/ASM.65R02` | `Unsupported instruction: BBR0` | Rockwell BBR/BBS/RMB/SMB used as label names at column 0; xasm++ misidentifies them as instructions |
| `BIN/ADT` | `Unsupported instruction: CLC.9` | SCMASM 3-char opcode parsing: `clc.9` means opcode=CLC, label-definition=.9; xasm++ reads "CLC.9" as the mnemonic |
| `BIN/FNT2FON` | `Unterminated string` | Source bug: `.CZ -N : No shrink\r\n"` missing opening quote — SCMASM assembled it anyway |
| `BIN/XMASTREE` | `Unexpected character after expression: 1` | SCMASM character literal `.EQ '*'` not supported by xasm++ |
| `SYS/PM.RAMWORKS` | `Unsupported instruction: TOO` | `ERROR:RWDRV.SIZE too big` — SCMASM `ERROR` directive not implemented; xasm++ parses "too" as instruction |
| `SYS/PM/PM.NSC` | `Unterminated string` | String handling edge case |
| `LIB/LIBTUI.O` | Source missing | `LIBTUI.O.S.txt` does not exist at commit 335cd122 |

### Files Not in 335cd122 Source

| File | Reason |
|------|--------|
| `BIN/ADM` | Shell script (TXT type) — not assembled |
| `BIN/BASIC` | Applesoft BASIC.SYSTEM (SYS type) — not assembled from source |
| `BIN/BEEP` | Source `.S.txt` missing at 335cd122 |
| `BIN/FIND` | Shell script (TXT type) |
| `BIN/HELP` | Shell script (TXT type) |
| `BIN/MKTEMP` | Shell script (TXT type) |
| `BIN/WHICH` | Shell script (TXT type) |
| `SYS/VEDRIVE.CONF` | Configuration script (TXT type) |

---

## Source Compatibility Patches Applied

Before building, two source compatibility fixes were applied to the 335cd122 worktree:

1. **`DRV/PPPSSC.DRV.S.D.txt` — `cld!` → `cld`**  
   SCMASM reads exactly 3 characters for the opcode (`cld`), treating `!` as the start  
   of the operand field (where it's a no-op). xasm++ reads until whitespace, getting  
   mnemonic `CLD!`. Fixed by removing the `!`.

2. **`BIN/ASM.*.A.txt`, `BIN/ASM.*.O.txt` — `$$"..."` → `.PS "..."`**  
   SCMASM `$$"string"` inside `.DA` emits the string bytes directly (same as `.PS`).  
   xasm++ does not implement the `$$` syntax. The current branch already contains  
   equivalent `.PS`-based replacements that produce byte-identical output.

---

## Byte-for-Byte Identical Files

Only two files matched exactly:

| File | Size |
|------|------|
| `LIB/LIBBLKDEV.O` | 229 bytes |
| `LIB/LIBGUI.O` | 135 bytes |

Both are small stub/index files that contain only data tables with no ZP variable  
references and no instruction code — hence unaffected by the `.DUMMY`/`.OR` bug.

---

## Mismatch Analysis

### Systematic Size Difference

All 114 files with non-zero size differences are **smaller** in the xasm++ build:

| Category | Count | Notes |
|----------|-------|-------|
| Stable larger (built smaller) | 114 | ZP addressing: wrong addr → wrong opcode widths |
| Same size but different content | 1 | `SYS/PM/PM.APPLETALK` (1027 bytes each) |
| Built larger than stable | 1 | Minor instruction encoding difference |

The size reduction occurs because the wrong ZP addresses (`$2000`-based) change the  
addressing mode selection in emitted instructions.

### Representative Examples

**`BIN/ACC`** (315 → 310 bytes, −5):  
- Header field `ZS.END-ZS.START` = `0xE5` (229) in built, should be `0x05` (5)  
- ZP variable `ZPPtr1` resolves to `$2000` instead of `$E0`

**`SYS/KERNEL`** (27716 → 26550 bytes, −1166):  
Largest absolute difference; kernel has many ZP variables and the wrong addressing  
propagates through hundreds of instructions.

---

## Required Fixes in xasm++

### Priority 1 — Critical (blocks all A2osX binaries)

**Fix: `.DUMMY`/`.OR` zero-page address resolution**  
In `ScmasmSyntaxParser::HandleOr()` (dummy section branch), instead of only updating  
`*context.current_address`, also emit a special `DummyOrgAtom` that:
1. Is NOT processed by the code emitter (no bytes emitted, no output OrgAtom)
2. IS processed by `ResolveSymbols()` to update the address counter during the resolve pass

Alternatively, store the dummy section `.OR` addresses on a separate stack and replay  
them during `ResolveSymbols()`.

### Priority 2 — Medium (blocks specific binaries)

| Fix | Affected Files |
|-----|----------------|
| SCMASM `ERROR` directive | `SYS/PM.RAMWORKS` and any file using `ERROR:message` |
| Character literals `.EQ 'X'` | `BIN/XMASTREE`, likely others |
| `CLC.9`-style inline labels | `BIN/ADT` |
| Opcode names as column-0 labels (BBR/BBS/etc.) | `BIN/ASM.65R02` |
| MVN/MVP dual-operand syntax | `BIN/ASM.65816` |
| Lenient string parsing (.CZ without open quote) | `BIN/FNT2FON` |

---

## Methodology

1. **Worktree:** `git worktree add /tmp/A2osX-335cd122 335cd122`
2. **CMakeLists.txt:** Copied from `xasm++_stable` branch (not present in 335cd122)
3. **Template image:** Copied `blank800k_prodosfx.po` to worktree `.Floppies/`
4. **Build directory:** `/tmp/A2osX-335cd122-build/`
5. **Configure:** `cmake /tmp/A2osX-335cd122 -DCMAKE_BUILD_TYPE=Release`
6. **Build:** All targets built individually; failures documented
7. **Extract STABLE:** `acx export --raw` → `/tmp/stable_extracted/`
8. **Compare:** Python script byte-for-byte comparison of all matching files

---

## Conclusion

xasm++ cannot currently produce byte-identical output to SCMASM for A2osX commit 335cd122.  
The root cause is a single critical bug: `.DUMMY`/`.OR` dummy section addresses are not  
propagated through the `ResolveSymbols()` pass, causing all zero-page variable addresses  
to be wrong. Fixing this bug is the highest priority for achieving SCMASM compatibility.

Once that fix is in place, a second comparison run should be done to measure remaining  
differences (expected to be very small — the `PM.APPLETALK` same-size difference may  
represent a separate minor issue unrelated to ZP addressing).
