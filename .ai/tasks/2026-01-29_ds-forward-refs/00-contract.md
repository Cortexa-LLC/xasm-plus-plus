# Task Contract

**Task ID:** 2026-01-29_ds-forward-refs
**Beads Task:** xasm++-960
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement forward reference support in DS (Define Storage) directive for Prince of Persia compatibility.

### Background and Context

Prince of Persia source files use DS directives with forward references:
```assembly
start:
        ; Code here
        DS $1000-*    ; Allocate space up to $1000
                      ; * = current address (forward ref)
```

The `*` symbol represents the current program counter, which may not be fully resolved on the first assembly pass when forward references exist earlier in the code.

### Current State

- DS with constant size works: `DS 100` ✓
- DS with resolved expressions works: `DS $1000-$0800` ✓
- DS with forward references fails: `DS $1000-*` ❌
- 2 PoP files blocked by this issue

### Desired State

- DS defers evaluation until addresses stabilize
- Multi-pass assembly converges with correct DS sizes
- `DS $1000-*` correctly allocates remaining space
- 2 PoP files unblocked

---

## Success Criteria

```
✓ DS with forward references works (DS $1000-*)
✓ Multi-pass assembly converges correctly
✓ Negative DS sizes detected and rejected
✓ All existing tests passing (574/575)
✓ New DS forward reference tests added
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ DS $1000-* works when * is forward-referenced
□ DS label-* works with forward-referenced labels
□ DS evaluates in later passes after addresses resolve
□ Negative sizes rejected with clear error
□ Assembly converges in 2-3 passes maximum
□ Error handling preserved
```

### Quality Requirements
```
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
□ Multi-pass convergence tested
```

---

## Technical Approach

### Problem Analysis

**DS Directive Purpose:**
- Allocates space without generating bytes
- Advances program counter by specified amount
- Size must be known to calculate subsequent addresses

**Forward Reference Issue:**
```assembly
        .org $0800
start:  LDA #$42
        STA $00
        DS $1000-*     ; Error: * not yet known on pass 1
                       ; * depends on instructions above
                       ; But instructions depend on DS size!
```

**Multi-Pass Solution:**
1. **Pass 1:** Assume DS size = 0 (or previous value)
2. **Pass 2:** Re-evaluate DS with resolved *
3. **Pass 3+:** Continue until addresses stabilize

### Implementation Strategy

**Current DS Implementation:**
Located in `src/syntax/merlin_syntax.cpp` - `handle_DS()` function

**Approach 1: Expression Deferral (Recommended)**
```cpp
void MerlinSyntaxParser::handle_DS(const std::string& size_expr) {
    try {
        // Try to evaluate expression
        int64_t size = EvaluateExpression(size_expr);

        if (size < 0) {
            throw std::runtime_error("DS size cannot be negative");
        }

        // Use the size
        current_section->current_offset += size;
    } catch (const ForwardReferenceException& e) {
        // Expression contains forward references
        // Defer to next pass - use previous size if available
        if (HasPreviousPassSize()) {
            current_section->current_offset += GetPreviousPassSize();
        }
        // Mark for re-evaluation
        MarkForReEvaluation(size_expr);
    }
}
```

**Approach 2: Atom-Based Deferral**
- Create DSAtom that holds the expression
- Re-evaluate expression in each pass
- Size changes trigger another pass

**Convergence Detection:**
- Track address changes between passes
- Stop when all addresses stable
- Maximum 10 passes safety limit

### Test Cases

1. **Simple forward reference:**
   ```assembly
   start:  LDA #$42
           DS $1000-*
   ```

2. **Label forward reference:**
   ```assembly
   start:  LDA #$42
           DS end-start
   end:
   ```

3. **Negative size detection:**
   ```assembly
           .org $2000
           DS $1000-*    ; Error: negative size
   ```

4. **Convergence test:**
   ```assembly
   loop:   DS $1000-*
           LDA #$42
           DS $2000-*
   ```

---

## Architecture Considerations

### Multi-Pass Assembly

xasm++ already supports multi-pass assembly:
- `src/assembler.cpp` - `Assemble()` method
- Iterates until symbols converge
- Maximum passes configuration

**Required Changes:**
1. DS must participate in convergence detection
2. DS size changes must trigger additional passes
3. Expression evaluation must handle forward references gracefully

### Expression Evaluator

Check `src/expression.cpp` for:
- Forward reference handling
- `*` (current address) symbol support
- Error propagation

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Non-convergence (infinite loop) | Medium | High | Max pass limit, detect cycles |
| Performance impact | Low | Low | Most programs converge in 2-3 passes |
| Breaking existing DS | Low | Medium | Comprehensive backward compatibility tests |
| Complex expressions | Medium | Medium | Incremental implementation, start simple |

---

## References

**Prince of Persia Analysis:**
- `docs/investigations/pop-compatibility-analysis.md`
- 2 files affected by DS forward references

**Related Code:**
- `src/syntax/merlin_syntax.cpp` - handle_DS()
- `src/assembler.cpp` - Multi-pass logic
- `src/expression.cpp` - Expression evaluation
- `tests/unit/test_merlin_syntax.cpp` - DS tests

**Multi-Pass Assembly:**
- `docs/architecture/01-core-abstractions.md` - Symbol resolution
- Existing multi-pass tests in test suite

---

## Implementation Phases

**Phase 1: Investigation** (1 hour)
- Review current DS implementation
- Review multi-pass assembly logic
- Identify integration points

**Phase 2: Simple Forward Reference** (2 hours)
- Implement DS with `*` forward reference
- Add basic tests
- Verify convergence

**Phase 3: Label Forward References** (1 hour)
- Support `DS label-*` patterns
- Add label tests

**Phase 4: Edge Cases** (1 hour)
- Negative size detection
- Convergence failure detection
- Error message improvements

**Total Estimated Effort:** 4-6 hours (as estimated)

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
