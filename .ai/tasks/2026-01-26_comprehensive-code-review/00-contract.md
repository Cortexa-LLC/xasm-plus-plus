# Task Contract

**Task ID:** 2026-01-26_comprehensive-code-review
**Created:** 2026-01-26
**Beads Task:** xasm++-c7w
**Requestor:** User
**Assigned Role:** Reviewer
**Workflow:** Standard (Code Quality Review)

---

## Task Description

Perform a comprehensive code quality review of the xasm++ codebase against ai-pack clean code standards.

### Background and Context

xasm++ has grown significantly with recent additions:
- Merlin syntax parser implementation
- Branch relaxation (6502 CPU plugin)
- Multiple directive handlers (PUT, HEX, USR, DS, etc.)
- Command-line interface
- Core assembler logic

After rapid feature development, it's time to assess code quality and identify refactoring opportunities to ensure long-term maintainability.

### Current State

- 429 tests passing (100%)
- 20/29 PoP files assembling (69%)
- Active development with feature additions
- No systematic code quality review completed

### Desired State

- Comprehensive code quality assessment documented
- Anti-patterns identified and catalogued
- Refactoring opportunities prioritized
- Technical debt mapped
- Actionable recommendations for improvement

---

## Success Criteria

```
✓ Full codebase reviewed against clean code standards
✓ Anti-patterns identified and documented
✓ Long functions (>50 lines) catalogued with refactoring suggestions
✓ Complex functions (high cyclomatic complexity) identified
✓ Refactoring opportunities prioritized by impact
✓ Technical debt report generated
✓ Zero-regression verification (all 429 tests still pass)
```

---

## Acceptance Criteria

### Functional Requirements
- [ ] Review all source files in `src/` directory
- [ ] Review all header files in `include/` directory
- [ ] Assess against ai-pack clean code standards
- [ ] Identify violations of SOLID principles
- [ ] Document code smells (long methods, large classes, feature envy, etc.)
- [ ] Check for proper error handling patterns
- [ ] Review resource management (RAII compliance)
- [ ] Assess naming conventions consistency

### Quality Requirements
- [ ] Findings documented in review report
- [ ] Findings prioritized (P1: Critical, P2: High, P3: Medium, P4: Low)
- [ ] Refactoring recommendations actionable
- [ ] No false positives (all findings legitimate)
- [ ] Report includes code examples
- [ ] Zero regressions (all tests still pass)

### Deliverables
- [ ] Comprehensive review report (30-review.md)
- [ ] Technical debt summary (docs/technical-debt/2026-01-26-assessment.md)
- [ ] Prioritized refactoring backlog
- [ ] Quick wins list (high impact, low effort)

---

## Review Scope

### Core Components (MANDATORY)

1. **Assembler Core** (`src/core/`)
   - assembler.cpp - Main assembly logic
   - section.cpp - Section management
   - symbol_table.cpp - Symbol resolution

2. **Syntax Parsers** (`src/syntax/`)
   - merlin_syntax.cpp - Merlin parser (PRIMARY FOCUS)
   - Other syntax handlers

3. **CPU Plugins** (`src/cpu/`)
   - cpu_6502.cpp - 6502 implementation
   - Branch relaxation logic

4. **Command-Line Interface** (`src/main.cpp`)
   - Argument parsing
   - File I/O
   - Error handling

5. **Data Structures** (`include/xasm++/`)
   - All header files
   - Interface design

### Quality Standards (ai-pack Framework)

**Primary References:**
- `.ai-pack/quality/clean-code/functions.md`
- `.ai-pack/quality/clean-code/naming.md`
- `.ai-pack/quality/clean-code/error-handling.md`
- `.ai-pack/quality/clean-code/solid-principles.md`
- `.ai-pack/quality/engineering-standards.md`

**Key Standards:**
1. **Functions** - Max 50 lines, single responsibility, max 3 parameters
2. **Naming** - Clear, intention-revealing, consistent
3. **Error Handling** - RAII, exceptions vs error codes, validation
4. **SOLID** - SRP, OCP, LSP, ISP, DIP compliance
5. **Complexity** - Max cyclomatic complexity of 10

---

## Anti-Patterns to Detect

### Code Smells
- Long methods (>50 lines)
- Long parameter lists (>3 parameters)
- Large classes (>500 lines)
- Duplicate code
- Dead code
- Magic numbers/strings
- Feature envy
- Primitive obsession
- Switch statements (consider polymorphism)

### Architectural Issues
- Tight coupling
- Missing abstractions
- God objects
- Circular dependencies
- Violation of separation of concerns

