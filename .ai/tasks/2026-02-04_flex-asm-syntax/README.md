# FLEX ASM Syntax Implementation Task

**Beads Task:** xasm++-k5s
**Status:** Planning Complete - Ready to Execute
**Priority:** P2 (Medium)
**Estimated Effort:** 45-60 hours

---

## Quick Summary

Implement FLEX ASM09 syntax support for Motorola 6809, adding macro processor and conditional assembly capabilities to xasm++.

### Key Features to Implement

1. **Basic Directives** - ORG, EQU, SET, FCB, FDB, FCC, RMB, END, SETDP, NAM, TTL, etc.
2. **Macro Processor** - MACRO/ENDM with parameter substitution
3. **Conditional Assembly** - IFC/ENDC for conditional compilation
4. **Advanced Directives** - LIB, REG, OPT, ERR, RPT/ENDR

### Documentation

- **Contract:** `00-contract.md` - Full requirements and acceptance criteria
- **Plan:** `10-plan.md` - Detailed implementation strategy (5 phases)
- **Reference:** `docs/reference/flex/flex-asm-syntax-reference.md` - FLEX syntax documentation
- **Manual:** `docs/reference/flex/ASM09-6809-assembler.pdf` - Original FLEX ASM manual

---

## Implementation Phases

### Phase 1: Basic Directives (10-15 hours)
- Create FlexAsmSyntax class extending MotorolaBaseSyntax
- Implement core directives (ORG, EQU, SET, FCB, FDB, FCC, RMB, END, SETDP)
- Implement listing control (NAM, TTL, STTL, PAG, SPC)
- Register with syntax registry
- Integration tests

### Phase 2: Macro Processor (15-20 hours)
- Create MacroProcessor class
- Parse MACRO/ENDM directives
- Implement parameter substitution
- Handle local labels in macros (.LABEL)
- Support nested macro calls
- Comprehensive testing

### Phase 3: Conditional Assembly (8-10 hours)
- Create ConditionalStack class
- Implement IFC/ENDC directives
- Evaluate conditions (symbol defined, expression != 0)
- Handle nested conditionals
- Integration tests

### Phase 4: Advanced Directives (10-15 hours)
- LIB - Library file inclusion
- REG - Register list definition
- OPT - Assembler options
- ERR - Force error
- RPT/ENDR - Repeat blocks
- Testing

### Phase 5: Documentation (2-5 hours)
- Update user guides
- Create developer documentation
- Final integration testing

---

## Quick Start (For Engineer)

1. **Read the contract:** `00-contract.md`
2. **Review the plan:** `10-plan.md`
3. **Check the reference:** `docs/reference/flex/flex-asm-syntax-reference.md`
4. **Start with Phase 1:** Begin implementing basic directives
5. **Update work log:** Document progress in `20-work-log.md`

---

## Testing Strategy

- **Unit Tests:** Each directive, macro processor, conditionals
- **Integration Tests:** Real FLEX programs
- **Compatibility Tests:** Compare with FLEX ASM output (if possible)

**Target Coverage:** 80%+ overall, 100% for macro processor

---

## Success Criteria

- ✅ CLI accepts `--cpu 6809 --syntax flex`
- ✅ Can assemble simple FLEX programs
- ✅ Macros work with parameter substitution
- ✅ Conditional assembly correctly skips/includes code
- ✅ All unit tests pass (target: 100+ tests)
- ✅ Integration tests pass
- ✅ Documentation complete

---

## References

- **FLEX ASM Manual:** ASM09 6809 Optimizing Assembler Version 3.2
- **PDF Location:** `docs/reference/flex/ASM09-6809-assembler.pdf`
- **Text Extract:** `docs/reference/flex/ASM09-6809-assembler.txt`
- **Syntax Reference:** `docs/reference/flex/flex-asm-syntax-reference.md`
- **Docusaurus:** Reference now published in documentation site

---

## Dependencies

**No Blockers** - All dependencies complete:
- ✅ 6809 CPU plugin (complete)
- ✅ MotorolaBaseSyntax (exists)
- ✅ Expression evaluator (exists)
- ✅ Multi-pass assembler (exists)

---

## Deliverables

**Source Code:**
- `include/xasm++/syntax/flex_syntax.h`
- `src/syntax/flex/flex_syntax.cpp`
- `include/xasm++/syntax/macro_processor.h`
- `src/syntax/flex/macro_processor.cpp`

**Tests:**
- `tests/unit/test_flex_syntax.cpp`
- `tests/unit/test_flex_macros.cpp`
- `tests/unit/test_flex_conditionals.cpp`
- `tests/integration/flex/*.asm`

**Documentation:**
- Syntax reference (✅ already created)
- API documentation
- User guide examples

---

## Status: ✅ Ready to Execute

All planning complete. Ready to spawn engineer agent to begin implementation.

**Command to spawn agent:**
```bash
bd show xasm++-k5s
agent engineer xasm++-k5s --stream
```
