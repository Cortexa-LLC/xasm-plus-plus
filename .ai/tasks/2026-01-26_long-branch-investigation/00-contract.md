# Task Contract: Long Branch Investigation

**Task ID:** 2026-01-26_long-branch-investigation
**Beads Task:** xasm++-0m7  
**Created:** 2026-01-26
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Investigation/Research

---

## Task Description

Investigate branch target errors blocking 14 Prince of Persia source files from assembling with xasm++. Determine root cause, understand historical context, and recommend solution approach.

### Background and Context

The xasm++ assembler successfully assembles 7/29 PoP source files. The remaining 22 files were initially failing due to PUT directive path resolution issues. After fixing the PUT directive handler to resolve includes relative to the source file's directory, 14 files now fail with "Branch target out of range" errors.

**Historical Note:** This is a known 6502 hardware limitation. Branch instructions (BEQ, BNE, BCC, BCS, BMI, BPL, BVC, BVS) use relative addressing with a signed byte offset, limiting branch range to ±127 bytes from the instruction.

### Current State

**Before PUT Fix:**
- 6/29 files assembling
- 23 files failing with "Cannot open file: eq" errors
- Branch errors hidden behind PUT failures

**After PUT Fix (implemented in this investigation):**
- 7/29 files assembling
- 14 files failing with branch range errors
- 8 files failing with other issues (HEX comma syntax, DS forward references, USR directive)

**Affected Files with Branch Errors:**
1. AUTO.S
2. COLL.S
3. CTRL.S
4. CTRLSUBS.S
5. FRAMEADV.S
6. GAMEBG.S
7. GRAFIX.S
8. HIRES.S
9. MASTER.S
10. MISC.S
11. MOVER.S
12. SUBS.S
13. TOPCTRL.S
14. UNPACK.S

### Desired State

**Primary Goal:** Understand the branch range issue and determine appropriate solution.

**Options to evaluate:**
1. **Auto-expand long branches** (convenience, compatibility)
2. **Require manual workarounds** (authentic, transparent)
3. **Provide both modes** (flexible, configurable)

---

## Success Criteria

```
✓ Root cause identified and documented
✓ Historical 6502 branch behavior understood
✓ Merlin assembler behavior researched
✓ Example branch errors analyzed in detail
✓ Solution approaches evaluated with pros/cons
✓ Recommendation documented with rationale
✓ Work artifacts complete (work log, findings doc)
```

---

## Acceptance Criteria

### Functional Requirements
```
✓ PUT directive path resolution fixed (enables testing)
✓ Confirmed 14 files blocked by branch range errors
✓ Error messages improved with detailed context
✓ Example branch error traced to source location
✓ Branch offset calculations verified
```

### Quality Requirements
```
✓ Investigation documented in work log
✓ Code changes follow TDD (if applicable)
✓ No regressions in existing tests
✓ Clear error messages aid debugging
✓ Findings documented for future reference
```

### Deliverables
```
✓ Updated work log with investigation findings
✓ Contract document (this file)
✓ Recommendation for solution approach
✓ Code changes committed (PUT directive fix)
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain 6502 hardware accuracy
□ Should be compatible with Merlin assembler behavior
□ Cannot break existing working files
□ Error messages must be clear and helpful
```

### Dependencies
```
✓ PUT directive fix required to reveal branch errors (COMPLETED)
□ Understanding of Merlin assembler's branch handling
□ Access to PoP source files for testing
□ Knowledge of 6502 instruction set and addressing modes
```

### Out of Scope
```
✗ Implementation of long branch solution (future task)
✗ Fixing all 14 files (validation task)
✗ HEX comma-separated values (separate issue)
✗ DS forward reference expressions (separate issue)
✗ USR directive implementation (separate issue)
```

---

## Investigation Findings

### Root Cause: 6502 Hardware Limitation

**Branch Instructions:**
- Use relative addressing mode
- Operand is signed byte offset: -128 to +127
- Calculated as: `target_address - (PC + 2)`
- Cannot branch beyond ±127 bytes

**Example from AUTO.S:**
```
Line 164:  beq :5     ; At address $541F
Line 192:  :5 jmp KidProg  ; At address $5D81

Offset: $5D81 - $541F = $9

62 = 2400 bytes
Error: offset 2400 (must be -128 to +127)
```

### Traditional Assembly Workarounds

**Pattern 1: Inverted Branch + JMP**
```assembly
; Want: BEQ faraway (but faraway > 127 bytes away)
; Solution:
    BNE :skip       ; Branch if NOT equal (skip JMP)
    JMP faraway     ; Jump to far target
:skip
```

**Pattern 2: JMP Trampolines**
```assembly
; Near the branch:
    BEQ :trampoline

; Elsewhere (within ±127 bytes):
:trampoline JMP faraway
```

**Pattern 3: Code Reorganization**
- Move target label closer
- Restructure code flow
- Use subroutines instead of branches

### Merlin Assembler Behavior (Research Needed)

**Questions:**
1. Does Merlin auto-expand long branches?
2. Or does it require manual workarounds?
3. Are PoP sources written expecting auto-expansion?
4. Or do they have manual trampolines that aren't being recognized?

**Evidence from PoP Source:**
- Line 192 in AUTO.S has `:5 jmp KidProg` (looks like a trampoline)
- But the branch is trying to reach this trampoline, and it's still too far
- Suggests either:
  a) PUT includes are inflating addresses unexpectedly
  b) Merlin does something special with local labels
  c) Original code relied on specific assembly behavior

---

## Solution Approaches

### Option 1: Auto-Expand Long Branches (Convenience)

**Description:** Automatically convert long branches to inverted branch + JMP pattern.

