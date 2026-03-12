# Task Contract

**Beads ID:** xasm++-fz5t
**Task ID:** xasm++-fz5t-20260305-investigate-a2osx-boot
**Created:** 2026-03-05
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Requires Review:** false
**Model:** claude-sonnet-4-6
**Timeout:** 45min

---

## ⚠️ PRIOR INVESTIGATION SUMMARY — READ THIS FIRST, DO NOT RE-INVESTIGATE

5 prior runs have established the following. **Do not spend turns re-discovering these facts.**

### RULED OUT
- Previous crash at $1FE2 — already fixed, not related
- ROL/ASL fix missing from installed xasm++ — fix IS present (commit bc13f48, Feb 27)
- Doubled `.s.s` extension — this was a red herring from stale cmake state, NOT the bug

### CONFIRMED
- `strings build/stage/sys/kernel | grep A2osX.kconfig` → "A2osX.kconfig" is in the kernel binary
- `cadius CATALOG build/a2osx-full.po | grep /root/` → /root/ directory exists but is EMPTY (no A2osX.kconfig file)
- The "Reading Kernel Config File...Error, Using Default" message is a non-fatal warning
- ROL A encodes as `0x26 0x00` (ROL $00, zeropage) instead of `0x2A` (ROL A, accumulator) in cmake builds — but isolated xasm++ test assembles ROL A correctly
- The cmake-generated xasm++ invocation (confirmed from A2osXHelpers.cmake) is:
  ```
  cd /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage
  xasm++ --cpu 65c02 --syntax scmasm -I /Users/bryanw/Projects/Vintage/Apple/A2osX/build/stage usr/src/sys/kernel.s
  ```

### HOW THE BUILD WORKS — READ THIS BEFORE TOUCHING ANYTHING

**`build/stage` is the cmake-managed working directory.** All source files are copied/symlinked there by cmake. Do NOT try to set this up manually.

**To build:**
```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX
cmake --build build 2>&1 | tail -30
```

**To build just the kernel and see the exact xasm++ invocation:**
```bash
cmake --build build --target sys/kernel -v 2>&1 | grep -A2 xasm
```

**Do not invoke xasm++ directly** without first running cmake to populate `build/stage`.

---

## Task: Verify ROL encoding and fix the boot failure

**Working directory:** /Users/bryanw/Projects/Vintage/Apple/A2osX
**xasm++ source:** /Users/bryanw/Projects/Vintage/tools/xasm++

### Step 1 — Run a full cmake build and check for ROL encoding errors

```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX
cmake --build build 2>&1 | tail -30
```

Then verify ROL encoding in the built kernel:
```bash
python3 -c "
data = open('build/stage/sys/kernel','rb').read()
for i, b in enumerate(data):
    if b == 0x26:
        print(f'ROL zeropage at {i:#06x}: {data[i:i+2].hex()}')
    elif b == 0x2A:
        print(f'ROL A at {i:#06x}')
" | head -20
```

If ROL A encodes as `0x2A` → encoding is fixed, proceed to Step 3.
If ROL A still encodes as `0x26 0x00` → investigate xasm++ scmasm accumulator handling (Step 2).

### Step 2 — If ROL still wrong, isolate the scmasm accumulator parsing

The bug is in xasm++ scmasm syntax mode — `ROL A` in scmasm should emit `0x2A` (accumulator mode).
Check `src/syntax/scmasm_directive_handlers.cpp` around line 830 for accumulator operand handling.

### Step 3 — Full build and disk image check

```bash
cmake --build build
cadius CATALOG build/a2osx-full.po
```

Then call TaskComplete with: ROL encoding status, any remaining errors, and what the next step should be.

---

## Success Criteria

```
✓ Identify the exact line in A2osXHelpers.cmake causing doubled extension
✓ Confirm or rule out doubled extension as root cause of wrong ROL encoding
✓ Apply fix and rebuild kernel
✓ Call TaskComplete with findings
```

## Out of Scope

```
✗ Full emulator boot validation (leave for next task)
✗ Investigating anything already in the RULED OUT section above
✗ Re-investigating kconfig file absence (known, non-fatal warning)
```
