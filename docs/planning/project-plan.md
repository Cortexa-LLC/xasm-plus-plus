# xasm++ Project Implementation Plan

**Version:** 1.0
**Date:** 2026-01-19
**Status:** Draft
**Project Lead:** Bryan Woodruff

---

## Executive Summary

This document outlines a phased implementation plan for **xasm++**, a modern C++ modular cross-assembler. The plan follows an incremental approach, building a minimal viable assembler first, then adding features and CPU/syntax support in subsequent phases.

**CPU Support (8 total):**
- 6502 family: 6502, 65C02, 65816
- Motorola: 6800, 6809, 68000
- Intel/Zilog: 8080/8085, Z80

**Syntax Support (8 total):**
- Base syntaxes: intel, zilog, motorola, mos
- Platform syntaxes: scmasm, merlin, edtasm, edtasm-m80

**Timeline Estimate:** Phases 0-7 represent complete implementation. Exact duration depends on available development capacity.

**Success Criteria:**
- Each phase delivers a working, tested increment
- Code follows SOLID principles throughout
- Maximize code reuse across CPU families (50-70% target)
- Platform-specific output formats for authentic retro development
- Comprehensive test coverage (85%+ overall)
- Documentation kept current with implementation

---

## Phase 0: Foundation and Setup

**Goal:** Establish project infrastructure and baseline configuration

**Duration:** 1-2 development sessions

### Tasks

#### 0.1 Repository Setup
- [x] Initialize git repository
- [x] Create directory structure:
  ```
  xasm++/
  ├── src/
  │   ├── core/
  │   ├── cpu/
  │   ├── syntax/
  │   ├── output/
  │   └── utils/
  ├── include/xasm++/
  ├── tests/
  ├── docs/
  │   ├── architecture/
  │   └── planning/
  ├── examples/
  └── CMakeLists.txt
  ```
- [ ] Create README.md with project description
- [ ] Create LICENSE file (choose license: MIT, GPL, BSD, etc.)
- [ ] Create .gitignore for C++ projects

#### 0.2 Build System Configuration
- [ ] Create root CMakeLists.txt
- [ ] Configure C++17 standard requirement
- [ ] Set up Debug and Release configurations
- [ ] Add compiler warnings (-Wall -Wextra -Wpedantic)
- [ ] Add sanitizers for Debug builds (ASan, UBSan)

#### 0.3 Testing Infrastructure
- [ ] Add GoogleTest via FetchContent
- [ ] Create tests/CMakeLists.txt with subdirectories:
  - tests/unit/ - Unit tests (GoogleTest)
  - tests/integration/ - Integration tests
  - tests/regression/ - Vasm compatibility tests
  - tests/e2e/ - Real-world programs
  - tests/fixtures/ - Shared test data
- [ ] Add BUILD_TESTING option
- [ ] Create test template and example unit test
- [ ] Set up regression test runner (Python script)
- [ ] Import test cases from vasm-ext:
  - SCMASM test suite
  - EDTASM test suite
  - Merlin test suite (when implemented)
- [ ] Create E2E test directories:
  - tests/e2e/apple2/prince_of_persia/
  - tests/e2e/coco/zaxxon/
- [ ] Document testing strategy (see docs/architecture/07-testing-strategy.md)

#### 0.4 Continuous Integration
- [ ] Create .github/workflows/ci.yml (if using GitHub)
- [ ] Configure builds for Linux, macOS, Windows
- [ ] Run tests on all platforms
- [ ] Code coverage reporting (optional: codecov.io)

#### 0.5 Development Tools
- [ ] Add .clang-format configuration
- [ ] Add .clang-tidy configuration (optional)
- [ ] Create scripts/format.sh for code formatting
- [ ] Create scripts/test.sh for test runner

### Deliverables

- ✅ Working build system (CMake)
- ✅ CI pipeline running tests
- ✅ Project documentation structure
- ✅ Development environment configured

### Acceptance Criteria

```
□ CMake builds successfully on Linux, macOS, Windows
□ Sample test compiles and runs
□ CI pipeline passes
□ README documents how to build and test
```

---

## Phase 1: Core Foundation - Minimal Viable Assembler

**Goal:** Build the core engine with minimal CPU and syntax support

**Focus:** One CPU (6502), one syntax (simple), binary output only

**Duration:** 5-8 development sessions

### Tasks

#### 1.1 Core Abstractions

**File:** `src/core/atom.cpp`, `include/xasm++/core/atom.h`

- [ ] Implement `AtomType` enum
- [ ] Implement `Atom` base class
- [ ] Implement concrete atom types:
  - `LabelAtom` - Symbol definition
  - `InstructionAtom` - CPU instruction
  - `DataAtom` - Raw data bytes
  - `SpaceAtom` - Reserved space
  - `AlignAtom` - Alignment directive
  - `OrgAtom` - Origin directive
- [ ] Write unit tests for all atom types

**File:** `src/core/expression.cpp`, `include/xasm++/core/expression.h`

- [ ] Implement `Expression` base class
- [ ] Implement expression types:
  - `LiteralExpr` - Numeric literal
  - `SymbolExpr` - Symbol reference
  - `BinaryOpExpr` - Binary operation (+, -, *, /, etc.)
  - `UnaryOpExpr` - Unary operation (-, ~, etc.)
- [ ] Implement `Evaluate()` method with symbol table
- [ ] Write unit tests for expression evaluation

**File:** `src/core/symbol.cpp`, `include/xasm++/core/symbol.h`

- [ ] Implement `SymbolType` enum
- [ ] Implement `Symbol` class
- [ ] Implement `SymbolTable` class:
  - Hash table (std::unordered_map)
  - Insert, lookup, update operations
  - Case-sensitivity option
  - Scope management (global, local)
- [ ] Write unit tests for symbol table operations

**File:** `src/core/section.cpp`, `include/xasm++/core/section.h`

- [ ] Implement `SectionAttributes` enum
- [ ] Implement `Section` class
- [ ] Implement atom chain management
- [ ] Write unit tests for section management

#### 1.2 Simple Assembler Engine

**File:** `src/core/assembler.cpp`, `include/xasm++/core/assembler.h`

- [ ] Implement `Assembler` class:
  - Constructor with CPU and syntax plugins
  - `AssembleFile()` method
  - Multi-pass resolution loop
  - Size change tracking
  - Error accumulation
- [ ] Implement pass management:
  - Initial pass (parsing)
  - Fast optimization phase (passes 2-50)
  - Safe optimization phase (passes 51-500)
  - Final pass (output generation)
- [ ] Write unit tests for assembler logic

#### 1.3 Minimal CPU Plugin (6502)

**File:** `src/cpu/cpu_plugin.cpp`, `include/xasm++/cpu/cpu_plugin.h`

- [ ] Define `CpuPlugin` interface
- [ ] Define `AddressingMode` enum
- [ ] Define `InstructionAtom` structure

**File:** `src/cpu/cpu_registry.cpp`, `include/xasm++/cpu/cpu_registry.h`

- [ ] Implement `CpuRegistry` singleton
- [ ] Implement `Register()` method
- [ ] Implement `Create()` factory method
- [ ] Write unit tests for registry

**File:** `src/cpu/m6502/cpu_6502.cpp`, `include/xasm++/cpu/m6502/cpu_6502.h`

- [ ] Implement `Cpu6502` class
- [ ] Implement subset of 6502 instructions (~20 common opcodes):
  - LDA, STA, LDX, STX, LDY, STY
  - ADC, SBC, AND, ORA, EOR
  - JMP, JSR, RTS, BEQ, BNE, BCC, BCS
  - NOP, CLC, SEC
