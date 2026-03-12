# Implementation Plan: 6809 CPU Support

**Beads Task:** xasm++-7iy
**Workflow:** Feature Workflow (Phase 0: Product Definition)
**Current Phase:** Phase 0 - Requirements & Documentation (Cartographer)

## Phase 0: Requirements & Documentation (COMPLETE ✅)

**Cartographer Deliverables:**

### 0.1: Reference Material Extraction ✅
- [x] Extracted Leventhal 6809 book (52,801 lines)
- [x] Extracted EDTASM Remastered manual (23,201 lines)
- [x] Studied vasm-ext EDTASM implementation
- [x] Web research on EDTASM+ syntax and 6809 addressing modes

### 0.2: Organized Reference Documentation ✅
Created in `docs/reference/6809/`:
- [x] `instruction-set.md` - Complete 6809 instruction set, addressing modes, register set
- [x] `edtasm-syntax.md` - EDTASM+ directive reference, syntax rules, examples

### 0.3: Product Requirements Document ✅
Created in `docs/product/6809-cpu-support/`:
- [x] `prd.md` - Comprehensive PRD with:
  - Executive summary
  - Problem statement and goals
  - 3 user personas (game dev, OS-9 dev, educator)
  - 3 detailed use cases
  - Functional requirements (P0/P1/P2)
  - 8 user stories across 3 epics
  - Non-functional requirements
  - Risk assessment
  - 5-phase timeline
  - Stakeholders and related docs

### 0.4: Competitive Analysis ✅
Documented in PRD:
- vasm-ext (reference implementation)
- asm6809
- LWASM
- Original EDTASM+

**Comparison:**
- xasm++ will match vasm-ext binary output (validation reference)
- Unified tooling across 6502 and 6809 (differentiator)
- Modern CMake build system (advantage)

## Phase 1: Architecture Design (NEXT)

**Owner:** Architect Role
**Duration:** TBD
**Deliverables:**

### 1.1: CPU Plugin Architecture
- Design how cpu_6809 integrates with existing plugin system
- Define instruction encoding strategy (opcode tables, addressing mode logic)
- Plan register model (A, B, D, X, Y, U, S, PC, DP, CC)

### 1.2: Syntax Parser Integration
- Design edtasm_syntax parser structure
- Define directive handlers (FCB, FDB, FCC, RMB, etc.)
- Plan macro system architecture (MACRO/ENDM, parameters, local labels)

### 1.3: Technical Design Document
Create `docs/architecture/6809-plugin.md`:
- Class diagrams
- Instruction encoding algorithm
- Addressing mode resolution
- Integration with core assembler

### 1.4: ADR (Architecture Decision Records)
Document key decisions:
- Big-endian vs little-endian handling
- Direct page optimization strategy
- Macro system implementation approach

**Acceptance Criteria:**
- [ ] Architecture design document complete
- [ ] Design reviewed and approved
- [ ] Integration points clearly defined
- [ ] No blocking technical issues identified

## Phase 2: Core Implementation (MVP)

**Owner:** Engineer Role
**Duration:** TBD
**Prerequisites:** Phase 1 complete

### 2.1: CPU Plugin Implementation
- Implement `include/xasm++/cpu/cpu_6809.h`
- Implement `src/cpu/m6809/cpu_6809.cpp`
- Opcode encoding for all 59 base instructions
- Addressing mode support (all variants)

