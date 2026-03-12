# Task Contract

**Task ID:** 2026-01-26_pop-analysis
**Created:** 2026-01-26
**Requestor:** Bryan Woodruff (Orchestrator)
**Assigned Role:** Archaeologist / Engineer
**Workflow:** Research
**Beads Task:** xasm++-ftf

---

## Task Description

Analyze all 27 remaining Prince of Persia source files that currently fail to assemble with xasm++. Identify the specific Merlin assembler features and directives that are blocking successful assembly.

### Background and Context

**Current State:**
- ✅ 2/29 PoP files assembling successfully (EQ.S, partial SUBS)
- ✅ Merlin Phases 1-8 complete (50 tests, basic directives working)
- ❌ 27/29 PoP files NOT assembling
- ⚠️ Unknown: Which features are blocking the remaining files

**Why This Task:**
The project goal is to "assemble Prince of Persia source code" (README). We've implemented Merlin Phases 1-8 with basic directives (PUT, DFB, DO/FIN, ASC, etc.), but 93% of PoP files still fail. Before implementing new features, we need data-driven analysis of what's actually blocking assembly.

**What We Suspect (needs verification):**
- MAC...<<< macro system (likely used extensively)
- Advanced string directives (DCI, INV, FLS, REV)
- CPU mode directives (MX, XC, LONGA, LONGI)
- Other unknown Merlin features

### Desired State

**After Completion:**
- ✅ Comprehensive failure report for all 27 files
- ✅ Blocking features identified and catalogued
- ✅ Frequency analysis (which features block most files)
- ✅ Priority roadmap for implementation
- ✅ Complexity estimates for each feature
- ✅ Report saved to `docs/investigations/`

---

## Success Criteria

```
✓ All 27 PoP files analyzed (100% coverage)
✓ Each file's blocking features documented
✓ Feature frequency table created (sorted by impact)
✓ Priority roadmap established (High/Med/Low)
✓ Complexity estimates provided (Small/Med/Large)
✓ Report written to docs/investigations/pop-compatibility-analysis.md
✓ Actionable implementation plan created
✓ No speculation - only observed failures
```

---

## Acceptance Criteria

### Functional Requirements
□ Analysis covers all 27 remaining PoP source files
□ Report identifies specific directives/features blocking each file
□ Frequency analysis shows which features are most critical
□ Priority ranking based on: (frequency × impact)
□ Complexity assessment for implementation planning

### Quality Requirements
□ Report is data-driven (actual errors, not guesses)
□ File paths and line numbers referenced for each issue
□ Examples of usage patterns provided
□ Implementation recommendations included
□ Report formatted in markdown

### Deliverables
□ `docs/investigations/pop-compatibility-analysis.md` created
□ Summary table: File → Blocking Features
□ Priority table: Feature → Frequency → Complexity → Priority
□ Implementation roadmap (Phase B plan)

---

## Constraints and Dependencies

### Constraints
□ Must use actual xasm++ error output (not speculation)
□ Must test each file individually to isolate issues
□ Must provide reproducible test commands
□ Must stay within 2-hour analysis window

### Dependencies
□ Prince of Persia source available at: ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/
□ xasm++ binary built and working
□ Merlin syntax parser functional (Phases 1-8 complete)

### Out of Scope
✗ Implementation of missing features (Phase B)
✗ Fixing bugs in existing directives
✗ Testing binary correctness (just assembly success/failure)
✗ Performance optimization

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Files to analyze: 27
- Expected blocking features: 5-10 different types
- Analysis per file: ~5 minutes
- Report writing: 30 minutes
- **Total:** 1.5-2 hours

---

## Resources and References

### PoP Source Location
```
~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II/01 POP Source/Source/
- 29 total .S files
- 2 currently assembling (EQ.S, partial SUBS)
- 27 need analysis
```

### Currently Assembled Files
```
✅ EQ.S - Equates and constants
✅ SUBS (partial) - Subroutines
```

### xasm++ Test Command
```bash
./xasm++ --cpu 6502 --syntax merlin --output test.bin <file.S>
```

### Reference Implementation
```
~/Projects/Vintage/tools/vasm-ext/syntax/merlin/syntax.c
- Complete Merlin implementation
- 227 directives in table (lines 2737-2967)
- Can reference for directive definitions
```

---

## Assumptions

```
1. PoP source files are valid Merlin syntax (they assembled with original Merlin)
2. Error messages from xasm++ are clear enough to identify missing features
3. Most files will share common blocking features (not unique per file)
4. Analysis can be completed in single session (no multi-day investigation)
5. Existing Merlin Phases 1-8 implementation is stable (no regressions)
```

*Note: If any assumption proves invalid, update this contract.*

---

## Risk Assessment

### Identified Risks
```
1. PoP files may use obscure Merlin features not documented
   - Probability: Medium
   - Impact: Medium (requires research)
   - Mitigation: Use vasm-ext source as reference

2. Error messages may be ambiguous
   - Probability: Low
   - Impact: Low (can examine source directly)
   - Mitigation: Check actual line numbers in files

3. Files may have interdependencies
   - Probability: High (PUT directives link files)
   - Impact: Low (analyze in dependency order)
   - Mitigation: Start with files that PUT others
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (Orchestrator) 2026-01-26
- [ ] Agent: Archaeologist/Engineer (TBD)

**Changes to Contract:**
[None yet]

---

## Notes

**Analysis Strategy:**
1. List all 27 files
2. Attempt to assemble each with xasm++
3. Capture error output
4. Categorize errors by feature type
5. Build frequency table
6. Generate priority roadmap

**Output Format:**
```markdown
# Prince of Persia Compatibility Analysis

## Summary
- Files analyzed: 27
- Blocking features identified: X
- High priority features: Y
- Medium priority: Z

## Detailed Analysis

### File: AUTO.S
- Status: FAILS
- Blocking features:
  * MAC directive (line 45)
  * >>> macro call (line 123)
- Priority: HIGH (macros block 20+ files)

[... repeat for all files ...]

## Priority Roadmap

| Feature | Files Blocked | Complexity | Priority |
|---------|---------------|------------|----------|
| MAC...<<< | 20 | Large | P1 |
| DCI | 15 | Small | P2 |
[...]
```

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-26
