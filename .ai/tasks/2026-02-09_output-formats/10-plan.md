# Implementation Plan

**Task ID:** 2026-02-09_output-formats
**Created:** 2026-02-09
**Author:** Orchestrator
**Plan Version:** 1.0

---

## Approach Summary

Implement multiple output format writers for xasm++. The contract identified this as HIGH RISK (12 files, 36K tokens) and recommends decomposition into 3 sequential subtasks for safety.

**DECISION: Following contract recommendation to decompose into 3 subtasks:**
1. **Subtask 1:** Interface + Intel HEX (4-5 files, ~12K tokens) ✅ SAFE
2. **Subtask 2:** S-Record + Binary Formats (5 files, ~15K tokens) ✅ SAFE
3. **Subtask 3:** Listing + CLI Integration (3 files, ~9K tokens) ✅ SAFE

**Key Technical Decisions:**
- Use OutputWriter interface/base class pattern
- Format-specific implementations inherit from base
- Factory pattern for format selection (optional, may defer to CLI)
- TDD methodology for each format

**Patterns to Use:**
- Interface/base class for common output operations
- Format-specific writers (IntelHexWriter, SRecWriter, BinaryWriter, ListingWriter)
- Command-line integration via CommandLineOptions extension

---

## Decomposition Plan

### Why Decompose?

From contract Lean Flow Analysis:
- **Total:** 12 files, 36K tokens
- **Risk Level:** HIGH (40% failure probability)
- **Mitigation:** Decompose into 3 subtasks

### Subtask Breakdown

**Subtask 1: Interface + Intel HEX**
- **Files:** 4-5 (interface + Intel HEX writer + tests)
- **Token estimate:** ~12K (SAFE)
- **Dependencies:** None
- **Deliverable:** OutputWriter interface, Intel HEX format working

**Subtask 2: S-Record + Binary Formats**
- **Files:** 5 (S-Record + Binary writers + tests)
- **Token estimate:** ~15K (SAFE)
- **Dependencies:** Subtask 1 (needs OutputWriter interface)
- **Deliverable:** S-Record and platform-specific binary formats working

**Subtask 3: Listing + CLI Integration**
- **Files:** 3 (listing writer if needed + CLI integration + tests)
- **Token estimate:** ~9K (SAFE)
- **Dependencies:** Subtask 1 and 2 (needs all writers)
- **Deliverable:** Listing output + command-line flags + full integration

---

## Critical Files Identified

### Subtask 1 Files

**To Create:**
```
1. include/xasm++/output/output_writer.h
   - Purpose: Base interface for all output writers
   - Rationale: Common contract for all formats

2. src/output/output_writer.cpp
   - Purpose: Base implementation (if needed)
   - Rationale: Shared utilities, helpers

3. include/xasm++/output/intel_hex_writer.h
   - Purpose: Intel HEX format writer interface
   - Rationale: Standard ROM programmer format

4. src/output/intel_hex_writer.cpp
   - Purpose: Intel HEX implementation
   - Rationale: Data records, EOF, checksums

5. tests/unit/test_intel_hex_writer.cpp
   - Purpose: Intel HEX unit tests
   - Rationale: TDD validation
```

### Subtask 2 Files

**To Create:**
```
6. include/xasm++/output/srec_writer.h
   - Purpose: Motorola S-Record writer interface

7. src/output/srec_writer.cpp
   - Purpose: S-Record implementation (S19/S28/S37)

8. include/xasm++/output/binary_writer.h
   - Purpose: Binary format writer interface

9. src/output/binary_writer.cpp
   - Purpose: Raw + platform-specific binaries (Apple II, C64, TRS-80, CP/M, Game Boy)

10. tests/unit/test_output_formats.cpp
   - Purpose: S-Record and Binary format tests
```

### Subtask 3 Files

**To Modify/Create:**
```
11. include/xasm++/output/listing_writer.h (if not exists)
    - Purpose: Listing file generator

12. src/output/listing_writer.cpp (if not exists)
    - Purpose: Address + hex + source listing

13. src/cli_parser.cpp
    - Current: Command-line parsing
    - Changes: Add --output-hex, --output-srec, --output-bin, --output-list flags

14. tests/integration/test_output_integration.cpp
    - Purpose: End-to-end integration tests
```

**Note:** Files listing_output.h/cpp already exist (untracked), may simplify Subtask 3.

---

## Step-by-Step Implementation Plan

### Subtask 1: Interface + Intel HEX (First to implement)

```
Step 1.1: Design OutputWriter interface
  - Action: Create base class with Write() method
  - Files: output_writer.h
  - Tests: N/A (interface)
  - Verification: Compiles

Step 1.2: Write Intel HEX tests (TDD RED)
  - Action: Create comprehensive test cases
  - Files: test_intel_hex_writer.cpp
  - Tests: Data records, EOF, checksums, extended address
  - Verification: Tests compile and fail

Step 1.3: Implement Intel HEX writer (TDD GREEN)
  - Action: Implement IntelHexWriter class
  - Files: intel_hex_writer.h/cpp
  - Tests: All Intel HEX tests pass
  - Verification: Tests GREEN, format validated

Step 1.4: Refactor and verify
  - Action: Clean up code
  - Files: All Subtask 1 files
  - Tests: All tests pass, zero warnings
  - Verification: Ready for Subtask 2
```

### Subtask 2: S-Record + Binary Formats (After Subtask 1)

