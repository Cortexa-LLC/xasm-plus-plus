# Task Contract: Post-P0 Architectural Review

**Task ID:** 2026-02-14_post-p0-review
**Beads Task:** xasm++-nvfq
**Created:** 2026-02-14
**Assigned Role:** Reviewer
**Workflow:** Research (Architectural Review)
**Priority:** P1

---

## Task Description

### Background and Context
Three P0 critical architectural issues were resolved:
1. Lambda anti-pattern eliminated (xasm++-cws7)
2. CPU 6502 magic values eliminated (xasm++-lmzc)
3. CPU 6809 magic values eliminated (xasm++-oi41)

User requests COMPREHENSIVE review that:
- All P0 changes are consistent and complete
- ALL remaining P1 issues are identified and cataloged
- ALL remaining P2 issues are identified and cataloged
- Quality standards are met
- Code organization follows best practices (registries at top)
- No regressions introduced

### Current State
**Recent commits:**
- 3688b94: Core directive lambdas → free functions
- 3d69e41: CPU 6502 magic values → constants
- 9fa5298: CPU 6809 magic values → constants

**Previous review (xasm++-99f4):**
- 450+ issues identified
- P0 fixes completed for 3 critical issues

### Desired State
**COMPREHENSIVE REVIEW (not just verification):**
1. P0 fixes are complete and consistent
2. Complete catalog of ALL remaining P1 issues
3. Complete catalog of ALL remaining P2 issues
4. Code organization assessment (registries at top)
5. ADR compliance verified
6. Recommendations for P1/P2 implementation

---

## Success Criteria

✓ P0 verification complete (lambda, 6502, 6809)
✓ Complete P1 issue catalog with line numbers
✓ Complete P2 issue catalog with line numbers
✓ Code organization assessed (registry placement)
✓ ADR compliance verified
✓ Deliverables: Comprehensive review report + P1/P2 implementation roadmap

---

## Acceptance Criteria

### Functional Requirements
- [ ] Verify lambda anti-pattern eliminated in ALL syntax parsers
- [ ] Verify CPU 6502 magic values eliminated (72 hex + 5 radix)
- [ ] Verify CPU 6809 magic values eliminated (215 hex + 1 added constant)
- [ ] Verify pattern consistency across all directive handlers
- [ ] Check code organization (registries at top of files)

### Quality Requirements
- [ ] ADR-001 compliance: Free function pattern everywhere
- [ ] ADR-003 compliance: Parameter order (label, operand, context)
- [ ] ADR-004 compliance: No magic values in P0 scope
- [ ] Code readability: Consistent file structure
- [ ] Documentation: All findings documented with line numbers

### Deliverables
- [ ] Comprehensive review report (markdown)
- [ ] Issue catalog (remaining issues if any)
- [ ] Code organization assessment
- [ ] Compliance checklist (ADR-001, 003, 004)
- [ ] Recommendations for any remaining work

---

## Review Scope

### 1. P0 Verification

**Lambda Anti-pattern (xasm++-cws7):**
- Check: `src/syntax/core_directive_handlers.cpp`
- Verify: No lambda wrappers in RegisterCoreDirectiveHandlers()
- Verify: All handlers use (label, operand, context) signature
- Verify: Pattern matches Merlin reference

**CPU 6502 Opcodes (xasm++-lmzc):**
- Check: `src/cpu/cpu_6502.cpp`, `include/xasm++/cpu/opcodes_6502.h`
- Verify: 72 magic hex values → Opcodes:: constants
- Verify: 5 magic radix values → RADIX_* constants
- Verify: 40+ opcode constants added

**CPU 6809 Opcodes (xasm++-oi41):**
- Check: `src/cpu/cpu_6809.cpp`, `include/xasm++/cpu/opcodes_6809.h`
- Verify: 215 magic hex values → Opcodes6809:: constants
- Verify: LBSR constant added
- Verify: Pattern matches CPU 6502/Z80

### 2. Code Organization Assessment

**Registry Placement:**
- Check: All *_directive_handlers.cpp files
- Verify: Registry functions at top of implementation files
- Verify: Consistent ordering: registry, then helpers, then handlers

**File Structure Consistency:**
- Check: All syntax parser implementation files
- Verify: Consistent structure across similar modules
- Verify: Logical grouping for readability

