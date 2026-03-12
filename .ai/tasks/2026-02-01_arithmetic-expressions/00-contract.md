# Task Contract

**Task ID:** 2026-02-01_arithmetic-expressions
**Created:** 2026-02-01
**Requestor:** Bryan Woodruff
**Assigned Role:** Engineer
**Workflow:** Feature
**Beads Task:** xasm++-q53

---

## Task Description

Implement arithmetic expression parsing in the assembler's `ParseExpression` function to support expressions with operators (+, -, *, /) in data directives.

### Background and Context

Currently, the `ParseExpression` function (src/core/assembler.cpp:25-81) only handles:
- Simple hex literals ($C0)
- Binary literals (%1010)
- Decimal numbers (42)
- Symbol references (label)

However, Merlin assembly source files use arithmetic expressions in DB/DW directives:
- `db $c0+4` - hex literal plus decimal
- `db $900-*` - hex literal minus program counter
- `dw label-1` - symbol reference with offset

This is blocking 3 of 29 Prince of Persia source files from assembling:
- FRAMEDEF.S (uses `db $c0+4` on lines 23, 66, 129, etc.)
- TABLES.S (uses similar expressions)
- UNPACK.S (uses similar expressions)

### Current State

**Error when assembling FRAMEDEF.S:**
```
Logic error: Invalid hex digit '+' in hex string: '$c0+4'
```

**Current ParseExpression logic (lines 30-38):**
```cpp
// Hex literal: $1234
if (!trimmed.empty() && trimmed[0] == '$') {
    // Strip addressing mode suffix (,X ,Y ,S)
    std::string hex_str = trimmed;
    size_t comma_pos = hex_str.find(',');
    if (comma_pos != std::string::npos) {
        hex_str = hex_str.substr(0, comma_pos);
    }
    uint32_t value = ParseHex(hex_str);  // ❌ Fails on "$c0+4"
    return std::make_shared<LiteralExpr>(value);
}
```

The function assumes any string starting with '$' is a simple hex literal and immediately calls `ParseHex()`, which throws an error on the '+' character.

### Desired State

ParseExpression should support arithmetic expressions:
- `$c0+4` → evaluates to $C4 (196 decimal)
- `10*20` → evaluates to 200
- `$900-*` → evaluates to ($900 - current_address)
- `label+10` → evaluates to (label_value + 10)

All 3 blocked POP files should assemble successfully:
- FRAMEDEF.S ✅
- TABLES.S ✅
- UNPACK.S ✅

Total assembly success: 29/29 (100%)

---

## Success Criteria

```
✓ ParseExpression handles expressions with +, -, *, / operators
✓ FRAMEDEF.S assembles without errors
✓ TABLES.S assembles without errors
✓ UNPACK.S assembles without errors
✓ All existing tests still pass
✓ Binary compatibility maintained (14/14 reference files)
✓ Assembly success rate: 29/29 (100%)
```

---

## Acceptance Criteria

### Functional Requirements
```
□ ParseExpression detects operators in input string
□ Expressions are parsed into AST (Abstract Syntax Tree)
□ Binary operators supported: +, -, *, /
□ Operator precedence: * / before + -
□ Mixed operand types: hex+decimal, symbol+literal, etc.
□ Expression evaluation returns correct results
□ Error handling for malformed expressions
```

### Quality Requirements
```
□ All existing tests passing
□ New unit tests for expression parsing
□ Code coverage maintained ≥80%
□ No regressions in binary compatibility
□ No new linting errors
□ Code follows existing patterns
```

### Non-Functional Requirements
```
□ Performance acceptable (negligible impact on assembly time)
□ Error messages clear and helpful
□ Code maintainable and well-documented
```

---

## Constraints and Dependencies

### Constraints
```
□ Must maintain backward compatibility with simple literals
□ Must not break existing symbol resolution
□ Must work within multi-pass assembly architecture
□ Cannot change Expression interface (used throughout codebase)
```

### Dependencies
```
□ Requires Expression class (include/xasm++/core/expression.h)
□ Depends on LiteralExpr, SymbolExpr classes
□ May need new BinaryOpExpr class for operators
□ Requires ConcreteSymbolTable for symbol lookup during evaluation
```

### Out of Scope
```
✗ Unary operators (-, +, ~, !)
✗ Parentheses for precedence override
✗ Bitwise operators (&, |, ^, <<, >>)
✗ Comparison operators (<, >, ==, !=)
✗ Complex expressions with multiple nested operations
```

