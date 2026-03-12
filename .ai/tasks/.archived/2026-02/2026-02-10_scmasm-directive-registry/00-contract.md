# Task Contract

**Task ID:** 2026-02-10_scmasm-directive-registry
**Beads Task:** xasm++-hnv8
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Orchestrator
**Workflow:** Refactor

---

## Task Description

Apply the DirectiveRegistry pattern to SCMASM's conditional directive handling (.DO, .ELSE, .FIN) and other directive processing to improve extensibility and maintainability.

### Background and Context

The SCMASM assembler currently processes directives using direct conditional checks and ad-hoc handling. This pattern has been successfully applied to other parts of the codebase (PASM, QASM) and needs to be extended to SCMASM for consistency and maintainability.

### Current State

SCMASM directive handling is currently implemented with inline conditional logic, making it harder to extend and maintain. The code would benefit from the DirectiveRegistry pattern for better organization.

### Desired State

SCMASM should use the DirectiveRegistry pattern to handle conditional directives (.DO, .ELSE, .FIN) and other directives in a consistent, extensible manner following the same pattern used in other assembler modules.

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ DirectiveRegistry pattern applied to SCMASM
✓ All directive handling uses registry instead of if/else chains
✓ All existing tests passing
✓ Zero compilation warnings
✓ Build succeeds
```

**Specific Measures:**
- ✓ ~20 SCMASM directives registered in DirectiveRegistry
- ✓ InitializeDirectiveRegistry() method created and called from constructor
- ✓ ParseLine() method refactored to use registry lookup
- ✓ Existing SCMASM tests still pass (no behavioral changes)
- ✓ Code compiles without warnings

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ DirectiveRegistry pattern applied to SCMASM parser
□ All 20+ SCMASM directives registered in InitializeDirectiveRegistry()
□ ParseLine() method refactored to use directive_registry_ lookup
□ Conditional directives (.DO, .ELSE, .FIN) handled via registry
□ All existing directive functionality preserved (no behavioral changes)
```

### Quality Requirements
```
□ All SCMASM tests passing
□ Zero compilation warnings
□ No linting errors
□ Code follows existing patterns (Merlin, SimpleSyntax implementations)
□ Inline documentation updated
```

### Non-Functional Requirements
```
□ Performance maintained (registry lookup is O(1))
□ Error handling preserved (same error messages as before)
□ Extensibility improved (new directives added easily)
□ Code maintainability improved (eliminate if/else chains)
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

**Complexity:** Small

**Rationale:**
- Number of files affected: 3
  - `include/xasm++/syntax/scmasm_syntax.h` (add directive_registry_ member)
  - `src/syntax/scmasm/scmasm_syntax.cpp` (refactor ParseLine, add InitializeDirectiveRegistry)
  - `tests/unit/test_scmasm_syntax.cpp` (verify existing tests pass)
- Lines of code estimate: ~200-300 lines total changes
- New concepts/patterns: No (DirectiveRegistry pattern already established in Merlin, SimpleSyntax)
- Integration complexity: Low (following established pattern)
- Risk level: Low (refactoring with existing tests for validation)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 3 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 3 files → ✅ IDEAL
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
3 × 3,000 tokens = 9,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 9,000 tokens → ✅ SAFE
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

Planned WIP: 1 agent (Engineer will complete this task)
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
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
  - ✅ 3 files (well within limit)
  - ✅ 9K tokens (safe)
  - ✅ 1 agent (ideal WIP)
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:**
This task is appropriately scoped for a single engineer to complete. The DirectiveRegistry pattern has already been successfully applied to Merlin and SimpleSyntax, providing a clear template to follow. The SCMASM parser has ~20 directives to register, similar in scope to previous implementations.

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
