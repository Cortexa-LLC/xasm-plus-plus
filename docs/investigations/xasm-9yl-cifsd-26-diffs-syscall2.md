# Bug Investigation Retrospective: xasm-9yl — sbin/cifsd: 26 diffs (same-size)

**Status:** Resolved — Expected Behavior (API Version Change)
**Severity:** P2 (Informational — no bug, confirmed expected difference)
**Reported:** 2026-03-12
**Resolved:** 2026-03-12
**Systems Affected:** `sbin/cifsd` (CIFS/SMB daemon for A2osX)
**Root Cause Category:** Kernel API Address Relocation (not a code bug)
**Task ID:** xasm-9yl
**Related Task:** xasm-hsf (predecessor), xasm-siu (bin/mv investigation), xasm-81v (bin/mv fix verification)

---

## Bug Summary

`sbin/cifsd` binary produced by the xasm++ build system (from A2osX commit `335cd122`)
has exactly **26 byte differences** vs the stable reference binary (from `STABLE.800.po`
disk image), yet both binaries are exactly the same size: **4920 bytes**.

The initial hypothesis was "SYSCALL-only" differences. This investigation fully confirms
that hypothesis: all 26 diffs are SYSCALL address bytes.

---

## Reproduction

```bash
STABLE="/tmp/stable_extracted/FULL800/SBIN/cifsd#062000"
BUILT="/tmp/A2osX-335cd122-build/stage/sbin/cifsd"

# Verify same size
wc -c "$STABLE" "$BUILT"
# Both: 4920

# Show all 26 diffs
cmp -l "$STABLE" "$BUILT"
```

**Output (all 26 byte differences):**

| cmp offset (1-based) | Stable (octal) | New (octal) | Stable (hex) | New (hex) |
|---|---|---|---|---|
| 256 | 0 | 123 | 0x00 | 0x53 |
| 257 | 342 | 1 | 0xE2 | 0x01 |
| 1302 | 0 | 123 | 0x00 | 0x53 |
| 1303 | 342 | 1 | 0xE2 | 0x01 |
| 1314 | 0 | 123 | 0x00 | 0x53 |
| 1315 | 342 | 1 | 0xE2 | 0x01 |
| 1613 | 0 | 123 | 0x00 | 0x53 |
| 1614 | 342 | 1 | 0xE2 | 0x01 |
| 1702 | 0 | 123 | 0x00 | 0x53 |
| 1703 | 342 | 1 | 0xE2 | 0x01 |
| 1805 | 0 | 123 | 0x00 | 0x53 |
| 1806 | 342 | 1 | 0xE2 | 0x01 |
| 2314 | 0 | 123 | 0x00 | 0x53 |
| 2315 | 342 | 1 | 0xE2 | 0x01 |
| 2395 | 0 | 123 | 0x00 | 0x53 |
| 2396 | 342 | 1 | 0xE2 | 0x01 |
| 4086 | 0 | 123 | 0x00 | 0x53 |
| 4087 | 342 | 1 | 0xE2 | 0x01 |
| 4137 | 0 | 123 | 0x00 | 0x53 |
| 4138 | 342 | 1 | 0xE2 | 0x01 |
| 4176 | 0 | 123 | 0x00 | 0x53 |
| 4177 | 342 | 1 | 0xE2 | 0x01 |
| 4208 | 0 | 123 | 0x00 | 0x53 |
| 4209 | 342 | 1 | 0xE2 | 0x01 |
| 4227 | 0 | 123 | 0x00 | 0x53 |
| 4228 | 342 | 1 | 0xE2 | 0x01 |

---

## Root Cause

**All 26 diffs are the 2-byte JSR target address of `A2osX.SYSCALL2` calls.**

### Detailed Analysis

The 26 byte diffs form exactly **13 consecutive byte-pairs**, all with identical transformation:
- Stable bytes: `[0x00, 0xE2]` (little-endian address **0xE200**)
- New bytes: `[0x53, 0x01]` (little-endian address **0x0153**)

Each diff pair is **always preceded by opcode `0x20` (JSR)** at offset-1, confirming
these are JSR instruction target addresses.

### JSR Target Address Map (new binary, 335cd122)

| Address | Count | Meaning (from `INC/A2osX.I.txt` @ 335cd122) |
|---|---|---|
| 0x0140 | 22 | `A2osX.SYSCALL` = `A2osX.GP` + 0 |
| 0x0143 | 9 | `A2osX.LIBCALL` = `A2osX.GP` + 3 |
| 0x0150 | 3 | `A2osX.SLEEP` = `A2osX.GP` + 16 |
| **0x0153** | **13** | **`A2osX.SYSCALL2` = `A2osX.GP` + 19** |

