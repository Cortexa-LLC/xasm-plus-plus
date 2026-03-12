# Work Log: Fix edtasm_syntax to Match Merlin Directive Registry Pattern

**Task ID:** 2026-02-13_edtasm-pattern-fix
**Beads Task:** xasm++-lwa6
**Engineer:** Engineer-1
**Started:** 2026-02-14

---

## Session 1: 2026-02-14 - Initial Investigation

### Completed
- [x] Read task contract and plan
- [x] Reviewed current edtasm_syntax.h and edtasm_syntax.cpp implementation
- [x] Reviewed Merlin reference pattern (merlin_syntax.h, merlin_syntax.cpp)
- [x] Reviewed DirectiveContext structure (directive_registry.h)
- [x] Confirmed understanding of required changes

### Current State Analysis
**Current Implementation (Wrong):**
- Parameter order: `(operands, label, Section&, ConcreteSymbolTable&)` ❌
- No DirectiveContext - direct Section/SymbolTable access ❌
- Member methods with lambda wrappers ❌
- Method name: `RegisterDirectives()` ❌

**Target Pattern (Merlin):**
- Parameter order: `(label, operand, DirectiveContext&)` ✅
- Uses DirectiveContext for state access ✅
- Free functions in namespace (edtasm::) ✅
- Method name: `InitializeDirectiveRegistry()` ✅
- Direct assignment: `directive_registry_[ORG] = edtasm::HandleOrg;` ✅

### Next Steps
1. Create edtasm_directive_handlers.h with forward declarations
2. Update edtasm_syntax.h with correct signature and friend declarations
3. Create edtasm_directive_handlers.cpp with 9 free functions
4. Update edtasm_syntax.cpp InitializeDirectiveRegistry() method
5. Update ParseDirective to use DirectiveContext
6. Remove member method implementations
7. Build and test

---

## Implementation Plan

### Phase 1: Header Updates
- [x] Create include/xasm++/directives/edtasm_simple_directive_handlers.h
- [x] Update include/xasm++/syntax/edtasm_syntax.h
  - [x] Add forward declarations for edtasm namespace
  - [x] Update DirectiveHandler signature
  - [x] Add friend declarations
  - [x] Rename RegisterDirectives() → InitializeDirectiveRegistry()
  - [x] Remove member method declarations

### Phase 2: Implementation Files
- [x] Create src/syntax/edtasm_simple_directive_handlers.cpp with 9 functions
  - [x] HandleOrg
  - [x] HandleEnd
  - [x] HandleEqu
  - [x] HandleSet
  - [x] HandleFcb
  - [x] HandleFdb
  - [x] HandleFcc
  - [x] HandleRmb
  - [x] HandleSetdp

### Phase 3: Update Main Parser
- [x] Update src/syntax/edtasm_syntax.cpp
  - [x] Rename RegisterDirectives() → InitializeDirectiveRegistry()
  - [x] Update to direct assignment (no lambdas)
  - [x] Update ParseDirective to use DirectiveContext
  - [x] Remove member method implementations

### Phase 4: Build and Test
- [x] Build with zero warnings
- [x] Run all EdtasmSyntaxTest tests (23 tests)
- [x] Verify 100% test pass rate

---

## Session 2: 2026-02-14 - Implementation Complete

### Completed
- [x] Created `include/xasm++/directives/edtasm_simple_directive_handlers.h`
  - Forward declarations for all 9 directive handlers
  - Functions in `xasm::edtasm` namespace
  - Proper documentation matching Merlin pattern

- [x] Updated `include/xasm++/syntax/edtasm_syntax.h`
  - Added friend declarations for edtasm namespace handlers
  - Changed DirectiveHandler signature to `(label, operand, DirectiveContext&)`
  - Renamed `RegisterDirectives()` to `InitializeDirectiveRegistry()`
  - Removed all member method declarations

- [x] Created `src/syntax/edtasm_simple_directive_handlers.cpp`
  - Implemented all 9 free functions in `xasm::edtasm` namespace
  - HandleOrg, HandleEnd, HandleEqu, HandleSet, HandleFcb, HandleFdb, HandleFcc, HandleRmb, HandleSetdp
  - Used helper functions for parsing (ParseNumber, Trim, GetParser)
  - All implementations match original functionality

