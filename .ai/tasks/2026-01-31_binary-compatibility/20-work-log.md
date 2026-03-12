# Work Log: Binary Compatibility

**Task ID:** 2026-01-31_binary-compatibility
**Beads Task:** xasm++-y7q
**Engineer:** AI Engineer
**Started:** 2026-01-31

---

## Session 1: Initial Investigation (2026-01-31)

### Goals
- Understand the three binary compatibility issues
- Verify current test state
- Begin TDD implementation for Phase 1 (equate-only files)

### Actions Taken
- Reviewed task contract and implementation plan
- Confirmed Beads task status (IN_PROGRESS)
- Starting investigation of current codebase

### Discoveries

**CRITICAL BUG FOUND: Forward Reference Resolution Failure**

The primary issue affecting SEQTABLE.bin is NOT just padding - it's that forward references
in DW directives are never resolved!

**Evidence:**
```
xasm++ at 0x130: 02 a6 fb ff ff 00 00 ff 00 00 f9 01 fb 02 cf d0
vasm at 0x130:   02 a6 fb ff ff 48 31 ff 48 31 f9 01 fb 02 cf d0
                                    ^^^^    ^^^^
                                Should be addresses (0x3148=12616), not 0x0000
```

**Root Cause:**
1. `HandleDW()` evaluates expressions immediately in Pass 1
2. Forward references (e.g., `dw startrun` where `startrun` is defined later) evaluate to 0
3. DataAtom stores raw bytes `00 00`
4. Pass 2 re-encodes Instructions but NOT DataAtoms
5. Final binary has 0x0000 instead of correct address

**Technical Details:**
- Location: `src/syntax/merlin/merlin_syntax.cpp`, line 341 (HandleDW)
- Bug: `expr->Evaluate(symbols)` called during Parse, not during Encode
- DataAtom doesn't store original expression, so can't re-evaluate

**Additional Issues Found:**
1. Leading zero padding: 224 bytes (0xE0) of zeros before data starts
2. Equate-only files produce non-zero output (should be 0 bytes)

### Next Steps
- Fix forward reference bug in DW directive (highest priority)
- Fix leading zero padding
- Fix equate-only file output


## Session 2026-01-31 13:14 - Engineer Implementation Start

### Understanding Current State

Ran full test suite: All 735 tests pass
Ran POP e2e: 10/29 files assemble, 2/10 binaries match

Analyzed binary differences:
1. EQ.bin, GAMEEQ.bin, HRPARAMS.bin: Equate-only files (xasm++ produces output, vasm produces 0 bytes)
2. VERSION.bin: ASC directive (xasm++ plain ASCII, vasm high-bit)
3. SEQTABLE.bin: Leading zeros (xasm++ pads from $0000, vasm starts at ORG)

### Implementation Plan

Following TDD approach:
1. Write failing test for ASC high-bit behavior
2. Implement fix in MerlinSyntaxParser::HandleAsc
3. Verify VERSION.bin now matches
4. Verify all existing tests still pass

Starting with Issue #2 (ASC high-bit) as it's the clearest and most impactful.



## Session 2026-01-31 13:30 - Engineer Implementation Start

### Understanding Current State

Ran full test suite: All 735 tests pass
Ran POP e2e: 10/29 files assemble, 2/10 binaries match

Analyzed binary differences:
1. EQ.bin, GAMEEQ.bin, HRPARAMS.bin: Equate-only files (xasm++ produces output, vasm produces 0 bytes)
2. VERSION.bin: ASC directive (xasm++ plain ASCII, vasm high-bit)
3. SEQTABLE.bin: Leading zeros (xasm++ pads from $0000, vasm starts at ORG)

### Implementation Plan

Following TDD approach:
1. Write failing test for ASC high-bit behavior
2. Implement fix in MerlinSyntaxParser::HandleAsc
3. Verify VERSION.bin now matches
4. Verify all existing tests still pass

Starting with Issue #2 (ASC high-bit) as it's the clearest and most impactful.

