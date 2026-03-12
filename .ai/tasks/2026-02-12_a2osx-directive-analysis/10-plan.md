# Implementation Plan

**Task ID:** 2026-02-12_a2osx-directive-analysis
**Created:** 2026-02-12
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This is a research/analysis task following the Research Workflow (`.ai-pack/workflows/research.md`). The goal is to analyze 190 A2oSX source files to identify all SCMASM directives used, compare against xasm++'s current implementation, and produce a gap analysis with priority rankings.

The approach uses automated shell scripting to extract directive usage patterns from the codebase, combined with manual inspection of sample files to understand directive semantics. Results will be documented in a comprehensive gap analysis document that will inform the implementation task (xasm++-zitw).

This is READ-ONLY analysis with no code modification, using tool-based aggregation rather than manual file-by-file reading. Token budget is estimated at ~17K, well within safe limits.

**Key Technical Decisions:**
- Automated extraction using `grep`, `awk`, `sort` for scalability
- Manual inspection of 5-10 sample files for directive semantics
- Priority ranking based on usage frequency + build criticality
- Effort estimates without detailed implementation planning (rough order of magnitude)

**Patterns to Use:**
- Shell script automation for large-scale pattern extraction
- Frequency analysis (uniq -c | sort) for usage patterns
- Sampling technique (inspect representative files, not all 190)
- Markdown tables for clear gap analysis presentation

---

## Critical Files Identified

### Files to Modify
```
None - this is a read-only analysis task
```

### Files to Create
```
1. .ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md
   - Purpose: Main deliverable with complete analysis
   - Rationale: Comprehensive documentation for implementation task

2. .ai/tasks/2026-02-12_a2osx-directive-analysis/directive-usage-report.txt
   - Purpose: Raw data (frequency-sorted directive list)
   - Rationale: Reference data for analysis

3. .ai/tasks/2026-02-12_a2osx-directive-analysis/missing-directives-list.txt
   - Purpose: Quick reference list of gaps
   - Rationale: Convenience for implementation planning

4. .ai/tasks/2026-02-12_a2osx-directive-analysis/analyze_directives.sh
   - Purpose: Automated analysis script
   - Rationale: Repeatable process for future re-runs
```

### Files to Read (for context)
```
- ~/Projects/Vintage/Apple/A2osX/**/*.S - A2oSX source files
- ~/Projects/Vintage/Apple/A2osX/**/*.S.txt - A2oSX source files
- src/syntax/scmasm_syntax.cpp - Current xasm++ SCMASM implementation
- src/syntax/scmasm_directive_handlers.cpp - Directive handlers
- include/xasm++/syntax/scmasm_syntax.h - SCMASM header
```

---

## Step-by-Step Implementation Plan

### Phase 1: Automated Directive Extraction
```
Step 1.1: Create analysis automation script
  - Action: Write analyze_directives.sh in task packet directory
  - Files: .ai/tasks/2026-02-12_a2osx-directive-analysis/analyze_directives.sh
  - Tests: N/A (shell script)
  - Verification: Script runs without errors

Step 1.2: Extract all directive usage from A2oSX
  - Action: Run script to scan 190 files and generate frequency report
  - Files: Output to directive-usage-report.txt
  - Tests: Verify output format (count + directive name per line)
  - Verification: Report contains expected directive patterns (.OP, .INB, etc.)

Step 1.3: Review raw output
  - Action: Examine top 50 most-used directives
  - Files: directive-usage-report.txt
  - Tests: N/A (manual review)
  - Verification: Output makes sense (known directives appear)
```

### Phase 2: xasm++ Implementation Inventory
```
Step 2.1: List currently implemented SCMASM directives
  - Action: grep RegisterDirective calls in scmasm_syntax.cpp
  - Files: src/syntax/scmasm_syntax.cpp
  - Tests: N/A (read-only)
  - Verification: Get complete list of registered directives

Step 2.2: Check directive handler implementations
  - Action: grep Handle.*Directive functions in scmasm_directive_handlers.cpp
  - Files: src/syntax/scmasm_directive_handlers.cpp
  - Tests: N/A (read-only)
  - Verification: Confirm which directives have implementations

Step 2.3: Create implemented directives inventory
  - Action: Document all directives currently supported by xasm++
  - Files: Working notes in task packet
  - Tests: N/A (documentation)
  - Verification: List is complete and accurate
```

