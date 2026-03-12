# Plan: useradd/asm.65816 investigation

## Workflow
bugfix.md

## Approach
1. useradd: find first diff → look for DUMMY equates or macro comma patterns
   - Check for any `.EQ *` inside `.DUMMY` blocks
   - Check for macro invocations with commas in comments
   - Enumerate all +3 blocks to count relaxations
2. asm.65816: check if 65816-specific directives are handled correctly in 65c02 mode
   - May need special handling for `MX`, `REP`, `SEP`, `XCE` etc.

## Execution Strategy
Sequential (useradd first as larger delta; asm.65816 second).

## Risks  
- useradd -48B may be a completely different bug not yet seen
- asm.65816 may require new CPU directive support
