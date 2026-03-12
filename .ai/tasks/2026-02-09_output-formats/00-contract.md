# Task Contract

**Task ID:** 2026-02-09_output-formats
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-hze

---

## Task Description

Implement multiple output formats for assembled code, enabling xasm++ to generate industry-standard formats used by ROM programmers, embedded systems, and vintage computing platforms.

### Background and Context

Cross-assemblers must support various output formats beyond raw binary to integrate with:
- ROM programmers (Intel HEX)
- Embedded toolchains (Motorola S-Records)
- Platform-specific loaders (Apple II, C64, TRS-80, etc.)
- Human-readable assembly listings

The xasm++ core already produces assembled code via the Section/Atom framework. This task creates format-specific output writers.

### Current State

- xasm++ assembles code successfully
- Output is only available internally
- No external output format writers exist
- Section/Atom framework provides structured binary data

### Desired State

- Multiple output format writers implemented
- Configurable via command-line flags
- Support for simultaneous multiple outputs
- Industry-standard format compliance
- Human-readable listing generation

---

## Success Criteria

```
✓ Intel HEX format implemented with checksums
✓ Motorola S-Record format implemented (S19/S28/S37)
✓ Binary format writers for 5+ platforms
✓ Listing file generation working
✓ All formats validated against standards
✓ Command-line integration complete
✓ Unit tests for each format (95%+ coverage)
✓ Real-world compatibility verified
```

---

## Acceptance Criteria

### Functional Requirements - Intel HEX Format
```
□ Record type 00 (data records)
□ Record type 01 (EOF)
□ Record type 04/05 (extended address for >64K)
□ Checksum calculation and validation
□ Configurable bytes-per-line (typically 16-32)
□ Proper address wrapping
```

### Functional Requirements - Motorola S-Record Format
```
□ S0 record (header/comment)
□ S1/S2/S3 records (16/24/32-bit addressing)
□ S5/S6 record (record count - optional)
□ S7/S8/S9 records (start address/EOF)
□ Checksum calculation
□ Auto-select S1/S2/S3 based on address range
```

### Functional Requirements - Binary Formats
```
□ Raw binary (no headers)
□ Apple II DOS 3.3 binary (address + length header)
□ Commodore 64 PRG format (2-byte load address)
□ TRS-80 CMD format (system loader format)
□ CP/M COM format (100h load address assumption)
□ Game Boy ROM format (header + checksums)
```

### Functional Requirements - Listing File Format
```
□ Address column (hex, 4-6 digits)
□ Hex bytes column (assembled output)
□ Source line column (original text)
□ Symbol table section
□ Cross-reference table (optional)
□ Configurable formatting
```

### Functional Requirements - Architecture
```
□ OutputWriter interface/base class
□ Format-specific implementations
□ Factory pattern for format selection
□ Support for multiple simultaneous outputs
□ Command-line flag integration (--output-hex, --output-srec, etc.)
```

### Quality Requirements
```
□ All tests passing
□ Code coverage ≥ 95% (critical path)
□ No compiler warnings
□ Code review approved
□ Format validation tests
```

### Non-Functional Requirements
```
□ Output files compatible with standard tools
□ Performance: <100ms for typical programs (<64KB)
□ Error handling for write failures
□ Proper resource cleanup (file handles)
```

---

## Constraints and Dependencies

### Constraints
```
□ Must work with existing Section/Atom framework
□ Must not modify core assembler logic
□ Must follow existing code style (Google C++ Style Guide)
□ Command-line flags must not conflict with existing options
```

### Dependencies
```
□ Section/Atom framework (already exists in xasm++ core)
□ CommandLineOptions class (may need extension)
□ File I/O utilities (standard C++ fstream)
```

### Out of Scope
```
✗ Input format parsing (assembler handles this)
✗ Disassembler functionality
✗ Object file formats (ELF, COFF, Mach-O)
✗ Linker functionality (linking multiple object files)
✗ Debugger integration (DWARF, symbol files)
```

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- Number of files affected: 10-12 (4 formats × headers/source + tests + CLI integration)
- Lines of code estimate: ~3,000-4,000
- New concepts/patterns: OutputWriter interface, format writers
- Integration complexity: Medium (command-line integration, file output)
- Risk level: Low (well-defined formats, standard implementations)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 12 files

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 12 files → ⚠️ ACCEPTABLE (requires justification)
```

### Batch Size Justification

Files: 12 (within acceptable range but requires justification)

**Why not decomposed further:**
- High cohesion - all output writers share common interface
- Single concern - output format generation
- Already decomposed by format type (could split if token limit hit)
- Natural grouping: Interface (2) + Intel HEX (2) + S-Record (2) + Binary (2) + Listing (2) + CLI (1) + Tests (1)

**Contingency for token limits:**
- If token limit hit, will decompose into:
  - Task 1: Interface + Intel HEX + tests (4 files, ~12K tokens)
  - Task 2: S-Record + Binary formats + tests (6 files, ~18K tokens)
  - Task 3: Listing + CLI integration + tests (3 files, ~9K tokens)

**Estimated tokens:** 12 × 3,000 = 36,000 tokens
**Status:** Within 25K-32K limit? ⚠️ BORDERLINE (may need decomposition)

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
12 × 3,000 tokens = 36,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 36,000 tokens → ❌ HIGH RISK (40% failure probability)
```

