# Work Log: Underscore Symbol Parsing Investigation

## Investigation Summary

**Root Cause Found:** `ScmasmSyntaxParser::ParseLabel()` does not allow labels to start with underscore, causing symbols like `_WRITE` to be misinterpreted as instruction mnemonics.

## Evidence

### 1. The Failure Pattern

Error from assembler:
```
A2osX.S.txt:18341: Invalid argument for _WRITE: Unsupported instruction: _WRITE
```

This error occurs when:
- A line defines a label starting with underscore: `_WRITE   .EQ  $1C`
- The label is NOT recognized as a label
- `_WRITE` is parsed as the opcode/mnemonic instead
- `.EQ` is parsed as the operand
- An `InstructionAtom` is created with mnemonic `_WRITE`
- CPU encoding fails because `_WRITE` is not a valid 6502 instruction

### 2. Root Cause: Inconsistent Identifier Rules

**Location:** `src/syntax/scmasm_syntax.cpp:642`

```cpp
// Labels must start with letter, ., or :
if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '.' && line[pos] != ':')) {
  return "";
}
```

**Problem:** This check does NOT allow underscore (`_`) as a starting character for labels.

**Comparison:** `ExpressionParser::ParseIdentifier()` at `src/common/expression_parser.cpp:613`

```cpp
// Identifier starts with letter, underscore, period, $, or ?
if (!std::isalpha(Peek()) && Peek() != '_' && Peek() != '.' &&
    Peek() != '$' && Peek() != '?') {
  throw std::runtime_error("Expected identifier");
}
```

**This DOES allow underscore as a starting character.**

### 3. Why Expressions Work but Labels Don't

When `_WRITE` appears in an expression like `LDA #<_Write`:
- The operand is parsed by `ExpressionParser`
- `ParseIdentifier()` correctly accepts `_Write` as a valid identifier
- It creates a `SymbolExpr` that looks up the symbol
- ✅ **This works**

When `_WRITE` appears as a label: `_WRITE   .EQ  $1C`:
- `ParseLabel()` checks if line starts with valid label character
- Underscore is NOT allowed
- Returns empty string (no label)
- `_WRITE` is then parsed as the opcode
- ❌ **This fails**

### 4. The Flow

```
Input line: "_WRITE   .EQ  $1C"
          ↓
ParseLabel() at line 642
          ↓
Check: isalpha('_') → false
       '_' == '.' → false  
       '_' == ':' → false
          ↓
Return "" (empty - not a label!)
          ↓
Parse "_WRITE" as opcode
          ↓
Parse ".EQ" as operand
          ↓
Create InstructionAtom(mnemonic="_WRITE", operand=".EQ")
          ↓
Encoding phase: cpu_->EncodeInstruction("_WRITE", ...)
          ↓
"Unsupported instruction: _WRITE" ❌
```

## Recommendation

**Fix:** Modify `src/syntax/scmasm_syntax.cpp:642` to allow underscore as a label starting character.

Change from:
```cpp
if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '.' && line[pos] != ':')) {
  return "";
}
```

To:
```cpp
if (pos >= line.length() || (!std::isalpha(line[pos]) && line[pos] != '_' && line[pos] != '.' && line[pos] != ':')) {
  return "";
}
```

This aligns the label parsing with identifier parsing rules used in expressions.

## Files Investigated

- `src/syntax/scmasm_syntax.cpp` - Label parsing logic (ROOT CAUSE at line 642)
- `src/common/expression_parser.cpp` - Identifier parsing (correct implementation at line 613)
- `src/assembler.cpp` - Error reporting location (line 490)
- `src/cpu/cpu_6502.cpp` - Instruction encoding (fails for unknown mnemonics)

## Status

✅ Root cause identified
✅ Fix location determined  
✅ Evidence documented
⏭️  Ready for engineer to implement fix
