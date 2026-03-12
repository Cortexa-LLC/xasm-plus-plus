# Task Contract

**Task ID:** 2026-02-01_65816-extended
**Created:** 2026-02-01
**Requestor:** Bryan Woodruff (Orchestrator)
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-hhy

---

## Task Description

Implement missing Merlin macro and 65816 features to enable UNPACK.S assembly.

### Background and Context

UNPACK.S is the final blocker for 100% Prince of Persia source compatibility (currently at 25/26 = 96%). The file uses advanced Merlin macro features and 65816-specific directives that aren't fully supported yet.

**Current Progress:** 25/26 (96%)
**Target:** 26/26 (100%)

### Current State

UNPACK.S fails to assemble with errors for:
1. **Label-based MAC definitions** - `MacroName MAC params` format not recognized
2. **<<< end marker** - Alternative to EOM not supported
3. **MX directive** - 65816 register size control not implemented

**Example from UNPACK.S:**
```assembly
stlx mac bank;addr     ; Define macro with label
 hex 9f                ; Emit $9F (STA Long opcode)
 da ]2                 ; Emit address parameter
 db ]1                 ; Emit bank parameter
 <<<                   ; End macro definition

 mx %11                ; Set 65816 register sizes (8-bit A, 8-bit X/Y)
 stlx $E1;$9D00        ; Use macro
```

**Current Implementation:**
- ✅ PMC/EOM macro style works (programmer's macro)
- ✅ MAC invocation works
- ✅ Parameter substitution (]1, ]2) works
- ❌ Label-based MAC definition fails
- ❌ <<< end marker not recognized
- ❌ MX directive not implemented

### Desired State

All three features implemented:
1. **Label-based MAC** - `MacroName MAC params` starts macro definition
2. **<<< marker** - Recognized as EOM alternative
3. **MX directive** - Sets 65816 accumulator/index register widths
4. **UNPACK.S** - Assembles successfully
5. **Test suite** - Shows 26/26 (100%)

---

## Success Criteria

```
✓ Label-based MAC definitions work
✓ <<< end marker terminates macros
✓ MX directive parses and validates
✓ UNPACK.S assembles without errors
✓ All existing tests still pass
✓ Prince of Persia test shows 26/26 (100%)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ Label-based MAC: MacroName MAC params recognized as macro definition start
□ Parameter names captured from MAC line
□ <<< recognized as EOM alternative (same behavior as EOM)
□ MX directive implemented with validation:
  - mx %00 = 16-bit A, 16-bit X/Y
  - mx %01 = 16-bit A, 8-bit X/Y
  - mx %10 = 8-bit A, 16-bit X/Y
  - mx %11 = 8-bit A, 8-bit X/Y
  - Also accept decimal: mx 0, mx 1, mx 2, mx 3
□ Macro expansion uses parameters correctly
□ Error handling for malformed macros/MX
```

### Quality Requirements
```
□ All existing tests passing
□ New unit tests for:
  - Label-based MAC definition
  - <<< end marker
  - MX directive (all modes)
□ No regressions in binary compatibility
□ No new linting errors
□ Code follows existing patterns
```

### Non-Functional Requirements
```
□ Performance acceptable (negligible impact)
□ Error messages clear and helpful
□ Code maintainable and well-documented
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain backward compatibility
□ PMC/EOM style macros must still work
□ Cannot change Merlin syntax parser interface
□ MX is directive only (not CPU mode change - just tracks state)
```

### Dependencies
```
□ Requires MerlinSyntaxParser (src/syntax/merlin/merlin_syntax.cpp)
□ Macro infrastructure already exists (PMC/EOM)
□ Parameter substitution already works (]1, ]2, etc.)
```

