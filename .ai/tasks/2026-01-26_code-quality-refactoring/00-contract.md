# Task Contract

**Task ID:** 2026-01-26_code-quality-refactoring
**Created:** 2026-01-26
**Beads Task:** xasm++-hxm
**Requestor:** User (Code Review Follow-up)
**Assigned Role:** Engineer
**Workflow:** Refactor
**Source Review:** `.ai/tasks/2026-01-26_comprehensive-code-review/`

---

## Task Description

Address all 19 findings from the comprehensive code quality review to bring xasm++ codebase up to clean code standards.

### Background and Context

A comprehensive code review identified 19 issues across the codebase:
- **2 Critical** (security and correctness)
- **8 Major** (maintainability and design)
- **9 Minor** (style and documentation)

The codebase has excellent fundamentals (197 tests passing, good architecture) but needs focused refactoring on key areas, primarily `assembler.cpp` and `cpu_6502.cpp`.

### Current State

- 429 tests passing (100%)
- 20/29 PoP files assembling (69%)
- Code review grade: C+
- 19 documented issues requiring fixes
- Core functionality stable

### Desired State

- All 19 review findings addressed
- Zero critical/major issues remaining
- All 429 tests still passing (zero regressions)
- Code review grade: A or better
- Maintainable, secure, extensible codebase

---

## Success Criteria

```
✓ All 2 critical issues resolved
✓ All 8 major issues resolved
✓ All 9 minor issues resolved (or documented as "won't fix" with rationale)
✓ Zero test regressions (429/429 tests passing)
✓ Zero new compiler warnings
✓ Code review re-assessment shows grade A or better
✓ All changes follow TDD methodology
```

---

## Issues to Address

### 🔴 Critical Issues (P1 - Must Fix)

#### [C1] ParseHex() Security Risk
**Location:** `assembler.cpp:10-15`
**Issue:** No input validation before calling `std::stoul`, can throw exceptions on malformed input, silent failure returns 0
**Risk:** Crash on malicious input, silent assembly errors
**Fix Required:**
- Add input validation (check for valid hex characters)
- Handle exceptions properly
- Return error instead of 0 on failure
- Add error diagnostics

**Acceptance Criteria:**
- [ ] Input validation added (regex or character check)
- [ ] Exception handling added
- [ ] Error reporting instead of silent 0 return
- [ ] Unit tests for malformed input
- [ ] Fuzz test added

---

#### [C2] Unchecked Dynamic Casts
**Location:** `assembler.cpp:184, 195, 215` (multiple locations)
**Issue:** Multiple `dynamic_pointer_cast` calls without null checks
**Risk:** Potential null pointer dereference, crash
**Fix Required:**
- Add null checks after every `dynamic_pointer_cast`
- Throw meaningful error on cast failure
- Document expected types

**Acceptance Criteria:**
- [ ] All `dynamic_pointer_cast` calls have null checks
- [ ] Error messages explain what went wrong
- [ ] Unit tests for incorrect cast scenarios

---

### 🟡 Major Issues (P2 - Should Fix)

#### [M1] God Method Anti-Pattern
**Location:** `Assembler::Assemble()` - 344 lines
**Issue:** Single method handles encoding, symbol resolution, error handling, convergence (violates Single Responsibility Principle)
**Impact:** Hard to understand, test, and maintain
**Fix Required:**
- Extract sub-methods with clear responsibilities:
  - `ResolveSymbols()` - Symbol resolution
  - `EncodeInstructions()` - Instruction encoding
  - `CheckConvergence()` - Multi-pass convergence
  - `HandleErrors()` - Error collection/reporting
- Keep `Assemble()` as coordinator (max 50 lines)

**Acceptance Criteria:**
- [ ] `Assemble()` reduced to <50 lines
- [ ] 4-6 extracted methods with single responsibilities
- [ ] Each method has unit tests
- [ ] All 429 tests still pass

---

#### [M2] Massive If-Else Chain
**Location:** `assembler.cpp` - 208-line instruction dispatch
**Issue:** Sequential if-else for instruction lookup (O(n) performance, hard to extend)
**Impact:** Slow, violates Open-Closed Principle
**Fix Required:**
- Replace with hash map lookup table (`std::unordered_map<std::string, InstructionHandler>`)
- O(1) lookup instead of O(n)
- Easier to add new instructions

**Acceptance Criteria:**
- [ ] If-else chain replaced with hash map
- [ ] Performance improved (benchmark)
- [ ] All 429 tests still pass
- [ ] Adding new instruction is simple (demonstrate)

---

#### [M3] Magic Numbers
**Location:** Throughout `assembler.cpp` and `cpu_6502.cpp`
**Issue:** Opcodes hardcoded without explanation (`0xA9`, `0xF0`, `0x20`, etc.)
**Impact:** Hard to understand, error-prone
**Fix Required:**
- Extract opcodes to named constants or enum
- Group by instruction type
- Add comments explaining each

**Acceptance Criteria:**
- [ ] All magic opcodes replaced with named constants
- [ ] Constants organized logically (enum class or namespace)
- [ ] Comments explain what each opcode does
- [ ] All 429 tests still pass

