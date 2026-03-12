# Task Contract

**Task ID:** 2026-01-29_hex-comma
**Beads Task:** xasm++-56z
**Created:** 2026-01-29
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature (.ai-pack/workflows/feature.md)

---

## Task Description

Implement HEX directive comma-separated value support for Prince of Persia compatibility.

### Background and Context

Prince of Persia source files use HEX directives with comma-separated bytes:
```assembly
hex 01,02,03,04      ; Currently fails
hex 010203           ; Currently works
```

This is a Merlin assembler convention. Currently xasm++ only supports continuous hex strings, blocking 6/29 PoP files.

### Current State

- HEX continuous works: `hex 010203` → bytes 01 02 03 ✓
- HEX comma-separated fails: `hex 01,02,03` ❌
- 6 PoP files blocked (114 HEX occurrences)

### Desired State

- HEX comma-separated works: `hex 01,02,03` → bytes 01 02 03 ✓
- HEX continuous still works: `hex 010203` → bytes 01 02 03 ✓
- 6 PoP files unblocked

---

## Success Criteria

```
✓ HEX comma-separated works (hex 01,02,03)
✓ HEX continuous still works (hex 010203)
✓ Mixed formats work (hex 01,02 03,04)
✓ All existing tests passing (574/575)
✓ New HEX comma tests added
✓ Zero compiler warnings
```

---

## Acceptance Criteria

### Functional Requirements
```
□ hex 01,02,03 → bytes 01 02 03
□ hex 01,02,03,04,05 → bytes 01 02 03 04 05
□ hex 010203 → bytes 01 02 03 (no change)
□ hex FF,00,AA → bytes FF 00 AA
□ Whitespace handling (hex 01 , 02 ,03)
□ Error handling preserved (invalid hex, odd digits)
```

### Quality Requirements
```
□ All tests passing (574/575 + new tests)
□ Zero compiler warnings
□ Code review approved
□ TDD process followed
```

---

## Implementation Approach

**Location:** `src/syntax/merlin_syntax.cpp` - `handle_HEX()` function

**Current Logic:**
```cpp
void MerlinSyntaxParser::handle_HEX(const std::string& hex_data) {
    // Parses continuous hex string only
    // "010203" → {0x01, 0x02, 0x03}
}
```

**New Logic:**
```cpp
void MerlinSyntaxParser::handle_HEX(const std::string& hex_data) {
    // Step 1: Split by commas
    std::vector<std::string> parts = split(hex_data, ',');

    // Step 2: Parse each part as hex string
    for (auto& part : parts) {
        // Existing hex parsing logic
    }
}
```

**Test Cases:**
1. `hex 01,02,03` → {0x01, 0x02, 0x03}
2. `hex 010203` → {0x01, 0x02, 0x03} (no change)
3. `hex FF,00,AA,BB` → {0xFF, 0x00, 0xAA, 0xBB}
4. Error handling for invalid hex

---

## References

**Prince of Persia Analysis:**
- `docs/investigations/pop-compatibility-analysis.md`
- 6 files affected: MASTER.S, BGDATA.S, etc.
- 114 HEX occurrences with commas

**Related Code:**
- `src/syntax/merlin/merlin_syntax.cpp` - handle_HEX()
- `tests/unit/test_merlin_syntax.cpp` - HEX tests

---

**Contract Version:** 1.0
**Last Updated:** 2026-01-29
