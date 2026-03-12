# Implementation Plan: Missing Directive Abbreviations

**Task:** Implement .ED, .FI, .EL abbreviation support  
**Effort:** ~3 hours  
**Priority:** P2-Low (optional for A2osX compatibility)

---

## Implementation Strategy

### TDD Approach (Test-Driven Development)

For each directive:
1. **RED:** Write failing test
2. **GREEN:** Implement minimal code to pass
3. **REFACTOR:** Clean up and optimize

---

## Task 1: Implement .ED (End Dummy)

**Effort:** 2-3 hours  
**Priority:** P2 (23 uses in A2osX)

### TDD Steps

#### RED Phase: Write Failing Tests

**Test 1: Valid .ED after .DUMMY**
```cpp
TEST(ScmasmDirectives, EdAfterDummySucceeds) {
  std::string source = 
    "        .DUMMY\n"
    "        .OR $00\n"
    "ZP.Ptr  .BS 2\n"
    "        .ED\n"
    "        .OR $8000\n";
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));
}
```

**Test 2: .ED without .DUMMY errors**
```cpp
TEST(ScmasmDirectives, EdWithoutDummyErrors) {
  std::string source = 
    "        .OR $8000\n"
    "        .ED\n";  // Error: not in dummy section
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_THROW(parser.Parse(source, section, symbols), std::runtime_error);
}
```

**Test 3: Multiple .ED in different dummy sections**
```cpp
TEST(ScmasmDirectives, MultipleEdInDifferentSections) {
  std::string source = 
    "        .DUMMY\n"
    "        .OR $00\n"
    "ZP1     .BS 2\n"
    "        .ED\n"
    "        .DUMMY\n"
    "        .OR $20\n"
    "DS1     .BS 4\n"
    "        .ED\n";
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));
}
```

#### GREEN Phase: Implement .ED

**Step 1:** Add constant
```cpp
// include/xasm++/directives/scmasm_directive_constants.h
namespace directives {
  constexpr const char *ED = ".ED";  // End dummy section
}
```

**Step 2:** Add dummy section tracking to parser
```cpp
// include/xasm++/syntax/scmasm_syntax.h
class ScmasmSyntaxParser {
private:
  bool in_dummy_section_;  // Track if currently in dummy section
  
public:
  bool InDummySection() const { return in_dummy_section_; }
  // Called by .DUMMY handler:
  void StartDummySection() { in_dummy_section_ = true; }
  // Called by .ED handler:
  void EndDummySection() { in_dummy_section_ = false; }
};
```

**Step 3:** Update .DUMMY handler
```cpp
// src/syntax/scmasm_directive_handlers.cpp
void HandleDummy(...) {
  auto *parser = static_cast<ScmasmSyntaxParser *>(ctx.parser_state);
  parser->StartDummySection();
  // ... existing dummy logic ...
}
```

**Step 4:** Create .ED handler
```cpp
// src/syntax/scmasm_directive_handlers.cpp
void HandleEd(const std::string &label, const std::string &operand,
              DirectiveContext &ctx) {
  auto *parser = static_cast<ScmasmSyntaxParser *>(ctx.parser_state);
  
  if (!parser->InDummySection()) {
    throw std::runtime_error(".ED without matching .DUMMY");
  }
  
  parser->EndDummySection();
  
  // No-op for now - dummy section just ends
  // Future: Could validate no more dummy operations after this
}
```

**Step 5:** Register directive
```cpp
// src/syntax/scmasm_syntax.cpp
void ScmasmSyntaxParser::InitializeDirectiveRegistry() {
  // ... existing directives ...
  
  directive_registry_[ED] = scmasm::HandleEd;
}
```

#### REFACTOR Phase
- Add comments explaining dummy section state machine
- Ensure dummy section auto-ends on `.OR` outside dummy mode
- Update documentation

---

## Task 2: Implement .FI (Finish)

**Effort:** 15 minutes  
**Priority:** P2 (1 use in A2osX)

### TDD Steps

#### RED Phase: Write Failing Tests

