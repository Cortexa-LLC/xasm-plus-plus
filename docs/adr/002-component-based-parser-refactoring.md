# ADR-002: Component-Based Parser Refactoring

**Status:** Proposed  
**Date:** 2026-02-09  
**Deciders:** Architect Agent, pending User approval

---

## Context

The current syntax parser implementations exhibit severe god-class anti-patterns:

- **Z80UniversalSyntaxParser**: 1,842 LOC, 50 methods, 23 directive handlers
- **MerlinSyntaxParser**: 1,764 LOC, 44 methods, 30 directive handlers
- **ScmasmSyntaxParser**: 1,330 LOC, 30 methods, 13 directive handlers

**Problems:**

1. **Single Responsibility Principle Violation**: Each class handles 9+ distinct responsibilities:
   - Line tokenization
   - Label parsing
   - Directive processing (20-30 directives each)
   - Macro expansion
   - Expression evaluation
   - Symbol table management
   - Segment management
   - Conditional assembly
   - Error reporting

2. **Testability**: God-classes are difficult to unit test:
   - Cannot test directives in isolation
   - Cannot mock dependencies
   - High coupling between concerns
   - Current test coverage: ~60%

3. **Maintainability**: Large monolithic classes are hard to understand and modify:
   - 1,800+ LOC per file
   - Cyclomatic complexity: 50-80 per method
   - Adding new directives requires modifying multiple methods
   - Bug fixes risk unintended side effects

4. **Code Duplication**: Expression evaluation duplicated across all three parsers:
   - 9 expression parsing methods × 3 parsers = 27 duplicated methods
   - ~900 LOC of duplicated expression parsing logic
   - Bug fixes require changes in 3 places

5. **Extensibility**: Adding new parsers or directives is expensive:
   - Must copy/paste entire god-class structure
   - Difficult to identify reusable components
   - High barrier to entry for new contributors

---

## Decision

**We will refactor the god-class syntax parsers into component-based architecture with clear separation of concerns.**

### Component Decomposition

**Core Components:**
1. **Lexer** (~150 LOC) - Tokenization and comment handling
2. **Parser** (~200 LOC) - Label/operand parsing
3. **DirectiveDispatcher** (~100 LOC) - Route directives to handlers
4. **DirectiveHandlers** (~600 LOC) - Individual directive implementations (Strategy pattern)
5. **ExpressionEvaluator** (~300 LOC) - Shared expression parsing (eliminates duplication)
6. **MacroProcessor** (~400 LOC) - Macro definition and expansion
7. **SegmentManager** (~150 LOC) - Segment state management
8. **ConditionalStack** (~100 LOC) - IF/ELSE/ENDIF handling
9. **LabelScopeManager** (~100 LOC) - Global/local label scoping
10. **Coordinating Parser** (~300 LOC) - Orchestrate components (Facade pattern)

**Total:** ~2,500 LOC split across focused components vs. 4,936 LOC in 3 monolithic classes

### Design Patterns

- **Strategy Pattern**: Directive handlers as pluggable strategies
- **Facade Pattern**: Parser coordinates components, maintains public API
- **Dependency Injection**: Components injected via constructor
- **Context Object**: `DirectiveContext` passed to handlers (avoid long parameter lists)
- **Registry Pattern**: `DirectiveDispatcher` maintains handler registry

### Component Interfaces

All components implement clear interfaces:
- `ILexer`: Tokenization interface
- `IParser`: Parsing interface
- `IDirectiveHandler`: Directive handling interface

This enables:
- Unit testing with mocks
- Runtime swapping of implementations
- Clear contracts between components

---

## Rationale

### Why Component-Based Over Other Approaches

**Alternative 1: Template-Based (CRTP)**
- Pros: Zero virtual call overhead
- Cons: More complex, harder to test, less flexible
- Why rejected: Performance overhead negligible for assembler workload

**Alternative 2: Keep God-Classes, Add Comments**
- Pros: No refactoring needed
- Cons: Doesn't solve fundamental design problems
- Why rejected: Doesn't improve testability or maintainability

**Alternative 3: Extract Only Directives**
- Pros: Partial improvement, less work
- Cons: Still leaves large god-classes, doesn't address duplication
- Why rejected: Incomplete solution

### Why Component-Based is Superior

1. **Single Responsibility Principle**: Each class has one focused purpose
2. **Testability**: Each component independently testable (~85% coverage target)
3. **Maintainability**: Smaller classes (~300 LOC max) easier to understand
4. **Code Reuse**: Shared components eliminate 900+ LOC duplication
5. **Extensibility**: Add directives without modifying parser (Open/Closed)
6. **Clarity**: Component boundaries make architecture explicit

