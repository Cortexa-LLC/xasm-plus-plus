# Implementation Plan

**Task ID:** 2026-01-22_complete-6502-syntax-parsing
**Created:** 2026-01-22
**Workflow:** Feature Development (`.ai-pack/workflows/feature.md`)

---

## Workflow Selection

**Chosen Workflow:** Feature Development

**Rationale:**
- Adding new parsing capabilities (new functionality)
- Extends existing assembler to support complete 6502 syntax
- Well-defined requirements (8 addressing mode syntaxes)
- Medium complexity (string parsing with edge cases)

---

## Implementation Approach

### High-Level Strategy

**Goal:** Extend `DetermineAddressingMode()` function in `src/core/assembler.cpp` to recognize all 8 missing addressing mode syntaxes.

**Approach:** Incremental implementation using TDD, organized into 4 groups based on syntax complexity.

### Group Organization

**Group 1: Accumulator Mode** (Simplest)
- Syntax: `A` (single character)
- Example: `ASL A`, `LSR A`, `ROL A`, `ROR A`
- Complexity: Low (exact match)
- Tests: ~5 tests

**Group 2: Indexed Modes (,X and ,Y)** (Medium)
- Syntax: `$addr,X` or `$addr,Y` or `label,X` or `label,Y`
- Examples: `LDA $80,X`, `LDX $80,Y`, `LDA $1234,X`, `STA table,Y`
- Complexity: Medium (comma detection, distinguish ZP vs Absolute)
- Tests: ~10 tests (ZeroPageX, ZeroPageY, AbsoluteX, AbsoluteY, labels, whitespace)

**Group 3: Simple Indirect Mode** (Medium)
- Syntax: `($addr)`
- Example: `JMP ($1234)`
- Complexity: Medium (parentheses detection)
- Tests: ~3 tests (absolute address, whitespace)

**Group 4: Complex Indirect Modes** (Highest)
- Syntax: `($addr,X)` (IndexedIndirect) and `($addr),Y` (IndirectIndexed)
- Examples: `LDA ($80,X)`, `STA ($40),Y`
- Complexity: High (parentheses + comma position matters)
- Tests: ~12 tests (both modes, whitespace, error cases)

**Group 5: Integration Tests** (Final validation)
- Full assembly programs using all syntaxes
- Complexity: Medium (end-to-end validation)
- Tests: ~10 integration tests

**Group 6: Error Handling** (Throughout)
- Invalid syntax detection
- Clear error messages
- Tests: ~10 error tests

**Total Estimated Tests:** ~50 new tests (30 unit + 10 integration + 10 error)

---

## Implementation Details

### Files to Modify

#### Primary File: `src/core/assembler.cpp`
**Function:** `DetermineAddressingMode(const std::string& operands)`
- **Current:** 23 lines, 4 addressing modes
- **After:** ~80-100 lines, 12 addressing modes (all syntaxes)

**Changes:**
- Add accumulator detection: `if (trimmed == "A")`
- Add indexed detection: look for `,X` or `,Y` suffix
- Add parentheses detection for indirect modes
- Add helper function for parsing indirect variants (optional)

#### Test Files to Modify

**`tests/unit/test_assembler.cpp`** (Integration tests)
- Add ~10 new tests for complete assembly programs
- Test all addressing modes in realistic code
- Examples:
  - Zero-page indexed loop: `LDA $80,X`
  - Indirect jump table: `JMP ($1234)`
  - Sprite drawing with indexed indirect: `LDA ($40,X)`

**Create new file:** `tests/unit/test_syntax_parsing.cpp` (Optional but recommended)
- Unit tests specifically for `DetermineAddressingMode()` function
- Makes it easy to test all syntax variants in isolation
- ~30 unit tests
- **Note:** May need to expose `DetermineAddressingMode()` for testing (make it a method, or extract to helper class)

---

## TDD Strategy

### RED-GREEN-REFACTOR Cycle

**For each group:**

1. **RED Phase:**
   - Write failing test for new syntax variant
   - Example: `TEST(SyntaxTest, AccumulatorMode) { EXPECT_EQ(DetermineMode("A"), Accumulator); }`
   - Run test → Verify it fails
   - Commit: `test: add test for accumulator mode syntax (RED)`

2. **GREEN Phase:**
   - Add minimal parsing logic to make test pass
   - Example: Add `if (trimmed == "A") return AddressingMode::Accumulator;`
   - Run test → Verify it passes
   - Run ALL tests → Verify no regressions
   - Commit: `feat: parse accumulator mode syntax (GREEN)`

