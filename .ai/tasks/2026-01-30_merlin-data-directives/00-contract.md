# Task Contract: Implement Merlin Data Directives (DA/DCI/INV/FLS)

## Objective
Add missing Merlin assembler data directives to xasm++ to support legacy 6502 code assembly.

## Requirements
1. Implement DA (Data, Ascii) directive
2. Implement DCI (Data with last byte Control-character Inverted) directive
3. Implement INV (INVerted text - all bytes with high bit set) directive
4. Implement FLS (FLaSh text - alternating normal/inverted bytes) directive

## Success Criteria
- [ ] All directives parse correctly in Merlin syntax
- [ ] Binary output matches Merlin assembler behavior
- [ ] Integration test passes
- [ ] Code follows TDD approach
- [ ] Type hints and docstrings complete

## Context
The Merlin assembler was a popular Apple II development tool with unique text encoding directives for creating Apple II text displays. These directives are essential for assembling legacy 6502 code.

## References
- Merlin assembler documentation
- Test file: tests/integration/binary_compat/test_sources/merlin/6502/03_data_directives.asm
