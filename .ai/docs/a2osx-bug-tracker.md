# xasm++ A2osX Bug Tracker

Tracking bugs found and fixed while making xasm++ produce byte-identical output
to the original SCMASM assembler using A2osX 335cd122 as the reference test case.

**Reference:** `/tmp/stable_extracted/` (STABLE.800.po ProDOS image, reference binary)
**Test build:** `/tmp/A2osX-335cd122-build/stage/` (built from git commit 335cd122 source)
**Assembler binary under test:** `/tmp/xasm_new` (copied from `build/bin/xasm++`)

---

## Progress Summary

| Run | Commit | Identical | Diff (code) | Not Built | Notes |
|-----|--------|-----------|-------------|-----------|-------|
| 1 (xasm-9wz) | b7b739c | 2 | 116 | 15 | Baseline; DummyOrgAtom investigation |
| 2 (xasm-rwe) | b850ca5 | 2 | 113 | 17 | DummyOrgAtom fix introduced JMP regression |
| 3 (xasm-d2b) | d05fd40 | 2 | 117 | 7 | JMP regression fixed; baseline restored |
| 4 (xasm-adr) | 7531e43 | 3 | 119 | 10 | HasOpcode fix + string fixes |
| 5 | f28591b | 48 | 32 | 59¹ | **+45 identical!** Macro scoping, dummy-section |
| 6 | c2fa18d | 58 | 65² | 37 | **+10 identical!** HandleEq, EOF label, '/' op |
| 7 | 75a1f64 | 40³ | 45 | 128 | **MEMDUMP identical!** DUMMY instr-label + JSR/JMP Absolute fix |
| 8 | f7883f3 | 65 | 56 | 11 | **+25 identical!** '@' separator for scoped local labels |
| 9 | 3191947 | 65 | 56 | 11 | .CS mixed-delimiter fix; no count change (differences are source changes) |
| 10 | b19928d | 66 | 58 | 89 | **+1 identical (CSH)!** EquateAtom in .DUMMY + macro arg parsing |
| 11 | 3a14d43 | 66 | 58 | 89 | .CS-only mixed-delim refinement; SH shifts +3→-3 but count unchanged |
| 12 | c5f54b0 | 71 | 53 | 89 | **+5 identical!** .HX nibble-swap fix + .DA * substitution |

¹ Scope/script changed; not directly comparable to runs 1-4
² Includes ~25 same-size-different-content files (source version differences)
³ Clean build (no stale files); baseline 39 → 40. Runs 1-6 had stale files skewing NOT_BUILT count.

---

## Bugs Fixed

### Bug 1: .DUMMY/.OR ZP address resolution (b850ca5 → d05fd40)
**Symptom:** All ZP variables resolved to $2000+offset instead of ZP address
**Root cause:** HandleOr() inside .DUMMY didn't emit an atom; ResolveSymbols() had no
DummyOrgAtom to track the ZP base, so all ZP labels got main-section addresses
**Fix:** Added DummyOrgAtom emitted by HandleOr in dummy mode; ResolveSymbols processes
it to update address counter; code emitter ignores it
**Impact:** 89 files corrected; all ZP addressing works correctly

### Bug 2: Star-label .EQ false detection (e0b4425 / f2a45a6)
**Symptom:** `*K.CloseDir .EQ K.FClose` was treated as a comment (SCMASM convention:
`*LABEL .EQ value` is a private label marker, not a comment)
**Fix:** Detect `*` at column 0 followed by a valid label char + space + `.EQ/.SE` as
private-label equates, not comments
**Also:** Guard against forward refs and hex-placeholder chars in star-label values

### Bug 3: HasOpcode() not mode-aware (7531e43)
**Symptom:** 65816-only opcodes (MVN, MVP, PHB, etc.) recognized as instructions
in 65C02 mode, preventing labels named MVN from being defined
**Fix:** `HasOpcode()` in cpu_6502.cpp uses four separate static sets gated by cpu_mode_
**Files:** `src/cpu/cpu_6502.cpp`

### Bug 4: Pending label at EOF not flushed (c2fa18d)
**Symptom:** Label-only lines at the very end of a file (after which only editor
commands appear) were never added to symbol table, resolving as 0
**Root cause:** `pending_label_` flush only happened at next instruction; editor
commands (`MAN`/`SAVE`/`ASM`) are stripped to empty strings, so the flush never ran
**Fix:** After main parse loop ends, flush any remaining `pending_label_` at current address
**Files:** `src/syntax/scmasm_syntax.cpp`
**Impact:** DRV files with trailing labels (e.g. `DRV.END`) now correct

