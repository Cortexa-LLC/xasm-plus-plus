# Contract: Fix A2osX Boot Hang After Kernel Config Message

**Task ID:** xasm-u32
**Priority:** P0
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++
**Related Projects:** /Users/bryanw/Projects/Vintage/Apple/A2osX

## Symptom

A2osX mini image boots to:
```
A2osX[Stage2]:Init
Relocating Kernel...
Reading Kernel Config File...Error, Using Default.
```
Then **hangs permanently**. Never reaches login prompt.

## Investigation Findings (DO NOT REPEAT — BUILD ON THESE)

### 1. Root Cause in KERNEL.S.INIT2.txt

File: `/Users/bryanw/Projects/Vintage/Apple/A2osX/SYS/KERNEL.S.INIT2.txt`

Lines 73-75:
```asm
jsr INIT2.KConfig
bcc *           FX only   ← INFINITE LOOP if carry=0
```

`INIT2.KConfig` (line 402) returns carry via `lda ZPPtr1; rol` at the end.

**On regular ProDOS (non-FX) without kconfig file:**
- `stz ZPPtr1` sets ZPPtr1 = 0
- `MLI.MLIENTRY` ($BF4B) called with `GFINFOEX` ($D5)
- If ProDOS returns error code $01 (BADCALL): `eor #MLI.E.BADCALL` → beq .2 → ZPPtr1 stays 0
- OPEN fails (no kconfig file) → goes to `.9`
- `.8`: `lda ZPPtr1` = 0, `rol` → carry = 0 (bit 7 of 0 = 0)
- Returns carry=0 → `bcc *` = INFINITE LOOP

**On regular ProDOS (non-FX) if GFINFOEX returns error ≠ BADCALL:**
- `ror ZPPtr1` sets ZPPtr1 bit 7 = old carry → ZPPtr1 = 0x80
- `.8`: `lda ZPPtr1` = 0x80, `rol` → carry = 1 (bit 7 = 1)
- Returns carry=1 → `bcc *` skipped → boot continues

**Key unknown:** What error code does ProDOS 2.4.3 return at $BF4B for call $D5?
- If $01 (BADCALL) → boot hangs
- If anything else → boot continues

### 2. STABLE Image Comparison

STABLE.800.po was built from commit c11287b3 (older source WITHOUT `>LDYA $FFFE`).
Current source is from commit 7c15ec42+ (newer, "still LIB problem" note in commit).
STABLE kernel = 27716 bytes. Our kernel = 29639 bytes (different source version, NOT a bug).

### 3. Current Build Status

- xasm++ binary: `/usr/local/bin/xasm++` version 0.9.0-b7b739c
- Mini image rebuilt: Mar 9 20:18 ✓
- Kernel rebuilt: Mar 9 20:18 ✓ (29639 bytes)
- No JMP $0000 in kernel ✓

### 4. Files

- Kernel source entry: `/Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/usr/src/sys/kernel.s`
- INIT2 source: `/Users/bryanw/Projects/Vintage/Apple/A2osX/SYS/KERNEL.S.INIT2.txt`
- Mini image: `/Users/bryanw/Projects/Vintage/Apple/A2osX/build/a2osx-mini.po`
- ProDOS 2.4.3 used: verified in CMakeLists.txt
- MLI constants: `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/MLI.I.txt`
  - MLI.GFINFOEX = $D5
  - MLI.MLIENTRY = $BF4B
  - MLI.E.BADCALL = $01

## Investigation Tasks for Engineer

1. **Determine what ProDOS 2.4.3 returns for GFINFOEX at $BF4B**
   - Check ProDOS 2.4.3 source or disassembly — does it return $01 BADCALL or something else?
   - ProDOS source: https://github.com/a2stuff/prodos-drivers or check the extracted PRODOS binary
   - Extracted ProDOS: `/Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/PRODOS`

2. **Trace the INIT2.KConfig return path on ProDOS 2.4.3**
   - If GFINFOEX returns BADCALL ($01): ZPPtr1 stays 0 → carry=0 → hang
   - If GFINFOEX returns something else: ZPPtr1=0x80 → carry=1 → continues

3. **Fix options (choose one):**
   - **Option A**: If ProDOS 2.4.3 does return BADCALL for unknown calls, then `INIT2.KConfig` has a bug for standard ProDOS without kconfig file. Fix: ensure carry is set before returning in the error path. This would be an A2osX source fix.
   - **Option B**: If the source code is correct and SCMASM produces different machine code (e.g., different carry handling), identify the xasm++ assembly difference that causes it.
   - **Option C**: Check if there's a version of INIT2.KConfig that explicitly sets carry before return.

4. **Check X.PrintF carry return**
   - If X.PrintF sets carry=1 on return, then `lda ZPPtr1; rol` with carry=1 and A=0 → carry=0 (bit7 of A=0). Still wrong.
   - Unless ZPPtr1 gets set somehow in the BADCALL path.

5. **Verify by building from known-good source revision**
   - Can we find a commit of A2osX source that was tested with the new INIT2.KConfig and boots?
   - git log in A2osX repo after commit 7c15ec42

## Build Commands

```bash
# Build xasm++
cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build

# Force kernel rebuild
rm /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage/sys/kernel
cmake --build /Users/bryanw/Projects/Vintage/Apple/A2osX/build

# Run xasm++ tests
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure
```

## Acceptance Criteria

- A2osX mini image boots past "Reading Kernel Config File" to login prompt
- xasm++ builds clean and all tests pass
- If fix is in A2osX source: document why
- If fix is in xasm++: fix, add test, all tests pass
