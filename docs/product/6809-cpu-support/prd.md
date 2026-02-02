# PRD: 6809 CPU Support with EDTASM+ Syntax

**Date:** 2026-02-02
**Cartographer:** Claude (Orchestrator Session)
**Status:** Draft
**Beads Task:** xasm++-7iy
**Priority:** P2 (High)

## Executive Summary

Add Motorola 6809 CPU support to xasm++ with EDTASM+ assembler syntax compatibility, enabling cross-assembly for TRS-80 Color Computer and other 6809-based vintage computing platforms. This expands xasm++'s reach beyond the 6502/65816 ecosystem into the vibrant 6809 retro computing community.

The 6809 is widely regarded as one of the most elegant 8-bit CPUs ever designed, featuring advanced addressing modes, native 16-bit operations, and two independent stacks. Adding 6809 support positions xasm++ as a multi-architecture vintage cross-assembler.

## Problem Statement

**Current State:**
xasm++ currently supports only 6502 and 65816 CPUs with Merlin and SCMASM syntax. Developers targeting 6809-based systems (TRS-80 Color Computer, Dragon 32/64, Vectrex) must use alternative assemblers (vasm, asm6809, LWASM) which may lack modern tooling integration or have different feature sets.

**Desired State:**
xasm++ becomes a unified cross-assembler supporting both 6502 and 6809 families, allowing developers to use a single, modern toolchain for multiple vintage platforms.

**Impact:**
- **TRS-80 CoCo Community:** ~15,000+ active retro developers (estimate based on CoCo Facebook groups, forums)
- **6809 Development:** Growing interest due to OS-9 preservation efforts and homebrew games
- **Project Positioning:** Establishes xasm++ as multi-architecture solution, not 6502-only

## Goals and Success Metrics

**Primary Goal:** Enable xasm++ to assemble 6809 code with EDTASM+ syntax compatibility at feature parity with 6502 support.

**Success Metrics:**
- **Binary Compatibility:** 100% compatibility with reference 6809 code (vasm-ext output)
- **Instruction Coverage:** All 59 6809 base instructions supported
- **Syntax Compatibility:** EDTASM+ directive set fully implemented
- **Community Adoption:** 10+ GitHub stars/forks within 3 months of release
- **Real-World Validation:** Successfully assembles 3+ existing TRS-80 CoCo programs

**Non-Goals:**
- HD6309 extended instructions (future enhancement)
- OS-9 module format generation (Phase 2 feature)
- Optimization passes (out of scope)
- Graphical IDE integration (separate project)

## User Personas

### Persona 1: Retro Game Developer
- **Name:** Alex (Hobbyist Game Developer)
- **Background:** Develops homebrew games for TRS-80 Color Computer
- **Needs:**
  - Reliable assembler for 6809 code
  - Modern toolchain (command-line, scriptable)
  - Binary compatibility with original EDTASM
  - Fast iteration cycles
- **Pain Points:**
  - vasm syntax differences frustrating
  - LWASM lacks some features
  - Original EDTASM requires emulator/hardware
- **Usage Pattern:** Daily development, hundreds of assemblies per week

### Persona 2: OS-9 Preservation Developer
- **Name:** Jamie (Open Source Maintainer)
- **Background:** Maintains NitrOS-9 operating system ports
- **Needs:**
  - Assemble large OS-9 codebases
  - INCLUDE directive for modular code
  - Macro system for OS-9 system calls
  - Reproducible builds
- **Pain Points:**
  - Need consistent tooling across team
  - Build system integration
  - Cross-platform compatibility (Linux/Mac/Windows)
- **Usage Pattern:** Infrequent but large assemblies, CI/CD integration

### Persona 3: Vintage Computing Educator
- **Name:** Taylor (Computer Science Teacher)
- **Background:** Teaches assembly programming using vintage systems
- **Needs:**
  - Simple, clear error messages
  - Multiple architecture support (teaches both 6502 and 6809)
  - Cross-platform tooling
  - Good documentation
- **Pain Points:**
  - Students struggle with inconsistent tooling
  - Hard to set up multiple assemblers
  - Limited modern IDE support
- **Usage Pattern:** Classroom use, 20-30 students per semester

## Use Cases

### Use Case 1: Assemble Simple 6809 Program

