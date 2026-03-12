# Task Contract: God-Class Refactoring Phase 1 - Expression Parser

**Task ID:** 2026-02-09_god-class-phase1-expression-parser
**Beads Task:** xasm++-9nox
**Created:** 2026-02-09
**Priority:** P1 (CRITICAL)

Extract expression parsing logic from 3 god-class syntax parsers into shared ExpressionParser component.

## Context

From god-class investigation (xasm++-uy8u):
- **3 syntax parsers** with 5,379 total LOC
- **37% code duplication** (~2,000 LOC)
- **12 recursive descent methods** duplicated across parsers
- **Expression parsing** is the single largest source of duplication

## Problem Statement

All 3 syntax parsers (Z80Universal, Merlin, SCMASM) have duplicated expression parsing logic:
- Recursive descent parsing for binary operators (+, -, *, /, %, &, |, ^, <<, >>)
- Unary operators (-, ~, !, <, >, LOWNYBBLE, HIGHNYBBLE)
- Parenthesized expressions
- Function calls (LOW, HIGH, etc.)
- Operator precedence handling

This duplication means:
- Bug fixes must be applied 3 times
- New operators require triple implementation
- Testing is 3x more expensive
- Inconsistent behavior across parsers

## Objective

Create shared `ExpressionParser` class that all 3 syntax parsers can use, eliminating ~1,200 LOC of duplication.

## Success Criteria

✓ ExpressionParser class created and tested independently
✓ All 3 parsers refactored to use ExpressionParser
✓ Zero code duplication in expression parsing logic
✓ All existing tests pass (no behavioral changes)
✓ Test coverage ≥90% for ExpressionParser
✓ ~1,200 LOC removed from god-class parsers
✓ Zero compiler warnings

## Acceptance Criteria

### Phase 1: Analysis & Design (4 hours)
- [ ] Analyze existing expression parsing methods in all 3 parsers
- [ ] Document common expression grammar (BNF or similar)
- [ ] Design ExpressionParser API:
  - Input: token stream or raw string
  - Output: parsed expression tree or value
  - Error handling strategy
- [ ] Identify parser-specific customization points
- [ ] Document strategy pattern for syntax-specific behaviors

### Phase 2: Test First (TDD) (4-6 hours)
- [ ] Create `tests/unit/test_expression_parser.cpp`
- [ ] Write comprehensive test suite:
  - Binary operators (all 9: +, -, *, /, %, &, |, ^, <<, >>)
  - Unary operators (-, ~, !, <, >, LOW, HIGH, etc.)
  - Operator precedence
  - Parenthesized expressions
  - Function calls
  - Edge cases (overflow, divide by zero, invalid syntax)