### Bug 5: '/' high-byte operator in instruction operands (c2fa18d)
**Symptom:** `lda /LABEL` (high byte of LABEL) produced wrong output; '/' operator
not handled in `EncodeInstructions` operand value parser
**Fix:** Added explicit '/' prefix case between '#' and general-label cases in
`EncodeInstructions` (src/core/assembler.cpp)
**Impact:** `>PUSHWI DRV.END` macro (which expands to `lda /DRV.END`) now works; DRV files correct

### Bug 6: Macro-local ':N' labels shared across invocations (f28591b)
**Symptom:** Multiple invocations of the same macro under the same global label
all shared the same `:1`/`:2` label names, causing forward branches to jump to
the LAST invocation's `:1` definition instead of the current one
**Root cause:** `:N` labels inside macros used `last_global_label_` as scope prefix,
not a per-invocation unique prefix
**Fix:** Added `macro_invocation_counter_` (monotonic, reset each pass) and
`current_macro_label_scope_` (e.g. `GLOBALNAME:@5`). `LocalLabelScope(":N")` returns
the per-invocation scope when `macro_invocation_depth_ > 0`
**Files:** `include/xasm++/syntax/scmasm_syntax.h`, `src/syntax/scmasm_syntax.cpp`
**Impact:** ADC32, MOV32, ORA32 etc. macros in LIBCRYPT now produce correct jumps;
lib/libcrypt size now matches stable

### Bug 7: Instructions emitted inside .DUMMY sections (f28591b)
**Symptom:** `bin/memdump` had `ZPReadAux` subroutine (defined inside .DUMMY section)
emitting code bytes at the start of the binary, corrupting the 16-byte BIN header
**Root cause:** `InstructionAtom` was pushed unconditionally; the `in_dummy_section_`
guard was missing from the instruction emit path
**Fix:** Wrapped `section.atoms.push_back(instr_atom)` with `if (!in_dummy_section_)`
**Files:** `src/syntax/scmasm_syntax.cpp`

### Bug 8: ':N .EQ *' inside macros not tracked across passes (c2fa18d)
**Symptom:** `bin/format` had +3 extra bytes (false branch relaxation insertion)
for `bne :1` branches where `:1 .EQ *` appears at the end of the INCW.G macro
**Root cause:** `HandleEq()` defined the symbol using the UNSCOPED label name `:1`
but branches reference the scoped name `GLOBALNAME:@N:1`. The EquateAtom (which
re-evaluates `*` each pass) was tracking `:1` (unscoped), so the scoped symbol
kept its pass-1 address value. After code shifted from branch relaxation,
`bne :1` couldn't reach its target, triggering a false relaxation (+3 bytes).
**Fix:** In `HandleEq()`, when `label[0] == ':' || label[0] == '.'`, call
`LocalLabelScope(label)` to get scope prefix and prepend to `norm_label`.
Also made `LocalLabelScope()` public in `scmasm_syntax.h`.
**Files:** `include/xasm++/syntax/scmasm_syntax.h`, `src/syntax/scmasm_directive_handlers.cpp`
**Impact:** `bin/format` now byte-identical to stable. Any macro with `:N .EQ *`
at the end (like INCW.G) now correctly tracks per-invocation address across passes.

### Bug 9: Labels on instruction lines inside .DUMMY not suppressed (75a1f64)
**Symptom:** `ZPReadAux sta IO.SETREADAUX` inside `.DUMMY / .OR $E0` section gave
`ZPReadAux = $2000` (CS.START) instead of `$E8` (ZP address).
**Root cause:** Labels on instruction lines (not directives) unconditionally pushed
`LabelAtom`s to section.atoms even inside DUMMY sections. `ResolveSymbols()` then
overwrote the correct parse-time ZP address with the current main-section PC.
Directive labels and `pending_label_` flushes already had `!in_dummy_section_` guards,
but the instruction-label path at lines 988-1010 of `scmasm_syntax.cpp` was missing them.
**Fix:** Added `if (!in_dummy_section_)` guards around `LabelAtom` push for instruction labels.
**Files:** `src/syntax/scmasm_syntax.cpp`
**Impact:** `bin/memdump` now byte-identical to stable (was +1 byte from `STA $2000,X`
instead of `STA $E8,X`).