**WARNING: Token budget risk**

RECOMMENDED ACTION: Decompose task into 3 smaller batches for safety.

### Decomposition Plan

This task is borderline (12 files, 36K tokens) and SHOULD be decomposed for safety:

**Subtask 1:** Interface + Intel HEX (4 files, ~12K tokens)
- Files:
  - include/xasm++/output/output_writer.h
  - src/output/output_writer.cpp
  - include/xasm++/output/intel_hex_writer.h
  - src/output/intel_hex_writer.cpp
  - tests/unit/test_intel_hex_writer.cpp
- Dependencies: None
- Estimated tokens: ~12K tokens (SAFE)

**Subtask 2:** S-Record + Binary Formats (5 files, ~15K tokens)
- Files:
  - include/xasm++/output/srec_writer.h
  - src/output/srec_writer.cpp
  - include/xasm++/output/binary_writer.h
  - src/output/binary_writer.cpp
  - tests/unit/test_output_formats.cpp
- Dependencies: Depends on Subtask 1 (needs OutputWriter interface)
- Estimated tokens: ~15K tokens (SAFE)

**Subtask 3:** Listing + CLI Integration (3 files, ~9K tokens)
- Files:
  - include/xasm++/output/listing_writer.h (if not already exists)
  - src/output/listing_writer.cpp (if not already exists)
  - src/cli_parser.cpp (modify for output flags)
  - tests/integration/test_output_integration.cpp
- Dependencies: Depends on Subtask 1 and 2
- Estimated tokens: ~9K tokens (SAFE)

**Execution Strategy:**
- Sequential (dependencies: 1 → 2 → 3)
- WIP Limit: 1 agent at a time (complete before next)

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (sequential execution)
```

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [ ] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [X] NO - SHOULD decompose (token budget 36K = HIGH RISK)

**Recommendation:** Decompose into 3 subtasks as detailed above for safety.

**Reference:** `gates/05-lean-flow.md` for enforcement details

---

## Resources and References

### Relevant Files
```
- include/xasm++/assembler/section.h - Section/Atom framework
- include/xasm++/cli/command_line_options.h - CLI integration point
- src/cli_parser.cpp - Command-line parsing
```

### Documentation
```
- Intel HEX: https://en.wikipedia.org/wiki/Intel_HEX
- Motorola S-Record: https://en.wikipedia.org/wiki/SREC_(file_format)
- Apple II: DOS 3.3 binary format (address + length header)
- C64 PRG: 2-byte little-endian load address
- TRS-80 CMD: System loader format documentation
- CP/M COM: 100h load address convention
- Game Boy ROM: https://gbdev.io/pandocs/The_Cartridge_Header.html
```

### Examples
```
- vasm: output modules (output_*.c)
- Online HEX/SREC converters for validation
```

---

## Assumptions

```
1. Section/Atom framework provides complete binary data
2. Command-line parser supports adding new flags
3. Standard C++ file I/O is sufficient (no platform-specific APIs needed)
4. Users want industry-standard format compatibility
5. Listing format can follow traditional assembler conventions
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Format specification ambiguities
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Validate against reference implementations, use standard test vectors

2. Platform-specific binary format edge cases
   - Probability: Medium
   - Impact: Low
   - Mitigation: Test with real platform loaders, document format details

3. Token budget overflow
   - Probability: Medium (36K tokens)
   - Impact: High (task failure)
   - Mitigation: DECOMPOSE into 3 subtasks as planned
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User [2026-02-09]
- [X] Agent: Orchestrator [2026-02-09]

**Changes to Contract:**
[None yet]

---

## Notes

**IMPORTANT:** This task has been identified as HIGH RISK due to token budget (36K tokens). The decomposition plan above should be followed to ensure successful completion.

The task should be split into 3 sequential subtasks:
1. Interface + Intel HEX
2. S-Record + Binary
3. Listing + CLI

**Additional note:** listing_output.h/cpp already exist (untracked files), so Subtask 3 may be simpler than estimated.

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
