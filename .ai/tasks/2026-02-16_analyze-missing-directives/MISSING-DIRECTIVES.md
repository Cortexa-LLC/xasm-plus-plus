# Missing Directives Summary

**Quick Reference for Implementation**

---

## Missing Directives (3 total)

| Directive | Uses | Full Form | Priority | Effort | Implementation |
|-----------|------|-----------|----------|--------|----------------|
| `.ED`     | 23   | (none - explicit end marker) | P2-Low | 2-3 hrs | Validation + no-op |
| `.FI`     | 1    | `.FIN` (already implemented) | P2-Low | 15 min | Alias to `.FIN` |
| `.EL`     | 1    | `.ELSE` (already implemented) | P2-Low | 15 min | Alias to `.ELSE` |

**Total Implementation Effort:** ~3 hours

---

## Directive Details

### .ED (End Dummy)

**Current Behavior:** xasm++ auto-detects dummy section end  
**Proposed Behavior:** Explicit validation that we're in a dummy section, then no-op  
**Location:** `scmasm_directive_constants.h`, `scmasm_directive_handlers.cpp`

**Implementation:**
```cpp
// In scmasm_directive_constants.h
constexpr const char *ED = ".ED";

// In scmasm_directive_handlers.cpp
void HandleEd(const std::string &label, const std::string &operand,
              DirectiveContext &ctx) {
  auto *parser = static_cast<ScmasmSyntaxParser *>(ctx.parser_state);
  
  if (!parser->InDummySection()) {
    throw std::runtime_error(".ED without matching .DUMMY");
  }
  
  // No-op - dummy section continues until next .OR or section change
  // This is just validation
}
```

**Test Cases:**
1. `.DUMMY` ... `.ED` → Should work (no-op)
2. `.ED` without `.DUMMY` → Should error
3. Nested dummy sections with `.ED`

---

### .FI (Finish/End Conditional)

**Current Behavior:** Must use `.FIN`  
**Proposed Behavior:** Alias `.FI` to `.FIN`  
**Location:** `scmasm_directive_constants.h`, directive registry

**Implementation:**
```cpp
// In scmasm_directive_constants.h
constexpr const char *FI = ".FI";  // Alias to .FIN

// In InitializeDirectiveRegistry()
// Control flow directives (.DO, .ELSE, .FI/.FIN, .LU, .ENDU)
// .FI is handled same as .FIN in ParseLine (not via registry)
```

**Special Handling Required:** `.FI` is a control flow directive, handled in `ParseLine()` not registry (same as `.FIN`, `.ELSE`, `.DO`)

**Test Cases:**
1. `.DO` ... `.FI` → Should work
2. `.DO` ... `.EL` ... `.FI` → Should work
3. Nested conditionals with `.FI`

---

### .EL (Else)

**Current Behavior:** Must use `.ELSE`  
**Proposed Behavior:** Alias `.EL` to `.ELSE`  
**Location:** `scmasm_directive_constants.h`, ParseLine logic

**Implementation:**
```cpp
// In scmasm_directive_constants.h
constexpr const char *EL = ".EL";  // Alias to .ELSE

// In ParseLine() control flow handling
if (opcode_upper == ELSE || opcode_upper == EL || opcode_upper == FIN || 
    opcode_upper == FI || opcode_upper == ENDU) {
  // These are handled by their opening directives (.DO, .LU)
  throw std::runtime_error("Mismatched " + opcode_upper);
}
```

**Special Handling Required:** `.EL` is a control flow directive, handled in `HandleDo()` not registry

**Test Cases:**
1. `.DO` ... `.EL` ... `.FIN` → Should work
2. `.DO` ... `.EL` ... `.FI` → Should work (both abbreviations)
3. Mismatched `.EL` without `.DO` → Should error

---

## Implementation Checklist

### Phase 1: Add Constants
- [ ] Add `.ED` constant to `scmasm_directive_constants.h`
- [ ] Add `.FI` constant to `scmasm_directive_constants.h`
- [ ] Add `.EL` constant to `scmasm_directive_constants.h`

### Phase 2: Implement Handlers

