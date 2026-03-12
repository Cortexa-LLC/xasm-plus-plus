# Task Contract

**Task ID:** 2026-01-25_phase2-enhanced-features
**Created:** 2026-01-25
**Requestor:** Bryan Woodruff
**Assigned Role:** Orchestrator
**Workflow:** Feature
**Beads Task:** xasm++-tzq

---

## Task Description

Implement Phase 2 enhanced assembly features for xasm++ cross-assembler, building on Phase 1 MVP foundation.

### Background and Context

Phase 1 MVP delivered:
- Working assembler binary (xasm++)
- 6502 CPU support (~20 opcodes)
- Simple and Merlin syntax parsers
- Binary output format
- 402 passing tests

Phase 2 must add production-ready features:
- Multi-pass assembly (forward reference support)
- Complete 6502 instruction set (all 56 legal opcodes)
- All addressing modes
- Symbol resolution

Without these features, xasm++ cannot assemble real-world 6502 code.

### Current State

- ~20 6502 opcodes implemented
- Single-pass assembly only
- Forward references fail
- Incomplete addressing mode coverage
- Merlin syntax fully working (50 tests)

### Desired State

- All 56 legal 6502 opcodes working
- 2-pass assembly with forward reference resolution
- All 6502 addressing modes implemented
- Real-world 6502 code assembles successfully

---

## Success Criteria

✓ All 56 legal 6502 opcodes implemented and tested
✓ Multi-pass assembly working (minimum 2 passes)
✓ Forward references resolve correctly
✓ All addressing modes supported
✓ 100% tests passing (existing + new)
✓ Zero compiler warnings
✓ Integration tests with real 6502 code
✓ Code coverage ≥85% overall, ≥95% for new code

---

## Acceptance Criteria

### Functional Requirements
□ Multi-pass assembler (min 2 passes)
□ All 56 legal 6502 opcodes
□ All addressing modes (implied, immediate, ZP, abs, indexed, indirect)
□ Forward label references work
□ Symbol table with proper scoping

### Quality Requirements
□ All tests passing
□ Code coverage 85%+ overall, 95%+ new code
□ Zero compiler warnings (mandatory)
□ TDD compliance verified
□ Code review approved
□ Documentation updated

### Non-Functional Requirements
□ Performance acceptable (2-pass <2x slower than 1-pass)
□ Memory usage reasonable
□ Error messages clear
□ Backward compatibility with Phase 1

---

## Constraints and Dependencies

### Constraints
□ Must maintain Phase 1 architecture (plugin system)
□ Must support both Simple and Merlin syntax
□ Zero breaking changes to existing API
□ C++17 standard (no C++20)

### Dependencies
□ Phase 1 MVP (xasm++-ac9) ✅ COMPLETE
□ Merlin syntax (xasm++-2xe) ✅ COMPLETE

### Out of Scope
✗ 65C02 extended opcodes (defer to Phase 3)
✗ 65816 16-bit opcodes (defer to Phase 3)
✗ Macro system (Phase 4+)
✗ Listing file generation (Phase 4+)
✗ Source-level debugging support (Phase 5+)

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- Number of files affected: ~8-12 files
- Lines of code estimate: ~2000-3000 LOC
- New concepts/patterns: Multi-pass architecture, symbol resolution
- Integration complexity: Medium (affects CPU, syntax, assembler core)
- Risk level: Medium (architectural changes to core)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** ~10 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 10 files → ⚠️ ACCEPTABLE
```

### Batch Size Justification

Files: 10 (within acceptable range but requires justification)

**Why not decomposed further:**
- Already decomposed into Beads subtasks (xasm++-gdx, xasm++-d9u)
- High cohesion - assembler core changes affect all components
- Subtasks have dependencies (multi-pass must precede full opcode set)

**Contingency for token limits:**
- If token limit hit, will decompose into: multi-pass + opcodes subtasks

**Estimated tokens:** ~10 × 3000 = 30,000 tokens
**Status:** Within 25K-32K limit? ⚠️ APPROACHING LIMIT - Monitor closely

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
10 × 3,000 tokens = 30,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT  ← WE ARE HERE
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 30,000 tokens → ⚠️ APPROACHING LIMIT
```

**Action:** Delegate to Engineer agent with explicit instruction to work incrementally and commit frequently.

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)  ← RECOMMENDED
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (sequential execution)
```

**Execution Strategy:**
1. Engineer agent: Implement multi-pass + opcodes
2. Tester agent: Validate tests + TDD compliance
3. Reviewer agent: Code quality review
4. Orchestrator: Commit after approvals

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size 10 files AND token budget 30K AND WIP 1
- [ ] NO - MUST decompose

**Rationale:** Within acceptable limits but requires careful monitoring. Engineer should work incrementally.

---

## Resources and References

### Relevant Files
```
- include/xasm++/assembler.h - Core assembler interface
- src/assembler.cpp - Multi-pass logic goes here
- include/xasm++/cpu/cpu_6502.h - CPU interface
- src/cpu/cpu_6502.cpp - Opcode implementations
- include/xasm++/syntax/*.h - Syntax parser interfaces
- tests/unit/test_cpu6502.cpp - CPU tests
- tests/integration/*.asm - Integration tests
```

### Documentation
```
- docs/architecture/assembler-core.md - Core architecture
- docs/cpu/6502-opcodes.md - 6502 instruction reference
- http://www.6502.org/tutorials/6502opcodes.html - External reference
```

### Examples
```
- Merlin implementation (src/syntax/merlin/) - Multi-pass patterns
- Phase 1 CPU (src/cpu/cpu_6502.cpp) - Existing opcode structure
```

---

## Assumptions

```
1. Existing symbol table can be extended for multi-pass (no rewrite needed)
2. CPU plugin interface supports all addressing modes
3. Two passes sufficient for 6502 (no complex relocations)
4. Test infrastructure can handle multi-pass testing
5. Performance overhead of 2-pass <100% (acceptable)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Token limit exceeded during implementation
   - Probability: Medium
   - Impact: High (agent fails mid-implementation)
   - Mitigation: Incremental commits, work in small steps

2. Breaking changes to existing API
   - Probability: Low
   - Impact: High (breaks Phase 1 tests)
   - Mitigation: Run full test suite frequently, TDD discipline

3. Performance degradation from 2-pass
   - Probability: Medium
   - Impact: Medium (slower builds)
   - Mitigation: Profile and optimize if >2x slowdown
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (Orchestrator) 2026-01-25
- [ ] Agent: Engineer (TBD)

**Changes to Contract:**
[None yet]

---

## Notes

**Execution Plan:**
1. Spawn Engineer agent with xasm++-tzq task
2. Engineer analyzes and creates detailed plan
3. Engineer implements incrementally with frequent commits
4. Spawn Tester for validation
5. Spawn Reviewer for code quality
6. Orchestrator commits after approvals

**Key Success Factor:** Incremental development to stay within token limits.

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-25
