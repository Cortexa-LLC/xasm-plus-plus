# Phase 1 Summary: 6809 CPU Support - Architecture Design

**Task:** xasm++-7iy (6809 CPU Support - Motorola 6809 Assembly)
**Phase:** Phase 1 - Architecture Design
**Date:** 2026-02-03
**Role:** Architect
**Status:** ✅ COMPLETE

---

## Executive Summary

Phase 1 (Architecture Design) for 6809 CPU support is **complete**. The technical architecture has been designed, APIs specified, data models documented, and key decisions captured in ADRs. The design extends xasm++'s existing CPU plugin and syntax parser frameworks while maintaining consistency with 6502/65816 implementation patterns.

**Ready for Implementation:** YES ✅

---

## Deliverables

All architecture artifacts have been created and persisted to the repository:

### 1. Architecture Design Document

**Location:** `docs/architecture/2026-02-02-6809-cpu-support/architecture.md`

**Contents:**
- **Architecture Overview:** High-level component diagram, integration points
- **Component Architecture:** Cpu6809 plugin and EdtasmSyntaxParser design
- **Data Architecture:** Addressing modes, opcode tables, indexed addressing
- **Integration Architecture:** Assembler engine flow, multi-pass strategy
- **Non-Functional Architecture:** Performance, security, scalability
- **Technology Stack:** C++20, CMake, GoogleTest
- **Architecture Decision Records:** 4 major decisions documented
- **Implementation Phasing:** Phase 2-4 detailed plans
- **Validation Strategy:** Unit testing, binary compatibility, performance

**Size:** 46,467 bytes (comprehensive)

---

### 2. API Specification

**Location:** `docs/architecture/2026-02-02-6809-cpu-support/api-spec.md`

**Contents:**
- **Cpu6809 Plugin API:** 59+ instruction encoding methods documented
- **EdtasmSyntaxParser API:** Parse methods, directive handlers
- **AddressingMode6809 Enumeration:** All 18 modes defined
- **Data Models:** OpcodeTable, MacroDefinition, ConditionalBlock
- **Error Handling:** Error codes, message formats
- **Performance Characteristics:** Time/space complexity tables
- **Thread Safety:** Analysis of thread-safe vs non-thread-safe components
- **Examples:** Complete end-to-end assembly example

**Size:** 20,529 bytes

---

### 3. Data Models Document

**Location:** `docs/architecture/2026-02-02-6809-cpu-support/data-models.md`

**Contents:**
- **6809 Instruction Encoding:** 1-5 byte formats, examples
- **Addressing Modes:** Enumeration, size table
- **Indexed Addressing Post-Byte:** Detailed encoding (16 sub-modes)
- **Opcode Tables:** Structure and examples (LDA, LDD, BEQ, PSHS)
- **Byte Order (Endianness):** Big-endian convention, utility functions
- **Register Set:** Encoding table, TFR/EXG, CC bits, stack masks
- **Direct Page Addressing:** DP register, optimization strategy
- **Branch Offset Calculation:** Short (8-bit) and long (16-bit)
- **Macro Data Model:** Definition, expansion, parameter substitution
- **Conditional Assembly:** Block structure, nesting
- **Symbol Table:** Entry structure
- **Number Format Parsing:** $hex, %binary, 'char', decimal
- **Expression Evaluation:** AST-based expression tree

**Size:** 18,013 bytes

---

### 4. Architecture Decision Record (ADR-001)

**Location:** `docs/architecture/2026-02-02-6809-cpu-support/adr-001-big-endian.md`

**Decision:** Big-Endian Byte Order Handling

**Summary:**
- **Context:** 6809 uses big-endian (MSB first), 6502 uses little-endian (LSB first)
- **Decision:** Create `ToBigEndian(uint16_t)` utility function in Cpu6809 class
- **Rationale:** Encapsulation, type safety, maintainability, clarity, testability
- **Consequences:**
  - ✅ Positive: Clear separation, easy to test, self-documenting
  - ⚠️ Negative: Slight vector allocation overhead (negligible)
