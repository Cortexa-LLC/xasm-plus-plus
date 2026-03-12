# Task Contract: Strip Editor Commands

**Task ID:** 2026-02-18_strip-editor-commands
**Beads Task:** xasm++-y9a3
**Created:** 2026-02-18
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Priority:** P1

---

## Problem

A2osX.S.txt contains Apple II line editor commands (NEW, AUTO, MAN, SAVE, ASM) that cause assembly errors.

**Impact:** 88 errors (44× MAN + 44× 3,1)

---

## Solution

Add preprocessing to SCMASM parser to strip editor command lines.

**Editor Commands to Strip:**
```
^(NEW|AUTO|MAN|SAVE|ASM|DELETE|LIST)\b
```

**Files Found:**
- Line 1: NEW
- Line 2: AUTO 3,1
- Line 708: MAN
- Line 708: SAVE usr/src/a2osx.s
- Line 709: ASM

---

## Implementation

**Files to Modify:** 1-2 files
- `src/syntax/scmasm_syntax.cpp` - Add line filtering in ParseLine()
- `tests/unit/test_scmasm_syntax.cpp` - Add editor command tests

**Token Budget:** ~6K-10K (SAFE)

---

## Acceptance Criteria

```
□ Editor commands stripped during parsing
□ A2osX build progresses past lines 1, 2, 708-709
□ All existing tests pass
□ New test: Editor commands ignored
□ Zero regressions
```

---

## Test Case

```assembly
NEW
AUTO 3,1
START   LDA #$00
        RTS
MAN
SAVE test.s
ASM
```

**Expected:** Editor commands ignored, assembly succeeds

---

**Investigation:** xasm++-jgx2 (Spelunker)
**Dependencies:** None
**Next Task:** xasm++-sduc (macro support) - BLOCKED until this completes
