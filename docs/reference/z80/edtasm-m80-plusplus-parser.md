# EDTASM-M80++ Parser Reference

**Document:** EDTASM-M80++ Syntax Parser for Z80
**Version:** 1.0
**Date:** 2026-02-10
**Status:** Complete

---

## Overview

The EDTASM-M80++ parser provides a universal Z80 assembly syntax that combines features from multiple popular Z80 assemblers:

- **EDTASM** - TRS-80 Editor Assembler syntax
- **M80** - Microsoft's Z80 Macro Assembler
- **ZMAC** - Popular Unix/Linux Z80 assembler
- **Z80ASM** - Various Z80 assembler variants

This "plus plus" parser is designed to accept code from any of these assemblers with minimal modifications, making it ideal for porting legacy Z80 code or working with mixed sources.

---

## Usage

### Command Line

```bash
xasm++ --cpu z80 --syntax edtasm_m80_plusplus input.asm -o output.bin
```

**Options:**
- `--cpu z80` - Selects Z80 CPU target
- `--syntax edtasm_m80_plusplus` - Selects this universal parser
- `-o output.bin` - Specifies output file

### In Code

```cpp
#include "xasm++/assembler.h"
#include "xasm++/syntax/edtasm_m80_plusplus_syntax.h"
#include "xasm++/cpu/cpu_z80.h"

// Create assembler
xasm::Assembler assembler;
assembler.SetCPU(std::make_unique<xasm::Z80CPU>());
assembler.SetSyntax(std::make_unique<xasm::EdtasmM80PlusPlusSyntaxParser>());

// Assemble source
assembler.AssembleFile("input.asm");
assembler.WriteOutput("output.bin");
```

---

## Number Formats

The parser supports multiple number format conventions:

### Hexadecimal

```assembly
; Suffix H (must start with digit)
LD    A,0FFH              ; $FF
LD    BC,1234H            ; $1234
LD    (8000H),A           ; Address $8000

; Prefix $ (ZMAC style)
LD    A,$FF               ; $FF
LD    BC,$1234            ; $1234
LD    ($8000),A           ; Address $8000

; Prefix 0x (C style)
LD    A,0xFF              ; $FF
LD    BC,0x1234           ; $1234
```

### Decimal

```assembly
; Plain numbers
LD    A,255               ; Decimal 255
LD    B,42                ; Decimal 42
LD    BC,4660             ; Decimal 4660
```

### Octal

```assembly
; Suffix O or Q
LD    A,377O              ; Octal 377 = $FF
LD    A,377Q              ; Octal 377 = $FF
```

### Binary

```assembly
; Suffix B (must start with digit)
LD    A,11111111B         ; Binary
LD    A,10101010B         ; Binary

; Prefix % (ZMAC style)
LD    A,%11111111         ; Binary
LD    A,%10101010         ; Binary
```

---

## Directives

### Origin and Location

#### ORG - Set Origin

```assembly
        ORG   $8000           ; Set program origin to $8000

START:  LD    A,$42
        RET
```

**Purpose:** Sets the assembly address for subsequent code.

---

### Data Definition

#### DB / DEFB / BYTE - Define Bytes

```assembly
; Single bytes
MESSAGE DB    'Hello',13,10,0    ; String with CR, LF, null
BUFFER  DB    0,0,0,0,0          ; Five zero bytes
FLAGS   DB    $FF,$00,$80        ; Hex values

; Expressions
TABLE   DB    START/256          ; High byte of address
        DB    START&255          ; Low byte of address
```

**Aliases:** `DB`, `DEFB`, `BYTE`

**Purpose:** Define byte data.

---

#### DW / DEFW / WORD - Define Words

```assembly
; 16-bit values (little-endian)
ADDR    DW    $8000              ; Stored as $00 $80
COUNT   DW    1000               ; Stored as $E8 $03
TABLE   DW    START,END          ; Two addresses

; Forward references supported
PTRS    DW    FUTURE_LABEL       ; Resolved in second pass
```

**Aliases:** `DW`, `DEFW`, `WORD`