---

#### [M4] No Input Validation
**Location:** String manipulation throughout codebase
**Issue:** String operations without bounds checking, buffer overflow potential
**Fix Required:**
- Add bounds checking before string operations
- Use `std::string_view` where appropriate
- Validate indices before access

**Acceptance Criteria:**
- [ ] All string accesses have bounds checking
- [ ] Unit tests for edge cases (empty strings, overflow)
- [ ] Static analysis (clang-tidy) passes

---

#### [M5] Exception Safety
**Location:** Catch-all handlers throughout
**Issue:** Catch-all handlers lose context, hard to debug
**Fix Required:**
- Catch specific exceptions
- Re-throw with additional context
- Log exceptions properly

**Acceptance Criteria:**
- [ ] Specific exception types caught
- [ ] Context preserved (use `std::throw_with_nested` or custom exceptions)
- [ ] Meaningful error messages
- [ ] Unit tests verify exception behavior

---

#### [M6] God Class
**Location:** `Cpu6502` - 100+ public methods
**Issue:** Single class with too many responsibilities (violates SRP and ISP)
**Impact:** Hard to test, understand, extend
**Fix Required:**
- Split into focused classes:
  - `Cpu6502Core` - Basic 6502 instructions
  - `Cpu6502AddressingModes` - Addressing mode decoding
  - `Cpu6502BranchHandler` - Branch logic & relaxation
  - `Cpu6502Validator` - Instruction validation
- Use composition or delegation

**Acceptance Criteria:**
- [ ] `Cpu6502` split into 3-4 focused classes
- [ ] Each class has <30 public methods
- [ ] All 429 tests still pass
- [ ] No performance regression

---

#### [M7] Code Duplication
**Location:** 50+ Encode methods with 95% identical switch statements
**Issue:** Massive code duplication, maintenance nightmare
**Fix Required:**
- Extract common pattern into template or helper
- Use table-driven approach where possible
- Reduce duplication to <10%

**Acceptance Criteria:**
- [ ] Duplication reduced from 95% to <10%
- [ ] Common patterns extracted to shared utilities
- [ ] All 429 tests still pass
- [ ] Easier to add new addressing modes (demonstrate)

---

#### [M8] Missing Error Reporting
**Location:** Silent failures throughout
**Issue:** Errors fail silently with no diagnostics
**Fix Required:**
- Add error reporting mechanism
- Return `Result<T, Error>` or use exceptions consistently
- Provide helpful error messages with line numbers, context

**Acceptance Criteria:**
- [ ] All failure points report errors
- [ ] Error messages include file, line, context
- [ ] Consistent error handling strategy
- [ ] Unit tests verify error messages

---

### 🔵 Minor Issues (P3 - Consider)

#### [N1] Missing Const-Correctness
**Location:** Multiple methods and parameters
**Issue:** Methods that don't modify state aren't marked `const`
**Fix Required:** Add `const` qualifiers where appropriate

**Acceptance Criteria:**
- [ ] All non-modifying methods marked `const`
- [ ] Const-correctness verified with compiler flags

---

#### [N2] Missing Documentation
**Location:** Public interfaces lack Doxygen comments
**Fix Required:** Add Doxygen comments to all public APIs

**Acceptance Criteria:**
- [ ] All public methods have Doxygen comments
- [ ] Doxygen builds without warnings

---

#### [N3] Inconsistent Naming
**Location:** Mix of camelCase, snake_case, PascalCase
**Fix Required:** Standardize to project convention

**Acceptance Criteria:**
- [ ] Single naming convention applied consistently
- [ ] Style guide documented

---

#### [N4-N9] Additional Minor Issues
(Style, formatting, optional optimizations - see review for details)

**Acceptance Criteria:**
- [ ] Addressed or documented as "won't fix" with rationale

---

## Implementation Plan

### Phase 1: Critical Fixes (1-2 days)
**Goal:** Fix security and correctness issues

**Tasks:**
1. [C1] Add ParseHex validation and error handling
2. [C2] Add null checks after dynamic_pointer_cast
3. Write tests for malformed input scenarios
4. Run full test suite (expect 429 passing)

**Deliverables:**
- Secure ParseHex implementation
- Safe dynamic casts
- Fuzz tests added

---

### Phase 2: God Method Refactoring (2-3 days)
**Goal:** Break down `Assemble()` into manageable pieces

**Tasks:**
1. [M1] Extract `ResolveSymbols()` method
2. [M1] Extract `EncodeInstructions()` method
3. [M1] Extract `CheckConvergence()` method
4. [M1] Extract `HandleErrors()` method
5. [M1] Reduce `Assemble()` to coordinator (<50 lines)
6. Write unit tests for each extracted method
7. Run full test suite

**Deliverables:**
- `Assemble()` <50 lines
- 4+ focused methods with single responsibilities
- Improved testability

---

### Phase 3: Dispatch Table (1 day)
**Goal:** Replace if-else chain with hash map

**Tasks:**
1. [M2] Create instruction handler interface
2. [M2] Implement hash map dispatch table
3. [M2] Replace if-else chain
4. [M2] Benchmark performance improvement
5. Run full test suite

