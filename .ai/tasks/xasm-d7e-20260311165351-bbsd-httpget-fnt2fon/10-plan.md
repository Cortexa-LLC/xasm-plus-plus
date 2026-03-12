# Plan: bbsd/httpget/fnt2fon investigation

## Workflow
bugfix.md

## Approach
Investigate each file in order of delta size (bbsd first, then httpget, then fnt2fon).
For each:
1. Generate listing
2. Find first diff byte and map to source line
3. Identify pattern
4. If same root cause as a prior fix: note as duplicate
5. If new pattern: implement fix

## Execution Strategy
Sequential within this task; parallel with other investigation tasks.

## Risks
- Files may share the same root cause → fix one, others may self-resolve
- fnt2fon may use unusual directives (font data .HS blocks)
