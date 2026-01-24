# xasm++ Reference Documentation

**Location:** `docs/references/`
**Purpose:** Reference materials for vintage CPU architectures and assembler syntax (6502, 6809, Z80)

---

## Overview

This directory contains reference materials used during the development of xasm++, including:
- CPU architecture documentation (6502, 6809, Z80)
- Assembler syntax references (Merlin, SCMASM, EDTASM)
- Opcode tables and addressing mode guides
- Historical documentation from vintage systems

These materials inform the design and implementation of xasm++ to ensure compatibility with existing assemblers and accurate emulation of vintage system behavior.

---

## Contents

### Syntax Comparison and Guides

**[6502-SYNTAX-COMPARISON.md](6502-SYNTAX-COMPARISON.md)**
- Comprehensive comparison of Merlin, SCMASM, and EDTASM syntax
- Addressing mode syntax (universal across assemblers)
- Directive differences (`.EQ` vs `EQU`, `.OR` vs `ORG`, etc.)
- 65C02 and 65816 requirements
- Implementation roadmap for xasm++

**Status:** ✅ Complete - primary reference for syntax implementation

### Architecture References

**[6809-ARCHITECTURE.md](6809-ARCHITECTURE.md)**
- Complete Motorola 6809 architecture reference
- All instruction types and addressing modes
- EDTASM+ assembler syntax (TRS-80 Color Computer)
- Register descriptions (A, B, D, X, Y, S, U, PC, DP, CC)
- Comparison with 6502 architecture
- Implementation requirements for xasm++

**Status:** ✅ Complete - ready for implementation

**[Z80-ARCHITECTURE.md](Z80-ARCHITECTURE.md)**
- Complete Zilog Z80 architecture reference
- All 252 opcodes (including undocumented)
- Multiple addressing modes and syntax variants
- Register descriptions (A, F, BC, DE, HL, IX, IY, SP, PC)
- Block operations and interrupt modes
- TRS-80 Model I/III/4 specifics
- Implementation requirements for xasm++

**Status:** ✅ Complete - ready for implementation

---

### Historical Documentation

**[Pages from 6502 Assembly Language Programming (Lance Levanthal).pdf](Pages%20from%206502%20Assembly%20Language%20Programming%20%28Lance%20Levanthal%29.pdf)**
- Classic 6502 programming reference
- Detailed opcode descriptions
- Addressing mode explanations
- Programming techniques and examples
- **Source:** Lance A. Leventhal, "6502 Assembly Language Programming", Osborne/McGraw-Hill, 1979

**[Pages from 6809 Assembly Language Programming (Lance Leventhal).pdf](Pages%20from%206809%20Assembly%20Language%20Programming%20%28Lance%20Leventhal%29.pdf)**
- Classic 6809 programming reference
- Complete instruction set with examples
- Addressing mode details
- TRS-80 Color Computer programming techniques
- **Source:** Lance A. Leventhal, "6809 Assembly Language Programming", Osborne/McGraw-Hill

**[Inside the Apple IIe.txt](Inside%20the%20Apple%20IIe.txt)**
- Apple IIe technical reference
- Memory map and I/O addresses
- ROM routines and entry points
- System architecture details
- **Source:** Quality Software, "Inside the Apple IIe"

---

## Usage Guidelines

### For Implementation

When implementing new features in xasm++:

1. **Check syntax comparison first** - `6502-SYNTAX-COMPARISON.md` documents all known syntax variants
2. **Verify opcode behavior** - Use Leventhal reference for detailed opcode descriptions
3. **Validate addressing modes** - Cross-reference with multiple sources
4. **Check system-specific details** - Apple IIe reference for platform-specific behavior

### For Testing

When creating test cases:

1. **Use realistic syntax** - Reference actual assembler syntax from comparison doc
2. **Test edge cases** - Check Leventhal for undocumented behaviors
3. **Validate opcodes** - Ensure exact opcode bytes match reference materials
4. **Test platform idioms** - Use Apple IIe reference for common patterns

---

## Reference Material Organization

### By CPU Architecture

