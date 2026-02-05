---
sidebar_position: 1
---

# FLEX ASM09 Syntax Reference

**Target System:** FLEX Operating System (6809)
**CPU Support:** Motorola 6809, 6800, 6801
**Source:** ASM09 6809 Optimizing Assembler Version 3.2
**Purpose:** Complete syntax reference for xasm++ FLEX compatibility

---

## Overview

FLEX ASM09 is the assembler for the FLEX Disk Operating System (FLEX DOS) developed by Technical Systems Consultants (TSC). It's a sophisticated 2-pass assembler with macro support, conditional assembly, and listing control.

### Key Features

- **2-pass assembly** with symbol resolution
- **Macro preprocessor** with parameters
- **Conditional assembly** directives
- **Listing control** with pagination and titles
- **Library support** for modular development
- **Expression evaluation** with full operator set
- **Backward compatible** with 6800/6801 code

### Differences from EDTASM

| Feature | FLEX ASM | EDTASM |
|---------|----------|---------|
| Macros | ✅ Full support | ❌ No macros |
| Conditional Assembly | ✅ IF/ENDIF | ❌ Limited |
| Listing Control | ✅ NAM, TTL, PAG, SPC | ❌ Basic |
| Libraries | ✅ LIB directive | ❌ No libraries |
| Comments | `*` or `+` | `;` or `*` |

---

## Source Line Format

FLEX ASM uses a multi-field format:

```
[LABEL] [OPERATION] [OPERAND] [;COMMENT]
```

### Label Field

- **Position:** Column 1
- **Format:** 1-8 alphanumeric characters, starting with letter
- **Terminator:** Space or tab (no colon needed)
- **Special:** `*` or `+` in column 1 indicates comment line

```asm
START    LDA    #$42      ; Label in column 1
LOOP     INX              ; Another label
         STA    BUFFER,X  ; No label
```

### Operation Field

Mnemonics, directives, or macro calls:
- **Mnemonics:** 6809 instruction set
- **Directives:** ORG, EQU, FCB, etc.
- **Macros:** User-defined macro names

### Operand Field

Expressions, addresses, or directive parameters.

### Comment Field

- Preceded by semicolon `;`
- Continues to end of line
- Also: Entire line comments start with `*` or `+` in column 1

---

## Directives

### Program Structure

#### ORG - Set Origin

```asm
ORG    $8000        ; Set assembly address
```

Sets the current assembly address.

#### END - End of Source

```asm
END    START        ; End with entry point
END                 ; End without entry point
```

Marks end of source file. Optional parameter specifies program start address.

#### SETDP - Set Direct Page

```asm
SETDP  $00          ; Direct page at $0000
SETDP  $10          ; Direct page at $1000
```

Informs assembler of direct page register value for optimization.

---

### Symbol Definition

#### EQU - Equate (Constant)

```asm
SCREEN  EQU   $0400      ; Define constant
ROWS    EQU   24         ; Numeric constant
```

Defines a constant symbol (cannot be redefined).

#### SET - Set Variable

```asm
COUNTER SET   0          ; Define variable
COUNTER SET   COUNTER+1  ; Can be redefined
```

Defines a variable symbol (can be reassigned).

---

### Data Definition

#### FCB - Form Constant Byte

```asm
        FCB   $01,$02,$03     ; Bytes
        FCB   'A','B','C'     ; Characters
        FCB   <ADDR           ; Low byte
        FCB   >ADDR           ; High byte
```

Defines byte-sized data.

#### FDB - Form Double Byte (Word)

```asm
        FDB   $1234           ; 16-bit word
        FDB   ADDRESS         ; Address value
```

Defines 16-bit word data (big-endian).

#### FCC - Form Constant Character

```asm
        FCC   /Hello World/   ; Slash delimiters
        FCC   'String'        ; Quote delimiters
        FCC   "String"        ; Double quote delimiters
```

Defines character string data. Delimiter is first non-space character after FCC.

#### RMB - Reserve Memory Bytes