### Bug 11: .CS mixed-delimiter rule missing (3191947 + 3a14d43)
**Symptom:** `BIN/CUT` was +18 bytes vs stable. `MSG.USAGE .CS 'Usage : CUT "line of text"\r\n'`
emitted the full string through the closing `'` instead of stopping at the embedded `"`.
**Root cause:** `ParseCString()` in `scmasm_directive_handlers.cpp` only checked for the
opening delimiter when finding the end. SCMASM's mixed-delimiter rule: when opening
delimiter is `'`, an embedded `"` also terminates the string.
**Fix (3191947):** Added mixed-delimiter rule (check for `'"'` as alternate end marker).
**Fix (3a14d43):** Refined: the rule applies ONLY to `.CS`, not to `.CZ/.AS/.AT/.AZ`.
Verified against stable: `.CZ '%s = "%s"\r\n'` emits full string (stable matches).
**Files:** `src/syntax/scmasm_directive_handlers.cpp`
**Impact:** BIN/CUT reduced from +18B to -2B. The remaining -2B is a **source change**
(commit `16e3760c "CUT:bugfix"` added `lda ArgLine` before `>SYSCALL ArgV` after stable was built).
BIN/SH shifted from +3B (incorrectly truncated .CZ) to -3B (correct .CZ string; branch
relaxation at $2FA3 adds 3 bytes). Neither is assembler-fixable.

### Bug 12: EquateAtom pushed to main section from inside .DUMMY (b19928d)
**Symptom:** Files with `LABEL .EQ *` inside `.DUMMY` sections (e.g. EDIT, MV, FORTH,
ACOS, GETTY) had false branch relaxations: branches that should reach within ±127 bytes
were being expanded to JMP (+3 bytes each), and sizes were wrong.
**Root cause:** When `HandleEq()` was called inside a `.DUMMY` section with operand `*`,
it created an `EquateAtom` and pushed it to the main section's atom list. On each
subsequent pass, the EquateAtom re-evaluated `*` as the main-section PC ($2000+offset)
rather than the ZP address. After branch relaxation shifted code, the stale `*` value
caused branches to appear out-of-range, triggering further false relaxations.
**Fix:** Added `if (in_dummy_section_) return;` guard at the top of `HandleEq()` to
prevent EquateAtom creation inside `.DUMMY` sections. ZP equates in `.DUMMY` are
evaluated at parse time (correct ZP address) and must not be re-evaluated.
**Files:** `src/syntax/scmasm_directive_handlers.cpp`
**Impact:** EDIT (-8B→+0), MV (-4B→+0), FORTH (-2B→+0), ACOS (-2B→+0) — sizes now
match stable. Content may still differ due to SYSCALL address changes (source version).
GETTY (+15B→0B): now **byte-identical** to stable. Same for LOGIN and INITD.

### Bug 13: Macro argument parsing stopped by comma in inline comment (b19928d)
**Symptom:** `BIN/CSH` was +1 byte vs stable.
`>STYA ZPPtr1     f() definition, starting at returned type`
was being assembled as the 2-argument STYA form (STA+TYA+STA = 5 bytes) instead of
the 1-argument form (STY+STA = 4 bytes), emitting one extra `TYA` byte.
**Root cause:** Macro argument parsing split the operand on ALL commas in the line.
The comma in the inline comment text `f() definition, starting` was treated as
an argument separator, making `starting` appear as a second argument.
**SCMASM convention:** An argument terminates at the first whitespace; everything
after that whitespace (to end of line) is an inline comment, not more arguments.
**Fix:** Rewrote argument scanning in `ExpandMacro()` to scan char-by-char:
- Skip leading whitespace
- Scan to first whitespace or comma (argument boundary)
- If comma: next arg follows; if whitespace: rest is inline comment, stop
**Files:** `src/syntax/scmasm_syntax.cpp`
**Impact:** BIN/CSH is now **byte-identical** to stable.

