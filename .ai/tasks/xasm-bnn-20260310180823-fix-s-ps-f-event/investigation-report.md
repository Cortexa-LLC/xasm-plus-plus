# Investigation Report: Missing S.PS.F.EVENT Symbol

**Task:** xasm-bnn  
**Date:** 2026-03-11  
**Status:** ✅ FIXED

## Problem Summary

20 A2osX binary files differed at byte offset 6 (0x0006) from the STABLE reference disk.
The stable reference had value `0x02`; xasm++ produced `0x00` (undefined symbol → 0).

**Affected files:** BIN/AARP, BIN/ARP, BIN/ETCONFIG, BIN/GOPHER, BIN/HTTPGET, BIN/IPCONFIG,
BIN/IRC, BIN/KCONFIG, BIN/NTPDATE, BIN/PING, BIN/RPCDUMP, BIN/SHUTDOWN, BIN/TELNET,
SBIN/BBSD, SBIN/CIFSD, SBIN/GUI, SBIN/HTTPD, SBIN/NETWORKD, SBIN/TELNETD, SBIN/VEDD

## Root Cause Analysis

### Where S.PS.F.EVENT is Used

All 20 affected source files contain a binary header pattern at the top:

```asm
CS.START    cld
            jmp (.1,x)
            .DA #$61                ; CPU level (6502)
            .DA #1                  ; BIN Layout Version 1
            .DA #S.PS.F.EVENT       ; Process flags byte ← byte[6]
            ...
```

The `.DA #S.PS.F.EVENT` at line 23 of each source file produces byte[6] of the binary.
When `S.PS.F.EVENT` is undefined, xasm++ evaluates it as 0 → byte[6] = 0x00.

### Where the Symbol Should Be Defined

The symbol lives in the process-flags section of the A2osX INC file (`INC/A2osX.I.txt`).

**Original 335cd122 commit** (`/tmp/A2osX-335cd122/INC/A2osX.I.txt`) already had the symbol
at line 528:
```
S.PS.F.CLOSEONX     .EQ %00000100
S.PS.F.EVENT        .EQ %00000010   ← present in 335cd122
S.PS.F.NOHUP        .EQ %00000001
```

**However**, the `xasm++_stable` branch (`/Users/bryanw/Projects/Vintage/Apple/A2osX/`) was
**missing** this symbol. The `xasm++_stable` branch (`git show xasm++_stable:INC/A2osX.I.txt`)
showed only:
```
S.PS.F.CLOSEONX     .EQ %00000100
S.PS.F.NOHUP        .EQ %00000001   ← S.PS.F.EVENT was missing!
```

The binary comparison tests assemble `xasm++_stable` branch source files, so the symbol
being absent in that branch caused xasm++ to treat it as undefined (= 0).

## Fix Applied

**File modified:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/A2osX.I.txt`

**Line added:** Line 729 — inserted `S.PS.F.EVENT .EQ %00000010` between
`S.PS.F.CLOSEONX` and `S.PS.F.NOHUP`.

```diff
 S.PS.F.CLOSEONX     .EQ %00000100
+S.PS.F.EVENT        .EQ %00000010
 S.PS.F.NOHUP        .EQ %00000001
```

The staged file `build/stage/inc/a2osx.i` was also updated (derived from source).

## Verification

Rebuilt BIN/AARP and BIN/ARP from source and confirmed:

| File | Before fix | After fix | Expected |
|------|-----------|-----------|----------|
| bin/aarp | 0x00 | **0x02** | 0x02 ✅ |
| bin/arp  | 0x00 | **0x02** | 0x02 ✅ |

Byte[6] is now `0x02` = `S.PS.F.EVENT` value (`%00000010`), matching the STABLE reference.

## Files Changed

- `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/A2osX.I.txt` — added `S.PS.F.EVENT .EQ %00000010` at line 729

## Suggested Commit Message

```
fix: add S.PS.F.EVENT .EQ %00000010 to INC/A2osX.I.txt

Symbol was missing from xasm++_stable branch, causing 20 binary files
to produce 0x00 at byte[6] instead of 0x02 (process flags field).

Fixes: 20-file diff@0x0006 in binary comparison run (xasm-adr).
```
