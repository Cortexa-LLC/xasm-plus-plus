# Task Contract: Z80 Phase 4 - Extended Number Formats

**Task ID:** 2026-02-09_z80-phase4-numbers
**Created:** 2026-02-09
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-a5g

---

## Task Description

Extend number format parsing to support multi-byte constants, character constants, and string-to-number conversions in Z80 Universal Syntax.

### Background and Context

Z80 Phase 3 (Expression Evaluation) is complete. Phase 4 adds enhanced number format support needed for:
- Multi-byte constants in DB/DW/DD directives
- Character literals in expressions
- String encoding as numeric constants
- 24-bit and 32-bit immediate values

### Current State

- Basic number parsing: decimal, hex ($xx, 0xNN), binary (xxxxB), octal (Oxx)
- Expression evaluation working (Phase 3 complete)
- Limited to 16-bit values
- No character literal support
- No string-to-number conversion

### Desired State

- Multi-byte constants (24-bit, 32-bit)
- Character constants: 'A', '\n', '\x1B'
- String to multi-byte: "AB" → $4142
- Expression evaluation in all constant contexts
- Proper range checking and warnings

---

## Success Criteria

```
✓ Character literals ('A', '\n', etc.) parsed correctly
✓ Multi-byte constants (24-bit, 32-bit) supported
✓ String-to-number conversion working
✓ Expression evaluation in constant contexts
✓ Range checking with appropriate warnings
✓ 20+ tests covering all formats
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements

**Character Constants:**
```
□ 'A' → 65 (0x41)
□ '\n' → 10 (0x0A)
□ '\r' → 13 (0x0D)
□ '\t' → 9 (0x09)
□ '\\' → 92 (0x5C)
□ '\'' → 39 (0x27)
□ '\x1B' → 27 (0x1B) - hex escape
□ '\101' → 65 (0x41) - octal escape
```

**Multi-Byte Constants:**
```
□ 24-bit: $123456, 0x123456
□ 32-bit: $12345678, 0x12345678
□ DB/DW/DD directives accept expressions
□ Range warnings for out-of-range values
```

**String-to-Number:**
```
□ "AB" → $4142 (16-bit, two bytes)
□ "ABC" → $414243 (24-bit, three bytes)
□ Proper endianness handling
```

### Quality Requirements

```
□ All tests passing (20+ tests)
□ Code coverage ≥ 90%
□ Zero compiler warnings
□ TDD methodology (RED-GREEN-REFACTOR)
□ Google C++ Style Guide compliance
```

---

## Constraints and Dependencies

### Constraints

```
□ Must integrate with existing ParseNumber
□ Must not break existing Phase 1-3 functionality
□ Character encoding: ASCII
□ Multi-byte values: little-endian by default
```

### Dependencies

```
✓ Z80 Phase 3 (Expression Evaluation) - COMPLETE
✓ ParseNumber method exists
✓ Google Test framework available
```

### Out of Scope

```
✗ Unicode support
✗ Multi-byte character encodings (UTF-8, etc.)
✗ Custom character sets
✗ Floating point literals
```

---

## Estimated Complexity

**Complexity:** Small

**Rationale:**
- Files affected: 1-2 files (enhance ParseNumber, add tests)
- Lines of code: ~200-300 lines
- New concepts: Character escapes, multi-byte parsing
- Integration complexity: Low (extends existing parser)
- Risk level: Low (isolated to number parsing)

---

## Lean Flow Analysis

### Batch Size Assessment

**Estimated Files:** 2 files

```
File Count: 2 files → ✅ IDEAL (1-5 files)
Token Budget: 2 × 3,000 = 6,000 tokens → ✅ SAFE (<20K)
WIP: 1 agent → ✅ IDEAL
```

**Proceed as single task packet:** YES

---

## Resources and References

```
- include/xasm++/syntax/z80_universal_syntax.h
- src/syntax/z80_universal/z80_universal_syntax.cpp
- tests/unit/test_z80_universal_syntax.cpp
```

---

## Assumptions

```
1. Character literals use C-style escape sequences
2. String-to-number uses ASCII encoding
3. Multi-byte values are little-endian (Z80 convention)
4. Out-of-range values generate warnings, not errors
```

---

## Risk Assessment

```
1. Character escape parsing complexity
   - Probability: Low
   - Impact: Low
   - Mitigation: Follow C escape sequence standard

2. Endianness confusion
   - Probability: Low
   - Impact: Medium
   - Mitigation: Test with known multi-byte values, document clearly
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [x] Requestor: Bryan Woodruff (Orchestrator) 2026-02-09
- [ ] Agent: Engineer [Pending]

---

## Notes

**Example Formats:**

```assembly
; Character constants
DB 'A'              ; Single character
DB 'Hello',0        ; String with null terminator
DB '\n'             ; Newline escape

; Multi-byte constants
DW $1234            ; 16-bit word
DD $12345678        ; 32-bit double word

; String to number
DW "AB"             ; Two-byte constant ($4142)

; Expressions with characters
DB 'A' + 1          ; 66 (0x42, 'B')
DB HIGH("AB")       ; 0x41
DB LOW("AB")        ; 0x42
```

**Implementation Notes:**
- Extend ParseNumber to recognize ' for character literals
- Add ParseCharLiteral method for escape sequences
- Add ParseStringAsNumber for multi-byte string constants
- Update ParseNumber to handle 24-bit and 32-bit values
- Add range checking in DB/DW/DD directive handlers

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-09
