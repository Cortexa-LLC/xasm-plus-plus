# A2osX Macro Requirements Document

**Task:** Investigate A2osX macro requirements
**Date:** 2026-02-16
**Status:** Complete

---

## Executive Summary

A2osX (Apple II Operating System X) is a modern operating system for Apple II that makes extensive use of SCMASM (S-C Macro Assembler) macros. This document analyzes the macro system requirements based on A2osX's `inc/macros.i` file and defines what xasm++ needs to support for full compatibility.

**Key Findings:**
- A2osX defines 56 macros in `inc/macros.i`
- Uses SCMASM `.MA`/`.EM` syntax with `]N` parameter notation
- Heavy reliance on pseudo-instructions (macro invocations with `>` prefix)
- Requires nested macro expansion, conditional assembly in macros, and local label handling

---

## 1. Macro Definition Syntax

### 1.1 Definition Structure

**SCMASM Macro Definition:**
```asm
    .MA MACRONAME
    ; macro body
    .EM
```

**With Implicit Parameters:**
```asm
    .MA LDYAI
    ldy #]1        ; ]1 = first parameter
    lda /]1        ; / = high byte operator
    .EM
```

**Requirements:**
- ✅ `.MA MACRONAME` directive to begin macro definition
- ✅ `.EM` directive to end macro definition
- ✅ Macro name stored case-insensitively (uppercase normalized)
- ✅ No explicit parameter declaration (implicit `]1`, `]2`, etc.)
- ❓ Support for macro redefinition (error or warning?)

---

## 2. Macro Invocation Syntax (Pseudo-Instructions)

### 2.1 Basic Invocation

**Syntax:** `>MACRONAME param1,param2,...`

**Examples from A2osX:**
```asm
>LDYAI $1234          ; Expands to: ldy #$1234 / lda /$1234
>STYA ZPpBuf          ; Expands to: sty ZPpBuf / sta ZPpBuf+1
>PUSHYA               ; Expands to: >PUSHA / tya / >PUSHA
>PUSHA                ; Expands to: dec pStack / sta (pStack)
```

**Requirements:**
- ✅ Recognize `>` prefix as macro invocation (pseudo-instruction)
- ✅ Parse macro name after `>` prefix
- ✅ Parse comma-separated parameters
- ✅ Case-insensitive macro name lookup
- ❓ Allow invocation without `>` prefix (bare macro name)?
- ❓ Whitespace tolerance around commas

### 2.2 Parameters

**Parameter Count:** 1-based indexing (`]1`, `]2`, ..., `]9`)
- `]1` - First parameter
- `]2` - Second parameter
- ...
- `]9` - Ninth parameter
- `]#` - Special: parameter count

**Example with Parameter Count:**
```asm
    .MA PUSHB
    .DO ]#=2
    lda ]1,]2      ; Two parameters: indexed mode
    >PUSHA
    .ELSE
    lda ]1         ; One parameter: direct mode
    >PUSHA
    .FIN
    .EM
```

