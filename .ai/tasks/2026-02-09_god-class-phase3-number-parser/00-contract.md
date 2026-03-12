# Task Contract: God-Class Phase 3 - Number Parser

**Task ID:** 2026-02-09_god-class-phase3-number-parser
**Beads Task:** xasm++-ai4z
**Created:** 2026-02-09
**Priority:** P2

Extract number format parsing into shared NumberParser component.

## Context

From god-class investigation:
- Multiple number formats duplicated across parsers
- Hex: `$FF`, `0xFF`, `0FFH`
- Binary: `%11111111`, `11111111B`
- Octal: `0377`, `377O`, `377Q`
- Decimal: `255`, `255D`

## Objective

Create shared NumberParser that all syntax parsers can use for consistent number parsing.

## Success Criteria

✓ NumberParser component created and tested
✓ Supports all number formats (hex, binary, octal, decimal)
✓ Extensible for syntax-specific formats
✓ All parser tests pass
✓ Reduces number parsing duplication

## Estimated Effort

8-12 hours

**Approved:** Orchestrator 2026-02-09