3. **REFACTOR Phase:**
   - Clean up parsing logic
   - Extract helpers if needed
   - Improve readability
   - Run ALL tests → Verify still passing
   - Commit: `refactor: clean up syntax parsing logic (REFACTOR)`

**Commit Strategy:**
- One commit per group (after GREEN or REFACTOR phase)
- Clear commit messages describing syntax added
- Examples:
  - `feat(parser): add accumulator mode syntax support`
  - `feat(parser): add indexed addressing mode syntax (,X and ,Y)`
  - `feat(parser): add indirect addressing mode syntax`
  - `feat(parser): add IndexedIndirect and IndirectIndexed syntax`
  - `test: add integration tests for complete 6502 syntax`
  - `feat(parser): add error handling for invalid syntax`

---

## Parsing Logic Design

### Proposed Algorithm

```
DetermineAddressingMode(operands):
  if operands.empty():
    return Implied

  trimmed = Trim(operands)

  // Group 1: Accumulator
  if trimmed == "A":
    return Accumulator

  // Group 2 & 4: Check for parentheses (indirect variants)
  if trimmed[0] == '(':
    // Complex: Parse IndexedIndirect vs IndirectIndexed vs Indirect
    if contains(trimmed, ",X)"):
      // ($addr,X) → IndexedIndirect
      extract address from trimmed
      return IndirectX
    else if contains(trimmed, "),Y"):
      // ($addr),Y → IndirectIndexed
      extract address from trimmed
      return IndirectY
    else:
      // ($addr) → Indirect
      extract address from trimmed
      return Indirect

  // Group 2: Check for indexed (,X or ,Y)
  if contains(trimmed, ",X") or contains(trimmed, ", X"):
    // $addr,X or label,X
    extract address/label from trimmed (before comma)
    if value <= 0xFF:
      return ZeroPageX
    else:
      return AbsoluteX

  if contains(trimmed, ",Y") or contains(trimmed, ", Y"):
    // $addr,Y or label,Y
    extract address/label from trimmed (before comma)
    if value <= 0xFF:
      return ZeroPageY
    else:
      return AbsoluteY

  // Group 3: Immediate
  if trimmed[0] == '#':
    return Immediate

  // Existing: Absolute or ZeroPage
  if trimmed[0] == '$':
    value = ParseHex(trimmed)
    if value <= 0xFF:
      return ZeroPage
    else:
      return Absolute

  // Default: Absolute (label reference)
  return Absolute
```

### Helper Functions (Optional)

**May need:**
- `ExtractAddress()` - Remove parentheses, commas, index registers
- `HasSuffix(str, suffix)` - Check for `,X` or `,Y`
- `TrimParens()` - Strip `(` and `)`

**Decision:** Start without helpers, refactor if code gets messy.

---

## Test Strategy

### Unit Tests (30 tests)

**Test `DetermineAddressingMode()` directly:**

**Accumulator (5 tests):**
- `"A"` → Accumulator
- `" A "` → Accumulator (whitespace)
- `"a"` → Error or Absolute? (case sensitivity)
- `"X"` → Absolute (not accumulator)
- `"Y"` → Absolute (not accumulator)

**Indexed X (5 tests):**
- `"$80,X"` → ZeroPageX
- `"$1234,X"` → AbsoluteX
- `"$80, X"` → ZeroPageX (whitespace)
- `"table,X"` → AbsoluteX (label)
- `"$FF,X"` → ZeroPageX (boundary)

**Indexed Y (5 tests):**
- `"$80,Y"` → ZeroPageY
- `"$1234,Y"` → AbsoluteY
- `"$80, Y"` → ZeroPageY (whitespace)
- `"buffer,Y"` → AbsoluteY (label)
- `"$00,Y"` → ZeroPageY (boundary)

**Indirect (3 tests):**
- `"($1234)"` → Indirect
- `"( $1234 )"` → Indirect (whitespace)
- `"($FFFF)"` → Indirect (boundary)

**IndexedIndirect (6 tests):**
- `"($80,X)"` → IndirectX
- `"($00,X)"` → IndirectX (boundary)
- `"( $80 , X )"` → IndirectX (whitespace)
- `"($FF,X)"` → IndirectX (max ZP)
- `"($1234,X)"` → Error (not zero page)
- `"($80,Y)"` → Error (wrong register)

