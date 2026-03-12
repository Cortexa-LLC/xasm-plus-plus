# Implementation Plan

**Task:** Merlin LUP Directive
**Estimated:** 2-3 hours

## Approach

Complete LUP directive implementation with nested loop support. LUP captures code block and expands it N times.

## Implementation Steps

### Phase 1: Review (30 min)
1. Check existing LUP implementation
2. Identify gaps (nested loops)
3. Review test coverage

### Phase 2: Implementation (1 hour)
1. Add/fix nested loop tracking
2. Ensure --^ terminator handling
3. Local label uniquification
4. Register directive if needed

### Phase 3: Testing (30-60 min)
1. Test basic LUP
2. Test nested LUP
3. Test local labels
4. Edge cases

## Files

- LUP handler (may exist)
- Nested state tracking
- Tests
- Total: 2 files
