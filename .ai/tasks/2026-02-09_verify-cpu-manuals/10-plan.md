# Implementation Plan

**Task ID:** 2026-02-09_verify-cpu-manuals
**Beads Task:** xasm++-v74o
**Created:** 2026-02-10
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This task involves systematically verifying all Z80 and 6809 opcode definitions in the xasm++ assembler against official CPU reference manuals. The verification will be performed by specialized agents working in parallel on independent CPU architectures.

The verification process will:
1. Use existing reference materials in docs/reference/cpu/ and docs/reference/6809/
2. Acquire official Z80 reference manual (Zilog Z80 CPU User Manual)
3. Cross-reference each opcode definition against official documentation
4. Document any discrepancies, errors, or missing opcodes
5. Create comprehensive verification reports in docs/architecture/

**Key Technical Decisions:**
- **Parallel verification approach**: Z80 and 6809 can be verified independently by separate agents since they are different CPU architectures with no shared dependencies
- **Reference-based verification**: Use official CPU manuals as the authoritative source, not just assembler documentation
- **Structured reporting**: Create detailed verification reports documenting each opcode category (basic, prefixed, etc.)

**Patterns to Use:**
- **Parallel Execution Pattern** - Independent verification tasks for Z80 and 6809
- **Systematic Review Pattern** - Group opcodes by category (basic, CB, ED, DD/FD for Z80; Page 1, 2, 3 for 6809)
- **Documentation-First Pattern** - Create verification reports before making any code changes

---

## Critical Files Identified

### Files to Read (for verification)

**Z80 Opcodes:**
```
1. include/xasm++/cpu/opcodes_z80.h
   - Contains: All Z80 opcode definitions (basic, CB, ED, DD, FD prefixed)
   - Size: 655 lines
   - Purpose: Primary verification target for Z80

2. src/cpu/z80/cpu_z80.cpp
   - Contains: Z80 CPU implementation
   - Purpose: Context for opcode usage and patterns
```

**6809 Opcodes:**
```
1. include/xasm++/cpu/opcodes_6809.h
   - Contains: All 6809 opcode definitions (page 1, 2, 3)
   - Size: 629 lines
   - Purpose: Primary verification target for 6809

2. src/cpu/m6809/cpu_6809.cpp
   - Contains: 6809 CPU implementation
   - Size: 55,796 bytes
   - Purpose: Context for opcode usage and patterns
```

**Reference Materials:**
```
1. docs/reference/cpu/6809-levanthal.txt
   - Lance Leventhal's 6809 Assembly Language Programming
   - Purpose: 6809 opcode reference

2. docs/reference/6809/6809-leventhal.txt
   - Duplicate/alternate location
   - Purpose: 6809 opcode reference

3. docs/reference/cpu/z80-architecture.md
   - Z80 architecture overview
   - Purpose: Context for Z80 design

4. docs/reference/cpu/6809-architecture.md
   - 6809 architecture overview
   - Purpose: Context for 6809 design
```

### Files to Create

**Verification Reports:**
```
1. docs/architecture/verification-z80-opcodes.md
   - Purpose: Complete Z80 opcode verification report
   - Contents:
     * Basic opcodes verification (unprefixed)
     * CB-prefixed opcodes (bit operations)
     * ED-prefixed opcodes (extended instructions)
     * DD-prefixed opcodes (IX register operations)
     * FD-prefixed opcodes (IY register operations)
     * Discrepancies found
     * Recommendations

2. docs/architecture/verification-6809-opcodes.md
   - Purpose: Complete 6809 opcode verification report
   - Contents:
     * Page 1 opcodes verification (unprefixed, $00-$FF)
     * Page 2 opcodes verification ($10 prefix)
     * Page 3 opcodes verification ($11 prefix)
     * Discrepancies found
     * Recommendations

3. docs/architecture/cpu-manual-verification-summary.md
   - Purpose: Executive summary of verification results
   - Contents:
     * Overall statistics (total opcodes checked, errors found)
     * Critical issues requiring immediate attention
     * Minor issues for future cleanup
     * Confidence assessment
     * Recommendations for next steps
```

### Reference Materials to Acquire

