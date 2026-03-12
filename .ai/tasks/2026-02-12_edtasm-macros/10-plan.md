# Implementation Plan

**Task:** EDTASM-M80++ Macro System
**Estimated:** 8-12 hours

## Approach

Adapt FLEX ASM macro processor for EDTASM-M80++ syntax. Implement macro definition/expansion, parameter substitution, local labels, and iteration directives.

## Implementation Steps

### Phase 1: Macro Processor Core (3-4 hours)
1. Review FLEX macro processor
2. Adapt for EDTASM syntax (& and % operators)
3. Implement macro storage/lookup
4. Implement parameter substitution
5. Implement expansion engine

### Phase 2: Directives (2-3 hours)
1. MACRO/ENDM - definition/end
2. LOCAL - local label generator
3. EXITM - early exit
4. REPT - repetition
5. IRP - parameter iteration
6. IRPC - character iteration

### Phase 3: Integration (2-3 hours)
1. Register all directives
2. Wire to macro processor
3. Handle nested macros
4. Test all functionality

### Phase 4: Testing (1-2 hours)
1. Tests for all 7 directives
2. Nested macro tests
3. Parameter substitution tests
4. Integration tests

## Files

- Macro processor (~300 lines)
- 7 handlers (~30 lines each)
- Tests (~20 tests)
- Total: 5-6 files

## Reference

- FLEX macro processor in flex_syntax.cpp
