# Task Contract: SCMASM Macro Support

**Task ID:** 2026-02-18_scmasm-macros
**Beads Task:** xasm++-sduc
**Created:** 2026-02-18
**Assigned Role:** Engineer
**Workflow:** Feature
**Priority:** P2
**Dependencies:** xasm++-y9a3 (editor commands)

---

## Problem

A2osX uses SCMASM macros extensively. xasm++ doesn't support macro system.

**Impact:** 156 macro invocation errors

**Top Macros:**
- 132× >LDYAI (Load Y and A immediate)
- 24× >STYA (Store Y and A)
- 8× >DEBUGOA (Debug output)

---

## Solution

Implement SCMASM macro system with definition and invocation support.

---

## Required Features

### 1. Macro Definition
```assembly
.MA LDYAI          ; Macro name
        LDY #]1    ; Parameter ]1
        LDA #]2    ; Parameter ]2
.EM                ; End macro
```

### 2. Macro Invocation
```assembly
>LDYAI 5,10       ; Expands to: LDY #5 / LDA #10
```

### 3. Parameter Substitution
- `]1`, `]2`, `]3`, etc. - Positional parameters
- Expand during macro invocation

### 4. Local Label Scoping
- Labels inside macros scoped to invocation
- Prefix: `:LOOP` becomes unique per call

---

## Implementation

**Files to Modify:** 5-8 files
- `include/xasm++/macro_processor.h` - New macro system
- `src/core/macro_processor.cpp` - Macro expansion engine
- `src/syntax/scmasm_directive_handlers.cpp` - Add .MA/.EM
- `src/syntax/scmasm_syntax.cpp` - Add > prefix parsing
- `tests/unit/test_scmasm_macros.cpp` - New tests

**Token Budget:** ~15K-24K (ACCEPTABLE)

---

## Acceptance Criteria

```
□ .MA directive defines macros
□ .EM directive ends macro definition
□ >MacroName invokes macros
□ ]1, ]2, etc. parameter substitution works
□ Local labels scoped correctly
□ All existing tests pass
□ A2osX macros >LDYAI, >STYA work
□ Zero regressions
```

---

## Test Cases

### Test 1: Simple Macro
```assembly
.MA CLEAR
        LDA #$00
.EM

START   >CLEAR
```

### Test 2: With Parameters
```assembly
.MA LDYAI
        LDY #]1
        LDA #]2
.EM

START   >LDYAI 5,10
```

### Test 3: Local Labels
```assembly
.MA COPYBYTE
:LOOP   LDA SRC,X
        STA DST,X
        DEX
        BNE :LOOP
.EM
```

---

**Investigation:** xasm++-jgx2 (Spelunker)
**Blocks:** xasm++-y9a3 (MUST complete first)
**Complexity:** Large (3-5 days estimate)
