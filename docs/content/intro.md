---
sidebar_position: 1
---

# Introduction

Welcome to **xasm++** - a modern C++ cross-assembler for vintage CPU architectures including the 6502 family and Motorola 6809.

## What is xasm++?

**xasm++** is a production-ready cross-assembler that supports multiple vintage CPU architectures through a clean plugin-based design. Perfect for retro computing, homebrew development, and preservation of classic software.

### Key Features

- 🎯 **Single Binary** - One executable supporting multiple CPU architectures and syntax modes
- 🔌 **CPU Plugin Architecture** - Clean polymorphic design with zero casting
- 🏗️ **Modern C++17** - Maintainable codebase following SOLID principles
- ✅ **Comprehensive Testing** - 1649/1649 tests passing (100%), TDD methodology
- 🚀 **Production Ready** - Successfully assembles Prince of Persia and other vintage software
- 🌍 **Cross-Platform** - Linux, macOS, Windows support via CMake

## Supported CPUs

### 6502 Family ✅ COMPLETE

All variants fully implemented with complete instruction sets:

- **6502** - MOS Technology 6502 (Apple II, Commodore 64, NES)
  - 56 legal opcodes, 13 addressing modes
  - Branch instruction relative addressing
  - Multi-pass assembly with forward reference resolution

- **65C02** - WDC 65C02 (Apple IIc, IIe enhanced)
  - All 6502 instructions plus 27 new instructions
  - Enhanced addressing modes (BIT immediate, STZ, BRA, etc.)

- **65C02 Rockwell** - Rockwell R65C02
  - 65C02 variant with additional bit manipulation instructions

- **65816** - WDC 65816 (Apple IIgs, SNES)
  - Full 16-bit architecture support
  - Long addressing (24-bit)
  - Mode-switching instructions (REP, SEP)

### Motorola 6809 ✅ COMPLETE

Fully implemented with all addressing modes:

- **6809** - Motorola 6809 (TRS-80 Color Computer, Dragon)
  - Complete instruction set (load/store, arithmetic, logic, branches)
  - All 16 indexed addressing modes
  - Long branches (16-bit displacements)
  - Big-endian byte order
  - Direct page register support

### Zilog Z80 ✅ COMPLETE

Fully implemented with comprehensive instruction set:

- **Z80** - Zilog Z80 (Game Boy, ZX Spectrum, TRS-80 Model I)
  - Complete instruction set (all standard opcodes)
  - All addressing modes and register combinations
  - Extended instructions (IX/IY indexed, bit operations)
  - Block transfer and search instructions
  - Multiple syntax variants supported

**Test Coverage:**
- All CPU tests: 100% passing
- Integration tests: Complete
- End-to-end: Prince of Persia assembly verified

## Supported Syntax Modes

### Merlin Syntax ✅ (6502/65C02/65816)
Apple II assembler syntax with full directive support:
- `ORG`, `EQU`, `DFB`, `DW`, `ASC`, `DS`, `END`
- Label conventions and forward references
- Expression evaluation

### SCMASM Syntax ✅ (6502/65C02/65816)
S-C Macro Assembler syntax:
- `.ORG`, `.BYTE`, `.WORD`, `.ASCII`
- Constant definitions with `=`
- Compatible with classic Apple II development

### EDTASM Syntax ✅ (6809)
TRS-80 Color Computer EDTASM+ syntax:
- `ORG`, `EQU`, `FCB`, `FDB`, `FCC`, `RMB`, `END`
- Motorola-style addressing modes
- Direct page directives (`SETDP`)

### FlexASM Syntax ✅ (6809)
Motorola FLEX assembler syntax:
- Standard Motorola directives
- Compatible with FLEX operating system tools

### Z80 Universal Syntax ✅ (Z80)
Flexible Z80 assembly syntax:
- Multiple number format support (hex, binary, decimal)
- Various label styles (local, global)
- Standard Z80 directives
- Compatible with popular Z80 assemblers

## Command-Line Usage