- [x] Updated `src/syntax/edtasm_syntax.cpp`
  - Renamed `RegisterDirectives()` to `InitializeDirectiveRegistry()`
  - Changed to direct assignment: `directive_registry_[ORG] = edtasm::HandleOrg;`
  - Updated ParseDirective to create DirectiveContext
  - Fixed parameter order to (label, operands, context)
  - Removed all member method implementations

- [x] Updated `src/CMakeLists.txt`
  - Added `syntax/edtasm_simple_directive_handlers.cpp` to build

### Test Results
✅ **All 23 EdtasmSyntax tests passing (100%)**
```
 1/23 Test #1020: EdtasmSyntaxTest.CommentWithAsterisk ..........   Passed
 2/23 Test #1021: EdtasmSyntaxTest.CommentWithSemicolon .........   Passed
 3/23 Test #1022: EdtasmSyntaxTest.BlankLines ...................   Passed
 4/23 Test #1023: EdtasmSyntaxTest.GlobalLabel ..................   Passed
 5/23 Test #1024: EdtasmSyntaxTest.LabelWithInstruction .........   Passed
 6/23 Test #1025: EdtasmSyntaxTest.OrgDirective .................   Passed
 7/23 Test #1026: EdtasmSyntaxTest.OrgDirectiveDecimal ..........   Passed
 8/23 Test #1027: EdtasmSyntaxTest.EndDirective .................   Passed
 9/23 Test #1028: EdtasmSyntaxTest.EndDirectiveWithEntryPoint ...   Passed
10/23 Test #1029: EdtasmSyntaxTest.EquDirective .................   Passed
11/23 Test #1030: EdtasmSyntaxTest.EquDirectiveHex ..............   Passed
12/23 Test #1031: EdtasmSyntaxTest.SetDirective .................   Passed
13/23 Test #1032: EdtasmSyntaxTest.SetDirectiveRedefine .........   Passed
14/23 Test #1033: EdtasmSyntaxTest.FcbDirectiveSingle ...........   Passed
15/23 Test #1034: EdtasmSyntaxTest.FcbDirectiveMultiple .........   Passed
16/23 Test #1035: EdtasmSyntaxTest.FcbDirectiveCharacter ........   Passed
17/23 Test #1036: EdtasmSyntaxTest.FdbDirectiveSingle ...........   Passed
18/23 Test #1037: EdtasmSyntaxTest.FdbDirectiveMultiple .........   Passed
19/23 Test #1038: EdtasmSyntaxTest.FccDirectiveSlashDelimiter ...   Passed
20/23 Test #1039: EdtasmSyntaxTest.FccDirectiveQuoteDelimiter ...   Passed
21/23 Test #1040: EdtasmSyntaxTest.RmbDirective .................   Passed
22/23 Test #1041: EdtasmSyntaxTest.SetdpDirective ...............   Passed
23/23 Test #1042: EdtasmSyntaxTest.CompleteProgram ..............   Passed
100% tests passed, 0 tests failed out of 23
```

### Build Results
✅ **Clean build with zero compiler warnings**
- Only benign linker warnings about duplicate libraries
- Full project builds successfully

### Implementation Details

**File Naming:**
- Used `edtasm_simple_directive_handlers.*` to distinguish from existing `edtasm_directive_handlers.*` (for EDTASM-M80++)
- This avoids confusion between simple EDTASM (CoCo) and extended EDTASM-M80++ (Z80)

**Pattern Compliance:**
- ✅ Correct parameter order: (label, operand, DirectiveContext&)
- ✅ Uses DirectiveContext for state access
- ✅ Free functions in edtasm namespace
- ✅ Direct assignment (no lambdas)
- ✅ Method name: InitializeDirectiveRegistry()
- ✅ Matches Merlin pattern exactly

---

## Blockers
None

---

## Notes
- Following Merlin pattern exactly as documented in contract
- This is pure refactoring - no functional changes
- All 23 existing tests continue passing
- Named files `edtasm_simple_*` to distinguish from EDTASM-M80++ handlers