- [ ] Implement addressing modes:
  - Immediate (#$00)
  - Absolute ($1234)
  - Zero Page ($00)
  - Relative (branches)
- [ ] Implement `CalculateSize()` for size determination
- [ ] Implement `Encode()` for instruction encoding
- [ ] Write unit tests for each instruction and addressing mode

**File:** `src/cpu/m6502/opcodes_6502.h`

- [ ] Create opcode table with encoding information
- [ ] Document opcodes with comments

#### 1.4 Simple Syntax Plugin

**File:** `src/syntax/syntax_plugin.cpp`, `include/xasm++/syntax/syntax_plugin.h`

- [ ] Define `SyntaxPlugin` interface
- [ ] Define `ParserState` structure

**File:** `src/syntax/syntax_registry.cpp`, `include/xasm++/syntax/syntax_registry.h`

- [ ] Implement `SyntaxRegistry` singleton
- [ ] Implement `Register()` method
- [ ] Implement `Create()` factory method
- [ ] Write unit tests for registry

**File:** `src/syntax/simple/simple_syntax.cpp`, `include/xasm++/syntax/simple/simple_syntax.h`

- [ ] Implement `SimpleSyntax` parser
- [ ] Support basic directives:
  - `.org` - Set origin
  - `.db`/`.byte` - Define byte(s)
  - `.dw`/`.word` - Define word(s)
  - `.ds`/`.space` - Reserve space
  - `.align` - Align to boundary
- [ ] Support label definitions (NAME:)
- [ ] Support instructions with operands
- [ ] Support comments (; to end of line)
- [ ] Support numeric literals:
  - Decimal (123)
  - Hexadecimal ($1234, 0x1234)
  - Binary (%11010010, 0b11010010)
- [ ] Write unit tests for lexer and parser

#### 1.5 Binary Output Plugin

**File:** `src/output/output_plugin.cpp`, `include/xasm++/output/output_plugin.h`

- [ ] Define `OutputPlugin` interface

**File:** `src/output/output_registry.cpp`, `include/xasm++/output/output_registry.h`

- [ ] Implement `OutputRegistry` singleton

**File:** `src/output/binary_output.cpp`, `include/xasm++/output/binary_output.h`

- [ ] Implement `BinaryOutput` plugin
- [ ] Serialize atom chain to binary file
- [ ] Write unit tests

#### 1.6 Command-Line Interface

**File:** `src/main.cpp`

- [ ] Add CLI11 via FetchContent
- [ ] Implement argument parsing:
  - `--cpu <name>` - CPU selection
  - `--syntax <name>` - Syntax selection
  - `--output <file>` - Output filename
  - `input.asm` - Input file
- [ ] Implement assembler invocation
- [ ] Implement error reporting
- [ ] Return appropriate exit codes

#### 1.7 Integration Testing

**File:** `tests/integration/test_minimal_assembly.cpp`

- [ ] Create simple .asm test files
- [ ] Verify assembly produces correct binary output
- [ ] Compare against known-good binaries (from vasm)
- [ ] Test error conditions (undefined symbols, invalid instructions)

### Deliverables

- ✅ Working assembler for subset of 6502 instructions
- ✅ Simple syntax parser
- ✅ Binary output generation
- ✅ Command-line tool
- ✅ ~85% test coverage

### Acceptance Criteria

```
□ Can assemble simple 6502 programs (LDA, STA, JMP, BEQ, etc.)
□ Handles forward references (multi-pass resolution)
□ Generates correct binary output
□ All unit tests passing
□ Integration tests passing
□ Code coverage ≥ 85%
□ Compiles on Linux, macOS, Windows
```

### Example Usage

```bash
$ xasm++ --cpu 6502 --syntax simple --output test.bin test.asm
xasm++ v0.1.0
Assembling test.asm...
Pass 1: Parsing
Pass 2: Resolving symbols
Pass 3: Final pass
Output written to test.bin (256 bytes)
Assembly complete: 0 errors, 0 warnings
```

---

## Phase 2: Complete 6502 Support, MOS Base Syntax, and Apple II Output

**Goal:** Full 6502 instruction set, MOS base syntax, SCMASM platform syntax, and Apple II output formats

**Focus:** Implement base classes for code reuse, then extend with platform-specific features

**Duration:** 5-7 development sessions

### Tasks

#### 2.1 Complete 6502 CPU Plugin

**File:** `src/cpu/m6502/cpu_6502.cpp`

- [ ] Add remaining 6502 instructions (~40 more):
  - All load/store: LDA, STA, LDX, STX, LDY, STY
  - All arithmetic: ADC, SBC, INC, DEC, INX, DEX, INY, DEY
  - All logic: AND, ORA, EOR, BIT
  - All shifts: ASL, LSR, ROL, ROR
  - All branches: BEQ, BNE, BCC, BCS, BMI, BPL, BVC, BVS
  - All jumps: JMP, JSR, RTS, RTI, BRK
  - All flags: CLC, SEC, CLD, SED, CLI, SEI, CLV
  - Stack: PHA, PLA, PHP, PLP, TSX, TXS
  - Transfer: TAX, TAY, TXA, TYA
  - Misc: NOP

- [ ] Add all addressing modes:
  - Implied
  - Accumulator (ASL A)
  - Immediate (#$00)
  - Zero Page ($00)
  - Zero Page,X ($00,X)
  - Zero Page,Y ($00,Y)
  - Absolute ($1234)
  - Absolute,X ($1234,X)
  - Absolute,Y ($1234,Y)
  - Indirect (($1234)) - JMP only
  - Indexed Indirect (($00,X))
  - Indirect Indexed (($00),Y)
  - Relative (branches)

- [ ] Implement zero-page optimization:
  - Detect when absolute address fits in zero page
  - Use shorter zero-page opcode if available
  - Track optimization in multi-pass resolution

- [ ] Write comprehensive unit tests for all opcodes

#### 2.2 MOS Base Syntax Plugin

**File:** `src/syntax/mos_base_syntax.cpp`, `include/xasm++/syntax/mos_base_syntax.h`

- [ ] Implement `MosBaseSyntax` base class (for code reuse)
- [ ] Common 6502/65C02/65816 addressing mode parsing
- [ ] Common expression parser (hex $, binary %, decimal)
- [ ] Common directives (.ORG, .EQU, .DB/.BYTE, .DW/.WORD, .DS/.SPACE)
- [ ] Virtual hooks for platform-specific extensions
- [ ] Write unit tests for base functionality

#### 2.3 MOS Generic Syntax Plugin

**File:** `src/syntax/mos/mos_syntax.cpp`, `include/xasm++/syntax/mos/mos_syntax.h`

- [ ] Implement `MosSyntax` class (extends MosBaseSyntax)
- [ ] Minimal generic 6502 syntax (no platform-specific features)
- [ ] Write unit tests

#### 2.4 SCMASM Syntax Plugin

**File:** `src/syntax/scmasm/scmasm_syntax.cpp`, `include/xasm++/syntax/scmasm/scmasm_syntax.h`

- [ ] Implement SCMASM parser (based on vasm-ext scmasm)
- [ ] Support SCMASM directives:
  - `.ORG` - Set origin
  - `.EQ` / `.EQU` - Define constant
  - `.DA` / `.DW` - Define address/word
  - `.DB` / `.DFB` - Define byte(s)
  - `.DS` - Define space
  - `.ASC` / `.AST` - Define ASCII string
  - `.HEX` - Define hex data
  - `.END` - End of assembly
- [ ] Support label types:
  - Global labels (LABEL)
  - Local labels (]LABEL or :LABEL)
  - Anonymous labels (:N, .N)
- [ ] Support expressions:
  - Arithmetic: +, -, *, /, %
  - Bitwise: &, |, ^, ~
  - Shift: <<, >>
  - Comparison: <, >, <=, >=, ==, !=
  - Special: <, >, ^ (lo/hi/bank byte)
- [ ] Support character literals ('A', $27)
- [ ] Support string literals ("text")
- [ ] Write unit tests for parser

#### 2.5 Enhanced Expression Evaluator

**File:** `src/core/expression.cpp`

- [ ] Add operator precedence handling
- [ ] Add lo/hi/bank byte operators
- [ ] Add expression simplification
- [ ] Support relocatable expressions
- [ ] Write unit tests for complex expressions

#### 2.6 Symbol Table Enhancements

**File:** `src/core/symbol.cpp`

- [ ] Add local symbol scoping
- [ ] Add symbol export/import (for future linker support)
- [ ] Add symbol cross-reference tracking
- [ ] Write unit tests

#### 2.7 Apple II Output Formats

**File:** `src/output/binary_output_base.cpp`, `include/xasm++/output/binary_output_base.h`

- [ ] Implement `BinaryOutputBase` base class (for code reuse)
- [ ] Common methods: WriteBytes, WriteWord, CollectDataBlocks
- [ ] Big-endian vs little-endian helpers

**File:** `src/output/apple2/dos33_output.cpp`, `include/xasm++/output/apple2/dos33_output.h`

- [ ] Implement DOS 3.3 binary format
  - Load address (2 bytes, little-endian)
  - Length (2 bytes, little-endian)
  - Data bytes
- [ ] Write unit tests with known-good binaries

**File:** `src/output/apple2/prodos_output.cpp`, `include/xasm++/output/apple2/prodos_output.h`

- [ ] Implement ProDOS SYS format
  - System file type
  - Load address handling
  - Execution address
- [ ] Write unit tests

**File:** `src/output/apple2/applesingle_output.cpp`, `include/xasm++/output/apple2/applesingle_output.h`

- [ ] Implement AppleSingle format (for modern file transfers)
  - Magic number
  - Version
  - Data fork with load address metadata
- [ ] Write unit tests

#### 2.8 Integration Testing

**File:** `tests/integration/test_scmasm.cpp`

- [ ] Port SCMASM test suite from vasm-ext
- [ ] Verify compatibility with SCMASM syntax
- [ ] Test all directives and expressions
- [ ] Compare output against vasm-generated binaries

**File:** `tests/regression/scmasm/`

- [ ] Run all regression tests against vasm output
- [ ] Verify byte-for-byte binary compatibility
- [ ] Test symbol table output

**File:** `tests/e2e/apple2/prince_of_persia/`

- [ ] Import Prince of Persia source code
- [ ] Assemble all binary components:
  - Main engine
  - Level data
  - Graphics data
  - Animation sequences
- [ ] Build complete disk image
- [ ] Compare against known-good build (vasm-ext)
- [ ] Validate in AppleWin/MAME emulator:
  - Game loads without errors
  - All levels playable
  - No visual glitches
  - Gameplay identical to original

### Deliverables

- ✅ Complete 6502 instruction set (all 56 legal opcodes)
- ✅ MosBaseSyntax base class (for code reuse)
- ✅ MOS generic syntax plugin
- ✅ SCMASM platform syntax support
- ✅ Enhanced expression evaluator
- ✅ Local symbol scoping
- ✅ Apple II output formats (DOS 3.3, ProDOS, AppleSingle)

### Acceptance Criteria

```
□ All 56 legal 6502 opcodes implemented
□ All 13 addressing modes supported
□ Zero-page optimization working
□ MOS base syntax provides 70% code reuse for SCMASM
□ SCMASM syntax fully functional
□ DOS 3.3 binaries load correctly on Apple II
□ ProDOS SYS files execute correctly
□ Can assemble real-world SCMASM programs
□ All unit tests passing (90%+ coverage for core)
□ Integration tests passing against vasm output
□ Regression tests: 100% byte-for-byte compatibility with vasm
□ E2E test: Prince of Persia assembles and runs correctly
□ Code coverage ≥ 85% overall
```

---

## Phase 3: Motorola Family and CoCo Platform Support

**Goal:** Add 6809 and 65C02 CPUs, Motorola base syntax, EDTASM platform syntax, and CoCo output formats (including OS-9 modules)

**Focus:** Validate plugin architecture with different CPU families, implement complex output formats

**Duration:** 8-10 development sessions

### Tasks

#### 3.1 65C02 CPU Plugin

**File:** `src/cpu/m6502/cpu_65c02.cpp`, `include/xasm++/cpu/m6502/cpu_65c02.h`

- [ ] Implement `Cpu65C02` class (extends/shares code with `Cpu6502`)
- [ ] Add 65C02-specific instructions:
  - BRA (relative branch always)
  - PHX, PHY, PLX, PLY (push/pull X/Y)
  - STZ (store zero)
  - TRB, TSB (test and reset/set bits)
  - BBR, BBS (bit branch)
  - RMB, SMB (reset/set memory bit)
  - WAI, STP (wait/stop)
- [ ] Add 65C02 addressing mode fixes:
  - JMP ($1234,X) - indexed indirect
  - BIT #$00 - immediate mode
- [ ] Register in `CpuRegistry`
- [ ] Write unit tests

**File:** `tests/integration/test_65c02.cpp`

- [ ] Create test suite for 65C02-specific features
- [ ] Verify correct opcode encoding

#### 3.2 6809 CPU Plugin

**File:** `src/cpu/m6809/cpu_6809.cpp`, `include/xasm++/cpu/m6809/cpu_6809.h`

- [ ] Implement `Cpu6809` class
- [ ] Implement 6809 instruction set (~60 instructions):
  - 8-bit loads: LDA, LDB
  - 16-bit loads: LDD, LDX, LDY, LDS, LDU
  - Stores: STA, STB, STD, STX, STY, STS, STU
  - Arithmetic: ADDA, ADDB, ADDD, SUBA, SUBB, SUBD
  - Logic: ANDA, ANDB, ORA, ORB, EORA, EORB
  - Branches: BEQ, BNE, BCC, BCS, BMI, BPL, BVC, BVS, BRA
  - Long branches: LBEQ, LBNE, LBCC, LBCS, LBMI, LBPL, LBVC, LBVS, LBRA
  - Jumps: JMP, JSR, BSR, RTS, RTI
  - Stack: PSHS, PSHU, PULS, PULU
  - Transfers: TFR, EXG
  - Misc: NOP, SYNC, CWAI, SWI, SWI2, SWI3
  - ... (full instruction set)

- [ ] Implement 6809 addressing modes:
  - Inherent
  - Immediate (8-bit, 16-bit)
  - Direct (8-bit offset)
  - Extended (16-bit address)
  - Indexed (complex: 16 modes!)
  - Relative (8-bit, 16-bit)
  - PCR (PC-relative)

- [ ] Implement indexed mode parser (most complex):
  - ,R (zero offset)
  - n,R (constant offset: 5-bit, 8-bit, 16-bit)
  - A,R / B,R / D,R (accumulator offset)
  - ,R+ / ,R++ (post-increment)
  - ,-R / ,--R (pre-decrement)
  - [n,R] (indirect variants)
  - [,R++], [,--R] (indirect autoincrement)

- [ ] Register in `CpuRegistry`
- [ ] Write extensive unit tests (6809 is complex!)

**File:** `tests/integration/test_6809.cpp`

- [ ] Create comprehensive 6809 test suite
- [ ] Test all indexed modes
- [ ] Compare against vasm 6809 output

#### 3.3 Motorola Base Syntax Plugin

**File:** `src/syntax/motorola_base_syntax.cpp`, `include/xasm++/syntax/motorola_base_syntax.h`

- [ ] Implement `MotorolaBaseSyntax` base class (for code reuse)
- [ ] Common Motorola addressing mode parsing (# for immediate, etc.)
- [ ] Common directives (ORG, EQU, FCB, FDB, FCC, RMB)
- [ ] Motorola expression syntax
- [ ] Virtual hooks for platform-specific extensions
- [ ] Write unit tests for base functionality

#### 3.4 Motorola Generic Syntax Plugin

**File:** `src/syntax/motorola/motorola_syntax.cpp`, `include/xasm++/syntax/motorola/motorola_syntax.h`

- [ ] Implement `MotorolaSyntax` class (extends MotorolaBaseSyntax)
- [ ] Generic Motorola syntax (works for 6800, 6809, 68000)
- [ ] Write unit tests

#### 3.5 EDTASM Syntax Plugin

**File:** `src/syntax/edtasm/edtasm_syntax.cpp`, `include/xasm++/syntax/edtasm/edtasm_syntax.h`

- [ ] Implement EDTASM parser (Tandy Color Computer assembler syntax)
- [ ] Support EDTASM directives:
  - `ORG` - Set origin
  - `EQU` - Define constant
  - `FCB` - Form constant byte
  - `FDB` - Form double byte (word)
  - `FCC` - Form constant character
  - `RMB` - Reserve memory bytes
  - `END` - End of assembly
- [ ] Support EDTASM label format (no colon)
- [ ] Support EDTASM expressions (similar to SCMASM)
- [ ] Register in `SyntaxRegistry`
- [ ] Write unit tests

**File:** `tests/integration/test_edtasm.cpp`

- [ ] Port EDTASM test suite from vasm-ext
- [ ] Verify compatibility

#### 3.6 CoCo Output Formats

**File:** `src/output/coco/decb_output.cpp`, `include/xasm++/output/coco/decb_output.h`

- [ ] Implement DECB binary format (Disk Extended Color BASIC)
  - Preamble: $00 $FF
  - Data blocks:
    - $00 (block type)
    - Length (2 bytes, big-endian)
    - Load address (2 bytes, big-endian)
    - Data bytes
  - Postamble:
    - $FF (end block)
    - $00 $00 (no more data)
    - Execution address (2 bytes, big-endian)
- [ ] Handle multiple sections (separate blocks)
- [ ] Determine execution address from END directive or entry symbol
- [ ] Write unit tests with known-good CoCo binaries

**File:** `src/output/coco/os9_output.cpp`, `include/xasm++/output/coco/os9_output.h`

- [ ] Implement OS-9 module format
  - Module header:
    - Sync bytes ($87 $CD)
    - Module size (2 bytes)
    - Module name offset
    - Type/language byte
    - Attribute/revision byte
    - Header parity (checksum)
  - Module body (code/data)
  - Module CRC (3 bytes)
  - Module name (null-terminated string)
- [ ] Support module types: program, subroutine, multi-module, data
- [ ] Calculate and insert CRC
- [ ] Write unit tests

#### 3.7 Integration Testing

**File:** `tests/integration/test_edtasm.cpp`

- [ ] Port EDTASM test suite from vasm-ext
- [ ] Verify compatibility with EDTASM syntax
- [ ] Test all 6809 addressing modes
- [ ] Compare output against vasm-generated binaries

**File:** `tests/integration/test_coco_output.cpp`

- [ ] Test DECB format loads correctly in MAME CoCo emulator
- [ ] Test OS-9 modules load correctly in OS-9 emulator
- [ ] Verify CRC calculation for OS-9 modules
- [ ] Compare against vasm-generated binaries

**File:** `tests/regression/edtasm/`

- [ ] Run all regression tests against vasm output
- [ ] Verify byte-for-byte binary compatibility

**File:** `tests/e2e/coco/zaxxon/`

- [ ] Import Zaxxon disassembly source
- [ ] Assemble complete game binary
- [ ] Compare against known-good build (vasm-ext)
- [ ] Verify DECB format structure:
  - Correct preamble ($00 $FF)
  - Valid load addresses
  - Proper postamble with execution address
- [ ] Validate in MAME CoCo emulator:
  - Game loads without errors
  - Title screen displays correctly
  - All game screens render properly
  - Gameplay identical to original
  - Sound/music works correctly

### Deliverables

- ✅ 65C02 CPU support (extends 6502 via inheritance)
- ✅ 6809 CPU support (complex indexed modes)
- ✅ MotorolaBaseSyntax base class (for code reuse)
- ✅ Motorola generic syntax
- ✅ EDTASM platform syntax support
- ✅ CoCo output formats (DECB and OS-9 modules)
- ✅ Validated plugin architecture (added 2 CPUs, 3 syntaxes with minimal core changes)

### Acceptance Criteria

```
□ 65C02 fully functional (27 new instructions beyond 6502)
□ 6809 fully functional (all 16 indexed addressing modes)
□ Motorola base syntax provides 70% code reuse for EDTASM
□ EDTASM syntax working (CoCo platform)
□ DECB binaries load and execute correctly on CoCo
□ OS-9 modules pass CRC validation and load correctly
□ Can assemble real 6809 programs (e.g., CoCo games)
□ Plugin architecture validated (added 2 CPUs, 3 syntaxes, 2 output formats)
□ All unit tests passing (90%+ coverage for core)
□ Integration tests passing (against vasm and real hardware/emulators)
□ Regression tests: 100% byte-for-byte compatibility with vasm
□ E2E test: Zaxxon assembles and runs correctly in MAME
□ Code coverage ≥ 85% overall
```

---

## Phase 4: Z80 CPU, Zilog Base Syntax, and EDTASM-M80

**Goal:** Add Z80 CPU support with Zilog base syntax for code reuse, EDTASM-M80 platform syntax, and TRS-80/CP/M output formats

**Focus:** Implement 8080 base class for future reuse, extend to Z80, validate Intel/Zilog family architecture

**Duration:** 8-10 development sessions

### Tasks

#### 4.1 8080 Base CPU Plugin

**File:** `src/cpu/intel/cpu_8080_base.cpp`, `include/xasm++/cpu/intel/cpu_8080_base.h`

- [ ] Implement `Cpu8080Base` base class (for code reuse with 8080/8085)
- [ ] Common 8080 family instruction encoding
- [ ] Base addressing modes (register, register pair, immediate, extended, indirect)
- [ ] Virtual hooks for CPU-specific extensions
- [ ] Write unit tests for base functionality

**Code Reuse Strategy:**
- `Cpu8080Base` provides 95% of 8080 implementation
- `CpuZ80` extends base with Z80-specific instructions
- Target: 60% code reuse between 8080 and Z80

#### 4.2 Z80 CPU Plugin

**File:** `src/cpu/z80/cpu_z80.cpp`, `include/xasm++/cpu/z80/cpu_z80.h`

- [ ] Implement `CpuZ80` class (extends `Cpu8080Base`)
- [ ] Add 8080 instruction set (~78 base instructions):
  - 8-bit loads: MOV r,r / MVI r,data / LDA/STA
  - 16-bit loads: LXI rp,data / LHLD/SHLD / SPHL
  - Arithmetic: ADD, ADC, SUB, SBB, INR, DCR, INX, DCX, DAD
  - Logic: ANA, XRA, ORA, CMP
  - Rotate: RLC, RRC, RAL, RAR
  - Jumps: JMP, Jcc (JZ, JNZ, JC, JNC, JP, JM, JPE, JPO)
  - Calls: CALL, Ccc, RET, Rcc, RST
  - Stack: PUSH, POP
  - I/O: IN, OUT
  - Misc: NOP, HLT, DI, EI, XCHG, XTHL, PCHL, DAA, CMA, STC, CMC

- [ ] Add Z80-specific instructions (~80 additional):
  - Enhanced loads: LD r,r / LD r,(HL) / LD r,(IX+d) / LD r,(IY+d)
  - New loads: LD I,A / LD A,I / LD R,A / LD A,R
  - Arithmetic: ADC HL,rp / SBC HL,rp / NEG
  - Logic variants with IX/IY
  - Bit operations: BIT b,r / SET b,r / RES b,r
  - Rotate/Shift: RLC, RRC, RL, RR, SLA, SRA, SRL (register and indirect)
  - Block operations: LDI, LDIR, LDD, LDDR, CPI, CPIR, CPD, CPDR
  - Block I/O: INI, INIR, IND, INDR, OUTI, OTIR, OUTD, OTDR
  - Relative jumps: JR, JR cc (JR Z, JR NZ, JR C, JR NC)
  - Misc: DJNZ, IM 0/1/2, RETI, RETN, EXX, EX AF,AF'

- [ ] Implement Z80 addressing modes:
  - Register (A, B, C, D, E, H, L)
  - Register pair (BC, DE, HL, SP, AF)
  - Index registers (IX, IY)
  - Immediate (n, nn)
  - Indirect register ((BC), (DE), (HL))
  - Indexed ((IX+d), (IY+d))
  - Extended ((nn))
  - Bit number (0-7)
  - Relative (DJNZ, JR)

- [ ] Register in `CpuRegistry`
- [ ] Write extensive unit tests (Z80 has 158 instructions with variants!)

**File:** `tests/integration/test_z80.cpp`

- [ ] Create comprehensive Z80 test suite
- [ ] Test IX/IY indexed modes
- [ ] Test bit operations
- [ ] Test block operations
- [ ] Compare against vasm z80 output

#### 4.3 Zilog Base Syntax Plugin

**File:** `src/syntax/zilog_base_syntax.cpp`, `include/xasm++/syntax/zilog_base_syntax.h`

- [ ] Implement `ZilogBaseSyntax` base class (for code reuse)
- [ ] Common Z80/8080/8085 addressing mode parsing
- [ ] Common Zilog expression syntax
- [ ] Common directives (ORG, EQU, DB/DEFB, DW/DEFW, DS/DEFS, INCLUDE)
- [ ] Virtual hooks for platform-specific extensions
- [ ] Write unit tests for base functionality

**Code Reuse Strategy:**
- `ZilogBaseSyntax` provides 70% of EDTASM-M80 implementation
- Template method pattern for directive handling
- Similar to MosBaseSyntax and MotorolaBaseSyntax patterns

#### 4.4 Zilog Generic Syntax Plugin

**File:** `src/syntax/zilog/zilog_syntax.cpp`, `include/xasm++/syntax/zilog/zilog_syntax.h`

- [ ] Implement `ZilogSyntax` class (extends ZilogBaseSyntax)
- [ ] Generic Zilog syntax (works for Z80, 8080, 8085)
- [ ] Standard Zilog mnemonics (LD, ADD, SUB, etc.)
- [ ] Write unit tests

#### 4.5 EDTASM-M80 Syntax Plugin

**File:** `src/syntax/edtasm_m80/edtasm_m80_syntax.cpp`, `include/xasm++/syntax/edtasm_m80/edtasm_m80_syntax.h`

- [ ] Implement EDTASM-M80 parser (M80-compatible syntax for Z80)
- [ ] Support EDTASM-M80 directives:
  - `ORG` - Set origin
  - `EQU` - Define constant
  - `DB` / `DEFB` - Define byte(s)
  - `DW` / `DEFW` - Define word(s)
  - `DS` / `DEFS` - Define space
  - `INCLUDE` - Include file
  - `END` - End of assembly
- [ ] Support M80-compatible label format
- [ ] Support M80 expression syntax ($ for hex, & for octal)
- [ ] Register in `SyntaxRegistry`
- [ ] Write unit tests

**File:** `tests/integration/test_edtasm_m80.cpp`

- [ ] Port EDTASM-M80 test suite from vasm-ext
- [ ] Verify M80 compatibility
- [ ] Test all Z80 addressing modes

#### 4.6 TRS-80 and CP/M Output Formats

**File:** `src/output/trs80/cmd_output.cpp`, `include/xasm++/output/trs80/cmd_output.h`

- [ ] Implement CMD format (TRS-80 Model I/III/4 executable)
  - Load module header:
    - $01 (object code)
    - $00-$FF (load module length)
    - Load address (2 bytes, little-endian)
  - Data blocks
  - Transfer address:
    - $02 (transfer address type)
    - Entry point (2 bytes, little-endian)
- [ ] Handle multiple sections
- [ ] Write unit tests with known-good CMD files

**File:** `src/output/cpm/com_output.cpp`, `include/xasm++/output/cpm/com_output.h`

- [ ] Implement COM format (CP/M executable)
  - Flat binary format
  - Load address: $0100 (standard CP/M TPA)
  - No header, just raw code
  - Maximum size: ~56KB (to $FFFF)
- [ ] Validate origin is $0100
- [ ] Write unit tests

**File:** `src/output/intel_hex_output.cpp`, `include/xasm++/output/intel_hex_output.h`

- [ ] Implement Intel HEX format (universal format)
  - Record types:
    - $00 - Data record
    - $01 - End of file
    - $02 - Extended segment address
    - $04 - Extended linear address
  - Checksum calculation
  - Multi-section support
- [ ] Write unit tests

#### 4.7 Integration Testing

**File:** `tests/integration/test_z80_output.cpp`

- [ ] Test CMD format loads correctly in TRS-80 emulator (trs80gp)
- [ ] Test COM format loads correctly in CP/M emulator (MAME)
- [ ] Verify Intel HEX parsing by other tools
- [ ] Compare against vasm-generated binaries

**File:** `tests/regression/edtasm_m80/`

- [ ] Run all regression tests against vasm output
- [ ] Verify byte-for-byte binary compatibility
- [ ] Test M80 compatibility

**File:** `tests/e2e/trs80/` (optional, if suitable program available)

- [ ] Import TRS-80 program source (if available)
- [ ] Assemble complete binary
- [ ] Validate in trs80gp emulator

### Deliverables

- ✅ 8080 base class (for future 8080/8085 implementation)
- ✅ Z80 CPU support (158 instructions including variants)
- ✅ ZilogBaseSyntax base class (for code reuse)
- ✅ Zilog generic syntax
- ✅ EDTASM-M80 platform syntax support
- ✅ TRS-80 CMD output format
- ✅ CP/M COM output format
- ✅ Intel HEX output format (universal)
- ✅ Validated Intel/Zilog family architecture

### Acceptance Criteria

```
□ 8080 base class provides 95% reuse for 8080/8085
□ Z80 fully functional (all 158 instructions)
□ Z80 extends 8080 base with 60% code reuse
□ Zilog base syntax provides 70% code reuse for EDTASM-M80
□ EDTASM-M80 syntax working (M80-compatible)
□ CMD binaries load and execute correctly on TRS-80
□ COM binaries load and execute correctly in CP/M
□ Intel HEX format parseable by external tools
□ Can assemble real Z80 programs
□ All unit tests passing (90%+ coverage for Z80)
□ Integration tests passing (against vasm and emulators)
□ Regression tests: 100% byte-for-byte compatibility with vasm
□ Code coverage ≥ 85% overall
```

---

## Phase 5: 68000 CPU and Amiga/Atari Outputs

**Goal:** Add 68000 CPU support with Amiga/Atari platform output formats

**Focus:** Complex 68000 architecture with varied addressing modes and size variants, Motorola S-Record format

**Duration:** 6-8 development sessions

**Note:** Motorola base syntax already implemented in Phase 3, can be reused here

### Tasks

#### 5.1 68000 CPU Plugin

**File:** `src/cpu/m68k/cpu_68000.cpp`, `include/xasm++/cpu/m68k/cpu_68000.h`

- [ ] Implement `Cpu68000` class
- [ ] Implement 68000 instruction set (~56 base instructions):
  - Data movement: MOVE, MOVEA, MOVEM, MOVEP, LEA, PEA, EXG, SWAP
  - Integer arithmetic: ADD, ADDA, ADDI, ADDQ, ADDX, SUB, SUBA, SUBI, SUBQ, SUBX
  - Multiply/Divide: MULS, MULU, DIVS, DIVU
  - Logic: AND, ANDI, OR, ORI, EOR, EORI, NOT
  - Shifts/Rotates: ASL, ASR, LSL, LSR, ROL, ROR, ROXL, ROXR
  - Bit operations: BSET, BCLR, BCHG, BTST
  - Branches: BRA, Bcc (BEQ, BNE, BCC, BCS, BGE, BLT, BGT, BLE, BHI, BLS, BPL, BMI, BVC, BVS)
  - Jumps: JMP, JSR, RTS, RTR, RTE, BSR
  - Decrement and branch: DBcc (DBRA, DBF, DBT, etc.)
  - Set conditionally: Scc (SEQ, SNE, SGE, SLT, etc.)
  - Test and set: TAS
  - Stack: LINK, UNLK
  - Arithmetic helpers: CLR, NEG, NEGX, EXT, TST, CMP, CMPA, CMPI, CMPM
  - Condition codes: CHK
  - Misc: NOP, TRAP, TRAPV, ILLEGAL, RESET, STOP
  - ... (full instruction set)

- [ ] Implement 68000 addressing modes:
  - Data register direct (Dn)
  - Address register direct (An)
  - Address register indirect ((An))
  - Address register indirect with postincrement ((An)+)
  - Address register indirect with predecrement (-(An))
  - Address register indirect with displacement (d16,An)
  - Address register indirect with index (d8,An,Xn)
  - Absolute short ($XXXX.W)
  - Absolute long ($XXXXXXXX.L)
  - PC-relative with displacement (d16,PC)
  - PC-relative with index (d8,PC,Xn)
  - Immediate (#$XX)
  - Status register (SR)
  - Condition code register (CCR)

- [ ] Implement instruction size variants (.B, .W, .L)
- [ ] Implement addressing mode validation per instruction
- [ ] Register in `CpuRegistry`
- [ ] Write extensive unit tests (68000 has complex mode restrictions!)

**File:** `tests/integration/test_68000.cpp`

- [ ] Create 68000 test suite
- [ ] Test all addressing modes
- [ ] Test size variants
- [ ] Test mode restrictions (e.g., MOVEA only .W/.L)
- [ ] Compare against vasm 68000 output

#### 5.2 Motorola S-Record Output

**File:** `src/output/motorola_srec_output.cpp`, `include/xasm++/output/motorola_srec_output.h`

- [ ] Implement Motorola S-Record format (universal Motorola format)
  - Record types:
    - S0 - Header record (optional)
    - S1 - Data record (16-bit address)
    - S2 - Data record (24-bit address)
    - S3 - Data record (32-bit address)
    - S5 - Count record (16-bit)
    - S7 - Start address (32-bit, terminator)
    - S8 - Start address (24-bit, terminator)
    - S9 - Start address (16-bit, terminator)
  - Checksum calculation
  - Multi-section support
  - Choose S1/S2/S3 based on address range
- [ ] Write unit tests

#### 5.3 Amiga Hunk Format (Optional/Complex)

**File:** `src/output/amiga/hunk_output.cpp`, `include/xasm++/output/amiga/hunk_output.h`

**Note:** Hunk format is complex and may be deferred to later phase if time-constrained

- [ ] Implement basic Amiga Hunk format (executable)
  - Hunk types:
    - HUNK_HEADER - File header
    - HUNK_CODE - Code section
    - HUNK_DATA - Data section
    - HUNK_BSS - Uninitialized data
    - HUNK_END - End of hunk
  - Section management
  - Relocation support (basic)
- [ ] Write unit tests
- [ ] Test loading in Amiga emulator (FS-UAE or WinUAE)

**Alternative:** If Hunk format too complex, defer to future phase and use S-Record for Amiga in Phase 5

#### 5.4 Atari ST TOS Format (Optional)

**File:** `src/output/atari/tos_output.cpp`, `include/xasm++/output/atari/tos_output.h`

**Note:** TOS format is less critical, may be deferred

- [ ] Implement TOS executable format
  - TOS header:
    - Magic number ($601A)
    - Text/data/BSS sizes
    - Symbol table size
    - Flags (relocation, etc.)
  - Text segment
  - Data segment
  - Relocation table (optional)
- [ ] Write unit tests
- [ ] Test loading in Atari ST emulator (Hatari)

**Alternative:** Use S-Record format for Atari ST in Phase 5, defer TOS to future phase

#### 5.5 Integration Testing

**File:** `tests/integration/test_68000_output.cpp`

- [ ] Test S-Record format parseable by external tools
- [ ] Test Hunk format loads in Amiga emulator (if implemented)
- [ ] Test TOS format loads in Atari ST emulator (if implemented)
- [ ] Compare against vasm-generated binaries

**File:** `tests/regression/motorola/`

- [ ] Run all regression tests against vasm output
- [ ] Verify byte-for-byte binary compatibility
- [ ] Test 68000-specific features

**File:** `tests/e2e/amiga/` (optional, if suitable program available)

- [ ] Import Amiga demo or program source (if available)
- [ ] Assemble complete binary
- [ ] Validate in FS-UAE or WinUAE emulator

### Deliverables

- ✅ 68000 CPU support (full instruction set)
- ✅ Motorola S-Record output (universal format)
- ⚠️ Amiga Hunk format (optional, may be deferred)
- ⚠️ Atari TOS format (optional, may be deferred)
- ✅ Reuse of MotorolaBaseSyntax from Phase 3
- ✅ Validated 68000 architecture

### Acceptance Criteria

```
□ 68000 fully functional (all ~56 base instructions)
□ All 68000 addressing modes working (14 modes)
□ Size variants (.B, .W, .L) implemented
□ Mode restrictions enforced per instruction
□ MotorolaBaseSyntax reused from Phase 3 (no duplication)
□ S-Record format working and parseable by external tools
□ Hunk format working (if implemented) or deferred to future phase
□ TOS format working (if implemented) or deferred to future phase
□ Can assemble real 68000 programs
□ All unit tests passing (90%+ coverage for 68000)
□ Integration tests passing (against vasm and emulators)
□ Regression tests: 100% byte-for-byte compatibility with vasm
□ Code coverage ≥ 85% overall
```

### Phase 5 Scope Decision

**Recommended approach:**
- **Phase 5 Core:** 68000 CPU + Motorola S-Record format
- **Optional:** Amiga Hunk format (if time permits)
- **Defer:** Atari TOS format to future phase (less critical)

This keeps Phase 5 focused and deliverable while maintaining option to add platform-specific formats incrementally.

---

## Phase 6: 65816 CPU, Merlin Syntax, and Apple IIgs OMF Output

**Goal:** Add 65816 CPU support, Merlin platform syntax, and OMF output format for Apple IIgs

**Focus:** 16-bit extension of 6502, extend MosBaseSyntax for code reuse, implement critical OMF format for proper linking

**Duration:** 6-8 development sessions

**Note:** 65816 extends 65C02 which extends 6502 - target 70% code reuse via inheritance

### Tasks

#### 6.1 65816 CPU Plugin

**File:** `src/cpu/m6502/cpu_65816.cpp`, `include/xasm++/cpu/m6502/cpu_65816.h`

- [ ] Implement `Cpu65816` class (extends `Cpu65C02`)
- [ ] Inherit all 6502 and 65C02 instructions (base already implemented)
- [ ] Add 65816-specific instructions:
  - PHB, PLB, PHD, PLD, PHK (push/pull new registers)
  - TCD, TDC, TCS, TSC (transfer instructions for 16-bit registers)
  - XBA (exchange B and A accumulators)
  - XCE (exchange carry and emulation bits)
  - PEA, PEI, PER (push effective address variants)
  - REP, SEP (reset/set processor status bits)
  - COP, BRL, JSL, RTL, MVN, MVP (long addressing and block moves)

- [ ] Add 65816 addressing modes (extends 6502/65C02):
  - Stack relative (sr,S)
  - Stack relative indirect indexed ((sr,S),Y)
  - Direct page indirect long ([dp])
  - Direct page indirect long indexed ([dp],Y)
  - Absolute long (long)
  - Absolute long indexed (long,X)
  - Absolute indirect long ([abs])
  - Block move (source,dest)
  - Program counter relative long (rlabel - 24-bit)

- [ ] Implement 16-bit accumulator and index modes:
  - Track M and X status bits (8-bit vs 16-bit)
  - Automatic opcode size adjustment based on mode
  - REP/SEP directive tracking

- [ ] Implement direct page register tracking:
  - D register affects direct page addressing
  - Bank register (DBR) for long addressing

- [ ] Register in `CpuRegistry`
- [ ] Write extensive unit tests (65816 mode switching is complex!)

**Code Reuse Strategy:**
- `Cpu65816` extends `Cpu65C02` (which extends `Cpu6502Base`)
- Inherits all 6502 opcodes: 56 instructions (100% reuse)
- Inherits all 65C02 extensions: 27 instructions (100% reuse)
- Adds only 65816-specific: ~20 new instructions
- **Target: 70% code reuse** (83/~100 instructions inherited)

**File:** `tests/integration/test_65816.cpp`

- [ ] Create 65816 test suite
- [ ] Test native mode vs emulation mode
- [ ] Test 16-bit accumulator/index modes
- [ ] Test long addressing modes
- [ ] Test block move instructions
- [ ] Compare against vasm 65816 output

#### 6.2 Merlin Syntax Plugin

**File:** `src/syntax/merlin/merlin_syntax.cpp`, `include/xasm++/syntax/merlin/merlin_syntax.h`

- [ ] Implement Merlin parser (extends `MosBaseSyntax` for code reuse)
- [ ] Support Merlin-specific directives:
  - `ORG` - Set origin
  - `EQU` - Define constant
  - `=` - Alternative EQU
  - `DFB` / `DB` - Define byte(s)
  - `DW` / `DA` - Define word/address
  - `ADR` - Define 3-byte address (65816)
  - `ADRL` - Define 4-byte long address (65816)
  - `DDB` - Define double-byte (big-endian)
  - `HEX` - Define hex data
  - `ASC` / `DCI` - ASCII strings (normal and inverted)
  - `STR` - Pascal string (length prefix)
  - `REV` - Reverse string
  - `FLS` - Flashing string
  - `DS` / `DSR` - Define space/reserve
  - `DO` / `ELSE` / `FIN` - Conditional assembly
  - `MAC` / `<<<` / `>>>` / `EOM` - Macro definition
  - `USE` / `PUT` - Include file
  - `SAV` - Save object file
  - `DSK` - Disk name
  - `XC` - Set CPU mode (65816 opcodes)
  - `MX` - Set M and X flags (65816 mode)

- [ ] Support Merlin label format:
  - Labels in first column (no colon)
  - Variables with colon suffix (:VAR)
  - Local labels (:]LOCAL)
  - Global labels (no prefix)

- [ ] Support Merlin expressions:
  - Standard operators (+, -, *, /, %, &, |, ^, ~, <<, >>)
  - String comparisons
  - Special operators: < (low byte), > (high byte), ^ (bank byte)
  - ! (force long addressing)

- [ ] Support Merlin addressing mode hints:
  - < force DP addressing
  - > force absolute addressing
  - ! force long addressing (65816)

- [ ] Register in `SyntaxRegistry`
- [ ] Write unit tests

**Code Reuse Strategy:**
- `MerlinSyntax` extends `MosBaseSyntax`
- Inherits expression parser, addressing mode logic
- Adds Merlin-specific directives and syntax quirks
- **Target: 60% code reuse** from MosBaseSyntax

**File:** `tests/integration/test_merlin.cpp`

- [ ] Port Merlin test suite from vasm-ext (when available)
- [ ] Verify Merlin syntax compatibility
- [ ] Test 65816-specific features (XC, MX, long addressing)

#### 6.3 Apple IIgs OMF Output Format

**File:** `src/output/apple2gs/omf_output.cpp`, `include/xasm++/output/apple2gs/omf_output.h`

**Note:** OMF (Object Module Format) is CRITICAL for Apple IIgs - required for proper linking

- [ ] Implement OMF v2.1 format (Apple IIgs standard)
  - File header:
    - BYTECNT (4 bytes) - Total file size
    - RESSPC (4 bytes) - Reserved space
    - LENGTH (4 bytes) - Record length
    - Records follow
  - Segment records:
    - SEGMENT ($E0) - Segment definition
      - BLKCNT, KIND, LABLEN, NUMLEN, VERSION
      - BANKSIZE, ORG, ALIGN, NUMSEX
      - Segment name (length-prefixed)
    - LCONST ($F2) - Load constant (code/data)
    - END ($00) - End of segment
  - Symbol records:
    - LOCAL ($F5) - Local symbol
    - GLOBAL ($F6) - Global symbol (exported)
    - GEQU ($FB) - Global equate
  - Relocation records:
    - RELOC ($E2) - Relocation expression
    - INTERSEG ($E3) - Intersegment reference
  - Expression evaluator for relocations
  - Strong/weak symbol resolution

- [ ] Support OMF segment types:
  - CODE ($00) - Code segment
  - DATA ($01) - Data segment
  - JUMPTABLE ($02) - Jump table
  - PATHNAME ($04) - Pathname
  - LIBRARY ($08) - Library dictionary
  - INITIALIZATION ($10) - Init segment
  - DIRECT_PAGE ($12) - Direct page/data
  - DYNAMIC ($41) - Dynamic segment

- [ ] Generate proper relocation entries:
  - Track all symbol references
  - Generate RELOC records for forward refs
  - Handle intersegment references

- [ ] Write unit tests with known-good OMF files

**File:** `src/output/apple2gs/s16_output.cpp`, `include/xasm++/output/apple2gs/s16_output.h`

- [ ] Implement GS/OS S16 format (simpler alternative to OMF)
  - Header:
    - Magic bytes
    - Load segment information
  - Single contiguous binary (no relocation)
- [ ] Write unit tests

#### 6.4 Integration Testing

**File:** `tests/integration/test_65816_output.cpp`

- [ ] Test OMF format with Apple IIgs tools:
  - Linkable with Apple Linker
  - Loads correctly in ORCA/M environment
- [ ] Test S16 format loads in GS/OS emulator
- [ ] Compare against vasm-generated binaries

**File:** `tests/regression/merlin/`

- [ ] Run all regression tests against vasm output
- [ ] Verify byte-for-byte binary compatibility
- [ ] Test Merlin-specific features

**File:** `tests/e2e/apple2gs/` (optional, if suitable program available)

- [ ] Import Apple IIgs program source (if available)
- [ ] Assemble to OMF format
- [ ] Link with Apple Linker
- [ ] Validate in GSplus or MAME emulator

### Deliverables

- ✅ 65816 CPU support (extends 65C02 with 70% code reuse)
- ✅ Merlin platform syntax (extends MosBaseSyntax with 60% code reuse)
- ✅ OMF output format (CRITICAL for Apple IIgs linking)
- ✅ GS/OS S16 format (simple alternative)
- ✅ Validated 65816 16-bit extension architecture

### Acceptance Criteria

```
□ 65816 fully functional (all ~100 instructions via inheritance)
□ 65816 extends Cpu65C02 with 70% code reuse
□ Native mode and emulation mode working
□ 16-bit accumulator/index modes working
□ Long addressing modes working (24-bit)
□ Merlin syntax fully functional
□ Merlin extends MosBaseSyntax with 60% code reuse
□ OMF format generates linkable object files
□ OMF files link correctly with Apple IIgs tools
□ S16 format working
□ Can assemble real Apple IIgs programs
□ All unit tests passing (90%+ coverage for 65816)
□ Integration tests passing (against vasm and IIgs tools)
□ Regression tests: 100% byte-for-byte compatibility with vasm
□ Code coverage ≥ 85% overall
```

### Phase 6 Critical Note

**OMF format is ESSENTIAL for Apple IIgs development:**
- Required for proper linking and relocation
- Without OMF, cannot create loadable GS/OS applications
- More complex than other output formats, but necessary
- Reference: Apple IIgs Toolbox Reference, OMF specification

---

## Phase 7: 8080/8085 and 6800 CPU Completion

**Goal:** Complete remaining CPUs: 8080, 8085 (reusing 8080 base from Phase 4), and 6800

**Focus:** High code reuse with existing base classes, Intel base syntax implementation

**Duration:** 5-7 development sessions

**Note:** 8080 base class already implemented in Phase 4 (used by Z80) - reuse for 95% code sharing

### Tasks

#### 7.1 8080 CPU Plugin

**File:** `src/cpu/intel/cpu_8080.cpp`, `include/xasm++/cpu/intel/cpu_8080.h`

- [ ] Implement `Cpu8080` class (uses `Cpu8080Base` from Phase 4)
- [ ] Inherit all 8080 instructions from base class:
  - 8-bit loads: MOV, MVI, LDA, STA, LDAX, STAX, LHLD, SHLD
  - 16-bit loads: LXI, PUSH, POP, SPHL, PCHL, XCHG, XTHL
  - Arithmetic: ADD, ADI, ADC, ACI, SUB, SUI, SBB, SBI
  - Increment/Decrement: INR, DCR, INX, DCX
  - Logic: ANA, ANI, XRA, XRI, ORA, ORI, CMP, CPI
  - Rotate: RLC, RRC, RAL, RAR
  - Jumps: JMP, Jcc (JZ, JNZ, JC, JNC, JP, JM, JPE, JPO)
  - Calls/Returns: CALL, Ccc, RET, Rcc, RST
  - I/O: IN, OUT
  - Stack/misc: DAD, DAA, CMA, CMC, STC
  - Control: NOP, HLT, DI, EI

- [ ] Register in `CpuRegistry`
- [ ] Write unit tests (mostly validating base class integration)

**Code Reuse Strategy:**
- `Cpu8080` uses `Cpu8080Base` implemented in Phase 4
- **95% code reuse** - minimal new code needed
- Primarily configuration and registration

**File:** `tests/integration/test_8080.cpp`

- [ ] Create 8080 test suite
- [ ] Verify all 78 base instructions work
- [ ] Compare against reference 8080 assemblers

#### 7.2 8085 CPU Plugin

**File:** `src/cpu/intel/cpu_8085.cpp`, `include/xasm++/cpu/intel/cpu_8085.h`

- [ ] Implement `Cpu8085` class (extends `Cpu8080`)
- [ ] Inherit all 8080 instructions (100% reuse)
- [ ] Add 8085-specific instructions:
  - RIM - Read interrupt mask
  - SIM - Set interrupt mask

- [ ] Add 8085-specific undocumented instructions (optional):
  - DSUB - Double subtract (HL = HL - BC)
  - ARHL - Arithmetic shift right HL
  - RDEL - Rotate DE left
  - LDHI - Load DE immediate (HL + immediate)
  - LDSI - Load DE immediate (SP + immediate)
  - RSTV - RST on overflow
  - SHLX - Store HL indirect indexed
  - LHLX - Load HL indirect indexed

- [ ] Register in `CpuRegistry`
- [ ] Write unit tests

**Code Reuse Strategy:**
- `Cpu8085` extends `Cpu8080`
- Inherits 78 instructions from 8080
- Adds only 2-8 new instructions
- **95% code reuse** from 8080

**File:** `tests/integration/test_8085.cpp`

- [ ] Create 8085 test suite
- [ ] Test 8085-specific instructions
- [ ] Test undocumented instructions (if implemented)

#### 7.3 Intel Base Syntax Plugin

**File:** `src/syntax/intel_base_syntax.cpp`, `include/xasm++/syntax/intel_base_syntax.h`

- [ ] Implement `IntelBaseSyntax` base class (for code reuse)
- [ ] Common Intel syntax patterns (8080/8085/8086 style)
- [ ] Common directives:
  - `ORG` - Set origin
  - `EQU` - Define constant
  - `DB` - Define byte(s)
  - `DW` - Define word(s)
  - `DS` - Define space
  - `END` - End of assembly
- [ ] Intel expression syntax
- [ ] Intel addressing mode formats
- [ ] Virtual hooks for CPU-specific extensions
- [ ] Write unit tests for base functionality

**Code Reuse Strategy:**
- Similar pattern to MosBaseSyntax, MotorolaBaseSyntax, ZilogBaseSyntax
- Provides foundation for Intel family syntaxes
- Target: 70% reuse for platform-specific Intel syntaxes

#### 7.4 Intel Generic Syntax Plugin

**File:** `src/syntax/intel/intel_syntax.cpp`, `include/xasm++/syntax/intel/intel_syntax.h`

- [ ] Implement `IntelSyntax` class (extends IntelBaseSyntax)
- [ ] Generic Intel syntax (works for 8080, 8085, future 8086)
- [ ] Standard Intel mnemonics
- [ ] Register in `SyntaxRegistry`
- [ ] Write unit tests

#### 7.5 6800 CPU Plugin

**File:** `src/cpu/m6800/cpu_6800.cpp`, `include/xasm++/cpu/m6800/cpu_6800.h`

**Note:** 6800 uses Motorola syntax (already implemented in Phase 3)

- [ ] Implement `Cpu6800` class
- [ ] Implement 6800 instruction set (~72 instructions):
  - 8-bit loads: LDAA, LDAB, STAA, STAB
  - 16-bit loads: LDS, LDX, STS, STX
  - Arithmetic: ADDA, ADDB, ADCA, ADCB, SUBA, SUBB, SBCA, SBCB
  - Increment/Decrement: INCA, INCB, INC, DECA, DECB, DEC, INX, DEX
  - Logic: ANDA, ANDB, ORAA, ORAB, EORA, EORB, BITA, BITB
  - Shifts/Rotates: ASLA, ASLB, ASL, ASRA, ASRB, ASR, LSRA, LSRB, LSR
  - Rotate: ROLA, ROLB, ROL, RORA, RORB, ROR
  - Branches: BRA, BEQ, BNE, BCC, BCS, BMI, BPL, BVC, BVS, BGE, BLT, BGT, BLE, BHI, BLS
  - Jumps/Calls: JMP, JSR, BSR, RTS, RTI
  - Stack: PSHA, PSHB, PULA, PULB, PSH, PUL (undocumented)
  - Arithmetic helpers: CMPA, CMPB, CPX, CLRA, CLRB, CLR, NEGA, NEGB, NEG, COMA, COMB, COM
  - Transfers: TAB, TBA, TAP, TPA, TSX, TXS
  - Decimal: DAA
  - Condition codes: CBA, SEC, CLC, SEV, CLV, SEI, CLI
  - Test: TSTA, TSTB, TST
  - Misc: NOP, SWI, WAI

- [ ] Implement 6800 addressing modes:
  - Accumulator (A, B)
  - Immediate (#$XX)
  - Direct ($XX - zero page)
  - Extended ($XXXX - absolute)
  - Indexed (offset,X)
  - Inherent

- [ ] Register in `CpuRegistry`
- [ ] Write unit tests

**Code Reuse Strategy:**
- Uses MotorolaBaseSyntax from Phase 3 (no new syntax needed)
- Similar addressing modes to 6809 (simpler - no complex indexed modes)
- Can reference 6809 implementation patterns

**File:** `tests/integration/test_6800.cpp`

- [ ] Create 6800 test suite
- [ ] Test all addressing modes
- [ ] Compare against vasm 6800 output

#### 7.6 Integration Testing

**File:** `tests/integration/test_intel_output.cpp`

- [ ] Test Intel syntax with 8080/8085
- [ ] Verify correct opcode generation
- [ ] Compare against reference assemblers

**File:** `tests/integration/test_6800_output.cpp`

- [ ] Test 6800 with Motorola syntax
- [ ] Verify correct opcode generation
- [ ] Compare against vasm 6800 output

**File:** `tests/regression/intel/`

- [ ] Run 8080/8085 regression tests
- [ ] Verify byte-for-byte compatibility

**File:** `tests/regression/m6800/`

- [ ] Run 6800 regression tests
- [ ] Verify byte-for-byte compatibility with vasm

### Deliverables

- ✅ 8080 CPU support (reuses base from Phase 4: 95% code reuse)
- ✅ 8085 CPU support (extends 8080: 95% code reuse)
- ✅ IntelBaseSyntax base class (for code reuse)
- ✅ Intel generic syntax
- ✅ 6800 CPU support (reuses MotorolaBaseSyntax: 70% code reuse)
- ✅ **All 8 planned CPUs now implemented**
- ✅ **All 8 planned syntaxes now implemented**

### Acceptance Criteria

```
□ 8080 fully functional (78 instructions via base class)
□ 8080 uses Cpu8080Base with 95% code reuse
□ 8085 fully functional (8080 + 2-8 new instructions)
□ 8085 extends Cpu8080 with 95% code reuse
□ IntelBaseSyntax provides 70% code reuse for Intel family
□ Intel generic syntax working
□ 6800 fully functional (72 instructions)
□ 6800 reuses MotorolaBaseSyntax from Phase 3
□ Can assemble 8080, 8085, and 6800 programs
□ All unit tests passing (90%+ coverage)
□ Integration tests passing (against vasm and reference assemblers)
□ Regression tests: 100% byte-for-byte compatibility
□ Code coverage ≥ 85% overall
□ ALL 8 CPUs COMPLETE: 6502, 65C02, 65816, 6800, 6809, 68000, 8080/8085, Z80
□ ALL 8 SYNTAXES COMPLETE: mos, scmasm, merlin, motorola, edtasm, zilog, edtasm-m80, intel
```

### Phase 7 Milestone

**This completes the core CPU and syntax implementation!**

After Phase 7:
- ✅ All 8 CPU architectures implemented
- ✅ All 8 syntax modes implemented
- ✅ Platform-specific output formats for major platforms
- ✅ Code reuse strategy validated (50-70% average)
- ✅ Comprehensive test coverage

**Remaining work (Phase 8+):**
- Advanced assembler features (macros, conditionals, includes)
- Additional output formats (hex, listing, symbols)
- Optimization passes
- Performance tuning
- Documentation and examples
- Release preparation

---

## Phase 8: Advanced Assembler Features

**Goal:** Add advanced features: macros, conditionals, includes, listing/symbol output

**Duration:** 5-7 development sessions

### Tasks

#### 8.1 Macro Preprocessor

**File:** `src/core/macro.cpp`, `include/xasm++/core/macro.h`

- [ ] Implement `Macro` class:
  - Name, parameters, body
  - Storage in symbol table
- [ ] Implement macro expansion:
  - Parameter substitution
  - Nested expansion support
  - Recursive expansion detection
  - Local label generation
- [ ] Add macro directives to syntax plugins:
  - `.MACRO` / `.ENDM` - Define macro
  - Invocation by name
- [ ] Write unit tests

#### 8.2 Conditional Assembly

**File:** `src/core/conditional.cpp`, `include/xasm++/core/conditional.h`

- [ ] Implement conditional assembly:
  - `.IF` / `.ELSE` / `.ENDIF`
  - `.IFDEF` / `.IFNDEF`
  - Expression evaluation in conditions
  - Nesting support
- [ ] Add to syntax plugins
- [ ] Write unit tests

#### 8.3 Include File Support

**File:** `src/core/include.cpp`, `include/xasm++/core/include.h`

- [ ] Implement `.INCLUDE` directive
- [ ] Support include paths (-I flag)
- [ ] Prevent circular includes
- [ ] Maintain source location tracking
- [ ] Write unit tests

#### 8.4 Listing Output Plugin

**File:** `src/output/listing_output.cpp`, `include/xasm++/output/listing_output.h`

- [ ] Implement `ListingOutput` plugin
- [ ] Generate .lst file with:
  - Line numbers
  - Addresses
  - Hex bytes generated
  - Source lines
  - Error messages
  - Symbol table dump
  - Cross-reference
- [ ] Write unit tests

#### 8.5 Symbol File Output Plugin

**File:** `src/output/symbol_output.cpp`, `include/xasm++/output/symbol_output.h`

- [ ] Implement `SymbolOutput` plugin
- [ ] Generate .sym file with symbol definitions
- [ ] Support multiple formats:
  - Apple II ProDOS format
  - VICE debugger format
  - Custom JSON format
- [ ] Write unit tests

#### 8.6 CLI Enhancements

**File:** `src/main.cpp`

- [ ] Add `--list <file>` option
- [ ] Add `--symbols <file>` option
- [ ] Add `--format <format>` option (binary, hex, srec)
- [ ] Add `-I <path>` for include paths
- [ ] Add `-D<symbol>=<value>` for predefined symbols
- [ ] Add `-v`/`--verbose` for detailed output
- [ ] Add `--version` flag
- [ ] Add `--help` with detailed usage

### Deliverables

- ✅ Macro preprocessor
- ✅ Conditional assembly
- ✅ Include file support
- ✅ Multiple output formats (listing, symbols)
- ✅ Enhanced CLI

### Acceptance Criteria

```
□ Macros functional with parameters
□ Conditional assembly working
□ Include files working
□ Listing output generated
□ Symbol output generated
□ Can assemble complex real-world programs
□ All unit tests passing
□ Integration tests passing
□ Code coverage ≥ 85%
```

---

## Phase 9: Optimization and Performance

**Goal:** Implement optimization passes and tune performance

**Duration:** 4-6 development sessions

### Tasks

#### 9.1 Optimization Pass

**File:** `src/core/optimizer.cpp`, `include/xasm++/core/optimizer.h`

- [ ] Implement optimization strategies:
  - Zero-page optimization (6502)
  - Short branch optimization (all CPUs)
  - Absolute to PC-relative (68000)
  - Direct page optimization (6809, 65816)
  - Dead code elimination (optional)
- [ ] Add optimization flags to CLI (--optimize, -O0, -O1, -O2)
- [ ] Write unit tests

#### 9.2 Performance Profiling and Tuning

- [ ] Profile assembler performance on large files
- [ ] Optimize hot paths:
  - Symbol table lookups
  - Expression evaluation
  - Instruction encoding
- [ ] Consider parallel parsing (if beneficial)
- [ ] Benchmark against vasm

### Deliverables

- ✅ Optimization pass
- ✅ Performance tuning

### Acceptance Criteria

```
□ Optimization pass working
□ Can assemble large programs (>50KB) efficiently
□ Performance within 2x of vasm (acceptable for modern C++)
□ All unit tests passing
□ Integration tests passing
□ Code coverage ≥ 85%
```

---

## Phase 10: Polish and Release Preparation

**Goal:** Documentation, examples, and v1.0 release

**Duration:** 3-5 development sessions

### Tasks

#### 10.1 Documentation

- [ ] Complete README.md:
  - Feature list
  - Installation instructions
  - Usage examples
  - Supported CPUs and syntaxes
- [ ] Write USER_GUIDE.md:
  - Comprehensive usage guide
  - All directives documented
  - Expression syntax
  - Macro usage
  - Optimization flags
- [ ] Write CONTRIBUTING.md:
  - How to add a new CPU plugin
  - How to add a new syntax plugin
  - Coding standards
  - Testing requirements
  - Pull request process
- [ ] Generate API documentation (Doxygen):
  - Document all public interfaces
  - Class diagrams
  - Plugin interfaces

#### 10.2 Example Programs

**Directory:** `examples/`

- [ ] Create examples for each CPU:
  - `examples/6502/hello.asm` - Simple 6502 program
  - `examples/6502/game_loop.asm` - Game loop example
  - `examples/6809/hello.asm` - Simple 6809 program
  - `examples/68000/hello.asm` - Simple 68000 program
  - `examples/z80/hello.asm` - Simple Z80 program
- [ ] Create examples for each syntax:
  - `examples/scmasm/` - SCMASM syntax examples
  - `examples/edtasm/` - EDTASM syntax examples
  - `examples/motorola/` - Motorola syntax examples
- [ ] Create advanced examples:
  - `examples/macros/` - Macro usage
  - `examples/conditional/` - Conditional assembly
  - `examples/includes/` - Include file usage

#### 10.3 Release Engineering

- [ ] Version numbering scheme (semantic versioning: MAJOR.MINOR.PATCH)
- [ ] Create CHANGELOG.md
- [ ] Tag releases in git
- [ ] Create release builds for:
  - Linux (x86_64, ARM64)
  - macOS (x86_64, Apple Silicon)
  - Windows (x86_64)
- [ ] Package releases (tar.gz, zip)
- [ ] Create GitHub releases with binaries

#### 10.4 Website and Communication

- [ ] Create project website (GitHub Pages or similar):
  - Feature overview
  - Download links
  - Documentation
  - Examples
- [ ] Announce to relevant communities:
  - 6502.org forum
  - Vintage computer communities
  - Homebrew game development forums

### Deliverables

- ✅ Complete documentation
- ✅ Example programs
- ✅ Release builds
- ✅ Project website

### Acceptance Criteria

```
□ README.md complete
□ USER_GUIDE.md complete
□ CONTRIBUTING.md complete
□ API documentation generated
□ Example programs for all CPUs
□ Release builds created
□ v1.0.0 released
```

---

## Future Phases (Post-1.0)

### Phase 11: Advanced Features

- Object file support (ELF, COFF, etc.)
- Linker integration
- Debug information generation (DWARF)
- Cross-reference generation
- Source-level profiling

### Phase 12: Language Server Protocol

- LSP server implementation
- IDE integration (VS Code, Vim, Emacs)
- Syntax highlighting
- Code completion
- Error checking as-you-type
- Go-to-definition
- Find references

### Phase 13: Interactive Features

- REPL mode (interactive assembly)
- Debugger integration
- Instruction-level simulation

---

## Resource Requirements

### Development Skills Needed

- **C++17 expertise** - Modern C++ idioms, templates, smart pointers
- **Assembly language knowledge** - Familiarity with 6502, 6809, 68000, Z80
- **Compiler/assembler theory** - Parsing, symbol resolution, multi-pass algorithms
- **Testing discipline** - Unit testing, integration testing, TDD practices
- **CMake proficiency** - Build system configuration
- **Git proficiency** - Version control, branching, tagging

### Time Estimates (Conservative)

**Core Implementation (Phases 0-7):**
- **Phase 0:** Foundation - 1-2 sessions (8-16 hours)
- **Phase 1:** Minimal Viable Assembler (6502) - 5-8 sessions (40-64 hours)
- **Phase 2:** Complete 6502 + SCMASM + Apple II - 5-7 sessions (40-56 hours)
- **Phase 3:** 65C02 + 6809 + EDTASM + CoCo - 8-10 sessions (64-80 hours)
- **Phase 4:** Z80 + Zilog + EDTASM-M80 + TRS-80/CP/M - 8-10 sessions (64-80 hours)
- **Phase 5:** 68000 + Amiga/Atari - 6-8 sessions (48-64 hours)
- **Phase 6:** 65816 + Merlin + OMF + IIgs - 6-8 sessions (48-64 hours)
- **Phase 7:** 8080/8085 + Intel + 6800 - 5-7 sessions (40-56 hours)

**Subtotal (Core CPU/Syntax):** 44-60 sessions (352-480 hours)

**Polish and Features (Phases 8-10):**
- **Phase 8:** Advanced Features - 5-7 sessions (40-56 hours)
- **Phase 9:** Optimization - 4-6 sessions (32-48 hours)
- **Phase 10:** Release Prep - 3-5 sessions (24-40 hours)

**Subtotal (Polish):** 12-18 sessions (96-144 hours)

**Total Estimated Time (Phases 0-10):** 56-78 sessions (448-624 hours, or 56-78 full development days)

**Note:** These are conservative estimates assuming part-time development with learning curve. Experienced developers might complete phases faster. Phases can be parallelized (e.g., multiple CPU implementations can proceed concurrently).

---

## Risk Mitigation Strategies

### Risk: Complexity Overload

**Mitigation:**
- Start with minimal viable implementation (Phase 1)
- Add features incrementally
- Maintain working state at end of each phase
- Don't over-engineer early

### Risk: Plugin Overhead

**Mitigation:**
- Measure performance at each phase
- Profile and optimize hot paths
- Use zero-cost abstractions where possible
- Accept reasonable overhead for modularity benefits

### Risk: Testing Burden

**Mitigation:**
- Write tests concurrently with implementation (TDD)
- Automate test runs via CI
- Reuse test suites from vasm-ext
- Focus on high-value tests (integration tests for correctness)

### Risk: Compatibility Issues

**Mitigation:**
- Test against vasm output as ground truth
- Comprehensive integration test suites
- Document intentional deviations from vasm
- Support multiple syntax modes for flexibility

---

## Success Metrics

### Quantitative Metrics

- **Test Coverage:** ≥ 85% overall, ≥ 90% for core modules
- **Performance:** Within 2x of vasm execution time (acceptable tradeoff for modularity)
- **Binary Compatibility:** 100% match with vasm output for equivalent inputs
- **Platform Support:** Linux, macOS, Windows builds passing CI
- **Documentation:** Every public API documented
- **Code Quality:** Zero compiler warnings with -Wall -Wextra -Wpedantic

### Qualitative Metrics

- **Extensibility:** Adding new CPU takes < 1 week
- **Maintainability:** New contributors can understand architecture from docs
- **Usability:** CLI follows Unix conventions, clear error messages
- **Community:** Active usage in vintage computing community

---

## Dependencies and Prerequisites

### Development Environment

- **Compiler:** GCC 9+, Clang 10+, or MSVC 2019+
- **Build Tool:** CMake 3.20+
- **Version Control:** Git 2.20+
- **Testing:** GoogleTest (fetched automatically)
- **Optional:** Clang-format, clang-tidy, ccache

### External Libraries

- **CLI11** - Command-line parsing (header-only, via FetchContent)
- **GoogleTest** - Unit testing (via FetchContent, optional)
- **spdlog** (optional) - Logging (if needed)

### Reference Materials

- **vasm-ext source code** - Reference implementation
- **sourcerer source code** - SOLID architecture reference
- **CPU datasheets** - 6502, 6809, 68000, Z80 instruction sets
- **Syntax manuals** - SCMASM, EDTASM, Motorola assembler docs

---

## Conclusion

This phased implementation plan provides a clear roadmap for building xasm++ from scratch. By starting with a minimal viable assembler and incrementally adding features, we ensure:

- **Working software at every phase** - No long "in development" periods
- **Validated architecture early** - Phase 1 validates core design
- **Manageable complexity** - Each phase has focused scope
- **Risk reduction** - Problems discovered early, not at the end
- **Clear milestones** - Each phase has concrete deliverables

The architecture combines the proven modularity of vasm-ext with modern C++ practices from sourcerer, resulting in an extensible, maintainable codebase that will serve the vintage computing community for years to come.

---

**Next Steps:**

1. Review and approve this project plan
2. Begin Phase 0: Foundation and Setup
3. Create initial project structure
4. Set up CI pipeline
5. Begin Phase 1 implementation

**Document Status:** ✅ Ready for Review and Approval
**Next Review:** After Phase 0 completion
