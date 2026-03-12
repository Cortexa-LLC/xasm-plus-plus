# Task Contract: A2oSX Build Validation Test

**Task ID:** 2026-02-12_a2osx-validation
**Beads Task:** xasm++-e6h9
**Created:** 2026-02-12
**Requestor:** Project Owner
**Assigned Role:** Engineer
**Workflow:** Standard

---

## Task Description

Validate xasm++ readiness as the official A2oSX Operating System build tool by building all 190 assembly source files, creating automated validation tests, and documenting results.

### Background and Context

A2oSX is a multitasking operating system for Apple II computers with 190+ assembly source files using SCMASM syntax. The project currently uses Merlin assembler, but xasm++ aims to become the official build tool.

**Prerequisites completed:**
- ✅ Directive gap analysis (25 directives identified)
- ✅ 100% SCMASM directive coverage implemented
- ✅ All P0 blockers resolved (.PS, .INB, .DUMMY, .OP, .LIST)
- ✅ 1564 tests passing

### Current State

xasm++ has complete SCMASM directive support but lacks real-world validation with A2oSX source files.

### Desired State

xasm++ successfully builds A2oSX OS with:
- >95% success rate on 190 source files
- Automated validation tests in CI
- Complete documentation for A2oSX developers

---

## Success Criteria

```
✓ Build validation report with success/failure statistics
✓ Integration test added and passing in CI
✓ Documentation complete (build instructions, migration guide)
✓ All existing 1564 tests still passing
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Attempted build of all A2oSX source files (locate repository)
□ Success rate calculated and documented
□ Any failures analyzed with root cause
□ Binary comparison with reference (if available)
```

### Quality Requirements
```
□ Integration test in tests/integration/a2osx/
□ Test builds representative A2oSX files
□ Test runs in CI pipeline
□ All existing 1564 tests still passing
□ Zero regressions introduced
```

### Documentation
```
□ Build validation report in docs/validation/a2osx/
□ Success rate and statistics
□ Known limitations documented
□ Build instructions for A2oSX developers
□ Migration guide from Merlin to xasm++
```

---

## Constraints and Dependencies

### Constraints
```
□ A2oSX source files external to xasm++ repository
□ May not have reference binaries for comparison
□ Complex include dependencies possible
□ Binary-exact matching not required (acceptable variation)
```

### Dependencies
```
✅ xasm++-9p8u: A2oSX directive analysis (COMPLETE)
✅ xasm++-zitw: SCMASM directives implementation (COMPLETE)
□ A2oSX source repository location (TBD)
□ Build order documentation (if available)
```

### Out of Scope
```
✗ Fixing bugs discovered (create separate tasks)
✗ Binary-exact output matching (nice-to-have, not required)
✗ Runtime testing of generated binaries
✗ A2oSX OS functionality testing
✗ Performance optimization
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: ~5 (integration test, documentation)
- Lines of code estimate: ~200-300
- New concepts/patterns: No (uses existing test infrastructure)
- Integration complexity: Low (CI integration straightforward)
- Risk level: Low (validation task, no production code changes)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 5 files (integration test, documentation)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 5 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
5 × 3,000 tokens = 15,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 15,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent → ✅ IDEAL
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

---

## Resources and References

### Relevant Files
```
- tests/integration/ - Integration test directory
- docs/validation/ - Validation documentation directory
- CMakeLists.txt - May need test registration
```

### Documentation
```
- .ai/tasks/2026-02-12_a2osx-directive-analysis/ - Gap analysis results
- .ai/tasks/2026-02-12_a2osx-directives/ - SCMASM implementation
```

### Examples
```
- tests/integration/merlin/ - Similar integration test structure
- tests/e2e/apple2/prince_of_persia/ - Complex real-world test example
```

---

## Assumptions

```
1. A2oSX source files are publicly accessible (GitHub or similar)
2. Binary-exact matching not required (variations acceptable)
3. Validation test can use representative sample files
4. CI pipeline can access external repositories (or copy samples)
5. A2oSX build documentation exists or can be inferred
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. A2oSX source not accessible
   - Probability: Medium
   - Impact: High
   - Mitigation: Locate public repository or use sample files

2. Undiscovered directive bugs
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Document issues, create fix tasks, continue validation

3. Complex macro usage not covered
   - Probability: Low
   - Impact: Medium
   - Mitigation: Test incrementally, fix as needed

4. Reference binaries unavailable
   - Probability: High
   - Impact: Low
   - Mitigation: Document as limitation, focus on successful assembly
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Orchestrator [2026-02-13]
- [ ] Engineer [TBD]

**Changes to Contract:**
[None yet]

---

## Notes

- This is a validation milestone for xasm++ production readiness
- Success means xasm++ can officially replace Merlin for A2oSX builds
- If bugs discovered, create separate fix tasks (don't block validation)
- Documentation critical for A2oSX developer adoption

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-13
