# Const Correctness Audit - Implementation Plan

## Task ID: xasm++-0h8g

### Implementation Approach

#### Phase 1: Review Previous Work
1. Check 2026-01-27_const-correctness-N1 findings
2. Identify what was already verified
3. Determine scope of new audit

#### Phase 2: Identify Candidates
1. Find all header files: `find include/ src/ -name "*.h" -o -name "*.hpp"`
2. Grep for public methods: look for `public:` sections
3. Identify methods that:
   - Return values without modifying state
   - Query state (getters)
   - Perform calculations without side effects

#### Phase 3: Test-Driven Verification (MANDATORY TDD)
1. RED: Write tests expecting const methods
2. GREEN: Add const qualifiers to make tests pass
3. REFACTOR: Clean up if needed

#### Phase 4: Verification
1. Build with zero warnings
2. Run full test suite
3. Verify no regressions

### Files to Review
- `include/` directory (public interfaces)
- `src/` headers (internal interfaces)
- Focus on classes not covered in previous task

### Testing Strategy
- Use existing `test_const_correctness.cpp` as reference
- Add new test cases for any newly identified classes
- Verify const objects can call const methods

### Success Criteria
- Zero warnings in build
- 100% tests passing
- All appropriate methods const
