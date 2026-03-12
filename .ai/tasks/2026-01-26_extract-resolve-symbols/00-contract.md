# Task Contract

**Task ID:** 2026-01-26_extract-resolve-symbols
**Created:** 2026-01-26
**Beads Task:** xasm++-5tu
**Parent Task:** 2026-01-26_code-quality-refactoring (xasm++-hxm)
**Requestor:** Orchestrator (God Method Refactoring Phase 2)
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Extract symbol resolution logic from the 344-line `Assembler::Assemble()` method into a focused private method `ResolveSymbols()`. This is subtask B1 of [M1] God Method Anti-Pattern from the comprehensive code quality review.

### Background and Context

The `Assemble()` method in `src/core/assembler.cpp` is 344 lines long and violates Single Responsibility Principle by handling symbol resolution, instruction encoding, convergence checking, and error handling. This task extracts ONLY the symbol resolution logic.

### Current State

- `Assemble()` is 344 lines with mixed responsibilities
- Symbol resolution logic intermingled with other concerns
- All 450 tests passing
- Code difficult to understand and test

### Desired State

- `ResolveSymbols()` private method created
- Symbol resolution logic extracted (~50 lines)
- `Assemble()` calls `ResolveSymbols()` instead of inlining logic
- All 450 tests still passing (zero regressions)
- Behavior identical to before refactoring

---

## Success Criteria

```
✓ New private method ResolveSymbols() created
✓ Symbol resolution logic extracted from Assemble()
✓ Assemble() calls ResolveSymbols() appropriately
✓ All 450 tests passing (zero regressions)
✓ Zero new compiler warnings
```

---

## Technical Specification

### Method Signature
```cpp
private:
  void ResolveSymbols(std::vector<std::shared_ptr<Atom>>& atoms,
                      ConcreteSymbolTable& symbols,
                      uint32_t pass,
                      AssemblerResult& result);
```

### Scope
Extract symbol resolution logic from `Assemble()` - typically includes iterating atoms, checking unresolved symbols, evaluating expressions. Do NOT extract encoding, convergence, or error reporting.

### Constraints
- ZERO behavior changes (pure refactoring)
- ZERO test regressions (all 450 tests must pass)
- Method must be `private`
- Follow existing code style

---

## Lean Flow Analysis

**Files:** 2 files (assembler.h, assembler.cpp)
**Tokens:** ~6K tokens
**Status:** ✅ SAFE - Small batch, proceed

---

## Resources

- `src/core/assembler.cpp` - Implementation
- `include/xasm++/assembler.h` - Header
- `tests/unit/test_assembler.cpp` - Tests
- Parent: `.ai/tasks/2026-01-26_code-quality-refactoring/00-contract.md`

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ [Requirement 1]
□ [Requirement 2]
□ [Requirement 3]
```

### Quality Requirements
```
□ All tests passing
□ Code coverage 80-90%
□ No linting errors
□ Code review approved
□ Documentation complete
```

### Non-Functional Requirements
```
□ Performance acceptable
□ Security validated
□ Accessibility considered
□ Error handling robust
```

---

## Constraints and Dependencies

### Constraints
```
□ [Technical constraint]
□ [Business constraint]
□ [Time constraint]
□ [Resource constraint]
```

### Dependencies
```
□ [Dependency on other task/feature]
□ [Dependency on external service]
□ [Dependency on team member]
□ [Dependency on tool/library]
```

### Out of Scope
```
✗ [Explicitly not included 1]
✗ [Explicitly not included 2]
✗ [Explicitly not included 3]
```

---

## Estimated Complexity

**Complexity:** [Trivial | Small | Medium | Large | Very Large]

**Rationale:**
- Number of files affected: [X]
- Lines of code estimate: [~X]
- New concepts/patterns: [Yes/No]
- Integration complexity: [Low/Medium/High]
- Risk level: [Low/Medium/High]

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** [X files]

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: [X files] → [Status]
```

