# Task Contract: Case-Insensitive Symbol Lookup in SCMASM

**Task ID:** 2026-02-18_case-insensitive-symbols
**Beads Task:** xasm++-w0qe
**Created:** 2026-02-18
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Priority:** P1

---

## Problem

A2osX assembly fails with 488 errors for API constants like `_WRITE`, `_STRLEN`, `_STRCMP`. Symbols are defined with mixed case (`_Write`, `_Read`) but referenced in uppercase (`_WRITE`, `_READ`) through macro parameter substitution.

**Error Messages:**
```
error: Invalid argument for _WRITE: Unsupported instruction: _WRITE
error: Invalid argument for _STRLEN: Unsupported instruction: _STRLEN
error: Invalid argument for _STRCMP: Unsupported instruction: _STRCMP
... (122 unique symbols affected)
```

**Impact:** 488 errors in A2osX build (97% of remaining 504 errors)

---

## Root Cause

From Spelunker investigation (xasm++-yywb):

**Symbol Definitions** (`inc/a2osx.i`):
```scmasm
_Write      .EQ $1C
_Read       .EQ $1A
_Open       .EQ $16
_Close      .EQ $18
_FileNo     .EQ $00
_ChMod      .EQ $02
... (122 total symbols)
```

**Macro Usage** (`A2osX.API.I.txt`):
```scmasm
.MA LIBC
    ldx #_]1      ; Parameter ]1 gets uppercased by SCMASM
    jsr A2osX.LIBC
.EM
```

**User Code:**
```scmasm
>LIBC WRITE   ; Expands to: ldx #_WRITE (uppercased)
```

**Flow:**
1. User writes: `>LIBC WRITE`
2. Macro expansion: `ldx #_]1` → `ldx #_WRITE` (parameter uppercased)
3. Symbol lookup fails: `_WRITE` not found (only `_Write` exists)

**Issue:** xasm++ SCMASM parser uses case-sensitive symbol table, but historical SCMASM assemblers were case-insensitive.

---

## Solution

Implement case-insensitive symbol lookup for SCMASM parser by normalizing symbols to uppercase.

**Approach (Option A - Recommended):**

Normalize symbols to uppercase during storage and lookup in SCMASM-specific symbol handling:

1. **Symbol Definition** (`.EQ` directive):
   - Convert symbol name to uppercase before storing
   - `_Write` → stored as `_WRITE`

2. **Symbol Reference** (expression evaluation):
   - Convert symbol name to uppercase before lookup
   - `_write` → lookup `_WRITE`

3. **Scope:**
   - Apply only to SCMASM parser (not global symbol table)
   - Other parsers (Merlin, EDTASM) remain case-sensitive

---

## Affected Symbols

**122 Unique API Constants:**

`_ACCEPT`, `_ATANF`, `_ATOF`, `_ATOI`, `_ATOL`, `_BIND`, `_CHAUX`, `_CHDIR`, `_CHMOD`, `_CHOWN`, `_CHTYP`, `_CLOSE`, `_CLOSEDIR`, `_CONNECT`, `_COSF`, `_CTIME2TIME`, `_EXECL`, `_EXECV`, `_EXPAND`, `_EXPF`, `_FCLOSE`, `_FEOF`, `_FFLUSH`, `_FGETS`, `_FILENO`, `_FILESEARCH`, `_FOPEN`, `_FORK`, `_FPRINTF`, `_FPUTC`, `_FPUTS`, `_FREAD`, `_FREE`, `_FREEADDRINFO`, `_FSCANF`, `_FSEEK`, `_FSTAT`, `_FTELL`, `_FWRITE`, `_GETADDRINFO`, `_GETC`, `_GETCHAR`, `_GETCWD`, `_GETDEVBYNAME`, `_GETENV`, `_GETGRGID`, `_GETGRNAME`, `_GETHOSTNAME`, `_GETPEERNAME`, `_GETPS`, `_GETPWNAME`, `_GETPWUID`, `_GETSOCKNAME`, `_GETSOCKOPT`, `_IOCTL`, `_KILL`, `_LISTEN`, `_LOADFILE`, `_LOADTXTFILE`, `_LOGF`, `_MALLOC`, `_MKFIFO`, `_MKDIR`, `_MKNOD`, `_OPEN`, `_OPENDIR`, `_PIPE`, `_PRINTF`, `_PUTS`, `_RAND`, `_READ`, `_READDIR`, `_REALLOC`, `_RECV`, `_RECVFROM`, `_REMOVE`, `_RENAME`, `_REWIND`, `_RMDIR`, `_SELECT`, `_SEND`, `_SENDTO`, `_SETENV`, `_SETSOCKOPT`, `_SINF`, `_SLEEP`, `_SOCKET`, `_SPRINTF`, `_SQRTF`, `_SRAND`, `_STAT`, `_STRCMP`, `_STRCPY`, `_STRDUP`, `_STRLEN`, `_STRLWR`, `_STRNCMP`, `_STRNCPY`, `_STRSTR`, `_STRTOL`, `_STRTOUL`, `_STRUPR`, `_SYSTEM`, `_TANF`, `_TIME`, `_TIME2CTIME`, `_TOLOWER`, `_TOUPPER`, `_UNGETC`, `_UNLINK`, `_UNSETENV`, `_USLEEP`, `_WAIT`, `_WRITE`, `_YIELD`

---

## Acceptance Criteria

