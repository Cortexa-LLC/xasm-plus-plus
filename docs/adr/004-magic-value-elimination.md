# ADR-0003: Magic Value Elimination Policy

**Status:** Accepted
**Date:** 2026-02-14
**Deciders:** Development Team
**Context Source:** Architectural Review (xasm++-99f4)

---

## Context

The comprehensive architectural review identified 450+ magic values across the codebase:

**Magic Value Distribution:**
- **245 magic hex values** - Opcodes, format bytes, masks
- **96 magic radix values** - 2, 8, 10, 16 in parsing code
- **66 magic strings** - Mnemonics, directives, format identifiers
- **43 magic characters** - Delimiters, operators, prefixes

**Problems:**
1. **Unmaintainable** - What does `0xCB` mean?
2. **Error-prone** - Easy to mistype `0xCB` vs `0xBC`
3. **Undocumentable** - Can't grep for "WAI instruction" if it's just `0xCB`
4. **Violates clean code** - Self-documenting code principle

**Example Issues:**
```cpp
// BAD: What is 0xCB?
if (opcode == 0xCB) {
  // ... 20 lines of code ...
}

// BAD: What radix is 16?
address = std::stoul(operand, nullptr, 16);

// BAD: Magic delimiter
operands = Split(operand, ',');
```

---

## Decision

**ZERO TOLERANCE for magic values in production code.**

All magic values MUST be replaced with named constants following this policy:

---

## 1. Magic Hex Values (Opcodes, Masks, Format Bytes)

### Policy

**All hexadecimal literal values MUST have named constants.**

### Implementation

```cpp
// Header: opcodes_<cpu>.h
namespace xasm {
namespace Opcodes {
  // Naming: MNEMONIC_ADDRESSINGMODE or descriptive name
  constexpr uint8_t WAI = 0xCB;
  constexpr uint8_t STP = 0xDB;
  constexpr uint8_t BRA = 0x80;

  // Multi-byte opcodes
  constexpr uint16_t STP_IMPLIED = 0x10FF;  // Page 2 opcode
}}

// Usage in code
if (opcode == Opcodes::WAI) {
  // ... implementation ...
}
```

### Naming Convention

**Format:** `<MNEMONIC>_<ADDRESSING_MODE>`

**Examples:**
- `LDA_IMMEDIATE` = 0xA9
- `LDA_ZEROPAGE` = 0xA5
- `LDA_ABSOLUTE` = 0xAD
- `JMP_INDIRECT` = 0x6C

**For non-opcode hex:**
- `INTEL_HEX_RECORD_MARKER` = ':'
- `SREC_TYPE_DATA` = 0x01

---

## 2. Magic Radix Values (2, 8, 10, 16)

### Policy

**All radix values in parsing functions MUST use constants.**

### Implementation

```cpp
// Header: radix_constants.h
namespace xasm {
  constexpr int RADIX_BINARY = 2;
  constexpr int RADIX_OCTAL = 8;
  constexpr int RADIX_DECIMAL = 10;
  constexpr int RADIX_HEXADECIMAL = 16;
}

// Usage
uint32_t value = std::stoul(operand, nullptr, RADIX_HEXADECIMAL);
uint32_t decimal = std::stoi(token, nullptr, RADIX_DECIMAL);
```

### Rationale
- **Clarity:** Intent is explicit
- **Grep-able:** Can search for "RADIX_HEXADECIMAL"
- **Self-documenting:** No need to remember "16 means hex"

---

## 3. Magic Strings (Mnemonics, Directives)

### Policy

**All string literals used for identification MUST have constants.**

### Implementation

```cpp
// Header: <cpu>_mnemonics.h
namespace xasm {
namespace Z80Mnemonics {
  constexpr const char* JR = "JR";
  constexpr const char* DJNZ = "DJNZ";
  constexpr const char* JP = "JP";
}}

// Usage
if (mnemonic == Z80Mnemonics::JR) {
  return INSTRUCTION_SIZE_TWO_BYTES;
}
```

### When to Use

**Use constants for:**
- CPU mnemonics (JR, DJNZ, etc.)
- Directive names (ORG, EQU, etc.)
- Output format identifiers
- Error message prefixes (if repeated)

**Don't use for:**
- One-off user messages
- Debug output (ephemeral)
- Comments (not code)

---

## 4. Magic Characters (Delimiters, Operators)

### Policy

**Frequently-used character literals MUST have named constants.**

### Implementation

```cpp
// Header: syntax_constants.h
namespace xasm {
namespace SyntaxChars {
  constexpr char COMMENT_DELIMITER = ';';
  constexpr char OPERAND_SEPARATOR = ',';
  constexpr char HEX_PREFIX = '$';
  constexpr char BINARY_PREFIX = '%';
}}

// Usage
if (line[0] == SyntaxChars::COMMENT_DELIMITER) {
  return;  // Skip comment
}

std::vector<std::string> operands = Split(operand, SyntaxChars::OPERAND_SEPARATOR);
```

