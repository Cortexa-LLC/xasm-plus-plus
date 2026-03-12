# Binary Comparison Report: xasm++ Run 4 (post-pending_label_, HasOpcode, unterminated string fixes)

**Date:** 2026-03-10  
**Task:** xasm-adr  
**xasm++ Commit:** 7531e43 ("fix: HasOpcode() excludes 65816-only opcodes in 65c02/6502 mode")  
**Prior Commits:** 53f1fa5 (unterminated strings), 0d281fd (pending_label_ in .DUMMY)  
**A2osX Source:** 335cd122 (via xasm++_stable branch, commit 0fa0ed4b)  
**Reference:** `/tmp/stable_extracted/` (from STABLE.800.po floppy image)  
**Prior Run:** xasm-d2b (run 3, d05fd40)

---

## Executive Summary

Run 4 shows **net improvement** over run 3: 1 newly identical file, 6 previously-failing files 
now build successfully, and **0 regressions**.

| Metric | Run 3 (d05fd40) | Run 4 (7531e43) | Change |
|--------|-----------------|-----------------|--------|
| Identical | 2 | 3 | +1 ✅ |
| Different | 113 | 119 | +6 (all were "not built") |
| Not built | 17 | 10 | -7 ✅ |
| **Total in-scope** | **132** | **132** | same scope |

(Note: excludes INC/ files, ROOT/, ETC/ and PRODOS binary from comparison scope)

---

## Improvements from Run 3 → Run 4

### Newly Identical (1 file)
- **A2OSX.LOGO** (1608 bytes) — Was "not built" in run 3; now correctly assembled and identical to stable.

### Newly Building — Were "Not Built" in Run 3 (6 files)
These files previously failed to assemble (xasm++ aborted with an error); they now build
but still differ from the stable reference:

| File | Likely Fix | Notes |
|------|-----------|-------|
| BIN/ASM.65816 | HasOpcode() mode-aware fix | 65816-only opcodes now excluded in 65C02 mode |
| BIN/ASM.65R02 | HasOpcode() mode-aware fix | Similar to ASM.65816 |
| BIN/FNT2FON | Unterminated string fix | Source uses unterminated .CS/.CZ string directives |
| LIB/LIBBLKDEV | pending_label_ .DUMMY fix | Uses .DUMMY blocks with labels |
| LIB/LIBTUI.O | pending_label_ .DUMMY fix | Similar |
| SYS/KERNEL | pending_label_ .DUMMY fix | Core kernel now assembles |

### No Regressions
Zero files went from identical/different → not-built. All previous improvements are preserved.

---

## Remaining Differences (119 files)

### Category 1: first_diff=0x0000 (6 files) — ASM Sub-assembler Table Files
**Files:** BIN/ASM.6502, BIN/ASM.65816, BIN/ASM.65C02, BIN/ASM.65R02, BIN/ASM.SW16, BIN/ASM.Z80

These are opcode table files (not standard executables). The first bytes are pointer words 
to lookup tables, which differ because the table sizes changed. The table content itself 
(opcodes, addressing modes) is correct; only layout addresses differ.

**Root cause:** Same-generation code compiled to slightly different sizes → address offsets differ.

### Category 2: first_diff=0x0005 (44 files) — BIN Layout Version 2 Files
**Sample files:** BIN/ASM, BIN/ATTR, BIN/CAT, BIN/CC, BIN/SH, DRV/DHGR.DRV, LIB/LIBBLKDEV, etc.

These are executables using BIN Layout Version 2 (`.DA #2` in source). The stable floppy 
was built from an older version of the source that used `.DA #1` (Version 1). The current 
source correctly specifies Version 2.

**Binary header at offset 5:** stable=0x01, built=0x02  
**Root cause:** Source evolution; stable was built before the v1→v2 binary header change.

**Note:** In run 3, the pending_label_ bug in .DUMMY blocks caused incorrect symbol table 
layout, which happened to produce byte 0x01 at this offset (matching stable by coincidence). 
Run 4 correctly produces 0x02 per the source code — this is an **improvement in correctness**, 
not a regression.

### Category 3: first_diff=0x0006 (20 files) — Missing S.PS.F.EVENT Symbol
**Files:** BIN/AARP, BIN/ARP, BIN/ETCONFIG, BIN/GOPHER, BIN/HTTPGET, BIN/IPCONFIG, 
BIN/IRC, BIN/KCONFIG, BIN/NTPDATE, BIN/PING, BIN/RPCDUMP, BIN/SHUTDOWN, BIN/TELNET, 
SBIN/BBSD, SBIN/CIFSD, SBIN/GUI, SBIN/HTTPD, SBIN/NETWORKD, SBIN/TELNETD, SBIN/VEDD