**Purpose:** Define 16-bit word data in little-endian format.

**Note:** Always stored as expressions to support forward references.

---

#### DS / DEFS / DSPACE / BLOCK - Define Space

```assembly
; Reserve memory
BUFFER  DS    256                ; 256 bytes of space
STACK   DS    128                ; 128 byte stack
TEMP    DEFS  10                 ; 10 bytes
```

**Aliases:** `DS`, `DEFS`, `DSPACE`, `BLOCK`

**Purpose:** Reserve uninitialized memory space.

---

#### DM / DEFM / TEXT / ASCII - Define Message

```assembly
; Text strings
MSG1    DM    'Hello, World!'
MSG2    DEFM  "Error: "
PROMPT  TEXT  'Enter name: '
```

**Aliases:** `DM`, `DEFM`, `TEXT`, `ASCII`

**Purpose:** Define text message (string without implicit terminator).

---

### Symbols and Constants

#### EQU - Define Constant

```assembly
; Constants
BDOS    EQU   5                  ; CP/M BDOS entry point
CONOUT  EQU   2                  ; Console output function
BUFFER  EQU   $8000              ; Buffer address

; Use in code
        LD    C,CONOUT
        CALL  BDOS
```

**Purpose:** Define a constant symbol.

**Note:** Symbol cannot be redefined.

---

#### = (Assignment Operator)

```assembly
; Variable symbols
COUNTER = 0                      ; Initial value
COUNTER = COUNTER+1              ; Can reassign
```

**Purpose:** Define a reassignable symbol.

**Note:** Unlike `EQU`, symbols defined with `=` can be changed.

---

### Conditional Assembly

#### IF / ELSE / ENDIF

```assembly
; Conditional compilation
DEBUG   EQU   1

        IF    DEBUG
        CALL  PRINT_DEBUG
        ENDIF

; With else
        IF    TARGET=1
        CALL  ROUTINE_A
        ELSE
        CALL  ROUTINE_B
        ENDIF
```

**Purpose:** Conditionally include/exclude code blocks.

**Conditions:**
- Non-zero = true (include IF block)
- Zero = false (include ELSE block)

---

#### IFN / IFNE - If Not Equal

```assembly
; If not equal (if non-zero)
        IFN   DEBUG
        ; Include if DEBUG is non-zero
        ENDIF
```

**Purpose:** Inverted condition (true if non-zero).

---

#### IFE / IFEQ - If Equal

```assembly
; If equal (if zero)
        IFE   DEBUG
        ; Include if DEBUG is zero
        ENDIF
```

**Purpose:** True if expression equals zero.

---

#### IFB / IFDEF - If Defined

```assembly
; Check if symbol defined
        IFDEF FEATURE_X
        ; Include if FEATURE_X is defined
        ENDIF
```

**Purpose:** Check if symbol has been defined.

---

#### IFNB / IFNDEF - If Not Defined

```assembly
; Check if symbol not defined
        IFNDEF FEATURE_X
        ; Include if FEATURE_X is not defined
        ENDIF
```

**Purpose:** Check if symbol has NOT been defined.

---

### Listing Control

#### TITLE - Set Listing Title

```assembly
        TITLE 'Z80 Bootstrap ROM v1.0'
```

**Purpose:** Sets title for assembly listing output.

---

#### PAGE - Page Break

```assembly
        PAGE                     ; Start new page in listing
```

**Purpose:** Insert page break in listing output.

---

#### LIST / NOLIST - Control Listing

```assembly
        NOLIST                   ; Disable listing output
        ; Include system equates (don't clutter listing)
        INCLUDE 'system.inc'
        LIST                     ; Re-enable listing
```

**Purpose:** Enable/disable listing output for sections of code.

---

#### LALL / XALL - List All

```assembly
        LALL                     ; List all macro expansions
```

**Purpose:** Show all macro expansions in listing.

---

#### SALL / XALL - Suppress All

```assembly
        SALL                     ; Suppress macro expansions
```

**Purpose:** Hide macro expansions in listing.

---

### Linkage and Scope