**Test 1: .FI ends conditional block**
```cpp
TEST(ScmasmDirectives, FiEndsConditional) {
  std::string source = 
    "        .DO 1\n"
    "        LDA #$01\n"
    "        .FI\n";
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));
}
```

**Test 2: .DO/.EL/.FI all abbreviations**
```cpp
TEST(ScmasmDirectives, AllAbbreviations) {
  std::string source = 
    "        .DO 1\n"
    "        LDA #$80\n"
    "        .EL\n"
    "        LDA #$40\n"
    "        .FI\n";
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));
}
```

#### GREEN Phase: Implement .FI

**Step 1:** Add constant
```cpp
// include/xasm++/directives/scmasm_directive_constants.h
namespace directives {
  constexpr const char *FI = ".FI";  // Alias to .FIN
}
```

**Step 2:** Update ParseLine to recognize .FI
```cpp
// src/syntax/scmasm_syntax.cpp
void ScmasmSyntaxParser::ParseLine(...) {
  // ...
  using namespace scmasm::directives;
  
  if (opcode_upper == DO) {
    HandleDo(operand, section, symbols, source, line_idx);
  } else if (opcode_upper == LU) {
    HandleLu(operand, section, symbols, source, line_idx);
  } else if (opcode_upper == ELSE || opcode_upper == EL ||
             opcode_upper == FIN || opcode_upper == FI ||
             opcode_upper == ENDU) {
    throw std::runtime_error("Mismatched " + opcode_upper);
  }
  // ...
}
```

**Step 3:** Update HandleDo to recognize .FI when scanning
```cpp
// src/syntax/scmasm_syntax.cpp
void ScmasmSyntaxParser::HandleDo(...) {
  // Find matching .ELSE or .FIN/.FI
  for (size_t i = line_idx + 1; i < source.size(); ++i) {
    // ... get directive ...
    
    using namespace scmasm::directives;
    if (directive == DO) {
      nesting++;
    } else if ((directive == ELSE || directive == EL) && nesting == 1) {
      else_line = i;
    } else if (directive == FIN || directive == FI) {
      nesting--;
      if (nesting == 0) {
        fin_line = i;
        break;
      }
    }
  }
  
  if (fin_line == std::string::npos) {
    throw std::runtime_error(".DO without matching .FIN or .FI");
  }
  // ...
}
```

#### REFACTOR Phase
- Ensure error messages mention both `.FIN` and `.FI`
- Update comments

---

## Task 3: Implement .EL (Else)

**Effort:** 15 minutes  
**Priority:** P2 (1 use in A2osX)

### TDD Steps

#### RED Phase: Write Failing Tests

**Test 1: .EL in conditional**
```cpp
TEST(ScmasmDirectives, ElInConditional) {
  std::string source = 
    "        .DO 0\n"
    "        LDA #$01\n"
    "        .EL\n"
    "        LDA #$02\n"
    "        .FIN\n";
  
  Section section;
  ConcreteSymbolTable symbols;
  ScmasmSyntaxParser parser;
  
  EXPECT_NO_THROW(parser.Parse(source, section, symbols));
  
  // Verify second LDA was assembled (condition false, .EL block executed)
}
```

#### GREEN Phase: Implement .EL

**Step 1:** Add constant
```cpp
// include/xasm++/directives/scmasm_directive_constants.h
namespace directives {
  constexpr const char *EL = ".EL";  // Alias to .ELSE
}
```

**Step 2:** Already done in .FI implementation (Step 2 and 3 above)
- `ParseLine()` already updated to recognize `.EL`
- `HandleDo()` already updated to scan for `.EL`

#### REFACTOR Phase
- Update comments to document abbreviations

---

## Integration Testing

### Test with Real A2osX Code

**Test file:** `tests/integration/a2osx/abbreviations.S.txt`

