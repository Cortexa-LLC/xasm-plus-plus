# Task Contract: God-Class Refactoring Phase 2 - ExpressionParser Integration

**Task ID:** 2026-02-09_god-class-phase2-integration
**Beads Task:** xasm++-6nra
**Created:** 2026-02-09
**Priority:** P1 (CRITICAL)

Integrate ExpressionParser component (from Phase 1) into all 3 god-class syntax parsers, eliminating ~1,200 LOC of duplicated code.

## Context

**Phase 1 Complete:** ExpressionParser component created (xasm++-9nox)
- 620 LOC implementation
- 25/25 tests passing
- Ready for integration

**Current State:** 3 god-class parsers with duplicated expression logic
- Z80Universal: 2,285 LOC (12 expression methods)
- Merlin: 1,764 LOC (12 expression methods)
- SCMASM: 1,330 LOC (12 expression methods)
- **Total duplication:** ~1,200 LOC (37%)

## Problem Statement

All 3 syntax parsers have their own expression parsing implementations:
- Same recursive descent logic duplicated 3x
- Bug fixes must be applied 3 times
- New operators require triple implementation
- High maintenance burden

## Objective

Replace duplicated expression parsing code in all 3 parsers with calls to shared ExpressionParser component.

## Success Criteria

✓ All 3 parsers use ExpressionParser for expression evaluation
✓ ~1,200 LOC removed from god-class parsers
✓ Zero duplicated expression parsing logic
✓ All existing tests pass (no behavioral changes)
✓ Zero compiler warnings
✓ Zero performance regression
✓ Code duplication reduced from 37% to <5%

## Acceptance Criteria

### Phase 1: Z80Universal Integration (Pilot) (3-4 hours)
- [ ] Analyze existing Z80Universal expression methods
- [ ] Implement ISymbolResolver for Z80Universal
- [ ] Add ExpressionParser member to Z80UniversalSyntaxParser
- [ ] Replace expression methods with ExpressionParser calls
- [ ] Run Z80 tests - all must pass
- [ ] Verify no behavioral changes (compare outputs)
- [ ] Remove old expression parsing methods
- [ ] Commit checkpoint
- [ ] Measure LOC reduction

### Phase 2: Merlin Integration (3-4 hours)
- [ ] Analyze existing Merlin expression methods
- [ ] Implement ISymbolResolver for Merlin
- [ ] Add ExpressionParser member to MerlinSyntaxParser
- [ ] Replace expression methods with ExpressionParser calls
- [ ] Run Merlin tests - all must pass
- [ ] Remove old expression parsing methods
- [ ] Commit checkpoint
- [ ] Measure LOC reduction

### Phase 3: SCMASM Integration (3-4 hours)
- [ ] Analyze existing SCMASM expression methods
- [ ] Implement ISymbolResolver for SCMASM
- [ ] Add ExpressionParser member to ScmasmSyntaxParser
- [ ] Replace expression methods with ExpressionParser calls
- [ ] Run SCMASM tests - all must pass
- [ ] Remove old expression parsing methods
- [ ] Commit checkpoint
- [ ] Measure LOC reduction

### Phase 4: Final Verification (1-2 hours)
- [ ] All parser tests passing (Z80, Merlin, SCMASM)
- [ ] Full test suite passes
- [ ] Zero compiler warnings
- [ ] LOC metrics documented:
  - Before: 5,379 LOC total
  - After: ~4,179 LOC total (1,200 LOC reduction)
  - Duplication: 37% → <5%
- [ ] Performance benchmarks (no regression)
- [ ] Code duplication analysis confirms <5%

## Implementation Strategy

### ISymbolResolver Pattern

Each parser must implement the ISymbolResolver interface to provide symbol resolution to ExpressionParser:

```cpp
// Each parser implements this interface
class Z80UniversalSyntaxParser : public ISyntaxParser, public ISymbolResolver {
  ExpressionParser expr_parser_;

public:
  Z80UniversalSyntaxParser() : expr_parser_(this) {}

  // ISymbolResolver implementation
  bool TryResolveSymbol(const std::string& name, uint32_t& value) override {
    return symbol_table_.TryGetSymbol(name, value);
  }

  // Replace old expression methods
  uint32_t EvaluateExpression(const std::string& expr) {
    return expr_parser_.Parse(expr);
  }
};
```

