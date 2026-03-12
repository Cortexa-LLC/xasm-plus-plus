# Implementation Plan

**Task ID:** xasm++-fz5t-20260305-investigate-a2osx-boot
**Beads ID:** xasm++-fz5t
**Created:** 2026-03-05
**Author:** Engineer (Qwen)
**Plan Version:** 1.0
**Model:** Qwen

---

## Approach Summary

This task requires a systematic investigation of why A2osX fails to boot when built with xasm++. The approach follows a diagnostic-first methodology:

**Phase 1:** Build and compare outputs - Assemble A2osX with xasm++, generate disk images, and compare against known-good builds at the binary level. Focus on relocation tables, config file format, and disk image structure.

**Phase 2:** Identify and fix root cause - Based on findings from Phase 1, implement targeted fixes in the assembler. Likely areas: symbol resolution, relocation table generation, or config file writing.

**Phase 3:** Validate end-to-end - Build complete A2osX disk image, boot in emulator, verify byte-for-byte match with reference.

**Key Technical Decisions:**
- Compare binaries at sector level, not just file level (disk images have headers)
- Use hexdump and diff for binary comparison
- Test in Virtual ][ before considering real hardware
- Maintain backward compatibility with all assembled projects

**Patterns to Use:**
- **Factory Pattern** - For creating different output formatters
- **Strategy Pattern** - For different config file formats
- **Diff-based debugging** - Compare xasm++ output vs reference

---

## Critical Files Identified

### Files to Read (for context)
```
- src/core/assembler.cpp - Main assembler engine
- src/core/relocation.cpp - Relocation table handling
- src/output/binary_writer.cpp - Binary output writer
- src/output/config_writer.cpp - Config file writer
- src/output/disk_writer.cpp - Disk image writer
- src/core/symbol_table.cpp - Symbol resolution
- tests/e2e/a2osx/ - End-to-end A2osX tests
- ~/Projects/Vintage/Apple/A2osX/Makefile - Build system
- ~/Projects/Vintage/Apple/A2osX/README.md - Build instructions
```

### Files to Modify (likely)
```
1. src/core/relocation.cpp
   - Current: Relocation table generation
   - Changes: Fix symbol offset calculation

2. src/output/config_writer.cpp
   - Current: Config file format
   - Changes: Fix header format or field ordering

3. src/output/disk_writer.cpp
   - Current: Disk image creation
   - Changes: Fix sector alignment or padding
```

---

## Step-by-Step Implementation Plan

### Phase 1: Binary Output Analysis
```
Step 1.1: Build A2osX with xasm++
  - Action: cd ~/Projects/Vintage/Apple/A2osX && make ASM=xasm XASM=xasm++ disk525
  - Files: Build output in ~/Projects/Vintage/Apple/A2osX/build/
  - Verification: Check for assembler errors

Step 1.2: Obtain reference build
  - Action: Build with known-good assembler or obtain reference binaries
  - Files: Reference .nib disk images
  - Verification: Reference boots successfully

Step 1.3: Compare disk image structures
  - Action: hexdump -C both disk images, compare headers
  - Files: Generated vs reference .nib files
  - Verification: Identify header differences

Step 1.4: Compare relocation tables
  - Action: Extract and compare relocation data from both builds
  - Files: Relocation sections in binaries
  - Verification: Identify offset or format differences

Step 1.5: Compare config files
  - Action: Extract and diff config file from both disk images
  - Files: Kernel config file in disk images
  - Verification: Identify format or content differences
```

### Phase 2: Fix Identified Issues
```
Step 2.1: Implement fix for relocation table
  - Action: Fix src/core/relocation.cpp if offset calculation incorrect
  - Files: src/core/relocation.cpp
  - Tests: Add unit test for relocation generation
  - Verification: Relocation table matches reference

Step 2.2: Implement fix for config file format
  - Action: Fix src/output/config_writer.cpp if header/format incorrect
  - Files: src/output/config_writer.cpp
  - Tests: Add unit test for config file generation
  - Verification: Config file matches reference format

Step 2.3: Implement fix for disk image structure
  - Action: Fix src/output/disk_writer.cpp if sector alignment incorrect
  - Files: src/output/disk_writer.cpp
  - Tests: Add unit test for disk image generation
  - Verification: Disk image structure matches reference
```

### Phase 3: End-to-End Validation
```
Step 3.1: Build complete A2osX disk image
  - Action: make ASM=xasm XASM=xasm++ disk525
  - Files: build/PrinceOfPersia_SideA.nib, build/PrinceOfPersia_SideB.nib
  - Verification: Build completes without errors

Step 3.2: Boot in emulator
  - Action: Launch Virtual ][ with generated disk image
  - Files: Generated .nib disk images
  - Verification: System boots to command line

Step 3.3: Verify byte-for-byte match
  - Action: diff against reference build
  - Files: Generated vs reference .nib files
  - Verification: No differences or acceptable differences documented

Step 3.4: Functional validation
  - Action: Test game functionality if applicable
  - Files: Running game in emulator
  - Verification: Game boots and is playable
```

---

## Testing Strategy

### Unit Tests
```
□ Relocation table generation - Test cases:
  - Forward reference resolution
  - Backward reference resolution
  - Cross-segment references
  - Error cases (unresolved symbols)

□ Config file format - Test cases:
  - Header format
  - Field ordering
  - Data encoding
  - Edge cases (empty config)

□ Disk image generation - Test cases:
  - Sector alignment
  - Padding
  - Header format
  - Multi-sector images
```

### Integration Tests
```
□ A2osX build integration
  - Setup: A2osX source tree
  - Execute: make ASM=xasm XASM=xasm++ disk525
  - Verify: Disk images generated without errors

□ Binary compatibility
  - Setup: Reference build and xasm++ build
  - Execute: Binary comparison
  - Verify: Byte-for-byte match
```

### Acceptance Tests
```
□ Full boot cycle
  - Given: A2osX disk image built with xasm++
  - When: Boot in Virtual ][
  - Then: System reaches command line prompt

□ Config file reading
  - Given: A2osX disk image with kernel config
  - When: Boot system
  - Then: Config file read successfully (no "Using Default" message)
```

### Coverage Target
```
- Overall: 99%+ (existing xasm++ tests)
- New code: 100%
- Critical paths: 100%
```

---

## Dependencies and Prerequisites

### Technical Dependencies
```
□ Apple II emulator (Virtual ][ or similar)
□ Reference A2osX binaries for comparison
□ xasm++ build system (cmake)
□ Unix tools (hexdump, diff, cmp)
```

### Task Dependencies
```
□ xasm++ must build successfully
□ All existing xasm++ tests must pass
□ A2osX source must be available
```

### Knowledge Dependencies
```
□ Understanding of Apple II disk image format (NIB)
□ Understanding of 6502 relocation tables
□ Understanding of A2osX boot process
```

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1: Disk image format complexity**
- **Probability:** Medium
- **Impact:** High
- **Mitigation:** Study NIB format specification, compare sector by sector
- **Contingency:** Use existing NIB parsing tools if available

**Risk 2: Relocation table corruption**
- **Probability:** Medium
- **Impact:** High
- **Mitigation:** Add detailed logging, compare against reference at each step
- **Contingency:** Revert to working assembler version if fix not found

**Risk 3: Endianness issues**
- **Probability:** Low
- **Impact:** High
- **Mitigation:** Verify byte order in all multi-byte values
- **Contingency:** Add byte-swapping logic if needed

### Integration Risks

**Risk 1: Breaking other assembled projects**
- **Mitigation:** Run full test suite, verify Prince of Persia still works
- **Contingency:** Feature flag or configuration option

---

## Rollback Plan

### If Implementation Fails

**Rollback Steps:**
```
1. Revert changes to src/core/relocation.cpp
2. Revert changes to src/output/config_writer.cpp
3. Revert changes to src/output/disk_writer.cpp
4. Rebuild xasm++
5. Verify original behavior restored
```

**Rollback Verification:**
```
✓ All xasm++ tests pass
✓ Prince of Persia builds correctly
✓ Other assembled projects still work
```

### Git Rollback
```
# If changes committed
git revert <commit-hash>

# If not committed
git restore .

# Verify
cmake --build build && ctest --output-on-failure
```

---

## Performance Considerations

### Expected Performance Impact
```
- Build time: Minimal impact (fixes are algorithmic, not I/O)
- Memory usage: No significant change
- Output size: Should match reference exactly
```

### Performance Targets
```
- Build time: Within 10% of reference build
- Memory usage: Same as baseline
- Output size: Byte-identical to reference
```

---

## Security Considerations

### Security Checklist
```
□ Input validation maintained
□ No new security vulnerabilities introduced
□ Buffer overflow protections intact
□ Memory safety preserved
```

---

## Alternative Approaches Considered

### Alternative 1: Modify A2osX to work with xasm++ output
**Pros:**
- Less invasive change
- Faster implementation

**Cons:**
- Violates "out of scope" constraint
- Makes xasm++ dependent on A2osX quirks

**Why Not Chosen:**
A2osX source changes are explicitly out of scope. xasm++ should produce standard-compliant output.

### Alternative 2: Use wrapper script to post-process xasm++ output
**Pros:**
- Quick fix
- No core changes

**Cons:**
- Hacky solution
- Hard to maintain
- Doesn't address root cause

**Why Not Chosen:**
Should fix the root cause in the assembler, not post-process output.

---

## Timeline Estimate

**Note:** Estimates are for planning only, not deadlines.

```
Phase 1: 4-6 hours
- Step 1.1: 30 min
- Step 1.2: 30 min
- Step 1.3: 1 hour
- Step 1.4: 1.5 hours
- Step 1.5: 1.5 hours

Phase 2: 2-4 hours
- Step 2.1: 1 hour
- Step 2.2: 1 hour
- Step 2.3: 1 hour

Phase 3: 1-2 hours
- Step 3.1: 30 min
- Step 3.2: 30 min
- Step 3.3: 30 min
- Step 3.4: 30 min

Testing: 1-2 hours
Documentation: 30 min

Total: 9-16 hours
```

---

## Success Metrics

### Completion Criteria
```
✓ Phase 1: Binary differences identified and documented
✓ Phase 2: Root cause fixed in source code
✓ Phase 3: A2osX boots successfully from xasm++ build
✓ All xasm++ tests passing
✓ Byte-for-byte match with reference (or documented acceptable differences)
```

### Quality Metrics
```
- Test coverage: 99%+
- Performance: No regression
- Code quality: Passes all linting
- Security: No new vulnerabilities
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [x] Author: Engineer (Qwen) [2026-03-05]
- [x] User/Stakeholder: Bryan Woodruff [2026-03-05]

**Revision History:**
```
v1.0 - 2026-03-05 - Initial plan
```

---

## Notes and Considerations

**Model Configuration:** This task is configured to use Qwen model. Add any model-specific instructions in work log.

**Emulator Access:** Virtual ][ or similar emulator must be available for Phase 3 validation.

**Reference Builds:** Need to obtain or build reference A2osX binaries for comparison.

**Related Projects:** A2osX at ~/Projects/Vintage/Apple/A2osX should be added as related project for preflight context.

---

**Plan Version:** 1.0
**Last Updated:** 2026-03-05