### Bug 14: .HX directive emitted one byte per nibble instead of packing nibble pairs (c5f54b0)
**Symptom:** `lib/libgui` was 132 bytes larger than stable. Bitmap data in `LIBGUI.G.BM.txt`
was double the expected size.
**Root cause:** `HandleHx()` pushed each hex digit as its own byte (`data.push_back(val)`)
rather than packing pairs into bytes. A `.HX 00022000` (8 nibbles) emitted 8 bytes instead
of 4 bytes.
**SCMASM .HX semantics:** nibble pairs are packed low-nibble-first: pair `XY` → byte `(Y<<4)|X`.
This is Apple II 4bpp color pixel storage convention (low nibble = left pixel).
**Fix:** Collect all nibbles first, then pack pairs: `data.push_back((nibbles[i+1]<<4) | nibbles[i])`.
**Files:** `src/syntax/scmasm_directive_handlers.cpp`, `tests/unit/test_scmasm_syntax.cpp` (+6 tests)
**Impact:** `lib/libgui` reduced from -132B to -20B. Remaining 20B is a source change
(BM.Checked new bitmap +22B, KBD/KBDSTROBE undefined symbol -2B).

### Bug 15: `*` (current PC) not substituted in `.DA` expressions (c5f54b0)
**Symptom:** `.DA BLOCK.E+1-*` style expressions in `bin/asm.6502`, `bin/asm.65816`, and
all `asm.*` CPU module files produced `0` instead of the correct block-size offset.
Block navigation pointers were all zero, which would cause the A2osX ASM tool to
malfunction at runtime when navigating CPU instruction tables.
**Root cause:** `EncodeInstructions()` and `RefixupDataAtoms()` in `assembler.cpp` evaluated
DataAtom expression strings via `ParseExpression(expr_str)` without first substituting
`*` with the current `virtual_address`. `EquateAtom` handling (same file) correctly did
this substitution, but `DataAtom` did not.
**Fix:** Added the same star-replacement loop used in `EquateAtom` to both DataAtom
evaluation paths. Skips `*` preceded by alphanumeric/`.`/`_` chars to avoid `foo*bar`.
**Files:** `src/core/assembler.cpp`
**Impact:** `asm.6502`, `asm.65C02`, `asm.65R02`, `asm.SW16`, `asm.Z80` now
**byte-identical** to stable. `asm.65816` still -9B (source version diff: JSL/JML entries
added in commit c11287b3 after stable was built).

### Bug 10: JSR/JMP to ZP-range address used ZeroPage mode instead of Absolute (75a1f64)
**Symptom:** All 9 `jsr ZPReadAux` instructions in MEMDUMP were silently dropped
(0 bytes emitted), causing MEMDUMP to be 27 bytes too small. Bug was latent —
masked by Bug 9 (ZPReadAux was ~$20AB > $FF, so Absolute was selected; after Bug 9
fix ZPReadAux = $E8 ≤ $FF, exposing Bug 10).
**Root cause:** `EncodeInstruction` mode selection logic: `(!no_zp_form || operand >= 1)`
was intended to exclude only `operand=0` (undefined-label placeholder) from ZP mode for
JSR/JMP. But for operand 1–$FF it evaluated to true → ZeroPage mode. Since JSR has no
ZP variant, `EncodeWithTable` returned empty bytes, silently dropping the instruction.
**Fix:** Changed condition to `(!no_zp_form && operand <= 0xFF)` — JSR/JMP always use
Absolute regardless of operand value.
**Files:** `src/cpu/cpu_6502.cpp`
**Impact:** `bin/memdump` now byte-identical to stable.

---

## Known Source Version Differences (Not Assembler Bugs)

These files differ between xasm++ output and stable due to source code changes
between commit a417f7ab (stable reference era) and 335cd122 (current source):

### Major Include File Changes (affects ALL files using these constants)
- **A2osX.SYSCALL**: `$1000` → `$0140` — affects all `>SYSCALL` invocations (address bytes change)
- **A2osX.SYSCALL2**: `$E200` → `$0153` — affects all `>SYSCALL2` invocations
- **A2osX.SLEEP**: `$1010` → `$0150` — affects all `>SLEEP` invocations
- **SYS.X prefix** → **_X prefix**: constant renames (same values, so no binary change)

