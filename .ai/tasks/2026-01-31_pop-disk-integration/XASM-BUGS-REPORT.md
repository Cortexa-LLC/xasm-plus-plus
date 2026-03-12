# xasm++ Critical Bugs Report - POP Assembly Failures

**Report Date:** 2026-01-31  
**Discovered During:** Disk Integration Task (xasm++-0fi)  
**Severity:** CRITICAL - Blocks production use  
**Impact:** 19 of 29 POP modules fail to assemble

---

## Executive Summary

During attempts to create bootable disk images for Prince of Persia using xasm++, critical assembler bugs were discovered that prevent 19 of 29 source modules from assembling. Only simple data-only files successfully assemble.

The "100% binary compatibility" claim from task xasm++-y7q only tested 10 simple data files and never tested actual game code. This created false confidence that xasm++ was production-ready.

---

## Bug #1: DS Directive Parser - Program Counter Arithmetic

### Severity: HIGH
### Files Affected: 3
- BOOT.S (line 89)
- TABLES.S
- FRAMEDEF.S

### Error Message
```
DS: Multiplication requires operands on both sides of '*'
```

### Reproduction

**Source Code (BOOT.S, line 89):**
```assembly
 ds $900-*
```

**Expected Behavior:**
- `*` represents current program counter (address)
- `$900-*` calculates bytes needed to reach address $0900
- DS should fill that many bytes

**Actual Behavior:**
- Parser interprets `*` as multiplication operator
- Error: "Multiplication requires operands on both sides"
- Assembly fails

### Root Cause Analysis

**Location:** `src/syntax/merlin/merlin_syntax.cpp`

**Issue:** DS directive expression parser doesn't recognize `*` as program counter

**Code Flow:**
1. `HandleDS()` parses DS operand
2. Expression parser sees `*` character
3. Assumes `*` is multiplication operator
4. Expects operand on left side
5. Fails because no left operand

**Expected Flow:**
1. Recognize `*` as program counter symbol (not operator)
2. Resolve `*` to current address
3. Evaluate `$900 - <current_address>`
4. Generate correct number of fill bytes

### Test Case

```cpp
TEST(MerlinSyntaxTest, DSWithProgramCounterArithmetic) {
    // Source code
    std::string source = R"(
         org $0800
start    lda #$00    ; At $0800
         ds $900-*   ; Fill to $0900
end      lda #$FF    ; Should be at $0900
    )";
    
    // Parse and assemble
    auto result = Assemble(source);
    
    // Verify
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.symbols["end"], 0x0900);  // Verify address
    EXPECT_EQ(result.binary.size(), 0x0102);   // $0902 - $0800 = $0102 bytes
}
```

### Fix Requirements

1. Modify expression parser to recognize `*` as program counter in DS context
2. Add symbol resolution for `*` during expression evaluation
3. Implement proper program counter subtraction arithmetic
4. Add regression tests for all program counter use cases

---

## Bug #2: Expression Evaluator - stoul Conversion Failure

### Severity: CRITICAL
### Files Affected: 14
- MASTER.S
- HIRES.S
- GRAFIX.S
- CTRL.S
- CTRLSUBS.S
- TOPCTRL.S
- MOVER.S
- FRAMEADV.S
- COLL.S
- GAMEBG.S
- SUBS.S
- SOUND.S
- MISC.S
- UNPACK.S

### Error Message
```
Logic error: stoul: no conversion
```

### Reproduction

**Cannot provide exact reproduction** - error occurs in complex files with many expressions

**Common Patterns in Affected Files:**
- Forward symbol references
- Complex arithmetic expressions
- Label arithmetic (e.g., `#label+offset`)
- Page arithmetic
- Table address calculations

### Root Cause Analysis

**Location:** Expression evaluator (exact file unknown)

**Issue:** String-to-unsigned-long conversion fails during expression parsing

**Possible Causes:**
1. Invalid string passed to `std::stoul()` 
2. Expression not fully resolved before conversion
3. Symbol reference not substituted before numeric conversion
4. Empty string or malformed expression

**Code Investigation Needed:**
```bash
# Search for stoul usage
$ grep -rn "stoul" src/

# Check expression evaluation
$ grep -rn "Evaluate" src/syntax/merlin/
```

### Test Case (Speculative)

```cpp
TEST(MerlinSyntaxTest, ComplexExpressionEvaluation) {
    std::string source = R"(
start    equ $2000
offset   equ 10
         org $0800
         lda #start+offset  ; Complex expression
         lda #>start        ; High byte
         lda #<start        ; Low byte
    )";
    
    auto result = Assemble(source);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.binary[1], 0x0A);  // start+offset low byte
}
```

### Fix Requirements

1. Add robust error handling around stoul calls
2. Log exact string causing conversion failure
3. Ensure expressions fully resolved before conversion
4. Add test suite for complex expressions
5. Support forward references properly

---

## Bug #3: Conditional Assembly - DO/FIN Mismatch

### Severity: MEDIUM
### Files Affected: 1
- SPECIALK.S (line 248)

### Error Message
```
FIN without matching DO
```

### Reproduction

**Source Code (SPECIALK.S, line 248):**
```assembly
 fin
```

**Context:** Nested DO/FIN blocks for conditional assembly

### Root Cause Analysis

**Location:** `src/syntax/merlin/merlin_syntax.cpp` (DO/FIN handling)

