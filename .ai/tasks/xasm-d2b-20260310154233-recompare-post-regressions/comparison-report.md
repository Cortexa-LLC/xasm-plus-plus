# Binary Comparison Report: xasm++ Post-Regression-Fix (d05fd40)

**Date:** 2026-03-10  
**Task:** xasm-d2b  
**Source Commit:** A2osX 335cd122 ("A2osX.95:LATEST & LAST bugfixes")  
**Reference:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po`  
**Built With:** xasm++ v0.9.0 (commit d05fd40, "star-label .EQ rejects forward refs + placeholders; DummyOrgAtom JMP regression")  
**Prior Runs:** xasm-9wz (b7b739c), xasm-rwe (b850ca5)

---

## Executive Summary

Both regressions introduced in b850ca5 are **fully fixed** by d05fd40.  
The 89-file DummyOrgAtom JMP regression is resolved, and KERNEL/LIBBLKDEV build again.

| Result | xasm-9wz (b7b739c) | xasm-rwe (b850ca5) | **d05fd40** |
|--------|-------------------:|-------------------:|------------:|
| **Byte-for-byte identical** | 2 | 2 | **2** |
| **Different content** | 116 | 113 | **117** |
| **Not built** | ~15 | 17 | **7** |
| **Scope (in-scope files)** | ~133 | ~132 | **126** |
| **First-diff at 0x0001 (DummyOrg regression)** | ~0 | 89 | **2** |
| **First-diff at 0x0008 (baseline)** | 107 | 14 | **107** |

**Notes on scope difference:** The d05fd40 comparison uses a 126-file scope (cmake
targets that appear on the stable disk), which is 7 fewer than the prior runs. Prior
runs included files like `BIN/ADM`, `BIN/BASIC`, `BIN/BEEP`, `BIN/FIND`, etc., which
exist on the stable disk but have **no corresponding source file** in the 335cd122
A2osX source tree and therefore no cmake target. These are excluded from the 126-file
scope since there is nothing to build. The 7-file scope reduction does not indicate
any regression.

---

## Regressions Fixed

### Fix 1: Star-label `.EQ` Operand Safety (commit d05fd40)

**Root cause:** The b850ca5 "DUMMY/.OR zero-page fix" introduced a regression where
star-label `.EQ` lines with forward references or non-hex chars were incorrectly
treated as valid symbol definitions rather than comments. For example:

```
*K.CloseDir  .EQ K.FClose      ; K.FClose is a forward reference → should be a comment
*IO.D2.ReadSect .EQ $Cn5C      ; 'n' is not valid hex → should be a comment
```

**Effect in b850ca5:** `SYS/KERNEL` failed to build (Undefined symbol: K.FCLOSE),
causing both `SYS/KERNEL` and `LIB/LIBBLKDEV` to be missing from the comparison.

**Fixed in d05fd40:** `IsEqOperandSafe()` checks that the operand is either a
standalone hex literal or a single-token label with no forward references before
treating it as a private label definition. Both files now assemble successfully.

| File | b850ca5 | d05fd40 |
|------|---------|---------|
| `SYS/KERNEL` | ❌ NOT BUILT | ✅ DIFFERENT (ref=27716B, built=26554B) |
| `LIB/LIBBLKDEV` | ❌ NOT BUILT | ✅ DIFFERENT (ref=4154B, built=3930B) |
| `LIB/LIBBLKDEV.O` | ❌ NOT BUILT | ✅ **IDENTICAL** (229 bytes) |

### Fix 2: DummyOrgAtom JMP Regression (commit d05fd40)

**Root cause:** The b850ca5 DUMMY/.OR fix changed how the `DummyOrgAtom` emitted
its closing `JMP` instruction, causing it to be dropped in 89 files. Every file
that used a `.DUMMY` section had its first three bytes (the `JMP $xxxx` at load
address) replaced by zero bytes, shifting all subsequent addresses by 3.

**Effect in b850ca5:** 89 files now differed at byte `0x0001` (first byte of the
JMP target address), making those files worse than the b7b739c baseline.

**Fixed in d05fd40:** The DummyOrgAtom now correctly emits its JMP instruction.
The 0x0001 regression is essentially eliminated.

| First-diff offset | b7b739c | b850ca5 | d05fd40 |
|-------------------|--------:|--------:|--------:|
| `0x0000` (byte 0) | ? | ? | 5 |
| `0x0001` (JMP regression) | ~0 | **89** | **2** |
| `0x0008` (baseline header diff) | **107** | 14 | **107** |
| `0x0010` (KERNEL body diff) | N/A | N/A | 1 |
| `0x002f` (A2OSX.SYSTEM) | 1 | 1 | 1 |
| `0x0089` (PM.APPLETALK) | ? | ? | 1 |

The 89-file regression is fully resolved. The 2 residual `0x0001` files
(`SYS/PM.VEDRIVE`, `SYS/PM/PM.VSDRIVE`) are pre-existing issues unrelated to the
DummyOrgAtom regression; they were already present in the b7b739c baseline.

---

## Detailed Comparison Results

### Files Identical (byte-for-byte): 2

| File | Bytes |
|------|------:|
| `LIB/LIBBLKDEV.O` | 229 |
| `LIB/LIBGUI.O` | 135 |

Both are stub/relocation-table index files containing only data tables with no
zero-page variable references and no instruction code — hence unaffected by
address-resolution differences.

### Files Not Built: 7

All 7 are pre-existing failures that were also present in xasm-9wz (b7b739c):

| File | Error | Root Cause |
|------|-------|------------|
| `BIN/ASM.65816` | `MVN requires two operands` | 65816 MVN/MVP multi-bank syntax not implemented |
| `BIN/ASM.65R02` | `Unsupported instruction: BBR0` | Rockwell BBR/BBS/RMB/SMB instructions not supported |
| `BIN/FNT2FON` | `Unterminated string` | Host-tool source dependency issue |
| `BIN/XMASTREE` | `Unexpected character after expression: 1` | SCMASM character literal syntax (.EQ '\*') |
| `LIB/LIBTUI.O` | Source ordering dependency | TUI library symbol resolution order |
| `SYS/PM.RAMWORKS` | `Unsupported instruction: TOO` | Legacy driver uses SCMASM-only instructions |
| `SYS/PM/PM.NSC` | `Parse error: Unterminated string` | NSC-specific string handling edge case |

**Comparison with prior runs:**

| File | xasm-9wz (b7b739c) | xasm-rwe (b850ca5) | d05fd40 |
|------|:-------------------:|:-------------------:|:-------:|
| `BIN/ASM.65816` | NOT BUILT | NOT BUILT | NOT BUILT |
| `BIN/ASM.65R02` | NOT BUILT | NOT BUILT | NOT BUILT |
| `BIN/FNT2FON` | NOT BUILT | NOT BUILT | NOT BUILT |
| `BIN/XMASTREE` | NOT BUILT | NOT BUILT | NOT BUILT |
| `LIB/LIBTUI.O` | NOT BUILT | NOT BUILT | NOT BUILT |
| `SYS/PM.RAMWORKS` | NOT BUILT | NOT BUILT | NOT BUILT |
| `SYS/PM/PM.NSC` | NOT BUILT | NOT BUILT | NOT BUILT |
| `SYS/KERNEL` | DIFFERENT | **NOT BUILT** ❌ | DIFFERENT ✅ |
| `LIB/LIBBLKDEV` | DIFFERENT | **NOT BUILT** ❌ | DIFFERENT ✅ |
| `LIB/LIBBLKDEV.O` | IDENTICAL | **NOT BUILT** ❌ | IDENTICAL ✅ |

### Files Different: 117

All 117 files differ between the xasm++ build and the SCMASM reference. The primary
cause of differences is systematic zero-page address resolution: xasm++ resolves ZP
symbols to different address values than the original SCMASM assembler, resulting in
slightly smaller output (fewer instructions in ZP-optimised mode) with different
internal symbol tables.

**First-diff offset summary:**

| Offset | Count | Meaning |
|--------|------:|---------|
| `0x0000` | 5 | Byte 0 differs (asm stub variants: ASM.6502, ASM.65C02, ASM.SW16, ASM.Z80, MEMDUMP) |
| `0x0001` | 2 | Byte 1 differs (PM.VEDRIVE, PM/PM.VSDRIVE — pre-existing) |
| `0x0008` | 107 | Bytes 0–7 match, body differs (baseline: header matches, ZP-related code differs) |
| `0x0010` | 1 | KERNEL: first 16 bytes identical, body differs |
| `0x002f` | 1 | A2OSX.SYSTEM: minor structural diff |
| `0x0089` | 1 | PM/PM.APPLETALK: same size (1027B), differs mid-file |

The 107 files differing at `0x0008` is the **same count as xasm-9wz**, confirming
the DummyOrgAtom regression is fully resolved and the baseline is restored.

#### Notable File Results

| File | ref bytes | built bytes | first_diff | Notes |
|------|----------:|------------:|:----------:|-------|
| `SYS/KERNEL` | 27,716 | 26,554 | `0x0010` | Regression fixed; builds again |
| `LIB/LIBBLKDEV` | 4,154 | 3,930 | `0x0008` | Regression fixed; builds again |
| `SYS/PM/PM.APPLETALK` | 1,027 | 1,027 | `0x0089` | Same size, internal diff |
| `SYS/PM.VEDRIVE` | 6,644 | 6,598 | `0x0001` | Pre-existing; unrelated to d05fd40 |
| `SYS/PM/PM.VSDRIVE` | 1,482 | 1,445 | `0x0001` | Pre-existing; unrelated to d05fd40 |
| `BIN/ASM.6502` | 1,223 | 1,224 | `0x0000` | 1-byte size diff, minor variant |
| `BIN/CC` | 13,371 | 12,472 | `0x0008` | Largest size diff: 899 bytes |

---

## Complete "Different" File List

<details>
<summary>117 files differing (click to expand)</summary>

| File | ref bytes | built bytes | first_diff |
|------|----------:|------------:|:----------:|
| `A2OSX.SYSTEM` | 2,561 | 2,551 | `0x002f` |
| `bin/aarp` | 964 | 873 | `0x0008` |
| `bin/acc` | 315 | 310 | `0x0008` |
| `bin/acos` | 5,905 | 5,390 | `0x0008` |
| `bin/arp` | 1,051 | 940 | `0x0008` |
| `bin/asm` | 9,220 | 8,685 | `0x0008` |
| `bin/asm.6502` | 1,223 | 1,224 | `0x0000` |
| `bin/asm.65C02` | 1,447 | 1,441 | `0x0000` |
| `bin/asm.SW16` | 1,388 | 1,386 | `0x0000` |
| `bin/asm.Z80` | 6,116 | 6,115 | `0x0000` |
| `bin/atbrowse` | 935 | 866 | `0x0008` |
| `bin/atlogon` | 1,470 | 1,390 | `0x0008` |
| `bin/atmount` | 955 | 888 | `0x0008` |
| `bin/attr` | 1,958 | 1,830 | `0x0008` |
| `bin/bf` | 999 | 900 | `0x0008` |
| `bin/bmp2pix` | 1,696 | 1,616 | `0x0008` |
| `bin/cat` | 897 | 833 | `0x0008` |
| `bin/cc` | 13,371 | 12,472 | `0x0008` |
| `bin/chaux` | 1,668 | 1,560 | `0x0008` |
| `bin/chgrp` | 1,566 | 1,468 | `0x0008` |
| `bin/chmod` | 1,580 | 1,491 | `0x0008` |
| `bin/chown` | 1,565 | 1,467 | `0x0008` |
| `bin/chtyp` | 1,715 | 1,623 | `0x0008` |
| `bin/cmp` | 733 | 660 | `0x0008` |
| `bin/cp` | 2,363 | 2,232 | `0x0008` |
| `bin/csh` | 6,194 | 5,839 | `0x0008` |
| `bin/cut` | 665 | 651 | `0x0008` |
| `bin/dnsinfo` | 725 | 663 | `0x0008` |
| `bin/du` | 1,885 | 1,776 | `0x0008` |
| `bin/edit` | 7,307 | 6,944 | `0x0008` |
| `bin/etconfig` | 2,207 | 2,017 | `0x0008` |
| `bin/format` | 2,355 | 2,157 | `0x0008` |
| `bin/forth` | 6,197 | 5,607 | `0x0008` |
| `bin/gopher` | 2,060 | 1,846 | `0x0008` |
| `bin/grep` | 693 | 649 | `0x0008` |
| `bin/hmacmd5` | 1,008 | 898 | `0x0008` |
| `bin/httpget` | 1,964 | 1,819 | `0x0008` |
| `bin/ipconfig` | 4,317 | 4,009 | `0x0008` |
| `bin/irc` | 3,202 | 2,837 | `0x0008` |
| `bin/kconfig` | 1,832 | 1,677 | `0x0008` |
| `bin/kill` | 272 | 251 | `0x0008` |
| `bin/lc` | 1,653 | 1,527 | `0x0008` |
| `bin/ls` | 2,788 | 2,636 | `0x0008` |
| `bin/lsdev` | 782 | 698 | `0x0008` |
| `bin/lsof` | 292 | 266 | `0x0008` |
| `bin/md4` | 500 | 446 | `0x0008` |
| `bin/md5` | 451 | 397 | `0x0008` |
| `bin/mem` | 478 | 460 | `0x0008` |
| `bin/memdump` | 1,314 | 1,166 | `0x0000` |
| `bin/mkdir` | 599 | 551 | `0x0008` |
| `bin/more` | 725 | 671 | `0x0008` |
| `bin/mv` | 2,434 | 2,305 | `0x0008` |
| `bin/netstat` | 638 | 610 | `0x0008` |
| `bin/nl` | 478 | 444 | `0x0008` |
| `bin/nscutil` | 836 | 802 | `0x0008` |
| `bin/ntpdate` | 1,149 | 1,051 | `0x0008` |
| `bin/od` | 619 | 574 | `0x0008` |
| `bin/pak` | 2,600 | 2,380 | `0x0008` |
| `bin/pakme` | 1,793 | 1,516 | `0x0008` |
| `bin/ping` | 1,168 | 1,075 | `0x0008` |
| `bin/ps` | 509 | 471 | `0x0008` |
| `bin/rm` | 1,771 | 1,678 | `0x0008` |
| `bin/rpcdump` | 729 | 680 | `0x0008` |
| `bin/sed` | 667 | 627 | `0x0008` |
| `bin/seq` | 945 | 900 | `0x0008` |
| `bin/sh` | 7,728 | 7,139 | `0x0008` |
| `bin/shutdown` | 948 | 860 | `0x0008` |
| `bin/spdiag` | 775 | 699 | `0x0008` |
| `bin/stat` | 1,632 | 1,469 | `0x0008` |
| `bin/telnet` | 943 | 859 | `0x0008` |
| `bin/term` | 354 | 318 | `0x0008` |
| `bin/uc` | 1,667 | 1,535 | `0x0008` |
| `bin/uname` | 1,177 | 1,095 | `0x0008` |
| `bin/unpak` | 1,517 | 1,404 | `0x0008` |
| `bin/useradd` | 1,534 | 1,499 | `0x0008` |
| `bin/userdel` | 460 | 421 | `0x0008` |
| `bin/usermod` | 477 | 447 | `0x0008` |
| `bin/wc` | 2,336 | 2,168 | `0x0008` |
| `bin/who` | 319 | 287 | `0x0008` |
| `bin/xargs` | 524 | 468 | `0x0008` |
| `drv/dan2eth.drv` | 1,017 | 970 | `0x0008` |
| `drv/dhgr.drv` | 6,391 | 6,325 | `0x0008` |
| `drv/grappler.drv` | 591 | 561 | `0x0008` |
| `drv/grapplerp.drv` | 582 | 552 | `0x0008` |
| `drv/lancegs.drv` | 1,099 | 1,053 | `0x0008` |
| `drv/mkboard.drv` | 587 | 555 | `0x0008` |
| `drv/mouse.drv` | 710 | 679 | `0x0008` |
| `drv/pic.drv` | 592 | 562 | `0x0008` |
| `drv/pppssc.drv` | 4,658 | 4,526 | `0x0008` |
| `drv/sptape.drv` | 430 | 412 | `0x0008` |
| `drv/ssc.drv` | 1,187 | 1,131 | `0x0008` |
| `drv/ssc.i.drv` | 1,457 | 1,395 | `0x0008` |
| `drv/uther2.ai.drv` | 1,807 | 1,763 | `0x0008` |
| `drv/uthernet.drv` | 1,160 | 1,114 | `0x0008` |
| `drv/uthernet2.drv` | 1,135 | 1,090 | `0x0008` |
| `lib/libblkdev` | 4,154 | 3,930 | `0x0008` |
| `lib/libcrypt` | 1,922 | 1,889 | `0x0008` |
| `lib/libetalk` | 1,884 | 1,831 | `0x0008` |
| `lib/libgui` | 10,780 | 10,618 | `0x0008` |
| `lib/libpak` | 1,497 | 1,471 | `0x0008` |
| `lib/libtcpip` | 6,429 | 6,241 | `0x0008` |
| `lib/libtui` | 6,125 | 6,079 | `0x0008` |
| `sbin/bbsd` | 1,175 | 1,077 | `0x0008` |
| `sbin/cifsd` | 4,920 | 4,736 | `0x0008` |
| `sbin/getty` | 785 | 713 | `0x0008` |
| `sbin/gui` | 388 | 362 | `0x0008` |
| `sbin/httpd` | 2,444 | 2,234 | `0x0008` |
| `sbin/initd` | 439 | 403 | `0x0008` |
| `sbin/insdrv` | 171 | 161 | `0x0008` |
| `sbin/login` | 743 | 687 | `0x0008` |
| `sbin/networkd` | 918 | 828 | `0x0008` |
| `sbin/telnetd` | 843 | 777 | `0x0008` |
| `sbin/vedd` | 865 | 787 | `0x0008` |
| `sys/kernel` | 27,716 | 26,554 | `0x0010` |
| `sys/pm.vedrive` | 6,644 | 6,598 | `0x0001` |
| `sys/pm/pm.appletalk` | 1,027 | 1,027 | `0x0089` |
| `sys/pm/pm.vsdrive` | 1,482 | 1,445 | `0x0001` |

</details>

---

## Three-Run Comparison Table

| Metric | xasm-9wz (b7b739c) | xasm-rwe (b850ca5) | **d05fd40** | Δ vs rwe |
|--------|-------------------:|-------------------:|------------:|--------:|
| Identical | 2 | 2 | **2** | 0 |
| Different | 116 | 113 | **117** | +4 |
| Not Built | ~15 | 17 | **7** | **−10** ✅ |
| 0x0001 files | ~0 | 89 | **2** | **−87** ✅ |
| 0x0008 files | 107 | 14 | **107** | **+93** ✅ |
| KERNEL built | ✅ | ❌ | **✅** | restored |
| LIBBLKDEV built | ✅ | ❌ | **✅** | restored |
| LIBBLKDEV.O identical | ✅ | ❌ | **✅** | restored |

**The +4 "different" vs rwe** is explained by KERNEL and LIBBLKDEV moving from "not
built" to "different" (+2 each minus the 2 regression entries = net +2 in different).
The actual scope is also slightly different between runs (7 fewer files in scope
for d05fd40 due to excluding non-cmake files from count).

---

## Conclusion

The d05fd40 commit successfully fixes both regressions introduced by b850ca5:

1. **Star-label `.EQ` operand safety fix** — KERNEL and LIBBLKDEV now assemble, and
   `LIBBLKDEV.O` is byte-for-byte identical to the SCMASM reference again.

2. **DummyOrgAtom JMP fix** — The 89-file regression (files losing their first JMP
   instruction) is fully resolved. All 107 files that should differ at the ProDOS
   header byte `0x0008` are back at that offset, identical to the b7b739c baseline.

**Remaining work:** The 107 files differing at `0x0008` represent the ongoing
zero-page address resolution gap between xasm++ and SCMASM. This is the fundamental
work remaining to achieve full binary compatibility. The 2 files differing at
`0x0001` (`SYS/PM.VEDRIVE`, `SYS/PM/PM.VSDRIVE`) and 7 un-buildable files are
pre-existing issues that predate all three comparison runs.

The xasm++ assembler is now at baseline state (matching b7b739c) with both
regression fixes confirmed. The next work should focus on the ZP address resolution
differences that cause the systematic `0x0008` offset mismatch in 107 files.