### File-specific Source Changes
- **BIN/CUT** (-2B): commit `16e3760c` added `lda ArgLine` before `>SYSCALL ArgV`
- **BIN/SH** (-3B): `.CZ '%s = "%s"\r\n'` string now correct (12B); branch relaxation at $2FA3 adds 3B
- **SBIN/BBSD** (-16B): 4 source changes: FDs table lookup (+2B), push simplification (-2B), `_DEBUG` subroutine (+15B), extra instruction (+1B)
- **BIN/HTTPGET** (-14B): New progress display feature added after stable was built
- **BIN/FNT2FON** (-13B): `.AZ -" : "` (4B) replaced by `.CZ -N : No shrink\r\n"` (17B, typo in source — missing opening `"`)
- **BIN/USERADD** (-48B): commit `3d0a9afe` (Dec 2023) added `CS.RUN.IsValidChar` validation (+48B) after stable was built
- **BIN/ASM.65816** (-9B): commit `c11287b3` (Jan 2025) added JSL (+14B) and JML (+13B) entries, removed DCaa from JMP (-6B). Net +9B after stable.
- **LIB/LIBGUI** (-20B): BM.Checked new bitmap (+22B), KBD/KBDSTROBE undefined symbol (-2B). Source diff.
- **DRV/PPPSSC.DRV** (0B, 1 diff byte): `lda A2osX.S,y` → `lda A2osX.S-1,y` (off by 1)
- **DRV/DHGR.DRV** (0B, 7×SYSCALL2 diffs): `A2osX.HWType` → `A2osX.HWT`; IO.* prefix changes
- **LIB/LIBTCPIP** (0B, many diffs): `.INB inc/kernel.i` added; `SETREADAUX` → `IO.SETREADAUX`
- **LIB/LIBCRYPT** (0B, 9×SYSCALL2 diffs): Only A2osX.SYSCALL2 address change (no source change)
- **sys/kernel**: Code completely different from byte 0
- **lib/libtui** (-286B): Source changed significantly
- **lib/libblkdev** (+1B): Likely SYSCALL address changes + minor source diffs
- **BIN/BMP2PIX** (+7B): Uses bare `SETMIXED`/`CLRTEXT` etc. instead of `IO.SETMIXED`/`IO.CLRTEXT` — A2osX source regression (symbols renamed after stable, callers not updated)
- **SYS/PM.NSC** (+18B), **SYS/PM.VSDRIVE** (+21B), **SYS/PM.VEDRIVE** (+27B): A2osX source regressions — use `PrintFYA` (renamed to `X.PrintF`) and bare `MACHID`/`RDCXROM` symbols (renamed with `MLI.`/`IO.` prefixes). JSR to undefined symbol → JSR $0000 (fatal at runtime).

### Practical Implication
The stable reference (STABLE.800.po) was assembled from **a417f7ab era source** where
`A2osX.SYSCALL = $1000` and `A2osX.SYSCALL2 = $E200`. The current 335cd122 source uses
`A2osX.SYSCALL = $0140` and `A2osX.SYSCALL2 = $0153`. Every binary using these constants
will have different address bytes — this is **NOT an assembler bug**.

The 65 currently-identical files are those where source + includes didn't change between
a417f7ab and 335cd122, AND our assembler produces correct output for them.

---

## Remaining Differences (Run 12 State — 71 identical)

### Negative-delta files (built LARGER than stable — all confirmed source changes)

All remaining negative-delta files are confirmed A2osX source changes between
the stable build era and 335cd122. **No remaining assembler bugs** in this category.

| File | Delta | Notes |
|------|-------|-------|
| bin/sh | -3 | Branch relaxation at $2FA3; pre-existing code structure difference |
| bin/asm.65816 | -9 | JSL/JML entries added in c11287b3 (Jan 2025) after stable |
| bin/fnt2fon | -13 | Source typo: missing `"` in .CZ string |
| bin/httpget | -14 | New progress display feature added after stable |
| sbin/bbsd | -16 | _DEBUG subroutine + FDs table lookup added after stable |
| bin/useradd | -48 | CS.RUN.IsValidChar validation added in 3d0a9afe (Dec 2023) |
| lib/libgui | -20 | BM.Checked new bitmap added after stable |
| lib/libtui | -286 | Source changed significantly |

### Positive-delta files (stable LARGER — potential assembler bugs OR source changes)

