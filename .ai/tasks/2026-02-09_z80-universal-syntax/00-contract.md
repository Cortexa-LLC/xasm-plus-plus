# Task Contract

**Task ID:** 2026-02-09_z80-universal-syntax
**Beads Task:** xasm++-1fn
**Created:** 2026-02-09
**Requestor:** User
**Assigned Role:** Engineer
**Workflow:** Feature (new syntax plugin)

---

## Task Description

Implement a universal Z80 syntax plugin that accepts source code from M80, ZMAC, and Z80ASM assemblers as a comprehensive superset, without requiring mode flags.

### Background and Context

Z80 is one of the most popular 8-bit CPUs, used in:
- **Game Boy** (modified Z80)
- **ZX Spectrum** (UK home computer)
- **TRS-80 Models I/III/4** (RadioShack computers)
- **Amstrad CPC** (European home computer)
- **MSX** (Japanese/European home computer standard)
- **CP/M systems** (early microcomputers)

Three major Z80 assemblers exist with overlapping but distinct syntax:
1. **M80** - Microsoft's original assembler (baseline)
2. **ZMAC** - Modern cross-assembler (superset of M80)
3. **Z80ASM** - TRS-80 focused assembler (unique local label syntax)

### Current State

- ✅ Z80 CPU plugin complete (xasm++-ocf)
  - 130+ instructions implemented
  - 130/130 tests passing
  - Production-ready
- ❌ No Z80 syntax plugin exists
- ❌ Cannot assemble Z80 source code

### Desired State

- ✅ Universal Z80 syntax plugin accepting M80, ZMAC, and Z80ASM source
- ✅ No mode flags required (superset approach)
- ✅ Context-aware disambiguation of syntax conflicts
- ✅ Comprehensive test coverage
- ✅ Can assemble real-world Z80 code from any of the three assemblers

---

## Success Criteria

Define objective, measurable criteria for completion:

```
✓ All unit tests passing (target: 50+ tests covering all directive variants)
✓ Integration tests with real M80, ZMAC, and Z80ASM source code passing
✓ Zero compiler warnings
✓ Code coverage ≥ 85%
✓ Can assemble complete Z80 programs and generate correct machine code
✓ Documentation complete (all public APIs documented)
✓ Syntax conflict resolution working ($ context disambiguation)
✓ All number format variations supported
```

---

## Acceptance Criteria

Detailed checklist of requirements that must be met:

### Functional Requirements - Data Directives
```
□ DB/DEFB/BYTE/DM/DEFM/TEXT/ASCII - define bytes
□ DW/DEFW/WORD - define words (16-bit)
□ DS/DEFS/DSPACE/BLOCK - reserve space
□ DC - define constant with count or high-bit-set string
□ DEF3/D3 - define 24-bit constants (ZMAC)
□ DEFD/DWORD - define 32-bit constants (ZMAC)
□ INCBIN - include binary file
```

### Functional Requirements - Labels
```
□ LABEL: - private label
□ LABEL:: - public label (M80 style)
□ $LOCAL: - local label (Z80ASM style)
□ ?TEMP: - temp label (ZMAC style)
□ .local: - local label (ZMAC style)
□ Label context disambiguation ($ in label vs hex)
```

### Functional Requirements - Symbols
```
□ EQU, = - fixed equate
□ SET, DEFL, = - redefinable equate
□ PUBLIC/GLOBAL/ENTRY - export symbols
□ EXTERN/EXTRN/EXT - import symbols
```

### Functional Requirements - Segments
```
□ CSEG - code segment
□ DSEG - data segment
□ ASEG - absolute segment
□ COMMON /name/ - common block
□ ORG address - set origin
□ PHASE/DEPHASE - overlays (ZMAC)
□ END [entry] - end assembly
```

### Functional Requirements - Macros
```
□ MACRO/ENDM - macro definition
□ LOCAL - local symbols in macros
□ EXITM - early exit from macro
□ REPT count - repeat block
□ IRP symbol,<list> - iterate over list
□ IRPC symbol,string - iterate over string
□ Parameter substitution with & and %
□ Nested macro support
```

### Functional Requirements - Conditionals
```
□ IF/ELSE/ENDIF - basic conditional
□ IFDEF/IFNDEF - symbol existence
□ IFEQ/IFNE - equality comparison
□ IFLT/IFGT/IFLE/IFGE - relational comparison
□ IF1/IF2 - pass-based conditional (M80)
□ IFB/IFNB - blank/non-blank (M80)
□ IFIDN/IFDIF - identical/different (M80)
□ Nested conditionals
```