- **Alternatives Considered:** Manual bit shifts, union-based, memcpy, Boost.Endian (all rejected)
- **Validation:** Test cases provided, binary compatibility plan

**Size:** 8,819 bytes

**Note:** Additional ADRs (002-004) will be created during implementation as specific decisions arise.

---

## Architecture Highlights

### Component Design

**Two Main Components:**

1. **Cpu6809 Plugin** (extends CPU plugin framework)
   - Responsibility: Encode 6809 instructions into machine code
   - Interface: 59+ instruction encoding methods
   - Self-contained: No dependencies on other xasm++ components
   - Key Features: Big-endian handling, indexed addressing (16 sub-modes), multi-page opcodes

2. **EdtasmSyntaxParser** (extends syntax parser framework)
   - Responsibility: Parse EDTASM+ syntax, generate Atom stream
   - Interface: Parse() method, 20+ directive handlers
   - Dependencies: Cpu6809, Section, SymbolTable, Expression
   - Key Features: Case-insensitive, macro system, conditional assembly, INCLUDE support

### Data Models

**Key Structures:**

1. **AddressingMode6809:** Enumeration with 18 modes
   - Basic: Inherent, Immediate8/16, Direct, Extended, Relative8/16
   - Indexed: 16 sub-modes (zero offset, 5/8/16-bit offset, accumulator, auto-inc/dec, PC-relative, indirect)

2. **OpcodeTable:** Maps addressing modes to opcodes
   - Uses `std::optional<uint8_t>` for valid mode/opcode combinations
   - Reduces duplication (one table per instruction, generic encoding function)

3. **Indexed Post-Byte:** Complex encoding (most challenging aspect)
   - 1-byte post-byte encodes: register (X/Y/U/S/PC), sub-mode, indirect flag
   - Additional 0-2 bytes for offsets
   - Detailed encoding table provided in data models

### Integration Strategy

**Multi-Pass Assembly:**
1. **Pass 1:** Gather symbols, determine approximate sizes
2. **Pass 2:** Resolve symbols, optimize addressing modes
3. **Pass 3:** Final encoding with all symbols resolved

**Flow:**
```
EDTASM+ Source
    ↓
EdtasmSyntaxParser::Parse()
    ↓
DetermineAddressingMode()
    ↓
Cpu6809::EncodeXXX()
    ↓
std::vector<uint8_t> (machine code)
    ↓
Section::AddAtom(ByteAtom)
    ↓
Multi-Pass Resolution
    ↓
Binary Output
```

### Key Design Decisions

**ADR-001: Big-Endian Byte Order**
- **Problem:** 6809 big-endian vs 6502 little-endian
- **Solution:** ToBigEndian() utility function
- **Impact:** Encapsulation, type safety, single point of change

**ADR-002: Direct Page Optimization** (outlined in architecture doc)
- **Problem:** When to use direct (2 bytes) vs extended (3 bytes)?
- **Solution:** Require explicit SETDP directive, no automatic tracking
- **Impact:** Predictable behavior, programmer control

**ADR-003: Macro System Implementation** (outlined in architecture doc)
- **Problem:** How to implement EDTASM+ macros?
- **Solution:** Text substitution with parameter replacement
- **Impact:** Simplicity, compatibility with original EDTASM+

**ADR-004: Indexed Addressing Post-Byte** (outlined in architecture doc)
- **Problem:** 16 sub-modes, complex encoding
- **Solution:** Separate EncodeIndexedPostByte() function, lookup table
- **Impact:** Clarity, testability, maintainability

---

## Technical Design Quality

### Strengths

✅ **Clear Component Boundaries:** CPU plugin and syntax parser are well-separated

✅ **Well-Defined Interfaces:** All APIs documented with signatures, parameters, returns, examples

✅ **Comprehensive Data Models:** Addressing modes, opcode tables, indexed encoding all specified

✅ **Big-Endian Handling:** Explicit ToBigEndian() function prevents byte order bugs

✅ **Indexed Addressing Strategy:** Most complex feature has detailed encoding table

✅ **Integration Plan:** Clear flow from source to binary, multi-pass strategy defined