**Actor:** Retro Game Developer
**Goal:** Assemble a "Hello World" program for TRS-80 CoCo
**Preconditions:** xasm++ installed, source file ready

**Main Flow:**
1. Write source code in EDTASM+ syntax
2. Run: `xasm++ --cpu 6809 --syntax edtasm hello.asm -o hello.bin`
3. Binary generated successfully
4. Load binary into CoCo emulator
5. Program runs correctly

**Postconditions:** Binary file created, ready for testing on hardware/emulator

**Alternative Flows:**
- Syntax error: Clear error message with line number and context
- Invalid opcode: Assembler suggests correct mnemonic

**Error Flows:**
- File not found: "Error: Cannot open 'hello.asm'"
- Invalid addressing mode: "Error: Invalid addressing mode for LDA at line 10"

### Use Case 2: Assemble Multi-File OS-9 Project

**Actor:** OS-9 Preservation Developer
**Goal:** Build modular OS-9 kernel component
**Preconditions:** Project with multiple .asm files, INCLUDE dependencies

**Main Flow:**
1. Main file INCLUDEs os9.d definitions
2. Run: `xasm++ --cpu 6809 --syntax edtasm -I./include main.asm -o module.bin`
3. Assembler processes all INCLUDEs recursively
4. Macros expanded correctly
5. Binary matches reference output

**Postconditions:** OS-9 module ready for integration

**Alternative Flows:**
- Missing INCLUDE: Search in include paths, report specific file
- Circular INCLUDE: Detect and report with dependency chain

### Use Case 3: Migrate from vasm to xasm++

**Actor:** Retro Game Developer
**Goal:** Switch existing project from vasm to xasm++
**Preconditions:** Project assembled with vasm6809_edtasm

**Main Flow:**
1. Run xasm++ on existing source
2. Assembler handles EDTASM+ syntax correctly
3. Binary output byte-identical to vasm output
4. Update build scripts to use xasm++
5. CI/CD tests pass

**Postconditions:** Project building with xasm++, no code changes needed

**Alternative Flows:**
- Syntax differences: Compatibility mode handles edge cases
- Directive extensions: xasm++ warns about unsupported features

## Functional Requirements

### Must Have (P0) - MVP

**REQ-001: Core 6809 Instruction Set**
Support all 59 base 6809 instructions with correct opcode encoding.
- **Acceptance Criteria:**
  - All instructions assemble correctly
  - Binary output matches reference assembler
  - Unit tests for each instruction
- **Rationale:** Foundational requirement for any 6809 assembler

**REQ-002: Addressing Modes**
Implement all 6809 addressing modes: Inherent, Immediate, Direct, Extended, Indexed (all variants), Relative.
- **Acceptance Criteria:**
  - Each addressing mode produces correct opcodes
  - Indexed mode variants (offset, auto-increment, indirect) work
  - Assembler chooses optimal mode when possible
- **Rationale:** 6809's power comes from flexible addressing

**REQ-003: EDTASM+ Data Directives**
Support FCB, FDB, FCC, RMB for defining data.
- **Acceptance Criteria:**
  - FCB: 8-bit values, expressions, character constants
  - FDB: 16-bit big-endian words
  - FCC: Flexible delimiter strings
  - RMB: Reserve uninitialized space
- **Rationale:** Core data definition capability

**REQ-004: Symbol Definition (EQU, SET)**
Define constants (EQU) and variables (SET).
- **Acceptance Criteria:**
  - EQU symbols cannot be redefined
  - SET symbols can be updated
  - Expression evaluation works
- **Rationale:** Essential for structured code

**REQ-005: ORG and END Directives**
Set assembly origin and mark end of source.
- **Acceptance Criteria:**
  - ORG sets current address
  - Multiple ORGs supported
  - END terminates assembly
- **Rationale:** Basic assembly control

**REQ-006: Comments**
Support asterisk column-1 comments and semicolon inline comments.
- **Acceptance Criteria:**
  - `*` in column 1 = comment line
  - `;` anywhere = comment to end of line
- **Rationale:** Code documentation

**REQ-007: Number Formats**
Parse decimal, hexadecimal ($), binary (%), and character constants.
- **Acceptance Criteria:**
  - `10` = decimal 10
  - `$FF` = hex 255
  - `%11110000` = binary 240
  - `'A'` = ASCII 65
