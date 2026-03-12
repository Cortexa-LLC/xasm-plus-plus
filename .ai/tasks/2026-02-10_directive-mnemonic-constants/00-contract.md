# Task Contract: Directive Mnemonic Constants

**Task ID:** 2026-02-10_directive-mnemonic-constants
**Beads Task:** xasm++-6y60
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Define directive name constants (ORG, EQU, DB, DW, etc.) to eliminate magic strings in directive handlers.

### Background and Context

Similar to the instruction mnemonic constants work (task xasm++-xvlw), directive handlers currently use hardcoded strings throughout the codebase. This creates maintainability issues and prevents compile-time checking.

### Current State

All directive handlers use magic strings:
- Z80UniversalSyntaxParser: `directive_registry_.Register("ORG", ...)`
- MerlinSyntaxParser: `if (directive == "ORG") { ... }`
- SimpleSyntaxParser: `if (directive == ".ORG") { ... }`

### Desired State

Directive constants defined in headers, organized by syntax family:
- `include/xasm++/directives/common_directives.h`
- `include/xasm++/directives/merlin_directives.h`
- `include/xasm++/directives/scmasm_directives.h`
- `include/xasm++/directives/z80_directives.h`

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ Four directive constant headers created
✓ 50+ directive constants defined
✓ Constants organized by syntax family (common, merlin, scmasm, z80)
✓ Proper namespace organization
✓ Documentation comments for each directive family
✓ No parser code modified (foundation only)
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ include/xasm++/directives/common_directives.h created with 10+ common directives
□ include/xasm++/directives/merlin_directives.h created with 30+ Merlin directives
□ include/xasm++/directives/scmasm_directives.h created with conditional directives
□ include/xasm++/directives/z80_directives.h created with Z80-specific directives
□ Each namespace properly scoped (xasm::CommonDirectives, xasm::MerlinDirectives, etc.)
□ constexpr const char* used for all constants
```

### Quality Requirements
```
□ All builds passing (no code changes, header-only)
□ No linting errors
□ Documentation comments for directive families
□ Consistent naming conventions
```

### Non-Functional Requirements
```
□ No parser code modified (this is foundation only)
□ Follow-up tasks identified and blocked on this one
```

---

## Constraints and Dependencies

### Constraints
```
✓ Header-only (no implementation files)
✓ Use constexpr const char* for constants
✓ Follow existing namespace conventions (xasm::)
✓ No parser code changes
```

### Dependencies
```
✓ None - this is the foundation task
```

### Blocks
```
This task blocks:
□ xasm++-3pl6: Merlin DirectiveRegistry
□ xasm++-j4hz: SimpleSyntax DirectiveRegistry
□ xasm++-t3nb: SCMASM DirectiveRegistry
□ xasm++-vb6g: Z80Universal Use Directive Constants
```

### Out of Scope
```
✗ Modifying any parser code (follow-up tasks)
✗ Creating DirectiveRegistry instances
✗ Testing directive handlers
✗ Updating existing registry registrations
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 4 (new headers)
- Lines of code estimate: ~200 lines total (50 per header)
- New concepts/patterns: No (following instruction mnemonic constants pattern)
- Integration complexity: Low (header-only, no code changes)
- Risk level: Low (no existing code modified)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 4 files (new headers)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 4 files → ✅ IDEAL
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
4 × 500 tokens = 2,000 tokens (header files are small)

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 2,000 tokens → ✅ SAFE
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
- [X] YES - Batch size ≤14 files (4 files) AND token budget ≤42K (2K tokens) AND WIP ≤3 (1 agent)
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:** Small, self-contained task with 4 new header files. No decomposition needed.

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
- [X] Requestor: User 2026-02-10
- [X] Orchestrator: 2026-02-10

**Changes to Contract:**
None

---

## Notes

This is the foundation task for the directive registry refactoring work. All other directive-related tasks are blocked on this one.

Directive constants are defined following the same pattern as instruction mnemonic constants (task xasm++-xvlw).

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10

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