**IndirectIndexed (6 tests):**
- `"($80),Y"` → IndirectY
- `"($00),Y"` → IndirectY (boundary)
- `"( $80 ) , Y"` → IndirectY (whitespace)
- `"($FF),Y"` → IndirectY (max ZP)
- `"($1234),Y"` → Error (not zero page)
- `"($80),X"` → Error (wrong register)

### Integration Tests (10 tests)

**Complete assembly programs:**

**Test 1:** Zero-page indexed loop
```assembly
    LDX #$00
loop:
    LDA table,X
    STA $80,X
    INX
    BNE loop
```

**Test 2:** Accumulator shifts
```assembly
    LDA #$42
    ASL A
    ROL A
    LSR A
    ROR A
```

**Test 3:** Indirect jump
```assembly
    JMP ($1234)
```

**Test 4:** IndexedIndirect sprite drawing
```assembly
    LDY #$00
spriteLoop:
    LDA ($40,X)
    STA ($80),Y
    INY
    BNE spriteLoop
```

**Test 5:** AbsoluteY scrolling
```assembly
    LDY scrollOffset
    LDA screenData,Y
    STA $0400,Y
```

**Test 6-10:** More realistic programs testing combinations

### Error Tests (10 tests)

**Invalid syntax:**
- `"($80,Y)"` → Error: IndexedIndirect requires ,X
- `"($1234,X)"` → Error: ZeroPage address required
- `"($80),X"` → Error: IndirectIndexed requires ,Y
- `"$80,Z"` → Error: Invalid index register
- `"(label,X)"` → Error: Indirect modes require address literal
- `"A,X"` → Error: Cannot index accumulator
- `"#$80,X"` → Error: Cannot index immediate
- `"($80"` → Error: Unclosed parenthesis
- `"$80,)"` → Error: Malformed index
- `"(,X)"` → Error: Missing address

---

## Execution Strategy

### Sequential Execution (No Parallelization)

**Rationale:**
- Single file modification (`src/core/assembler.cpp`)
- Groups depend on shared parsing function
- Cannot parallelize without merge conflicts
- Tests run sequentially anyway

**Execution Order:**
1. Group 1: Accumulator (simplest, builds confidence)
2. Group 2: Indexed modes (,X ,Y)
3. Group 3: Simple Indirect
4. Group 4: Complex Indirect modes
5. Group 5: Integration tests
6. Group 6: Error handling (incrementally throughout)

### Progress Tracking

**Todo List:**
- [ ] Group 1: Accumulator mode
- [ ] Group 2: Indexed modes (,X ,Y)
- [ ] Group 3: Simple Indirect
- [ ] Group 4: Complex Indirect modes
- [ ] Group 5: Integration tests
- [ ] Group 6: Error handling
- [ ] Tester validation
- [ ] Reviewer validation
- [ ] Acceptance

---

## Dependencies

### Technical Dependencies
- ✅ Phase 2.3 complete (all CPU opcodes implemented)
- ✅ Assembler infrastructure exists
- ✅ Test infrastructure (Google Test) set up

### Blocking Dependencies
- None

---

## Risks and Mitigation

### Risk 1: Parsing Ambiguity
**Risk:** Complex syntax like `($80),Y` vs `($80,X)` hard to distinguish
**Likelihood:** Low (comma position differs)
**Impact:** High (wrong addressing mode)
**Mitigation:**
- Clear parsing order: check IndexedIndirect first (`,X)` inside parens)
- Then check IndirectIndexed (`,Y` outside parens)
- Comprehensive tests for both variants

### Risk 2: Whitespace Edge Cases
**Risk:** Various whitespace patterns not handled
**Likelihood:** Medium
**Impact:** Medium (user frustration)
**Mitigation:**
- Aggressive trimming at function start
- Use `std::string::find()` instead of exact matching
- Test with whitespace variants explicitly

### Risk 3: Backward Compatibility
**Risk:** New parsing breaks existing syntax
**Likelihood:** Low (additive changes only)
**Impact:** High (regression)
**Mitigation:**
- Run all 155 existing tests after each group
- Add explicit tests for existing syntax (Immediate, ZeroPage, Absolute)
- Check test_assembler.cpp still passes

### Risk 4: Label vs Address Ambiguity
**Risk:** `table,X` could be label or malformed address
**Likelihood:** Low (no `$` prefix means label)
**Impact:** Low (undefined label will error later)
**Mitigation:**
- Treat non-`$` prefixed operands as labels
- Let symbol resolution handle undefined labels
- Document behavior in comments

---