```
Step 2.1: Write S-Record tests (TDD RED)
  - Action: Create S-Record test cases
  - Files: test_output_formats.cpp
  - Tests: S0/S1/S2/S3/S9 records, checksums
  - Verification: Tests compile and fail

Step 2.2: Implement S-Record writer (TDD GREEN)
  - Action: Implement SRecWriter class
  - Files: srec_writer.h/cpp
  - Tests: S-Record tests pass
  - Verification: Tests GREEN

Step 2.3: Write Binary format tests (TDD RED)
  - Action: Create binary format test cases
  - Files: test_output_formats.cpp
  - Tests: Raw binary, Apple II, C64, TRS-80, CP/M, Game Boy
  - Verification: Tests compile and fail

Step 2.4: Implement Binary writer (TDD GREEN)
  - Action: Implement BinaryWriter class
  - Files: binary_writer.h/cpp
  - Tests: Binary tests pass
  - Verification: Tests GREEN, all formats validated

Step 2.5: Refactor and verify
  - Action: Clean up code
  - Files: All Subtask 2 files
  - Tests: All tests pass
  - Verification: Ready for Subtask 3
```

### Subtask 3: Listing + CLI Integration (After Subtasks 1 & 2)

```
Step 3.1: Check if listing_output.h/cpp exist
  - Action: Verify untracked files
  - Files: Check include/xasm++/output/, src/output/
  - Tests: N/A
  - Verification: Determine if implementation needed

Step 3.2: Implement/verify listing writer
  - Action: Create or validate ListingWriter
  - Files: listing_writer.h/cpp (if needed)
  - Tests: Listing format tests
  - Verification: Listing output working

Step 3.3: Add command-line integration
  - Action: Extend CommandLineOptions, update cli_parser.cpp
  - Files: cli_parser.cpp, command_line_options.h
  - Tests: CLI integration tests
  - Verification: Flags working

Step 3.4: End-to-end integration tests
  - Action: Create integration tests
  - Files: test_output_integration.cpp
  - Tests: Assemble + output in all formats
  - Verification: All formats working end-to-end

Step 3.5: Final refactor and verify
  - Action: Clean up all code
  - Files: All output format files
  - Tests: All tests pass
  - Verification: Complete implementation
```

---

## Testing Strategy

### Unit Tests (Per Subtask)

**Subtask 1 - Intel HEX:**
```
□ Data records (record type 00)
□ EOF record (record type 01)
□ Extended address (record types 04/05)
□ Checksum calculation
□ Bytes-per-line variations
```

**Subtask 2 - S-Record & Binary:**
```
□ S0 header record
□ S1/S2/S3 data records (auto-select by address range)
□ S9 EOF record
□ S-Record checksums
□ Raw binary output
□ Apple II DOS 3.3 format (address + length header)
□ C64 PRG format (2-byte load address)
□ TRS-80 CMD format
□ CP/M COM format
□ Game Boy ROM format
```

**Subtask 3 - Listing & CLI:**
```
□ Listing format (address, hex, source)
□ Symbol table generation
□ Command-line flag parsing
□ Multiple simultaneous outputs
□ Error handling (file write failures)
```

### Coverage Target
```
- Overall: 95%+
- Critical paths: 100%
- New code: 95%+
```

---

## Dependencies and Prerequisites

### Task Dependencies
```
□ Section/Atom framework - EXISTS ✓
□ CommandLineOptions class - EXISTS (may need extension)
```

### Subtask Dependencies
```
Subtask 1: None
Subtask 2: Depends on Subtask 1 (needs OutputWriter interface)
Subtask 3: Depends on Subtask 1 & 2 (needs all writers)
```

---

## Execution Strategy

**Sequential execution (dependencies):**
1. Implement Subtask 1 first (foundation)
2. Then Subtask 2 (extends interface)
3. Finally Subtask 3 (integrates everything)

**WIP Limit:** 1 agent at a time (complete before next)

---

## Risk Assessment and Mitigation

### Technical Risks

**Risk 1:** Format specification ambiguities
- **Probability:** Medium
- **Impact:** Medium
- **Mitigation:** Use standard test vectors, validate against reference implementations
- **Contingency:** Document deviations, test with real tools

**Risk 2:** Token budget overflow (if not decomposed)
- **Probability:** High (36K tokens as single task)
- **Impact:** High (task failure)
- **Mitigation:** **DECOMPOSE into 3 subtasks** ✅
- **Contingency:** Already mitigated by decomposition

---

## Success Metrics

### Completion Criteria (All Subtasks)
```
✓ Subtask 1 complete (Intel HEX working)
✓ Subtask 2 complete (S-Record + Binary working)
✓ Subtask 3 complete (Listing + CLI working)
✓ All format tests passing
✓ Integration tests passing
✓ Zero compiler warnings
✓ Command-line flags working
✓ All formats validated against standards
```

### Quality Metrics
```
- Test coverage: 95%+
- Code quality: Google C++ Style Guide
- Documentation: Doxygen comments complete
- Real-world compatibility: Tested with standard tools
```

---

## Plan Approval

**Plan Status:** Approved

**Reviewed By:**
- [X] Author: Orchestrator [2026-02-09]
- [X] Requestor: User [2026-02-09]

**Revision History:**
```
v1.0 - 2026-02-09 - Initial plan with decomposition strategy
```

---

## Notes and Considerations

**IMPORTANT:** This plan documents the decomposition approach recommended by the contract due to HIGH RISK (36K tokens).

**Next Steps:**
1. Create 3 Beads subtasks for each piece
2. Set up sequential dependencies (Subtask 2 depends on 1, Subtask 3 depends on 1 & 2)
3. Implement sequentially

**Estimated total effort:** 12-15 hours (across all subtasks)
- Subtask 1: 4-5 hours
- Subtask 2: 5-6 hours
- Subtask 3: 3-4 hours

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-09