The source uses `.DA #S.PS.F.EVENT` in the binary header (a process flags constant). 
This symbol is referenced but **not defined** in the source files on disk (it was removed 
from INC/a2osx.i between the stable build and the current 335cd122 source).

**Stable byte[6]:** 0x02 (S.PS.F.EVENT was 2 in the old includes)  
**Built byte[6]:** 0x00 (symbol undefined → xasm++ evaluates to 0, no warning)

**Root cause:** Missing symbol definition in INC/a2osx.i — pre-existing issue unrelated to 
current fixes. Would be fixed by adding `S.PS.F.EVENT .EQ 2` to a2osx.i.

### Category 4: first_diff=0x0008 (45 files) — Code Address Table Differs
**Files:** BIN/ACC, BIN/ACOS, BIN/ATBROWSE, BIN/BF, BIN/CSH, BIN/CUT, DRV/*.DRV, LIB/*, etc.

These are standard executables (BIN Layout Version 1) where the first 8 bytes of the 
header match, but the code section addresses differ at offset 8+ (CS.END, ID.END, DS.END, etc.).

**Root cause:** The assembled code is slightly different in size from the stable reference.
Common causes: different macro expansion sizes, slightly different instruction sequences.
This category existed in run 3 as well (14 files at this offset in run 3, now 45 because 
6 moved from "not built" to "different" and more were moved from 0x0001 to 0x0008 as the 
pending_label_ fix corrected their assembly).

### Category 5: Special Cases

**SYS/KERNEL (first_diff=0x000d, stable=27716, built=28102):** Large complex file; built 
is 386 bytes larger than stable. First diff at offset 0x0d (13) — matches 13 bytes of header
then diverges. Kernel now assembles (was not-built in run 3).

**A2OSX.SYSTEM (first_diff=0x0028, stable=2561, built=2539):** The system boot stub.
Matches first 40 bytes (0x28) then diverges. Built is 22 bytes smaller than stable.
First_diff moved from 0x2f (run 3) to 0x28 (run 4) — slightly earlier divergence.

**LIB/LIBTUI.O (first_diff=0x0088, stable=219, built=219):** Same size (219 bytes),
first diff at offset 0x88 — only 1 byte differs at that offset. Very close to matching.

**SYS/PM.VEDRIVE (first_diff=0x0001):** Single file with diff at byte 1.

---

## Not Built (10 files)

These files cannot currently be assembled by xasm++ (assembly errors or unsupported macros):

| File | Reason |
|------|--------|
| BIN/ADM | Unsupported >PSTR / PR#3 macros |
| BIN/BASIC | Unsupported macros |
| BIN/BEEP | Build error |
| BIN/FIND | Build error |
| BIN/HELP | Build error |
| BIN/MKTEMP | Build error |
| BIN/WHICH | Build error |
| BIN/XMASTREE | Build error |
| SYS/PM.RAMWORKS | Not in cmake targets / no source |
| SYS/VEDRIVE.CONF | Configuration file, not assembled |

These 10 were also not built in run 3 (BIN/BEEP, SYS/PM.RAMWORKS, SYS/VEDRIVE.CONF 
were in run 3's not-built list too; the others remain the same).

---

## Key Findings

1. **A2OSX.LOGO is now identical** to stable — first file to be byte-for-byte correct.
2. **SYS/KERNEL now assembles** — the kernel was previously failing due to the pending_label_ 
   bug in .DUMMY blocks. This is the most significant improvement in run 4.
3. **BIN/FNT2FON now assembles** — fixed by the unterminated string handling for .CS/.CZ/.AS/.AT.
4. **HasOpcode fix** enabled BIN/ASM.65816 and BIN/ASM.65R02 to build.
5. **No regressions** — all previously-working files still work.
6. **39 files** moved from first_diff=0x0001 to 0x0005/0x0006/0x0008 — this reflects the 
   pending_label_ fix correcting the binary header layout from "accidentally wrong but matching 
   stable" to "correctly implementing the source". These binaries are now MORE correct per 
   source but differ from the stable which was built with older source.

---

## Delta vs. Stable (Size Perspective)

| Category | Count | Notes |
|----------|-------|-------|
| **Identical** | **3** | A2OSX.LOGO, LIB/LIBBLKDEV.O, LIB/LIBGUI.O |
| Different (header layout) | 6+44 | ASM tables + v2 header files |
| Different (event flag) | 20 | Missing S.PS.F.EVENT symbol |
| Different (code size) | 45+4 | Content differences, mostly close |
| Not built | 10 | Unsupported features |
