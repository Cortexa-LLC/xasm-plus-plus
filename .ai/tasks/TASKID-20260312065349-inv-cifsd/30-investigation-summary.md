# Investigation Summary: sbin/cifsd — 26 Diffs Analysis

**Date:** 2026-03-12  
**File:** sbin/cifsd  
**Size:** 4920 bytes (both builds)  
**Differences:** 26 bytes  
**Status:** ✅ RESOLVED — Source change confirmed

---

## Executive Summary

**Conclusion: NOT AN ASSEMBLER BUG**

All 26 byte differences in sbin/cifsd are due to legitimate A2osX kernel SYSCALL address relocation between source commits ce94435 (reference) and 335cd122 (current). The xasm++ assembler is working correctly.

---

## Root Cause

### Kernel Architecture Change

Between A2osX commits ce94435 and 335cd122, the kernel SYSCALL dispatcher address was relocated:

- **OLD (ce94435):** K.SYSCALL = $E200
- **NEW (335cd122):** K.SYSCALL = $E253
- **Delta:** +$0053 (83 bytes)

### Impact on cifsd Binary

The cifsd daemon makes 26 SYSCALL invocations, each using symbolic addressing:

```assembly
>SYSCALL GetMemPtr  ; Expands to: JSR K.SYSCALL
```

When assembled:
- **OLD build:** JSR $E200 → machine code: `20 00 E2`
- **NEW build:** JSR $E253 → machine code: `20 53 E2`

**Result:** 26 byte differences, all in the second byte of JSR instructions (0x00→0x53)

---

## Evidence

### 1. Symbol Table Comparison

Verified both builds' symbol tables show different K.SYSCALL addresses matching the observed delta.

### 2. Binary Diff Pattern

All 26 differences follow the exact same pattern:
- Byte value change: 0x00 → 0x53
- Context: Within JSR instruction sequences
- Delta matches SYSCALL relocation: +$0053

### 3. Source Code Analysis

cifsd.s contains exactly 26 SYSCALL macro invocations, matching the diff count perfectly.

### 4. xasm++ Assembly Verification

xasm++ correctly:
- Resolved K.SYSCALL symbol from current kernel headers
- Generated proper JSR instructions with correct target address
- Maintained all other code generation unchanged

---

## Verification Process

1. ✅ Extracted symbol tables from both builds
2. ✅ Identified K.SYSCALL address difference
3. ✅ Analyzed binary diff pattern (all 0x00→0x53)
4. ✅ Confirmed diff count matches SYSCALL invocation count in source
5. ✅ Verified xasm++ symbol resolution is correct

---

## Conclusion

**xasm++ Status:** ✅ WORKING CORRECTLY

The assembler is faithfully translating the source code according to the symbol definitions in the current A2osX kernel. The differences are expected and correct.

**Recommendation:** Mark sbin/cifsd as "EXPECTED DIFF - SYSCALL RELOCATION" in validation tracking.

---

## Related Investigations

This follows the same pattern as other A2osX binaries affected by kernel restructuring:
- All binaries with SYSCALL invocations show similar patterns
- Delta is consistent across all affected files
- No assembler defects identified

---

## Documentation

- **Full Investigation Report:** `/tmp/INVESTIGATION-cifsd-26-diffs.md`
- **Knowledge Graph:** Entity "cifsd SYSCALL investigation" with complete findings
- **Validation Report:** `.ai/tasks/xasm-89o-20260312063117-run15-full-comparison/30-validation-report.md`

---

**Investigator:** Engineer Agent  
**Date:** 2026-03-12  
**Status:** ✅ COMPLETE
