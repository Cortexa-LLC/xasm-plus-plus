# Implementation Plan: Merlin Data Directives

## Approach
Follow TDD by writing tests first, then implementing directives in the Merlin parser.

## Steps

### 1. Test-Driven Development Cycle
1. ✅ Review existing test file (03_data_directives.asm)
2. ✅ Understand directive behavior from test expectations
3. ✅ Write/verify unit tests for each directive
4. ✅ Implement directives one by one (RED → GREEN → REFACTOR)

### 2. Implementation Details

#### DA (Data, Ascii)
- Parse text strings after DA directive
- Support both single-byte and comma-separated values
- Convert text to ASCII bytes

#### DCI (Data with Inverted last byte)
- Similar to DA but invert high bit of last byte
- Used for null-terminated or inverted-terminated strings in Apple II

#### INV (Inverted text)
- All bytes have high bit set (OR with 0x80)
- Used for inverse video text on Apple II

#### FLS (Flash text)
- Alternating normal and inverted bytes
- Odd-indexed bytes have high bit set
- Used for flashing text effect on Apple II

### 3. Integration
- Ensure directives work in Merlin syntax parser
- Verify they don't interfere with Scmasm parser
- Update any necessary documentation

## Testing Strategy
- Unit tests for each directive
- Binary compatibility test (03_data_directives.asm)
- Edge cases: empty strings, special characters
