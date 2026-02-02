# EDTASM+ Assembler Syntax Reference

**Target Platform:** TRS-80 Color Computer (CoCo)
**Assembler:** Disk EDTASM+ (Tandy/Microsoft)
**CPU Support:** Motorola 6809, HD6309

## Overview

EDTASM+ is the standard assembler for TRS-80 Color Computer development. It features a straightforward syntax with powerful macro capabilities and conditional assembly.

**Key Characteristics:**
- Case-insensitive (traditional) or case-sensitive (modern implementations)
- Column-oriented source format
- Flexible delimiters for string constants
- Rich set of assembler directives
- Macro system with parameters and local labels

## Source Line Format

```
[label] [opcode] [operand] [;comment]
```

**Field Rules:**
1. **Label** (Column 1+): Optional, up to 6 characters (original), longer in modern variants
2. **Opcode** (after label or whitespace): Instruction mnemonic or directive
3. **Operand**: Address, immediate value, or expression
4. **Comment**: Preceded by semicolon (`;`) or asterisk (`*`) in column 1

## Comments

```asm
* Asterisk in column 1 = full line comment
        LDA     #10     ; Semicolon comment anywhere
```

## Assembler Directives

### ORG - Set Origin Address

Sets the assembly address for subsequent code.

```asm
        ORG     $0400           ; Start assembly at $0400
START   LDA     #0
```

**Source:** [FLEX 6809 Assembler](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)

### END - End Assembly

Marks the end of the source file.

```asm
        END     START           ; End, entry point at START
```

### EQU - Equate Symbol

Defines a constant symbol (cannot be redefined).

```asm
BUFSIZE EQU     256             ; Define constant
MAXVAL  EQU     BUFSIZE*2       ; Expression
```

**Source:** [FLEX 6809 Assembler Directives](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)

### SET - Set Variable

Defines a variable symbol (can be redefined).

```asm
COUNT   SET     0               ; Initial value
COUNT   SET     COUNT+1         ; Can change
```

### FCB - Form Constant Byte

Defines 8-bit data values.

```asm
        FCB     1,2,3,4,5       ; Multiple bytes
        FCB     $FF             ; Hex value
        FCB     %11110000       ; Binary value
        FCB     'A'             ; ASCII character
        FCB     255             ; Decimal value
```

**Source:** [FLEX 6809 Directives](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)

### FDB - Form Double Byte

Defines 16-bit word values (big-endian).

```asm
        FDB     $1234           ; Single word
        FDB     $1000,$2000     ; Multiple words
        FDB     START           ; Address of label
        FDB     START+10        ; Expression
```

### FCC - Form Constant Characters

Defines ASCII string with flexible delimiter.

```asm
        FCC     /Hello, World!/ ; Using slash delimiter
        FCC     "Double quotes" ; Using quotes
        FCC     'Single quotes' ; Using apostrophe
        FCC     |Any char|      ; Any non-alphanumeric
```

**Note:** First non-whitespace character after FCC is the delimiter.

**Source:** [FLEX 6809 Directives](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)

### RMB - Reserve Memory Bytes

Reserves uninitialized space.

```asm
BUFFER  RMB     256             ; 256 bytes of space
VARS    RMB     10              ; 10 bytes
STACK   RMB     $100            ; 256 bytes (hex)
```

**Source:** [FLEX 6809 Directives](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)

### SETDP - Set Direct Page

Informs assembler of Direct Page register value for optimization.

```asm
        SETDP   $10             ; Direct page is $10xx
        LDA     $50             ; Assembles as direct mode (DP:$50)
```

### NAM - Name

Sets program name (for listing).

```asm
        NAM     MYPROG
```

### TTL/TITLE - Title

Sets listing title.

```asm
        TTL     My Program V1.0
```

### PAGE - New Page

Forces page break in listing.

```asm
        PAGE
```

### SPC - Space

Inserts blank lines in listing.

```asm
        SPC     3               ; 3 blank lines
```

### OPT - Options

Controls listing options.

```asm
        OPT     MC              ; List macro calls
        OPT     NOMC            ; Don't list macro calls
        OPT     MD              ; List macro definitions
        OPT     MEX             ; List macro expansions
        OPT     L               ; Enable listing
        OPT     NOL             ; Disable listing
```

### LIST/NOLIST - Listing Control

Toggles listing output.

```asm
        LIST                    ; Enable listing
        ; ... code listed ...
        NOLIST                  ; Disable listing
        ; ... code not listed ...
```

## Conditional Assembly

### COND/ENDC

Basic conditional assembly.

```asm
DEBUG   EQU     1               ; Debug flag

        COND    DEBUG           ; Assemble if DEBUG != 0
        LDA     #$FF
        STA     DEBUGPORT
        ENDC
```

### IF Family