✅ **Non-Functional Requirements:** Performance, security, scalability addressed

✅ **Validation Strategy:** Unit tests, binary compatibility, performance benchmarks

✅ **Technology Choices:** C++20, CMake, GoogleTest (consistent with existing code)

### Areas Requiring Careful Implementation

⚠️ **Indexed Addressing Post-Byte:** Most complex aspect - 16 sub-modes
- **Mitigation:** Detailed encoding table provided, separate function, extensive tests

⚠️ **Direct Page Optimization:** Programmer must use SETDP directive
- **Mitigation:** Explicit is better than implicit, matches reference assemblers

⚠️ **Macro System Complexity:** Parameters, local labels, unique IDs
- **Mitigation:** Text-based approach is simple, recursion limits prevent issues

⚠️ **Big-Endian Byte Order:** Must be consistent across all 16-bit values
- **Mitigation:** ToBigEndian() function ensures uniformity, unit tests validate

---

## Implementation Readiness

### Ready for Engineer Handoff: YES ✅

**Checklist:**
- [x] Architecture document complete and comprehensive
- [x] API specification complete with examples
- [x] Data models documented in detail
- [x] Major decisions documented (ADRs)
- [x] Component boundaries clearly defined
- [x] Integration strategy specified
- [x] Technology choices justified
- [x] Risks assessed and mitigated
- [x] Validation strategy defined
- [x] No blocking technical issues identified

### What Engineers Have

1. **Clear Specifications:**
   - Exact API signatures for all 59+ instruction encoding methods
   - Detailed opcode tables for each instruction
   - Indexed addressing post-byte encoding table
   - Directive handler specifications

2. **Implementation Guidance:**
   - ToBigEndian() utility function design
   - Opcode table structure (reduces duplication)
   - Multi-pass assembly strategy
   - Macro expansion algorithm

3. **Validation Approach:**
   - Unit test specifications
   - Binary compatibility test plan (compare with vasm-ext)
   - Performance benchmarks

4. **Examples:**
   - Complete end-to-end assembly example
   - Encoding examples for each instruction type
   - Addressing mode determination logic

---

## Next Steps

### Phase 2: Core Implementation (MVP)

**Goal:** Working 6809 assembler with basic functionality

**Tasks for Engineers:**

1. **Complete Cpu6809 Implementation:**
   - Implement all 59 base instructions (extend stub)
   - Implement all addressing modes (immediate, direct, extended, indexed)
   - Implement indexed addressing post-byte encoding
   - Implement big-endian byte order handling
   - Unit tests for each instruction and addressing mode