**Note:** Keep scope minimal. Support only basic binary operators needed for POP files.

---

## Estimated Complexity

**Complexity:** Medium

**Rationale:**
- Number of files affected: 2 (assembler.cpp, expression.h)
- Lines of code estimate: ~100 lines (parser + new expression class)
- New concepts: Expression tree parsing with operators
- Integration complexity: Medium (fits into existing Expression framework)
- Risk level: Medium (affects core expression evaluation)

---

## Lean Flow Analysis (MANDATORY)

### Batch Size Assessment

**Estimated Files:** 2-3 files
- src/core/assembler.cpp (ParseExpression)
- include/xasm++/core/expression.h (BinaryOpExpr class)
- tests/unit/test_expression.cpp (new tests)

**Batch Size Evaluation:**
```
File Count Assessment:
├─ 1-5 files   → ✅ IDEAL: Small batch, proceed
├─ 6-14 files  → ⚠️ ACCEPTABLE: Document decomposition consideration
├─ 15-26 files → ❌ TOO LARGE: MUST decompose into 2-3 task packets
└─ 27+ files   → ❌ CRITICAL: MUST decompose into 3+ task packets

Your Task: 2-3 files → ✅ IDEAL
```

### Token Budget Estimation

**Conservative Estimate:**
```
Files × Average Tokens Per File = Estimated Total
3 × 2,000 tokens = 6,000 tokens

Agent Output Limit: 25K-32K tokens

Status:
├─ <20K tokens → ✅ SAFE
├─ 20-25K tokens → ⚠️ APPROACHING LIMIT
├─ 25-42K tokens → ❌ HIGH RISK (40% failure probability)
└─ >42K tokens → ❌ GUARANTEED FAILURE

Your Task: 6,000 tokens → ✅ SAFE
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
- [X] YES - Batch size ≤14 files AND token budget ≤42K AND WIP ≤3
- [ ] NO - MUST decompose (batch >14 files OR tokens >42K OR WIP >3)

---

## Resources and References

### Relevant Files
```
- src/core/assembler.cpp - ParseExpression function (lines 25-81)
- include/xasm++/core/expression.h - Expression interface
- tests/unit/test_expression.cpp - Expression tests

Test files:
- FRAMEDEF.S (line 23): db $01,0,1,0,$c0+4
- TABLES.S: Similar expressions
- UNPACK.S: Similar expressions
```

### Examples
```
Merlin expressions:
  db $c0+4    → 196 decimal ($C4)
  db $900-*   → ($900 - current_address)
  dw label-1  → (label_address - 1)
  db 10*20    → 200 decimal
```

---

## Assumptions

```
1. Operator precedence follows standard math rules (* / before + -)
2. Left-to-right evaluation for same precedence
3. No spaces required around operators ($c0+4 same as $c0 + 4)
4. Symbol values available via ConcreteSymbolTable during evaluation
```

---

## Risk Assessment

### Identified Risks
```
1. Breaking existing expression evaluation
   - Probability: Low
   - Impact: High
   - Mitigation: Comprehensive unit tests, TDD approach

2. Precedence bugs (wrong order of operations)
   - Probability: Medium
   - Impact: Medium
   - Mitigation: Test cases for precedence: 2+3*4 should = 14 not 20

3. Symbol resolution in expressions
   - Probability: Low
   - Impact: Medium
   - Mitigation: Use existing SymbolExpr evaluation logic
```

---

## Approvals and Sign-Off

**Contract Approved By:**
- [X] Requestor: Bryan Woodruff (2026-02-01)
- [ ] Agent: Engineer [Date]

---

## Notes

This task completes the Merlin assembler implementation for Prince of Persia source files.
After this fix, we should achieve 29/29 (100%) assembly success.

**Related tasks:**
- xasm++-8uc (P0): stoul conversion - ✅ FIXED
- xasm++-g3g (P1): DS program counter - ✅ FIXED
- xasm++-ly2 (P2): DO/FIN tracking - ✅ FIXED
- xasm++-4ay: Missing directives - ✅ FIXED
- xasm++-7f4: XC CPU mode - ✅ FIXED

**Current status:** 26/29 (89.7%)
**Target status:** 29/29 (100%)

---

**Contract Version:** 1.0
**Last Updated:** 2026-02-01
