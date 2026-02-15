# xasm++ Examples

This directory contains example assembly programs demonstrating Phase 1 capabilities.

## Running Examples

### Simple Syntax (default)
```bash
../build/bin/xasm++ hello_6502.asm -o hello_6502.bin
```

### Merlin Syntax
```bash
../build/bin/xasm++ merlin_demo.asm -o merlin_demo.bin --syntax merlin
```

## Example Programs

### hello_6502.asm
Simple 6502 program demonstrating:
- Immediate addressing (`lda #$42`)
- Absolute addressing (`sta $0200`)
- Register operations (`inx`, `iny`)
- Branch instructions (`bne loop`)
- Jump instructions (`jmp start`)
- Basic control flow

**Expected output:** 20 bytes
```
a9 42 8d 00 02 a2 00 a0 10 e8 c8 e0 10 d0 fa 4c 00 80 ea 60
```

### merlin_demo.asm
Merlin syntax demonstration showing:
- Asterisk comments (`* comment`)
- Column-based layout
- `equ` and `=` directives
- Indexed addressing (`SCREEN,x`)
- Label references

**Expected output:** 17 bytes

## Phase 1 Features Demonstrated

✅ **Core Abstractions**
- Symbol table (labels, equates)
- Sections with origin (`org $8000`)
- Expressions (literal values, symbol references)

✅ **6502 CPU Plugin**
- Essential instructions (LDA, STA, LDX, LDY, INX, INY, CPX, BEQ, BNE, JMP, NOP, RTS)
- Addressing modes (Immediate, ZeroPage, Absolute, Relative)
- Proper opcode encoding

✅ **Syntax Plugins**
- Simple syntax (`.org`, `;` comments)
- Merlin syntax (`org`, `*` comments, column layout, `equ`/`=`)

✅ **Binary Output**
- Raw binary file generation
- Correct little-endian encoding
- Origin directive support

✅ **Command-Line Interface**
- Input/output file specification
- Syntax selection (`--syntax`)
- CPU selection (`--cpu`, defaults to 6502)
- Help and version flags

## Verifying Output

Use `hexdump` to inspect generated binaries:
```bash
hexdump -C hello_6502.bin
```

Compare opcodes against 6502 reference:
- http://www.6502.org/tutorials/6502opcodes.html
- http://www.obelisk.me.uk/6502/reference.html
