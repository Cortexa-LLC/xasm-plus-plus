# xasm++ - Modern Modular Cross-Assembler

**Version:** 1.0.0 (Production Ready)
**Author:** Bryan Woodruff, Cortexa LLC
**License:** MIT

[![CI Status](https://github.com/Cortexa-LLC/xasm-plus-plus/workflows/CI/badge.svg)](https://github.com/Cortexa-LLC/xasm-plus-plus/actions)
[![Code Coverage](https://img.shields.io/badge/coverage-77.9%25-green)](https://github.com/Cortexa-LLC/xasm-plus-plus)

---

## Overview

**xasm++** is a production-ready cross-assembler for vintage CPU architectures. It combines proven modular design patterns with modern SOLID principles to create an extensible, maintainable assembler suitable for retro computing, homebrew development, and preservation of classic software.

### Key Features

- 🎯 **Single Binary** - One executable supporting 7 CPU variants (6502, 65C02, 65C02 Rockwell, 65816, 6809, Z80)
- 🔌 **CPU Plugin Architecture** - Clean polymorphic design with zero casting
- 🏗️ **Modern C++20** - Maintainable codebase following SOLID principles
- ✅ **Comprehensive Testing** - 1649/1649 tests passing (100%), TDD methodology
- 🚀 **Production Ready** - Successfully assembles Prince of Persia and other vintage software
- 🌍 **Cross-Platform** - Linux, macOS, Windows with full CI coverage
- 📊 **Quality Assurance** - 77.9% code coverage, automated format checking

### Supported CPUs

#### 6502 Family ✅ COMPLETE
- **6502** - MOS Technology 6502 (Apple II, Commodore 64, NES)
- **65C02** - WDC 65C02 (Apple IIc, IIe enhanced)
- **65C02 Rockwell** - Rockwell R65C02 with bit manipulation
- **65816** - WDC 65816 (Apple IIgs, SNES) with 16-bit support

#### Motorola ✅ COMPLETE
- **6809** - Motorola 6809 (TRS-80 Color Computer, Dragon)

#### Zilog ✅ COMPLETE
- **Z80** - Zilog Z80 (Game Boy, ZX Spectrum, TRS-80 Model I)

### Supported Syntax Modes

- ✅ **Merlin** - Apple II assembler syntax (6502/65C02/65816)
- ✅ **SCMASM** - S-C Macro Assembler (6502/65C02/65816)
- ✅ **EDTASM** - TRS-80 Color Computer EDTASM+ (6809)
- ✅ **FlexASM** - Motorola FLEX assembler (6809)
- ✅ **Z80 Universal** - Flexible Z80 assembly syntax

---

## Project Status

**Status:** Production Ready ✅
**Total Tests:** 1649/1649 passing (100%)
**Code Coverage:** 77.9% line coverage
**CI Status:** 8/8 jobs passing (Ubuntu, macOS, Windows × Debug/Release + Coverage + Format)

### Continuous Integration

All platforms continuously tested via GitHub Actions:

- ✅ **Ubuntu** (Debug + Release) - 100% tests passing
- ✅ **macOS** (Debug + Release) - 100% tests passing
- ✅ **Windows** (Debug + Release) - 100% tests passing
- ✅ **Code Coverage** - 77.9% line coverage
- ✅ **Format Check** - clang-format validation

### Completed Features

- ✅ 6502 family CPU support (4 variants)
- ✅ 6809 CPU support (Motorola)
- ✅ Z80 CPU support (Zilog)
- ✅ CPU plugin architecture with clean polymorphic design
- ✅ 5 syntax parsers (Merlin, SCMASM, EDTASM, FlexASM, Z80 Universal)
- ✅ Multi-pass assembly with convergence
- ✅ Forward reference resolution
- ✅ Expression evaluation
- ✅ Binary output generation
- ✅ Cross-platform support (Linux, macOS, Windows)

### Real-World Validation

xasm++ has been validated by successfully assembling:
- **Prince of Persia** (Apple II) - Complete 29-file assembly with bootable disk images
- Demonstrates compatibility with historical source code
- Proves production readiness

See [Documentation Site](https://cortexa-llc.github.io/xasm-plus-plus/) for comprehensive guides and API reference.

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

- `build/bin/xasm++` - Main assembler executable (in development)
- `build/tests/unit/test_assembler` - Assembler unit tests
- `build/tests/unit/test_cpu6502` - 6502 CPU plugin tests

### Requirements

- **C++ Compiler:** GCC 9+, Clang 10+, or MSVC 2019+
- **CMake:** 3.20 or later
- **Git:** 2.20 or later
- **Google Test:** Automatically fetched by CMake

---

## Quality Assurance

xasm++ uses multiple tools to ensure code quality:

### Testing

**Test Suite:**
- **500 tests** total (100% passing)
- **267 unit tests** - Component-level validation
- **231 integration tests** - Cross-component behavior
- **2 E2E tests** - Full assembler pipeline validation
- **85%+ code coverage** - Industry-leading coverage

**Test Commands:**
```bash
# Run all tests
cd build && ctest

# Run specific test suites
./tests/unit/test_assembler
./tests/unit/test_cpu6502
```

### Static Code Analysis

**SonarQube Integration:**

The ai-pack framework provides SonarQube analysis tools for comprehensive code quality metrics.

**Setup (one-time):**
```bash
# 1. Start SonarQube server (uses Docker)
cd .ai-pack
python3 scripts/setup-sonarqube.py

# SonarQube will be available at http://localhost:9000
# Default credentials: admin/admin
```

**Run Analysis:**
```bash
# Generate compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# Run SonarQube scanner
sonar-scanner

# View results at http://localhost:9000
```

**Configuration:**
- `sonar-project.properties` - SonarQube configuration at repository root
- Analyzes C++ code with compile_commands.json
- Tracks code smells, bugs, vulnerabilities, and technical debt

### Code Quality Metrics

**Achieved Metrics (Post-Refactoring):**
- **Quality Grade:** A- (improved from C+)
- **Code Duplication:** <5% (reduced from 95%)
- **Compiler Warnings:** Zero
- **Test Coverage:** 85%+
- **Documentation:** 100% API coverage (Doxygen)
- **Lines of Code:** ~800 clean lines (60% reduction from 2,000)

**See Also:**
- [Code Quality Refactoring Complete](build/CODE_QUALITY_REFACTORING_COMPLETE.md) - Full refactoring report
- [Remaining Concerns](REMAINING_CONCERNS.md) - Future enhancements

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
