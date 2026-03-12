# Acceptance: bin/mv 1042-diff Investigation

## Task ID
xasm-81v

## Completion Status
✅ **COMPLETE** - 2026-03-12

## Acceptance Criteria Met

### ✅ Root Cause Identified
**CONFIRMED**: Two distinct causes identified from prior investigation (task xasm-siu):

1. **Assembler Bug (2 bytes)**:
   - Double-dot label reference bug in scmasm_syntax.cpp
   - Pattern: `JSR X.BasePath..1` resolved incorrectly to `JSR $0000`
   - Location: bin/mv MV.S source file

2. **Source Code Changes (1042 bytes)**:
   - Legitimate A2osX source modifications
   - Not assembler bugs
   - Normal codebase evolution

### ✅ Fix Verified
**CONFIRMED**: Fix applied in commit ce94435

**Fix Location**: `src/syntax/scmasm_syntax.cpp` lines 1135-1145

**Fix Details**:
- Added `at_word_start` check to local label detection
- Prevents second dot in `..1` from being treated as local label prefix
- Ensures `X.BasePath..1` resolves correctly

**Verification Steps**:
1. Source code inspection: ✅ Fix present
2. Build verification: ✅ xasm++ builds successfully
3. Test suite: ✅ All 1864 tests pass
4. Regression test: ✅ `ScmasmSyntaxTest.GlobalLabelWithDoubleDotForwardReference` added
5. A2osX bin/mv: ✅ Builds successfully (2805 bytes)

### ✅ Documentation Complete
**CONFIRMED**: Full documentation provided

1. Work log: Complete investigation summary
2. Knowledge graph: Entity created with completion observation
3. Root cause analysis: Documented from prior investigation
4. Fix verification: All steps documented
5. Test results: All results recorded

## Verdict

### Assembler Bug
**STATUS**: FIXED ✅

The double-dot label reference bug has been:
- Identified and fixed in commit ce94435
- Verified in current source code
- Protected by regression test
- Validated by full test suite (1864 tests pass)

### Source Code Differences
**STATUS**: NOT A BUG ✅

The 1042 remaining byte differences are:
- Legitimate A2osX source code changes
- Not related to assembler bugs
- Normal evolution of the codebase
- No action required

## Summary

Investigation verified that prior task xasm-siu correctly identified and fixed the bin/mv assembler bug. The fix is confirmed present and working. All remaining differences are legitimate source code changes.

**No further action required.**

## Sign-off

Task completed by: Engineer Agent
Date: 2026-03-12
Status: ✅ ACCEPTED

## Beads Task Status
Task ID: xasm-81v
Status: ✅ CLOSED
Closed: 2026-03-12
