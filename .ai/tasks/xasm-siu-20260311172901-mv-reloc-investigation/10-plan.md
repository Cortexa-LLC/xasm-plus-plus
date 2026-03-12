# Plan: bin/mv reloc-table investigation

## Workflow
bugfix.md

## Approach
1. Query KG (both xasm++ and A2osX) before reading source
2. Analyze relocation table: how many entries, are they consistently shifted?
3. If consistent shift of +2: find the 2-byte insertion point in code before reloc
4. Generate listing, map byte offset to source line
5. Identify bug pattern (DUMMY equate, string, macro, etc.)
6. Fix if assembler bug, document if source change

## Execution Strategy
Sequential. Single file, deep investigation.
