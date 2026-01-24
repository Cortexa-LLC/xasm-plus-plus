# xasm++ - Modern Modular Cross-Assembler

**Version:** 0.2.0 (Active Development - Phase 2.4 Complete)
**Author:** Bryan Woodruff, Cortexa LLC
**License:** MIT

---

## Overview

**xasm++** is a modern C++ cross-assembler designed for vintage CPU architectures. It combines proven modular design patterns with modern SOLID principles to create an extensible, maintainable assembler suitable for retro computing and homebrew development.

### Key Features

- 🎯 **Single Binary** - One executable supporting multiple CPU architectures (6502 currently implemented)
- 🔌 **Plugin Architecture** - Extensible design for new CPUs and syntaxes
- 🏗️ **Modern C++17/20** - Clean, maintainable codebase with SOLID principles
- ✅ **Comprehensive Testing** - 197 tests, 100% passing, TDD methodology
- 🧪 **6502 CPU Support** - Complete implementation with 56 legal opcodes and 13 addressing modes
- 📝 **Full Syntax Parsing** - All 6502 addressing mode syntaxes supported with whitespace tolerance
- 🌍 **Cross-Platform** - Linux, macOS, Windows support via CMake

### Supported CPUs

- ✅ **6502** - MOS Technology 6502 (Apple II, Commodore 64, NES) - **COMPLETE**
  - 56 legal opcodes fully implemented
  - 13 addressing modes with complete syntax parsing
  - Branch instruction relative addressing
  - Multi-pass assembly with label resolution
  - 155 CPU tests, 100% passing

### Planned CPUs

- **65C02** - WDC 65C02 (Apple IIc, IIe enhanced)
- **6809** - Motorola 6809 (TRS-80 Color Computer)
- **68000** - Motorola 68000 (Amiga, Atari ST, Sega Genesis)
- **Z80** - Zilog Z80 (TRS-80, Sinclair ZX Spectrum, Game Boy)

### Supported Syntaxes (Planned)

- **SCMASM** - S-C Macro Assembler (Apple II)
- **Merlin** - Merlin assembler (Apple IIgs)
- **EDTASM** - EDTASM+ (TRS-80 Color Computer)
- **Motorola** - Motorola syntax (68000)
- **GNU-as** - GNU assembler style

---

## Project Status

**Current Phase:** Phase 2.4 - Complete 6502 Syntax Parsing ✅ **COMPLETE**

### Completed Phases

✅ **Phase 0:** Architecture and Planning
✅ **Phase 1:** Project Foundation
- Build system (CMake)
- Testing infrastructure (Google Test)
- Core abstractions (Atom, Section, Assembler)

✅ **Phase 2.1-2.3:** 6502 CPU Plugin
- Complete CPU plugin implementation
- 56 legal opcodes (ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP, JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA)
- 13 addressing modes fully implemented
- 155 CPU tests, 100% passing

✅ **Phase 2.4:** Complete 6502 Syntax Parsing
- All 8 addressing mode syntaxes implemented:
  - Accumulator (`ASL A`)
  - ZeroPageX/Y (`LDA $80,X`, `LDX $80,Y`)
  - AbsoluteX/Y (`LDA $1234,X`, `LDA $1234,Y`)
  - Indirect (`JMP ($1234)`)
  - IndexedIndirect (`LDA ($80,X)`)
  - IndirectIndexed (`LDA ($80),Y`)
- Branch instruction relative addressing
- Label support for all indexed modes
- Whitespace tolerance
- Multi-pass assembly with convergence
- 42 assembler tests (20 new), 100% passing

### Current Status

**Total Tests:** 197 (42 assembler + 155 CPU)
**Test Pass Rate:** 100% (197/197)
**Code Quality:** Zero compiler warnings, Reviewer approved
**Documentation:** Comprehensive task packets and inline comments

See [Project Plan](docs/planning/project-plan.md) for detailed roadmap and [Task Packets](.ai/tasks/) for implementation details.

---

## Documentation

