# Work Log

**Task ID:** xasm-z5o
**Started:** 2026-03-10
**Status:** Completed

---

## Work Sessions

### Session 1: 2026-03-10

#### Objectives for This Session
```
✓ Review prior investigation findings from KG
✓ Apply fixes to A2osX project (CMakeLists.txt + KERNEL.S.INIT2.txt)
✓ Rebuild kernel and verify fix
✓ Verify disk image created with fixed kernel
```

#### Work Completed

**Prior Investigation Context (from KG)**

Prior sessions (xasm++-fz5t, xasm-u32) had already identified two root causes:

**ROOT CAUSE 1: CMakeLists.txt NAMES order**
- `find_file(PRODOS800_TEMPLATE NAMES blank800k.po ...)` finds the standard ProDOS template first
- Should prefer `blank800k_prodosfx.po` (ProDOS FX template) when both are present
- If standard ProDOS is used as template, Stage2 sees regular ProDOS, then hangs

**ROOT CAUSE 2: INIT2.KConfig returns carry=0 for regular ProDOS**

In `SYS/KERNEL.S.INIT2.txt` at lines ~363-364:
```asm
eor #MLI.E.BADCALL
beq .2          ; regular ProDOS — jumps to try OPEN kconfig (fails)
ror ZPPtr1      ; FX bad file — carry=1 from MLI error → ZPPtr1.bit7=1
bra .9
```

When BADCALL (regular ProDOS, GFINFOEX not supported):
- `beq .2` taken → tries MLI.OPEN of kconfig → fails → `.9` → error message
- `lda ZPPtr1` ($00), `rol` → carry=0 → returns carry=0
- Caller: `bcc *` → **INFINITE LOOP / BOOT HANG**

#### Fix Applied

**Fix 1: `CMakeLists.txt` line 34**
```cmake
# Before:
find_file(PRODOS800_TEMPLATE NAMES blank800k.po

# After:
find_file(PRODOS800_TEMPLATE NAMES blank800k_prodosfx.po blank800k.po
```

**Fix 2: `SYS/KERNEL.S.INIT2.txt` lines 363-370**
```asm
; Before:
eor #MLI.E.BADCALL
beq .2          regular ProDOS

ror ZPPtr1      Bad file, but ProDOS.FX detected
bra .9

; After:
eor #MLI.E.BADCALL
bne .4          FX bad file, ProDOS.FX detected

sec             BADCALL = regular ProDOS, force carry=1
ror ZPPtr1      ZPPtr1.bit7=1 => carry=1 on return
bra .2

.4  ror ZPPtr1  Bad file, but ProDOS.FX detected
    bra .9
```

Logic correction:
- BADCALL (regular ProDOS): `bne .4` NOT taken → `sec; ror ZPPtr1` (ZPPtr1=$80) → `bra .2` → OPEN kconfig (fails) → `.9` → `lda ZPPtr1`($80), `rol` → carry=1 → `bcc *` NOT taken → boot continues ✓
- FX bad file (non-BADCALL MLI error): `bne .4` taken → `.4: ror ZPPtr1` (carry=1 from error) → ZPPtr1=$80 → `bra .9` → `lda ZPPtr1`($80), `rol` → carry=1 ✓

**Files Modified:**
- `/Users/bryanw/Projects/Vintage/Apple/A2osX/CMakeLists.txt` - NAMES order fix
- `/Users/bryanw/Projects/Vintage/Apple/A2osX/SYS/KERNEL.S.INIT2.txt` - INIT2.KConfig carry fix

#### Build Verification
```
✓ Kernel assembled successfully: build/stage/sys/kernel (28102 bytes)
✓ sec;ror ZPPtr1;bra pattern found at offset 0x0249 in kernel binary
✓ bcc * at 0x006d is now safe (caller, carry=1 from KConfig → not taken)
✓ Mini disk image rebuilt: build/a2osx-mini.po (819200 bytes, 2026-03-10 11:48)
```

#### Note on build failure
The `bin/asm.65816` target fails with "MVN requires two operands" — this is a pre-existing xasm++ issue unrelated to the boot fix. The mini target (which includes kernel + minimal system) built successfully.

### Next Steps
- Human reviewer should test `build/a2osx-mini.po` in Apple II emulator
- Verify boot reaches login prompt on standard ProDOS (non-FX) disk
