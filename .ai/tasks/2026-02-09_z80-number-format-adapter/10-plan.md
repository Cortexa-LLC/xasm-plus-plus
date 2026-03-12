# Implementation Plan: Z80 Number Format Adapter

**Task:** 2026-02-09_z80-number-format-adapter
**Beads ID:** xasm++-81es
**Engineer:** AI Engineer Agent
**Date:** 2026-02-09

## Requirements Analysis

From test_z80_expressions.cpp, Z80 syntax requires:
- `0FFH` - hex with H suffix
- `377O` / `377Q` - octal with O/Q suffix  
- `11111111B` - binary with B suffix
- `255D` - decimal with D suffix
- `[...]` - brackets as alternative to parentheses

Current ExpressionParser supports:
- `$FF` - hex with $ prefix
- `0xFF` - hex with 0x prefix
- `%10101010` - binary with % prefix
- `0b10101010` - binary with 0b prefix
- `123` - decimal
- `(...)` - parentheses only

## Design: INumberParser Interface (Option A)

Create a callback interface to allow syntax-specific number parsing:

```cpp
class INumberParser {
  virtual bool TryParse(const std::string& token, int64_t& value) = 0;
  virtual ~INumberParser() = default;
};
```

Modify ExpressionParser:
- Add `INumberParser* number_parser_` member (nullable)
- In ParseNumber(), try number_parser_ first, then fall back to standard formats
- In ParsePrimary(), try number_parser_ for bracket support

Implement Z80NumberParser:
- Parse H-suffix hex: `0FFH`
- Parse O/Q-suffix octal: `377O`, `377Q`
- Parse B-suffix binary: `11111111B`
- Parse D-suffix decimal: `255D`

## Implementation Steps

### Phase 1: Create Interface (TDD)
1. ✅ Write test for INumberParser interface
2. ✅ Create INumberParser interface in expression_parser.h
3. ✅ Verify test passes

### Phase 2: Modify ExpressionParser (TDD)
1. ✅ Write test for ExpressionParser with custom parser
2. ✅ Add number_parser_ member to ExpressionParser
3. ✅ Modify ParseNumber() to use callback
4. ✅ Verify tests pass

### Phase 3: Implement Z80NumberParser (TDD)
1. ✅ Write tests for H-suffix hex (0FFH)
2. ✅ Implement H-suffix parsing
3. ✅ Write tests for O/Q-suffix octal (377O, 377Q)
4. ✅ Implement O/Q-suffix parsing
5. ✅ Write tests for B-suffix binary (11111111B)
6. ✅ Implement B-suffix parsing
7. ✅ Write tests for D-suffix decimal (255D)
8. ✅ Implement D-suffix parsing
9. ✅ Verify all tests pass

### Phase 4: Bracket Support (TDD)
1. ✅ Write tests for bracket expressions [...]
2. ✅ Modify ParsePrimary() to handle brackets
3. ✅ Verify tests pass

### Phase 5: Integration (TDD)
1. ✅ Integrate Z80NumberParser into Z80UniversalSyntaxParser
2. ✅ Run all Z80 expression tests (48 tests)
3. ✅ Verify zero behavioral changes
4. ✅ Verify all tests pass

### Phase 6: Documentation
1. ✅ Add docstrings to INumberParser
2. ✅ Add docstrings to Z80NumberParser
3. ✅ Update ExpressionParser documentation
4. ✅ Update work log

## File Changes

**New Files:**
- None (adding to existing headers/sources)

**Modified Files:**
- `include/xasm++/common/expression_parser.h` - Add INumberParser interface
- `src/common/expression_parser.cpp` - Modify ParseNumber() and ParsePrimary()
- `include/xasm++/syntax/z80_universal_syntax.h` - Add Z80NumberParser class
- `src/syntax/z80_universal_syntax.cpp` - Implement Z80NumberParser

**Test Files:**
- `tests/unit/test_expression_parser.cpp` - Unit tests for interface
- `tests/unit/test_z80_expressions.cpp` - Already has integration tests

## Estimated Effort

- Interface creation: 30 min
- ExpressionParser modification: 45 min
- Z80NumberParser implementation: 1 hour
- Bracket support: 30 min
- Integration and testing: 45 min
- Documentation: 30 min

**Total: 4 hours**

## Success Criteria

- [ ] INumberParser interface created
- [ ] ExpressionParser uses callback pattern
- [ ] Z80NumberParser implements all formats
- [ ] Bracket support works
- [ ] All 48 Z80 expression tests pass
- [ ] Zero behavioral changes to existing code
- [ ] All code has docstrings and type hints
- [ ] Work log updated

## Notes

- Design is extensible: other syntaxes can implement INumberParser
- Maintains backward compatibility: number_parser_ is optional (nullable)
- Clean separation: syntax-specific logic stays in syntax parser
