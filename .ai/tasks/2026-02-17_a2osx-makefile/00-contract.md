# Task Contract

**Task ID:** 2026-02-17_a2osx-makefile
**Beads Task:** xasm++-gops
**Created:** 2026-02-17
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature

---

## Task Description

Create a Makefile for A2osX that preprocesses file structure (symlinks) to match expected Unix-like paths, then invokes xasm++ for assembly.

### Background and Context

A2osX source files use Unix-like include paths (`usr/src/shared/printf.s`) but files are actually stored with different structure (`A2osX/SYS/KERNEL.S.PRINTF.txt`). Rather than implementing complex path mapping in xasm++, we'll use a Makefile preprocessing approach similar to Prince of Persia's preprocessing pattern.

Spelunker investigation (xasm++-txym) found:
- A2osX references: `usr/src/shared/printf.s`
- Actual location: `A2osX/SYS/KERNEL.S.PRINTF.txt`
- Include paths: `inc/` → `A2osX/INC/`

### Current State

- xasm++ has `--include-path` working correctly
- A2osX fails at line 7 with "cannot open file: usr/src/shared/printf.s"
- No build system exists for A2osX

### Desired State

- Makefile in `~/Projects/Vintage/Apple/A2osX/`
- `make` successfully assembles A2osX past line 7
- Temporary structure in `/tmp/a2osx-build/` matches expected paths
- Build is reproducible and clean

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ Makefile exists at ~/Projects/Vintage/Apple/A2osX/Makefile
✓ make prep creates /tmp/a2osx-build/ structure with all required symlinks
✓ make build assembles A2osX past line 7 (currently fails at line 7)
✓ All .INB include directives resolve correctly
✓ make clean removes all temporary and generated files
✓ Build is reproducible (can run multiple times successfully)
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ prep target creates /tmp/a2osx-build/usr/src/shared/ directory
□ prep target creates /tmp/a2osx-build/inc/ directory
□ prep target symlinks all KERNEL.S.*.txt files to usr/src/shared/*.s
□ prep target symlinks all INC/* files to inc/*
□ build target invokes xasm++ with --include-path /tmp/a2osx-build
□ build target successfully assembles A2osX
□ clean target removes /tmp/a2osx-build/ directory
□ clean target removes any generated binaries/listings
□ all target runs prep then build
```

### Quality Requirements
```
□ Makefile follows GNU Make best practices
□ Pattern similar to Prince of Persia Makefile
□ No hard-coded paths (use variables)
□ Documentation comments in Makefile
□ Error handling for missing files
```

### Non-Functional Requirements
```
□ Build completes in reasonable time (<30 seconds)
□ Symlinks work correctly on macOS
□ Cleanup is thorough (no leftover files)
□ Reproducible (can run make clean && make multiple times)
```

---

## Constraints and Dependencies

### Constraints
```
□ Must work on macOS (where A2osX development happens)
□ Must use /tmp directory (not permanent location)
□ Must not modify A2osX source files
□ Must follow GNU Make conventions
```

### Dependencies
```
□ xasm++ must be installed and in PATH
□ A2osX source must be at ~/Projects/Vintage/Apple/A2osX/
□ xasm++ --include-path feature working (already complete)
```

### Out of Scope
```
✗ Implementing path mapping in xasm++ (using Makefile approach instead)
✗ Modifying A2osX source files
✗ Creating permanent directory structure (use /tmp)
✗ Supporting Windows or Linux (macOS only for now)
✗ Handling all 17+ file mappings (discover as needed during implementation)
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Number of files affected: 1 (new Makefile)
- Lines of code estimate: ~100 lines (Makefile)
- New concepts/patterns: No (following PoP pattern)
- Integration complexity: Low (just invokes xasm++)
- Risk level: Low (temporary files only, no code changes)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 1 file (new Makefile)

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
1 × 3,000 tokens = 3,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 3,000 tokens → ✅ SAFE
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

Planned WIP: 1 agent (Engineer only)
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
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Rationale:** Single file (Makefile), 3K tokens, 1 agent - well within all limits.

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