**Naming Conventions:**
- Verify: Consistent handler naming (HandleX)
- Verify: Consistent registry naming (Register*DirectiveHandlers)
- Verify: Consistent namespace usage

### 3. Pattern Consistency

**Directive Handler Pattern:**
- All syntax parsers follow Merlin pattern
- No lambda wrappers anywhere
- Direct assignment: registry[X] = Handler
- Free functions in namespace

**Parameter Order:**
- All handlers: (label, operand, context)
- DirectiveContext used consistently
- No parameter reordering lambdas

**Magic Value Elimination:**
- No magic hex values in reviewed files
- No magic radix values (2, 8, 10, 16)
- All constants properly named and documented

---

## Out of Scope

✗ Performance optimization (unless architectural issue)
✗ Test coverage improvement (report on existing only)
✗ Documentation updates (focus on code quality only)
✗ Implementation of fixes (review only, not fix)

**This is a COMPREHENSIVE REVIEW: P0 verification + P1/P2 discovery.**

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Files to review: ~10-15 key files (P0 changes + context)
- Focus: Verification, not discovery
- Reference: Previous fixes and ADRs
- Risk level: Low (verification only, no changes)

---

## Lean Flow Analysis

### Batch Size Assessment
**Estimated Files:** 10-15 files (verification focused)
**Status:** ⚠️ ACCEPTABLE (verification review, not implementation)

### Token Budget Estimation
```
15 files × 2,000 tokens (review output) = 30,000 tokens
Status: ⚠️ APPROACHING LIMIT
```

**Note:** This is a review task, output will be report text (not code), should stay within limits.

### Work In Progress Planning
**Agents:** 1 (this review)
**WIP Status:** ✅ Within limits

### Decomposition Decision
- [x] YES - Proceed as single task (verification review, focused scope)

---

## Resources and References

### P0 Commits to Verify
- 3688b94: Core directive lambdas fix
- 3d69e41: CPU 6502 opcodes fix
- 9fa5298: CPU 6809 opcodes fix

### ADRs to Verify Against
- ADR-001: Directive Handler Pattern
- ADR-003: Parameter Order Standardization
- ADR-004: Magic Value Elimination

### Reference Implementations
- `src/syntax/merlin_directive_handlers.cpp` - Correct pattern
- `src/cpu/cpu_z80.cpp` - Best practice CPU implementation
- Previous review: `.ai/tasks/2026-02-14_architectural-review/`

### Previous Review Results
- Total issues: 450+
- P0 issues: 157 (3 addressed)
- Expected remaining: <50 overall

---

## Review Checklist

### P0 Verification
- [ ] Lambda anti-pattern: ELIMINATED
- [ ] CPU 6502 magic values: ELIMINATED
- [ ] CPU 6809 magic values: ELIMINATED
- [ ] Pattern consistency: VERIFIED

### Code Organization
- [ ] Registries at top: VERIFIED
- [ ] File structure consistent: VERIFIED
- [ ] Naming conventions: VERIFIED
- [ ] Readability: ACCEPTABLE

### ADR Compliance
- [ ] ADR-001: COMPLIANT
- [ ] ADR-003: COMPLIANT
- [ ] ADR-004: COMPLIANT (P0 scope)

### Deliverables
- [ ] Review report created
- [ ] Issue catalog (if issues found)
- [ ] Organization assessment complete
- [ ] Recommendations documented

---

## Assumptions

1. P0 commits (3688b94, 3d69e41, 9fa5298) are the source of truth
2. Test pass rate (98%, 1535/1564) is acceptable baseline
3. Focus is verification, not new issue discovery
4. Code organization improvements are advisory, not blocking
5. User's "quality expectation" = ADR compliance + clean organization

---

## Risk Assessment

### Identified Risks

1. **Incomplete P0 fixes**
   - Probability: Low (all tests passing)
   - Impact: High (would need rework)
   - Mitigation: Thorough line-by-line verification

2. **New issues introduced**
   - Probability: Low (refactoring, not new code)
   - Impact: Medium (would need P0.x tasks)
   - Mitigation: Compare before/after, check for regressions

3. **Code organization not meeting expectations**
   - Probability: Medium (subjective standard)
   - Impact: Low (advisory improvements)
   - Mitigation: Document current state, provide specific recommendations

---

## Approvals

**Contract Approved By:**
- [x] Orchestrator: AI Orchestrator (2026-02-14)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-14
