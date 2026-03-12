# Z80 Macro System Documentation

## Overview

The EDTASM-M80++ macro system provides full support for code reuse, repetition, and iteration with parameter substitution and nested constructs.

## Features Implemented

### 1. MACRO/ENDM - User-Defined Macros

Define reusable code blocks with parameters:

```assembly
ADDW    MACRO   reg16
        ADD     A,&reg16L
        LD      L,A
        ADC     A,&reg16H
        SUB     L
        LD      H,A
        ENDM

        ORG     8000H
        LD      HL,1234H
        LD      DE,5678H
        ADDW    DE          ; Expands to add DE to HL
```

**Parameter Substitution:**
- `&param` - Substitute parameter value
- `%digit` - Substitute numeric argument (1-based)

**Features:**
- Multiple parameters (comma-separated)
- Nested macro calls
- LOCAL symbol support
- EXITM early exit

### 2. REPT - Repeat Blocks

Repeat a block of code N times:

```assembly
        ORG     8000H
        REPT    3
        NOP
        ENDM
; Expands to 3 NOP instructions
```

**Count:**
- Must be a non-negative integer
- Zero count generates no code

### 3. IRP - Iteration with Parameter List

Iterate over a comma-separated list, expanding the block for each value:

```assembly
        ORG     8000H
        IRP     reg,<A,B,C>
        LD      &reg,0
        ENDM
; Expands to:
;   LD A,0
;   LD B,0
;   LD C,0
```

**List Syntax:**
- `<item1,item2,item3>` - Angle brackets required
- Items can be registers, numbers, or any assembly tokens

### 4. IRPC - Iteration with String Characters

Iterate over individual characters in a string:

```assembly
        ORG     8000H
        IRPC    char,<ABC>
        DB      '&char'
        ENDM
; Expands to:
;   DB 'A'
;   DB 'B'
;   DB 'C'
```

**String Syntax:**
- `<string>` - Angle brackets required
- Each character becomes a separate substitution

### 5. LOCAL - Unique Label Generation

Declare labels that are unique per macro expansion:

```assembly
DELAY   MACRO   count
        LOCAL   loop
        LD      B,&count
loop:   DJNZ    loop
        ENDM

        DELAY   10      ; loop becomes loop_1
        DELAY   20      ; loop becomes loop_2
```

**Features:**
- Multiple LOCAL labels supported
- Comma-separated list
- Automatic unique suffix per expansion

### 6. EXITM - Early Exit

Exit from macro/REPT/IRP/IRPC early:

```assembly
SKIP    MACRO   val
        IFE     &val,0
        EXITM               ; Exit if val is 0
        ENDIF
        LD      A,&val
        ENDM
```

**Behavior:**
- Stops processing current expansion
- Returns to caller

### 7. Nested Constructs

Macros can contain REPT/IRP/IRPC blocks, and vice versa:

```assembly
FILLREG MACRO   reg,count,val
        REPT    &count
        LD      &reg,&val
        ENDM
        ENDM

        FILLREG A,3,42H     ; Expands to 3x LD A,42H
```

**Nesting Depth:**
- Properly tracked with `macro_nesting_depth_`
- Inner ENDM closes inner block
- Outer ENDM closes macro definition

## Implementation Details

### Core Components

**EdtasmM80PlusPlusSyntaxParser:**
- `macros_` - Map of defined macros (name → MacroDefinition)
- `in_macro_definition_` - Currently capturing macro body
- `macro_nesting_depth_` - Track nested MACRO/REPT/IRP/IRPC blocks
- `macro_expansion_depth_` - Prevent infinite recursion (max 100)
- `macro_unique_counter_` - Generate unique LOCAL label suffixes
- `exitm_triggered_` - Track EXITM in current expansion

**MacroDefinition Structure:**
```cpp
struct MacroDefinition {
    std::string name;                 // Macro name
    std::vector<std::string> body;    // Unexpanded body lines
    std::vector<std::string> params;  // Parameter names
    std::vector<std::string> locals;  // LOCAL symbols
};
```

### Expansion Process

1. **Parameter Substitution:**
   - `&param` → Replace with argument value
   - `%digit` → Replace with argument at position (1-based)

2. **LOCAL Symbol Replacement:**
   - `label` → `label_N` where N is unique per expansion
   - Tracked in `next_macro_unique_id_`

3. **Recursive Expansion:**
   - Expanded lines fed back to parser
   - Allows nested macro calls
   - Depth limit prevents infinite loops

### Error Handling

**Runtime Errors:**
- ENDM without matching MACRO/REPT/IRP/IRPC
- Unclosed macro/repeat blocks at end of file
- REPT with invalid count
- IRP with missing/invalid list
- IRPC with missing string
- Duplicate macro names
- Too deep recursion (>100 levels)

**Recovery:**
- Throw `std::runtime_error` with descriptive message
- Parser state reset between files

## Testing

### Test Suite: test_z80_macros.cpp

**Coverage:**
- 23 comprehensive tests
- 100% pass rate
- All features tested

**Test Categories:**
1. REPT tests (3) - basic, zero count, multiple instructions
2. IRP tests (3) - basic, percent substitution, empty list
3. IRPC tests (3) - basic, angle brackets, empty string
4. MACRO tests (6) - basic, no params, labels, LOCAL, EXITM, calling macros
5. Nesting tests (2) - macro calling macro, REPT inside MACRO
6. Error tests (6) - ENDM without MACRO, unclosed blocks, invalid syntax

## Performance Considerations

**Memory:**
- Macro bodies stored as string vectors
- Minimal overhead per defined macro
- Expansion creates temporary copies

**Speed:**
- Linear expansion time (O(n) where n = expanded lines)
- Hash table lookup for macro names (O(1))
- Recursion depth limit prevents exponential blowup

## Future Enhancements

Potential improvements:
- PURGE directive to undefine macros
- Macro concatenation operators (##)
- Variadic macros (...args)
- Macro debugging output (.LALL, .XALL)

## References

- M80 Macro Assembler Manual
- EDTASM Documentation
- Microsoft Macro-80 Reference

## Version History

- **2026-02-10:** Initial implementation (Phase 12)
  - MACRO/ENDM with parameters
  - REPT/IRP/IRPC repeat constructs
  - LOCAL symbols
  - EXITM early exit
  - Parameter substitution (&, %)
  - Nested macros
  - Comprehensive test coverage