- [ ] Tests initially RED (implementation doesn't exist)

### Phase 3: Implementation (6-8 hours)
- [ ] Create `include/xasm++/common/expression_parser.h`
- [ ] Create `src/common/expression_parser.cpp`
- [ ] Implement recursive descent parser:
  ```cpp
  class ExpressionParser {
  public:
    explicit ExpressionParser(ISymbolResolver* resolver);

    // Main API
    uint32_t Parse(const std::string& expr);
    bool TryParse(const std::string& expr, uint32_t& result);

  private:
    // Recursive descent methods
    uint32_t ParseExpression();
    uint32_t ParseLogicalOr();
    uint32_t ParseLogicalAnd();
    uint32_t ParseBitwiseOr();
    uint32_t ParseBitwiseXor();
    uint32_t ParseBitwiseAnd();
    uint32_t ParseShift();
    uint32_t ParseAddSub();
    uint32_t ParseMulDiv();
    uint32_t ParseUnary();
    uint32_t ParsePrimary();
  };
  ```
- [ ] All ExpressionParser tests GREEN
- [ ] Refactor for clarity

### Phase 4: Integration - Z80Universal (Pilot) (2-3 hours)
- [ ] Add ExpressionParser member to Z80UniversalSyntaxParser
- [ ] Replace 12 expression methods with delegations to ExpressionParser
- [ ] Run Z80 tests - all must pass
- [ ] Verify no behavioral changes
- [ ] Remove old expression methods from Z80UniversalSyntaxParser
- [ ] Commit checkpoint

### Phase 5: Integration - Merlin (2-3 hours)
- [ ] Add ExpressionParser member to MerlinSyntaxParser
- [ ] Replace expression methods with delegations
- [ ] Run Merlin tests - all must pass
- [ ] Remove old expression methods
- [ ] Commit checkpoint

### Phase 6: Integration - SCMASM (2-3 hours)
- [ ] Add ExpressionParser member to ScmasmSyntaxParser
- [ ] Replace expression methods with delegations
- [ ] Run SCMASM tests - all must pass
- [ ] Remove old expression methods
- [ ] Commit checkpoint

### Phase 7: Verification (1-2 hours)
- [ ] All parser tests passing (Z80, Merlin, SCMASM)
- [ ] ExpressionParser test coverage ≥90%
- [ ] Zero compiler warnings
- [ ] LOC metrics:
  - Z80Universal: Reduced by ~400 LOC
  - Merlin: Reduced by ~350 LOC
  - SCMASM: Reduced by ~300 LOC
- [ ] Code duplication: <5% (down from 37%)
- [ ] Performance benchmarks (no regression)

## Design Approach

### Expression Grammar (Simplified BNF)

```bnf
expression    := logical_or
logical_or    := logical_and ('||' logical_and)*
logical_and   := bitwise_or ('&&' bitwise_or)*
bitwise_or    := bitwise_xor ('|' bitwise_xor)*
bitwise_xor   := bitwise_and ('^' bitwise_and)*
bitwise_and   := shift ('&' shift)*
shift         := add_sub (('<<' | '>>') add_sub)*
add_sub       := mul_div (('+' | '-') mul_div)*
mul_div       := unary (('*' | '/' | '%') unary)*
unary         := ('-' | '~' | '!' | '<' | '>') unary | primary
primary       := NUMBER | SYMBOL | '(' expression ')' | function_call
function_call := IDENTIFIER '(' expression ')'
```

### Strategy Pattern for Customization

```cpp
// Allow syntax-specific symbol resolution
class ISymbolResolver {
public:
  virtual ~ISymbolResolver() = default;
  virtual bool TryResolveSymbol(const std::string& name, uint32_t& value) = 0;
};

// Each parser provides its own resolver
class Z80SymbolResolver : public ISymbolResolver {
  // Resolves from symbol table
};
```

## Metrics

**Before Extraction:**
| Parser | LOC | Expression Methods | Duplication |
|--------|-----|-------------------|-------------|
| Z80Universal | 2,285 | 12 (~450 LOC) | ~400 LOC |
| Merlin | 1,764 | 12 (~400 LOC) | ~350 LOC |
| SCMASM | 1,330 | 12 (~350 LOC) | ~300 LOC |
| **Total** | **5,379** | **36** | **~1,050 LOC** |

**After Extraction:**
| Component | LOC | Methods |
|-----------|-----|---------|
| ExpressionParser | ~500 | 12 |
| Z80Universal (reduced) | 1,885 | ~51 |
| Merlin (reduced) | 1,414 | ~28 |
| SCMASM (reduced) | 1,030 | ~15 |
| **Total** | **4,329** | **~106** |

**Improvement:**
- **LOC reduction:** 1,050 LOC (19.5%)
- **Duplication removed:** 37% → ~8%
- **Methods reduced:** 36 → 12 (centralized)

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Regression bugs | High | Comprehensive tests, incremental integration |
| Performance degradation | Low | Benchmark before/after, inline hot paths |
| API doesn't fit all parsers | Medium | Pilot with Z80 first, adjust API if needed |
| Incomplete extraction | Medium | Thorough analysis phase, cross-check all parsers |

## Files to Create/Modify

**New Files:**
- `include/xasm++/common/expression_parser.h`
- `src/common/expression_parser.cpp`
- `tests/unit/test_expression_parser.cpp`

**Modified Files:**
- `include/xasm++/syntax/z80_universal_syntax.h` (reduce methods)
- `src/syntax/z80_universal/z80_universal_syntax.cpp` (delegate to ExpressionParser)
- `include/xasm++/syntax/merlin_syntax.h` (reduce methods)
- `src/syntax/merlin/merlin_syntax.cpp` (delegate)
- `include/xasm++/syntax/scmasm_syntax.h` (reduce methods)
- `src/syntax/scmasm/scmasm_syntax.cpp` (delegate)
- `src/CMakeLists.txt` (add expression_parser.cpp)
- `tests/unit/CMakeLists.txt` (add test_expression_parser.cpp)

## Estimated Effort

- Analysis & Design: 4 hours
- TDD (Tests + Implementation): 10-14 hours
- Integration (3 parsers): 6-9 hours
- Verification: 1-2 hours

**Total:** 21-29 hours (3-4 days)

## Dependencies

- None (can start immediately)
- Based on god-class investigation (xasm++-uy8u) - completed

## Next Phases

After Phase 1 completes:
- **Phase 2:** Number Parser extraction (8-12 hours)
- **Phase 3:** Conditional Assembly extraction (12-16 hours)
- **Phase 4:** Macro Processor extraction (16-24 hours)
- **Phase 5:** Directive Handler extraction (20-32 hours)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer (with strong refactoring skills)
- **Reviewer:** Architect + Code quality review
- **Tester:** Comprehensive test validation

**Approved:** Orchestrator 2026-02-09

---

**This is the highest-priority architectural improvement. Start immediately after approval.**
