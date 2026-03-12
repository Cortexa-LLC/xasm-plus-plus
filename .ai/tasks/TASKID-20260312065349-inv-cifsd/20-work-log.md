# Work Log: sbin/cifsd Investigation

**Task:** TASKID-20260312065349-inv-cifsd  
**Engineer:** AI Agent  
**Date:** 2026-03-12

---

## Session 1: Investigation Completed (Previous Work)

### Discovery: Prior Investigation Found

Upon starting this task, discovered that a comprehensive investigation has already been completed:

**Knowledge Graph Entity:** "cifsd SYSCALL investigation"  
**Investigation Report:** `/tmp/INVESTIGATION-cifsd-26-diffs.md`  
**Date Completed:** 2026-03-12 (earlier today)

### Investigation Summary

**VERDICT: ✅ SOURCE CHANGE, NOT ASSEMBLER BUG**

Root cause identified:
- All 26 byte differences are due to A2osX kernel SYSCALL address relocation
- OLD (ce94435): SYSCALL dispatcher at $E200
- NEW (335cd122): SYSCALL dispatcher moved to $E253
- Change affects all 26 SYSCALL invocations in cifsd

### Evidence Collected

1. **Symbol table comparison:**
   - OLD: `K.SYSCALL = E200`
   - NEW: `K.SYSCALL = E253`
   - Delta: +$0053 (83 bytes)

2. **Binary analysis verified:**
   - All 26 diffs show pattern: 0x00→0x53
   - Matches SYSCALL relocation delta exactly
   - xasm++ correctly assembled per symbol definitions

3. **Source verification:**
   - cifsd.s contains 26 SYSCALL invocations
   - Each invocation uses symbolic `>SYSCALL` addressing
   - xasm++ correctly resolved each to current K.SYSCALL value

### Key Files Analyzed

- `/tmp/A2osX-335cd122-build/sbin/cifsd` (4920 bytes, 26 diffs)
- `/tmp/stable_extracted/FULL800/sbin/cifsd` (4920 bytes, reference)
- Symbol tables from both builds
- cifsd.s source code

### Technical Details

**Diff locations:** All 26 differences are in the second byte of JSR instructions to K.SYSCALL:
- Pattern: `20 00 E2` → `20 53 E2`
- Instruction: `JSR $E200` → `JSR $E253`
- Reason: Kernel restructuring between commits

**Architectural Change Documented:**
The A2osX kernel underwent SYSCALL relocation between ce94435 and 335cd122, affecting:
- All system binaries making SYSCALL invocations
- No code changes required (symbolic addressing works correctly)
- xasm++ behavior is correct and expected

---

## Conclusion

✅ **Task Complete**

**Root Cause:** A2osX kernel SYSCALL address moved from $E200 to $E253 between source versions

**xasm++ Status:** Working correctly - no assembler bug

**Recommendation:** Mark cifsd differences as "expected" in validation reports. These are legitimate architectural changes in the A2osX source, not defects in xasm++.

**Documentation:**
- Full investigation report: `/tmp/INVESTIGATION-cifsd-26-diffs.md`
- Knowledge graph updated with findings
- Task packet completed

---

**Session End:** 2026-03-12  
**Time Spent:** <5 minutes (leveraging prior investigation)  
**Status:** ✅ COMPLETE
