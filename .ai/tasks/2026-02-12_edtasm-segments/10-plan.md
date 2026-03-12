# Implementation Plan

**Task:** EDTASM-M80++ Segment Directives
**Estimated:** 4-6 hours

## Approach

Implement 3 segment directives (ASEG/CSEG/DSEG) to support relocatable code. Integrate with existing segment_manager infrastructure.

## Implementation Steps

### Phase 1: Analysis (1 hour)
1. Review segment_manager.h API
2. Understand segment types
3. Review address resolution

### Phase 2: Implementation (2-3 hours)
1. Implement ASEG handler
2. Implement CSEG handler
3. Implement DSEG handler
4. Integrate with segment manager
5. Register directives

### Phase 3: Testing (1-2 hours)
1. Test each segment type
2. Test segment switching
3. Test address resolution
4. Integration tests

## Files

- 3 handlers (~30 lines each)
- Segment integration
- Tests
- Total: 3-4 files
