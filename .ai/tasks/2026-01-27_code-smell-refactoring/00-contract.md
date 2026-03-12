# Task Contract: Code Smell Refactoring (Tier 1 & 2)

**Task ID:** 2026-01-27_code-smell-refactoring
**Created:** 2026-01-27
**Requestor:** Bryan W.
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Implement Tier 1 and Tier 2 improvements identified in code smell analysis to enhance code maintainability and eliminate remaining low/medium-severity smells.

### Background and Context

Recent code smell analysis (xasm++-fka) identified 9 code smells across 5 categories. While zero critical issues exist and code quality is Grade A-, there are opportunities for incremental improvement:

**Tier 1 (Optional - Low effort, low risk):**
1. Extract InitializeInstructionHandlers by category (~1 hour)
2. Extract string utilities to string_utils.h (~2 hours)

**Tier 2 (Deferred - Originally for future phases):**
3. Extract CPU variant classes (~2-3 days)
4. Introduce Address value object (~1 day)

**Note:** Tier 2 items were originally marked "defer to future phases" but user has requested implementation now.

### Current State

- Grade A- code quality (85%+ coverage, 500 passing tests)
- 173-line InitializeInstructionHandlers method (declarative registration)
- Duplicate string utilities in assembler.cpp and merlin_syntax.cpp
- Large Cpu6502 class (478+879 lines) handling all CPU variants
- Raw uint32_t used for addresses throughout codebase

### Desired State

- InitializeInstructionHandlers extracted into 5 category-specific methods
- String utilities consolidated in include/xasm++/util/string_utils.h
- CPU variant classes extracted (Cpu6502Base, Cpu65C02, Cpu65816)
- Address value object providing type safety and validation
- All 500 tests passing
- Zero regression in functionality
- Improved maintainability and extensibility

---

## Success Criteria

```
✓ All tests passing (500/500)
✓ Code coverage maintained (≥85%)
✓ Zero compiler warnings
✓ Zero functional regression
✓ Tier 1 refactorings complete and tested
✓ Tier 2 refactorings complete and tested
✓ Documentation updated
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Tier 1.1: InitializeInstructionHandlers split into 5 methods:
  - RegisterMemoryInstructions() (LDA, LDX, STA, STX, LDY, STY)
  - RegisterArithmeticInstructions() (ADC, SBC, INC, DEC, INX, INY, DEX, DEY)
  - RegisterBranchInstructions() (BEQ, BNE, BCC, BCS, BPL, BMI, BVC, BVS)
  - RegisterStackInstructions() (PHA, PLA, PHP, PLP)
  - RegisterControlInstructions() (NOP, BRK, RTI, RTS, JMP, JSR, CLC, SEC, etc.)

□ Tier 1.2: String utilities module created:
  - include/xasm++/util/string_utils.h (declarations)
  - src/util/string_utils.cpp (implementations)
  - Functions: Trim(), ToUpper(), ParseHex(), ParseBinary()
  - All existing uses updated to reference centralized utilities

□ Tier 2.1: CPU variant class hierarchy:
  - Cpu6502Base class (core 56 opcodes)
  - Cpu65C02 class extends Cpu6502Base (adds 27 opcodes)
  - Cpu65816 class extends Cpu65C02 (adds 100+ opcodes)
  - Proper use of virtual methods and inheritance
  - No duplicate code between variants

□ Tier 2.2: Address value object:
  - Address class in include/xasm++/types/address.h
  - Type-safe construction from uint32_t
  - is_zero_page(), is_absolute(), to_hex() methods
  - Implicit conversion to uint32_t where needed
  - All address-related code updated
```

### Quality Requirements
```
□ All tests passing (500/500)
□ Code coverage ≥85%
□ Zero compiler warnings
□ Zero clang-tidy regressions
□ TDD followed (tests first where applicable)
□ Code review passed (Reviewer validation)
```

### Non-Functional Requirements
```
□ Performance maintained (no measurable slowdown)
□ Binary size acceptable (no significant bloat)
□ Backward compatibility with existing assembly code
□ Incremental commits (one refactoring per commit)
□ Clear commit messages documenting rationale
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain 100% backward compatibility
□ All refactorings must be behavior-preserving
□ Cannot break existing test suite
□ Must follow existing code style and conventions
□ Should minimize churn in test files
```

