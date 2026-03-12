# Task Contract

**Task ID:** 2026-02-12_a2osx-directive-analysis
**Created:** 2026-02-12
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer (Research/Analysis)
**Workflow:** Research
**Beads Task:** xasm++-9p8u

---

## Task Description

Analyze all 190 A2oSX operating system source files to identify SCMASM assembler directives used, compare against xasm++'s current SCMASM implementation, and produce a gap analysis with prioritized recommendations for missing directive support.

### Background and Context

A2oSX is a multitasking operating system for Apple II computers with 190 assembly source files written in SCMASM (S-C Macro Assembler) syntax. The xasm++ assembler already has basic SCMASM syntax support implemented in `src/syntax/scmasm_syntax.cpp`.

This analysis is needed to determine what additional SCMASM directives must be implemented before xasm++ can build the complete A2oSX operating system. This would make xasm++ the first modern cross-assembler capable of building A2oSX, providing a valuable real-world validation test (similar to Prince of Persia for Merlin syntax).

The A2oSX source files are located at: `~/Projects/Vintage/Apple/A2osX/`

### Current State

- xasm++ has basic SCMASM support (ORG, EQU, DS, etc.)
- A2oSX source files use additional directives that may not be implemented:
  - .OP (CPU selection: 65C02 vs 6502)
  - .INB (include binary file)
  - .LIST (listing control)
  - .TF (target file specification)
  - .DUMMY (define dummy section)
  - .EP (entry point)
  - Possibly others
- No systematic analysis of directive usage has been performed
- Unknown which directives are critical vs optional

### Desired State

- Complete inventory of all SCMASM directives used across 190 A2oSX files
- Gap analysis showing which directives are missing from xasm++
- Priority ranking (P0 blockers, P1 important, P2 nice-to-have)
- Implementation effort estimates for each missing directive
- Clear roadmap for A2oSX build support

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ All 190 A2oSX source files scanned for directive usage
✓ Complete inventory of unique directives with usage counts
✓ Gap analysis document comparing A2oSX usage vs xasm++ implementation
✓ Priority ranking (P0/P1/P2) for each missing directive
✓ Effort estimates (hours) for implementing each directive
✓ Documented in `.ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md`
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements
```
□ Scan all .S and .S.txt files in ~/Projects/Vintage/Apple/A2osX/ recursively
□ Extract all SCMASM directives (lines starting with '.' or standard mnemonics)
□ Count usage frequency for each directive
□ Compare against xasm++ SCMASM implementation (src/syntax/scmasm_syntax.cpp)
□ Identify directives that are MISSING from xasm++
□ Identify directives that are IMPLEMENTED in xasm++
```

### Quality Requirements
```
□ Gap analysis document is clear and actionable
□ Priority rankings have clear justification
□ Effort estimates are reasonable and justified
□ Documentation follows markdown format
□ Results are reproducible (methodology documented)
```

### Deliverables
```
□ `directive-gap-analysis.md` - Main analysis document with:
  - Executive summary
  - Methodology
  - Complete directive inventory (usage counts)
  - Gap analysis table (implemented vs missing)
  - Priority rankings with rationale
  - Effort estimates
  - Recommendations
□ `directive-usage-report.txt` - Raw data (directive frequencies)
□ `missing-directives-list.txt` - Quick reference of gaps
```

---

## Constraints and Dependencies

### Constraints
```
□ READ-ONLY analysis (no code changes)
□ A2oSX files must exist at ~/Projects/Vintage/Apple/A2osX/
□ xasm++ source must be current (src/syntax/scmasm_syntax.cpp)
□ Analysis must complete within single agent session
```

### Dependencies
```
□ A2oSX repository cloned and available
□ xasm++ source code accessible
□ Access to SCMASM documentation (for directive semantics)
```