- **[Architecture Overview](docs/architecture/00-overview.md)** - System design and abstractions
- **[Project Plan](docs/planning/project-plan.md)** - Phased implementation plan
- **[Task Tracking](.ai/tasks/)** - Development task packets

---

## Building

The project uses CMake and builds successfully on macOS, Linux, and Windows.

```bash
# Clone repository
git clone <repository-url>
cd xasm++

# Initialize submodules (ai-pack framework)
git submodule update --init --recursive

# Build
mkdir -p build
cd build
cmake ..
make -j8

# Run tests
./tests/unit/test_assembler  # 42 tests
./tests/unit/test_cpu6502     # 155 tests
```

### Build Output

- `build/src/xasm++` - Main assembler executable (in development)
- `build/tests/unit/test_assembler` - Assembler unit tests
- `build/tests/unit/test_cpu6502` - 6502 CPU plugin tests

### Requirements

- **C++ Compiler:** GCC 9+, Clang 10+, or MSVC 2019+
- **CMake:** 3.20 or later
- **Git:** 2.20 or later
- **Google Test:** Automatically fetched by CMake

---

## Usage

### Current API Usage

The assembler engine and 6502 CPU plugin are fully functional. CLI interface is coming in Phase 3.

**Example C++ API usage:**

```cpp
#include "assembler.h"
#include "cpu6502.h"

// Create assembler and CPU plugin
Assembler assembler;
Cpu6502 cpu;
assembler.SetCpuPlugin(&cpu);

// Create a code section
Section section(".text", static_cast<uint32_t>(SectionAttributes::Code), 0x8000);

// Add instructions
section.atoms.push_back(std::make_shared<InstructionAtom>("LDA", "#$42"));
section.atoms.push_back(std::make_shared<InstructionAtom>("STA", "$80"));
section.atoms.push_back(std::make_shared<LabelAtom>("loop", 0));
section.atoms.push_back(std::make_shared<InstructionAtom>("INX", ""));
section.atoms.push_back(std::make_shared<InstructionAtom>("BNE", "loop"));

// Assemble
assembler.AddSection(section);
AssemblerResult result = assembler.Assemble();

if (result.success) {
    // Access encoded bytes
    for (auto& atom : section.atoms) {
        if (atom->type == AtomType::Instruction) {
            auto inst = std::dynamic_pointer_cast<InstructionAtom>(atom);
            // inst->encoded_bytes contains machine code
        }
    }
}
```

**Supported 6502 Syntax:**

All 13 addressing modes with full syntax support:
- Implied: `INX`, `RTS`
- Accumulator: `ASL A`, `ROR A`
- Immediate: `LDA #$42`
- ZeroPage: `LDA $80`
- ZeroPageX: `LDA $80,X`
- ZeroPageY: `LDX $80,Y`
- Absolute: `JMP $1234`
- AbsoluteX: `LDA $1234,X`
- AbsoluteY: `LDA $1234,Y`
- Indirect: `JMP ($1234)`
- IndexedIndirect: `LDA ($80,X)`
- IndirectIndexed: `LDA ($80),Y`
- Relative: `BNE loop` (branch instructions)

### Future CLI Usage (Phase 3)

Coming soon:

```bash
xasm++ --cpu 6502 --syntax scmasm -o program.bin program.asm
```

### Planned Command-Line Options

```
Usage: xasm++ [options] input.asm

Options:
  --cpu <name>         CPU target (6502, 6809, 68000, z80)
  --syntax <name>      Syntax mode (scmasm, merlin, edtasm, motorola)
  -o, --output <file>  Output filename
  --list <file>        Generate listing file
  --symbols <file>     Generate symbol file
  --format <format>    Output format (binary, hex, srec)
  -I <path>            Include path
  -D<symbol>=<value>   Define symbol
  -v, --verbose        Verbose output
  --version            Show version
  --help               Show help
```

---

## Design Philosophy

xasm++ combines architectural patterns from two proven projects:

1. **[vasm](http://sun.hasenbraten.de/vasm/)** - Proven modular C assembler
   - Three-layer design (Syntax → CPU → Output)
   - Atom-based intermediate representation
   - Multi-pass resolution strategy

2. **[sourcerer](../sourcerer/)** - Modern C++ disassembler
   - SOLID principles throughout
   - Plugin architecture with registries
   - Strategy pattern for algorithms
   - Comprehensive testing

The result is a modern, extensible assembler that's maintainable and easy to extend with new CPU architectures and syntax modes.

---

## Contributing

Contributions are welcome! This project is in active development.

### How to Contribute

1. **Add a New CPU Plugin**
   - Implement `CpuPlugin` interface
   - Create opcode tables
   - Register in `CpuRegistry`
   - Add unit tests

2. **Add a New Syntax Plugin**
   - Implement `SyntaxPlugin` interface
   - Handle directives and expressions
   - Register in `SyntaxRegistry`
   - Add integration tests

3. **Improve Core Functionality**
   - Follow SOLID principles
   - Maintain 85%+ test coverage
   - Document design decisions

See [CONTRIBUTING.md](CONTRIBUTING.md) (coming soon) for detailed guidelines.

---

## Acknowledgments

- **vasm** - Volker Barthelmann and Frank Wille for the excellent reference implementation
- **sourcerer** - For demonstrating SOLID architecture in C++
- The vintage computing community for continued support

---

## Relationship to vasm-ext

The author (Bryan Woodruff) has contributed syntax modules to vasm-ext:
- SCMASM (Apple II S-C Macro Assembler)
- EDTASM++/M80 (CoCo/6809 and Z80)
- Merlin (Apple IIgs)

**xasm++** is a separate, clean-room C++ implementation inspired by vasm's architecture but not derived from vasm source code. It reimagines the proven patterns in modern C++ with SOLID principles.

---

## License

MIT License - see [LICENSE](LICENSE) file for details

Copyright (c) 2026 Bryan Woodruff, Cortexa LLC

---

## Contact

- **Author:** Bryan Woodruff
- **Company:** Cortexa LLC
- **Email:** bryan.woodruff@cortexa.com
- **GitHub:** [xasm++](https://github.com/your-org/xasm++) (placeholder)

---

## Development Roadmap

### Phase 0: Foundation ✅ COMPLETE
- [x] Architecture design
- [x] Project planning
- [x] Repository setup
- [x] CMake configuration
- [x] Testing infrastructure (Google Test)

### Phase 1: Project Foundation ✅ COMPLETE
- [x] Core abstractions (Atom, Expression, Symbol, Section)
- [x] Build system working
- [x] Test infrastructure
- [x] Basic assembler engine

### Phase 2.1-2.3: 6502 CPU Plugin ✅ COMPLETE
- [x] Complete 6502 CPU plugin with 56 legal opcodes
- [x] All 13 addressing modes implemented
- [x] 155 CPU tests, 100% passing
- [x] Opcode encoding for all addressing modes

### Phase 2.4: Complete 6502 Syntax Parsing ✅ COMPLETE
- [x] Accumulator mode syntax (`ASL A`)
- [x] Indexed mode syntax (`,X`, `,Y`)
- [x] Indirect mode syntax (`($addr)`)
- [x] Complex indirect modes (`($addr,X)`, `($addr),Y`)
- [x] Branch instruction relative addressing
- [x] Label support for all modes
- [x] Whitespace tolerance
- [x] 42 assembler tests, 100% passing

### Phase 3: CLI and File I/O (Next)
- [ ] Command-line interface
- [ ] Source file parsing
- [ ] Binary output (.bin, .prg)
- [ ] Listing file generation
- [ ] Symbol file generation

### Phase 4-6: Advanced Features
- [ ] Directives (.org, .byte, .word, .include)
- [ ] Macro support
- [ ] Conditional assembly
- [ ] Additional CPU architectures (65C02, 6809, Z80, 68000)
- [ ] Multiple syntax modes (SCMASM, Merlin, EDTASM)

See [Project Plan](docs/planning/project-plan.md) for complete roadmap and [Task Packets](.ai/tasks/) for detailed implementation history.

---

**Status:** 🚧 Under Active Development 🚧

Star ⭐ this repository to follow progress!
