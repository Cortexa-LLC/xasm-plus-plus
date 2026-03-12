# Task Contract

**Task ID:** 2026-01-24_merlin-syntax-1-3
**Created:** 2026-01-24
**Requestor:** Bryan Woodruff
**Assigned Role:** Orchestrator → Engineer
**Workflow:** Feature
**Beads Task:** xasm++-vp5

---

## Task Description

Implement Merlin Phases 1-3: Foundation, Local Labels, and DUM Blocks for the Merlin syntax parser.

### Background and Context

**Why:** This is Phase 1-3 of the Merlin Syntax Support feature. We need basic Merlin parsing to enable assembling Prince of Persia source code.

**Problem:** xasm++ currently has only a basic SimpleSyntax parser. Merlin syntax differs significantly:
- Comments: `*` in column 1, `;` inline
- No dot prefixes on directives (ORG not .ORG)
- Three-tier label system: global, :local, ]variable
- DUM/DEND memory allocation blocks

**Context:** This builds on complete 65816 CPU support (Phase 2.5 complete). We're implementing the minimal subset needed for Prince of Persia.

### Current State

**What Exists:**
- ✅ Complete CPU plugin (6502/65C02/65816)
- ✅ SimpleSyntax parser (basic implementation)
- ✅ Core assembler (Section, Symbol, Expression classes)
- ✅ Empty directory: src/syntax/merlin/

**What's Missing:**
- ❌ Merlin comment parsing
- ❌ Merlin line parser (label/directive/operand)
- ❌ Core directives: ORG, EQU, DB, DW, HEX
- ❌ Local label scoping (:label)
- ❌ DUM blocks and ]variable labels

### Desired State

**After Completion:**
- ✅ MerlinSyntax class parsing basic Merlin files
- ✅ Comments (`*` and `;`) working
- ✅ Global labels working
- ✅ Local labels (:label) with proper scoping
- ✅ Core directives (ORG, EQU, DB, DW, HEX, DS)
- ✅ DUM/DEND blocks with ]variable labels
- ✅ Number formats ($hex, %binary, decimal)
- ✅ Comprehensive test coverage

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ Can parse Merlin comments (* and ;)
✓ Can parse global labels
✓ Can parse local labels (:label) with proper scoping
✓ Can parse ]variable labels in DUM blocks
✓ Directives work: ORG, EQU, DB, DW, HEX, DS, DUM, DEND
✓ Number formats work: $hex, %binary, decimal
✓ All tests passing (100%)
✓ Code coverage ≥ 85% overall, 95%+ for new code
✓ Zero compiler warnings
✓ Can assemble Phase 1-3 test programs
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ Phase 1: Foundation
  □ Comment parsing: * (column 1) and ; (inline)
  □ Line parsing: label, directive, operand, comment
  □ Global labels (no : or ] prefix)
  □ Directives: ORG, EQU, DB, DW, HEX
  □ Number formats: $hex, %binary, decimal
  □ Simple expressions: +, -, *, /, &, |, ^

□ Phase 2: Local Labels
  □ :LOCAL label syntax
  □ Label scoping (local to last global)
  □ Forward/backward references
  □ Unique local labels per scope

□ Phase 3: DUM Blocks & Variables
  □ DUM/DEND blocks
  □ ]VARIABLE labels in DUM blocks
  □ DS (define space) directive
  □ Variable label references
```

### Quality Requirements
```
□ All tests passing (100%)
□ Code coverage ≥ 85% overall
□ New code coverage ≥ 95%
□ Zero compiler warnings
□ cmake --build build succeeds
□ ctest passes all tests
□ Tester validation: APPROVED
□ Reviewer validation: APPROVED
```

### Non-Functional Requirements
```
□ Follows existing code patterns (SimpleSyntax model)
□ Clean separation: header (.h) declarations, source (.cpp) implementations
□ Error messages helpful for debugging
□ Performance: parses 30K line file in <1 second
```

---

## Constraints and Dependencies

### Constraints
```
□ Must follow existing code patterns (SimpleSyntax model)
□ C++ best practice: declarations in .h, implementations in .cpp
□ Must not break existing tests
□ Zero compiler warnings required
```

### Dependencies
```
✓ Complete CPU plugin (6502/65C02/65816) - DONE (Phase 2.5)
✓ Core assembler engine (Section, Symbol, Expression) - DONE
✓ GoogleTest framework - AVAILABLE
✓ CMake build system - AVAILABLE
```

### Out of Scope
```
✗ PUT directive (file inclusion) - Phase 4
✗ Conditional assembly (DO/FIN) - Phase 6
✗ Macros (MAC...<<<) - Phase 7
✗ String directives (ASC, DCI, etc.) - Phase 5
✗ Integration with real PoP files - Phase 8
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 4 files (2 new, 2 modified)
- Lines of code estimate: ~1,500-2,000 lines total
  - merlin_syntax.h: ~150 lines
  - merlin_syntax.cpp: ~800-1,000 lines
  - test_merlin_syntax.cpp: ~500-800 lines
  - CMakeLists.txt: ~10 lines
- New concepts/patterns: Yes (label scoping, DUM blocks)
- Integration complexity: Low (follows SimpleSyntax pattern)
- Risk level: Low (isolated syntax plugin, doesn't affect CPU or core)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 4 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 4 files → ✅ IDEAL (proceed as single task)
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
4 × 3,000 tokens = 12,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 12,000 tokens → ✅ SAFE (well within limits)
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

Planned WIP: 1 agent (Engineer for this task)
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
  - Files: 4 ≤ 14 ✅
  - Tokens: 12K ≤ 42K ✅
  - WIP: 1 ≤ 3 ✅
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
- include/xasm++/syntax/simple_syntax.h - Existing syntax plugin pattern
- src/syntax/simple_syntax.cpp - Implementation reference
- include/xasm++/assembler.h - Assembler API
- include/xasm++/symbol.h - Symbol table interface
- tests/unit/test_simple_syntax.cpp - Testing pattern
```

### Documentation
```
- .ai/tasks/2026-01-24_merlin-syntax-support/10-plan.md - Overall Merlin plan (Phases 1-8)
- .ai/tasks/2026-01-24_merlin-syntax-support/00-contract.md - Feature contract
- ~/Projects/Vintage/tools/vasm-ext/syntax/merlin/syntax.c - Reference implementation
```

### Examples
```
- src/syntax/simple_syntax.cpp - Similar syntax plugin
- tests/unit/test_cpu6502.cpp - TDD pattern with GoogleTest
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
1. Label scoping complexity
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Test exhaustively, follow vasm-ext model

2. Integration with existing Symbol table
   - Probability: Low
   - Impact: Medium
   - Mitigation: Use existing ConcreteSymbolTable API, add tests

3. Number format parsing edge cases
   - Probability: Low
   - Impact: Low
   - Mitigation: Comprehensive test coverage for $hex, %binary, decimal
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-01-24)
- [X] Agent: Orchestrator (2026-01-24)

**Changes to Contract:**
None - Initial version

---

## Notes

This is Phase 1-3 of the larger Merlin Syntax Support feature (Phases 1-8). The overall goal is to assemble Prince of Persia source code. This task focuses on the foundation and basic label system.

**Key Design Decisions:**
- Use existing Symbol table API (no modifications needed)
- Follow SimpleSyntax pattern for consistency
- Incremental TDD: RED-GREEN-REFACTOR cycle mandatory
- Each phase builds on previous (Foundation → Local Labels → DUM Blocks)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-24

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
