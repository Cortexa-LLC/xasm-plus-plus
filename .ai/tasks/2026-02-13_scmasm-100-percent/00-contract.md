# Contract: Phase 3 - Complete SCMASM 100% Coverage

**Beads Task:** xasm++-f2n8
**Created:** 2026-02-13
**Owner:** Orchestrator → Engineer
**Priority:** P1

---

## Executive Summary

Implement remaining 7 SCMASM directives to achieve 100% coverage for A2oSX compatibility.

**Current:** 72% (18/25 directives)
**Target:** 100% (25/25 directives)

---

## Requirements

### P1 Directives (242 uses in A2oSX)

#### 1. .CS / .CZ - C-Strings (118 uses)
- C-style strings with escape sequences (\a \b \e \f \n \r \t \v \0 \\ \" \' \xHH)
- .CZ adds null terminator
- No high-bit manipulation

#### 2. .TF - Text File/Title (83 uses)
- Set listing title or include text file
- Stub implementation acceptable

#### 3. .DO / .FIN - Conditional Assembly (68 uses)
- Conditional assembly blocks
- Nestable, expression-based

### P2 Directives (23 uses)

#### 4. .EP - Entry Point (21 uses)
- Define program entry point
- Metadata storage

#### 5. .AC - ASCII Compression (2 uses)
- Three-tier nibble encoding tables
- Complex state machine

### P3 Directives (edge cases)

#### 6. .HX - Hex Nibble Storage
- Pack nibbles into bytes

#### 7. .TA - Target Address
- No-op in cross-assembler

---

## Acceptance Criteria

1. All 7 directives implemented
2. 30+ new tests, all passing
3. Zero compiler warnings
4. 100% SCMASM coverage (25/25 directives)

---

## Implementation Order

1. .CS / .CZ (most used)
2. .TF (simple stub)
3. .EP (simple metadata)
4. .HX (simple nibble packing)
5. .TA (no-op)
6. .DO / .FIN (complex conditional)
7. .AC (most complex)

---

## Estimated Effort

20-29 hours conservative
8-12 hours actual (based on Phase 2 efficiency)

---

## References

- vasm-ext comparison: `.ai/tasks/2026-02-13_a2osx-p0-directives/vasm-ext-comparison.md`
- Directive gap analysis: `.ai/tasks/2026-02-12_a2osx-directive-analysis/directive-gap-analysis.md`