**If 15+ files, MANDATORY decomposition:**
```markdown
### Decomposition Plan

This task is too large (X files) and MUST be decomposed:

**Subtask 1:** [Name] ([Y] files)
- Files: [list]
- Estimated tokens: ~[Y × 3000] tokens
- Dependencies: [None | Depends on X]

**Subtask 2:** [Name] ([Y] files)
- Files: [list]
- Estimated tokens: ~[Y × 3000] tokens
- Dependencies: [Depends on Subtask 1]

[Add more subtasks as needed]

**Execution Strategy:**
- Sequential (dependencies) OR Parallel (independent)
- WIP Limit: Max 3 spawned agents simultaneously
```

**If 6-14 files, document reasoning:**
```markdown
### Batch Size Justification

Files: [X] (within acceptable range but requires justification)

**Why not decomposed further:**
- [Reason: High cohesion - all files tightly coupled]
- [Reason: Single concern - one logical unit]
- [Reason: Already minimal viable batch]

**Contingency for token limits:**
- [If token limit hit, will decompose into: X + Y]

**Estimated tokens:** ~[X × 3000] = [total] tokens
**Status:** Within 25K-32K limit? [Yes/No]
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
[X] × 3,000 tokens = [total] tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: [total] tokens → [Status]
```

**If >42K tokens:**
```
⚠️ WARNING: Token budget risk

REQUIRED ACTION: Decompose task into smaller batches

Target: Each batch ≤42K tokens (≤14 files)
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: [X agents]
```

**If planning parallel execution:**
```markdown
### Parallel Execution Plan

**Agents to spawn:** [X]
**WIP limit:** Maximum 3 concurrent spawned agents

**Agent 1:** [Task description]
- Files: [list]
- Estimated tokens: [X]
- Dependencies: [None]

**Agent 2:** [Task description]
- Files: [list]
- Estimated tokens: [X]
- Dependencies: [None | Depends on Agent 1]

[If >3 agents, MUST decompose or run sequentially]

**Coordination Strategy:**
- [How will shared resources be managed?]
- [Execution order if sequential?]
- [Verification approach?]
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [ ] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**If NO, decomposition is MANDATORY:**
```markdown
See Decomposition Plan above.
Each subtask created as separate task packet.
Cannot proceed until decomposed.
```

**Reference:** `gates/05-lean-flow.md` for enforcement details

---

## Resources and References

### Relevant Files
```
- path/to/file1.ext - [Description]
- path/to/file2.ext - [Description]
```

### Documentation
```
- [Link to design doc]
- [Link to API spec]
- [Link to related issue]
```

### Examples
```
- path/to/example.ext - [Similar implementation]
- [External reference/tutorial]
```

---

## Assumptions

```
1. [Assumption 1]
2. [Assumption 2]
3. [Assumption 3]
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. [Risk 1]
   - Probability: [Low/Medium/High]
   - Impact: [Low/Medium/High]
   - Mitigation: [Strategy]

2. [Risk 2]
   - Probability: [Low/Medium/High]
   - Impact: [Low/Medium/High]
   - Mitigation: [Strategy]
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [ ] Requestor: [Name] [Date]
- [ ] Agent: [Role] [Date]

**Changes to Contract:**
[Document any contract changes here with date and rationale]

---

## Notes

[Any additional notes, clarifications, or context]

---

**Contract Version:** 1.0
**Last Updated:** [Date]

---

## Usage Instructions

This template should be instantiated at: `.ai/tasks/YYYY-MM-DD_task-name/00-contract.md`

**When to create:**
- At the start of any new task
- Before planning or implementation begins

**Who creates it:**
- Orchestrator (for complex tasks)
- Worker (for assigned tasks)
- User (can provide initial version)

**Key principles:**
- Be specific and measurable
- Clarify ambiguities upfront
- Document assumptions
- Get agreement before proceeding
