# Task Contract: Verify Z80 Opcodes Against Official Manual

**Task ID:** 2026-02-10_verify-z80-opcodes
**Beads Task:** xasm++-1491
**Created:** 2026-02-10
**Parent Task:** 2026-02-09_verify-cpu-manuals (xasm++-v74o)

Systematically verify all Z80 opcode definitions in opcodes_z80.h against the official Zilog Z80 CPU User Manual.

## Success Criteria
✓ Acquire/access official Zilog Z80 CPU User Manual
✓ Verify all basic opcodes (unprefixed $00-$FF)
✓ Verify all CB-prefixed opcodes (bit operations)
✓ Verify all ED-prefixed opcodes (extended instructions)
✓ Verify all DD-prefixed opcodes (IX register operations)
✓ Verify all FD-prefixed opcodes (IY register operations)
✓ Document all discrepancies found
✓ Create comprehensive verification report

## Acceptance Criteria

### Reference Material
□ Obtain official Zilog Z80 CPU User Manual (UM008011-0816 or equivalent)
□ Verify manual covers all opcode categories

### Basic Opcodes Verification ($00-$FF)
□ Verify all 256 basic opcodes against manual
□ Check mnemonic spelling
□ Check operand encoding
□ Check flag effects
□ Check timing/cycle counts
□ Document any discrepancies

### CB-Prefixed Opcodes Verification (Bit Operations)
□ Verify all CB xx opcodes (256 combinations)
□ Check bit manipulation instructions (BIT, SET, RES)
□ Check rotation/shift instructions (RLC, RRC, RL, RR, SLA, SRA, SRL)
□ Document any discrepancies

### ED-Prefixed Opcodes Verification (Extended Instructions)
□ Verify all valid ED xx opcodes (~90 valid opcodes)
□ Check block operations (LDIR, LDDR, CPIR, CPDR, etc.)
□ Check I/O operations (IN, OUT, INI, IND, OUTI, OUTD)
□ Check special instructions (NEG, RETI, RETN, etc.)
□ Document invalid ED xx combinations
□ Document any discrepancies

### DD-Prefixed Opcodes Verification (IX Operations)
□ Verify all DD xx opcodes (IX register variants)
□ Check IX indexing modes
□ Check DD CB xx yy opcodes (IX indexed bit operations)
□ Document any discrepancies

### FD-Prefixed Opcodes Verification (IY Operations)
□ Verify all FD xx opcodes (IY register variants)
□ Check IY indexing modes
□ Check FD CB xx yy opcodes (IY indexed bit operations)
□ Document any discrepancies

### Documentation
□ Create verification report: docs/architecture/verification-z80-opcodes.md
□ Report structure includes:
  - Executive summary
  - Basic opcodes section
  - CB-prefixed section
  - ED-prefixed section
  - DD-prefixed section
  - FD-prefixed section
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
- Cross-referencing against official manual

**Out of Scope:**
- Fixing any discrepancies found (separate task)
- Modifying opcodes_z80.h
- Changing CPU implementation
- Testing opcode behavior

## Resources

**Files to Read:**
- `include/xasm++/cpu/opcodes_z80.h` - Z80 opcode definitions
- `src/cpu/z80/cpu_z80.cpp` - Z80 CPU implementation (for context)
- Official Zilog Z80 CPU User Manual (to be acquired)
- `docs/reference/cpu/z80-architecture.md` - Z80 overview

**Files to Create:**
- `docs/architecture/verification-z80-opcodes.md` - Verification report

**Tools Available:**
- Read, Write, Edit, Grep, Glob, Bash
- Internet access (if needed to acquire manual)

## Constraints

**Time:**
- Target: 2-3 hours
- Focus on thoroughness over speed

**Quality:**
- Must verify ALL opcode categories
- Must document ALL discrepancies found
- Report must be detailed enough for future engineer to understand findings

**Format:**
- Use markdown format
- Structured by opcode category
- Include hex values, mnemonics, references

## Deliverables

1. **Verification Report** (docs/architecture/verification-z80-opcodes.md)
   - Executive summary with statistics
   - Detailed findings by category
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
- Z80 has complex prefix system - be systematic
- Expected total opcodes: ~700+ across all categories

**Approved:** Orchestrator 2026-02-10
