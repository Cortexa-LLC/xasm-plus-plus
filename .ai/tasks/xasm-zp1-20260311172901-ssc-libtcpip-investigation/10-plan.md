# Plan: SSC/libtcpip/libetalk investigation

## Workflow
bugfix.md (or research.md if all are source changes)

## Approach
1. Query KG first for both projects
2. For each file: diff analysis → classify as SYSCALL/source or assembler bug
3. If SYSCALL pattern: document and close
4. If new pattern: deep investigation + fix

## Execution Strategy
Parallel analysis across 4 files (pure read/compare, no build conflicts).
Fix if bug found: sequential build + test.