The 13 SYSCALL2 calls correspond to the 13 diff pairs.

### Version-to-Version API Address Comparison

Both binaries use identical addresses for `SYSCALL`, `LIBCALL`, and `SLEEP` —
only `SYSCALL2` differs:

| API Entry | Stable Binary | New Binary (335cd122) |
|---|---|---|
| `A2osX.SYSCALL` | `JSR 0x0140` | `JSR 0x0140` (same) |
| `A2osX.LIBCALL` | `JSR 0x0143` | `JSR 0x0143` (same) |
| `A2osX.SLEEP` | `JSR 0x0150` | `JSR 0x0150` (same) |
| `A2osX.SYSCALL2` | `JSR 0xE200` | `JSR 0x0153` (DIFFERENT) |

**The stable binary used `SYSCALL2 = 0xE200`** — a far address outside the GP table.  
**The new binary uses `SYSCALL2 = 0x0153`** — GP+19, properly placed in the GP dispatch table.

This is a kernel API reorganization between A2osX versions where `SYSCALL2` was moved
from an ad-hoc address `0xE200` into the `A2osX.GP` global page dispatch table.

### Source Evidence

From `INC/A2osX.I.txt` at commit `335cd122`:
```
A2osX.GP    .EQ $0140
A2osX.SYSCALL  .EQ A2osX.GP+00   ; = 0x0140
A2osX.SYSCALL2 .EQ A2osX.GP+19   ; = 0x0153
```

From `INC/MACROS.I.txt` at commit `335cd122`:
```
.MA SYSCALL
  ldx #_]1
  jsr A2osX.SYSCALL
.EM

.MA SYSCALL2
  ldx #_]1
  jsr A2osX.SYSCALL2
.EM
```

From `SBIN/CIFSD.S.txt` at commit `335cd122`: 13 occurrences of `>SYSCALL2` macro.

---

## Fix Applied

**No fix required.** This is expected behavior.

The stable reference binary was built from an older A2osX version where `SYSCALL2`
lived at address `0xE200`. The new binary is correctly built from `335cd122` where
`SYSCALL2` has been moved into the GP dispatch table at `0x0153`.

The assembler (`xasm++`) is producing correct output. The SYSCALL macro expansion
is working correctly. All 26 diffs are fully accounted for.

---

## Why Tests Missed It

This is not a bug — no test coverage was needed. The "diffs" are intentional
API-version differences between a reference (stable) binary and the current build.
The investigation process worked correctly: initial hypothesis of "SYSCALL-only"
was confirmed exactly.

---

## Investigation Methodology

1. Located both binaries: stable (disk image extract) and new (335cd122 build)
2. Confirmed same size: 4920 bytes each
3. Used `cmp -l` to enumerate all 26 differing bytes
4. Observed all diffs occur in consecutive pairs
5. Verified each pair is preceded by `0x20` (JSR opcode)
6. Decoded pair bytes as little-endian 16-bit addresses
7. Confirmed all 13 pairs have identical stable→new transformation
8. Cross-referenced addresses with `INC/A2osX.I.txt` from 335cd122
9. Confirmed `0x0153` = `A2osX.SYSCALL2` (GP+19, GP=0x0140)
10. Verified JSR address distribution in both binaries confirms the mapping
11. Identified that stable binary used old `SYSCALL2=0xE200` (pre-GP-table era)
12. Counted SYSCALL2 macro calls in source: exactly 13 (matches 13 diff pairs)

---

## Lessons Learned

- `SYSCALL2` was at some point an ad-hoc far-page address (`0xE200`) before being
  moved into the `A2osX.GP` dispatch table at offset +19.
- Same-size binary diffs with uniform byte-pair patterns are a reliable indicator
  of SYSCALL address changes between A2osX kernel versions.
- The `cmp -l` tool with Python analysis is effective for confirming SYSCALL-only diffs.

---

## Similar Patterns

- `xasm-siu` / `xasm-81v`: `bin/mv` had 1042-diff same-size — confirmed assembler bug + SYSCALL address changes. Different root cause (assembler bug was genuine there).
- `bin/du`: +2B investigation — size difference, not same-size.

---

## Related Documents

- Architecture: `docs/architecture/` (A2osX kernel GP dispatch table)
- Similar Bugs: [xasm-81v: bin/mv 1042-diff investigation](./xasm-81v-binmv-1042-diffs.md) (if exists)
- Original Bug Report: Beads task `xasm-9yl`
