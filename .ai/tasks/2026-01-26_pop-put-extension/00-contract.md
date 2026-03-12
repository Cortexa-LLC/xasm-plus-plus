# Task Contract

**Task ID:** 2026-01-26_pop-put-extension
**Created:** 2026-01-26
**Requestor:** Bryan Woodruff (Orchestrator)
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Modify PUT directive handler to auto-append .S extension when no extension is present. This is the PRIMARY blocker for Prince of Persia compatibility (16 files blocked).

### Background and Context

**Analysis Finding:** The PoP analysis (docs/investigations/pop-compatibility-analysis.md) identified PUT directive extension handling as the #1 blocker:
- 63 occurrences across 16 files (70% of failures)
- Files use: `put eq` instead of `put eq.S`
- Error: "Cannot open file: eq"

This is a simple fix with massive impact (16 files unblocked).

### Current State

PUT directive requires explicit extension:
- ✅ Works: `put eq.S`
- ❌ Fails: `put eq` (file not found)

### Desired State

PUT directive auto-appends .S extension:
- ✅ Works: `put eq.S` (unchanged)
- ✅ Works: `put eq` (auto-appends .S)
- ✅ Works: `put data.HEX` (keeps existing extension)

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ PUT directive auto-appends .S when no extension present
✓ PUT directive keeps existing extension if present
✓ SUBS.S assembles successfully (has 5 PUT directives)
✓ 16 previously failing PoP files now assemble
✓ All existing tests passing
✓ New tests added for PUT extension logic
✓ No regressions in working files (6 currently assembling)
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ Modify handle_PUT() in merlin_syntax.cpp
□ Check if filename contains '.' (has extension)
□ If no extension: append ".S"
□ If extension exists: use as-is
□ Test: "put eq" → finds "eq.S"
□ Test: "put data.HEX" → finds "data.HEX"
```

### Quality Requirements
```
□ All tests passing (including existing Merlin tests)
□ Code coverage ≥ 85%
□ TDD process followed (RED-GREEN-REFACTOR)
□ Tester validation (APPROVED)
□ Reviewer validation (APPROVED)
□ Zero compiler warnings
```

### Non-Functional Requirements
```
□ No performance impact
□ Backward compatible (explicit .S still works)
□ Clear error messages if file still not found
□ Follows existing code patterns
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
- Number of files affected: 1 (merlin_syntax.cpp)
- Lines of code estimate: ~10-15 lines
- New concepts/patterns: No (standard string manipulation)
- Integration complexity: Low (isolated change in handle_PUT())
- Risk level: Low (easy to test, clear behavior)

**Estimated Effort:** 1-2 hours

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 1 file (merlin_syntax.cpp)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 1 file → ✅ IDEAL
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
**Beads Task:** xasm++-sos
