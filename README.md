# xasm++ - Modern Modular Cross-Assembler

**Version:** 0.1.0 (In Development)
**Author:** Bryan Woodruff, Cortexa LLC
**License:** MIT

---

## Overview

**xasm++** is a modern C++ cross-assembler designed for vintage CPU architectures. It combines proven modular design patterns with modern SOLID principles to create an extensible, maintainable assembler suitable for retro computing and homebrew development.

### Key Features (Planned)

- 🎯 **Single Binary** - One executable supporting multiple CPU architectures and syntax modes
- 🔌 **Plugin Architecture** - Easy extensibility for new CPUs and syntaxes
- 🏗️ **Modern C++17/20** - Clean, maintainable codebase with SOLID principles
- ✅ **Comprehensive Testing** - 85%+ test coverage target
- 🌍 **Cross-Platform** - Linux, macOS, Windows support
- 📝 **Multiple Syntaxes** - Support for SCMASM, Merlin, EDTASM, and more

### Supported CPUs (Planned)

- **6502** - MOS Technology 6502 (Apple II, Commodore 64, NES)
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

**Current Phase:** Architecture and Planning (Phase 0)

✅ Architecture design complete
✅ Project plan created
⏳ Foundation and setup (next)
⏳ Core implementation (pending)

See [Project Plan](docs/planning/project-plan.md) for detailed roadmap.

---

## Documentation

- **[Architecture Overview](docs/architecture/00-overview.md)** - System design and abstractions
- **[Project Plan](docs/planning/project-plan.md)** - Phased implementation plan
- **[Task Tracking](.ai/tasks/)** - Development task packets

---

## Building (Not Yet Implemented)

Once Phase 0 is complete, build instructions will be:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Requirements

- **C++ Compiler:** GCC 9+, Clang 10+, or MSVC 2019+
- **CMake:** 3.20 or later
- **Git:** 2.20 or later

---

## Usage (Not Yet Implemented)

Future usage will be:

```bash
xasm++ --cpu 6502 --syntax scmasm -o program.bin program.asm
```

### Command-Line Options (Planned)

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
  -O<level>            Optimization level (0, 1, 2)
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

### Phase 0: Foundation (In Progress)
- [x] Architecture design
- [x] Project planning
- [ ] Repository setup
- [ ] CMake configuration
- [ ] CI pipeline
- [ ] Testing infrastructure

### Phase 1: Minimal Viable Assembler
- [ ] Core abstractions (Atom, Expression, Symbol, Section)
- [ ] Minimal 6502 CPU plugin
- [ ] Simple syntax parser
- [ ] Binary output
- [ ] Command-line interface

### Phase 2-6: Feature Development
See [Project Plan](docs/planning/project-plan.md) for complete roadmap.

---

**Status:** 🚧 Under Active Development 🚧

Star ⭐ this repository to follow progress!