- **Rationale:** Standard assembler notation

**REQ-008: Expressions**
Evaluate arithmetic expressions (+, -, *, /).
- **Acceptance Criteria:**
  - Order of operations correct
  - Parentheses supported
  - Symbol references work
- **Rationale:** Computed values common in assembly

### Should Have (P1) - Post-MVP

**REQ-009: INCLUDE Directive**
Include external source files.
- **Acceptance Criteria:**
  - Recursive includes work
  - Include path search
  - Circular include detection
- **Rationale:** Essential for large projects

**REQ-010: Macro System**
MACRO/ENDM with parameters (\\1-\\9), local labels (\\.label), unique IDs (\\@).
- **Acceptance Criteria:**
  - Macros define and expand correctly
  - Parameters substitute
  - Local labels unique per invocation
- **Rationale:** Code reuse, OS-9 system calls use macros

**REQ-011: Conditional Assembly**
COND/ENDC, IF/ELSE/ENDIF family.
- **Acceptance Criteria:**
  - Conditional blocks assemble based on expressions
  - Nesting supported
  - All comparison operators (EQ, NE, GT, LT, GE, LE)
- **Rationale:** Platform-specific code

**REQ-012: SETDP Directive**
Inform assembler of Direct Page register value for optimization.
- **Acceptance Criteria:**
  - Assembler uses SETDP for direct mode selection
  - Optimization applied automatically
- **Rationale:** Performance optimization

**REQ-013: Listing Control**
OPT, LIST/NOLIST, PAGE, TTL directives.
- **Acceptance Criteria:**
  - Listing output controlled by directives
  - Options modify listing format
- **Rationale:** Documentation and debugging

### Nice to Have (P2) - Future Enhancements

**REQ-014: OS-9 MOD Directive**
Generate OS-9 module headers with sync bytes and CRC.
- **Acceptance Criteria:**
  - MOD syntax recognized
  - Header structure correct
  - CRC calculated properly
- **Rationale:** OS-9 development support

**REQ-015: Case-Insensitive Mode**
Flag to enable case-insensitive symbol matching.
- **Acceptance Criteria:**
  - `--nocase` flag works
  - Symbols match regardless of case
- **Rationale:** Compatibility with original EDTASM

**REQ-016: Cross-Reference Listing**
Generate symbol cross-reference in listing.
- **Acceptance Criteria:**
  - All symbol uses listed
  - File and line numbers shown
- **Rationale:** Debugging aid

## Non-Functional Requirements

**Performance:**
- Assembly speed: >10,000 lines/second on modern hardware
- Memory usage: <100MB for typical projects (<50K lines)
- Startup time: <100ms

**Security:**
- No code execution from source files
- Path traversal prevention in INCLUDE directives
- Safe expression evaluation (no buffer overflows)

**Compatibility:**
- Binary output identical to vasm-ext EDTASM syntax module (reference implementation)
- Source code compatible with original EDTASM+ (case-insensitive mode)
- Build on Linux, macOS, Windows

**Maintainability:**
- Clean separation: CPU plugin, syntax parser, core assembler
- Unit test coverage: >80% for 6809-specific code
- Documentation: Reference docs, examples, inline comments

**Usability:**
- Clear error messages with line numbers and context
- Helpful suggestions for common mistakes
- Consistent command-line interface with 6502 support

## User Stories

### Epic 1: Core 6809 Assembly

**US-001: Assemble Basic 6809 Instruction**
- **As a** developer
- **I want** to assemble a simple LDA instruction
- **So that** I can verify 6809 support works
- **Acceptance Criteria:**
  - [ ] `LDA #$FF` assembles to correct opcode
  - [ ] `LDA $1000` uses extended addressing
  - [ ] `LDA ,X` uses indexed addressing
- **Priority:** P0
- **Estimate:** 2 points

**US-002: Support All Addressing Modes**
- **As a** developer
- **I want** all 6809 addressing modes to work
- **So that** I can write idiomatic 6809 code
- **Acceptance Criteria:**
  - [ ] Inherent: `CLRA` works
  - [ ] Immediate: `LDA #10` works
  - [ ] Direct: `LDA $50` works (with SETDP)
  - [ ] Extended: `LDA $1000` works
  - [ ] Indexed: `LDA ,X`, `LDA 10,X`, `LDA ,X+`, `LDA ,-X` work
  - [ ] Indirect: `LDA [,X]` works
  - [ ] Relative: `BRA LABEL` works