---

## Consequences

### Positive

1. **Improved Testability**
   - Each component unit testable in isolation
   - Can mock dependencies
   - Target 85%+ test coverage (up from 60%)

2. **Reduced Complexity**
   - Average class size: 140 LOC (down from 1,645 LOC)
   - Max cyclomatic complexity: <15 per method (down from 50-80)
   - Largest component: 400 LOC (down from 1,842 LOC)

3. **Code Reuse**
   - Shared `ExpressionEvaluator`: Eliminates 900 LOC duplication
   - Shared `ConditionalStack`, `SegmentManager`: Consistent behavior
   - Reusable directive handlers where applicable

4. **Maintainability**
   - Clear separation of concerns
   - Easy to locate and fix bugs
   - Reduced risk of unintended side effects

5. **Extensibility**
   - Add new directives: Implement handler, register in dispatcher
   - Add new parser: Implement lexer/parser, reuse components
   - Open/Closed Principle: Extend without modifying existing code

6. **Backward Compatibility**
   - Public API unchanged
   - All existing tests pass
   - No breaking changes for clients

### Negative

1. **Performance Overhead**
   - Virtual call overhead: ~5-10ns per call
   - Expected total overhead: <5% increase in parse time
   - Mitigation: Use `final` keyword, inline hot paths
   - Trade-off: Acceptable for massive maintainability improvement

2. **Memory Overhead**
   - Multiple component objects: ~800 bytes per parser instance
   - Increase from 500 bytes (god-class) to 1,300 bytes
   - Mitigation: Parsers are short-lived (per-file)
   - Trade-off: Negligible (atoms dominate memory)

3. **Implementation Effort**
   - Estimated 10-15 days for complete refactoring
   - Risk: Medium on first parser, low on subsequent
   - Mitigation: Phased approach, comprehensive testing

4. **Learning Curve**
   - New contributors must understand component architecture
   - Mitigation: Clear documentation, examples
   - Long-term: Easier to learn 10 small components than 1 large god-class

### Neutral

1. **More Files**
   - Before: 3 god-class files
   - After: ~30 component files
   - Impact: Better organization, easier to navigate

2. **Indirection**
   - Components communicate through interfaces
   - Impact: Clearer boundaries, explicit dependencies

---

## Implementation Plan

### Phase 1: Extract Shared Components (2-3 days)
- Extract `ExpressionEvaluator`
- Extract `ConditionalStack`
- Extract `SegmentManager`
- Extract `LabelScopeManager`
- Unit test each component (90%+ coverage)

### Phase 2: Refactor Z80UniversalSyntaxParser (3-4 days)
- Create component interfaces
- Implement Z80-specific lexer/parser
- Implement directive handlers (23 handlers)
- Wire components via DI
- Verify all tests pass (regression suite)

### Phase 3: Refactor MerlinSyntaxParser (2-3 days)
- Implement Merlin-specific lexer/parser
- Implement directive handlers (30 handlers)
- Reuse shared components
- Verify all tests pass

### Phase 4: Refactor ScmasmSyntaxParser (2-3 days)
- Implement SCMASM-specific lexer/parser
- Implement directive handlers (13 handlers)
- Reuse shared components
- Verify all tests pass

### Phase 5: Consolidate and Document (1-2 days)
- Identify common directive handlers
- Optimize hot paths (profile-guided)
- Update documentation
- Performance benchmark

**Total Effort:** 10-15 days

---

## Alternatives Considered

### Alternative 1: Incremental Refactoring (Extract Methods)

**Approach:** Break up god-class methods without creating separate components.

**Pros:**
- Lower risk
- Faster initial implementation
- No performance overhead

**Cons:**
- Doesn't solve fundamental design problems
- Still violates SRP
- Limited testability improvement
- Doesn't enable code reuse

**Why Not Chosen:** Doesn't achieve architectural goals

---

### Alternative 2: Template-Based Component System (CRTP)

**Approach:** Use Curiously Recurring Template Pattern to avoid virtual calls.

```cpp
template<typename Derived>
class ParserBase {
    void ParseLine() {
        auto lexer = static_cast<Derived*>(this)->GetLexer();
        // ...
    }
};

class Z80Parser : public ParserBase<Z80Parser> {
    Z80Lexer GetLexer() { return lexer_; }
};
```

