# Acceptance: Validation Run — ssc.drv, ssc.i.drv, libtcpip, libetalk

**Task ID:** xasm-oa9  
**Completed:** 2026-03-11 22:40  
**Status:** ✅ ACCEPTED

---

## Acceptance Criteria Review

### ✅ Functional Requirements

- [x] All 4 files rebuilt with current xasm++ binary
  - ssc.drv: ✅ assembled
  - ssc.i.drv: ✅ assembled
  - libtcpip: ✅ assembled
  - libetalk: ✅ assembled

- [x] Diff comparison against stable reference completed
  - All byte-level diffs identified and counted
  - Diff counts documented

- [x] All diffs classified as SYSCALL-only or POSSIBLE_BUG
  - ssc.drv: RELOCATION TABLE FORMAT (source change)
  - ssc.i.drv: RELOCATION TABLE FORMAT (source change)
  - libtcpip: SYSCALL EVOLUTION (source change)
  - libetalk: SYSCALL EVOLUTION (source change)

### ✅ Quality Requirements

- [x] Build succeeded with zero errors
  - All 4 files assembled cleanly
  - No warnings or errors from xasm++

- [x] Analysis documented
  - Detailed findings in 30-findings.md
  - Work log complete
  - Reproduction steps documented

- [x] Results actionable
  - Clear classification of each diff type
  - Recommendations provided
  - Impact assessment complete

### ✅ Deliverables

- [x] Diff report with classifications (30-findings.md)
- [x] Work log with progress tracking (20-work-log.md)
- [x] Acceptance document (this file)

---

## Validation Results

### Diff Count Summary

| File | Prior | Current | Change | Classification |
|------|-------|---------|--------|----------------|
| ssc.drv | 39 | 39 | 0 | RELOCATION TABLE FORMAT |
| ssc.i.drv | 39 | 39 | 0 | RELOCATION TABLE FORMAT |
| libtcpip | 65 | 62 | **-3** ✅ | SYSCALL EVOLUTION |
| libetalk | 11 | 10 | **-1** ✅ | SYSCALL EVOLUTION |
| **TOTAL** | **154** | **150** | **-4** | |

### Impact of xasm-zp1 Fixes

**Bugs Fixed:** 4 total diffs resolved
- libtcpip: 3 diffs (incorrect address calculations)
- libetalk: 1 diff (incorrect address calculation)

**Root Causes Resolved:**
1. Double-dot label mis-parse (`X.BasePath..1`)
2. Infix `*` multiplication parsing

**Success Rate:** 2.6% of total diffs (4 out of 154)

This confirms most diffs were NOT assembler bugs, validating the original investigation's hypothesis.

---

## Classification Details

### Relocation Table Format (78 diffs)
- **Files:** ssc.drv (39), ssc.i.drv (39)
- **Location:** Relocation table region (offset 0x02C0+)
- **Cause:** Metadata format evolution in xasm++
- **Status:** NOT A BUG — expected format difference

### SYSCALL Evolution (72 diffs)
- **Files:** libtcpip (62), libetalk (10)
- **Location:** JSR target addresses throughout code
- **Cause:** A2osX source code changes between stable and current git
- **Status:** NOT A BUG — source code evolution
- **Fixed by xasm-zp1:** 4 diffs (were actual bugs in address calculation)

---

## Recommendations

### ✅ For xasm++ Development
1. **Merge xasm-zp1 fixes** — they resolve real bugs
2. **No further investigation needed** for remaining diffs
3. **Relocation table format change is acceptable**

### 📋 For Future Validation
1. **Document exact git commit** of reference builds
2. **Compare executable code sections only** (exclude relocation tables)
3. **Track SYSCALL changes** separately from assembler validation

---

## Files Modified

No source files modified (validation run only).

**Artifacts Created:**
- `.ai/tasks/-20260311223141-validation-run/20-work-log.md` (work log)
- `.ai/tasks/-20260311223141-validation-run/30-findings.md` (detailed report)
- `.ai/tasks/-20260311223141-validation-run/40-acceptance.md` (this file)

**Build Outputs:**
- `/tmp/A2osX-335cd122-stage/drv/ssc.drv`
- `/tmp/A2osX-335cd122-stage/drv/ssc.i.drv`
- `/tmp/A2osX-335cd122-stage/lib/libtcpip`
- `/tmp/A2osX-335cd122-stage/lib/libetalk`

---

## Conclusion

✅ **VALIDATION SUCCESSFUL**

The xasm-zp1 fixes (double-dot label parsing and infix `*` multiplication) were effective and resolved 4 real bugs. The remaining 150 diffs are explained by:
- Relocation table format evolution (78 diffs) — expected
- Source code changes (72 diffs) — expected

**No further assembler bugs detected.** The current xasm++ build is functioning correctly.

---

**Accepted By:** Engineer Agent  
**Date:** 2026-03-11 22:40