```assembly
*--------------------------------------
* Test S-C Macro Assembler Abbreviations
*--------------------------------------
                .DUMMY
                .OR $00
ZP.Ptr1         .BS 2
ZP.Ptr2         .BS 2
                .ED             ; End dummy

                .DUMMY
                .OR $20
DS.Buffer       .BS 256
                .ED             ; End dummy

                .OR $8000

* Test conditional abbreviations
                .DO DRVR
SCR.SIZE        .EQ 80
                .EL             ; .ELSE abbreviation
SCR.SIZE        .EQ 40
                .FI             ; .FIN abbreviation

* Test full forms still work
                .DO 1
MSG             .AS "Test"
                .ELSE
MSG             .AS "Fail"
                .FIN

                RTS
```

**Expected output:**
- Zero-page symbols defined correctly
- Conditional assembly works with abbreviations
- No errors

---

## Acceptance Criteria

### .ED (End Dummy)
- [ ] `.DUMMY` ... `.ED` works (no error)
- [ ] `.ED` without `.DUMMY` throws error
- [ ] Multiple `.ED` in different sections works
- [ ] Dummy section state tracked correctly
- [ ] Symbol table entries for dummy variables correct

### .FI (Finish)
- [ ] `.DO` ... `.FI` works
- [ ] `.DO` ... `.EL` ... `.FI` works
- [ ] Nested conditionals with `.FI` work
- [ ] Error message mentions `.FI` as valid

### .EL (Else)
- [ ] `.DO` ... `.EL` ... `.FIN` works
- [ ] `.DO` ... `.EL` ... `.FI` works (both abbreviations)
- [ ] Conditional logic correct (else block executes when condition false)
- [ ] Error message mentions `.EL` as valid

### Integration
- [ ] A2osX test file with all abbreviations assembles correctly
- [ ] Binary output matches expected (if comparative test available)
- [ ] No regressions in existing directive tests

---

## Files to Modify

1. ✅ `include/xasm++/directives/scmasm_directive_constants.h` - Add constants
2. ✅ `include/xasm++/syntax/scmasm_syntax.h` - Add dummy section tracking
3. ✅ `src/syntax/scmasm_directive_handlers.cpp` - Implement HandleEd
4. ✅ `src/syntax/scmasm_syntax.cpp` - Update ParseLine, HandleDo, InitializeDirectiveRegistry
5. ✅ `tests/unit/test_scmasm_directives.cpp` - Add unit tests
6. ✅ `tests/integration/a2osx/abbreviations.S.txt` - Add integration test

---

## Risk Mitigation

### Risk 1: Dummy section state gets out of sync

**Mitigation:**
- Explicit state machine (start/end methods)
- Validation in .ED handler
- Unit tests for edge cases

### Risk 2: .FI/.EL break existing .FIN/.ELSE

**Mitigation:**
- Update all scanning logic to check both forms
- Don't remove existing checks, add new ones
- Comprehensive tests for both forms

### Risk 3: Control flow nesting gets confused

**Mitigation:**
- Same nesting logic applies to abbreviations
- Test nested conditionals with mixed forms (.DO/.EL/.FIN, .DO/.ELSE/.FI)

---

## Estimated Timeline

| Task | Effort | Estimated Time |
|------|--------|----------------|
| .ED implementation | Medium | 2-3 hours |
| .FI implementation | Low | 15 minutes |
| .EL implementation | Low | 15 minutes |
| Integration testing | Low | 30 minutes |
| Documentation | Low | 15 minutes |
| **Total** | | **3-4 hours** |

---

## Success Metrics

1. ✅ All unit tests pass (100%)
2. ✅ Integration test with A2osX snippet passes
3. ✅ No regressions in existing tests
4. ✅ Code review approved
5. ✅ Documentation updated

---

## Optional: Future Enhancements

### Better Dummy Section Tracking
- Track multiple nested dummy sections (if SCMASM supports this)
- Provide better error messages (which .DUMMY is unclosed)

### Enhanced Error Messages
- "Did you mean .FIN instead of .FI?" (if .FI used incorrectly)
- "Unclosed .DUMMY at line X" (if .ED missing)

### Documentation
- Add abbreviations to directive reference
- Update SCMASM compatibility guide