**Pros:**
- Zero virtual call overhead
- All inlined by compiler

**Cons:**
- More complex implementation
- Harder to test (can't mock easily)
- Less flexible (compile-time binding)
- Steeper learning curve

**Why Not Chosen:** 
- Performance overhead negligible for assembler (not in hot loop)
- Complexity not justified
- Testability more important than marginal performance gain

---

### Alternative 3: Scripting Language for Directives

**Approach:** Implement directives in embedded scripting language (Lua, Python).

**Pros:**
- Extremely flexible
- User-extensible
- Dynamic directive loading

**Cons:**
- Massive implementation effort
- Runtime dependency
- Performance overhead (much larger than virtual calls)
- Complexity explosion

**Why Not Chosen:** Over-engineering, not justified for current needs

---

### Alternative 4: Keep God-Classes, Focus on Other Refactoring

**Approach:** Leave parsers as-is, refactor other areas of codebase.

**Pros:**
- Zero risk of regression in parsers
- Can address other technical debt
- No performance impact

**Cons:**
- Parsers remain unmaintainable
- Testing remains difficult
- Duplication remains
- New parsers remain expensive to add

**Why Not Chosen:** Parsers are core of assembler, must be maintainable

---

## Risks and Mitigation

### Risk 1: Performance Degradation

**Probability:** Low  
**Impact:** Medium

**Mitigation:**
- Benchmark before/after with real assembly files
- Profile hot paths (likely in expression evaluation)
- Use `final` keyword to enable devirtualization
- Inline frequently-called methods
- Accept <5% overhead as acceptable trade-off

**Contingency:**
- If overhead >5%, profile and optimize specific hot paths
- Consider CRTP for hot paths only if needed
- Worst case: Keep god-class for one parser, refactor others

### Risk 2: Regression in Behavior

**Probability:** Medium  
**Impact:** High

**Mitigation:**
- Comprehensive regression test suite (existing tests)
- Binary compatibility tests (compare output byte-for-byte)
- Refactor one parser at a time (isolate issues)
- No changes to public API
- Code review of each component

**Contingency:**
- Git bisect to identify regression
- Fix component or revert specific change
- Tests prevent shipping broken code

### Risk 3: Incomplete Refactoring

**Probability:** Low  
**Impact:** Medium

**Mitigation:**
- Phased approach (complete one parser before next)
- Clear success criteria per phase
- Tests must pass before proceeding

**Contingency:**
- If time-constrained, keep refactored parsers and leave others
- Parsers are independent (no coupling)
- Document which parsers refactored vs. legacy

---

## Success Metrics

### Quantitative

**Code Size:**
- Target: 50% reduction in total parser code
- Before: 4,936 LOC (3 parsers)
- After: ~2,500 LOC (components + facades)

**Test Coverage:**
- Target: 85%+ overall, 100% critical paths
- Before: ~60%
- After: >85%

**Cyclomatic Complexity:**
- Target: Max 15 per method
- Before: 50-80 per method
- After: <15 per method

**Performance:**
- Target: <5% overhead
- Measure: Parse time on real assembly files
- Accept: Performance is acceptable trade-off

### Qualitative

- ✅ Single Responsibility Principle adhered
- ✅ Open/Closed Principle enabled
- ✅ Each component independently testable
- ✅ Code intent is clear and documented
- ✅ New directives easy to add
- ✅ New parsers easier to implement

---

## Related Decisions

- [ADR-001: 6809 Big Endian Encoding](001-6809-big-endian.md) - Precedent for ADR usage
- Future ADR: Shared Expression Evaluator implementation details
- Future ADR: Directive handler plugin system

---

## References

- [Architecture Document](../architecture/2026-02-09-god-class-refactoring/architecture.md)
- [API Specification](../architecture/2026-02-09-god-class-refactoring/api-spec.md)
- [Data Models](../architecture/2026-02-09-god-class-refactoring/data-models.md)
- [God-Class Investigation](.ai/tasks/2026-02-09_god-class-investigation/)
- [xasm++ Architecture Overview](../architecture/00-overview.md)
- SOLID Principles: `.ai-pack/quality/engineering-standards.md`

---

## Approval

**Proposed By:** Architect Agent  
**Date:** 2026-02-09

**Awaiting Approval:**
- [ ] User/Stakeholder
- [ ] Technical Lead

**Revision History:**
- v1.0 - 2026-02-09 - Initial proposal

---

**Status:** Proposed (Pending Approval)