#### PUBLIC / ENTRY - Export Symbols

```assembly
; Make symbols visible to linker
        PUBLIC START,ERROR_HANDLER
        PUBLIC DATA_TABLE

START:  JP    MAIN
ERROR_HANDLER:
        RET
```

**Purpose:** Export symbols for linking with other modules.

---

#### EXTERNAL / EXTRN - Import Symbols

```assembly
; Reference symbols from other modules
        EXTERNAL PRINT_STRING,BDOS_CALL
        EXTRN    BUFFER,STACK_TOP

        CALL  PRINT_STRING       ; Call external routine
        LD    HL,BUFFER          ; Reference external data
```

**Purpose:** Declare symbols defined in other modules.

---

### Special Directives

#### END - End of Assembly

```assembly
        END   START              ; End assembly, entry at START
```

**Purpose:** Mark end of source file, optionally specify entry point.

---

#### $INCLUDE / INCLUDE - Include File

```assembly
; Include other source files
        $INCLUDE 'hardware.inc'
        INCLUDE  "system.inc"
```

**Purpose:** Include another source file at this location.

---

#### ASEG / CSEG / DSEG - Segment Control

```assembly
        ASEG                     ; Absolute segment
        ORG   $8000
CODE:   ; code here

        DSEG                     ; Data segment
DATA:   DS    256

        CSEG                     ; Code segment (relocatable)
        ; relocatable code
```

**Purpose:** Control segment types (absolute/code/data).

---

#### PHASE / DEPHASE - Phase Control

```assembly
; Assemble code for execution at different address
        PHASE $9000              ; Code runs at $9000
ROUTINE:
        LD    A,(HL)
        RET
        DEPHASE                  ; Return to normal addressing

; Copy ROUTINE to $9000 and call it
        LD    HL,ROUTINE
        LD    DE,$9000
        LD    BC,ROUTINE_END-ROUTINE
        LDIR
        CALL  $9000
```

**Purpose:** Assemble code that will be relocated at runtime.

---

## String and Character Support

### String Literals

```assembly
; Double quotes
MESSAGE DB    "Hello, World!",0

; Single quotes
PROMPT  DB    'Enter name: ',0

; Mixed quotes
DATA    DB    "It's working!",0
        DB    'Say "Hello"',0
```

**Supported delimiters:** Double quotes (`"`) and single quotes (`'`)

---

### Escape Sequences

```assembly
; Standard C-style escapes
CRLF    DB    "Line 1\r\nLine 2\r\n",0
TAB     DB    "Col1\tCol2\tCol3",0
QUOTE   DB    "She said \"Hi\"",0
SLASH   DB    "Path: C:\\DIR\\FILE",0
```

**Supported escapes:**
- `\r` - Carriage return ($0D)
- `\n` - Line feed ($0A)
- `\t` - Tab ($09)
- `\\` - Backslash
- `\"` - Double quote
- `\'` - Single quote

---

## Expressions

The parser supports arithmetic expressions in operands:

```assembly
; Arithmetic operators
MIDPOINT EQU  (START+END)/2      ; Average of two addresses
SIZE     EQU  END-START          ; Calculate size
DOUBLED  EQU  VALUE*2            ; Multiplication

; Bitwise operators
HIMASK   EQU  VALUE&$FF00        ; High byte mask
LOMASK   EQU  VALUE&$00FF        ; Low byte mask
COMBINED EQU  HIGH|LOW           ; Bitwise OR

; In instructions
        LD    A,(BUFFER+5)       ; Address + offset
        LD    BC,TABLE_SIZE*2    ; Expression as operand
        ADD   A,VALUE/256        ; High byte
```

**Supported operators:**
- `+` - Addition
- `-` - Subtraction
- `*` - Multiplication
- `/` - Integer division
- `&` - Bitwise AND
- `|` - Bitwise OR
- `^` - Bitwise XOR
- `~` - Bitwise NOT
- `<<` - Left shift
- `>>` - Right shift

---

## Labels

### Global Labels

