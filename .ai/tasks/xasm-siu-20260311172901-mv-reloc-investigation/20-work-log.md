# Work Log: bin/mv Reloc Investigation

## Session: 2026-03-12

### Investigation Complete

#### Phase 1: Binary Diff Analysis

Compared stable (`/tmp/stable_extracted/BIN/MV.dump`) vs built (`/tmp/A2osX-335cd122-build/stage/bin/mv`):
- Both: **2434 bytes** (0B delta confirmed)
- Differences: **1044 bytes** differ
- Header identical (including entry point $2010, file size field $0161)
- Reloc table: 12 entries + terminator (26 bytes) - but entries [2] and [3] differ:
  - stable: $2491, $2493 → built: $2493, $2495 (shifted +2)

#### Phase 2: Insertion Point Discovery

Found the +2 byte insertion at vaddr **$2138** (machine code offset 270):

| Binary | Instruction | Bytes |
|--------|-------------|-------|
| stable | `LDA $0065` + `JSR $2348` + `LDA $0066` + `JSR $2348` | 12 bytes |
| built  | `LDY #$65` + `LDA ($da),Y` + `JSR $234a` + `LDY #$66` + `LDA ($da),Y` + `JSR $234a` | 14 bytes |

This inserts +2 bytes, shifting all code at vaddr ≥ $2138. Source change from `lda hSrcBasePath` (direct ZP) to `>LDA.G hSrcBasePath` (LDY + LDA(pData),Y macro).

#### Phase 3: Size Compensation Discovery

Since both files are same size, found -2 byte compensation at vaddr ~$254e:

| Binary | Instruction | Bytes |
|--------|-------------|-------|
| stable | `CMP #$40` + `BEQ +$02` + `SEC` + `RTS` + `JSR $27f5` | 8 bytes |
| built  | `CMP #$40` + `BNE +$3c` + `JSR $27f5` | 5 bytes |

The "error path" was changed: stable returns inline (SEC;RTS = 2 bytes), built branches far. Net: -2 bytes compensating the +2 from Phase 2.

#### Phase 4: JSR $0000 Bug Discovery

At vaddr $27c9 in stable ($27cb in built):
- stable: `JSR $27ce` (correct)
- built: `JSR $0000` (**BUG!**)

Generated listing with fresh xasm++ assembly showing:
```
JSR X.BasePath.X.BASEPATH..@.1  = $20 $00 $00  ← UNRESOLVED!
X.BASEPATH..1:  (label defined at $27d2)
```

#### Phase 5: Root Cause - Label Scoping Bug

**Bug:** In `src/syntax/scmasm_syntax.cpp` local label expansion code (line 1131):
```cpp
bool at_word_start =
    (k == 0) || (!std::isalnum((unsigned char)instr_operand[k - 1]) &&
                 instr_operand[k - 1] != '_');
```

For `jsr X.BasePath..1` at `k=10` (the second `.`):
- `instr_operand[k-1] = '.'` (non-alnum, non-underscore)  
- `at_word_start = true` → **WRONG!**

The second `.` in `X.BasePath..1` (a SCMASM global label with `..` notation) triggers the local label expander. Result: reference becomes `X.BASEPATH.X.BASEPATH..@.1` but definition is `X.BASEPATH..1` → forward reference never resolved → `JSR $0000`.

#### Phase 6: Fix Applied

**File:** `src/syntax/scmasm_syntax.cpp`
**Change:** Added `instr_operand[k-1] != '.'` to `at_word_start` condition:
```cpp
bool at_word_start =
    (k == 0) || (!std::isalnum((unsigned char)instr_operand[k - 1]) &&
                 instr_operand[k - 1] != '_' &&
                 instr_operand[k - 1] != '.');  // ← NEW
```

**Verification:**
- Reassembled MV.S with fixed xasm++: produces 2434 bytes
- `JSR X.BasePath..1` now resolves to `JSR $27ce` (correct)
- Fixed binary matches built binary exactly EXCEPT at the 2 bytes that were $0000 (now $ce $27)

#### Phase 7: Regression Test Added

Added `ScmasmSyntaxTest.GlobalLabelWithDoubleDotForwardReference` in:
`tests/unit/test_scmasm_syntax.cpp`

Tests that `jsr X.BasePath..1` in a function labeled `X.BasePath..` resolves to the correct address (not $0000).

### Results

- **All 1835 tests pass** (was 1834; +1 new regression test)
- Fixed binary vs built: 2 bytes differ (JSR $27ce in fixed vs JSR $0000 in built)
- Fixed binary vs stable: 1042 bytes differ (all from intentional source code changes)

### Summary of Findings

| Issue | Category | Status |
|-------|----------|--------|
| LDA abs → LDY+LDA(pData),Y at $2138 (+2 bytes) | Source code change | Expected diff, not a bug |
| BEQ+SEC+RTS → BNE at ~$254e (-2 bytes) | Source code change | Expected diff, not a bug |
| JSR X.BasePath..1 → JSR $0000 | **xasm++ BUG** | **FIXED** |

### Files Modified

1. `src/syntax/scmasm_syntax.cpp` — Fix: prevent double-dot from triggering local label expander
2. `tests/unit/test_scmasm_syntax.cpp` — New regression test

### Suggested Commit Message

```
fix(scmasm): don't treat '..' in global label names as local label start

In SCMASM, labels like 'X.BasePath..1' use double-dot notation to create
a numbered sub-label under a module ('X.BasePath.'). The second '.' before
'1' was incorrectly firing the local label expander because the previous
character '.' is non-alphanumeric (satisfying at_word_start).

This caused 'jsr X.BasePath..1' to generate mangled reference
'X.BASEPATH.X.BASEPATH..@.1' instead of 'X.BASEPATH..1', leaving the JSR
address as $0000 (unresolved forward reference).

Fix: add `instr_operand[k-1] != '.'` to the at_word_start guard so that
double-dot patterns in global label names are never mis-split.

Found during bin/mv relocation table investigation (xasm-siu task).
1835 tests pass.
```