```asm
BUFFER  RMB   256           ; Reserve 256 bytes
WORK    RMB   16            ; Reserve 16 bytes
```

Reserves memory space without initialization.

---

### Listing Control

#### NAM or TTL - Name/Title

```asm
        NAM   Program Title
        TTL   Module Name
```

Sets program title for listing header.

#### STTL - Subtitle

```asm
        STTL  Section: Main Loop
```

Sets subtitle for current page.

#### PAG - Page Break

```asm
        PAG                   ; Force new page
```

Forces new page in listing.

#### SPC - Space Lines

```asm
        SPC   2               ; Insert 2 blank lines
        SPC   5               ; Insert 5 blank lines
```

Inserts blank lines in listing.

---

### Macro Directives

#### MACRO - Begin Macro Definition

```asm
MOVB    MACRO   SRC,DEST     ; Define macro with parameters
        LDA     SRC
        STA     DEST
        ENDM
```

Begins macro definition. Parameters are positional.

#### ENDM - End Macro

```asm
        ENDM                  ; End macro definition
```

Terminates macro definition.

#### Macro Invocation

```asm
        MOVB    $80,$90      ; Call macro
```

Invokes macro with actual parameters.

---

### Conditional Assembly

#### IFC - If Condition

```asm
        IFC     DEBUG        ; If DEBUG defined
        JSR     TRACE
        ENDC
```

Conditional assembly based on symbol definition.

#### ENDC - End Conditional

```asm
        ENDC                 ; End conditional block
```

Terminates conditional assembly block.

---

### Advanced Directives

#### LIB - Include Library

```asm
        LIB     STDLIB       ; Include library file
```

Includes external source file during assembly.

#### REG - Register List

```asm
        REG     A,B,X,Y      ; Define register list for PSHS/PULS
```

Defines register lists for stack operations.

#### OPT - Assembler Options

```asm
        OPT     C,L,P        ; Set options
```

Controls assembler behavior:
- `C` - Enable cycle counting
- `L` - Generate listing
- `P` - Enable pagination

#### ERR - Generate Error

```asm
        ERR     Invalid configuration
```

Forces assembler error with custom message.

#### RPT - Repeat Block

```asm
        RPT     10           ; Repeat 10 times
        NOP
        ENDR
```

Repeats following code block N times.

---

## Expressions

### Operators

FLEX ASM supports standard operators with C-like precedence:

| Operator | Description | Precedence |
|----------|-------------|------------|
| `+` | Addition | Low |
| `-` | Subtraction | Low |
| `*` | Multiplication | Medium |
| `/` | Division | Medium |
| `%` | Modulo | Medium |
| `&` | Bitwise AND | Low |
| `\|` | Bitwise OR | Low |
| `^` | Bitwise XOR | Low |
| `<<` | Shift left | Medium |
| `>>` | Shift right | Medium |
| `<` | Low byte | High |
| `>` | High byte | High |

### Expression Examples

```asm
        LDA     #$FF&$0F      ; Bitwise AND
        LDX     #TABLE+16     ; Addition
        LDA     #>ADDRESS     ; High byte
        LDB     #<ADDRESS     ; Low byte
```

---

## Number Formats

FLEX ASM supports multiple number formats:

| Format | Prefix/Suffix | Example |
|--------|---------------|---------|
| Decimal | None | `123` |
| Hexadecimal | `$` prefix | `$1A2F` |
| Binary | `%` prefix | `%10110101` |
| Octal | `@` prefix | `@377` |
| Character | Single quotes | `'A'` |

---

## Addressing Modes (6809)

FLEX ASM uses standard Motorola syntax:

```asm
; Inherent
        NOP

; Immediate
        LDA     #$42
        LDX     #$1234

; Direct (Zero Page)
        LDA     $80
        LDA     <ADDRESS      ; Force direct

; Extended (Absolute)
        LDA     $1234
        LDA     >ADDRESS      ; Force extended

; Indexed
        LDA     ,X            ; No offset
        LDA     5,X           ; 5-bit offset
        LDA     $80,X         ; 8-bit offset
        LDA     $1234,X       ; 16-bit offset
        LDA     A,X           ; Accumulator offset
        LDA     D,X           ; D accumulator offset
        LDA     [,X]          ; Indirect
        LDA     [,X++]        ; Auto-increment

; Relative (Branches)
        BNE     LOOP
        LBRA    FARAWAY       ; Long branch
```

