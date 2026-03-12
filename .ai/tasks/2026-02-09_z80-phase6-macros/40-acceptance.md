# Acceptance Criteria - Z80 Phase 6: Macro System

**Task ID:** 2026-02-09_z80-phase6-macros  
**Completed:** 2026-02-09  
**Engineer:** AI Agent

---

## Requirements Met

### ✅ MACRO/ENDM Implementation
- [x] Define macros with MACRO directive
- [x] End macro definitions with ENDM
- [x] Support parameterized macros
- [x] Invoke macros by name with arguments
- [x] Prevent nested macro definitions

**Tests:**
- MacroSimpleDefinition
- MacroSimpleInvocation
- MacroWithParameters

---

### ✅ Parameter Substitution
- [x] &PARAM syntax (concatenation/label building)
- [x] %PARAM syntax (expression evaluation)
- [x] Bare PARAM syntax (case-insensitive word replacement)
- [x] Word-boundary aware substitution

**Tests:**
- MacroWithAmpersandSubstitution
- MacroWithPercentSubstitution
- MacroMultipleInvocations

---

### ✅ LOCAL Label Support
- [x] LOCAL directive for macro-local labels
- [x] Unique label generation per invocation
- [x] Multiple local labels in single macro
- [x] Local labels work in nested macro invocations

**Tests:**
- MacroLocalLabel
- MacroMultipleLocalLabels
- NestedMacroWithLocalLabels

---

### ✅ EXITM Implementation
- [x] Early exit from macro expansion
- [x] Works within nested contexts
- [x] Properly terminates macro body processing

**Tests:**
- MacroExitm

---

### ✅ REPT Implementation
- [x] Repeat block N times
- [x] % counter substitution (0-based)
- [x] Expression evaluation for count
- [x] Proper handling of % in binary numbers (101%B)

**Tests:**
- ReptSimple
- ReptWithCounter

---

### ✅ IRP Implementation
- [x] Iterate over comma-separated value list
- [x] Parameter substitution (&PARAM, %PARAM, bare)
- [x] Angle bracket syntax: `<val1, val2, ...>`
- [x] Expression evaluation in values

**Tests:**
- IrpSimple
- IrpWithExpressions

---

### ✅ IRPC Implementation
- [x] Iterate over string characters
- [x] Parameter substitution for each character
- [x] Quoted string support
- [x] Bare string support

**Tests:**
- IrpcSimple
- IrpcWithQuotedString

---

### ✅ Nested Macro Support
- [x] Macros can invoke other macros
- [x] Proper recursion depth tracking (100 level limit)
- [x] Local labels remain unique in nested contexts
- [x] Parameter substitution works in nested macros

**Tests:**
- NestedMacroDefinitions
- NestedMacroInvocations
- NestedMacroWithLocalLabels
- MacroRecursionDepthLimit

---

## Code Quality

### ✅ Clean Implementation
- [x] No compiler warnings
- [x] Follows project coding standards
- [x] Proper error handling
- [x] Refactored duplicate code

### ✅ Type Hints & Documentation
- [x] Doxygen-style API documentation
- [x] Inline comments for complex logic
- [x] Clear function parameter descriptions
- [x] Usage notes for each directive

### ✅ Tests (TDD)
- [x] Tests written before implementation
- [x] All 19 macro-related tests passing
- [x] 110/112 total project tests passing
- [x] 2 failures are pre-existing EQU directive issues

---

## Files Modified

1. **include/xasm++/syntax/z80_universal_syntax.h**
   - Added macro system member variables
   - Added macro handler method declarations
   - Comprehensive API documentation

2. **src/syntax/z80_universal/z80_universal_syntax.cpp**
   - Implemented all macro directives
   - Parameter substitution engine
   - Macro expansion logic
   - Local label uniquification
   - Refactored duplicate code

3. **tests/unit/test_z80_universal_syntax.cpp**
   - 13 macro tests
   - 2 REPT tests
   - 4 IRP/IRPC tests

---

## Test Results

```
[==========] Running 19 tests from 1 test suite.
[       OK ] Z80UniversalSyntaxTest.MacroSimpleDefinition
[       OK ] Z80UniversalSyntaxTest.MacroSimpleInvocation
[       OK ] Z80UniversalSyntaxTest.MacroWithParameters
[       OK ] Z80UniversalSyntaxTest.MacroWithAmpersandSubstitution
[       OK ] Z80UniversalSyntaxTest.MacroWithPercentSubstitution
[       OK ] Z80UniversalSyntaxTest.MacroMultipleInvocations
[       OK ] Z80UniversalSyntaxTest.MacroLocalLabel
[       OK ] Z80UniversalSyntaxTest.MacroMultipleLocalLabels
[       OK ] Z80UniversalSyntaxTest.MacroExitm
[       OK ] Z80UniversalSyntaxTest.NestedMacroDefinitions
[       OK ] Z80UniversalSyntaxTest.NestedMacroInvocations
[       OK ] Z80UniversalSyntaxTest.NestedMacroWithLocalLabels
[       OK ] Z80UniversalSyntaxTest.MacroRecursionDepthLimit
[       OK ] Z80UniversalSyntaxTest.ReptSimple
[       OK ] Z80UniversalSyntaxTest.ReptWithCounter
[       OK ] Z80UniversalSyntaxTest.IrpSimple
[       OK ] Z80UniversalSyntaxTest.IrpWithExpressions
[       OK ] Z80UniversalSyntaxTest.IrpcSimple
[       OK ] Z80UniversalSyntaxTest.IrpcWithQuotedString
[  PASSED  ] 19 tests.
```

---

## Acceptance

✅ **All acceptance criteria met**

**Features Implemented:**
- MACRO/ENDM with parameter substitution
- LOCAL labels with unique generation
- EXITM for early exit
- REPT with % counter
- IRP for value list iteration
- IRPC for string character iteration
- Nested macro support with depth limiting

**Quality Standards:**
- Zero warnings
- Clean, refactored code
- Comprehensive documentation
- Full TDD coverage

**Ready for:** Integration into xasm++ Z80 assembler

---

**Signed off:** 2026-02-09