```assembly
; Standard labels (start at column 1 or with no leading spaces)
START:  LD    A,0
        RET

LOOP    LD    B,10              ; Colon optional
LOOP1:  DJNZ  LOOP1
        RET
```

---

### Local Labels

```assembly
; ZMAC-style (digits only)
OUTER:  LD    B,10
1:      PUSH  BC                 ; Local label
        CALL  INNER
        POP   BC
        DJNZ  1b                 ; Reference backwards (1b)
        JR    2f                 ; Reference forwards (2f)
2:      RET

; Z80ASM-style (dot prefix)
FUNC:   LD    A,0
.loop:  INC   A
        CP    10
        JR    NZ,.loop           ; Local to FUNC
        RET
```

**Local label scopes:**
- ZMAC: `1b` (backward), `1f` (forward)
- Z80ASM: `.name` (scoped to previous global label)

---

## Comments

```assembly
; Semicolon comments (entire line or end of line)
        LD    A,42               ; Load answer

; Some assemblers support asterisk in column 1
*       This is a comment line

; C-style comments also supported
        LD    A,0                // Load zero
        /* Multi-line comment
           spanning several lines
           is supported */
        RET
```

---

## Forward References

The parser fully supports forward references:

```assembly
; Reference before definition
        LD    HL,TABLE           ; TABLE defined later
        CALL  SUBROUTINE         ; SUBROUTINE defined later
        JP    FORWARD_LABEL      ; FORWARD_LABEL defined later

; Data with forward references
POINTERS:
        DW    ROUTINE1           ; All resolved in second pass
        DW    ROUTINE2
        DW    ROUTINE3

; Later in source
TABLE:  DB    1,2,3,4,5
SUBROUTINE:
        RET
FORWARD_LABEL:
        RET
ROUTINE1:
        RET
ROUTINE2:
        RET
ROUTINE3:
        RET
```

**Implementation:** DW directive always stores as expressions to enable proper forward reference resolution.

---

## Performance

### Benchmarks

Based on integration tests:

**Large File Handling:**
- 10,000 instruction program: < 100ms
- 50,000 byte data block: < 150ms

**Rapid Parsing:**
- 1,000 simple instructions: < 10ms (tested in RapidParsingPerformance)

**Deep Nesting:**
- 20-level conditional nesting: < 1ms

### Optimization Strategies

The parser is optimized for:
1. Single-pass directive handling where possible
2. Efficient string processing (no repeated allocations)
3. Fast number format detection (suffix-based)
4. Minimal memory allocations in hot paths

---

## Compatibility Notes

### M80 Compatibility

**Fully Compatible:**
- ✅ Hex suffix (0FFH)
- ✅ PUBLIC/EXTRN directives
- ✅ All data directives (DB, DW, DS)
- ✅ Conditional assembly (IF/ENDIF)
- ✅ TITLE, PAGE directives

**Limitations:**
- ❌ Macros not yet implemented
- ❌ REPT not yet implemented
- ❌ IRP/IRPC not yet implemented

---

### ZMAC Compatibility

**Fully Compatible:**
- ✅ $ prefix for hex
- ✅ % prefix for binary
- ✅ Local labels (numeric with b/f suffix)
- ✅ Dollar-hex ($FF)
- ✅ All basic directives

**Limitations:**
- ❌ Macros not yet implemented
- ❌ Some ZMAC-specific pseudo-ops

---

### Z80ASM Compatibility

**Fully Compatible:**
- ✅ Dot-prefix local labels
- ✅ 0x hex prefix
- ✅ Standard directives
- ✅ All addressing modes

**Limitations:**
- ❌ Macros not yet implemented

---

## Known Limitations

1. **Macros Not Implemented**
   - MACRO/ENDM directives not yet supported
   - REPT not yet supported
   - IRP/IRPC not yet supported
   - **Workaround:** Use includes or hand-expand macros

2. **Listing Output**
   - Listing control directives recognized but listing not generated
   - TITLE/PAGE/LIST/NOLIST/LALL/SALL parsed but no output
   - **Workaround:** Use external tools for listing generation

