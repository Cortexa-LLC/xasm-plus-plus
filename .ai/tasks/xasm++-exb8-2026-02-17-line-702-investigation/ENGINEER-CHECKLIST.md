# Engineer Checklist: Fix Star Operator Bug

## Pre-Implementation

- [ ] Read `investigation-report.md` for full context
- [ ] Review `fix-diagram.txt` for visual understanding
- [ ] Understand root cause (see `FINDINGS.md`)
- [ ] Check out clean branch from main

## Implementation

- [ ] Open `src/syntax/scmasm_syntax.cpp`
- [ ] Locate `EvaluateExpression()` function (line ~821)
- [ ] Find existing `*` check (line ~826-828)
- [ ] Add new code after line 828 (see fix-diagram.txt)
- [ ] Verify code compiles without warnings

## Testing

- [ ] Create test file:
  ```assembly
  .OR $8000
  TEST1 .EQ *       ; Should be $8000
  TEST2 .EQ *+4     ; Should be $8004
  TEST3 .EQ *-2     ; Should be $7FFE
  TEST4 .EQ *+$10   ; Should be $8010
  TEST5 .EQ *&$FF   ; Should be $00
  ```

- [ ] Run: `./build/bin/xasm++ --syntax scmasm test.s`
- [ ] Verify: No errors
- [ ] Check: Symbol table shows correct values

- [ ] Test A2osX.S.txt:
  ```bash
  ./build/bin/xasm++ --syntax scmasm \
    ~/Projects/Vintage/Apple/A2osX/A2osX.S.txt \
    -o A2osX.bin
  ```
- [ ] Verify: Assembles past line 702
- [ ] Check: No new errors

## Edge Cases

Test these should still work:

- [ ] `TEST .EQ *` (star alone)
- [ ] `.OR *` (star in .OR directive)
- [ ] `LDA #<*` (star with size operator)
- [ ] `JMP *+5` (star in branch offset)

Test these should gracefully fail or work:

- [ ] `TEST .EQ **2` (multiply or star-star?)
- [ ] `TEST .EQ 2**+4` (star in middle)

## Code Review

- [ ] Code is readable and commented
- [ ] Follows existing code style
- [ ] No magic numbers
- [ ] Error messages are clear
- [ ] No memory leaks
- [ ] No undefined behavior

## Documentation

- [ ] Update CHANGELOG.md with bug fix
- [ ] Add entry to known issues (if partial fix)
- [ ] Update user documentation if needed
- [ ] Add comment in code explaining the fix

## Regression Testing

- [ ] Run existing test suite
- [ ] All tests pass
- [ ] No new warnings
- [ ] No performance degradation

## Git

- [ ] Commit with clear message:
  ```
  Fix: Handle * operator in expressions like *+4
  
  - SCMASM * operator now works in expressions, not just alone
  - Fixes A2osX line 702 parsing error
  - Adds lookahead check before falling to ExpressionParser
  - Minimal scope change (5 lines in scmasm_syntax.cpp)
  
  Issue: xasm++-exb8-2026-02-17-line-702
  ```

- [ ] Push to feature branch
- [ ] Create PR with link to investigation docs

## Acceptance Criteria

✅ `.EQ *+4` assembles correctly  
✅ A2osX.S.txt assembles past line 702  
✅ No regression in existing functionality  
✅ Code is clean and documented  
✅ Tests pass  

## If Problems Occur

**Error: Still fails on *+4**
- Check: Did you rebuild after editing?
- Check: Are you testing the right binary?
- Check: Is there a syntax error in the new code?

**Error: Now * alone doesn't work**
- Check: Order of if statements (exact * must come first)
- Check: Did you accidentally modify existing * check?

**Error: Test suite failures**
- Check: Which tests fail?
- Check: Are they related to expression parsing?
- Review: Did the fix break other number formats?

**Need Help?**
- Read: `investigation-report.md` section on alternatives
- Review: `execution-trace.txt` to understand flow
- Consider: Option 1 or 2 from investigation report

## Done!

- [ ] All checklist items complete
- [ ] PR approved and merged
- [ ] Close investigation task
- [ ] Archive investigation docs