### Phase 3: Gap Analysis
```
Step 3.1: Cross-reference A2oSX usage vs xasm++ implementation
  - Action: Compare directive-usage-report.txt vs implemented list
  - Files: Both reports
  - Tests: N/A (analysis)
  - Verification: Identify all missing directives

Step 3.2: Categorize by priority
  - Action: Assign P0/P1/P2 based on usage frequency + criticality
  - Files: Gap analysis document
  - Tests: N/A (judgment call)
  - Verification: Priorities make logical sense

Step 3.3: Estimate implementation effort
  - Action: For each missing directive, estimate hours to implement
  - Files: Gap analysis document
  - Tests: N/A (estimation)
  - Verification: Estimates are reasonable ROM (not detailed)
```

### Phase 4: Manual Inspection
```
Step 4.1: Select 5-10 representative A2oSX files
  - Action: Pick files with high directive variety
  - Files: SYS/KERNEL.S.txt, LIB/*.S.txt, SBIN/*.S.txt samples
  - Tests: N/A (sampling)
  - Verification: Sample covers major directive types

Step 4.2: Understand directive semantics from usage
  - Action: For unknown directives, examine actual usage in context
  - Files: Sampled A2oSX files
  - Tests: N/A (learning)
  - Verification: Can explain what each directive does

Step 4.3: Document findings
  - Action: Add directive semantics notes to gap analysis
  - Files: directive-gap-analysis.md
  - Tests: N/A (documentation)
  - Verification: Explanations are clear and accurate
```

### Phase 5: Documentation
```
Step 5.1: Write gap analysis document
  - Action: Create directive-gap-analysis.md with all sections
  - Files: .ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md
  - Tests: N/A (documentation)
  - Verification: Document is comprehensive and actionable

Step 5.2: Create missing directives quick reference
  - Action: Extract just missing directive names to separate file
  - Files: missing-directives-list.txt
  - Tests: N/A (extraction)
  - Verification: List matches gap analysis table

Step 5.3: Write recommendations section
  - Action: Propose phased implementation approach
  - Files: directive-gap-analysis.md (recommendations section)
  - Tests: N/A (strategic planning)
  - Verification: Recommendations are practical and prioritized
```

---

## Testing Strategy

### Unit Tests
```
N/A - This is a research/analysis task with no code implementation
```

### Integration Tests
```
N/A - This is a research/analysis task with no code implementation
```

### Acceptance Tests
```
□ All 190 A2oSX files scanned successfully
  - Setup: A2oSX repository available
  - Execute: Run analyze_directives.sh
  - Verify: directive-usage-report.txt contains all directives

□ Gap analysis document is comprehensive
  - Setup: Complete analysis
  - Execute: Review directive-gap-analysis.md
  - Verify: All sections present and complete

□ Priority rankings are justified
  - Setup: Gap analysis complete
  - Execute: Review P0/P1/P2 assignments
  - Verify: Rationale provided for each priority

□ Effort estimates are reasonable
  - Setup: Gap analysis complete
  - Execute: Review effort estimates
  - Verify: Estimates are ROM and justified
```

### Coverage Target
```
N/A - No code coverage for analysis tasks
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ bash - Shell scripting for automation
□ grep/awk/sort - Text processing tools
□ A2oSX repository - Source files to analyze
```

### Task Dependencies
```
□ A2oSX repository must be cloned at ~/Projects/Vintage/Apple/A2osX
□ xasm++ source code must be accessible
```