### Functional Requirements - Expressions
```
□ Arithmetic: +, -, *, /, % (mod)
□ Bitwise: &, |, ^, ~, <<, >>
□ Logical: &&, ||, !
□ Comparison: ==, !=, <, >, <=, >=
□ Unary: HIGH, LOW (byte extraction)
□ C-style precedence
□ Parentheses and brackets
```

### Functional Requirements - Number Formats
```
□ $FF - hex with $ prefix
□ 0xFF - hex with 0x prefix
□ 0FFH - hex with H suffix
□ 11111111B - binary with B suffix
□ 377O, 377Q - octal with O/Q suffix
□ 255, 255D - decimal
□ Context disambiguation ($ prefix: hex vs label)
```

### Functional Requirements - Comments
```
□ ; single-line comment
□ .COMMENT delim...delim - block comment (M80)
□ comment X...X - block comment (ZMAC)
```

### Functional Requirements - Listing Control
```
□ .LIST/.XLIST - toggle listing
□ *LIST ON/OFF - Z80ASM style
□ .TITLE, .SUBTTL - titles
□ EJECT - page break
□ SPACE - blank lines
□ NAME - module name
```

### Functional Requirements - Special Features
```
□ *RADIX, .RADIX - set default number base (2-16)
□ DATE/TIME - insert system date/time (Z80ASM)
□ Undocumented Z-80: ixh/ixl/iyh/iyl, sl1
□ Optional: Cycle counting (t, tilo, tihi, ocf) - ZMAC
```

### Quality Requirements
```
□ All tests passing (target: 50+ unit tests)
□ Integration tests with real source code
□ Code coverage ≥ 85%
□ Zero compiler warnings
□ Zero linting errors
□ Code review approved (Reviewer role)
□ Documentation complete (Doxygen for all public APIs)
□ Follows existing syntax plugin patterns
```

### Non-Functional Requirements
```
□ Performance acceptable (< 1ms per line)
□ Syntax error messages clear and helpful
□ Memory usage reasonable
□ Extensible design for future features
```

---

## Constraints and Dependencies

### Constraints
```
✓ Must follow xasm++ syntax plugin architecture
✓ Must integrate with existing Z80 CPU plugin
✓ Must use C++17 (xasm++ standard)
✓ Must follow Google C++ Style Guide
✓ Must use Catch2 for unit tests
```

### Dependencies
```
✓ Z80 CPU plugin (xasm++-ocf) - COMPLETE ✓
✓ xasm++ core framework (Section, Atom, SymbolTable) - EXISTS
✓ Existing syntax plugin patterns (Merlin, SCMASM, FLEX) - EXISTS
```

### Out of Scope
```
✗ Strict mode enforcement (--strict-m80, --strict-zmac) - future enhancement
✗ Complete ZMAC cycle counting (optional, can add later)
✗ Output format generation beyond machine code
✗ Linker support (.REL format) - future enhancement
✗ HD64180 extended instructions - future enhancement
```

---

## Estimated Complexity

**Complexity:** Large

**Rationale:**
- **Number of files affected:** 3-4 files
  - Header: `include/xasm++/syntax/z80_universal_syntax.h`
  - Implementation: `src/syntax/z80_universal/z80_universal_syntax.cpp`
  - Unit tests: `tests/unit/test_z80_universal_syntax.cpp`
  - Integration tests: `tests/integration/z80_universal/` (multiple .asm files)
- **Lines of code estimate:** ~1,500-2,000 lines total
  - Header: ~400 lines
  - Implementation: ~800-1,000 lines
  - Tests: ~300-600 lines
- **New concepts/patterns:** No (follows existing syntax plugin pattern)
- **Integration complexity:** Medium (integrates with Z80 CPU plugin)
- **Risk level:** Medium (complex syntax, many variants to support)

---

## Lean Flow Analysis (MANDATORY)

**Purpose:** Prevent token limit failures and verification chaos

### Batch Size Assessment

**Estimated Files:** 3-4 files (header, implementation, unit tests, integration tests)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 3-4 files → ✅ IDEAL
```

**Status:** ✅ PROCEED - Batch size optimal

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
4 files × 3,000 tokens = 12,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 12,000 tokens → ✅ SAFE
```

**Status:** ✅ PROCEED - Well within token budget

### Work In Progress (WIP) Planning

**Concurrent Execution Assessment:**
```
How many spawned agents will run simultaneously?

├─ 1 agent → ✅ IDEAL (complete before next)
├─ 2-3 agents → ⚠️ ACCEPTABLE (within limits)
└─ 4+ agents → ❌ EXCEEDS LIMIT (verification chaos)

Planned WIP: 1 agent (Engineer for this task)
```