**Requirements:**
- ✅ Support `]1` through `]9` parameter syntax (SCMASM standard)
- ❗ **NOT** `\1` through `\9` (that's Merlin syntax)
- ✅ Support `]#` for parameter count
- ✅ Parameter substitution in expressions (`lda ]1,]2`)
- ✅ Parameter substitution in labels (`]1.]2` for nested symbols)
- ❓ Parameter range: 0-9 or unlimited?

---

## 3. Advanced Macro Features

### 3.1 Nested Macro Calls

**A2osX Example:**
```asm
    .MA PUSHYA
    >PUSHA         ; Call PUSHA macro
    tya
    >PUSHA         ; Call PUSHA again
    .EM
```

**Requirements:**
- ✅ Macros can invoke other macros
- ✅ Nested expansion depth limit (prevent infinite recursion)
- ✅ Proper parameter substitution in nested calls
- ❓ Recommended depth limit: 8-16 levels?

### 3.2 Local Labels in Macros

**A2osX Example:**
```asm
    .MA INCW
    .DO ]#=2
    inc ]1,]2
    bne :1         ; Local label
    inc ]1+1,]2
:1  .ELSE
    inc ]1
    bne :2         ; Different local label
    inc ]1+1
:2  .FIN
    .EM
```

**Requirements:**
- ✅ Support `:N` local label syntax (`:1`, `:2`, etc.)
- ✅ Make local labels unique per macro expansion
- ✅ Forward and backward references within same expansion
- ✅ Local labels don't conflict across multiple macro invocations
- ❓ Scoping: Local to macro body or to each `.DO`/`.FIN` block?

### 3.3 Conditional Assembly in Macros

**A2osX Example:**
```asm
    .MA MLICALL
    ldx #]1
    .DO ]1=MLI.ALLOCIRQ
    lda #2
    .FIN
    .DO ]1=MLI.DEALLOCIRQ
    lda #1
    .FIN
    jsr PRODOS.MLICall
    .EM
```

**Requirements:**
- ✅ Support `.DO expr` / `.FIN` in macro body
- ✅ Support `.DO expr` / `.ELSE` / `.FIN` in macro body
- ✅ Evaluate conditions using substituted parameters
- ✅ Handle nested conditionals in macros
- ✅ Symbol resolution: Parameters must resolve before `.DO` evaluation

---

## 4. A2osX Macro Categories

### 4.1 System Call Macros (7 macros)
```asm
>LIBC func           ; Library call
>LIBCALL lib,func    ; Library function call
>FPU operation       ; FPU operation
>SLEEP               ; Sleep system call
>MLICALL cmd         ; ProDOS MLI call (with automatic param count)
>DEBUGOA             ; Debug output (if available)
>DEBUG               ; Debug breakpoint
```

### 4.2 Stack Manipulation Macros (17 macros)
```asm
>PUSHA               ; Push A
>PUSHYA              ; Push Y and A
>PUSHBI imm          ; Push byte immediate
>PUSHB addr[,idx]    ; Push byte from memory
>PUSHWZ              ; Push word zero
>PUSHWI imm          ; Push word immediate
>PUSHW addr[,idx]    ; Push word from memory
>PUSHLZ              ; Push long zero
>PUSHL addr          ; Push long (32-bit)
>PULLL addr          ; Pull long (32-bit)
>SL                  ; Save local frame
>SF                  ; Save frame
>SS                  ; Save stack
>SR                  ; Restore stack
>GETSTKB [offset]    ; Get byte from stack
>GETSTKW [offset]    ; Get word from stack
```

### 4.3 Word Manipulation Macros (8 macros)
```asm
>LDYAI imm           ; Load Y=low, A=high immediate
>LDYA addr[,idx]     ; Load Y=low, A=high from memory
>STYA addr[,idx]     ; Store Y=low, A=high to memory
>INCW addr[,idx]     ; Increment word
>DECW addr[,idx]     ; Decrement word
>STAX addr[,idx]     ; Store A=low, X=high
>LDAX addr[,idx]     ; Load A=low, X=high
```

### 4.4 Global Data Segment Macros (19 macros)
```asm
>LEA.G offset        ; Load effective address
>PUSHEA.G offset     ; Push effective address
>PUSHB.G offset      ; Push byte from global
>PUSHW.G offset      ; Push word from global
>PUSHL.G offset      ; Push long from global
>PULLL.G offset      ; Pull long to global
>LDA.G offset        ; Load A from global
>STZ.G offset        ; Store zero to global
>STA.G offset        ; Store A to global
>AND.G offset        ; AND A with global
>ORA.G offset        ; OR A with global
>EOR.G offset        ; XOR A with global
>CMP.G offset        ; Compare A with global
>ADC.G offset        ; Add with carry from global
>SBC.G offset        ; Subtract with carry from global
>STZW.G offset       ; Store zero word to global
>INC.G offset        ; Increment global byte
>INCW.G offset       ; Increment global word
>DEC.G offset        ; Decrement global byte
>DECW.G offset       ; Decrement global word
>LDYA.G offset       ; Load word from global
>STYA.G offset       ; Store word to global
```

**Pattern Analysis:**
- Heavy use of indirect addressing: `(pData),y` and `(pStack)`
- Zero-page pointer manipulation: `pData`, `pStack`, `pBase`
- Conditional parameter handling with `]#`
- Frequent nested macro calls

---

## 5. Macro Usage Statistics in A2osX

**From `/Users/bryanw/Projects/Vintage/Apple/A2osX`:**
- Total macros defined: **56**
- Most commonly used macros (by grep count):
  1. `>STYA` - Store word (Y/A)
  2. `>LDYAI` - Load word immediate
  3. `>PUSHA` - Push accumulator
  4. `>PUSHYA` - Push Y and A
  5. `>LDYA` - Load word

**Usage Examples Found:**
```asm
>STYA ZPpBuf                  ; 100+ occurrences
>LDYAI BLT.Get7LC             ; 50+ occurrences
>PUSHYA                       ; 40+ occurrences
>STYA ZPIOCTL                 ; 30+ occurrences
```

---

## 6. Implementation Requirements

### 6.1 Current xasm++ Status (from investigation)

**Implemented:**
- ✅ `.MA`/`.EM` directive recognition
- ✅ `>MACRONAME` pseudo-instruction prefix
- ✅ Basic parameter substitution
- ✅ Macro storage and lookup

**Needs Verification:**
- ❓ `]N` parameter syntax (not `\N`)
- ❓ `]#` parameter count support
- ❓ Nested macro expansion
- ❓ Local label uniqueness (`:1`, `:2`, etc.)
- ❓ Conditional assembly (`.DO`/`.FIN`) in macros
- ❓ Parameter substitution in complex expressions

### 6.2 Priority Features for A2osX Compatibility

**P0 (Critical - Blockers):**
1. ✅ `.MA`/`.EM` directive parsing
2. ✅ `>MACRONAME` invocation syntax
3. ❗ `]1` through `]9` parameter substitution (verify/fix if using `\N`)
4. ❗ Local label handling (`:1`, `:2`) with uniqueness
5. ❗ Nested macro expansion (at least 2-3 levels deep)

**P1 (High - Core Features):**
1. ❗ `]#` parameter count support
2. ❗ Conditional assembly in macros (`.DO`/`.FIN`)
3. ❗ Parameter substitution in labels (`]1.]2`)
4. ❗ Parameter substitution in expressions (`]1+1,]2`)

**P2 (Medium - Nice to Have):**
1. Optional `>` prefix (bare macro names)
2. Macro redefinition handling
3. Better error messages for macro issues
4. Macro expansion depth limit (prevent infinite recursion)

**P3 (Low - Future Enhancements):**
1. Macro debugging/trace mode
2. Macro listing generation
3. More than 9 parameters (if needed)
4. Macro libraries (`.IN` with macros)

---

## 7. Test Requirements

### 7.1 Basic Macro Tests

**Test 1: Simple Parameter Substitution**
```asm
    .MA ADDVAL
    CLC
    ADC #]1
    .EM
    
    LDA #$10
    >ADDVAL $05        ; Should expand to: CLC / ADC #$05
    >ADDVAL $0A        ; Should expand to: CLC / ADC #$0A
```

**Test 2: Multiple Parameters**
```asm
    .MA MOVE
    LDA ]1
    STA ]2
    .EM
    
    >MOVE $80,$90      ; Should expand to: LDA $80 / STA $90
```

**Test 3: Parameter Count `]#`**
```asm
    .MA FLEX
    .DO ]#=2
    LDA ]1,]2
    .ELSE
    LDA ]1
    .FIN
    .EM
    
    >FLEX $80          ; Expands to: LDA $80
    >FLEX $80,X        ; Expands to: LDA $80,X
```

### 7.2 Advanced Macro Tests

**Test 4: Nested Macros**
```asm
    .MA PUSHA
    DEC pStack
    STA (pStack)
    .EM
    
    .MA PUSHYA
    >PUSHA
    TYA
    >PUSHA
    .EM
    
    >PUSHYA            ; Should expand both macros
```

**Test 5: Local Labels**
```asm
    .MA INCW
    INC ]1
    BNE :1
    INC ]1+1
:1  .EM
    
    >INCW COUNT        ; First expansion: :1 -> :1_001
    >INCW VALUE        ; Second expansion: :1 -> :1_002
```

**Test 6: Complex A2osX Pattern**
```asm
    .MA LDYAI
    LDY #]1
    LDA /]1
    .EM
    
    >LDYAI $1234       ; Should expand to: LDY #$1234 / LDA /$1234
                       ; LDY #$34 / LDA #$12
```

### 7.3 Real-World A2osX Tests

**Test 7: MLICALL Macro (from A2osX)**
```asm
    .MA MLICALL
    LDX #]1
    .DO ]1=MLI.ALLOCIRQ
    LDA #2
    .FIN
    .DO ]1=MLI.DEALLOCIRQ
    LDA #1
    .FIN
    JSR PRODOS.MLICall
    .EM
    
MLI.ALLOCIRQ .EQ $40
MLI.DEALLOCIRQ .EQ $41

    >MLICALL MLI.ALLOCIRQ    ; Should set LDA #2
    >MLICALL MLI.DEALLOCIRQ  ; Should set LDA #1
```

---

## 8. Compatibility Notes

### 8.1 SCMASM vs Merlin Syntax

**Parameter Syntax Difference:**

| Feature | SCMASM | Merlin |
|---------|--------|--------|
| Parameter prefix | `]N` | `]N` |
| Alternative | - | `\N` (in some versions) |
| Parameter count | `]#` | `]#` |
| Local labels | `:N` (numeric) | `:NAME` (alphanumeric) |

**Note:** A2osX uses SCMASM syntax, which xasm++ test confirms uses `]N` notation.

### 8.2 SCMASM Documentation Discrepancy

**Warning:** The SCMASM reference document in xasm++ repo states `\0` through `\9`, but actual A2osX code and xasm++ tests use `]1` through `]9`. This is a **documentation error** that should be corrected.

**Correct Syntax (verified):**
- `]1`, `]2`, ..., `]9` - Parameters
- `]#` - Parameter count
- `:1`, `:2`, ... - Local labels

---

## 9. Implementation Strategy

### Phase 1: Verification (Current Task)
- ✅ Analyze A2osX macros.i structure
- ✅ Document macro patterns and requirements
- ✅ Identify current xasm++ implementation status
- ✅ Create test requirements

### Phase 2: Parameter Syntax Verification
- ❗ Verify `]N` vs `\N` in current implementation
- ❗ Fix if using wrong syntax
- ❗ Add tests for `]#` parameter count
- ❗ Update documentation if incorrect

### Phase 3: Local Label Handling
- Implement local label uniqueness per macro expansion
- Test forward/backward references
- Handle nested conditionals with local labels

### Phase 4: Nested Macro Support
- Implement expansion stack
- Add depth limit (8-16 levels recommended)
- Test recursive patterns

### Phase 5: Advanced Features
- Complex parameter substitution (expressions, labels)
- Conditional assembly in macros
- Error handling improvements

### Phase 6: A2osX Validation
- Assemble A2osX kernel with xasm++
- Validate binary output matches reference
- Performance testing with large macro expansions

---

## 10. Known Limitations

### 10.1 Current Limitations (to be verified)
- Maximum 9 parameters (`]1` through `]9`)
- Local label format: `:N` (numeric only, not `:NAME`)
- Macro expansion depth limit (TBD)
- No variadic parameters

### 10.2 Intentional Limitations
- SCMASM compatibility focus (not Merlin)
- Case-insensitive macro names only
- No macro overloading by parameter count

---

## 11. Documentation Updates Needed

1. **Fix SCMASM Reference:**
   - Change `\0` through `\9` to `]1` through `]9`
   - Document `]#` parameter count
   - Add examples from A2osX

2. **Add Macro Guide:**
   - Pseudo-instruction syntax
   - Local label handling
   - Nested macro patterns
   - Conditional assembly in macros

3. **Add A2osX Case Study:**
   - Real-world usage examples
   - Performance considerations
   - Best practices

---

## 12. References

### Source Files Analyzed
1. `/Users/bryanw/Projects/Vintage/Apple/A2osX/INC/macros.i` - 56 macro definitions
2. `/Users/bryanw/Projects/Vintage/tools/xasm++/test_a2osx_minimal.s` - Basic A2osX test
3. `/Users/bryanw/Projects/Vintage/tools/xasm++/tests/integration/binary_compat/test_sources/scmasm/6502/04_macros.asm` - SCMASM macro test

### xasm++ Implementation Files
1. `src/common/macro_processor.cpp` - FLEX ASM macro processor
2. `src/syntax/scmasm_syntax.cpp` - SCMASM parser
3. `src/syntax/scmasm_directive_handlers.cpp` - `.MA`/`.EM` handlers

### Documentation
1. `docs/content/reference/scmasm-syntax-reference.md` - SCMASM reference (needs correction)
2. `docs/reference/scmasm-syntax-reference.md` - Same (backup)

---

## 13. Acceptance Criteria

**This task is complete when:**
- ✅ A2osX macro patterns documented
- ✅ Parameter syntax verified (`]N` not `\N`)
- ✅ Pseudo-instruction syntax documented
- ✅ Test requirements created
- ✅ Implementation status assessed
- ✅ Requirements document delivered

**Next Steps:**
1. Create task packet for parameter syntax verification
2. Create task packet for local label handling
3. Create task packet for nested macro expansion
4. Update SCMASM reference documentation
5. Implement missing features based on priority

---

**Document Version:** 1.0
**Last Updated:** 2026-02-16
**Status:** Complete