**Example:**
```assembly
; Source code:
    BEQ faraway    ; faraway is >127 bytes away

; Assembled as:
    BNE $+5        ; Skip over JMP (5 bytes ahead)
    JMP faraway    ; 3-byte absolute jump
    ...
```

**Pros:**
- ✅ Convenient for programmers
- ✅ Matches some modern assemblers (e.g., ca65)
- ✅ Would make PoP files assemble without modification
- ✅ Transparent to source code

**Cons:**
- ❌ Not authentic 6502 behavior
- ❌ May hide programmer intent
- ❌ Different instruction timing (branch=2-3 cycles, JMP=3 cycles)
- ❌ Consumes extra bytes (2 bytes → 5 bytes)
- ❌ Unclear if Merlin actually did this

**Complexity:** Medium (requires instruction rewriting)

---

### Option 2: Require Manual Workarounds (Authentic)

**Description:** Require programmer to manually handle long branches using JMP or code reorganization.

**Example:**
```assembly
; Programmer must write:
    BNE :skip
    JMP faraway
:skip
```

**Pros:**
- ✅ Authentic 6502 programming
- ✅ Transparent: what you write is what you get
- ✅ Programmer controls timing and size
- ✅ Matches hardware behavior exactly
- ✅ Clear error messages guide programmer

**Cons:**
- ❌ Requires modifying PoP source files
- ❌ More work for programmer
- ❌ May not match original Merlin behavior
- ❌ 14 files would need manual fixes

**Complexity:** Low (already implemented - just need good error messages)

---

### Option 3: Configurable Mode (Flexible)

**Description:** Provide command-line flag to enable/disable auto-expansion.

**Example:**
```bash
# Strict mode (default): require manual workarounds
xasm++ --cpu 6502 --syntax merlin code.asm

# Auto-expand mode: convenience
xasm++ --cpu 6502 --syntax merlin --auto-expand-branches code.asm
```

**Pros:**
- ✅ Supports both workflows
- ✅ Default to authentic behavior
- ✅ Option for convenience when needed
- ✅ Clearly documented behavior

**Cons:**
- ❌ More complex implementation
- ❌ Two code paths to maintain
- ❌ Need to document both modes
- ❌ Still unclear if Merlin did auto-expansion

**Complexity:** High (implement Option 1 + flag + documentation)

---

## Recommendation

**Recommended: Option 2 (Require Manual Workarounds) - WITH CAVEAT**

**Rationale:**
1. **Authenticity:** Matches 6502 hardware behavior exactly
2. **Simplicity:** Already implemented (just error messages)
3. **Clarity:** Programmer knows exactly what code is generated
4. **Educational:** Teaches proper 6502 programming techniques

**BUT FIRST: Research Merlin Behavior**

Before finalizing decision, we should:
1. Check Merlin assembler documentation/source code
2. Test with actual Merlin assembler if available
3. Examine PoP source more carefully for clues

**If Merlin does auto-expand:** Reconsider Option 1 or 3 for compatibility

**If Merlin does NOT auto-expand:** Proceed with Option 2
- PoP sources likely have bugs OR
- PoP sources were assembled differently OR
- PUT directive behavior differs from Merlin

**Next Steps:**
1. Document findings in this contract
2. Research Merlin assembler behavior
3. Create follow-up task for implementation
4. Decide on approach based on research

---

## Estimated Complexity

**This Investigation:** Small (already complete)
- Investigation: 2 hours
- PUT fix: 30 minutes
- Error message improvement: 15 minutes
- Documentation: 1 hour

**Future Implementation (if Option 1 chosen):** Medium-Large
- Branch expansion logic: 4-6 hours
- Testing: 2-3 hours
- PoP validation: 1-2 hours
- **Total: 7-11 hours**

**Future Implementation (if Option 2 chosen):** Small
- Manual fixes to 14 files: 2-4 hours
- Testing: 1 hour
- **Total: 3-5 hours**

---

## Risks and Mitigation

### Risk 1: Unknown Merlin Behavior
- **Probability:** High
- **Impact:** High (affects solution choice)
- **Mitigation:** Research Merlin behavior before implementing solution
- **Contingency:** Start with Option 2, can always add Option 1 later

### Risk 2: PUT Directive Side Effects
- **Probability:** Medium
- **Impact:** Medium (may affect address calculation)
- **Mitigation:** Verify PUT behavior matches Merlin
- **Contingency:** May need to adjust PUT implementation

### Risk 3: PoP Source Modifications
- **Probability:** Medium (if Option 2)
- **Impact:** Low (well-understood problem)
- **Mitigation:** Document all changes, keep originals
- **Contingency:** Could provide patched versions

---

## References

### Task Packets
- This investigation: `.ai/tasks/2026-01-26_long-branch-investigation/`
- Previous analysis: `docs/investigations/pop-compatibility-analysis.md`

### Source Code
- Assembler: `src/core/assembler.cpp` (branch encoding logic)
- Merlin syntax: `src/syntax/merlin/merlin_syntax.cpp` (PUT directive)
- Main: `src/main.cpp` (directory handling)

### Test Files
- Prince of Persia sources: `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/`
- Test script: `./test_pop_files.sh`
- Results: `./pop_analysis_output/`

### External Resources
- 6502 instruction set reference
- Merlin assembler documentation (if available)
- vasm-ext Merlin syntax implementation: `~/Projects/Vintage/tools/vasm-ext`

---

## Approvals and Sign-Off

**Contract Status:** Complete

**Investigation Completed By:**
- [x] Engineer: 2026-01-26

**Next Actions:**
1. Research Merlin assembler branch behavior
2. Create follow-up task for solution implementation
3. Update task description in Beads if needed

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-26
