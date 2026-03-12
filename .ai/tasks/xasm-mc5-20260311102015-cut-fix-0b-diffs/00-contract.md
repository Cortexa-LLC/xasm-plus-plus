# Contract: Fix BIN/CUT +18B Regression and 0B-Diff Files

**Task ID:** xasm-mc5
**Created:** 2026-03-11
**Requestor:** Orchestrator
**Assigned Role:** Engineer
**Workflow:** Bugfix
**Requires Review:** true

---

## KG Orientation (MANDATORY FIRST STEP)

**Before filling out this contract, query the Knowledge Graph to understand existing context.**

```
mcp__kg__search_knowledge("<task keywords>")
mcp__kg__search_knowledge("<affected component or file area>")
```

**Fill in findings below — this prevents rediscovering what's already indexed:**

### Existing KG Context
```
Entities found:    [list relevant entities from search_knowledge results]
Key observations:  [any past decisions, bugs, or design notes found]
Related files:     [file paths from get_file_context results]
Gaps / unknowns:   [what the KG didn't know — needs investigation]
```

> Skip this section only for trivial 1-step tasks. For all code changes: KG first, then grep.

---

## Task Description

Fix assembler bugs causing xasm++ to produce non-identical output for A2osX binaries.
Primary: BIN/CUT +18B regression from `.CS` directive mis-parsing mixed delimiters.
Secondary: investigate and fix 0B-diff files (same size, different content).

### Background and Context

xasm++ is a modern C++ reimplementation of the Apple II SCMASM assembler used to build A2osX.
We compare xasm++ output against a reference stable build byte-for-byte.
Current run 8 (commit f7883f3): 65/132 identical. BIN/CUT is a new regression.

### Current State

- BIN/CUT: xasm++ produces 18 extra bytes vs stable
  - 16 bytes: `"line of text"\r\n` spuriously emitted in MSG.USAGE data
  - 2 bytes: branch relaxation artifact from the 16-byte size shift
- DRV/DHGR.DRV: same size as stable but content differs at offset 0x03F7
- DRV/PPPSSC.DRV: same size as stable but content differs at offset 0x0050
- LIB/LIBTCPIP: same size as stable but content differs at offset 0x00A0
- LIB/LIBCRYPT: same size as stable but content differs at offset 0x005A

### Desired State

- BIN/CUT is byte-identical to stable
- At least 2 of the 4 0B-diff files become byte-identical to stable
- Total identical count increases from 65

---

## Success Criteria

```
✓ cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build exits 0
✓ ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure exits 0
✓ BIN/CUT is identical to stable after A2osX rebuild
✓ At least 2 of {DRV/DHGR, DRV/PPPSSC, LIB/LIBTCPIP, LIB/LIBCRYPT} are identical to stable
✓ Total identical count > 65
```

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
- src/syntax/scmasm_directive_handlers.cpp — .CS directive (primary fix)
- src/syntax/scmasm_syntax.cpp — general syntax parsing

Reference binaries:
- /tmp/A2osX-335cd122/BIN/CUT.S.txt — CUT source
- /tmp/stable_extracted/bin/cut.dump — CUT stable reference
- /tmp/A2osX-335cd122-build/stage/bin/cut — CUT built output
- /tmp/A2osX-335cd122/DRV/DHGR.DRV.S.txt — DHGR source
- /tmp/stable_extracted/drv/dhgr.drv.dump — DHGR stable reference
- /tmp/A2osX-335cd122/DRV/PPPSSC.DRV.S.txt — PPPSSC source
- /tmp/stable_extracted/drv/pppssc.drv.dump — PPPSSC stable reference
- /tmp/A2osX-335cd122/LIB/LIBTCPIP.S.txt — LIBTCPIP source
- /tmp/stable_extracted/lib/libtcpip.dump — LIBTCPIP stable reference
- /tmp/A2osX-335cd122/LIB/LIBCRYPT.S.txt — LIBCRYPT source
- /tmp/stable_extracted/lib/libcrypt.dump — LIBCRYPT stable reference
- /tmp/run_compare_v2.py — comparison script
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

This template should be instantiated at: `.ai/tasks/<beads-id>-<YYYYMMDDHHMMSS>-<short-desc>/00-contract.md`

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
