# Task Contract

**Task ID:** 2026-01-30_binary-compat-framework
**Created:** 2026-01-30
**Requestor:** Bryan Woodruff
**Beads Task:** xasm++-3gd
**Updated:** 2026-01-30 (Golden File Testing + Python Scripts)

## Requirements

Create comprehensive binary compatibility test framework validating xasm++ output against **golden reference binaries** (checked into repo) for all syntax/CPU combinations.

### Methodology: Golden File / Snapshot Testing

**Approach:**
1. Generate reference binaries ONCE using vasm-ext
2. Check reference binaries into repo (small, typically < 1KB each)
3. Tests compare xasm++ output against committed references
4. Regenerate only when needed (new tests, bug fixes, intentional changes)

**Benefits:**
- Fast tests (no vasm execution on every run)
- Deterministic (same references every time)
- No vasm dependency in CI
- Git tracks reference changes
- Works offline

### Scope: 6502 Family (6 Combinations)

| Syntax | CPU | Reference Generator | Golden Files Location |
|--------|-----|---------------------|----------------------|
| Merlin | 6502 | vasmm6502_merlin | tests/integration/binary_compat/golden/merlin/6502/ |
| Merlin | 65C02 | vasmm6502_merlin -m65c02 | tests/integration/binary_compat/golden/merlin/65c02/ |
| Merlin | 65816 | vasmm6502_merlin -m65816 | tests/integration/binary_compat/golden/merlin/65816/ |
| SCMASM | 6502 | vasmm6502_scmasm | tests/integration/binary_compat/golden/scmasm/6502/ |
| SCMASM | 65C02 | vasmm6502_scmasm -m65c02 | tests/integration/binary_compat/golden/scmasm/65c02/ |
| SCMASM | 65816 | vasmm6502_scmasm -m65816 | tests/integration/binary_compat/golden/scmasm/65816/ |

### Functional Requirements

1. **Test Framework**
   - Reusable BinaryCompatTest base class
   - Parameterized GTest integration (syntax × CPU matrix)
   - Compare xasm++ output against golden files
   - Clear failure reporting with hexdump diffs

2. **Test Sources** (~30 .asm files)
   - 6502: Basic instructions, all addressing modes
   - 65C02: BRA, PHX/PLY, STZ, TRB/TSB, (zp) indirect
   - 65816: REP/SEP, 24-bit addressing, JML/JSL/RTL, MVN/MVP
   - Syntax-specific: Merlin macros/conditionals, SCMASM strings/loops
   
3. **Golden Reference Binaries** (~30 .bin files)
   - Generated ONCE using vasm-ext
   - Checked into git (tests/integration/binary_compat/golden/)
   - Named to match test sources (01_basic_ops.bin, etc.)
   - Small files (< 1KB typically)

4. **Reference Regeneration Tool (Python)**
   - Script: `scripts/regenerate_golden_binaries.py`
   - Cross-platform (Windows, macOS, Linux)
   - Builds vasm-ext if needed (using make/cmake detection)
   - Regenerates all golden files
   - Documents which vasm version used
   - Usage: `python scripts/regenerate_golden_binaries.py [--syntax merlin|scmasm] [--cpu 6502|65c02|65816]`
   
5. **Build Integration**
   - CMake integration for test suite
   - No vasm dependency for normal test runs
   - Tests compare xasm++ output vs golden files

6. **CI/CD Pipeline**
   - Run on every commit
   - No vasm build required (golden files in repo)
   - Fail on any binary mismatch
   - Generate comparison reports
   - Cross-platform (Windows, macOS, Linux)

### Directory Structure

```
tests/integration/binary_compat/
├── framework/
│   ├── BinaryCompatTest.h
│   ├── BinaryCompatTest.cpp
│   └── golden_file_test.cpp         # Core comparison logic
├── test_sources/                     # Source .asm files
│   ├── merlin/
│   │   ├── 6502/*.asm
│   │   ├── 65c02/*.asm
│   │   └── 65816/*.asm
│   └── scmasm/
│       ├── 6502/*.asm
│       ├── 65c02/*.asm
│       └── 65816/*.asm
├── golden/                           # Reference binaries (checked in)
│   ├── merlin/
│   │   ├── 6502/*.bin
│   │   ├── 65c02/*.bin
│   │   └── 65816/*.bin
│   └── scmasm/
│       ├── 6502/*.bin
│       ├── 65c02/*.bin
│       └── 65816/*.bin
├── test_merlin_compat.cpp
├── test_scmasm_compat.cpp
├── CMakeLists.txt
└── README.md

scripts/
├── regenerate_golden_binaries.py     # Cross-platform Python script
└── compare_binaries.py               # Binary comparison utility (optional)
```

### Python Script Requirements

**regenerate_golden_binaries.py:**
```python
#!/usr/bin/env python3
"""
Regenerate golden reference binaries for binary compatibility tests.

Usage:
    python scripts/regenerate_golden_binaries.py               # All combinations
    python scripts/regenerate_golden_binaries.py --syntax merlin  # Only Merlin
    python scripts/regenerate_golden_binaries.py --cpu 65c02   # Only 65C02
    python scripts/regenerate_golden_binaries.py --force       # Rebuild vasm-ext

Requirements:
    - vasm-ext source at ~/Projects/Vintage/tools/vasm-ext
    - make (Unix) or cmake (Windows) for vasm-ext build
    - Python 3.7+
"""
```

