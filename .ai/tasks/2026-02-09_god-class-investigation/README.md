# God-Class Investigation - Syntax Parser Refactoring

**Task ID:** xasm++-uy8u
**Priority:** P1
**Created:** 2026-02-09

## Overview

Investigate severe god-class violations in syntax parser implementations and propose refactoring strategies to improve maintainability and adherence to Single Responsibility Principle.

## God-Class Findings

### 1. z80_universal_syntax (SEVERE)
- **Header:** 420 lines
- **Implementation:** 1,842 lines (!!)
- **Public Methods:** 50
- **Directive Handlers:** 23
- **Responsibilities:** Parsing, 23 directive handlers, macro expansion, expression evaluation, segment management, symbol handling

### 2. merlin_syntax (SEVERE)
- **Header:** 265 lines
- **Implementation:** 1,764 lines (!!)
- **Public Methods:** 44
- **Directive Handlers:** 30
- **Responsibilities:** Parsing, 30 directive handlers, macro expansion, expression evaluation, symbol handling

### 3. scmasm_syntax (LARGE)
- **Header:** 450 lines
- **Implementation:** 1,330 lines
- **Public Methods:** 30
- **Directive Handlers:** 13
- **Responsibilities:** Parsing, 13 directive handlers, expression evaluation, symbol handling

## Single Responsibility Violations

Each syntax class is handling multiple responsibilities:
1. Line parsing (tokenization)
2. Directive handling (20-30 different directives)
3. Macro expansion
4. Expression evaluation
5. Symbol table management
6. Address calculation
7. Segment management (Z80)
8. Error reporting
9. Output generation

## Proposed Investigation Areas

1. **Identify Common Patterns** - What responsibilities are shared across all parsers?
2. **Directive Handler Extraction** - Can we extract directive handlers into separate classes?
3. **Expression Evaluator Extraction** - Should expression evaluation be a separate service?
4. **Macro Processor Integration** - Is macro handling properly separated?
5. **Segment Manager Pattern** - Should segment management follow a consistent pattern?

## Success Criteria

- Comprehensive analysis of all three god-classes
- Identification of refactoring opportunities
- Proposed class decomposition strategy
- Estimated effort for refactoring
- Prioritized refactoring plan
- Risk assessment for breaking changes

## References

- Architectural Review: `.ai/tasks/2026-02-09_architectural-review-patterns/architecture-review.md`
- SOLID Principles: `.ai-pack/quality/engineering-standards.md`
