# Task Contract: FLEX ASM Phase 2 - Macro Processor

**Beads Task:** xasm++-684
**Date:** 2026-02-05
**Priority:** P2 (Medium)
**Depends On:** Phase 1 complete (xasm++-k5s) ✅

---

## Objective

Implement the FLEX ASM09 macro processor, enabling macro definition with MACRO/ENDM directives and macro expansion with parameter substitution.

## Background

**Current State:**
- ✅ FLEX ASM Phase 1 complete (13 tests passing)
- ✅ Basic directives implemented: ORG, END, EQU, SET, FCB, FDB, FCC, RMB, SETDP, NAM, TTL, STTL, PAGE, SPC
- ✅ FlexAsmSyntax class extends MotorolaBaseSyntax
- ❌ Phase 2 TODO stubs at lines 368, 375, 380 in `src/syntax/motorola/flex_syntax.cpp`

**Goal:** Add macro definition and expansion capabilities to FLEX ASM, matching the FLEX Assembler specification.

**Why This Matters:**
- Macros are a core FLEX ASM feature used heavily in FLEX programs
- Enables code reuse and abstraction in 6809 assembly
- Required for assembling real FLEX source code

## Requirements

### Functional Requirements

**FR-1: MACRO/ENDM Directive Parsing**
- Parse MACRO directive with optional parameter list
- Syntax: `NAME MACRO [param1,param2,...,param8]`
- Collect macro body lines until ENDM
- Store macro definition for later expansion
- Maximum 8 parameters per macro (FLEX spec)

**FR-2: Macro Expansion**
- Recognize macro invocations during normal parsing
- Parse macro arguments (comma-separated)
- Substitute parameters in macro body with arguments
- Expand macro into source lines for parsing
- Support nested macro calls (macro calling another macro)

**FR-3: Local Label Handling**
- Recognize local labels (`.LABEL` notation) in macro body
- Generate unique label names per macro invocation
- Pattern: `.LOOP` → `.LOOP_001`, `.LOOP_002`, etc.
- Prevents label conflicts across multiple macro invocations

**FR-4: Parameter Substitution**
- Replace parameter names with arguments in macro body
- Handle missing arguments (empty string substitution)
- Case-sensitive parameter matching
- Preserve whitespace and formatting

**FR-5: Error Handling**
- Error: Nested MACRO (not allowed)
- Error: ENDM without MACRO
- Error: Undefined macro invocation
- Error: Wrong parameter count in macro call
- Clear error messages with line numbers

### Non-Functional Requirements

**NFR-1: Performance**
- Macro expansion in linear time O(n) where n = lines
- No performance regression vs Phase 1
- Efficient string substitution (avoid excessive copying)

**NFR-2: Code Quality**
- Clean separation: MacroProcessor class (separate from FlexAsmSyntax)
- Unit tests for macro processor (>80% coverage)
- Integration tests with real macro programs
- Follows xasm++ coding standards

**NFR-3: Compatibility**
- Matches FLEX ASM09 macro behavior
- Compatible with existing Phase 1 directives
- No breaking changes to FlexAsmSyntax API

## Acceptance Criteria

### Must Have (P0)

- [ ] **AC-1:** MACRO directive parses correctly with 0-8 parameters
- [ ] **AC-2:** ENDM directive ends macro definition correctly
- [ ] **AC-3:** Macro invocations expand with parameter substitution
- [ ] **AC-4:** Local labels (.LABEL) are unique per invocation
- [ ] **AC-5:** Nested macro calls work (macro calling another macro)
- [ ] **AC-6:** Error messages for invalid macro usage
- [ ] **AC-7:** All unit tests pass (target: 20+ new tests)
- [ ] **AC-8:** Integration test with real FLEX macro program
- [ ] **AC-9:** Phase 1 tests still pass (no regression)
- [ ] **AC-10:** Code reviewed and approved

### Should Have (P1)

- [ ] MacroProcessor class has comprehensive unit tests
- [ ] Documentation for macro processor design
- [ ] Performance profiling shows no regression

### Nice to Have (P2)

- [ ] Macro recursion detection (macro calling itself)
- [ ] Macro expansion depth limit (safety)
- [ ] Better error messages with macro expansion trace

## Code Architecture

### New Components

**MacroProcessor class:**
```cpp
// include/xasm++/syntax/macro_processor.h
class MacroProcessor {
public:
    struct MacroDefinition {
        std::string name;
        std::vector<std::string> parameters;
        std::vector<std::string> body;
        int definition_line;
    };

    void DefineMacro(const std::string& name,
                    const std::vector<std::string>& parameters,
                    const std::vector<std::string>& body);

    bool IsMacro(const std::string& name) const;

    std::vector<std::string> ExpandMacro(
        const std::string& name,
        const std::vector<std::string>& arguments,
        int invocation_line);

private:
    std::unordered_map<std::string, MacroDefinition> macros_;
    int expansion_counter_ = 0;

    std::string SubstituteParameters(
        const std::string& line,
        const MacroDefinition& macro,
        const std::vector<std::string>& arguments);

    std::string MakeLocalLabelUnique(
        const std::string& label,
        int expansion_id);
};
```

