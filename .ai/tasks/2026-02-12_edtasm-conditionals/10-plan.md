# Implementation Plan

**Task:** EDTASM-M80++ Conditional Directives
**Estimated:** 6-8 hours

## Approach

Create conditional evaluation engine following Merlin IF/ELSE/ENDIF pattern. Implement 13 conditional directives that control code inclusion based on symbol state, expression evaluation, assembly pass, and text comparison.

## Implementation Steps

### Phase 1: Conditional Engine (2-3 hours)
1. Create conditional state tracker (stack for nested conditionals)
2. Implement symbol lookup (IFDEF/IFNDEF)
3. Implement expression evaluator (IFEQ/IFNE/IFLT/IFGT/IFLE/IFGE)
4. Implement pass detection (IF1/IF2)
5. Implement text comparison (IFB/IFNB/IFIDN/IFDIF)

### Phase 2: Directive Handlers (2-3 hours)
1. Implement 13 directive handlers
2. Wire to conditional engine
3. Register with DirectiveRegistry
4. Handle ELSE clause
5. Handle ENDIF

### Phase 3: Testing (2 hours)
1. Write tests for each directive
2. Test nested conditionals
3. Test edge cases
4. Integration testing

## Files

- Conditional engine (~200 lines)
- 13 handlers (~20 lines each)
- Tests (~15 tests)
- Total: 3-4 files

## Reference

- Merlin conditional implementation in merlin_syntax.cpp
- Conditional assembler pattern
