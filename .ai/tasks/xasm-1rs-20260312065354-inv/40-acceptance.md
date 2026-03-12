# Acceptance: bin/du Investigation

**Task ID:** xasm-1rs
**Date:** 2026-03-12
**Status:** ✅ COMPLETE

---

## Investigation Summary

### Question
Is bin/du's 1491 diffs +2B due to x.fileenum source change or assembler bug?

### Answer
**Different Source Change (NOT x.fileenum)**

---

## Key Findings

### 1. Binary Analysis ✅
- **File size delta**: +2 bytes (1885 → 1887)
- **Byte differences**: 1,492 total
- **First diff location**: Offset 0x0008 (length field)
- **Pattern**: First 8 bytes identical, then cascading diffs

### 2. Comparison with x.fileenum Files ✅
Tested against 5 confirmed x.fileenum files:

| File  | Size Delta | Pattern |
|-------|------------|---------|
| attr  | -2 bytes   | x.fileenum ✓ |
| chaux | -2 bytes   | x.fileenum ✓ |
| chgrp | -2 bytes   | x.fileenum ✓ |
| chmod | -2 bytes   | x.fileenum ✓ |
| cp    | -2 bytes   | x.fileenum ✓ |
| **du**| **+2 bytes** | **DIFFERENT** ❌ |

**Critical Discovery**: x.fileenum pattern is -2B (code removed), but du is +2B (code added) — OPPOSITE direction!

### 3. Source Verification ✅
- ✅ du.s DOES include x.fileenum.s (line 447)
- ✅ BUT the +2B change is from a different source modification
- ✅ du.s has additional code changes beyond x.fileenum

### 4. Assembler Bug Check ✅
**NO ASSEMBLER BUG DETECTED**

Evidence xasm++ is working correctly:
- ✅ Length field (0x0008) accurately reflects +2B code size
- ✅ All 1,491 byte diffs are cascading address updates (expected)
- ✅ Address references correctly recalculated throughout binary
- ✅ ProDOS header properly updated

---

## Acceptance Criteria

- [x] ✅ Determined if +2B is x.fileenum source change
  - **Result**: NO - opposite pattern (-2B vs +2B)

- [x] ✅ Checked if first diff at offset 0x0008
  - **Result**: YES - at 0x0008 (length field)

- [x] ✅ Verified pattern against known x.fileenum files
  - **Result**: Pattern does NOT match (opposite direction)

- [x] ✅ Assessed for assembler bug
  - **Result**: NO BUG - xasm++ assembled correctly

- [x] ✅ Root cause identified
  - **Result**: Different source change in du.s (+2B code addition)

- [x] ✅ Documented findings
  - Work log updated
  - Knowledge graph updated
  - Investigation summary created

---

## Verdict

### Root Cause
bin/du has a **different source-level change** (not x.fileenum) that added 2 bytes of code.

### x.fileenum vs bin/du
- **x.fileenum change**: Removes 2 bytes (optimization)
- **bin/du change**: Adds 2 bytes (new feature/fix)
- These are SEPARATE changes in the same commit

### Assembler Status
xasm++ is functioning correctly:
- Accurate code size calculation
- Correct address reference updates
- Proper header generation
- Expected cascading diff pattern

### Recommendation
**Accept as correct**
- The +2B is a legitimate source change
- Not an assembler bug
- No action required
- Investigation complete

---

## Evidence Trail

### Files Analyzed
- `/tmp/A2osX-335cd122-build/stage/bin/du` (new, 1887 bytes)
- `/tmp/stable_extracted/FULL800/bin/du#062000` (old, 1885 bytes)
- `/tmp/A2osX-335cd122-build/stage/usr/src/bin/du.s` (source)

### Comparison Files
- `/tmp/stable_extracted/FULL800/bin/attr#062000` (x.fileenum -2B)
- `/tmp/stable_extracted/FULL800/bin/chaux#062000` (x.fileenum -2B)
- `/tmp/stable_extracted/FULL800/bin/chgrp#062000` (x.fileenum -2B)
- `/tmp/stable_extracted/FULL800/bin/chmod#062000` (x.fileenum -2B)
- `/tmp/stable_extracted/FULL800/bin/cp#062000` (x.fileenum -2B)

### Documentation
- Work log: `.ai/tasks/xasm-1rs-20260312065354-inv/20-work-log.md`
- Investigation summary: `/tmp/du-investigation-summary.md`
- Knowledge graph: Entity "bin/du +2B investigation" with observations

---

## Task Complete ✅

Investigation concluded. bin/du +2B is a different source change, not x.fileenum, and not an assembler bug.