**6502 (MOS Technology 6502)**
- Core architecture: `6502-SYNTAX-COMPARISON.md` (addressing modes section)
- Opcodes: `Pages from 6502 Assembly Language Programming (Lance Levanthal).pdf`
- 56 legal opcodes
- 13 addressing modes
- **Status:** ✅ Fully supported in xasm++ (Phase 2.4)

**65C02 (WDC 65C02)**
- Extensions: `6502-SYNTAX-COMPARISON.md` (65C02 section)
- New opcodes: BRA, PHX, PLX, PHY, PLY, STZ, TRB, TSB
- Enhanced addressing modes: `LDA ($80)`, `JMP ($1234,X)`, `BIT #$80`
- Rockwell/WDC extensions: BBR/BBS/RMB/SMB
- **Status:** ⏳ Planned (Phase 4+)

**65816 (WDC 65C816)**
- Architecture: `6502-SYNTAX-COMPARISON.md` (65816 section)
- 16-bit registers, 24-bit addressing
- New opcodes: PHB, PLB, PHK, PHD, PLD, MVN, MVP, JML, JSL, RTL, etc.
- Register width control: `MX` directive
- **Status:** ⏳ Planned (Phase 4+)

**6809 (Motorola 6809)**
- Architecture: `6809-ARCHITECTURE.md` (complete reference)
- Opcodes: `Pages from 6809 Assembly Language Programming (Lance Leventhal).pdf`
- Advanced instruction set (~59 instruction types)
- 13 addressing modes with sophisticated indexing
- Dual accumulators (A, B, or combined as D)
- Two 16-bit index registers (X, Y)
- Dual stack pointers (S, U)
- Primary syntax: EDTASM+ (TRS-80 Color Computer)
- **Status:** ⏳ Planned (Phase 5+)

**Z80 (Zilog Z80)**
- Architecture: `Z80-ARCHITECTURE.md` (complete reference)
- ~252 opcodes (including undocumented instructions)
- Intel 8080 compatible
- Two register sets (main and alternate)
- Powerful block operations (LDIR, CPIR, etc.)
- Sophisticated interrupt handling (3 modes)
- Index registers (IX, IY) with displacement
- Primary systems: TRS-80, ZX Spectrum, MSX, Game Boy
- **Status:** ⏳ Planned (Phase 5+)

### By Syntax Variant

**Merlin**
- Reference: `6502-SYNTAX-COMPARISON.md` (Merlin section)
- Apple II and IIgs standard assembler
- No line numbers
- Semicolon comments
- Directives: `EQU`, `ORG`, `DFB`, `DA`, `ASC`, `PUT`
- **Status:** ⏳ Planned (Phase 3+)

**SCMASM**
- Reference: `6502-SYNTAX-COMPARISON.md` (SCMASM section)
- S-C Macro Assembler (Apple II)
- BASIC-style line numbers (1000, 1010, ...)
- Asterisk comments
- Dot-prefixed directives: `.EQ`, `.OR`, `.DA`, `.AS`, `.IN`
- **Status:** ⏳ Planned (Phase 3+)

**EDTASM+**
- Reference: `6809-ARCHITECTURE.md` (EDTASM+ section)
- TRS-80 Color Computer assembler (6809)
- Semicolon comments
- Directives: `EQU`, `ORG`, `FCB`, `FDB`, `FCC`, `RMB`, `SETDP`
- Conditional assembly: `IFEQ`, `IFNE`, `ENDC`
- **Status:** ⏳ Planned (Phase 5+ with 6809 support)

---

## Adding New References

When adding new reference materials to this directory:

1. **Add the file** to this directory
2. **Update this README** with:
   - File name and description
   - Source attribution
   - Relevant section (CPU architecture, syntax, platform-specific)
   - Status (how it relates to xasm++ implementation)
3. **Update relevant documents**:
   - If syntax-related: Update `6502-SYNTAX-COMPARISON.md`
   - If opcode-related: Create or update opcode reference docs
   - If platform-specific: Note in platform documentation

---

## Sources and Attribution

All reference materials in this directory are used for educational and development purposes.