#### .ED (End Dummy)
- [ ] Create `HandleEd()` in `scmasm_directive_handlers.cpp`
- [ ] Add to directive registry in `InitializeDirectiveRegistry()`
- [ ] Add test: `.DUMMY` ... `.ED` (valid)
- [ ] Add test: `.ED` without `.DUMMY` (error)

#### .FI (Finish)
- [ ] Update `ParseLine()` to recognize `.FI` as `.FIN` alias
- [ ] Update `HandleDo()` to recognize `.FI` when scanning for block end
- [ ] Add test: `.DO` ... `.FI` (valid)
- [ ] Add test: `.DO` ... `.EL` ... `.FI` (valid)

#### .EL (Else)
- [ ] Update `ParseLine()` to recognize `.EL` as `.ELSE` alias
- [ ] Update `HandleDo()` to recognize `.EL` when scanning for else clause
- [ ] Add test: `.DO` ... `.EL` ... `.FIN` (valid)
- [ ] Add test: `.EL` without `.DO` (error)

### Phase 3: Testing
- [ ] Unit tests for each directive
- [ ] Integration test with A2osX snippet using all abbreviations
- [ ] Binary compatibility test vs S-C Macro Assembler

---

## A2osX Usage Statistics

**Directive usage across A2osX codebase:**

| Directive Type | Count | Percentage |
|----------------|-------|------------|
| Critical (implemented) | 8985 | **99.7%** |
| Abbreviations (missing) | 25 | **0.3%** |

**Breakdown of missing directives:**
- `.ED`: 23 uses (0.26% of all directives)
- `.FI`: 1 use (0.01% of all directives)
- `.EL`: 1 use (0.01% of all directives)

**Conclusion:** Missing directives represent **less than 1%** of total directive usage in A2osX.

---

## Risk Assessment

### Can A2osX be assembled without abbreviations?

**YES** ✅ with minor source modifications:

**Option A: Source Modifications**
1. Replace `.ED` → (remove or comment out)
2. Replace `.FI` → `.FIN` (1 occurrence)
3. Replace `.EL` → `.ELSE` (1 occurrence)

**Option B: Implement Abbreviations**
- Estimated effort: 3 hours
- Complete compatibility: No source modifications needed
- Cleaner user experience

**Recommendation:** **Option B** (implement abbreviations) for production quality.

---

## Files to Modify

1. **include/xasm++/directives/scmasm_directive_constants.h**
   - Add `.ED`, `.FI`, `.EL` constants

2. **src/syntax/scmasm_directive_handlers.cpp**
   - Implement `HandleEd()`
   - Update documentation

3. **src/syntax/scmasm_syntax.cpp**
   - Update `InitializeDirectiveRegistry()` for `.ED`
   - Update `ParseLine()` to recognize `.FI` and `.EL`
   - Update `HandleDo()` to handle `.EL` and `.FI`

4. **include/xasm++/syntax/scmasm_syntax.h**
   - Add `InDummySection()` method (if not exists)
   - Add dummy section tracking state

5. **tests/unit/test_scmasm_directives.cpp** (or similar)
   - Add unit tests for `.ED`, `.FI`, `.EL`

6. **tests/integration/a2osx/**
   - Add integration test with abbreviations

---

## Reference: Control Flow Directive Handling

**Current Implementation Pattern:**

Control flow directives (`.DO`, `.ELSE`, `.FIN`, `.LU`, `.ENDU`) are **NOT** in the directive registry because they require:
- Line skipping (don't execute skipped blocks)
- Nested scoping (`.DO` within `.DO`)
- Forward scanning (find matching `.FIN` before processing)

**Location:** `ScmasmSyntaxParser::ParseLine()` and `HandleDo()`, `HandleLu()`

**To add `.FI` and `.EL` abbreviations:**
1. Update `ParseLine()` mismatched directive check
2. Update `HandleDo()` scanning logic to recognize both forms
3. No registry changes needed (control flow directives bypass registry)

---

## Next Steps

1. **Decide:** Implement abbreviations or modify A2osX sources?
2. **If implementing:** Follow checklist above (TDD approach)
3. **If skipping:** Document workaround for users

**Recommended:** Implement abbreviations for production quality assembler.