## Architecture Decisions

### Decision 1: Keep Parsing in Assembler.cpp
**Options:**
- A) Keep `DetermineAddressingMode()` in assembler.cpp (current)
- B) Extract to new `Parser` class

**Choice:** A (Keep in assembler.cpp)

**Rationale:**
- Simpler for now (one file change)
- Function is still manageable size (~80-100 lines)
- Can refactor to Parser class later if needed
- Reduces scope of this task

### Decision 2: Error Handling Approach
**Options:**
- A) Return `AddressingMode::Invalid` enum value
- B) Throw exception
- C) Return empty vector (current behavior)

**Choice:** C for now, consider A in future

**Rationale:**
- Matches existing error handling pattern
- Higher-level code checks for empty encoding
- Less disruptive to existing error flow
- Can improve in separate refactoring task

### Decision 3: Helper Functions
**Options:**
- A) Add helper functions immediately
- B) Wait until refactor phase (if needed)

**Choice:** B (Wait)

**Rationale:**
- Keep initial implementation simple
- YAGNI principle (You Ain't Gonna Need It)
- Refactor phase will reveal if helpers needed
- Can extract during REFACTOR phase of TDD

---

## Success Metrics

### Quantitative Metrics
- [ ] All 155 existing tests still pass
- [ ] At least 30 new unit tests added
- [ ] At least 10 integration tests added
- [ ] At least 10 error tests added
- [ ] Test coverage ≥85% overall, 95%+ for parsing
- [ ] Zero compiler warnings
- [ ] Build time not increased (should be ~same)

### Qualitative Metrics
- [ ] Code is readable and self-documenting
- [ ] Parsing logic is clear and maintainable
- [ ] Error messages are helpful to users
- [ ] Tester approves (score ≥80/100)
- [ ] Reviewer approves (no blocking/major issues)

---

## Timeline Estimate

**Total Estimated Effort:** 4-6 hours

**Breakdown:**
- **Group 1 (Accumulator):** 30 minutes
  - Tests: 15 min
  - Implementation: 10 min
  - Validation: 5 min

- **Group 2 (Indexed):** 1 hour
  - Tests: 30 min
  - Implementation: 20 min
  - Validation: 10 min

- **Group 3 (Simple Indirect):** 30 minutes
  - Tests: 15 min
  - Implementation: 10 min
  - Validation: 5 min

- **Group 4 (Complex Indirect):** 1.5 hours
  - Tests: 45 min
  - Implementation: 30 min
  - Validation: 15 min

- **Group 5 (Integration):** 1 hour
  - Tests: 50 min
  - Validation: 10 min

- **Group 6 (Error Handling):** 30 minutes (incremental throughout)
  - Tests: 20 min
  - Implementation: 10 min

- **Review and Polish:** 30 minutes
  - Code cleanup
  - Documentation
  - Final testing

---

## Rollback Plan

**If implementation blocked:**

1. **Per-group rollback:**
   - Each group has separate commit
   - Can revert to previous group if issues found
   - Example: `git revert <commit-hash>`

2. **Complete rollback:**
   - All changes in single branch
   - Can abandon branch if fundamental issues
   - No changes merged to main until complete

**Rollback triggers:**
- Existing tests start failing and can't be fixed quickly
- Parsing logic becomes unmaintainable (>200 lines in one function)
- Fundamental design flaw discovered

---

## Post-Implementation Tasks

### Immediate (This Task)
- [ ] Tester validation (`/ai-pack test`)
- [ ] Reviewer validation (`/ai-pack review`)
- [ ] Update acceptance document
- [ ] Commit final status
- [ ] Push to GitHub

### Future (Separate Tasks)
- **Parser Refactoring** (Optional)
  - Extract `Parser` class
  - Unit test parsing directly
  - Priority: Low

- **Binary Output** (Next phase?)
  - Generate `.prg` or `.bin` files
  - Write assembled bytes to disk
  - Priority: Medium

- **Other CPU Architectures** (Future)
  - Z80, 68000, ARM, etc.
  - Separate CPU plugins
  - Priority: Low (6502 is primary goal)

---

## Sign-Off

**Plan Author:** Orchestrator Role (ai-pack framework)
**Date:** 2026-01-22

**Plan Approval:**
- [x] Orchestrator (plan is ready for execution)
- [ ] Engineer (will implement)
- [ ] Tester (will validate)
- [ ] Reviewer (will review)

---

**Plan Version:** 1.0
**Last Updated:** 2026-01-22
