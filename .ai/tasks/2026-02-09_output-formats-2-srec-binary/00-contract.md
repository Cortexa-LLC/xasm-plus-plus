# Task Contract: Output Formats 2 - S-Record + Platform-Specific Binary

**Task ID:** 2026-02-09_output-formats-2-srec-binary
**Created:** 2026-02-09
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-lyai

---

## Task Description

Implement S-Record format writer and platform-specific binary format writers for vintage computing platforms.

### Background and Context

Output Formats Subtask 1 implemented the OutputWriter interface and Intel HEX format. This task extends the output format system with:
1. **S-Record formats** (S19/S28/S37) - Industry-standard format used by Motorola tools
2. **Platform-specific binary formats** - Native executable formats for vintage computers

These formats are essential for:
- Cross-development targeting vintage hardware
- Compatibility with period-accurate toolchains
- Direct loading on emulators and real hardware

### Current State

- OutputWriter interface exists and is proven (Intel HEX implementation complete)
- No S-Record format support
- No platform-specific binary format support
- CLI cannot output in vintage platform formats

### Desired State

- S-Record writer supporting S19 (16-bit), S28 (24-bit), S37 (32-bit) variants
- Platform-specific binary writers for:
  - **TRS-DOS/NEWDOS/80** format (Z80 systems: TRS-80 Model I/III/4)
  - **CoCo DOS (LOADM)** format (6809 systems: TRS-80 Color Computer)
  - **OS/9** format (6809/Z80 systems: OS-9 operating system)
- Comprehensive test coverage for all formats
- Ready for CLI integration (Subtask 3)

---

## Success Criteria

```
✓ S-Record writer implemented (S19/S28/S37 variants)
✓ TRS-DOS/NEWDOS/80 binary writer implemented (Z80)
✓ CoCo DOS (LOADM) binary writer implemented (6809)
✓ OS/9 binary writer implemented (6809/Z80)
✓ All format writers inherit OutputWriter interface
✓ Comprehensive test suite with 100% pass rate
✓ Zero compiler warnings
✓ Documentation complete (format specs, usage examples)
```

---

## Acceptance Criteria

### Functional Requirements

**S-Record Format:**
```
□ SRecordWriter class inherits OutputWriter
□ Supports S19 format (16-bit addresses, max 64KB)
□ Supports S28 format (24-bit addresses, max 16MB)
□ Supports S37 format (32-bit addresses, max 4GB)
□ Automatic format selection based on address range
□ Correct checksum calculation (two's complement)
□ Configurable bytes-per-record (default 32)
□ Header record (S0) with filename/metadata
□ Data records (S1/S2/S3) for program code
□ Count records (S5/S6) for record count
□ Start address records (S7/S8/S9) for entry point
```

**TRS-DOS/NEWDOS/80 Binary Format (Z80):**
```
□ TrsDosWriter class inherits OutputWriter
□ Binary header with load address
□ Contiguous binary data
□ Entry point address
□ Compatible with SYSTEM command on TRS-80
□ Multiple segment support (address gaps)
```

**CoCo DOS (LOADM) Format (6809):**
```
□ CocoLoadmWriter class inherits OutputWriter
□ Preamble block (0x00 header)
□ Data blocks with address and length
□ Postamble block (0xFF terminator)
□ Entry point address (if specified)
□ Compatible with LOADM command on Color Computer
□ Multiple segment support
```

**OS/9 Format (6809/Z80):**
```
□ Os9ModuleWriter class inherits OutputWriter
□ Module header with sync bytes ($87CD)
□ Module type (program, subroutine, data, etc.)
□ Module name
□ CRC checksum (CRC-24)
□ Compatible with OS-9 module format specification
□ Supports both 6809 and Z80 OS-9 variants
```

### Quality Requirements

```
□ All tests passing (target: 40+ tests across all formats)
□ Code coverage ≥ 90% for new code
□ Zero compiler warnings
□ Code follows Google C++ Style Guide
□ Comprehensive documentation (format specs, examples)
□ TDD methodology followed (RED-GREEN-REFACTOR)
```

### Non-Functional Requirements

```
□ Performance: O(n) where n = number of bytes
□ Memory: Minimal allocations, streaming output
□ Extensibility: Easy to add new formats
□ Maintainability: Clear separation of format-specific logic
```

---

## Constraints and Dependencies

### Constraints

```
□ Must follow OutputWriter interface (established in Subtask 1)
□ Must not break existing Intel HEX writer
□ Must use std::ostream for output (no file I/O in writer)
□ Must validate format specifications against period documentation
```

### Dependencies