### Knowledge Dependencies
```
□ Understanding of SCMASM directive syntax (learning as we go)
□ Familiarity with A2oSX project structure (will explore)
□ Access to SCMASM documentation (online or inferred from code)
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** A2oSX uses non-standard directive syntax
- **Probability:** Low
- **Impact:** Medium (would complicate analysis)
- **Mitigation:** Manual inspection of sample files first
- **Contingency:** Adjust grep patterns if needed

**Risk 2:** SCMASM documentation not available
- **Probability:** Medium
- **Impact:** Medium (harder to understand semantics)
- **Mitigation:** Infer behavior from A2oSX source comments and usage patterns
- **Contingency:** Document assumptions and verify during implementation

**Risk 3:** Too many missing directives (overwhelming scope)
- **Probability:** Low
- **Impact:** Medium (implementation task becomes large)
- **Mitigation:** Priority ranking helps focus on critical directives first
- **Contingency:** Decompose implementation into multiple phases

---

## Rollback Plan

### If Implementation Fails

**N/A - This is read-only analysis with no system changes to rollback**

If analysis proves infeasible:
```
1. Document blockers encountered
2. Provide partial results (what was discovered)
3. Recommend alternative approaches
4. No rollback needed (no changes made)
```

---

## Performance Considerations

### Expected Performance Impact
```
- Shell script execution: <5 minutes for 190 files
- Manual file inspection: 1-2 hours (human time)
- Gap analysis writing: 1-2 hours (human time)
```

### Performance Targets
```
- Script execution: <5 minutes
- Total task completion: 4-6 hours
```

### Monitoring Plan
```
N/A - No production system impact
```

---

## Security Considerations

### Security Checklist
```
N/A - Read-only analysis task with no security impact
```

### Security Review Points
```
N/A - No security-sensitive code
```

---

## Alternative Approaches Considered

### Alternative 1: Manual File-by-File Analysis
**Pros:**
- More thorough understanding of each file
- Better context for directive usage

**Cons:**
- Would take 40-80 hours for 190 files (not scalable)
- High token usage (190 files × 1K tokens = 190K)
- Prone to human error (missing patterns)

**Why Not Chosen:**
Automated scanning + sampling provides 90% of the value at 10% of the cost

### Alternative 2: Use Existing SCMASM Parser
**Pros:**
- Would catch edge cases in directive usage
- More accurate than grep-based extraction

**Cons:**
- xasm++ parser may not handle all A2oSX syntax yet (circular dependency)
- Would require building parser integration first
- Slower than grep-based approach

**Why Not Chosen:**
Grep-based extraction is "good enough" for gap analysis, parser can come later

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: 0.5 hours
- Step 1.1: 0.25 hours (write script)
- Step 1.2: 0.1 hours (run script)
- Step 1.3: 0.15 hours (review output)

Phase 2: 1 hour
- Step 2.1-2.3: 1 hour (inventory xasm++ implementation)

Phase 3: 1.5 hours
- Step 3.1-3.3: 1.5 hours (gap analysis and prioritization)

Phase 4: 1.5 hours
- Step 4.1-4.3: 1.5 hours (manual inspection)

Phase 5: 2 hours
- Step 5.1-5.3: 2 hours (documentation)

Total: 6.5 hours
```

---

## Success Metrics

### Completion Criteria
```
✓ All 190 A2oSX files scanned
✓ Directive usage report generated
✓ Gap analysis document complete
✓ Priority rankings assigned with rationale
✓ Effort estimates provided
✓ Recommendations section written
✓ Deliverables in task packet directory
```

### Quality Metrics
```
- Directive inventory completeness: 100% (all directives found)
- Gap analysis clarity: Human-readable, actionable
- Priority justification: Clear rationale for each P0/P1/P2
- Effort estimates: Reasonable ROM (not detailed)
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator (2026-02-12)
- [ ] User/Stakeholder: Bryan Woodruff (pending)

**Revision History:**
```
v1.0 - 2026-02-12 - Initial plan
```

---

## Notes and Considerations

- This task uses Research Workflow (.ai-pack/workflows/research.md)
- Automated extraction preferred over manual reading for scalability
- Sampling technique (5-10 files) provides adequate semantic understanding
- Token budget ~17K is safe for this approach
- Deliverables will directly inform implementation task (xasm++-zitw)
- If >20 missing directives found, implementation may need phased approach

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-12