- **Priority:** P0
- **Estimate:** 5 points

**US-003: Define Data with FCB/FDB/FCC**
- **As a** developer
- **I want** to define data in my program
- **So that** I can include strings and tables
- **Acceptance Criteria:**
  - [ ] `FCB 1,2,3` creates byte array
  - [ ] `FDB $1234` creates word (big-endian)
  - [ ] `FCC /Hello/` creates string
  - [ ] `RMB 256` reserves space
- **Priority:** P0
- **Estimate:** 3 points

### Epic 2: EDTASM+ Syntax

**US-004: Use EQU for Constants**
- **As a** developer
- **I want** to define named constants
- **So that** my code is readable
- **Acceptance Criteria:**
  - [ ] `BUFSIZE EQU 256` defines constant
  - [ ] Cannot redefine EQU symbol
  - [ ] EQU evaluates expressions
- **Priority:** P0
- **Estimate:** 2 points

**US-005: Include External Files**
- **As a** developer
- **I want** to INCLUDE header files
- **So that** I can organize large projects
- **Acceptance Criteria:**
  - [ ] `INCLUDE os9.d` works
  - [ ] Include path search (-I flag)
  - [ ] Recursive includes supported
  - [ ] Circular include detected
- **Priority:** P1
- **Estimate:** 5 points

**US-006: Use Macros**
- **As a** developer
- **I want** to define and use macros
- **So that** I can avoid code duplication
- **Acceptance Criteria:**
  - [ ] MACRO/ENDM defines macro
  - [ ] Parameters (\\1-\\9) work
  - [ ] Local labels (\\.label) work
  - [ ] Unique IDs (\\@) work
  - [ ] Macro expansion correct
- **Priority:** P1
- **Estimate:** 8 points

### Epic 3: Validation and Quality

**US-007: Match Reference Output**
- **As a** developer
- **I want** binary output identical to vasm
- **So that** I can trust xasm++ output
- **Acceptance Criteria:**
  - [ ] Byte-for-byte match on test suite
  - [ ] All addressing modes produce same opcodes
  - [ ] Expression evaluation identical
- **Priority:** P0
- **Estimate:** 3 points

**US-008: Get Clear Error Messages**
- **As a** developer
- **I want** helpful error messages
- **So that** I can fix problems quickly
- **Acceptance Criteria:**
  - [ ] Line number and context shown
  - [ ] Error description clear
  - [ ] Suggestions for common mistakes
- **Priority:** P0
- **Estimate:** 3 points

## Wireframes and Designs

N/A - Command-line tool

## Technical Considerations

**Architecture:**
- Extend existing CPU plugin system (used for 6502/65816)
- Implement `cpu_6809.cpp` / `cpu_6809.h` for instruction encoding
- Implement `edtasm_syntax.cpp` / `edtasm_syntax.h` for parser
- Reuse core assembler (multi-pass, expression evaluator, symbol table)

**Known Constraints:**
- Big-endian word storage (6809 native format)
- Direct page optimization requires SETDP directive
- Indexed addressing has complex encoding rules

**Integration Points:**
- **CLI:** `--cpu 6809 --syntax edtasm` flags
- **Output:** Binary format (same as 6502)
- **Build System:** CMakeLists.txt updates

**Data Requirements:**
- Opcode tables: 6809 instruction→opcode mapping
- Addressing mode rules: Valid modes per instruction
- Directive handlers: EDTASM+ specific directives

**Reference Implementation:**
- vasm-ext EDTASM syntax module: Well-tested, comprehensive
- Use as validation reference for binary compatibility

## Dependencies

**Requires:**
- Existing CPU plugin architecture (present in xasm++)
- Parser framework (present in xasm++)
- Expression evaluator (present in xasm++)
- Multi-pass assembler (present in xasm++)

**Blocks:**
- None currently

**External Dependencies:**
- pdftotext (for extracting reference material) - DONE
- Test suite: Real 6809 code samples for validation