### Out of Scope
```
✗ Implementation of missing directives (separate task: xasm++-zitw)
✗ Building A2oSX with xasm++ (separate task: xasm++-e6h9)
✗ Fixing bugs in existing SCMASM implementation
✗ Performance optimization
✗ Documentation beyond gap analysis
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files to analyze: 190 (large dataset)
- Lines of code to scan: ~30K-50K total
- New concepts/patterns: Understanding SCMASM directive semantics
- Integration complexity: Low (read-only analysis)
- Risk level: Low (no code changes)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos (see `principles/LEAN-FLOW.md`)

### Batch Size Assessment

**Estimated Files:** 190 files (READ-ONLY analysis, not code modification)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 190 files (READ-ONLY) → ⚠️ SPECIAL CASE
```

**Special Case Justification:**

This task involves analyzing 190 files, which far exceeds normal batch size limits. However:

1. **READ-ONLY Operation:** No code modification, only pattern scanning
2. **Tool-Based Analysis:** Will use `grep`, `awk`, scripts (not manual reading)
3. **Aggregated Output:** Results compressed into summary tables/counts
4. **Methodology:**
   ```bash
   # Automated directive extraction
   find ~/Projects/Vintage/Apple/A2osX -name "*.S" -o -name "*.S.txt" | \
     xargs grep -h "^\s*\." | \
     sort | uniq -c | sort -rn
   ```

**Token Budget:**
```
Analysis workflow:
1. Automated scan (shell commands): ~1K tokens
2. Sample file inspection (5-10 files): ~5K tokens
3. xasm++ comparison: ~3K tokens
4. Gap analysis document: ~8K tokens
Total: ~17K tokens → ✅ SAFE
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

Planned WIP: 1 agent (this analysis task)
Status: ✅ IDEAL
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - READ-ONLY analysis with tool-based automation
- Batch size: 190 files but automated scanning (not manual)
- Token budget: ~17K (within limits)
- WIP: 1 agent

**Rationale:** Analysis tasks are fundamentally different from implementation tasks. Automated scanning of 190 files produces aggregated results (~50 unique directives), not 190× token usage.

---

## Resources and References

### Relevant Files
```
- ~/Projects/Vintage/Apple/A2osX/ - Target codebase (190 .S/.S.txt files)
- src/syntax/scmasm_syntax.cpp - Current xasm++ SCMASM implementation
- include/xasm++/syntax/scmasm_syntax.h - SCMASM header
- src/syntax/scmasm_directive_handlers.cpp - Directive handlers
```

### Documentation
```
- .ai-pack/workflows/research.md - Research workflow guide
- tests/e2e/apple2/prince_of_persia/README.md - Similar validation test example
```

### Examples
```
- SCMASM manual (if available online or in A2oSX docs/)
- Existing SCMASM source files in A2oSX for reference
```

---

## Assumptions

```
1. A2oSX repository is cloned and up-to-date at ~/Projects/Vintage/Apple/A2osX/
2. All .S and .S.txt files use SCMASM syntax consistently
3. xasm++ SCMASM implementation is current (no pending PRs/branches)
4. Standard SCMASM directive syntax: directives start with '.' (e.g., .OP, .INB)
5. Priority can be determined by usage frequency + build criticality
6. Effort estimates don't require detailed implementation planning (rough estimates OK)
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. A2oSX files use non-standard directive syntax
   - Probability: Low
   - Impact: Medium (would complicate parsing)
   - Mitigation: Manual inspection of sample files first

2. Too many missing directives (implementation infeasible)
   - Probability: Low
   - Impact: Medium (would require extended timeline)
   - Mitigation: Priority ranking helps focus on critical directives

3. SCMASM documentation unavailable (can't determine semantics)
   - Probability: Medium
   - Impact: Medium (would require reverse-engineering)
   - Mitigation: Use A2oSX source comments and behavior inference
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-02-12)
- [ ] Agent: Engineer (pending)

**Changes to Contract:**
[None yet]

---

## Notes

- This is a prerequisite for task xasm++-zitw (directive implementation)
- Results will inform priority and scope of implementation work
- If >20 missing directives found, may need to decompose implementation into multiple phases

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-12
