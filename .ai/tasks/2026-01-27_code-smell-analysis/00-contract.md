# Task Contract: Code Smell Analysis

**Task ID:** 2026-01-27_code-smell-analysis
**Created:** 2026-01-27
**Requestor:** Bryan W.
**Assigned Role:** Reviewer
**Workflow:** Research

---

## Task Description

Analyze the xasm++ C++ codebase against the ai-pack code smells quality standard (`.ai-pack/quality/clean-code/13-code-smells.md`) to identify remaining design issues after recent refactoring.

### Background and Context

The project recently underwent major code quality improvements:
- Refactored M1 god method (344 → 53 lines, 43% → 10% complexity)
- Fixed 19 code quality issues (C+ → A- grade)
- Upgraded to C++20
- Fixed 43+ clang-tidy warnings
- Current state: 85%+ coverage, zero compiler warnings, 358 clang-tidy warnings (mostly style)

The ai-pack framework was just updated to include a comprehensive code smells guide (commit 6127491) covering 23 code smells in 5 categories. This analysis will assess if additional design issues exist beyond what previous tools detected.

### Current State

- Production-quality code (Grade A-)
- All functional tests passing (500/500)
- Static analysis complete (clang-tidy)
- No remaining critical issues known

### Desired State

Comprehensive assessment of code smells with:
- Identification of any remaining design issues
- Severity and priority classification
- Actionable refactoring recommendations
- Documentation to guide future quality improvements

---

## Success Criteria

```
✓ All 5 code smell categories analyzed (Bloaters, OO Abusers, Change Preventers, Dispensables, Couplers)
✓ Findings documented with file:line locations
✓ Severity levels assigned (Critical, High, Medium, Low)
✓ Refactoring recommendations provided for each finding
✓ Summary statistics generated (counts by category and severity)
✓ Priority actions identified
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Review all C++ source files in src/ and include/
□ Check for all 23 code smell types:
  - Bloaters: Long Method, Large Class, Primitive Obsession, Long Parameter List, Data Clumps
  - OO Abusers: Switch Statements, Temporary Field, Refused Bequest, Alternative Classes with Different Interfaces
  - Change Preventers: Divergent Change, Shotgun Surgery, Parallel Inheritance Hierarchies
  - Dispensables: Comments, Duplicate Code, Lazy Class, Data Class, Dead Code, Speculative Generality
  - Couplers: Feature Envy, Inappropriate Intimacy, Message Chains, Middle Man
□ Document each finding with location, severity, explanation, recommendation
□ Generate summary statistics
```

### Quality Requirements
```
□ Analysis based on authoritative standard (.ai-pack/quality/clean-code/13-code-smells.md)
□ Findings include code examples where applicable
□ Recommendations reference established refactoring patterns
□ Report structured for actionability
□ Cross-reference with previous refactoring results
```

### Non-Functional Requirements
```
□ Analysis completed systematically (all files covered)
□ Findings prioritized by impact
□ Report readable and maintainable
□ Recommendations feasible within project constraints
```

---

## Constraints and Dependencies

### Constraints
```
□ Analysis only - no implementation work
□ Focus on production code (exclude tests, scripts)
□ Use ai-pack standard as single source of truth
□ Respect existing architectural patterns (table-driven encoding)
```

### Dependencies
```
□ Access to ai-pack code smells guide (.ai-pack/quality/clean-code/13-code-smells.md)
□ Existing code quality documentation (REMAINING_CONCERNS.md, CODE_QUALITY_REFACTORING_COMPLETE.md)
□ Project source files (src/, include/)
```

### Out of Scope
```
✗ Implementation of fixes (separate task)
✗ Test code analysis (separate concern)
✗ Build system or tooling improvements
✗ Documentation file reviews
✗ Performance optimization recommendations
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: ~15 source files (analysis, not modification)
- Lines of code to review: ~5,000 LOC
- New concepts/patterns: Applying 23 code smell patterns
- Integration complexity: Low (read-only analysis)
- Risk level: Low (no code changes)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** ~15 files (for analysis)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: ~15 files → ❌ AT BOUNDARY
```

