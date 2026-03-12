# Task Contract: A2osX Macro Investigation

**Task ID:** 2026-02-18_a2osx-macro-investigation
**Created:** 2026-02-18
**Requestor:** Orchestrator
**Assigned Role:** Spelunker (Runtime Investigation)
**Workflow:** Research
**Beads Task:** xasm++-jgx2

---

## Task Description

Investigate A2osX macro invocation failures to determine what SCMASM macro support needs to be implemented in xasm++.

### Background and Context

After implementing CRLF fixes (commits f9c9153, 70bca71), A2osX assembly now progresses from line 77 to line 605+, where it encounters macro invocation failures with `>LDYAI`, `>STYA`, and other macro calls.

### Current State

**Build Command:**
```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make
```

**Current Errors:**
```
error: Invalid argument for >LDYAI: Unsupported instruction: >LDYAI
error: Invalid argument for >STYA: Unsupported instruction: >STYA
error: Invalid argument for >DEBUGOA: Unsupported instruction: >DEBUGOA
error: Invalid argument for MAN: Unsupported instruction: MAN
error: Invalid argument for USR/SRC/A2OSX.S.QC: Unsupported instruction: USR/SRC/A2OSX.S.QC
```

### Desired State

Complete investigation report with:
1. Root cause analysis of macro failures
2. SCMASM macro syntax documentation
3. List of required macro features
4. Implementation recommendations for Engineer

---

## Success Criteria

```
✓ Root cause identified for >LDYAI, >STYA macro failures
✓ SCMASM macro syntax documented (definition + invocation)
✓ List of required macro features created
✓ Implementation recommendations provided
✓ Minimal test cases identified
```

---

## Acceptance Criteria

### Investigation Requirements
```
□ Run A2osX build and capture full error output
□ Examine A2osX source files (A2osX.S.txt, macros.i)
□ Analyze macro definition syntax (.MA/.EM directives)
□ Analyze macro invocation syntax (> prefix)
□ Document parameter passing mechanisms
□ Identify local label scoping rules
```

### Documentation Requirements
```
□ Root cause analysis in work log
□ SCMASM macro feature requirements listed
□ Implementation recommendations with complexity estimate
□ Minimal test cases identified
□ Work log updated with all findings
```

### Quality Requirements
```
□ Evidence-based findings (not assumptions)
□ Clear implementation guidance for Engineer
□ Minimal reproducible test cases
□ Ready for Engineer implementation phase
```

---

## Constraints and Dependencies

### Constraints
```
□ Investigation only - no code changes
□ Must use actual A2osX build output for evidence
□ Must examine source files for syntax understanding
□ Time limit: 30-60 minutes
```

### Dependencies
```
□ A2osX repository at ~/Projects/Vintage/Apple/A2osX/
□ xasm++ build at ~/Projects/Vintage/tools/xasm++/build/bin/xasm++
□ A2osX Makefile with path mappings (already created)
```

### Out of Scope
```
✗ Implementing macro support (Engineer's task)
✗ Modifying xasm++ source code
✗ Modifying A2osX source files
✗ Full SCMASM macro specification
```

---

## Estimated Complexity

**Complexity:** Small (Investigation)

**Rationale:**
- Files to examine: ~3-5 (A2osX.S.txt, macros.i, xasm++ source)
- Investigation scope: Macro syntax and features
- No code changes required
- Risk level: Low

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 0 files (investigation only, no code changes)

This is a pure investigation task - Spelunker will examine files but not modify them.

### Token Budget Estimation

**Conservative Estimate:**
```
Investigation tasks typically use 10K-20K tokens for:
- Reading source files
- Running test commands
- Writing documentation

Status: ✅ SAFE (well below limits)
```

### Work In Progress

**Planned WIP:** 1 agent (Spelunker only)

---

## Resources and References

### Relevant Files to Examine
```
- ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt - Main source with macro invocations
- ~/Projects/Vintage/Apple/A2osX/INC/macros.i - Macro definitions
- ~/Projects/Vintage/Apple/A2osX/SHARED/X.PRINTF.S.txt - Shared code
- src/syntax/scmasm_directive_registry.cpp - Current SCMASM directives
```

### Documentation
```
- tests/e2e/apple2/a2osx/README.md - A2osX E2E test guide
- Previous commits: f9c9153 (CRLF fixes), 70bca71 (E2E docs)
```

---

## Assumptions

```
1. A2osX source uses standard SCMASM macro syntax
2. Macro definitions exist in INC/macros.i or similar files
3. xasm++ has minimal or no macro support currently
4. Investigation can be completed in 30-60 minutes
```

---

## Risk Assessment

### Identified Risks
```
1. Macro syntax may be complex/extensive
   - Probability: Medium
   - Impact: Medium (affects implementation scope)
   - Mitigation: Document minimum viable features first

2. Multiple macro-related issues may exist
   - Probability: High
   - Impact: Low (all documented together)
   - Mitigation: Prioritize by frequency/criticality
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Orchestrator: Claude Sonnet 4.5 [2026-02-18]
- [ ] Spelunker: [Will claim task]

---

## Notes

This investigation determines scope for the next implementation phase. Findings will guide Engineer's macro support implementation.

Expected deliverables in work log:
1. Root cause analysis
2. SCMASM macro requirements
3. Implementation recommendations
4. Test cases

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-18