```
✓ OutputWriter interface (Subtask 1 - COMPLETE)
✓ Section/Atom framework (exists)
✓ Google Test framework (available)
✓ CMake build system (configured)
```

### Out of Scope

```
✗ CLI integration (Subtask 3)
✗ Listing output (Subtask 3)
✗ File I/O (handled by CLI, not writers)
✗ Platform-specific loader utilities
✗ Emulator-specific formats (not period-accurate)
```

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 8 files (4 headers + 4 implementations)
- Lines of code estimate: ~1500 lines (4 writers × ~375 lines each)
- New concepts/patterns: S-Record format, vintage binary formats
- Integration complexity: Low (follows established OutputWriter pattern)
- Risk level: Low (isolated new code, existing tests validate interface)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 8 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 8 files → ⚠️ ACCEPTABLE (requires justification)
```

### Batch Size Justification

Files: 8 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion: All writers follow same OutputWriter pattern
- Single concern: Output format implementations
- Parallel test development: All writers can be tested independently
- Already minimal viable batch: Each writer is atomic (header + impl)

**Contingency for token limits:**
- If token limit hit, will decompose into: S-Record (2 files) + Binary formats (6 files, can be done in parallel)

**Estimated tokens:** ~8 × 3000 = 24,000 tokens
**Status:** Within 25K-32K limit? YES (24K < 25K, borderline but acceptable)

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
8 × 3,000 tokens = 24,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 24,000 tokens → ⚠️ APPROACHING LIMIT (borderline, but acceptable)
```

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [x] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

**Justification:**
- 8 files (within limit)
- 24K tokens (borderline but acceptable)
- 1 agent (ideal)
- High cohesion (all follow same pattern)

**Contingency:** If agent hits token limit during implementation, decompose into:
1. S-Record writer (2 files, ~6K tokens)
2. Binary format writers (6 files, ~18K tokens)

---

## Resources and References

### Relevant Files

```
- include/xasm++/output/output_writer.h - Base interface (existing)
- include/xasm++/output/intel_hex_writer.h - Reference implementation (existing)
- src/output/intel_hex_writer.cpp - Reference implementation (existing)
```

### Documentation

```
- S-Record format specification (Motorola)
- TRS-DOS system documentation
- CoCo DOS technical manual
- OS-9 module format specification
- Intel HEX writer (reference for pattern)
```

### Examples

```
- tests/unit/test_intel_hex_writer.cpp - Test pattern reference
```

---

## Assumptions

```
1. OutputWriter interface is stable (no changes needed)
2. Section/Atom framework provides all necessary data
3. Format specifications are well-documented
4. Test framework supports multiple output formats
5. Platform-specific formats are for file output, not direct hardware interaction
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks

```
1. Token budget borderline (24K approaching 25K limit)
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Implement in phases, decompose if needed
   - Contingency: Split into S-Record + Binary formats subtasks

2. Format specifications ambiguous or incomplete
   - Probability: Low
   - Impact: Medium
   - Mitigation: Reference period documentation, test against known binaries
   - Contingency: Implement core format, mark advanced features as future work

3. Multiple binary formats increase test complexity
   - Probability: Low
   - Impact: Low
   - Mitigation: Follow Intel HEX test pattern, test each format independently
   - Contingency: Add integration tests in Subtask 3
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Bryan Woodruff (Orchestrator) 2026-02-09
- [ ] Agent: Engineer [Pending]

**Changes to Contract:**
```
2026-02-09: Added platform-specific binary format requirements:
  - TRS-DOS/NEWDOS/80 (Z80)
  - CoCo DOS LOADM (6809)
  - OS/9 (6809/Z80)
```

---

## Notes

**Platform-Specific Binary Formats:**

**TRS-DOS/NEWDOS/80 (Z80):**
- Used on TRS-80 Model I, III, 4
- Simple header-data-entry format
- Load address, binary data, entry point
- Command: `SYSTEM "filename"`

**CoCo DOS LOADM (6809):**
- Used on TRS-80 Color Computer
- Preamble (0x00), data blocks, postamble (0xFF)
- Each block: address, length, data
- Command: `LOADM "filename"` then `EXEC &HXXXX`

**OS/9 Module Format (6809/Z80):**
- Used on OS-9 operating system
- Sync bytes ($87CD), header, CRC
- Self-contained executable modules
- Types: program, subroutine, trap handler, device driver
- Both 6809 and Z80 variants exist

**Implementation Strategy:**
1. Start with S-Record (most similar to Intel HEX)
2. Then TRS-DOS (simplest binary format)
3. Then CoCo LOADM (block-based)
4. Finally OS/9 (most complex, CRC calculation)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
