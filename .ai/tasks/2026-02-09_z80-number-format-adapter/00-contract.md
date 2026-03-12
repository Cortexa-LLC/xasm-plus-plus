# Task Contract: Z80 Number Format Adapter

**Task ID:** 2026-02-09_z80-number-format-adapter
**Beads Task:** xasm++-81es
**Created:** 2026-02-09
**Priority:** P1 (BLOCKS Phase 2 integration)

Create adapter to handle Z80-specific number formats in ExpressionParser.

## Problem Statement

Phase 2 integration blocked because ExpressionParser doesn't support Z80-specific number formats:
- `0FFH` - hex with H suffix
- `377O` or `377Q` - octal with O/Q suffix
- `11111111B` - binary with B suffix
- `255D` - decimal with D suffix
- `[...]` - brackets as alternative to parentheses

## Objective

Design and implement clean solution for syntax-specific number formats in ExpressionParser.

## Success Criteria

✓ ExpressionParser supports Z80 number formats
✓ Design is extensible (other syntaxes can add their formats)
✓ All 48 Z80 expression tests pass
✓ Zero behavioral changes
✓ Clean architecture (no regex hacks)

## Design Options

### Option A: Callback Interface (Recommended)
```cpp
class INumberParser {
  virtual bool TryParse(const std::string& token, uint32_t& value) = 0;
};

class ExpressionParser {
  INumberParser* number_parser_;

  uint32_t ParseNumber() {
    uint32_t value;
    if (number_parser_->TryParse(current_token_, value)) {
      return value;
    }
    // Fall back to standard formats
  }
};
```

### Option B: Preprocessing Layer
Convert Z80 formats to standard before parsing (already attempted, fragile)

### Recommended: Option A
- Clean separation of concerns
- Extensible to other syntaxes
- Testable independently

## Acceptance Criteria

- [ ] Create INumberParser interface
- [ ] Modify ExpressionParser to use callback
- [ ] Implement Z80NumberParser
- [ ] All Z80 expression tests pass
- [ ] Integration tests pass
- [ ] Documentation complete

## Estimated Effort

2-4 hours

**Approved:** Orchestrator 2026-02-09
