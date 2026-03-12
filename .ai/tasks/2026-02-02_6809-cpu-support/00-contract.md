# Task Contract: 6809 CPU Support - Motorola 6809 Assembly

**Beads Task:** xasm++-7iy
**Created:** 2026-02-02
**Priority:** P2 (High)
**Type:** Feature - New CPU Support

## Context

xasm++ currently supports 6502 and 65816 CPUs. We need to add support for the **Motorola 6809** CPU to enable cross-assembly for TRS-80 Color Computer and other 6809-based systems.

The 6809 is a sophisticated 8-bit CPU with:
- More powerful instruction set than 6502
- Advanced addressing modes
- Native 16-bit operations
- Different assembly syntax (EDTASM+)

## Reference Materials Available

1. **Existing Implementation:**
   - `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm` - TRS-80 Disk EDTASM+ reference implementation

2. **Documentation:**
   - `~/Downloads/Disk EDTASM Remastered (Tandy).pdf` - Official EDTASM+ syntax and directives
   - `~/Downloads/6809 Assembly Language Programming (Lance Leventhal).pdf` - Complete instruction set reference

3. **Online Resources:**
   - TRS-80 Color Computer Disk EDTASM+ documentation
   - Community resources and examples

## Requirements

### Phase 0: Requirements & Documentation (Cartographer)

**R1: Extract and Organize Reference Material**
- Extract text from PDF documentation using `pdftotext`
- Create structured reference documentation in `docs/reference/6809/`
- Organize by:
  - Instruction set and opcodes
  - Addressing modes
  - Assembler directives (EDTASM+ syntax)
  - Pseudo-ops and macros
  - Examples and patterns

**R2: Research EDTASM+ Syntax**
- Study vasm-ext EDTASM implementation
- Research online for TRS-80 Color Computer Disk EDTASM+ documentation
- Document syntax differences from Merlin (6502) and other syntaxes
- Identify compatibility requirements

**R3: Define Product Requirements**
- What 6809 features must be supported?
- Which EDTASM+ directives are critical?
- What level of compatibility with original EDTASM+?
- Which addressing modes?
- What's the target use case? (retro development, historical code assembly)

**R4: Create PRD**
- Product Requirements Document in `docs/product/6809-cpu-support/`
- Define feature scope and priorities
- Acceptance criteria for 6809 support

### Phase 1: Architecture Design (Architect)

**R5: CPU Plugin Architecture**
- Design how 6809 CPU integrates with existing plugin system
- Define instruction encoding strategy
- Address mode handling
- Syntax parser integration

**R6: Technical Design Document**
- Architecture design in `docs/architecture/6809-plugin.md`
- Class structure and interfaces
- Integration points

### Phase 2: Implementation (Engineer)

**R7: Core 6809 CPU Implementation**
- Implement `cpu_6809.cpp` / `cpu_6809.h`
- Instruction encoding for all 6809 opcodes
- Addressing mode support

**R8: EDTASM+ Syntax Support**
- Parser for EDTASM+ syntax
- Directive handling
- Compatibility mode

**R9: Tests and Validation**
- Unit tests for all instructions
- Integration tests with real 6809 code
- Binary compatibility validation

## Acceptance Criteria

### Phase 0 (Cartographer) - Documentation & Requirements
- [ ] Reference documentation extracted and organized in `docs/reference/6809/`
- [ ] Instruction set reference complete
- [ ] EDTASM+ syntax documented
- [ ] PRD created with feature scope and priorities
- [ ] Competitive analysis (vs. other 6809 assemblers)

### Phase 1 (Architect) - Design
- [ ] Architecture design document complete
- [ ] CPU plugin integration strategy defined
- [ ] Technical design reviewed and approved

### Phase 2 (Engineer) - Implementation
- [ ] All 6809 instructions implemented and tested
- [ ] EDTASM+ syntax parser working
- [ ] Real-world 6809 code assembles correctly
- [ ] Binary output matches reference assembler

## Out of Scope (For Now)

- OS-9 assembler syntax (focus on EDTASM+ first)
- Advanced macro capabilities beyond basic EDTASM+
- Optimization passes (future enhancement)

## Dependencies

- Existing CPU plugin architecture (from 6502/65816)
- Parser framework (existing)

## Risks

| Risk | Mitigation |
|------|------------|
| EDTASM+ syntax incompatibilities | Study vasm-ext implementation, research online |
| Incomplete documentation | Extract from multiple sources, validate with community |
| Complex addressing modes | Reference Leventhal book, test incrementally |
| Opcode encoding complexity | Use authoritative references, validate against known assemblers |

## Success Metrics

- All 6809 instructions supported
- Real TRS-80 Color Computer code assembles
- Binary output matches reference assembler
- Documentation quality enables future maintenance

## Stakeholders

- **Retro Computing Community:** Need reliable 6809 assembler
- **TRS-80 CoCo Developers:** Need EDTASM+ compatibility
- **Project Maintainers:** Need clean architecture and good docs