### Migration Pattern

For each parser:

1. **Add includes:**
   ```cpp
   #include "xasm++/common/expression_parser.h"
   ```

2. **Inherit ISymbolResolver:**
   ```cpp
   class ParserName : public ISyntaxParser, public ISymbolResolver
   ```

3. **Add member:**
   ```cpp
   ExpressionParser expr_parser_;
   ```

4. **Initialize in constructor:**
   ```cpp
   ParserName() : expr_parser_(this) {}
   ```

5. **Implement ISymbolResolver:**
   ```cpp
   bool TryResolveSymbol(const std::string& name, uint32_t& value) override {
     return symbol_table_.TryGetSymbol(name, value);
   }
   ```

6. **Replace expression methods:**
   ```cpp
   // Before: 50+ lines of parsing logic
   uint32_t ParseExpression() { /* complex code */ }

   // After: Single delegation
   uint32_t ParseExpression() {
     return expr_parser_.Parse(current_expression_);
   }
   ```

7. **Remove old methods** (12 per parser)

## Metrics

**Before Integration:**
| Parser | LOC | Expression Methods | Expression LOC |
|--------|-----|-------------------|----------------|
| Z80Universal | 2,285 | 12 | ~450 |
| Merlin | 1,764 | 12 | ~400 |
| SCMASM | 1,330 | 12 | ~350 |
| **Total** | **5,379** | **36** | **~1,200** |

**After Integration:**
| Parser | LOC | Expression Methods | Expression LOC |
|--------|-----|-------------------|----------------|
| Z80Universal | ~1,835 | 0 (delegated) | 0 |
| Merlin | ~1,364 | 0 (delegated) | 0 |
| SCMASM | ~980 | 0 (delegated) | 0 |
| ExpressionParser | 426 | 12 | 426 |
| **Total** | **~4,605** | **12** | **426** |

**Improvement:**
- **LOC reduction:** 774 LOC (14.4% overall, 37% in expression code)
- **Duplication removed:** 1,200 → 0 LOC
- **Methods centralized:** 36 → 12
- **Single source of truth:** 3 implementations → 1

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Behavioral changes | High | Compare outputs before/after, comprehensive testing |
| Test failures | High | Pilot with Z80 first, incremental integration |
| Performance regression | Low | Benchmark before/after, ExpressionParser already optimized |
| Symbol resolution differences | Medium | Test symbol resolution for each parser individually |

## Files to Modify

**Z80Universal:**
- `include/xasm++/syntax/z80_universal_syntax.h` (add ISymbolResolver, ExpressionParser member)
- `src/syntax/z80_universal/z80_universal_syntax.cpp` (replace methods, remove old code)

**Merlin:**
- `include/xasm++/syntax/merlin_syntax.h`
- `src/syntax/merlin/merlin_syntax.cpp`

**SCMASM:**
- `include/xasm++/syntax/scmasm_syntax.h`
- `src/syntax/scmasm/scmasm_syntax.cpp`

## Estimated Effort

- Z80Universal integration: 3-4 hours
- Merlin integration: 3-4 hours
- SCMASM integration: 3-4 hours
- Verification: 1-2 hours

**Total:** 10-14 hours (1.5-2 days)

## Dependencies

- **Requires:** Phase 1 complete (xasm++-9nox) ✓
- **Blocks:** Phase 3-5 of god-class refactoring

## Next Phases

After Phase 2 completes:
- **Phase 3:** Number Parser extraction (8-12 hours)
- **Phase 4:** Conditional Assembly extraction (12-16 hours)
- **Phase 5:** Macro Processor extraction (16-24 hours)
- **Phase 6:** Directive Handler extraction (20-32 hours)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer (refactoring specialist)
- **Reviewer:** Architect + Code quality review
- **Tester:** Comprehensive regression testing

**Approved:** Orchestrator 2026-02-09

---

**This integration realizes the 37% duplication removal identified in Phase 0 investigation.**
