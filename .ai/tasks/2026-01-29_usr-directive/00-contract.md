# Task Contract

**Task ID:** 2026-01-29_usr-directive
**Beads Task:** xasm++-cv2
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement USR directive for Prince of Persia compatibility - final feature for 100% PoP compatibility.

### Background and Context

Prince of Persia source files use the USR directive:
```assembly
USR expression    ; Call user-defined assembly-time routine
```

The USR directive in Merlin Pro allows calling user-defined routines during assembly. In practice, for PoP files, this is used minimally and typically as a placeholder or for advanced macro functionality.

This is the **final feature** blocking 100% PoP compatibility (29/29 files).

### Current State

- USR directive not implemented ❌
- 5 PoP files blocked (single USR occurrence)
- 24/29 PoP files assembling (83%)

### Desired State

- USR directive implemented ✓
- USR accepts expressions as arguments
- 29/29 PoP files assembling (100%) ✓
- **100% Prince of Persia compatibility achieved** 🎯

---

## Success Criteria

```
✓ USR directive parses correctly
✓ USR with expression arguments works
✓ All existing tests passing (574/575)
✓ New USR tests added
✓ 100% PoP compatibility (29/29 files)
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ USR expression - parses and evaluates expression
□ USR without arguments - handles gracefully
□ USR as no-op - doesn't generate bytes
□ USR doesn't affect program counter
□ Error handling for invalid expressions
```

### Quality Requirements
```
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
```

---

## Technical Approach

### USR Directive Behavior

**Merlin Pro USR Directive:**
- Syntax: `USR expression`
- Purpose: Call user-defined assembly-time routine
- Effect: Evaluates expression, no bytes generated
- Common use: Advanced macro functionality, conditional assembly

**Implementation Strategy:**

For xasm++ compatibility, USR can be implemented as:

**Option 1: Simple No-Op (Recommended for PoP)**
```cpp
void MerlinSyntaxParser::handle_USR(const std::string& expr) {
    // Evaluate expression for validation
    try {
        int64_t result = EvaluateExpression(expr);
        // Expression valid, no action needed
        // USR doesn't generate bytes or affect PC
    } catch (const std::exception& e) {
        throw std::runtime_error("USR: Invalid expression");
    }
}
```

**Option 2: Hook System (Future Enhancement)**
```cpp
void MerlinSyntaxParser::handle_USR(const std::string& expr) {
    int64_t routine_id = EvaluateExpression(expr);

    // Call registered user routine (if any)
    if (user_routines_.count(routine_id)) {
        user_routines_[routine_id](this);
    }
    // Otherwise no-op
}
```

For PoP compatibility, **Option 1** is sufficient.

### Implementation Location

**File:** `src/syntax/merlin_syntax.cpp`

**Add handler:**
```cpp
void MerlinSyntaxParser::HandleUsr(const std::string& operand) {
    // Parse and validate expression
    if (!operand.empty()) {
        try {
            ParseExpression(operand, current_section);
        } catch (const std::exception& e) {
            ReportError("Invalid USR expression: " + std::string(e.what()));
            return;
        }
    }

    // USR directive doesn't generate any bytes
    // No atoms added, no PC change
}
```

**Add to directive table:**
```cpp
if (directive == "USR") {
    HandleUsr(operand);
    return true;
}
```

### Test Cases

1. **Simple USR:**
   ```assembly
   USR $1234
   ```

2. **USR with expression:**
   ```assembly
   USR label+10
   ```

3. **USR in sequence:**
   ```assembly
           .org $1000
   start:  LDA #$42
           USR $0001
           STA $00
   ```

4. **Error handling:**
   ```assembly
   USR undefined_symbol  ; Should error
   ```

---

## Prince of Persia Context

**Files Affected:** 5 files with single USR occurrence

**Example Usage from PoP:**
The USR directive appears minimally in PoP source code, typically as:
```assembly
USR expression
```

**Impact:**
- Unblocks final 5 PoP files
- Achieves 100% PoP compatibility milestone
- 29/29 files assembling successfully

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| USR needs complex functionality | Low | Low | Start with no-op, enhance if needed |
| Breaking existing code | Very Low | Low | USR is new, no existing code uses it |
| Expression evaluation issues | Low | Low | Reuse existing expression evaluator |

---

## References

**Prince of Persia Analysis:**
- `docs/investigations/pop-compatibility-analysis.md`
- 5 files affected by USR directive

**Related Code:**
- `src/syntax/merlin_syntax.cpp` - Directive handlers
- `src/expression.cpp` - Expression evaluation
- `tests/unit/test_merlin_syntax.cpp` - Directive tests

**Merlin Pro Documentation:**
- USR directive reference
- Assembly-time routine calling

---

## Implementation Phases

**Phase 1: Basic Implementation** (30 minutes)
- Add HandleUsr() method
- Register in directive table
- Basic expression validation

**Phase 2: Testing** (30 minutes)
- Add unit tests for USR
- Test with PoP files
- Verify 100% PoP compatibility

**Phase 3: Documentation** (15 minutes)
- Update work log
- Document USR behavior
- Mark milestone achieved

**Total Estimated Effort:** 1-2 hours (as estimated)

---

## Success Milestone

**🎯 100% Prince of Persia Compatibility**

Upon completion of this task:
- ✅ All 29 PoP source files assemble successfully
- ✅ All blocking features implemented
- ✅ Major compatibility milestone achieved
- ✅ Demonstrates production-ready Merlin syntax support

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
