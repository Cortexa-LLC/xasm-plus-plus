# Work Log: A2osX Boot Hang Fix

## Task: xasm-u32
## Status: COMPLETE

## Session: 2026-03-10

### Investigation Summary

Confirmed root causes and implemented fixes.

#### Root Cause 1: Wrong ProDOS Template in CMakeLists.txt

**File:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/CMakeLists.txt`
**Line:** 34

The `find_file(PRODOS800_TEMPLATE NAMES ...)` had `blank800k.po` BEFORE `blank800k_prodosfx.po`:
```cmake
# OLD (wrong order):
find_file(PRODOS800_TEMPLATE NAMES blank800k.po blank800k_prodosfx.po
```

CMake's `find_file` finds the FIRST NAME in order. Since `samples/blank800k.po` (regular ProDOS 2.4.3) exists and is listed first, it was always selected over the ProDOS FX template.

**Fix:** Swapped the NAMES order:
```cmake
# NEW (correct order):
find_file(PRODOS800_TEMPLATE NAMES blank800k_prodosfx.po blank800k.po
```

**Verification:** After deleting CMakeCache.txt and reconfiguring:
- CMake now reports: `ProDOS 800K tmpl: .Floppies/blank800k_prodosfx.po` ✓
- Mini image PRODOS binary matches blank800k_prodosfx.po exactly ✓

#### Root Cause 2: INIT2.KConfig returns carry=0 for regular ProDOS

**File:** `/Users/bryanw/Projects/Vintage/Apple/A2osX/SYS/KERNEL.S.INIT2.txt`
**Lines:** 412-421

On regular ProDOS 2.4.3, `MLI.MLIENTRY` with `GFINFOEX` ($D5) returns error $01 (BADCALL) because ProDOS 2.4.3 doesn't support this FX-only call.

The BADCALL path (`eor #$01; beq .2`) left `ZPPtr1=0`, so at function end:
```asm
lda ZPPtr1  ; = 0 (regular ProDOS, no GFINFOEX support)
rol         ; carry = 0 (bit7 was 0)
rts         ; returns with carry=0
```

Caller: `bcc *` = infinite loop if carry=0. **BOOT HANGS.**

**Fix:** In the BADCALL path, add `sec; ror ZPPtr1` to set ZPPtr1.bit7=1:
```asm
; OLD:
eor #MLI.E.BADCALL
beq .2          regular ProDOS (ZPPtr1 stays 0 → carry=0 → HANG)

ror ZPPtr1      Bad file, ProDOS.FX detected
bra .9

; NEW:
eor #MLI.E.BADCALL
bne .4          not BADCALL: ProDOS.FX with bad file

sec             regular ProDOS: set carry=1 for return
ror ZPPtr1      ZPPtr1.bit7=1 → carry=1 on rts (continue boot)
bra .2

.4  ror ZPPtr1  Bad file, ProDOS.FX detected
    bra .9
```

**Note:** Label `.4` was chosen (not `.3`) because `.3` was already used as a copy-loop label later in `INIT2.KConfig`. Using `.3` would have caused the `bne` to branch to the copy loop instead.

**Verification:**
- Kernel binary has `sec; ror ZP[0]; bra` pattern at offset 0x2C8 ✓
- `bne .4` at 0x2C6 (offset=$05) correctly targets `ror ZPPtr1` at 0x2CD ✓
- `bra .2` at 0x2CB (offset=$07) correctly targets `jsr $BF00` (MLI.OPEN) at 0x2D4 ✓

### Boot Path Analysis

**ProDOS FX (no kconfig file):**
1. `MLI.MLIENTRY($D5)`: SUCCESS, carry=0
2. `bcc .1` → `sec; ror ZPPtr1` → ZPPtr1=0x80
3. Fall through to .2: OPEN kconfig → fails (no file)
4. `bcs .9` → error message
5. `.8`: `lda ZPPtr1=0x80; rol` → carry=1
6. Returns carry=1 → `bcc *` not taken → **BOOTS ✓**

**Regular ProDOS 2.4.3 (no kconfig file, kernel fix applied):**
1. `MLI.MLIENTRY($D5)`: ERROR $01 (BADCALL), carry=1
2. `bcc .1` → NOT taken
3. `eor #$01` → A=0, Z=1
4. `bne .4` → NOT taken (Z=1)
5. `sec; ror ZPPtr1` → ZPPtr1=0x80 ← NEW FIX
6. `bra .2` → OPEN kconfig → fails (no file)
7. `bcs .9` → error message
8. `.8`: `lda ZPPtr1=0x80; rol` → carry=1
9. Returns carry=1 → `bcc *` not taken → **BOOTS ✓**

### Files Changed

1. `/Users/bryanw/Projects/Vintage/Apple/A2osX/CMakeLists.txt`
   - Line 34: Swapped `blank800k.po blank800k_prodosfx.po` → `blank800k_prodosfx.po blank800k.po`

2. `/Users/bryanw/Projects/Vintage/Apple/A2osX/SYS/KERNEL.S.INIT2.txt`
   - Lines ~412-421: Fixed INIT2.KConfig BADCALL path
   - Changed `beq .2` → `bne .4`
   - Added `sec; ror ZPPtr1; bra .2` for regular ProDOS path
   - Changed old `.3` label → `.4` (avoid conflict with existing `.3` copy loop)

### Tests

- xasm++ tests: All pass except 2 pre-existing failures (CLITest.OrgDefaultZero, MaxPassesDefault)
- No regressions introduced

### Suggested Commit Message

```
fix(a2osx): fix boot hang after 'Reading Kernel Config File' message

Two root causes identified and fixed:

1. CMakeLists.txt: NAMES order caused regular ProDOS 2.4.3 to be
   selected over ProDOS FX. blank800k_prodosfx.po is now tried first.
   A2osX requires ProDOS FX for GFINFOEX ($D5) support.

2. KERNEL.S.INIT2.txt: INIT2.KConfig returned carry=0 when called on
   regular ProDOS 2.4.3 (which returns BADCALL for GFINFOEX). The
   caller's `bcc *` caused an infinite loop. Fixed by adding
   `sec; ror ZPPtr1` in the BADCALL path so carry=1 is always returned
   when A2osX can continue booting.
```

