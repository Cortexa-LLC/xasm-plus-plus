# Task Contract: FLEX ASM Syntax Processor for 6809

**Beads Task:** xasm++-k5s
**Date:** 2026-02-04
**Priority:** P2 (Medium)

---

## Objective

Implement FLEX ASM09 syntax support for the Motorola 6809, enabling xasm++ to assemble programs written for the FLEX Operating System.

## Background

FLEX ASM09 is the assembler for the FLEX Disk Operating System (FLEX DOS) developed by Technical Systems Consultants (TSC). It's a sophisticated 2-pass assembler with capabilities beyond our current EDTASM implementation:

- **Macro preprocessor** with parameter substitution
- **Conditional assembly** (IFC/ENDC)
- **Library inclusion** (LIB directive)
- **Advanced listing control** (NAM, TTL, STTL, PAG, SPC)
- **Repeat blocks** (RPT/ENDR)

**Current State:**
- ✅ 6809 CPU support complete (462/462 tests passing)
- ✅ EDTASM syntax support complete
- ✅ MotorolaBaseSyntax foundation available for code reuse
- ✅ Reference documentation captured (ASM09-6809-assembler.pdf)

**Goal:** Enable assembling FLEX programs with full compatibility.

## Requirements

### Phase 1: Basic Directives (Foundation)

Implement core FLEX directives compatible with EDTASM base:

**Program Structure:**
- `ORG` - Set origin address
- `END` - End of source (with optional entry point)
- `SETDP` - Set direct page register value

**Symbol Definition:**
- `EQU` - Define constant symbol
- `SET` - Define variable symbol (reassignable)

**Data Definition:**
- `FCB` - Form Constant Byte (8-bit)
- `FDB` - Form Double Byte (16-bit, big-endian)
- `FCC` - Form Constant Character (string with delimiter)
- `RMB` - Reserve Memory Bytes

**Listing Control (Basic):**
- `NAM` / `TTL` - Set program title
- `STTL` - Set subtitle
- `PAG` - Force page break
- `SPC` - Insert blank lines

**Test Coverage:**
- Each directive with valid syntax
- Error cases (invalid parameters)
- Integration with 6809 CPU encoding

### Phase 2: Macro Processor (New Capability)

Implement macro definition and expansion:

**Directives:**
- `MACRO` - Begin macro definition with parameters
- `ENDM` - End macro definition

**Features:**
- Positional parameter substitution (`SRC`, `DEST`, etc.)
- Local labels within macros (`.LABEL` notation)
- Nested macro calls
- Parameter validation

**Example:**
```asm
MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM

        MOVB    $80,$90      ; Expands to LDA $80 / STA $90
```

**Test Coverage:**
- Simple macros (no parameters)
- Macros with parameters (1-8 params)
- Local labels in macros
- Nested macro calls
- Error cases (undefined macros, wrong parameter count)

### Phase 3: Conditional Assembly

Implement conditional compilation:

**Directives:**
- `IFC` - If Condition (symbol defined)
- `ENDC` - End Conditional

**Features:**
- Symbol existence checking
- Expression evaluation for conditions
- Nested conditionals
- Skip code blocks when condition false

**Example:**
```asm
        IFC     DEBUG
        JSR     TRACE
        ENDC
```

**Test Coverage:**
- Simple conditionals (defined/undefined symbols)
- Nested conditionals
- Expression-based conditions
- Error cases (unmatched IFC/ENDC)

### Phase 4: Advanced Directives

Implement remaining FLEX directives:

**Library Support:**
- `LIB` - Include external source file

**Register Lists:**
- `REG` - Define register lists for PSHS/PULS

**Assembler Control:**
- `OPT` - Set assembler options (C=cycle count, L=listing, P=pagination)
- `ERR` - Force assembler error with message

**Repeat Blocks:**
- `RPT` - Repeat block N times
- `ENDR` - End repeat block

**Test Coverage:**
- Each directive with typical usage
- Error cases
- Integration tests

## Code Architecture

### Extend MotorolaBaseSyntax

```
MotorolaBaseSyntax (existing ~1000 lines)
  ├─ Common parsing (~60% reuse)
  ├─ Basic directives (ORG, EQU, FCB, etc.)
  └─ Expression evaluation

FlexAsmSyntax (new ~800-1200 lines)
  ├─ Inherit from MotorolaBaseSyntax
  ├─ Macro processor (~300 lines)
  ├─ Conditional assembly (~200 lines)
  ├─ Advanced directives (~200 lines)
  └─ FLEX-specific parsing (~100 lines)
```

### New Components

**MacroProcessor class:**
```cpp
class MacroProcessor {
    struct Macro {
        std::string name;
        std::vector<std::string> parameters;
        std::vector<std::string> body;
    };

    void DefineMacro(const std::string& name,
                    const std::vector<std::string>& params,
                    const std::vector<std::string>& body);

    std::vector<std::string> ExpandMacro(
        const std::string& name,
        const std::vector<std::string>& args);
};
```

**ConditionalStack:**
```cpp
class ConditionalStack {
    std::stack<bool> conditions_;

    void PushCondition(bool condition);
    void PopCondition();
    bool ShouldAssemble() const;  // All conditions must be true
};
```

