# Implementation Plan

**Task:** 6809 Indexed Addressing Modes
**Estimated:** 4-6 hours

## Approach

Implement all 11 indexed addressing variants with post-byte encoding per 6809 specification.

## Implementation Steps

### Phase 1: Post-byte Engine (2 hours)
1. Study 6809 post-byte format
2. Implement post-byte generator
3. Handle 5/8/16-bit offsets
4. Handle register selection
5. Handle indirect flag

### Phase 2: Addressing Modes (2 hours)
1. ,X ,Y ,U ,S indexed
2. ,PCR program counter relative
3. Offset calculation
4. Auto-increment/decrement
5. Indirect modes

### Phase 3: Integration (1 hour)
1. Wire to instruction encoding
2. Update all instructions with indexed
3. Test encoding correctness

### Phase 4: Testing (1 hour)
1. Test all variants
2. Verify post-bytes match spec
3. Test offset ranges
4. Integration tests

## Files

- cpu_6809.cpp (indexed implementation)
- Tests
- Total: 2 files

## Reference

- 6809-levanthal.txt (post-byte format)
- verification-6809-opcodes.md