3. **Complex Expressions**
   - Some advanced expression features may not work
   - Operator precedence follows C conventions
   - **Workaround:** Use parentheses for complex expressions

4. **Object File Formats**
   - Currently outputs binary only
   - No .REL (Microsoft relocatable) format
   - No .O (Unix object) format
   - **Workaround:** Use binary output and link manually

---

## Examples

### Example 1: Hello World (CP/M)

```assembly
        TITLE 'CP/M Hello World'
        
; CP/M system equates
BDOS    EQU   5                  ; BDOS entry point
CONOUT  EQU   2                  ; Console output function
WRMSG   EQU   9                  ; Write message function

        ORG   $100               ; CP/M TPA start

START:  LD    DE,MESSAGE         ; Point to message
        LD    C,WRMSG            ; Write string function
        CALL  BDOS               ; Call BDOS
        RET                      ; Return to CP/M

MESSAGE:
        DB    'Hello, World!',13,10,'$'

        END   START
```

---

### Example 2: Interrupt Handler (TRS-80)

```assembly
        TITLE 'TRS-80 Interrupt Handler'
        
        ORG   $8000              ; User RAM

; Interrupt mode 1 handler (RST 38H)
        ORG   $38
        JP    ISR                ; Jump to handler

ISR:    PUSH  AF                 ; Save registers
        PUSH  BC
        PUSH  DE
        PUSH  HL
        
        ; Handle interrupt
        LD    A,(TICK_COUNT)
        INC   A
        LD    (TICK_COUNT),A
        
        POP   HL                 ; Restore registers
        POP   DE
        POP   BC
        POP   AF
        EI                       ; Re-enable interrupts
        RETI                     ; Return from interrupt

TICK_COUNT:
        DB    0

        END
```

---

### Example 3: Data Tables

```assembly
        TITLE 'Lookup Tables'
        
        ORG   $8000

; Sine table (0-90 degrees, 256 scale)
SINE_TABLE:
        DB    0,4,9,13,18,22,27,31
        DB    36,40,44,49,53,57,62,66
        ; ... (abbreviated)

; Jump table
DISPATCH:
        DW    CMD_HELP           ; Command 0
        DW    CMD_LIST           ; Command 1
        DW    CMD_RUN            ; Command 2
        DW    CMD_EXIT           ; Command 3

; Use jump table
EXECUTE:
        LD    A,(COMMAND)        ; Get command number
        ADD   A,A                ; Multiply by 2 (word size)
        LD    HL,DISPATCH
        ADD   A,L
        LD    L,A
        JR    NC,1f
        INC   H
1:      LD    E,(HL)             ; Load handler address
        INC   HL
        LD    D,(HL)
        EX    DE,HL
        JP    (HL)               ; Jump to handler

CMD_HELP:
        ; Help command implementation
        RET

CMD_LIST:
        ; List command implementation
        RET

CMD_RUN:
        ; Run command implementation
        RET

CMD_EXIT:
        ; Exit command implementation
        RET

COMMAND:
        DB    0

        END
```

---

## Troubleshooting

### Common Errors

**"Unexpected character"**
- Check for invalid number format (e.g., hex without leading digit: `FFH` should be `0FFH`)
- Verify string delimiters match (opened with `"` but closed with `'`)

**"Undefined symbol"**
- Symbol not defined before use and not forward-referenceable
- Check spelling and case (symbols are case-sensitive)
- Ensure symbol is defined in included files

**"Invalid expression"**
- Expression syntax error
- Division by zero
- Shift count out of range

**"Phase error"**
- Symbol value changed between passes
- Usually caused by self-referential expressions
- Check for symbols defined differently in each pass

---

## Further Reading

- [Z80 Architecture Reference](../cpu/z80-architecture.md) - Complete Z80 CPU documentation
- [Z80 Instruction Set](../cpu/z80-instructions.md) - Detailed instruction reference
- [xasm++ Developer Guide](../../development/developer-guide.md) - Parser development info

---

**Document Version:** 1.0
**Last Updated:** 2026-02-10
**Author:** xasm++ Team
**Status:** Complete
