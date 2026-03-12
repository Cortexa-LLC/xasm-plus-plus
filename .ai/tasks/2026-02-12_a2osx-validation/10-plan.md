# Implementation Plan: A2oSX Build Validation Test

**Task ID:** 2026-02-12_a2osx-validation
**Beads Task:** xasm++-e6h9
**Created:** 2026-02-13
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

This validation task will confirm xasm++ readiness for production use with A2oSX. The approach has three phases:

1. **Discovery Phase**: Locate A2oSX source repository, understand build structure, identify representative sample files
2. **Build Validation Phase**: Attempt to build all 190 source files, document success/failure, analyze root causes
3. **Testing & Documentation Phase**: Create automated validation tests, write documentation for A2oSX developers

**Key Technical Decisions:**
- Use integration test framework (existing infrastructure)
- Sample files will be copied into xasm++ repository (no external dependencies)
- Focus on assembly success, not binary-exact matching
- Document all issues but don't block on bugs (create separate fix tasks)

**Patterns to Use:**
- Integration test pattern (like tests/integration/merlin/)
- Validation report format (markdown documentation)
- CI pipeline integration (CMakeLists.txt test registration)

---

## Critical Files Identified

### Files to Create
```
1. tests/integration/a2osx/CMakeLists.txt
   - Purpose: Test registration for A2oSX validation
   - Rationale: Integrate with existing CI pipeline

2. tests/integration/a2osx/sample_*.S
   - Purpose: Representative A2oSX source files (3-5 samples)
   - Rationale: No external dependencies, always available

3. tests/integration/a2osx/test_a2osx_build.py
   - Purpose: Automated build validation test
   - Rationale: Continuous validation in CI

4. docs/validation/a2osx/build-validation-report.md
   - Purpose: Validation results and statistics
   - Rationale: Document success rate and issues

5. docs/validation/a2osx/migration-guide.md
   - Purpose: Guide for A2oSX developers migrating from Merlin
   - Rationale: Facilitate adoption
```

### Files to Modify
```
1. tests/integration/CMakeLists.txt
   - Current: Registers existing integration tests
   - Changes: Add a2osx subdirectory

2. docs/README.md (or similar)
   - Current: Documentation index
   - Changes: Link to A2oSX validation documentation
```

### Files to Read (for context)
```
- tests/integration/merlin/ - Reference for integration test structure
- tests/e2e/apple2/prince_of_persia/ - Complex real-world test example
- .ai/tasks/2026-02-12_a2osx-directive-analysis/ - Directive analysis results
```

---

## Step-by-Step Implementation Plan

### Phase 1: Discovery

**Step 1.1: Locate A2oSX Source Repository**
  - Action: Search for A2oSX repository (GitHub: A2osX/A2osX or similar)
  - Verification: Can access source files

**Step 1.2: Analyze Build Structure**
  - Action: Review A2oSX build system, identify dependencies
  - Files: Makefile, build scripts
  - Verification: Understand build order and dependencies

**Step 1.3: Select Representative Samples**
  - Action: Choose 3-5 representative .S files (simple, medium, complex)
  - Criteria:
    - Simple: Basic directives, no macros
    - Medium: Some directives, basic macros
    - Complex: Many directives, nested macros, includes
  - Verification: Files represent full directive coverage

---

### Phase 2: Build Validation

**Step 2.1: Manual Build Attempt**
  - Action: Build selected samples manually with xasm++
  - Command: `xasm++ --syntax scmasm sample.S -o sample.bin`
  - Verification: Note successes and failures

**Step 2.2: Systematic Validation**
  - Action: If A2oSX accessible, attempt build of all 190 files
  - Approach: Script to iterate files, log results
  - Tracking: Success count, failure count, error categories
  - Verification: Complete pass through all files

**Step 2.3: Root Cause Analysis**
  - Action: For each failure, determine root cause
  - Categories:
    - Directive bug (fix in separate task)
    - Missing feature (document as limitation)
    - Invalid A2oSX code (document as known issue)
  - Verification: Each failure has documented explanation

---

### Phase 3: Testing & Documentation