### C++ Specific
- Raw pointers (prefer smart pointers)
- Missing const-correctness
- Non-RAII resource management
- Unnecessary copies (missing move semantics)
- Inconsistent error handling

---

## Deliverable Format

### Review Report Structure

```markdown
# Comprehensive Code Quality Review
**Date:** 2026-01-26
**Reviewer:** [Agent ID]

## Executive Summary
- Overall quality rating (1-5)
- Key findings count
- Priority breakdown

## Findings by Priority

### P1: Critical (Immediate Action Required)
[Issues requiring immediate attention]

### P2: High (Address Soon)
[Important improvements]

### P3: Medium (Plan for Future Sprint)
[Moderate improvements]

### P4: Low (Nice to Have)
[Optional improvements]

## Findings by Component
[Organized by file/component]

## Technical Debt Summary
- Total debt estimate
- Quick wins (high impact, low effort)
- Long-term improvements

## Recommendations
1. Immediate actions
2. Short-term roadmap
3. Long-term architecture improvements
```

---

## Prioritization Framework

**P1 (Critical):**
- Security vulnerabilities
- Memory leaks
- Crashes/undefined behavior
- Major SOLID violations blocking extension

**P2 (High):**
- Functions >100 lines
- Cyclomatic complexity >15
- Duplicate code (significant)
- Missing error handling

**P3 (Medium):**
- Functions >50 lines
- Complexity >10
- Naming inconsistencies
- Missing const-correctness

**P4 (Low):**
- Style inconsistencies
- Optional optimizations
- Documentation gaps

---

## Dependencies

**Required:**
- Access to all source files
- ai-pack clean code standards
- Static analysis tools (optional but recommended)

**Blocks:**
- Future refactoring tasks
- Architecture improvements
- Code cleanup initiatives

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| False positives | Medium | Manual verification of each finding |
| Scope creep | High | Stick to defined components |
| Implementation temptation | High | Review only - no code changes |
| Overwhelming report | Medium | Prioritize findings effectively |

---

## Reference Standards

### ai-pack Clean Code Standards
1. **Functions**: `.ai-pack/quality/clean-code/functions.md`
   - Max 50 lines
   - Single Responsibility Principle
   - Command-Query Separation
   - Max 3 parameters

2. **Naming**: `.ai-pack/quality/clean-code/naming.md`
   - Intention-revealing
   - Avoid disinformation
   - Pronounceable
   - Searchable

3. **Error Handling**: `.ai-pack/quality/clean-code/error-handling.md`
   - RAII principle
   - Exception safety
   - Fail-fast principle

4. **SOLID Principles**: `.ai-pack/quality/clean-code/solid-principles.md`
   - Single Responsibility
   - Open-Closed
   - Liskov Substitution
   - Interface Segregation
   - Dependency Inversion

5. **C++ Modern Practices**: `.ai-pack/quality/clean-code/cpp-modern-practices.md`
   - Smart pointers over raw pointers
   - RAII for resource management
   - Move semantics
   - Const-correctness

---

## Stakeholders

- **xasm++ Maintainers:** Need code quality visibility
- **Future Contributors:** Need clean, maintainable codebase
- **PoP Compatibility Project:** Depends on stable foundation

---

## Estimated Effort

- **Review Execution:** 2-3 hours (thorough analysis)
- **Report Writing:** 1-2 hours (documentation)
- **Prioritization:** 30 minutes
- **Total:** 3.5-5.5 hours

---

## Notes

**Important Constraints:**
- **READ ONLY** - This is a review task, NOT a refactoring task
- No code changes permitted during review
- Focus on identifying issues, not fixing them
- Document findings objectively with code examples
- Prioritize based on impact and effort

**Follow-up:**
- After review, create separate refactoring tasks for P1/P2 findings
- Quick wins can be addressed immediately
- Long-term improvements go into backlog

**Success Metric:**
- A clear, actionable roadmap for code quality improvements
- Prioritized backlog of refactoring tasks
- No surprises (all technical debt visible)

---

## Output Location

**Primary Report:** `.ai/tasks/2026-01-26_comprehensive-code-review/30-review.md`

**Technical Debt Report:** `docs/technical-debt/2026-01-26-assessment.md`
- Executive summary
- Debt by component
- Prioritized backlog
- Quick wins list

**Work Log:** `.ai/tasks/2026-01-26_comprehensive-code-review/20-work-log.md`
- Review progress tracking
- Findings as discovered
- Time spent per component