**FlexAsmSyntax enhancements:**
```cpp
enum class ParserMode {
    Normal,
    InMacroDefinition,
};

class FlexAsmSyntax : public MotorolaBaseSyntax {
private:
    MacroProcessor macro_processor_;
    ParserMode mode_ = ParserMode::Normal;
    MacroDefinition current_macro_;

    void HandleMACRO(const std::string& name,
                    const std::vector<std::string>& params);
    void HandleENDM();
    void CollectMacroLine(const std::string& line);
};
```

## Testing Strategy

### Unit Tests

**test_macro_processor.cpp:**
- Define simple macro (no parameters)
- Define macro with parameters (1-8)
- Expand macro with correct argument count
- Expand macro with missing arguments
- Local label uniquification
- Nested macro calls
- Error cases (undefined macro, wrong arg count)

**test_flex_macros.cpp:**
- MACRO directive parsing
- ENDM directive parsing
- Macro expansion during assembly
- Integration with Phase 1 directives
- Error handling

### Integration Tests

**tests/integration/flex/macro_simple.asm:**
```asm
; Simple macro without parameters
DELAY   MACRO
        LDA     #$FF
        NOP
        ENDM

        DELAY
        DELAY
```

**tests/integration/flex/macro_params.asm:**
```asm
; Macro with parameters
MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM

        MOVB    $80,$90
        MOVB    $A0,$B0
```

**tests/integration/flex/macro_local.asm:**
```asm
; Macro with local labels
COPY    MACRO   COUNT
        LDX     #COUNT
.LOOP   LDA     ,X+
        STA     ,Y+
        LEAX    -1,X
        BNE     .LOOP
        ENDM

        COPY    10      ; .LOOP becomes .LOOP_001
        COPY    20      ; .LOOP becomes .LOOP_002
```

## Documentation Requirements

### Developer Documentation
- [ ] Architecture document: `docs/architecture/flex-macro-processor.md`
- [ ] API documentation for MacroProcessor class
- [ ] Update FlexAsmSyntax API docs

### User Documentation
- [ ] Update FLEX ASM reference with macro examples
- [ ] Add macro programming guide

## Estimated Effort

**Total:** 15-20 hours

**Breakdown:**
- MacroProcessor class implementation: 4-5 hours
- MACRO/ENDM parsing: 3-4 hours
- Macro expansion integration: 5-6 hours
- Integration testing: 3-5 hours

## Risks and Mitigations

**Risk 1: Parameter substitution complexity**
- **Impact:** High - Core feature
- **Probability:** Medium
- **Mitigation:** Start simple, add features incrementally
- **Fallback:** Ship basic macros (no parameters) first

**Risk 2: Local label uniqueness**
- **Impact:** Medium - Important for multi-invocation macros
- **Probability:** Low
- **Mitigation:** Use simple counter-based approach
- **Fallback:** Require explicit labels (no local labels)

**Risk 3: Testing without real FLEX code**
- **Impact:** Medium - May miss edge cases
- **Probability:** Medium
- **Mitigation:** Create comprehensive synthetic tests from FLEX manual
- **Fallback:** Clearly document known limitations

## Dependencies

**Completed:**
- ✅ Phase 1 complete (basic directives)
- ✅ FlexAsmSyntax class exists
- ✅ MotorolaBaseSyntax provides parsing foundation
- ✅ Expression evaluator exists

**Blocks:**
- Phase 3 (Conditional Assembly) - waits for Phase 2

**No blockers** - Ready to implement.

## Deliverables

1. **Source Code:**
   - `include/xasm++/syntax/macro_processor.h`
   - `src/syntax/flex/macro_processor.cpp`
   - Updates to `src/syntax/motorola/flex_syntax.cpp`

2. **Tests:**
   - `tests/unit/test_macro_processor.cpp`
   - `tests/unit/test_flex_macros.cpp`
   - `tests/integration/flex/macro_*.asm` (test programs)

3. **Documentation:**
   - `docs/architecture/flex-macro-processor.md`
   - Updated FLEX ASM reference

4. **Work Log:**
   - Progress tracking in `20-work-log.md`

## Success Criteria

1. ✅ Can define macros with MACRO/ENDM
2. ✅ Macro expansion works with parameter substitution
3. ✅ Local labels are unique per invocation
4. ✅ Nested macro calls work correctly
5. ✅ All unit tests pass (target: 20+ new tests)
6. ✅ Integration tests with macro programs pass
7. ✅ Phase 1 tests still pass (no regression)
8. ✅ Code reviewed and approved
9. ✅ Documentation complete

## References

- **Original Task:** `.ai/tasks/2026-02-04_flex-asm-syntax/`
- **Phase 1 Implementation:** `src/syntax/motorola/flex_syntax.cpp`
- **FLEX ASM Manual:** `docs/reference/flex/ASM09-6809-assembler.pdf`
- **Syntax Reference:** `docs/reference/flex/flex-asm-syntax-reference.md`
- **MotorolaBaseSyntax:** `src/syntax/motorola_base_syntax.cpp`

---

**Status:** ✅ Ready to Plan
**Next Step:** Create implementation plan (10-plan.md)