**Deliverables:**
- O(1) instruction lookup
- Easier extensibility
- Performance metrics

---

### Phase 4: Constants & Validation (1 day)
**Goal:** Extract magic numbers and add validation

**Tasks:**
1. [M3] Extract opcode constants to enum/namespace
2. [M4] Add string bounds checking
3. [M5] Improve exception handling
4. [M8] Add error reporting mechanism
5. Run full test suite

**Deliverables:**
- Named opcode constants
- Robust input validation
- Comprehensive error reporting

---

### Phase 5: God Class Refactoring (2-3 days)
**Goal:** Split `Cpu6502` into focused classes

**Tasks:**
1. [M6] Design class decomposition
2. [M6] Extract `Cpu6502AddressingModes`
3. [M6] Extract `Cpu6502BranchHandler`
4. [M6] Extract `Cpu6502Validator`
5. [M6] Update `Cpu6502` to use composition
6. Run full test suite

**Deliverables:**
- 3-4 focused classes
- Each class <30 methods
- Improved maintainability

---

### Phase 6: Code Duplication (1 day)
**Goal:** Eliminate duplication in Encode methods

**Tasks:**
1. [M7] Analyze duplication patterns
2. [M7] Extract common template/helper
3. [M7] Refactor Encode methods to use shared code
4. Run full test suite

**Deliverables:**
- Duplication <10%
- Shared encoding utilities

---

### Phase 7: Minor Issues & Documentation (1 day)
**Goal:** Address minor issues and polish

**Tasks:**
1. [N1] Add const-correctness
2. [N2] Add Doxygen documentation
3. [N3] Standardize naming conventions
4. [N4-N9] Address or document minor issues
5. Final test suite run
6. Generate Doxygen documentation

**Deliverables:**
- Const-correct code
- Complete documentation
- Style consistency

---

## Workflow: Refactor

Using: `.ai-pack/workflows/refactor.md`

**Key Principles:**
- Behavior stays the same (zero functional changes)
- TDD for all changes (test first, then refactor)
- Small incremental steps
- Run tests after each change
- Commit frequently

---

## Dependencies

**Required:**
- Access to all source files
- Code review report (`.ai/tasks/2026-01-26_comprehensive-code-review/`)
- Test suite (all 429 tests)
- Build system

**Blocks:**
- Future feature development (cleaner foundation)
- Architecture improvements
- Performance optimizations

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Breaking tests | High | Run tests after each change, commit frequently |
| Scope creep | High | Stick to identified issues, no new features |
| Performance regression | Medium | Benchmark before/after, especially dispatch table |
| Introducing bugs | High | TDD methodology, thorough testing |

---

## Quality Gates

**Mandatory:**
1. **Zero Test Regressions** - All 429 tests must pass after each phase
2. **Zero New Warnings** - Build must be clean
3. **TDD Process** - Tests before implementation for all changes
4. **Code Review** - Reviewer validates each phase
5. **PoP Compatibility** - 20/29 files must still assemble

---

## Acceptance Criteria Summary

### Critical
- [ ] [C1] ParseHex validation implemented and tested
- [ ] [C2] All dynamic casts have null checks

### Major
- [ ] [M1] Assemble() <50 lines, extracted methods
- [ ] [M2] If-else chain replaced with hash map
- [ ] [M3] Magic numbers replaced with constants
- [ ] [M4] Input validation added
- [ ] [M5] Exception handling improved
- [ ] [M6] Cpu6502 split into focused classes
- [ ] [M7] Code duplication <10%
- [ ] [M8] Error reporting implemented

### Minor
- [ ] [N1-N9] Addressed or documented

### Quality
- [ ] All 429 tests passing
- [ ] Zero new compiler warnings
- [ ] Zero performance regressions
- [ ] Code review grade A or better

---

## Estimated Effort

**By Phase:**
- Phase 1 (Critical): 1-2 days
- Phase 2 (God Method): 2-3 days
- Phase 3 (Dispatch): 1 day
- Phase 4 (Constants): 1 day
- Phase 5 (God Class): 2-3 days
- Phase 6 (Duplication): 1 day
- Phase 7 (Minor): 1 day

**Total: 9-13 days** (approximately 2 weeks)

**Can be parallelized** if multiple engineers work on independent phases.

---

## Stakeholders

- **xasm++ Maintainers:** Need clean, maintainable codebase
- **PoP Compatibility Project:** Depends on stable, tested foundation
- **Future Contributors:** Will benefit from clean code

---

## Notes

**Important:**
- This is REFACTORING, not new features
- Behavior must stay identical (zero functional changes)
- Test-driven approach is mandatory
- Commit after each successful test run
- Document any "won't fix" decisions with rationale

**Success Metric:**
- Clean, maintainable codebase that's easy to extend
- Zero technical debt in core components
- Foundation for future PoP compatibility work

---

## References

**Source Review:** `.ai/tasks/2026-01-26_comprehensive-code-review/30-review.md`

**Standards:**
- `.ai-pack/quality/clean-code/functions.md`
- `.ai-pack/quality/clean-code/solid-principles.md`
- `.ai-pack/workflows/refactor.md`
