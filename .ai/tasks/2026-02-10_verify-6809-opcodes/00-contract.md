# Task Contract: Verify 6809 Opcodes Against Official Manual

**Task ID:** 2026-02-10_verify-6809-opcodes
**Beads Task:** xasm++-lkbc
**Created:** 2026-02-10
**Parent Task:** 2026-02-09_verify-cpu-manuals (xasm++-v74o)

Systematically verify all 6809 opcode definitions in opcodes_6809.h against the official reference (Lance Leventhal's 6809 Assembly Language Programming).

## Success Criteria
✓ Access 6809 reference manual (Leventhal)
✓ Verify all Page 1 opcodes (unprefixed $00-$FF)
✓ Verify all Page 2 opcodes ($10 prefix)
✓ Verify all Page 3 opcodes ($11 prefix)
✓ Document all discrepancies found
✓ Create comprehensive verification report

## Acceptance Criteria

### Reference Material
□ Access Lance Leventhal's 6809 Assembly Language Programming
□ File: docs/reference/cpu/6809-levanthal.txt or docs/reference/6809/6809-leventhal.txt
□ Verify manual covers all opcode pages

### Page 1 Opcodes Verification (Unprefixed $00-$FF)
□ Verify all valid Page 1 opcodes (~60 valid opcodes)
□ Check mnemonic spelling
□ Check operand encoding
□ Check addressing modes
□ Check flag effects (CC register)
□ Check timing/cycle counts
□ Document any discrepancies

### Page 2 Opcodes Verification ($10 prefix)
□ Verify all $10 xx opcodes (~50 valid opcodes)
□ Check 16-bit operations
□ Check extended addressing modes
□ Check Y register operations
□ Document any discrepancies

### Page 3 Opcodes Verification ($11 prefix)
□ Verify all $11 xx opcodes (~30 valid opcodes)
□ Check U stack operations
□ Check extended addressing modes
□ Document any discrepancies

### Documentation
□ Create verification report: docs/architecture/verification-6809-opcodes.md
□ Report structure includes:
  - Executive summary
  - Page 1 opcodes section
  - Page 2 opcodes section
  - Page 3 opcodes section
  - Discrepancies found (with severity: critical/major/minor)
  - Recommendations
□ Each discrepancy includes:
  - Opcode hex value
  - Current xasm++ definition
  - Official manual definition
  - Severity level
  - Recommended fix

## Scope

**In Scope:**
- Verification of opcode definitions only
- Documentation of discrepancies
- Creation of verification report
- Cross-referencing against Leventhal's reference

**Out of Scope:**
- Fixing any discrepancies found (separate task)
- Modifying opcodes_6809.h
- Changing CPU implementation
- Testing opcode behavior

## Resources

**Files to Read:**
- `include/xasm++/cpu/opcodes_6809.h` - 6809 opcode definitions
- `src/cpu/m6809/cpu_6809.cpp` - 6809 CPU implementation (for context)
- `docs/reference/cpu/6809-levanthal.txt` - Leventhal's reference book
- `docs/reference/6809/6809-leventhal.txt` - Alternate location
- `docs/reference/cpu/6809-architecture.md` - 6809 overview

**Files to Create:**
- `docs/architecture/verification-6809-opcodes.md` - Verification report

**Tools Available:**
- Read, Write, Edit, Grep, Glob, Bash

## Constraints

**Time:**
- Target: 1-2 hours
- Focus on thoroughness over speed

**Quality:**
- Must verify ALL opcode pages
- Must document ALL discrepancies found
- Report must be detailed enough for future engineer to understand findings

**Format:**
- Use markdown format
- Structured by opcode page
- Include hex values, mnemonics, references

## Deliverables

1. **Verification Report** (docs/architecture/verification-6809-opcodes.md)
   - Executive summary with statistics
   - Detailed findings by page
   - Discrepancy list with severity
   - Recommendations

2. **Updated Work Log** (20-work-log.md)
   - Document verification process
   - Note any issues encountered
   - Record findings summary

## Notes

- This is VERIFICATION only - do not fix issues found
- If uncertain about an opcode, mark as "unclear" with notes
- Focus on accuracy - better to take extra time than miss errors
- 6809 has simpler structure than Z80 (3 pages vs complex prefixes)
- Expected total opcodes: ~140 across all pages
- Many $00-$FF slots are invalid/unused - note these as well

**Context:**
The 6809 has a three-page opcode structure:
- Page 1: Basic instructions ($00-$FF, but only ~60 valid)
- Page 2: Extended instructions with $10 prefix (~50 valid)
- Page 3: Extended instructions with $11 prefix (~30 valid)

Many byte combinations are invalid/reserved. Document which opcodes are defined vs undefined.

**Approved:** Orchestrator 2026-02-10