**Z80 Manual (REQUIRED):**
```
- Zilog Z80 CPU User Manual (UM008011-0816)
- Or: Z80 Family CPU User Manual
- Official source for Z80 opcodes, timing, flags
- Likely available online from Zilog or archive sites
```

---

## Step-by-Step Implementation Plan

### Phase 1: Preparation and Resource Acquisition

```
Step 1.1: Gather Reference Materials
  - Action: Verify existing 6809 reference materials are accessible
  - Files: docs/reference/cpu/6809-levanthal.txt
  - Action: Acquire official Z80 CPU User Manual (PDF or text)
  - Verification: Can access all required reference materials

Step 1.2: Review Opcode File Structure
  - Action: Read opcodes_z80.h to understand structure
  - Action: Read opcodes_6809.h to understand structure
  - Files: include/xasm++/cpu/opcodes_z80.h, opcodes_6809.h
  - Understanding: How opcodes are defined, what fields exist
  - Verification: Clear understanding of data structures

Step 1.3: Create Task Packets for Verification Agents
  - Action: Create Z80 verification task packet
  - Location: .ai/tasks/2026-02-10_verify-z80-opcodes/
  - Action: Create 6809 verification task packet
  - Location: .ai/tasks/2026-02-10_verify-6809-opcodes/
  - Verification: Task packets follow template structure
```

### Phase 2: Parallel Verification Execution

**CRITICAL: Parallelization Decision**

This task qualifies for MANDATORY parallel execution per Execution Strategy Gate:
- **2 independent subtasks** (Z80 verification, 6809 verification)
- **Different files** (opcodes_z80.h vs opcodes_6809.h)
- **Different CPUs** (no shared state or dependencies)
- **Isolated deliverables** (separate verification reports)

**Parallelization Strategy: PARALLEL**
- Spawn 2 Worker agents simultaneously
- Agent 1: Z80 verification
- Agent 2: 6809 verification
- No dependencies between agents
- Results consolidated by Orchestrator

```
Step 2.1: Spawn Z80 Verification Agent
  - Action: Create Beads task for Z80 verification
  - Action: Spawn Worker agent with Z80 verification instructions
  - Task Packet: .ai/tasks/2026-02-10_verify-z80-opcodes/
  - Deliverable: docs/architecture/verification-z80-opcodes.md
  - Estimated Files: 1 verification report
  - Estimated Time: 2-3 hours

Step 2.2: Spawn 6809 Verification Agent (PARALLEL)
  - Action: Create Beads task for 6809 verification
  - Action: Spawn Worker agent with 6809 verification instructions
  - Task Packet: .ai/tasks/2026-02-10_verify-6809-opcodes/
  - Deliverable: docs/architecture/verification-6809-opcodes.md
  - Estimated Files: 1 verification report
  - Estimated Time: 2-3 hours

Step 2.3: Monitor Parallel Execution
  - Action: Check agent progress periodically
  - Action: Respond to any questions or blockers
  - Action: Ensure both agents have necessary resources
  - Verification: Both agents making steady progress
```

### Phase 3: Results Consolidation and Review

```
Step 3.1: Wait for Agent Completion
  - Action: Wait for both verification agents to complete
  - Verification: Both verification reports created
  - Verification: No blockers reported

Step 3.2: Review Verification Reports
  - Action: Read verification-z80-opcodes.md
  - Action: Read verification-6809-opcodes.md
  - Action: Identify common patterns in findings
  - Action: Assess severity of discrepancies
  - Verification: Reports are complete and thorough

Step 3.3: Create Summary Report
  - Action: Consolidate findings from both reports
  - Action: Create cpu-manual-verification-summary.md
  - Contents:
    * Total opcodes verified (Z80 + 6809)
    * Total discrepancies found
    * Critical issues (wrong opcodes, missing opcodes)
    * Minor issues (documentation, comments)
    * Overall confidence assessment
    * Recommended next steps
  - Verification: Summary accurately reflects both reports

Step 3.4: Document in Task Packet
  - Action: Update 20-work-log.md with results
  - Action: Create 40-acceptance.md with findings
  - Action: Close Beads task with summary
  - Verification: Task packet complete
```

---

## Execution Strategy Analysis

### Subtask Inventory
1. Z80 Verification - Files: opcodes_z80.h, verification report - Independent: YES
2. 6809 Verification - Files: opcodes_6809.h, verification report - Independent: YES