### Dependencies
```
□ Code smell analysis report (.ai/tasks/2026-01-27_code-smell-analysis/)
□ Existing test suite (500 tests)
□ CMake build system
□ C++20 compiler support
```

### Out of Scope
```
✗ Adding new CPU opcodes or features
✗ Changing assembler behavior or syntax
✗ Performance optimization beyond refactoring
✗ New test scenarios (only maintain existing coverage)
✗ Documentation beyond inline code comments
```

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- Number of files affected: ~15-20 (multiple refactorings)
- Lines of code estimate: ~1,500 LOC touched
- New concepts/patterns: Yes (value object, class extraction)
- Integration complexity: Medium (must maintain compatibility)
- Risk level: Medium (large refactoring with multiple touchpoints)
- Total estimated effort: 4-5 days

**Breakdown by Tier:**
- Tier 1.1 (InitializeInstructionHandlers): 1-2 hours
- Tier 1.2 (String utilities): 2-3 hours
- Tier 2.1 (CPU variants): 2-3 days
- Tier 2.2 (Address value object): 1 day

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 15-20 files (modifications + new files)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 15-20 files → ❌ TOO LARGE - MUST DECOMPOSE
```

### Decomposition Plan (MANDATORY)

This task is too large (15-20 files, 4-5 days effort) and MUST be decomposed:

**Subtask 1: Tier 1 Refactorings (Low Risk)**
- Files: 3-5 files
- Work: InitializeInstructionHandlers extraction + String utilities
- Estimated tokens: ~4,000 tokens
- Estimated effort: 3-4 hours
- Dependencies: None
- Risk: Low (small, localized changes)

**Subtask 2: CPU Variant Extraction (High Risk)**
- Files: 6-8 files (cpu_6502.h/cpp, cpu_65c02.cpp, cpu_65816.cpp, tests)
- Work: Extract Cpu6502Base, refactor inheritance
- Estimated tokens: ~12,000 tokens
- Estimated effort: 2-3 days
- Dependencies: Subtask 1 complete (clean baseline)
- Risk: Medium-High (large refactoring, many tests affected)

**Subtask 3: Address Value Object (Medium Risk)**
- Files: 6-8 files (new address.h, update all uses)
- Work: Introduce Address type, update call sites
- Estimated tokens: ~8,000 tokens
- Estimated effort: 1 day
- Dependencies: Subtasks 1 & 2 complete (stable codebase)
- Risk: Medium (pervasive change across codebase)

**Execution Strategy: SEQUENTIAL** (dependencies require ordered execution)
- Subtask 1 first (low risk, establishes clean baseline)
- Subtask 2 second (high risk, needs stable foundation)
- Subtask 3 third (medium risk, builds on previous work)

**WIP Limit:** 1 agent at a time (sequential execution)

### Token Budget Estimation

**Conservative Estimate:**
```
Subtask 1: 5 files × 3,000 tokens = 15,000 tokens
Subtask 2: 8 files × 3,000 tokens = 24,000 tokens
Subtask 3: 8 files × 3,000 tokens = 24,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
Subtask 1: 15K tokens → ✅ SAFE
Subtask 2: 24K tokens → ⚠️ APPROACHING LIMIT
Subtask 3: 24K tokens → ⚠️ APPROACHING LIMIT
```

**Mitigation for Subtasks 2 & 3:**
- Break large refactorings into multiple commits
- Focus agent on one file at a time where possible
- Use incremental refactoring approach

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
Planned WIP: 1 agent (sequential execution required due to dependencies)

Sequential execution:
Agent 1 (Subtask 1) → complete → Agent 2 (Subtask 2) → complete → Agent 3 (Subtask 3)

Cannot parallelize due to:
- Subtask 2 depends on Subtask 1 (clean baseline)
- Subtask 3 depends on Subtasks 1 & 2 (stable codebase)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [ ] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [X] NO - MUST decompose (batch 15-20 files AND total effort 4-5 days)

**Decomposition is MANDATORY**

Three subtasks will be created as separate Beads tasks:
1. xasm++-[id1]: Tier 1 refactorings (3-4 hours)
2. xasm++-[id2]: CPU variant extraction (2-3 days)
3. xasm++-[id3]: Address value object (1 day)

Each subtask will have its own task packet in `.ai/tasks/YYYY-MM-DD_subtask-name/`

---

## Resources and References

### Relevant Files
```
Tier 1.1:
- src/core/assembler.cpp:158-330 - InitializeInstructionHandlers method