## Acceptance Criteria

### Functional Requirements

- [ ] All Phase 1 directives work correctly
- [ ] Macro processor handles parameters and expansion
- [ ] Conditional assembly skips/includes code correctly
- [ ] Advanced directives implemented
- [ ] Compatible with existing 6809 CPU encoding
- [ ] All FLEX addressing modes supported

### Quality Requirements

- [ ] Unit tests for each directive (80%+ coverage)
- [ ] Integration tests with real FLEX source code
- [ ] Macro processor has comprehensive tests
- [ ] Conditional assembly has edge case tests
- [ ] Code follows xasm++ coding standards
- [ ] Documentation updated

### Performance Requirements

- [ ] Macro expansion is efficient (linear time)
- [ ] Conditional assembly has minimal overhead
- [ ] Assembly speed comparable to EDTASM

## Testing Strategy

### Unit Tests

Create test files for each phase:
- `test_flex_directives.cpp` - Basic directives
- `test_flex_macros.cpp` - Macro processor
- `test_flex_conditionals.cpp` - Conditional assembly
- `test_flex_advanced.cpp` - Advanced directives

### Integration Tests

Use real FLEX source code examples:
- Simple FLEX programs (10-50 lines)
- Programs with macros
- Programs with conditional assembly
- Complete FLEX applications (if available)

### Compatibility Tests

Compare output with original FLEX assembler (if possible):
- Same source → same binary
- Validate instruction encoding
- Validate address resolution

## Documentation Requirements

### User Documentation

- [x] FLEX ASM syntax reference (docs/reference/flex/)
- [ ] Update Quick Start guide with FLEX example
- [ ] Add FLEX to supported syntax modes list
- [ ] Create FLEX programming guide

### Developer Documentation

- [ ] Architecture document for macro processor
- [ ] API documentation for FlexAsmSyntax class
- [ ] Extension guide for future syntax modes

## Estimated Effort

**Total:** 45-60 hours

**Breakdown:**
- **Phase 1:** Basic directives - 10-15 hours
  - Directive parsing and handling
  - Unit tests
  - Integration with existing infrastructure

- **Phase 2:** Macro processor - 15-20 hours
  - Macro definition parsing
  - Parameter substitution engine
  - Local label management
  - Comprehensive testing

- **Phase 3:** Conditional assembly - 8-10 hours
  - Condition evaluation
  - Code block skipping
  - Nested conditional handling
  - Testing

- **Phase 4:** Advanced directives - 10-15 hours
  - Library inclusion (file I/O)
  - Register list parsing
  - Assembler options
  - Repeat blocks
  - Testing

- **Documentation:** 2-5 hours
  - Update user guides
  - Write developer docs

## Risks and Mitigations

**Risk 1: Macro processor complexity**
- **Impact:** High - Core feature
- **Mitigation:** Start with simple macros, add features incrementally
- **Fallback:** Ship without macros in v1, add in v2

**Risk 2: FLEX source code availability**
- **Impact:** Medium - Testing harder without real code
- **Mitigation:** Create synthetic test cases based on manual
- **Fallback:** Implement to spec, validate when code available

**Risk 3: Incompatibility with original assembler**
- **Impact:** Medium - User expectations
- **Mitigation:** Document known differences
- **Fallback:** Clearly mark as "FLEX-compatible" not "FLEX-exact"

## Dependencies

**Completed:**
- ✅ 6809 CPU plugin (complete)
- ✅ MotorolaBaseSyntax (exists)
- ✅ Expression evaluator (exists)
- ✅ Multi-pass assembler (exists)

**No blockers** - Ready to implement.

## Deliverables

1. **Source Code:**
   - `include/xasm++/syntax/flex_syntax.h`
   - `src/syntax/flex/flex_syntax.cpp`
   - `src/syntax/flex/macro_processor.cpp` (new)
   - `src/syntax/flex/macro_processor.h` (new)

2. **Tests:**
   - `tests/unit/test_flex_syntax.cpp`
   - `tests/unit/test_flex_macros.cpp`
   - `tests/integration/flex/` (test programs)

3. **Documentation:**
   - FLEX ASM reference (already created)
   - Quick start examples
   - API documentation

4. **Build Integration:**
   - Updated CMakeLists.txt
   - CLI option: `--syntax flex`

## Success Criteria

1. ✅ Can assemble simple FLEX programs
2. ✅ Macros work with parameter substitution
3. ✅ Conditional assembly correctly skips/includes code
4. ✅ All unit tests pass (target: 100+ tests)
5. ✅ Integration tests with real FLEX code pass
6. ✅ Documentation complete and published
7. ✅ CLI accepts `--cpu 6809 --syntax flex`

## References

- **FLEX ASM Manual:** `docs/reference/flex/ASM09-6809-assembler.pdf`
- **Syntax Reference:** `docs/reference/flex/flex-asm-syntax-reference.md`
- **EDTASM Implementation:** `src/syntax/edtasm/` (reference pattern)
- **MotorolaBaseSyntax:** `src/syntax/motorola_base_syntax.cpp` (base class)

---

**Status:** ✅ Ready to Plan
**Next Step:** Create implementation plan (10-plan.md)