### Independence Assessment
- Total subtasks: 2
- Independent: 2
- Dependencies: None (different CPU architectures)
- File conflicts: None (different source files, different reports)

### Strategy Decision
**Strategy:** PARALLEL
**Rationale:** Both subtasks are completely independent. Z80 and 6809 are different CPU architectures with no shared code. Each agent will:
- Read different opcode files
- Reference different manuals
- Create different verification reports
- No coordination needed during execution

**Workers:** 2 workers (1 per CPU)
**Launch:** Single message block with 2 Task() calls
**Coordination:** None needed during execution; orchestrator consolidates at end

### Implementation Plan
**Workers:** 2 parallel workers
**Launch:** Spawn both in single message block
**Coordination:** Orchestrator consolidates results after both complete

---

## Testing Strategy

### Verification Tests (Not Code Tests)

This is a documentation/verification task, not a code change task. Testing means:

**Completeness Verification:**
```
□ Z80 Verification Report Complete
  - All basic opcodes checked (256 opcodes $00-$FF)
  - All CB-prefixed opcodes checked (256 opcodes)
  - All ED-prefixed opcodes checked (~90 valid opcodes)
  - All DD-prefixed opcodes checked (IX operations)
  - All FD-prefixed opcodes checked (IY operations)
  - Total: ~700+ Z80 opcodes verified

□ 6809 Verification Report Complete
  - All Page 1 opcodes checked ($00-$FF, ~60 valid)
  - All Page 2 opcodes checked ($10xx, ~50 valid)
  - All Page 3 opcodes checked ($11xx, ~30 valid)
  - Total: ~140+ 6809 opcodes verified
```

**Quality Verification:**
```
□ Each opcode entry includes:
  - Opcode hex value
  - Mnemonic
  - Operands
  - Manual reference (page/section)
  - Verification status (✓ correct, ✗ error, ? unclear)
  - Notes on any discrepancies

□ Discrepancies are documented with:
  - Current xasm++ definition
  - Official manual definition
  - Severity (critical/major/minor)
  - Recommended fix
```

**Cross-Reference Verification:**
```
□ Z80 report references official manual sections
□ 6809 report references Leventhal text sections
□ Both reports use consistent format
□ Summary report accurately reflects detailed reports
```

### Acceptance Tests

**User scenario: Verify Accuracy**
- Given: Opcode definitions in opcodes_z80.h and opcodes_6809.h
- When: Cross-referenced against official CPU manuals
- Then: All opcodes verified, discrepancies documented

**User scenario: Identify Issues**
- Given: Verification reports with discrepancies
- When: Review summary report
- Then: Clear understanding of what needs fixing

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ Beads - For task tracking and dependencies
□ Official CPU Manuals - Authoritative reference source
  - Z80: Zilog Z80 CPU User Manual (to be acquired)
  - 6809: Lance Leventhal's book (already have)
```

### Task Dependencies
```
✓ xasm++-46sn (Update Tests - Verify Opcode Refactoring) - COMPLETED
  - This task verified opcode refactoring worked correctly
  - Now we verify opcodes match official specs
```

### Knowledge Dependencies
```
□ Understanding of Z80 instruction set architecture
□ Understanding of 6809 instruction set architecture
□ Familiarity with opcode encoding (hex values)
□ Ability to read CPU technical manuals
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Z80 Manual Not Available Online
- **Probability:** Low (many archive sites have it)
- **Impact:** High (blocks Z80 verification)
- **Mitigation:** Check multiple sources (Zilog, archive.org, datasheets)
- **Contingency:** Use secondary sources (Z80 programming books, technical references)

**Risk 2:** Opcode Definitions Too Complex to Verify Manually
- **Probability:** Medium (hundreds of opcodes)
- **Impact:** Medium (time-consuming, error-prone)
- **Mitigation:** Systematic approach, group by category, use checklists
- **Contingency:** Focus on critical opcodes first, defer exhaustive check

**Risk 3:** Discrepancies Found Are Numerous
- **Probability:** Medium (mature codebase, but possible)
- **Impact:** Medium (requires follow-up fixing task)
- **Mitigation:** Document all discrepancies clearly with severity levels
- **Contingency:** Prioritize critical errors, defer minor issues