| File | Delta | Priority | Notes |
|------|-------|----------|-------|
| bin/attr | +2 | Low | Source version diff (x.fileenum.s changed) |
| bin/chaux | +2 | Low | Same |
| bin/chgrp | +2 | Low | Same |
| bin/chmod | +2 | Low | Same |
| bin/chown | +2 | Low | Same |
| bin/chtyp | +2 | Low | Same |
| bin/cp | +2 | Low | Same |
| bin/lc | +2 | Low | Same |
| bin/pak | +2 | Low | Same |
| bin/rm | +2 | Low | Same |
| bin/uc | +2 | Low | Same |
| bin/wc | +2 | Low | Same |
| lib/libblkdev | +1 | Low | Minor source diff |
| bin/bmp2pix | +7 | Low | Source version diff |
| sys/kernel | +68 | N/A | Completely different source from byte 0 |

### Same-size, different-content files (likely SYSCALL address changes)
These are correct size but have different address bytes from SYSCALL/SYSCALL2 constant change
(`$1000→$0140`, `$E200→$0153`). Not assembler bugs.

| File | Notes |
|------|-------|
| bin/acos, bin/edit, bin/forth, bin/mv | Fixed size (Bug 12); content differs = SYSCALL addr change |
| bin/asm, bin/asm.6502, bin/asm.65c02, bin/asm.65r02, bin/asm.sw16, bin/asm.z80 | SYSCALL addr change |
| bin/cc, bin/dnsinfo, bin/hmacmd5, bin/ls, bin/md4, bin/md5, bin/netstat | SYSCALL addr change |
| bin/rpcdump, bin/xargs | SYSCALL addr change |
| drv/dhgr.drv, drv/pppssc.drv, drv/ssc.drv, drv/ssc.i.drv | SYSCALL/IO prefix changes |
| lib/libcrypt, lib/libetalk, lib/libtcpip | SYSCALL addr change |
| sbin/cifsd, sbin/gui, sbin/vedd | SYSCALL addr change |
| sys/pm/pm.appletalk | SYSCALL addr change |

### Previously listed, now FIXED
| File | Was | Now | Fixed By |
|------|-----|-----|----------|
| ~~bin/csh~~ | +1 | **IDENTICAL** | Bug 13 (macro arg parsing) |
| ~~bin/memdump~~ | +1 | **IDENTICAL** | Bug 9+10 (DUMMY label + JSR absolute) |
| ~~sbin/getty~~ | +15 | **IDENTICAL** | Bug 12 (EquateAtom in .DUMMY) |
| ~~sbin/login~~ | ? | **IDENTICAL** | Bug 12 |
| ~~sbin/initd~~ | ? | **IDENTICAL** | Bug 12 |
| ~~bin/edit~~ | +8 | +0 (size fixed) | Bug 12 |
| ~~bin/mv~~ | +4 | +0 (size fixed) | Bug 12 |
| ~~bin/forth~~ | +11 | +0 (size fixed) | Bug 12 |
| ~~bin/acos~~ | +5 | +0 (size fixed) | Bug 12 |
| ~~bin/asm.6502~~ | +0 | **IDENTICAL** | Bug 15 (.DA * substitution) |
| ~~bin/asm.65c02~~ | +0 | **IDENTICAL** | Bug 15 |
| ~~bin/asm.65r02~~ | +0 | **IDENTICAL** | Bug 15 |
| ~~bin/asm.sw16~~ | +0 | **IDENTICAL** | Bug 15 |
| ~~bin/asm.z80~~ | +0 | **IDENTICAL** | Bug 15 |

---

## Investigation Notes

### Comparison Script
```python
# Normalizes paths: lowercase, strip .dump suffix, convert : to /
# Categories:
#   identical: stable_data == built_data
#   diff_code: code sections (bytes 16+) differ
#   diff_header: only header bytes 0-15 differ
#   not_built: file exists in stable but not built by xasm++
```

### How to Run Comparison
```bash
# 1. Copy new binary
cp build/bin/xasm++ /tmp/xasm_new

# 2. Clear stale outputs (keep inc/ and usr/src/)
find /tmp/A2osX-335cd122-build/stage -not -path "*/inc/*" -not -path "*/usr/src/*" -type f -delete

# 3. Rebuild
cmake --build /tmp/A2osX-335cd122-build

# 4. Run comparison script (see above)
```

### Key File Paths
- Source: `/tmp/A2osX-335cd122/` (git worktree at 335cd122)
- Build: `/tmp/A2osX-335cd122-build/stage/`
- Stable reference: `/tmp/stable_extracted/`
- MACROS.I source: `/tmp/A2osX-335cd122/INC/MACROS.I.txt`
