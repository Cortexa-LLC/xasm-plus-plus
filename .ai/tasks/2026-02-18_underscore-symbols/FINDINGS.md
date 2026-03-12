# Underscore Symbol Parsing - Investigation Complete

## Problem Statement
Why are underscore-prefixed symbols like `_WRITE`, `_STRLEN` being treated as instructions, causing 366 assembly errors?

## Root Cause (CONFIRMED)

**Location:** `src/syntax/scmasm_syntax.cpp:642`

The `ParseLabel()` function does not allow labels to start with underscore:

```cpp
// Labels must start with letter, ., or :
if (pos >= line.length() || 
    (!std::isalpha(line[pos]) && line[pos] != '.' && line[pos] != ':')) {
  return "";  // ← Rejects underscore-prefixed identifiers
}
```

## What Happens

1. Source line: `_WRITE   .EQ  $1C`
2. Parser checks first character: `_`
3. Not a letter, period, or colon → NOT a label
4. Parser interprets `_WRITE` as opcode, `.EQ` as operand
5. Creates instruction with mnemonic `_WRITE`
6. CPU encoding fails: "Unsupported instruction: _WRITE"

## Why Expressions Work But Labels Don't

| Context | Parser | Allows `_`? | Result |
|---------|--------|-------------|--------|
| In expression: `LDA #<_Write` | `ExpressionParser::ParseIdentifier()` | ✅ Yes (line 613) | Works |
| As label: `_WRITE .EQ $1C` | `ScmasmSyntaxParser::ParseLabel()` | ❌ No (line 642) | **FAILS** |

The inconsistency between these two parsers is the bug.

## The Fix (One Line)

**Add underscore to allowed first characters:**

```cpp
if (pos >= line.length() || 
    (!std::isalpha(line[pos]) && line[pos] != '_' &&  // ← ADD THIS
     line[pos] != '.' && line[pos] != ':')) {
  return "";
}
```

## Impact

- **Fixes:** All 366 errors in A2osX.S.txt
- **Risk:** Minimal - underscore already allowed in identifier continuation
- **Compatibility:** Aligns with SCMASM behavior and expression parser

## Verification

After applying fix, test:
```bash
cd ~/Projects/Apple/A2osX
make clean && make
```

Should complete successfully without underscore symbol errors.

---

**Investigation Complete - Ready for Implementation**