### 2.2: Syntax Parser Implementation
- Implement `include/xasm++/syntax/edtasm_syntax.h`
- Implement `src/syntax/edtasm/edtasm_syntax.cpp`
- Directive handlers (ORG, END, EQU, SET, FCB, FDB, FCC, RMB)
- Comment parsing (* column 1, ; inline)
- Number format parsing ($, %, ', decimal)

### 2.3: Unit Tests
- Test suite for each instruction
- Test suite for each addressing mode
- Test suite for each directive
- Expression evaluation tests

### 2.4: Integration
- Update CMakeLists.txt
- CLI integration (--cpu 6809 --syntax edtasm)
- Build system validation

**Acceptance Criteria:**
- [ ] All 59 instructions assemble correctly
- [ ] All addressing modes work
- [ ] Core directives implemented
- [ ] Unit tests passing (>80% coverage)
- [ ] Binary output matches vasm-ext on test cases

## Phase 3: EDTASM+ Features

**Owner:** Engineer Role
**Duration:** TBD
**Prerequisites:** Phase 2 complete

### 3.1: INCLUDE Directive
- File inclusion with path search
- Recursive include support
- Circular include detection

### 3.2: Macro System
- MACRO/ENDM definition
- Parameter substitution (\\1-\\9)
- Local labels (\\.label)
- Unique IDs (\\@)
- Macro expansion

### 3.3: Conditional Assembly
- COND/ENDC basic conditionals
- IF/ELSE/ENDIF family (EQ, NE, GT, LT, GE, LE)
- Nesting support

### 3.4: Advanced Directives
- SETDP (direct page optimization)
- OPT (listing options)
- LIST/NOLIST
- PAGE, TTL, NAM

**Acceptance Criteria:**
- [ ] INCLUDE works with test projects
- [ ] Macros expand correctly
- [ ] Conditional assembly tested
- [ ] Advanced directives functional

## Phase 4: Validation and Polish

**Owner:** Engineer + Tester
**Duration:** TBD
**Prerequisites:** Phase 3 complete

### 4.1: Binary Compatibility Tests
- Compare xasm++ output to vasm-ext byte-by-byte
- Test suite: 100+ test cases covering all features
- Real-world validation: Assemble existing 6809 programs

### 4.2: Performance Optimization
- Profile assembly speed
- Optimize hot paths
- Memory usage optimization

### 4.3: Error Message Quality
- Improve error messages (clarity, suggestions)
- Add context to error reports
- Test error cases

### 4.4: Documentation
- User guide for 6809 support
- EDTASM+ migration guide
- Examples and tutorials

**Acceptance Criteria:**
- [ ] 100% binary compatibility with vasm-ext on test suite
- [ ] 3+ real TRS-80 CoCo programs assemble successfully
- [ ] Assembly speed >10,000 lines/second
- [ ] Error messages clear and helpful
- [ ] Documentation complete

## Phase 5: OS-9 Support (Future)

**Owner:** Engineer
**Duration:** TBD
**Prerequisites:** Phase 4 complete

### 5.1: MOD Directive
- OS-9 module header generation
- Sync bytes ($87CD)
- Header parity byte
- Module CRC calculation

### 5.2: OS-9 System Call Macros
- OS9 macro for system calls
- Standard include files (os9.d)
- Error code definitions

### 5.3: OS-9 Output Format
- Native OS-9 module format output
- Integration with OS-9 filesystem tools

**Acceptance Criteria:**
- [ ] MOD directive generates valid OS-9 modules
- [ ] OS-9 programs assemble and run
- [ ] Module CRC verified correct

## Execution Strategy

**Sequential Execution** (each phase depends on previous):
1. Phase 0: Documentation (Cartographer) ✅ COMPLETE
2. Phase 1: Architecture (Architect) ← NEXT
3. Phase 2: Core Implementation (Engineer)
4. Phase 3: EDTASM+ Features (Engineer)
5. Phase 4: Validation (Engineer + Tester)
6. Phase 5: OS-9 Support (Engineer)

**Blocking:** Each phase must complete before next begins.

## Success Criteria

Phase 0 ✅:
- [x] Reference documentation complete
- [x] PRD created and comprehensive
- [x] Competitive analysis done
- [x] Requirements clearly defined

Phase 1-5: See acceptance criteria in each phase above.

**Overall Success:**
- 100% binary compatibility with vasm-ext
- All 6809 instructions supported
- EDTASM+ syntax fully compatible
- Real TRS-80 CoCo code assembles
- Community adoption (10+ stars)

## Next Action

**Delegate to Architect** for Phase 1: Architecture Design

**Architect Task:**
- Create `docs/architecture/6809-plugin.md`
- Design CPU plugin integration
- Design syntax parser structure
- Document ADRs for key decisions
- Prepare for Engineer handoff