---

## Comment Styles

FLEX ASM supports multiple comment styles:

```asm
* Full line comment (column 1)
+ Full line comment with page break control

        LDA     #$42      ; End of line comment

; Standalone comment line (any column)
```

**Comment Types:**
- `*` in column 1 - Regular comment, page break if <14 lines remain
- `+` in column 1 - Forces new page before comment
- `;` - End-of-line comment or standalone comment

---

## Macro Features

### Parameter Substitution

```asm
MOVE    MACRO   SRC,DEST
        LDA     SRC          ; Parameter 1
        STA     DEST         ; Parameter 2
        ENDM

        MOVE    $80,$90      ; Invocation
```

### Local Labels in Macros

```asm
WAIT    MACRO   COUNT
.LOOP   DEC     COUNT
        BNE     .LOOP
        ENDM
```

Local labels (starting with `.`) are unique per macro invocation.

---

## Example Programs

### Simple Program

```asm
        NAM     Hello World
        TTL     FLEX ASM Example

SCREEN  EQU     $0400

        ORG     $8000

START   LDX     #SCREEN
        LDY     #MESSAGE
LOOP    LDA     ,Y+
        BEQ     DONE
        STA     ,X+
        BRA     LOOP

DONE    RTS

MESSAGE FCC     /HELLO WORLD/
        FCB     0

        END     START
```

### With Macros

```asm
        NAM     Macro Example

; Define macro
PUSH2   MACRO   R1,R2
        PSHS    R1,R2
        ENDM

POP2    MACRO   R1,R2
        PULS    R1,R2
        ENDM

        ORG     $8000

START   LDX     #$1234
        LDY     #$5678

        PUSH2   X,Y          ; Save registers
        JSR     SUBRTN
        POP2    X,Y          ; Restore registers

        RTS

SUBRTN  ; Subroutine code
        RTS

        END     START
```

---

## Compatibility Notes

### FLEX vs EDTASM

**Compatible:**
- Basic directives (ORG, EQU, FCB, FDB, FCC, RMB, END)
- 6809 instruction mnemonics
- Expression syntax

**FLEX Additions:**
- Macro support (MACRO/ENDM)
- Conditional assembly (IFC/ENDC)
- Library includes (LIB)
- Advanced listing control
- Repeat blocks (RPT/ENDR)

**Migration:**
- EDTASM programs generally work in FLEX ASM
- FLEX programs may use features not in EDTASM

---

## References

- **Original Manual:** ASM09 6809 Optimizing Assembler Version 3.2
- **Publisher:** Southwest Technical Products Corporation
- **Copyright:** 1980 Southwest Technical Products Corporation
- **Availability:** `docs/reference/flex/ASM09-6809-assembler.pdf`

---

## Implementation Notes

For xasm++ developers implementing FLEX ASM support:

### Code Reuse Opportunities

1. **Extend MotorolaBaseSyntax** (~60% code reuse)
   - Basic directive handling (ORG, EQU, FCB, etc.)
   - Expression evaluation
   - Addressing mode parsing

2. **New Components Needed**
   - Macro preprocessor
   - Conditional assembly handler
   - Listing control system
   - Library inclusion mechanism

### Estimated Implementation

- **Phase 1:** Basic directives (10-15 hours)
- **Phase 2:** Macro processor (15-20 hours)
- **Phase 3:** Conditional assembly (8-10 hours)
- **Phase 4:** Advanced directives (10-15 hours)
- **Total:** 45-60 hours

### Testing Strategy

- Unit tests for each directive
- Macro expansion tests
- Conditional assembly tests
- Integration with existing 6809 CPU
- Real FLEX source code compatibility
