# Task Contract: Z80 Phase 10 - Integration & Polish

**Task ID:** 2026-02-09_z80-phase10-integration
**Beads Task:** xasm++-plrm
**Created:** 2026-02-10
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Refactor

---

## Task Description

Integration testing and polish for EDTASM-M80++ parser. Validate parser against real M80/ZMAC/Z80ASM source code, add edge case tests, optimize performance, and complete documentation.

### Background and Context

This is Phase 10 (final phase) of the Z80 parser feature implementation. All individual features (Phases 1-9) are complete:
- ✅ Phase 1-6: Core features
- ✅ Phase 7: Conditionals
- ✅ Phase 8: Listing control
- ✅ Phase 9: Special features

Now need to validate the complete parser works with real-world Z80 assembly code, handle edge cases, and ensure production quality.

### Current State

EDTASM-M80++ parser has all individual features implemented and unit tested. Need integration tests with real code samples and edge case coverage.

### Desired State

- Parser validated against real M80/ZMAC/Z80ASM source files
- Edge cases handled gracefully
- Performance optimized
- Documentation complete
- Production-ready quality

---

## Success Criteria

```
✓ Integration tests with real Z80 code passing
✓ Edge cases handled (empty files, large files, complex macros)
✓ All tests passing (97%+ maintained)
✓ Build passing with no warnings
✓ Documentation complete
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Integration tests with real M80 code samples
□ Integration tests with real ZMAC code samples
□ Integration tests with real Z80ASM code samples
□ Edge case: Empty files handled
□ Edge case: Large files (>10K lines) handled
□ Edge case: Complex nested macros handled
□ Edge case: Deep conditional nesting handled
□ Error messages clear and helpful
```

### Quality Requirements
```
□ All tests passing
□ Coverage ≥80%
□ No compilation warnings
□ Performance benchmarks documented
```

### Documentation Requirements
```
□ Parser usage guide complete
□ Directive reference complete
□ Example code samples provided
□ Known limitations documented
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files: 5+ (tests, documentation, samples)
- Lines of code: ~500-1000 LOC (mostly tests)
- New patterns: Integration testing
- Integration: High (validates all phases together)
- Risk: Medium (edge cases may reveal bugs)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 6 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 6 files → ⚠️ ACCEPTABLE
```

### Batch Size Justification

Files: 6 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all files related to integration testing
- Single concern - validating complete parser functionality
- Already minimal viable batch for integration testing

**Contingency for token limits:**
- If token limit hit, will decompose into: unit tests + integration tests + documentation

**Estimated tokens:** ~6 × 3,000 = 18,000 tokens
**Status:** Within 25K-32K limit? Yes ✅

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
6 × 3,000 tokens = 18,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 18,000 tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (final integration phase)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3

---

## Resources and References

### Relevant Files
```
- src/syntax/edtasm_m80_plusplus/edtasm_m80_plusplus_syntax.cpp - Parser implementation
- tests/unit/test_edtasm_m80_plusplus_syntax.cpp - Existing tests
- tests/integration/ - New integration tests location
- docs/parsers/ - Parser documentation
```

### Documentation
```
- M80 Assembler Manual - Reference for M80 compatibility
- ZMAC documentation - Reference for ZMAC compatibility
- Z80ASM reference - Reference for Z80ASM compatibility
```

### Example Sources
```
- Real M80 source code samples (if available)
- Real ZMAC source code samples (if available)
- Real Z80ASM source code samples (if available)
```

---

## Dependencies

**Depends on:**
- ✅ Phase 9: Special Features (xasm++-aho) - COMPLETE

**Blocks:**
- Nothing (final phase)

---

## Approvals

**Contract Approved By:**
- [X] Orchestrator: 2026-02-10

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-10
