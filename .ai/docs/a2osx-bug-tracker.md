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

### Bug 11: .CS mixed-delimiter rule missing (3191947)
**Symptom:** `BIN/CUT` was +18 bytes vs stable. `MSG.USAGE .CS 'Usage : CUT "line of text"\r\n'`
emitted the full string through the closing `'` instead of stopping at the embedded `"`.
**Root cause:** `ParseCString()` in `scmasm_directive_handlers.cpp` only checked for the
opening delimiter when finding the end. SCMASM's mixed-delimiter rule: when opening
delimiter is `'`, an embedded `"` also terminates the string.
**Fix:** Added `if (delimiter == '\'')` block checking for `'"'` as alternate end marker,
taking whichever comes first (the `"` or the matching `'`).
**Files:** `src/syntax/scmasm_directive_handlers.cpp`
**Impact:** BIN/CUT reduced from +18B to -2B. The remaining -2B is a **source change**
(commit `16e3760c "CUT:bugfix"` added `lda ArgLine` before `>SYSCALL ArgV` after stable was built).
BIN/CUT cannot be made byte-identical to stable since the source legitimately changed.

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
- **DRV/PPPSSC.DRV** (0B, 1 diff byte): `lda A2osX.S,y` → `lda A2osX.S-1,y` (off by 1)
- **DRV/DHGR.DRV** (0B, 7×SYSCALL2 diffs): `A2osX.HWType` → `A2osX.HWT`; IO.* prefix changes
- **LIB/LIBTCPIP** (0B, many diffs): `.INB inc/kernel.i` added; `SETREADAUX` → `IO.SETREADAUX`
- **LIB/LIBCRYPT** (0B, 9×SYSCALL2 diffs): Only A2osX.SYSCALL2 address change (no source change)
- **sys/kernel**: Code completely different from byte 0
- **lib/libtui** (+286 bytes): Source changed significantly
- **lib/libblkdev** (+1B), **bin/csh** (-1B): Likely SYSCALL address changes + minor source diffs

### Practical Implication
The stable reference (STABLE.800.po) was assembled from **a417f7ab era source** where
`A2osX.SYSCALL = $1000` and `A2osX.SYSCALL2 = $E200`. The current 335cd122 source uses
`A2osX.SYSCALL = $0140` and `A2osX.SYSCALL2 = $0153`. Every binary using these constants
will have different address bytes — this is **NOT an assembler bug**.

The 65 currently-identical files are those where source + includes didn't change between
a417f7ab and 335cd122, AND our assembler produces correct output for them.

---

## Remaining Positive-Delta Files (Potential Assembler Bugs)

Files where xasm++ produces MORE bytes than stable — these are candidates for
further assembler bug investigation:

| File | Delta | Priority | Notes |
|------|-------|----------|-------|
| bin/csh | +1 | Medium | 1-byte branch relaxation |
| ~~bin/memdump~~ | ~~+1~~ | ~~Medium~~ | **FIXED** (Bug 9+10: DUMMY label + JSR absolute) |
| bin/cp | +2 | Medium | Branch relaxation |
| bin/du | +2 | Medium | Branch relaxation |
| drv/dhgr.drv | +3 | High | Likely `:N .EQ *` in a different macro |
| drv/mkboard.drv | +3 | High | Same pattern as dhgr.drv |
| drv/pppssc.drv | +3 | High | Same pattern |
| bin/mv | +4 | Medium | Multiple small relaxations |
| bin/acos | +5 | Medium | |
| bin/edit | +8 | Medium | |
| bin/asm.65816 | +9 | Low | 65816 mode differences |
| bin/sh | +9 | High | Shell is critical binary |
| bin/forth | +11 | Medium | |
| bin/fnt2fon | +13 | Low | Font tool |
| bin/httpget | +14 | Medium | Network utility |
| sbin/getty | +15 | High | Boot-critical login process |
| sbin/bbsd | +16 | Medium | |
| lib/libtcpip | +17 | High | Network library |
| bin/cc | +29 | Low | Compiler |
| bin/useradd | +48 | Low | User admin |
| lib/libgui | +132 | High | GUI library; significant delta |

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