**Step 3.1: Create Integration Test**
  - Action: Create tests/integration/a2osx/ directory structure
  - Files:
    - CMakeLists.txt (test registration)
    - sample_simple.S (basic test)
    - sample_medium.S (moderate complexity)
    - sample_complex.S (advanced features)
    - test_a2osx_build.py (test script)
  - Tests: Test should build all samples successfully
  - Verification: `ctest -R a2osx` passes

**Step 3.2: Write Build Validation Report**
  - Action: Create docs/validation/a2osx/build-validation-report.md
  - Content:
    - Executive summary (success rate, key findings)
    - Detailed results (file-by-file if available)
    - Root cause analysis
    - Known limitations
    - Recommended fixes (create tasks)
  - Verification: Report is complete and clear

**Step 3.3: Write Migration Guide**
  - Action: Create docs/validation/a2osx/migration-guide.md
  - Content:
    - Introduction (why xasm++?)
    - Command-line comparison (Merlin vs xasm++)
    - Build system integration
    - Known differences and workarounds
    - Support and issue reporting
  - Verification: Guide is actionable for A2oSX developers

**Step 3.4: Register CI Test**
  - Action: Update tests/integration/CMakeLists.txt
  - Changes: `add_subdirectory(a2osx)`
  - Verification: CI runs A2oSX validation test

**Step 3.5: Full Verification**
  - Action: Run full test suite
  - Commands:
    ```bash
    cmake --build build --clean-first
    cd build && ctest --output-on-failure
    ```
  - Verification: All 1564+ tests passing (including new A2oSX test)

---

## Testing Strategy

### Integration Tests
```
□ A2oSX Simple Sample - Test cases:
  - Basic directives (.DA, .DB, .AS, .HS)
  - Simple macros
  - Label definitions
  - Expected: Builds successfully, produces binary output

□ A2oSX Medium Sample - Test cases:
  - More directives (.PS, .INB, .LIST)
  - Nested macros
  - Includes (if supported)
  - Expected: Builds successfully

□ A2oSX Complex Sample - Test cases:
  - Advanced directives (.DUMMY, .OP, .TF)
  - Complex macro usage
  - Conditional assembly
  - Expected: Builds successfully or documents known limitation
```

### Verification Tests
```
□ Existing tests still pass - All 1564 tests
□ New A2oSX test passes - Integration test
□ CI pipeline green - All platforms
```

---

## Execution Strategy

**Sequential execution** (dependencies between phases):
1. Discovery (must understand A2oSX structure first)
2. Build Validation (must validate before testing)
3. Testing & Documentation (finalizes findings)

**Estimated effort:** 4-6 hours
- Phase 1: 1-2 hours (discovery, analysis)
- Phase 2: 2-3 hours (validation, root cause analysis)
- Phase 3: 1-2 hours (testing, documentation)

---

## Risk Mitigation

### Risk: A2oSX Source Not Accessible
**Mitigation:**
- Search GitHub: `A2osX/A2osX` or similar
- If not found, use samples from directive analysis
- Create representative files based on analysis results
- Document limitation in validation report

### Risk: Many Build Failures
**Mitigation:**
- Document each failure clearly
- Create fix tasks for bugs (don't block validation)
- Mark known limitations in migration guide
- Focus on samples that DO work for integration test

### Risk: Complex Build Dependencies
**Mitigation:**
- Start with standalone files (no includes)
- Document dependency requirements
- Provide build order guidance in migration guide

---

## Success Metrics

**Primary:**
- Integration test created and passing in CI
- Build validation report complete
- Migration guide complete

**Secondary:**
- >95% build success rate (if all 190 files tested)
- Zero regressions in existing tests
- Clear actionable fix tasks created for any bugs

---

## Deliverables Checklist

- [ ] A2oSX source repository located
- [ ] Representative samples selected (3-5 files)
- [ ] Build validation completed (manual or full 190 files)
- [ ] Integration test created in tests/integration/a2osx/
- [ ] Test registered in CMakeLists.txt
- [ ] Test passing in CI
- [ ] Build validation report written
- [ ] Migration guide written
- [ ] Documentation linked from main README
- [ ] All existing tests still passing (1564+)

---

**Plan approved by:** Orchestrator
**Ready for execution by:** Engineer
**Estimated effort:** 4-6 hours