**Status:** ✅ PROCEED - Single agent

### Decomposition Decision

**Final Assessment:**

**Proceed as single task packet?**
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose

**Justification:**
- 3-4 files (ideal batch size)
- ~12K tokens (well within budget)
- Single agent (minimal WIP)
- High cohesion (all files tightly coupled)
- Single logical unit (syntax plugin)

**Status:** ✅ PROCEED as single task packet

---

## Resources and References

### Relevant Files
```
- include/xasm++/cpu/cpu_z80.h - Z80 CPU plugin (dependency)
- include/xasm++/syntax/merlin_syntax.h - Example syntax plugin pattern
- include/xasm++/syntax/scmasm_syntax.h - Example syntax plugin pattern
- include/xasm++/syntax/flex_syntax.h - Example syntax plugin pattern
- include/xasm++/core/section.h - Section interface
- include/xasm++/core/symbol_table.h - Symbol table interface
```

### Documentation
```
- M80 Reference: https://www.msxarchive.nl/pub/msx/programming/asm/m80l80.txt
- ZMAC Reference: http://48k.ca/zmac.html
- Z80ASM Reference: https://www.trs-80.com/sub-utilities-z80assem-manual.htm
- Reference Implementation: ~/Projects/Vintage/tools/vasm-ext/syntax/edtasm-m80/
```

### Examples
```
- Tests: tests/unit/test_merlin_syntax.cpp (similar syntax test pattern)
- Tests: tests/unit/test_scmasm_syntax.cpp (similar syntax test pattern)
- Tests: tests/unit/test_flex_syntax.cpp (macro handling example)
```

---

## Assumptions

```
1. Z80 CPU plugin API is stable and complete (xasm++-ocf)
2. Existing syntax plugin architecture is sufficient for Z80 needs
3. Universal superset approach is acceptable (no strict mode required)
4. Context-based disambiguation for $ prefix is sufficient
5. Real-world Z80 source code is available for integration testing
6. Google C++ Style Guide and C++17 standard apply
7. Catch2 test framework is available
```

*Note: If any assumption proves invalid, revisit this contract.*

---

## Risk Assessment

### Identified Risks
```
1. Syntax Ambiguity ($ prefix: hex vs local label)
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Context-aware parsing (numeric = hex, label = identifier)

2. Expression Precedence Conflicts (M80 vs ZMAC vs Z80ASM)
   - Probability: Low
   - Impact: Low
   - Mitigation: Use ZMAC C-style precedence (most expected), document differences

3. Macro System Complexity (nested macros, parameter substitution)
   - Probability: Medium
   - Impact: High
   - Mitigation: Follow FLEX ASM macro processor pattern (already implemented)

4. Integration with Z80 CPU Plugin (instruction encoding)
   - Probability: Low
   - Impact: Medium
   - Mitigation: Z80 CPU plugin is complete and well-tested

5. Real-World Compatibility (existing Z80 code may use undocumented features)
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Incremental testing with real source code, add features as needed
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: User (2026-02-09)
- [ ] Agent: Engineer (pending assignment)

**Changes to Contract:**
- 2026-02-09: Initial contract created

---

## Notes

**Design Philosophy:**
- Universal superset approach (no mode flags)
- Context-aware disambiguation
- Maximum compatibility with real-world code
- Follows existing xasm++ syntax plugin patterns
- TDD methodology (RED → GREEN → REFACTOR)

**Testing Strategy:**
- Unit tests for each directive variant (50+ tests)
- Integration tests with real M80, ZMAC, Z80ASM source
- Test syntax conflict resolution
- Test all number format variations
- Test nested macros and conditionals

**Implementation Strategy:**
- Phase 1: Core directives (DB, DW, DS, ORG, END)
- Phase 2: Symbols (EQU, SET, PUBLIC, EXTERN)
- Phase 3: Expressions (operators, HIGH/LOW)
- Phase 4: Number formats (all variants)
- Phase 5: Segments (CSEG, DSEG, ASEG, COMMON)
- Phase 6: Macros (MACRO/ENDM, REPT, IRP, IRPC)
- Phase 7: Conditionals (IF/ELSE/ENDIF family)
- Phase 8: Listing control (.LIST, .TITLE, etc.)
- Phase 9: Special features (RADIX, DATE/TIME, undocumented)
- Phase 10: Integration and polish

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
**Beads Task ID:** xasm++-1fn
