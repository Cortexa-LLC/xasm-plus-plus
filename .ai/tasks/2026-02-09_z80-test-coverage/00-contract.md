# Task Contract: Improve Z80 Test Coverage

**Task ID:** 2026-02-09_z80-test-coverage
**Beads Task:** xasm++-37qd
**Created:** 2026-02-09
**Priority:** P2

Audit Z80 test coverage, identify gaps, and add missing tests to reach 90%+ coverage target.

## Problem Statement

Current Z80 test coverage status is unknown:
- No coverage metrics available
- Unclear which instructions are tested
- Potential gaps in edge case coverage
- No systematic coverage tracking

This creates risks:
- Bugs may go undetected
- Refactoring is riskier without coverage safety net
- Code quality confidence is lower
- Edge cases may be untested

## Objective

Achieve 90%+ test coverage for Z80 CPU implementation with comprehensive test suite.

## Success Criteria

✓ Coverage baseline established and documented
✓ Coverage gaps identified and prioritized
✓ New tests added to fill critical gaps
✓ 90%+ line coverage achieved
✓ 95%+ coverage for critical paths
✓ Edge cases comprehensively tested
✓ Coverage tracking integrated into CI (optional)

## Acceptance Criteria

### Phase 1: Coverage Audit (Baseline)
- [ ] Enable code coverage in build system (gcov/lcov)
- [ ] Run existing test suite with coverage enabled
- [ ] Generate coverage report
- [ ] Document current coverage percentage
- [ ] Identify untested code paths
- [ ] Categorize gaps by priority (critical, important, nice-to-have)

### Phase 2: Gap Analysis
- [ ] List all Z80 instructions implemented
- [ ] Check which instructions have tests
- [ ] Identify missing instruction tests
- [ ] Document edge cases that need testing:
  - Boundary values (0x00, 0xFF, 0xFFFF)
  - Overflow conditions
  - Flag behaviors
  - Prefix combinations (CB, ED, DD, FD)
- [ ] Create prioritized test backlog

### Phase 3: Test Implementation (TDD)
- [ ] Write tests for critical gaps (P0)
- [ ] Write tests for important gaps (P1)
- [ ] Write tests for nice-to-have gaps (P2)
- [ ] All new tests passing
- [ ] Coverage metrics improving with each test batch

### Phase 4: Verification
- [ ] Final coverage report generated
- [ ] 90%+ line coverage achieved
- [ ] 95%+ branch coverage for critical code
- [ ] All tests passing (existing + new)
- [ ] Zero test flakiness
- [ ] Coverage documentation complete

### Phase 5: CI Integration (Optional)
- [ ] Add coverage target to CMakeLists.txt
- [ ] Fail build if coverage drops below threshold
- [ ] Generate coverage badge
- [ ] Document coverage process in README

## Test Categories to Cover

### 1. Basic Instruction Coverage
Every Z80 instruction should have at least one test:
- Load instructions (LD)
- Arithmetic (ADD, SUB, ADC, SBC, INC, DEC)
- Logical (AND, OR, XOR, CP)
- Rotate/Shift (RLCA, RLA, RRCA, RRA)
- Jump/Call/Return (JP, JR, CALL, RET)
- Stack operations (PUSH, POP)
- I/O operations (if implemented)

### 2. Prefix Instructions (CB, ED, DD, FD)
- CB prefix (bit operations): BIT, SET, RES, RLC, RRC, RL, RR, SLA, SRA, SRL
- ED prefix (extended): Block operations (LDIR, CPIR), NEG, IM, etc.
- DD prefix (IX operations)
- FD prefix (IY operations)

### 3. Edge Cases
- Minimum values (0x00)
- Maximum values (0xFF, 0xFFFF)
- Boundary crossings (0xFF → 0x00)
- Negative displacements
- Flag state changes

### 4. Error Conditions
- Invalid operands
- Out-of-range values
- Unsupported instructions
- Malformed encodings

## Coverage Metrics

**Target Coverage Levels:**
- **Overall line coverage:** ≥90%
- **Critical code (encoding logic):** ≥95%
- **Branch coverage:** ≥85%
- **Function coverage:** 100% (all public methods tested)

**Current Baseline:**
- To be measured in Phase 1

**Expected Improvement:**
- Add 50-100 new test cases
- Cover all untested instructions
- Cover all edge cases for critical instructions

## Tools and Setup

### Coverage Tools (Linux/macOS)
```bash
# Enable coverage in CMake
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="--coverage" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"

# Build and run tests
cmake --build build
cd build && ctest

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_html
open coverage_html/index.html
```

### Alternative: gcovr (simpler)
```bash
# After running tests with coverage enabled
gcovr --root . --html --html-details -o coverage.html
open coverage.html
```

## Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Coverage tool not available | High | Provide multiple tool options (lcov, gcovr) |
| Tests too slow | Medium | Parallelize test execution, use Catch2 sections |
| Coverage target too ambitious | Low | Start with 80%, incrementally reach 90% |
| Breaking existing tests | High | Run full suite after each new test batch |

## Files to Modify

**Existing Test File:**
- `tests/unit/test_cpu_z80.cpp` (add new test cases)

**Build System:**
- `CMakeLists.txt` (add coverage flags)
- `tests/unit/CMakeLists.txt` (coverage target)

**Documentation:**
- `docs/testing/coverage.md` (new file with coverage guide)

## Estimated Effort

- Coverage audit: 1 hour
- Gap analysis: 1 hour
- Test implementation: 4-6 hours (50-100 tests)
- Verification: 1 hour
- CI integration: 1 hour (optional)

**Total:** 8-10 hours

## Dependencies

- None (can run in parallel with other work)
- Recommended: Do after Z80 refactoring complete (xasm++-3syi)

## Stakeholders

- **Owner:** Orchestrator
- **Implementer:** Engineer (with Tester focus)
- **Reviewer:** Tester + Code quality review

**Approved:** Orchestrator 2026-02-09
