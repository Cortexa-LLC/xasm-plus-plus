# Implementation Plan

**Task:** Fix multi-parameter macro substitution
**Workflow:** Bugfix (.ai-pack/workflows/bugfix.md)
**Role:** Engineer

---

## Investigation Phase

### Files to Examine

1. **`src/syntax/scmasm_syntax.cpp`** - InvokeMacro() method
   - How are parameters parsed from operand string?
   - How are ]N placeholders substituted?
   
2. **`include/xasm++/syntax/scmasm_syntax.h`** - MacroDefinition struct
   - How are parameters stored?

3. **Previous fix:** xasm++-f0hn fixed ]N syntax
   - Review what was fixed
   - Check if parameter splitting was implemented

### Root Cause Hypothesis

Based on debug output showing "1 params" when 2 were passed:
- Parameter splitting on commas may not be working
- Or parameters may be parsed but not all substituted

## Implementation Phase

### Step 1: Debug Parameter Parsing

Read InvokeMacro() to understand:
- How operand string is split into parameters
- Expected format: "TmpPtr1,X" → ["TmpPtr1", "X"]

### Step 2: Fix Parameter Splitting

Likely need to enhance parameter parsing:
```cpp
// Parse parameters from "TmpPtr1,X" format
std::vector<std::string> params;
// Split on commas, trim whitespace
```

### Step 3: Fix ]N Substitution

Ensure ALL ]N placeholders (]1-]9) are replaced:
```cpp
for (size_t i = 0; i < params.size(); ++i) {
    std::string placeholder = "]" + std::to_string(i + 1);
    // Replace all occurrences in macro body
}
```

### Step 4: Add Test

Create test in `tests/unit/test_scmasm_syntax.cpp`:
```cpp
TEST_F(ScmasmSyntaxTest, MacroMultipleParameters) {
    std::string source = R"(
        .OR $0800
        .MA STYA
            sta ]1
            sty ]2
        .EM
        >STYA TmpPtr1,X
    )";
    
    // Verify both parameters substituted
}
```

### Step 5: Validate A2osX

Test A2osX assembly:
```bash
xasm++ --cpu 65c02 --syntax scmasm ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt
```

Expected: Line 76+ assembles successfully

## Testing Strategy

1. **Unit test:** Multi-parameter macro
2. **Integration test:** A2osX macros (LDYAI, STYA, LDYA)
3. **Regression:** All existing tests pass

## Files to Modify

- `src/syntax/scmasm_syntax.cpp` (InvokeMacro method)
- `tests/unit/test_scmasm_syntax.cpp` (add test)

## Risks

- Low risk - focused bugfix
- Parameter splitting is well-understood logic

---

**Plan Version:** 1.0
**Last Updated:** 2026-02-16