```bash
# Assemble for 6502 with Merlin syntax
xasm++ --cpu 6502 --syntax merlin -o output.bin input.asm

# Assemble for 6809 with EDTASM syntax
xasm++ --cpu 6809 --syntax edtasm -o output.bin input.asm

# Apple IIgs with 65816
xasm++ --cpu 65816 --syntax merlin -o output.obj source.asm

# Z80 with universal syntax
xasm++ --cpu z80 --syntax z80universal -o output.bin input.asm
```

## Project Status

**Current Version:** Production Ready
**Total Tests:** 1649/1649 passing (100%) ✅
**Build System:** CMake with organized output (build/bin/, build/lib/, build/Testing/)

### Continuous Integration

All platforms continuously tested via GitHub Actions:

- ✅ **Ubuntu** (Debug + Release) - 100% tests passing
- ✅ **macOS** (Debug + Release) - 100% tests passing
- ✅ **Windows** (Debug + Release) - 100% tests passing
- ✅ **Code Coverage** - 77.9% line coverage
- ✅ **Format Check** - clang-format validation

**CI Status:** 8/8 jobs passing ✅

**Completed Features:**
- ✅ 6502 family CPU support (4 variants: 6502, 65C02, 65C02 Rockwell, 65816)
- ✅ 6809 CPU support (Motorola)
- ✅ Z80 CPU support (Zilog)
- ✅ CPU plugin architecture with clean polymorphic design
- ✅ Merlin syntax parser (Apple II)
- ✅ SCMASM syntax parser (Apple II)
- ✅ EDTASM syntax parser (TRS-80 CoCo)
- ✅ FlexASM syntax parser (Motorola FLEX)
- ✅ Z80 Universal syntax parser
- ✅ SimpleSyntax parser (basic generic syntax)
- ✅ Multi-pass assembly with convergence
- ✅ Forward reference resolution
- ✅ Expression evaluation
- ✅ Binary output generation

### Planned Enhancements

- **68000** - Motorola 68000 (Amiga, Atari ST, Sega Genesis)
- Listing output (with cross-references)
- Symbol table output
- Macro support with parameters
- Conditional assembly (IF/ELSE/ENDIF)
- Include file support

## Quick Start

Ready to get started? Head over to the [Quick Start Guide](getting-started/quick-start.md) to begin assembling code.

## Design Philosophy

xasm++ combines architectural patterns from proven assembler projects:

### Modular Design (inspired by vasm)
- Three-layer architecture: Syntax → CPU → Output
- Atom-based intermediate representation
- Multi-pass resolution with optimization

### Modern Architecture
- SOLID principles throughout
- Plugin architecture with polymorphic dispatch
- Strategy pattern for algorithms
- Dependency injection
- Comprehensive unit testing

The result is a modern, extensible assembler that's easy to maintain and extend with new CPU architectures and syntax modes.

## Architecture Highlights

### CPU Plugin System
Clean polymorphic interface eliminates casting and enables runtime CPU selection:

```cpp
class CpuPlugin {
    virtual std::vector<uint8_t> EncodeInstruction(
        const std::string& mnemonic,
        uint32_t operand,
        const std::string& operand_str
    ) const = 0;
};
```

All CPU variants implement this interface, enabling:
- Zero runtime overhead
- Type-safe dispatch
- Easy addition of new CPUs

## Real-World Validation

xasm++ has been validated by successfully assembling:
- **Prince of Persia** (Apple II) - Complete assembly with 100% test pass rate
- Demonstrates compatibility with historical source code
- Proves production readiness

## Community and Support

- **GitHub Repository**: [xasm++](https://github.com/Cortexa-LLC/xasm-plus-plus)
- **Issue Tracker**: Report bugs and request features
- **Documentation**: Comprehensive guides and API reference

## License

Copyright © 2026 Cortexa LLC

xasm++ is released under the MIT License. See the [LICENSE](https://github.com/Cortexa-LLC/xasm-plus-plus/blob/main/LICENSE) file for details.

---

**Next Steps:**
- [Quick Start Guide](getting-started/quick-start.md) - Start assembling code
- [Architecture Overview](architecture/overview.md) - Understand the design
- [Reference Documentation](reference/merlin-syntax-reference.md) - Syntax details
- [Developer Documentation](api/assembler.md) - API integration