**Features:**
- Cross-platform path handling (pathlib)
- Build vasm-ext if not present
- Detect OS and use appropriate build tools
- Generate all golden files
- Validate output
- Report statistics
- Document vasm version used

### Non-Functional Requirements

- **Cross-Platform:** Works on Windows, macOS, Linux
- **Performance:** Tests run in < 30 seconds (no vasm execution)
- **Maintainability:** Easy to add new combinations, regenerate references
- **Documentation:** Clear instructions for regenerating golden files
- **Reliability:** Byte-for-byte comparison accuracy
- **Dependencies:** Python 3.7+ (standard library only, no pip packages)

## Acceptance Criteria

- [ ] Framework infrastructure complete
  - [ ] BinaryCompatTest base class implemented
  - [ ] Golden file comparison logic
  - [ ] GTest parameterization working
  
- [ ] Test sources written (minimum 30 .asm files)
  - [ ] Merlin: 15 files (5 per CPU variant)
  - [ ] SCMASM: 15 files (5 per CPU variant)

- [ ] Golden reference binaries generated (30 .bin files)
  - [ ] Generated using vasm-ext
  - [ ] Checked into git
  - [ ] Named to match test sources
  
- [ ] Reference regeneration tool (Python)
  - [ ] scripts/regenerate_golden_binaries.py created
  - [ ] Cross-platform (tested on Windows, macOS, Linux)
  - [ ] Builds vasm-ext if needed
  - [ ] Regenerates all or selective golden files
  - [ ] Documents which vasm version used
  - [ ] No external pip dependencies (stdlib only)
  
- [ ] All 6 combinations validated
  - [ ] Merlin+6502: 100% binary match
  - [ ] Merlin+65C02: 100% binary match
  - [ ] Merlin+65816: 100% binary match
  - [ ] SCMASM+6502: 100% binary match
  - [ ] SCMASM+65C02: 100% binary match
  - [ ] SCMASM+65816: 100% binary match
  
- [ ] CI integration complete
  - [ ] GitHub Actions workflow created
  - [ ] No vasm dependency in CI
  - [ ] Tests run automatically on push
  - [ ] Works on multiple platforms (Linux, macOS, Windows)
  - [ ] Clear failure reports generated
  
- [ ] Documentation complete
  - [ ] README.md explains golden file approach
  - [ ] Instructions for regenerating references (cross-platform)
  - [ ] Instructions for adding new tests
  - [ ] Troubleshooting guide

## Success Metrics

- 100% binary compatibility with vasm-ext golden files (byte-for-byte)
- Tests run in < 30 seconds (vs > 2 minutes with vasm execution)
- Zero false positives/negatives
- Tests catch regressions immediately
- Framework supports future CPU additions
- Works on Windows, macOS, and Linux

## Workflow: Regenerating Golden Files

**When to regenerate:**
- Adding new test source files
- Fixing bugs in xasm++ that change binary output (intentionally)
- Updating to newer vasm-ext version
- Discovering vasm-ext bugs

**How to regenerate (cross-platform):**
```bash
# Regenerate all golden files
python scripts/regenerate_golden_binaries.py

# Regenerate specific syntax
python scripts/regenerate_golden_binaries.py --syntax merlin

# Regenerate specific CPU
python scripts/regenerate_golden_binaries.py --cpu 65c02

# Force rebuild vasm-ext
python scripts/regenerate_golden_binaries.py --force

# Review changes
git diff tests/integration/binary_compat/golden/

# Commit if correct
git add tests/integration/binary_compat/golden/
git commit -m "Update golden reference binaries"
```

## Stakeholders

- **Product Owner:** Bryan Woodruff
- **Tech Lead:** Bryan Woodruff
- **QA:** Automated test suite

## Dependencies

- vasm-ext source code (for regeneration only, not normal tests)
- Python 3.7+ (cross-platform)
- CMake build system
- GTest framework (already integrated)
- GitHub Actions (for CI)

## Risks & Mitigations

**Risk:** Golden files become stale
**Mitigation:** Document vasm version used, add regeneration to workflow checklist

**Risk:** Binary mismatches due to vasm bugs
**Mitigation:** Document known issues, verify against vintage hardware, regenerate if vasm fixed

**Risk:** Git repo size (binary files)
**Mitigation:** Binaries are tiny (< 1KB each), 30 files = ~30KB total (negligible)

**Risk:** Windows compatibility issues
**Mitigation:** Use Python for all scripts, pathlib for paths, subprocess for cross-platform execution

## Timeline

Estimated: 12 hours (reduced from 15.5 with vasm execution approach)
- Framework: 3 hours (simpler without vasm integration)
- Test sources: 6 hours
- Golden file generation: 1 hour
- Python regeneration script: 1.5 hours (cross-platform)
- GTest integration: 1 hour
- CI setup: 1 hour (simpler without vasm)
- Documentation: 1 hour
