# Task Contract

**Task ID:** 2026-02-16_fix-inb-directive
**Beads Task:** xasm++-cqxw
**Created:** 2026-02-16
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Bugfix

---

## Task Description

Fix the .INB directive to properly include and parse files so that symbols defined in included files become available to the main assembly.

### Background and Context

S-C Macro Assembler uses .INB directive to include other source files. A2osX uses this extensively to include system constants and macros from files like `inc/mli.i`.

Currently, the .INB directive exists but doesn't properly include/parse the file contents, causing symbols defined in included files to remain undefined.

Example from A2osX.S.txt:
```assembly
Line 9:  .INB inc/mli.i
Line 77: .DA #MLI.GETPREFIX  ; Fails with "Undefined symbol: MLI.GETPREFIX"
```

The symbol `MLI.GETPREFIX` is defined in `inc/mli.i:25` as `.EQ $C7` but isn't available after the .INB directive.

### Current State

- .INB directive handler exists in `src/syntax/scmasm_directive_handlers.cpp`
- Parser recognizes .INB but doesn't process file contents properly
- Included files are not being parsed as if they were inline
- Symbols from included files remain undefined
- A2osX fails at line 77 with "Undefined symbol: MLI.GETPREFIX"

### Desired State

- .INB directive reads and parses included files
- Symbols defined in included files are available to the main assembly
- Forward and backward references across includes work correctly
- A2osX assembles past line 77 (and ideally much further)
- Compatible with S-C Macro Assembler include behavior

---

## Success Criteria

```
✓ inc/mli.i successfully included and parsed
✓ MLI.GETPREFIX symbol defined and available
✓ A2osX.S.txt assembles past line 77
✓ All existing tests still pass
✓ New tests added for .INB directive
✓ Code coverage ≥ 85%
```

---

## Acceptance Criteria

### Functional Requirements
```
□ .INB directive reads included file
□ Included file parsed as S-C Macro Assembler syntax
□ Symbols from included files available to main file
□ Relative paths resolved correctly (relative to main file location)
□ Nested includes supported (.INB within included file)
□ Forward references work across include boundaries
□ Error handling for missing/unreadable files
```

### Quality Requirements
```
□ All existing tests passing (1673/1673)
□ New unit tests for .INB directive (minimum 5 test cases)
□ Code coverage ≥ 85%
□ No regressions in symbol handling
□ Documentation updated
```

### Non-Functional Requirements
```
□ Parser performance not degraded
□ Error messages clear and helpful
□ Consistent with S-C Macro Assembler behavior
□ Include depth limit to prevent infinite recursion
```

---

## Constraints and Dependencies

### Constraints
- Must maintain existing .INB directive syntax
- Must not break other syntax parsers (Merlin, etc.)
- Must handle relative and absolute paths
- Must prevent infinite include loops

### Dependencies
- Requires understanding of scmasm_syntax.cpp ParseLine() flow
- May need to modify directive handler pattern
- Affects symbol table population in first pass

### Related Files
- `src/syntax/scmasm_syntax.cpp` - Main parser
- `src/syntax/scmasm_directive_handlers.cpp` - .INB handler
- `tests/unit/test_scmasm_syntax.cpp` - Unit tests
- `~/Projects/Vintage/Apple/A2osX/inc/mli.i` - Real include file to test

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 3 files (scmasm_syntax.cpp, scmasm_directive_handlers.cpp, test_scmasm_syntax.cpp)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 3 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
3 × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 9,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (this task only)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose

---

## Technical Notes

### S-C Macro Assembler .INB Behavior

**Expected behavior:**
1. Read included file from disk
2. Parse it line-by-line as if inline in main file
3. Symbols defined in included file added to global symbol table
4. Labels in included file resolved normally
5. Nested includes processed recursively
6. Relative paths resolved relative to main file directory

**Include path resolution:**
```assembly
; Main file: src/A2osX.S.txt
.INB inc/mli.i  ; Resolves to src/inc/mli.i
```

### Implementation Approach

**Option 1: Recursive ParseFile()**
- Create ParseFile() method that takes filename
- Main assembly calls ParseFile(main_file)
- .INB handler calls ParseFile(included_file)
- Maintains same parsing state across includes

**Option 2: Inline Expansion**
- .INB handler reads included file
- Inserts lines into current line buffer
- Parser continues as if lines were inline
- Simpler but may affect line number tracking

**Recommendation:** Option 1 (cleaner separation, better error messages)

### Test Cases

**Test Case 1: Simple include**
```assembly
; main.asm
.INB inc.asm

; inc.asm
VALUE .EQ $FF
```

**Test Case 2: Symbol usage**
```assembly
; main.asm
.INB inc.asm
  LDA VALUE  ; Should resolve to $FF

; inc.asm
VALUE .EQ $FF
```

**Test Case 3: Nested includes**
```assembly
; main.asm
.INB inc1.asm

; inc1.asm
.INB inc2.asm

; inc2.asm
VALUE .EQ $42
```

**Test Case 4: Forward reference across include**
```assembly
; main.asm
  JMP LABEL
.INB inc.asm

; inc.asm
LABEL NOP
```

**Test Case 5: Missing file error**
```assembly
.INB nonexistent.asm  ; Should give clear error
```

---

## Resources and References

### Relevant Files
```
- src/syntax/scmasm_syntax.cpp - Main S-C Macro Assembler parser
- src/syntax/scmasm_directive_handlers.cpp - Current .INB handler
- tests/unit/test_scmasm_syntax.cpp - Unit tests
- ~/Projects/Vintage/Apple/A2osX/inc/mli.i - Real include file to verify
```

### Related Tasks
```
- xasm++-ijxa - Inline comments (completed)
- xasm++-6isx - Phase directives (completed)
- xasm++-786z - Dots in symbols (completed)
```

---

## Assumptions

```
1. Include files use the same syntax as the main file (S-C Macro Assembler)
2. Include paths are relative to main file directory by default
3. Circular includes are not intentionally used in real code
4. Include depth limit of 10 is reasonable
5. All files are UTF-8 text files
```

---

## Risk Assessment

### Identified Risks
```
1. Infinite include loops
   - Probability: Low
   - Impact: High (stack overflow)
   - Mitigation: Track include stack, enforce depth limit

2. Path resolution ambiguity
   - Probability: Medium
   - Impact: Medium (wrong file included)
   - Mitigation: Clear documentation, test with real A2osX files

3. Line number tracking in error messages
   - Probability: Medium
   - Impact: Low (confusing errors)
   - Mitigation: Track source file and line separately
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User (2026-02-16)
- [ ] Agent: Engineer (to be approved)

---

## Notes

This is the final blocking issue for A2osX line 77. Once .INB works, A2osX should assemble significantly further (possibly to completion if no other issues exist).

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