### Threshold

**Use constant if:**
- Character appears in 3+ locations, OR
- Semantic meaning is not obvious, OR
- Could vary by syntax (e.g., comment char)

**Exception:**
- Standard characters (' ', '\n', '\t') don't need constants

---

## 5. Magic Numbers (Sizes, Offsets, Counts)

### Policy

**All numeric literals with semantic meaning MUST have constants.**

### Implementation

```cpp
// Header: instruction_constants.h
namespace xasm {
  constexpr int INSTRUCTION_SIZE_SINGLE_BYTE = 1;
  constexpr int INSTRUCTION_SIZE_TWO_BYTES = 2;
  constexpr int INSTRUCTION_SIZE_THREE_BYTES = 3;

  constexpr int BITS_PER_BYTE = 8;
  constexpr int MAX_8BIT_VALUE = 255;
  constexpr int MAX_16BIT_VALUE = 65535;
}

// Usage
if (mnemonic == Z80Mnemonics::JR) {
  return INSTRUCTION_SIZE_TWO_BYTES;
}
```

---

## Exceptions

### Allowed Magic Values

**These do NOT need constants:**

1. **Mathematical constants:** 0, 1, -1 in arithmetic
   ```cpp
   count++;         // OK
   value = value + 1;  // OK
   ```

2. **Array/string indices:** 0, 1, 2 for positional access
   ```cpp
   char first = str[0];  // OK
   ```

3. **Boolean comparisons:** 0 for null check
   ```cpp
   if (ptr == nullptr) { }  // OK (but prefer nullptr)
   ```

4. **Loop initialization:** 0 in for loops
   ```cpp
   for (int i = 0; i < count; ++i) { }  // OK
   ```

5. **Powers of 2 in bit operations** (but consider named constants if repeated)
   ```cpp
   value << 1  // OK (shift by 1)
   value & 0xFF  // REQUIRES constant: BYTE_MASK
   ```

---

## Migration Strategy

### Priority Order

**P0 (Critical):**
1. CPU opcodes (6502: 72, 6809: 215 instances)
2. Lambda anti-patterns (blocks other work)

**P1 (High):**
3. Radix values (96 instances)
4. Output formatter magic values (60+ instances)

**P2 (Medium):**
5. Instruction sizes, format constants
6. Remaining magic characters/strings

### Reference Implementation

**Commit 7341e43:** Rockwell CPU magic value elimination
- Shows opcode constant creation pattern
- Demonstrates naming convention
- Example of systematic replacement

---

## Consequences

### Positive

✅ **Self-documenting code** - `Opcodes::WAI` instead of `0xCB`
✅ **Grep-able** - Can search for "WAI" to find all uses
✅ **Refactor-safe** - Change constant definition, all uses update
✅ **Error reduction** - Type safety, no typos in hex values
✅ **Maintainable** - Clear intent

### Negative

⚠️ **More lines of code** - Constant definitions add LOC
⚠️ **Header dependencies** - More includes needed

### Neutral

🔄 **Compilation time** - Minimal impact (header-only constants)

---

## Compliance

### For New Code

**MANDATORY:**
- No magic hex values (use Opcodes:: namespace)
- No magic radix values (use RADIX_* constants)
- No magic mnemonics (use Mnemonics:: namespace)
- No magic format bytes (use named constants)

### Code Review Checklist

- [ ] No unexplained hex literals (0x...)
- [ ] No radix magic (2, 8, 10, 16 in parsing)
- [ ] No repeated string literals
- [ ] No semantic numeric literals without constants
- [ ] Constants follow naming convention

---

## Verification

### Automated Detection

```bash
# Find potential magic hex values
grep -r "0x[0-9A-Fa-f]\+" src/ | grep -v "Opcodes::" | grep -v "constexpr"

# Find radix magic values
grep -r "stoul.*nullptr.*16\|stoi.*nullptr.*10" src/

# Find magic strings (harder - manual review needed)
grep -r '"[A-Z]\{2,\}"' src/
```

### Quality Gate

**Before PR merge:**
1. Run detection scripts
2. Verify all flagged values have justification or constants
3. Check naming follows convention

---

## References

**Architectural Review:** `.ai/tasks/2026-02-14_architectural-review/`
**Reference Commit:** 7341e43 (Rockwell magic value fix)
**Related ADRs:** ADR-0001, ADR-0002
**Implementation Tasks:**
- xasm++-lmzc (CPU 6502 opcodes) - P0
- xasm++-oi41 (CPU 6809 opcodes) - P0

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-14 | AI Architect | Initial policy based on architectural review |