### Out of Scope
```
✗ Actual 65816 instruction encoding changes based on MX
✗ 65816-specific instructions beyond MX directive
✗ Nested macros (already restricted)
✗ Macro libraries or advanced features
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 2 (merlin_syntax.cpp, merlin_syntax.h)
- Lines of code estimate: ~50 lines (small additions to existing code)
- New concepts: Label-based MAC parsing, <<< marker, MX directive
- Integration complexity: Low (extends existing macro system)
- Risk level: Low (isolated changes, existing infrastructure)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 2-3 files
- src/syntax/merlin/merlin_syntax.cpp (HandleMAC extension, HandleMX, HandleEOM update)
- include/xasm++/syntax/merlin_syntax.h (HandleMX declaration, MX state)
- tests/unit/test_merlin_syntax.cpp (new tests)

**Batch Size:** 2-3 files → ✅ IDEAL

### Token Budget Estimation

```
Files × Average Tokens Per File = Estimated Total
3 × 2,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status: <20K tokens → ✅ SAFE
```

### Work In Progress

**Planned WIP:** 1 agent (Engineer) → ✅ IDEAL

---

## Resources and References

### Relevant Files
```
- src/syntax/merlin/merlin_syntax.cpp - Add HandleMX(), update MAC parsing
- include/xasm++/syntax/merlin_syntax.h - Add HandleMX(), mx_state_ member
- tests/unit/test_merlin_syntax.cpp - Add tests

Test file:
- UNPACK.S - Real-world usage example
```

### Merlin Syntax Reference

**Label-based MAC (from 6502-SYNTAX-COMPARISON.md):**
```assembly
MACRONAME MAC param1;param2;...   ; Label is macro name
          <body using ]1, ]2>
          <<<                      ; End macro
```

**MX Directive (65816):**
```assembly
mx %00    ; 16-bit A, 16-bit X/Y (native mode)
mx %01    ; 16-bit A, 8-bit X/Y
mx %10    ; 8-bit A, 16-bit X/Y
mx %11    ; 8-bit A, 8-bit X/Y (emulation mode)
```

**Equivalent decimal:**
```assembly
mx 0      ; Same as %00
mx 1      ; Same as %01
mx 2      ; Same as %10
mx 3      ; Same as %11
```

---

## Implementation Approach

### Suggested Strategy

1. **Label-based MAC support:**
   - Detect when label is present and directive == "MAC"
   - Extract macro name from label
   - Extract parameters from operands (semicolon-separated)
   - Set macro definition mode
   - Continue capturing lines until <<< or EOM

2. **<<< end marker:**
   - In macro definition mode, detect "<<<" as trimmed line
   - Treat same as EOM (end definition, register macro)
   - Clear macro definition state

3. **MX directive:**
   - Add HandleMX(const std::string& operand)
   - Parse binary (%00-%11) or decimal (0-3) operand
   - Validate range
   - Store in mx_state_ member
   - (Note: actual 65816 encoding changes out of scope)

4. **Testing:**
   - Unit test label-based MAC definition
   - Unit test <<< terminator
   - Unit test MX directive with all modes
   - Integration test with UNPACK.S

---

## Risk Assessment

### Identified Risks

```
1. Breaking existing PMC/EOM macros
   - Probability: Low
   - Impact: High
   - Mitigation: Keep both paths working, comprehensive tests

2. Parameter parsing conflicts
   - Probability: Low
   - Impact: Medium
   - Mitigation: Careful delimiter handling (semicolon vs comma)

3. <<< appearing in non-macro context
   - Probability: Low
   - Impact: Low
   - Mitigation: Only recognize in macro definition mode
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (Orchestrator) (2026-02-01)
- [ ] Agent: Engineer [Date]

---

## Notes

This is the FINAL blocker for 100% Prince of Persia assembly success.

**Progress:**
- ✅ 24/29 baseline
- ✅ XC directive (27 commits ago)
- ✅ Arithmetic expressions (this session)
- ✅ 25/26 current (FRAMEDEF.S recovered)
- ⚠️ UNPACK.S - Last blocker (this task)
- 🎯 26/26 - 100% TARGET

**After this task:**
- Prince of Persia: 100% compatibility ✅
- Merlin syntax: Production-ready
- Real-world validation: Complete

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-01