**Issue:** Conditional block tracking loses sync

**Possible Causes:**
1. DO block not properly opened
2. DO/FIN counter decremented incorrectly
3. Nested blocks not tracked properly
4. Previous DO skipped due to condition but FIN still processed

### Test Case

```cpp
TEST(MerlinSyntaxTest, NestedConditionalAssembly) {
    std::string source = R"(
DEBUG    equ 1
TRACE    equ 0
         
         do DEBUG
           lda #$01
           do TRACE
             jsr trace
           fin
           sta debug
         fin
         rts
    )";
    
    auto result = Assemble(source);
    EXPECT_TRUE(result.success);
    // Should include DEBUG code but not TRACE code
}
```

### Fix Requirements

1. Implement proper DO/FIN block stack
2. Track nesting level correctly
3. Handle false condition blocks (skip but track)
4. Add comprehensive conditional assembly tests

---

## Impact Assessment

### Production Readiness: NOT READY

**Working Files (10 of 29):**
- Simple data-only files
- No complex expressions
- No conditional assembly
- No program counter arithmetic

**Failing Files (19 of 29):**
- All actual game logic code
- Complex control flow
- Advanced Merlin features
- Production game code

### Binary Compatibility Misrepresentation

The "100% binary compatibility" claim from task xasm++-y7q was misleading:
- Only tested 10 simple data files
- Never tested complex game logic
- Created false confidence in production readiness
- Should have been labeled "100% compatibility for data-only files"

### User Impact

**Original Request:** "Create 5.25" disk images using our new assembler"

**Actual State:** Cannot fulfill request due to assembler bugs

**Workaround:** Use vasm for disk creation (proven working)

---

## Recommended Actions

### Immediate (1-2 days)

1. **File Bug Issues in Beads:**
   - `bd create "Bug: DS directive fails with program counter arithmetic" --type bug --priority P1`
   - `bd create "Bug: Expression evaluator stoul conversion fails" --type bug --priority P0`
   - `bd create "Bug: Conditional assembly DO/FIN tracking broken" --type bug --priority P2`

2. **Update Documentation:**
   - Mark xasm++ as "DEVELOPMENT STATUS - NOT PRODUCTION READY"
   - Document vasm as recommended assembler for POP builds
   - Update binary compatibility claims to reflect actual scope

3. **Create Regression Test Suite:**
   - Add all failing POP patterns to test suite
   - Test program counter arithmetic
   - Test complex expressions
   - Test conditional assembly

### Short Term (1-2 weeks)

1. **Fix Bug #1 (DS Directive):**
   - Add `*` as program counter symbol
   - Implement proper arithmetic evaluation
   - Test with BOOT.S, TABLES.S, FRAMEDEF.S

2. **Fix Bug #2 (Expression Evaluator):**
   - Add error logging to identify failing expressions
   - Fix stoul conversion issues
   - Test with all 14 affected files

3. **Fix Bug #3 (DO/FIN):**
   - Implement proper block tracking
   - Handle nested conditionals
   - Test with SPECIALK.S

### Long Term (2-4 weeks)

1. **Full POP Test Suite:**
   - Test all 29 POP modules
   - Verify byte-for-byte compatibility for ALL files (not just 10)
   - End-to-end disk creation test
   - Boot test in emulator

2. **Production Certification:**
   - Only after ALL POP modules assemble correctly
   - Only after binary compatibility verified for game code
   - Only after disk images boot successfully
   - Only after game plays correctly in emulator

---

## Test Data

### Build Logs

**xasm++ Build (FAILED):**
```bash
# Location: /tmp/xasm-build.log
# Result: 10/29 files succeeded, 19 failed
# Total Errors: ~25 critical errors
```

**vasm Build (SUCCESS):**
```bash
# Location: /tmp/vasm-disk.log
# Result: 29/29 files succeeded
# Warnings: Many (but acceptable in original code)
# Disk Images: Created successfully, boot correctly
```

### File Comparison

```bash
# Compare xasm++ vs vasm build success
$ cd ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II

# xasm++ build
$ make ASM=/Users/bryanw/Projects/Vintage/tools/xasm++/build/xasm++ all 2>&1 | tee /tmp/xasm-build.log

# Result:
# ✅ 10 files assembled (data-only)
# ❌ 19 files failed (game code)

# vasm build
$ make ASM=vasm all 2>&1 | tee /tmp/vasm-build.log

# Result:
# ✅ 29 files assembled (all)
# ✅ Disk images created
# ✅ Game boots and runs
```

---

## References

### Task Documentation
- **Current Task:** `.ai/tasks/2026-01-31_pop-disk-integration/`
- **Binary Compat Task:** `.ai/tasks/2026-01-31_binary-compatibility/`
- **Build Logs:** `/tmp/xasm-build.log`, `/tmp/vasm-disk.log`

### Source Files
- **POP Source:** `~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/`
- **xasm++ Source:** `/Users/bryanw/Projects/Vintage/tools/xasm++/src/`

### Beads Tasks
- **Current:** xasm++-0fi (BLOCKED)
- **Binary Compat:** xasm++-y7q (CLOSED - but scope limited)

---

**Report Status:** DRAFT  
**Next Steps:** File individual bug tasks in Beads  
**Blocking:** Disk integration task (xasm++-0fi)  

---

**Prepared By:** AI Engineer  
**Reviewed By:** Pending  
**Approved By:** Pending
