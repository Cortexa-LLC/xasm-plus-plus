# Task Brief: Validate A2osX Assembly

**Created**: 2026-02-16  
**Status**: BLOCKED  
**Priority**: HIGH  
**Assigned**: Tester Agent  

---

## Objective

Rebuild xasm++, install, and validate that A2osX assembles successfully after the .ED/.FI/.EL directive implementation.

---

## Dependencies

- **Prerequisite**: xasm++-xmwf task (all tests passing)
- **Source tree**: /Users/bryanw/Projects/Vintage/tools/xasm++
- **Target project**: /Users/bryanw/Projects/Vintage/Apple/A2osX

---

## Outcome

### ✅ Completed
- xasm++ rebuilt successfully (no build errors)
- xasm++ installed to /usr/local/bin/xasm++
- Installation verified

### ❌ Blocked
- **A2osX assembly FAILED**
- **Blocking issue**: Missing support for Merlin editor directives NEW and AUTO

---

## Critical Finding

**Issue**: A2osX source files contain Merlin editor directives (NEW, AUTO) that are not recognized by xasm++ SCMASM parser.

**Evidence**:
```
A2OSX.S.txt starts with:
NEW
  AUTO 3,1
```

**Error**:
```
error: Invalid argument for 3,1: Unsupported instruction: 3,1
```

**Impact**: 156+ A2osX source files affected. Cannot validate A2osX until fixed.

---

## Required Fix

Implement NEW and AUTO as no-op directives in SCMASM syntax parser:

1. Add HandleNew() function (no-op)
2. Add HandleAuto() function (no-op)
3. Register directives in InitializeDirectiveRegistry()
4. Add test coverage

**Estimated effort**: 1-2 hours

---

## Files

- **Work log**: 20-work-log.md (detailed investigation)
- **Test report**: 30-review.md (validation findings)
- **This brief**: 00-brief.md

---

## Next Action

**Requires**: Engineer to implement NEW/AUTO directive support  
**Then**: Tester re-validates A2osX assembly  
**Goal**: Confirm A2osX builds successfully with xasm++