### Integration Risks

**Risk 1:** Verification Reports Inconsistent Format
- **Mitigation:** Provide clear template/format to both agents upfront
- **Contingency:** Orchestrator reformats during consolidation

---

## Rollback Plan

This is a documentation task - no code changes. No rollback needed.

If verification reports are incomplete:
- Restart specific verification subtask
- Provide additional guidance to agent

---

## Performance Considerations

### Expected Performance Impact
- No code changes = no performance impact
- This is purely documentation/verification

### Time Estimates
```
Z80 Verification: 2-3 hours (700+ opcodes)
6809 Verification: 1-2 hours (140+ opcodes)
Consolidation: 30 minutes
Total: 3.5-5.5 hours elapsed (with parallel execution)
         5-8 hours serial (if done sequentially)
```

**Parallel execution saves 1.5-2.5 hours** (40-45% time reduction)

---

## Security Considerations

Not applicable - this is documentation/verification only.

---

## Alternative Approaches Considered

### Alternative 1: Sequential Verification (Z80 first, then 6809)
**Pros:**
- Simpler coordination
- Learn from Z80 process, improve 6809

**Cons:**
- Takes 40-45% longer (serial vs parallel)
- No real benefit (tasks are independent)

**Why Not Chosen:**
Tasks are completely independent. Parallel execution is faster with no downside.

### Alternative 2: Single Agent Does Both CPUs
**Pros:**
- Consistent format/style
- Single handoff

**Cons:**
- Much longer elapsed time (5-8 hours)
- Agent fatigue/errors late in task
- No parallelism benefit

**Why Not Chosen:**
Parallelism is safe here (independent CPUs) and significantly faster.

### Alternative 3: Automated Script Verification
**Pros:**
- Very fast
- Repeatable

**Cons:**
- Requires parsing official manual (PDF/text)
- Hard to automate judgment calls ("is this opcode correct?")
- Initial script development time > manual verification

**Why Not Chosen:**
Manual verification is appropriate for one-time audit. Automation overkill.

---

## Timeline Estimate

**With Parallel Execution:**
```
Phase 1 (Preparation): 30 minutes
- Gather materials: 10 min
- Review structure: 10 min
- Create task packets: 10 min

Phase 2 (Verification - PARALLEL): 2-3 hours
- Z80 verification: 2-3 hours (PARALLEL)
- 6809 verification: 1-2 hours (PARALLEL)
- Elapsed: max(2-3, 1-2) = 2-3 hours

Phase 3 (Consolidation): 30 minutes
- Review reports: 15 min
- Create summary: 10 min
- Document results: 5 min

Total Elapsed: 3.5-4.5 hours
Total Agent Effort: 5-8 hours (but parallelized)
```

**Without Parallel Execution (for comparison):**
```
Total Elapsed: 5-8 hours (40-45% slower)
```

---

## Success Metrics

### Completion Criteria
```
✓ Official Z80 manual acquired/accessed
✓ Official 6809 manual accessed (Leventhal)
✓ All Z80 opcodes verified against manual
✓ All 6809 opcodes verified against manual
✓ Verification reports created in docs/architecture/
✓ Summary report created
✓ All discrepancies documented
✓ Task packet complete (work log, acceptance)
```

### Quality Metrics
```
- Verification completeness: 100% of opcodes checked
- Documentation quality: All discrepancies have severity, recommendation
- Report clarity: Summary provides clear action items
- Cross-references: All opcodes traceable to manual sections
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Orchestrator: 2026-02-10

**Revision History:**
```
v1.0 - 2026-02-10 - Initial plan
```

---

## Notes and Considerations

**Key Points:**
1. This is a VERIFICATION task, not a FIX task. If discrepancies found, document them. Fixing is a separate follow-up task.

2. Parallel execution is ideal here - Z80 and 6809 are completely independent.

3. Focus on accuracy over speed. Better to take extra time and catch all issues.

4. Verification reports should be detailed enough that a future engineer can understand exactly what was checked and what was found.

5. If uncertain about an opcode, mark it as "unclear" with notes - don't guess.

**Success looks like:**
- Two comprehensive verification reports
- Clear summary of findings
- Actionable recommendations for any fixes needed
- High confidence that opcode definitions match official specs

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-10