Tier 1.2:
- src/core/assembler.cpp:13-29 - ParseHex, Trim helpers
- src/syntax/merlin/merlin_syntax.cpp:29-57 - StripComments, Trim, ToUpper

Tier 2.1:
- include/xasm++/cpu/cpu_6502.h - Cpu6502 class (478 lines)
- src/cpu/m6502/cpu_6502.cpp - Implementation (879 lines)
- src/cpu/m6502/cpu_65c02.cpp - 65C02 variant (170 lines)
- src/cpu/m6502/cpu_65816.cpp - 65816 variant (308 lines)

Tier 2.2:
- All files using uint32_t for addresses (widespread)
```

### Documentation
```
- .ai/tasks/2026-01-27_code-smell-analysis/code-smell-findings.md - Analysis report
- .ai/tasks/2026-01-27_code-smell-analysis/30-review.md - Review with recommendations
- .ai-pack/quality/clean-code/13-code-smells.md - Code smells standard
```

### Examples
```
- EncodeWithTable() - Example of successful refactoring (M1 god method → clean abstraction)
- Table-driven encoding - Pattern to maintain
```

---

## Assumptions

```
1. All 500 tests must continue passing after each refactoring
2. Performance impact is negligible (refactoring, not optimization)
3. Binary compatibility is not required (recompilation acceptable)
4. Tier 2 items approved despite "defer" recommendation
5. Sequential execution is acceptable (3 subtasks, ~1 week total)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Test breakage during refactoring
   - Probability: Medium (large changes to CPU classes)
   - Impact: High (blocks progress, requires debugging)
   - Mitigation: TDD approach, incremental commits, run tests after each change

2. Performance regression
   - Probability: Low (refactoring preserves logic)
   - Impact: Medium (may need optimization)
   - Mitigation: Benchmark critical paths before/after, use profiler if needed

3. Token limit exceeded during Subtask 2 or 3
   - Probability: Medium (24K tokens approaches limit)
   - Impact: Medium (agent must restart mid-task)
   - Mitigation: Break into smaller commits, focus on one file at a time

4. Scope creep (temptation to add features)
   - Probability: Medium (touching many files)
   - Impact: Low (extends timeline)
   - Mitigation: Strict adherence to "behavior-preserving refactoring only"

5. Integration conflicts between subtasks
   - Probability: Low (sequential execution)
   - Impact: Medium (merge conflicts, rework)
   - Mitigation: Sequential execution with clean baseline between subtasks
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan W. [2026-01-27]
- [ ] Agent: Engineer [Pending]

**Changes to Contract:**
None yet.

---

## Notes

**Decomposition Required:**
This contract defines the overall scope, but work will be executed as 3 separate subtasks due to size and complexity. Each subtask will have its own Beads task and may have its own mini task packet for work tracking.

**Sequential Execution:**
Subtasks must be executed sequentially due to dependencies:
1. Tier 1 first (establishes clean baseline)
2. CPU variants second (high-risk refactoring needs stable foundation)
3. Address value object third (pervasive change benefits from stable CPU code)

**Estimated Timeline:**
- Subtask 1: 3-4 hours
- Subtask 2: 2-3 days
- Subtask 3: 1 day
- **Total: ~4-5 days** (approximately 1 week)

**Beads Tasks:**
- Subtask 1 (Tier 1): xasm++-asp
- Subtask 2 (Tier 2.1): xasm++-fno (depends on asp)
- Subtask 3 (Tier 2.2): xasm++-avo (depends on asp, fno)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-27
