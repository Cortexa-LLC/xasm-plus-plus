# Plan: SYSCALL verification + x.fileenum batch

## Workflow
research.md (classification task; fix only if new bug found)

## Approach
1. Query KG first
2. Group A: run SYSCALL-pattern check on all files programmatically
3. Group B: check shared include, verify 2-byte source change
4. Document all findings in bug tracker
5. Fix if any new assembler bug found

## Execution Strategy
Mostly parallel (read-only comparisons). Sequential only if a bug fix is needed.
