# Task Contract

**Task ID:** 2026-02-16_analyze-missing-directives
**Beads Task:** xasm++-55bg
**Created:** 2026-02-16
**Requestor:** Orchestrator
**Assigned Role:** Inspector
**Workflow:** Research

---

## Task Description

Perform comprehensive analysis of all S-C Macro Assembler directives needed to assemble A2osX, identifying what's missing and what needs to be implemented.

### Background

A2osX.S.txt currently fails at line 23 with ".DUMMY not yet implemented". Rather than implementing directives one-by-one as we encounter errors, we need a complete analysis upfront to implement all missing directives efficiently.

### Current State

- A2osX uses multiple S-C Macro Assembler directives
- Some are implemented, some are not
- We need a complete gap analysis before implementation

### Desired State

Complete documentation of:
1. All directives used in A2osX
2. All directives currently implemented in xasm++
3. Gap analysis (what's missing)
4. Description of what each missing directive does
5. Priority classification (critical vs optional/no-op)

---

## Success Criteria

```
✓ Complete list of directives used in A2osX
✓ Complete list of implemented directives in xasm++
✓ Gap analysis with missing directives identified
✓ Each missing directive documented with purpose
✓ Priority classification for implementation order
✓ Deliverable: Markdown report ready for engineer
```

---

## Analysis Approach

### Step 1: Extract A2osX Directives

Scan all A2osX source files for directives:
```bash
cd ~/Projects/Vintage/Apple/A2osX
grep -h "^\s\+\." inc/*.i A2osX.S.txt | extract directives
```

### Step 2: List Implemented Directives

Check xasm++ source:
```bash
grep directive_registry src/syntax/scmasm_syntax.cpp
```

### Step 3: Gap Analysis

Compare: Used in A2osX ∩ Not in xasm++ = Missing directives

### Step 4: Research Each Missing Directive

For each missing directive, document:
- Purpose (what does it do?)
- Syntax (how is it used?)
- Examples from A2osX
- Critical vs optional (does A2osX need it?)

### Step 5: Prioritize

Classify as:
- **P0 Critical** - Required for A2osX assembly
- **P1 Important** - Used but might have workarounds
- **P2 Optional** - No-ops or listing control (safe to stub)

---

## Resources

**A2osX Source:**
- `~/Projects/Vintage/Apple/A2osX/A2osX.S.txt` - Main file
- `~/Projects/Vintage/Apple/A2osX/inc/*.i` - Include files

**xasm++ Source:**
- `src/syntax/scmasm_syntax.cpp` - Directive registry
- `src/syntax/scmasm_directive_handlers.cpp` - Implementations

**Documentation:**
- S-C Macro Assembler manual (if available online)
- Compare with implemented directives to understand patterns

---

## Deliverable Format

Create report: `.ai/tasks/2026-02-16_analyze-missing-directives/ANALYSIS.md`

```markdown
# S-C Macro Assembler Directive Analysis for A2osX

## Summary
- Total directives in A2osX: X
- Currently implemented: Y
- Missing (need implementation): Z

## Directives Used in A2osX
[Complete list with usage counts]

## Currently Implemented in xasm++
[Complete list]

## Missing Directives (Gap Analysis)

### Critical (P0) - Must Implement
1. .DIRECTIVE1 - Description, usage, example
2. .DIRECTIVE2 - Description, usage, example

### Important (P1) - Should Implement
1. .DIRECTIVE3 - Description, usage, example

### Optional (P2) - Can Stub/Skip
1. .DIRECTIVE4 - Description, why optional

## Implementation Recommendations
[Ordered list of what to implement first]
```

---

## Constraints

- Analysis only, no implementation
- Must be comprehensive (find ALL missing directives)
- Must research each directive's purpose
- Classification must be accurate (critical vs optional)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-16