### Batch Size Justification

Files: 15 (at boundary, requires justification)

**Why not decomposed further:**
- Read-only analysis (no code generation overhead)
- Single concern: Comprehensive code smell assessment
- Sequential analysis more effective than parallel for this task type
- Report consolidation requires holistic view

**Contingency for token limits:**
- If token limit hit during analysis, will complete by category:
  - Batch 1: Bloaters + OO Abusers (~6 files)
  - Batch 2: Change Preventers + Dispensables (~6 files)
  - Batch 3: Couplers + Summary (~3 files)

**Estimated tokens:** ~15 × 500 (read-only analysis, not full file content) = ~7,500 tokens
**Status:** Within 25K-32K limit? **YES - SAFE** (read-only analysis requires much less context than code generation)

### Token Budget Estimation

**Conservative Estimate:**
```
Analysis Type: Read-only code review
Files × Estimated Analysis Tokens = Total
15 × 500 tokens = 7,500 tokens (analysis context, not full file reads)

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: ~7,500 tokens → ✅ SAFE
```

**Rationale for lower token estimate:**
- Analysis task, not code generation
- Agent can reference files by pattern detection, not full content inclusion
- Report output is structured findings, not code transformations

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Reviewer)
```

**Sequential execution preferred:**
- Single holistic analysis
- Consolidated report
- No parallelization benefit for analysis task

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size at boundary BUT justified: read-only analysis with token budget ≤20K AND WIP = 1
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Justification:**
Analysis tasks have different characteristics than implementation tasks. The 15-file boundary is for code generation work where full file context must be maintained. For read-only analysis, the agent can work more efficiently by pattern matching and targeted examination.

**Contingency:** If analysis runs long, decompose by category as documented above.

---

## Resources and References

### Relevant Files
```
- .ai-pack/quality/clean-code/13-code-smells.md - Authoritative standard (2,523 lines, 23 code smells)
- src/syntax/merlin/merlin_syntax.cpp - Main syntax handler (~1,500 lines, recently refactored)
- src/syntax/merlin/merlin_tables.cpp - Table-driven encoding
- include/xasm++/*.h - Core abstractions (expression, assembler, etc.)
- REMAINING_CONCERNS.md - Previous quality assessment
- CODE_QUALITY_REFACTORING_COMPLETE.md - Recent refactoring summary
```

### Documentation
```
- .ai-pack/quality/engineering-standards.md - Overall standards index
- CLANG_TIDY_ANALYSIS.md - Recent static analysis results
```

### Examples
```
- Previous refactoring work: M1 god method decomposition, test structure improvements
```

---

## Assumptions

```
1. The ai-pack code smells guide is comprehensive and authoritative
2. Previous refactoring addressed most critical issues (valid baseline)
3. Remaining issues are likely medium-to-low severity
4. Findings will inform prioritized future refactoring work
5. Analysis can be completed in single session without exceeding token limits
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Token limit exceeded during analysis
   - Probability: Low (read-only analysis, ~7.5K tokens)
   - Impact: Medium (would need to decompose by category)
   - Mitigation: Contingency plan documented above

2. Analysis finds zero issues (false negative)
   - Probability: Low (comprehensive 23-pattern standard)
   - Impact: Low (indicates high code quality)
   - Mitigation: Systematic review process, cross-check with clang-tidy results

3. Analysis finds too many issues (overwhelming)
   - Probability: Medium (new standard may be stricter)
   - Impact: Medium (need prioritization)
   - Mitigation: Severity classification and priority recommendations included
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan W. [2026-01-27]
- [ ] Agent: Reviewer [Pending]

**Changes to Contract:**
None yet.

---

## Notes

This is a research/assessment task using the Reviewer role. The goal is to identify remaining quality issues after significant recent improvements. Findings will guide future refactoring priorities but do not require immediate action.

**Beads Task:** xasm++-fka

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-27