```asm
        IFEQ    TARGET          ; If TARGET == 0
        ; ...
        ENDIF

        IFNE    TARGET          ; If TARGET != 0
        ; ...
        ENDIF

        IFGT    COUNT-10        ; If COUNT > 10
        ; ...
        ELSE
        ; ...
        ENDIF

        IFGE    VALUE-5         ; If VALUE >= 5
        IFLT    SIZE-100        ; If SIZE < 100
        IFLE    LIMIT-50        ; If LIMIT <= 50
```

## Macro System

### MACRO/ENDM

Define reusable code blocks.

```asm
* Macro definition
DELAY   MACRO
        LDB     \\1             ; Parameter 1
\\.LOOP DECB                    ; Local label
        BNE     \\.LOOP
        ENDM

* Macro invocation
        DELAY   #10             ; Call with parameter
```

**Parameters:**
- `\\1` through `\\9` - Positional parameters
- `\\@` - Unique ID for this invocation
- `\\.label` - Local label unique to invocation

## INCLUDE Directive

Include external source files.

```asm
        INCLUDE os9.d           ; Include OS-9 definitions
        INCLUDE macros.asm      ; Include macro library
```

**Note:** INCLUDE allows splitting large projects into modules.

## Number Formats

```asm
        LDA     #10             ; Decimal (default)
        LDA     #$FF            ; Hexadecimal ($ prefix)
        LDA     #%11110000      ; Binary (% prefix)
        LDA     #'A'            ; ASCII character
```

## Expressions

EDTASM+ supports arithmetic expressions:

```asm
RESULT  EQU     (VALUE+10)*2    ; Arithmetic
        LDA     #BUFFER+OFFSET  ; Address arithmetic
        FDB     START-END       ; Subtraction
```

**Operators:**
- `+`, `-` - Addition, subtraction
- `*`, `/` - Multiplication, division
- `&`, `|`, `^` - AND, OR, XOR (vasm extension)
- `<`, `>` - Low byte, high byte extraction

## Symbol Rules

**Valid characters:**
- Start: `A-Z`, `a-z`, `_` (underscore)
- Continue: `A-Z`, `a-z`, `0-9`, `_`, `$` (dollar sign in vasm-ext)

**Case sensitivity:**
- Original EDTASM: case-insensitive
- Modern implementations: case-sensitive by default, `-nocase` flag for compatibility

## OS-9 Support

### System Calls

```asm
        INCLUDE os9.d           ; OS-9 definitions

        LDA     #READ
        LDX     #FILENAME
        OS9     I$Open          ; System call
        BCS     ERROR
```

### MOD Directive

OS-9 module header (implementation-specific).

```asm
TypeLang EQU    $11
AttRev   EQU    $81

        MOD     MODEND,MODNAME,TypeLang,AttRev,START,MEMSIZE

MODNAME FCC     /MyProgram/
        FCB     $00
START   ; Code begins
MODEND  EQU     *
```

## Differences from Other Syntaxes

**vs. Merlin (6502):**
- No colon before labels (EDTASM uses whitespace)
- Different directive names (FCB vs ASC/HEX)
- Macro syntax uses `\\` prefix instead of `]`
- Case-insensitive by default

**vs. AS6809/LWASM:**
- Similar directive set
- EDTASM has richer macro system
- Different local label syntax

## Complete Example

```asm
* Example EDTASM program for CoCo
* Print "Hello, World!" to screen

        ORG     $0400           ; Program starts at $0400

PUTCHR  EQU     $A002           ; ROM routine for char output

START   LDX     #MESSAGE        ; Point to message
LOOP    LDA     ,X+             ; Get character
        BEQ     DONE            ; Zero byte = end
        JSR     PUTCHR          ; Print it
        BRA     LOOP            ; Next character

DONE    RTS                     ; Return to BASIC

MESSAGE FCC     /Hello, World!/
        FCB     $0D             ; Carriage return
        FCB     $00             ; Null terminator

        END     START           ; End, entry at START
```

## References

### Online Resources
- [TRS-80 EDTASM Documentation](http://www.trs-80.org/edtasm/)
- [EDTASM+ - CoCopedia](https://www.cocopedia.com/wiki/index.php/EDTASM+)
- [Color Computer Archive - Programming Manuals](https://colorcomputerarchive.com/repo/Documents/Manuals/Programming/)
- [FLEX 6809 Assembler Directives](http://datapipe-blackbeltsystems.com/windows/flex/asm5.html)
- [asm6809 Documentation](https://www.6809.org.uk/asm6809/doc/asm6809.shtml)

### Print References
- Disk EDTASM Remastered (Tandy) - Official Manual
- 6809 Assembly Language Programming (Lance Leventhal)
- TRS-80 Color Computer Technical Reference Manual

### Implementation Reference
- vasm-ext EDTASM syntax module: `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm/`