## Risks and Mitigations

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Complex indexed addressing encoding | High | Medium | Reference Leventhal book and vasm implementation |
| EDTASM+ syntax edge cases | Medium | Medium | Use vasm-ext as reference, test with real code |
| Binary incompatibility with vasm | High | Low | Extensive test suite, byte-by-byte comparison |
| Incomplete documentation | Medium | Low | Multiple sources: Leventhal, EDTASM manual, vasm-ext |
| Macro system complexity | Medium | Medium | Phased implementation, start with basic macros |
| OS-9 module format unknown | Low | High | Defer to Phase 2, basic MOD recognition OK |

## Timeline and Phases

**Phase 0: Documentation and Planning (CURRENT)**
- [x] Extract reference materials from PDFs
- [x] Study vasm-ext EDTASM implementation
- [x] Create instruction set reference
- [x] Create syntax reference
- [x] Create PRD
- **Deliverable:** Comprehensive documentation

**Phase 1: Architecture Design**
- [ ] Design CPU plugin integration
- [ ] Define instruction encoding strategy
- [ ] Plan syntax parser structure
- [ ] Create technical design document
- **Deliverable:** Architecture design doc

**Phase 2: Core Implementation (MVP)**
- [ ] Implement cpu_6809.cpp (instruction encoding)
- [ ] Implement edtasm_syntax.cpp (parser)
- [ ] Support all instructions and addressing modes
- [ ] Implement core directives (ORG, END, EQU, FCB, FDB, FCC, RMB)
- [ ] Unit tests for all instructions
- **Deliverable:** Working 6809 assembler (basic)

**Phase 3: EDTASM+ Features (Post-MVP)**
- [ ] Implement INCLUDE directive
- [ ] Implement macro system
- [ ] Implement conditional assembly
- [ ] Implement SETDP optimization
- [ ] Implement listing control
- **Deliverable:** Feature-complete EDTASM+ compatibility

**Phase 4: Validation and Polish**
- [ ] Binary compatibility tests vs vasm-ext
- [ ] Assemble real TRS-80 CoCo programs
- [ ] Performance optimization
- [ ] Error message improvements
- [ ] Documentation polish
- **Deliverable:** Production-ready release

**Phase 5: OS-9 Support (Future)**
- [ ] MOD directive with full header generation
- [ ] OS-9 system call macro library
- [ ] OS-9 output format
- **Deliverable:** Full OS-9 development support

## Open Questions

- [x] Where is reference material? - RESOLVED: Found PDFs, vasm-ext implementation
- [x] Which EDTASM+ features are critical? - RESOLVED: See Must Have requirements
- [ ] How to handle HD6309 extended instructions? - NOT BLOCKING: Future enhancement
- [ ] Should we support LWASM extensions? - NOT BLOCKING: EDTASM+ first
- [ ] Case-sensitive or case-insensitive default? - NOT BLOCKING: Case-sensitive default, --nocase flag

## Stakeholders

**Owner:** Bryan Woodruff (Project Maintainer)
**Reviewers:** Architect (Phase 1), Engineer (Phase 2-4)
**Approvers:** Project Maintainer
**Informed:** TRS-80 CoCo community, NitrOS-9 team, 6809 developers

## Related Documents

**Reference Documentation:**
- [6809 Instruction Set Reference](../../reference/6809/instruction-set.md)
- [EDTASM+ Syntax Reference](../../reference/6809/edtasm-syntax.md)

**Implementation:**
- **Architecture:** `docs/architecture/6809-plugin.md` (Phase 1 deliverable)
- **Task Packets:** `.ai/tasks/2026-02-02_6809-cpu-support/`
- **Beads Task:** xasm++-7iy

**External References:**
- vasm-ext EDTASM module: `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm/`
- Leventhal 6809 book: `~/Downloads/6809 Assembly Language Programming (Lance Leventhal).pdf`
- EDTASM manual: `~/Downloads/Disk EDTASM Remastered (Tandy).pdf`

## Changelog

| Date | Version | Changes | Author |
|------|---------|---------|--------|
| 2026-02-02 | 1.0 | Initial draft | Claude (Cartographer) |

---

**Next Steps:**
1. ✅ PRD Review and Approval
2. Delegate to **Architect** for technical design (Phase 1)
3. Create Beads tasks for implementation phases
4. Delegate to **Engineer** for implementation (Phase 2+)