2. **Create EdtasmSyntaxParser:**
   - Implement class structure
   - Implement core directives (ORG, END, EQU, SET, FCB, FDB, FCC, RMB)
   - Implement comment parsing (* and ;)
   - Implement number format parsing ($, %, ', decimal)
   - Implement instruction parsing and addressing mode determination
   - Integrate with Cpu6809 for encoding
   - Unit tests for each directive

3. **CLI Integration:**
   - Add `--cpu 6809` flag
   - Add `--syntax edtasm` flag
   - Validation: 6809 requires EDTASM syntax

4. **Build System:**
   - Add `src/syntax/edtasm/edtasm_syntax.cpp` to CMakeLists.txt
   - Verify existing `src/cpu/m6809/cpu_6809.cpp` is included
   - Update include paths

**Acceptance Criteria:**
- All 59 instructions assemble correctly
- All addressing modes work
- Core directives functional
- Simple 6809 programs assemble successfully
- Unit tests passing (>80% coverage)

**Estimated Effort:** 2-3 weeks (depends on Engineer availability)

---

### Phase 3: EDTASM+ Features (Post-MVP)

**Goal:** Feature parity with EDTASM+

**Features:**
- INCLUDE directive (file inclusion, recursive, circular detection)
- Macro system (MACRO/ENDM, parameters \\1-\\9, local labels \\.label, unique IDs \\@)
- Conditional assembly (COND/ENDC, IF/ELSE/ENDIF family)
- Advanced directives (SETDP, NAM, TTL, PAGE, SPC, OPT, LIST/NOLIST)

**Estimated Effort:** 2-3 weeks

---

### Phase 4: Validation and Polish (Quality)

**Goal:** Production-ready release

**Tasks:**
- Binary compatibility test suite (100+ test cases)
- Byte-by-byte comparison with vasm-ext
- Assemble real TRS-80 CoCo programs (3+ programs)
- Performance optimization (>10,000 lines/second)
- Error message improvements
- Documentation polish

**Estimated Effort:** 1-2 weeks

---

## Risks and Mitigations

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Indexed addressing bugs | High | Medium | Detailed encoding table, separate function, extensive tests |
| Big-endian byte order errors | High | Low | ToBigEndian() function, unit tests, careful review |
| EDTASM+ syntax edge cases | Medium | Medium | Use vasm-ext as reference, test with real code |
| Binary incompatibility | High | Low | Extensive test suite, byte-by-byte comparison |
| Direct page optimization bugs | Medium | Low | Explicit SETDP directive, warnings for suboptimal code |
| Macro system complexity | Medium | Medium | Phased implementation, recursion limits, testing |
| Performance below targets | Low | Low | Profile early, optimize hot paths |

---

## Success Metrics

**Architecture Design Success:** ✅ ACHIEVED
- ✓ Clear component boundaries defined
- ✓ Integration points specified
- ✓ Data models documented
- ✓ ADRs created for major decisions
- ✓ Technology choices justified
- ✓ Risks identified and mitigated
- ✓ Implementation phases defined
- ✓ Validation strategy documented

**Implementation Success (Future Phases):**
- All 59 6809 instructions supported
- EDTASM+ syntax fully compatible
- Binary output matches vasm-ext (100%)
- Real TRS-80 CoCo code assembles (3+ programs)
- Performance targets met (>10,000 lines/sec)
- Unit test coverage >80%
- Documentation complete
- Community adoption (10+ GitHub stars)

---

## References

**Architecture Documents:**
- [Architecture Design](docs/architecture/2026-02-02-6809-cpu-support/architecture.md)
- [API Specification](docs/architecture/2026-02-02-6809-cpu-support/api-spec.md)
- [Data Models](docs/architecture/2026-02-02-6809-cpu-support/data-models.md)
- [ADR-001: Big-Endian Byte Order](docs/architecture/2026-02-02-6809-cpu-support/adr-001-big-endian.md)

**Product Requirements:**
- [PRD: 6809 CPU Support](docs/product/6809-cpu-support/prd.md)

**Reference Documentation:**
- [6809 Instruction Set Reference](docs/reference/6809/instruction-set.md)
- [EDTASM+ Syntax Reference](docs/reference/6809/edtasm-syntax.md)

**Implementation:**
- Task Packet: `.ai/tasks/2026-02-02_6809-cpu-support/`
- Beads Task: xasm++-7iy

**External References:**
- vasm-ext EDTASM module: `~/Projects/Vintage/tools/vasm-ext/syntax/edtasm/`
- Motorola 6809 Datasheet
- 6809 Assembly Language Programming (Lance Leventhal)
- Disk EDTASM Remastered (Tandy)

---

## Conclusion

Phase 1 (Architecture Design) is **complete and ready for implementation**. The technical architecture provides:

1. **Clear component design** - Cpu6809 plugin and EdtasmSyntaxParser
2. **Detailed specifications** - APIs, data models, encodings
3. **Implementation guidance** - Design patterns, algorithms, strategies
4. **Validation plan** - Unit tests, binary compatibility, performance benchmarks
5. **Risk mitigation** - Identified risks with mitigation strategies

The design leverages xasm++'s existing plugin architecture, extends it consistently, and provides comprehensive documentation for Engineers to implement confidently.

**Recommendation:** PROCEED to Phase 2 (Core Implementation - MVP)

---

**Architect:** Claude
**Date:** 2026-02-03
**Status:** Phase 1 Complete ✅
**Next:** Phase 2 - Engineer Implementation

---

**End of Phase 1 Summary**