- [ ] SCMASM symbol definitions normalized to uppercase
- [ ] SCMASM symbol references normalized to uppercase during lookup
- [ ] Mixed-case symbol definitions work: `_Write .EQ $1C`
- [ ] Uppercase symbol references work: `LDA #_WRITE`
- [ ] Lowercase symbol references work: `LDA #_write`
- [ ] Other parsers (Merlin, EDTASM) remain case-sensitive
- [ ] Unit tests pass for case-insensitive symbol lookup
- [ ] A2osX build reduces from 488 to 4 errors (488 errors fixed)
- [ ] All existing tests continue to pass

---

## Test Cases

**Unit Tests to Add** (`tests/unit/test_scmasm_syntax.cpp`):

```cpp
TEST_F(ScmasmSyntaxTest, CaseInsensitiveSymbols) {
  // Define symbol with mixed case
  Parse("_Write .EQ $1C");

  // Reference with different cases
  Parse("LDA #_WRITE");   // Uppercase
  Parse("LDA #_write");   // Lowercase
  Parse("LDA #_Write");   // Mixed case

  // All should resolve to same value ($1C)
  EXPECT_EQ(EvaluateExpression("_WRITE"), 0x1C);
  EXPECT_EQ(EvaluateExpression("_write"), 0x1C);
  EXPECT_EQ(EvaluateExpression("_Write"), 0x1C);
}

TEST_F(ScmasmSyntaxTest, CaseInsensitiveInMacros) {
  // LIBC macro with case-insensitive parameter
  Parse(".MA LIBC");
  Parse("LDX #_]1");
  Parse("JSR A2osX.LIBC");
  Parse(".EM");

  // Define API constant
  Parse("_Write .EQ $1C");

  // Invoke macro with uppercase parameter
  Parse(">LIBC WRITE");

  // Should resolve _WRITE to _Write (normalized)
  // Verify generated code: LDX #$1C
}

TEST_F(ScmasmSyntaxTest, CaseInsensitiveExpressions) {
  Parse("BASE .EQ $2000");

  // Expression with different cases
  EXPECT_EQ(EvaluateExpression("BASE + $10"), 0x2010);
  EXPECT_EQ(EvaluateExpression("base + $10"), 0x2010);
  EXPECT_EQ(EvaluateExpression("Base + $10"), 0x2010);
}
```

**Integration Test:**
```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make 2>&1 | grep -c "error:.*_"
# Expected: 0 (down from 488)
```

---

## Files to Modify

1. `src/syntax/scmasm_directive_handlers.cpp`
   - `HandleEq()` - Normalize symbol name to uppercase before defining
   - Line ~10-30 (directive handler for `.EQ`)

2. `src/syntax/scmasm_syntax.cpp`
   - `EvaluateExpression()` - Normalize symbol names during lookup
   - `ParseLabel()` - Normalize label names to uppercase
   - Symbol storage and retrieval in `symbols_` map

3. `src/core/expression_parser.cpp` (if needed)
   - May need SCMASM-specific symbol normalization in expression evaluation
   - Check if symbol lookup happens here or in syntax parser

4. `tests/unit/test_scmasm_syntax.cpp`
   - Add test cases for case-insensitive symbols

---

## Implementation Strategy

**Phase 1: Symbol Definition**
1. Modify `.EQ` handler to uppercase symbol names
2. Modify label parsing to uppercase label names
3. Test: Symbol definitions with mixed case

**Phase 2: Symbol Reference**
4. Modify expression evaluation to uppercase symbol names during lookup
5. Test: Symbol references with mixed case

**Phase 3: Integration**
6. Test with A2osX build
7. Verify error reduction from 488 to 4

**Phase 4: Verification**
8. Run full xasm++ test suite
9. Verify no regressions in other parsers

---

## Dependencies

**Blocks:**
- None

**Blocked By:**
- None (independent of `:N` local labels task)

**Can Run in Parallel With:**
- xasm++-x997 (`:N` local labels task)

---

## Complexity Estimate

**Size:** Medium
- 3-4 files to modify
- ~50-100 lines of code changes
- ~50 lines of test code
- Symbol table normalization logic

**Time Estimate:** 2-3 hours
- Investigation: 30 minutes (identify all symbol storage/lookup points)
- Implementation: 60 minutes
- Testing: 45 minutes
- Integration verification: 15 minutes

---

## Success Metrics

**Error Reduction:**
- Before: 488 errors (after `:N` labels fix)
- After: 4 errors
- Reduction: 488 errors (97% of remaining)

**Test Coverage:**
- Unit tests for case-insensitive symbols
- Integration test with A2osX (122 API constants)

**Quality:**
- All existing tests pass
- No regressions in other parsers (Merlin, EDTASM remain case-sensitive)

---

## Notes

- Historical SCMASM assemblers (S-C Assembler 2.0) were case-insensitive
- Case-insensitivity should be SCMASM-specific (not global)
- This fix will resolve 97% of remaining A2osX errors
- After this fix + `:N` labels, only 4 path reference errors remain

---

## Alternative Approach (Not Recommended)

**Option B:** Normalize macro parameters during substitution

**Pros:**
- More targeted fix
- Only affects macro parameter expansion

**Cons:**
- More invasive (modifies parameter substitution logic)
- May break other use cases (e.g., string parameters)
- Doesn't address direct symbol references outside macros

**Recommendation:** Use Option A (normalize in symbol table)

---

## References

- Investigation: `.ai/tasks/2026-02-18_a2osx-remaining-errors/20-work-log.md`
- Symbol definitions: `/tmp/a2osx-build/inc/a2osx.i`
- Macro definitions: `/tmp/a2osx-build/inc/A2osX.API.I.txt`
- Test command: `cd ~/Projects/Vintage/Apple/A2osX && XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make`