**Primary Sources:**
1. **sourcerer** - Cortexa LLC multi-CPU disassembler
   - Merlin and SCMASM formatter implementations
   - Test cases and sample output
   - Location: `/Users/bryanw/Projects/Vintage/tools/sourcerer/`

2. **vasm-ext** - Extended vasm assembler
   - Syntax module documentation
   - Opcode tables
   - Author contributions by Bryan Woodruff (SCMASM, EDTASM, Merlin modules)

3. **Historical Documentation**
   - Lance A. Leventhal, "6502 Assembly Language Programming"
   - Lance A. Leventhal, "6809 Assembly Language Programming"
   - Quality Software, "Inside the Apple IIe"

**Copyright:**
- xasm++ documentation: Copyright © 2026 Bryan Woodruff, Cortexa LLC, MIT License
- Historical references: Copyright their respective owners, used under fair use for educational purposes

---

## External Resources

### Online References

**6502 Architecture:**
- [6502.org](http://6502.org/) - 6502 community and resources
- [Nesdev Wiki - 6502](https://www.nesdev.org/wiki/CPU) - Detailed 6502 documentation
- [Visual 6502](http://visual6502.org/) - 6502 die visualization

**65C02 and 65816:**
- [WDC 65C02 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c02s.pdf)
- [WDC 65C816 Datasheet](https://www.westerndesigncenter.com/wdc/documentation/w65c816s.pdf)
- [65816 Programming Manual](http://www.defence-force.org/computing/oric/coding/annexe_2/65816.pdf)

**Apple II:**
- [AppleWin](https://github.com/AppleWin/AppleWin) - Apple II emulator
- [Apple II Documentation Project](https://www.apple2online.com/web_documents/)
- [Brutal Deluxe](https://www.brutaldeluxe.fr/) - Apple II technical resources

**6809 Architecture:**
- [6809.org.uk](http://www.6809.org.uk/) - 6809 community and resources
- [Color Computer Archive](https://colorcomputerarchive.com/) - CoCo documentation
- [CoCo SDC](http://cocosdc.blogspot.com/) - Hardware and software resources
- [Motorola 6809 Datasheet](http://www.6809.org.uk/dragon/illegal-opcodes.html)

**Z80 Architecture:**
- [Z80.info](http://z80.info/) - Comprehensive Z80 resources
- [Z80 CPU User Manual](https://www.zilog.com/docs/z80/um0080.pdf) - Official Zilog documentation
- [ClrHome Z80 Tables](http://clrhome.org/table/) - Complete opcode reference
- [TRS-80 Model I/III](http://www.trs-80.org/) - TRS-80 community
- [WorldOfSpectrum](https://worldofspectrum.org/) - ZX Spectrum resources

**Assemblers:**
- [Merlin 32](https://github.com/lroathe/merlin32) - Modern Merlin cross-assembler
- [vasm](http://sun.hasenbraten.de/vasm/) - Multi-architecture assembler
- [ca65](https://cc65.github.io/doc/ca65.html) - cc65 assembler
- [zmac](http://48k.ca/zmac.html) - Z80 macro assembler

---

## Status

**Current Phase:** Phase 2.4 Complete - 6502 Syntax Parsing
**Next Phase:** Phase 3 - CLI and File I/O
**Future Phases:** Syntax modes (Merlin, SCMASM), 65C02/65816/6809/Z80 support

**Reference Material Status:**
- ✅ 6502 core architecture documented
- ✅ Syntax comparison complete (Merlin, SCMASM, EDTASM)
- ✅ 6502 addressing modes fully documented (13 modes)
- ✅ 6809 architecture documented (complete with EDTASM+ syntax)
- ✅ Z80 architecture documented (complete with syntax variants)
- ✅ Historical references copied (Leventhal 6502, Leventhal 6809, Apple IIe)
- ⏳ 65C02 extensions documented (implementation pending)
- ⏳ 65816 extensions documented (implementation pending)

---

**Last Updated:** 2026-01-24
**Maintained By:** Bryan Woodruff, Cortexa LLC

