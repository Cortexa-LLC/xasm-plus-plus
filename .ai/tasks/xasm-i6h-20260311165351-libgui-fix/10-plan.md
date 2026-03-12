# Plan: lib/libgui -132B investigation

## Workflow
bugfix.md

## Approach
1. Generate assembly listing for LIBGUI.S.txt
2. Find first byte difference (python3 binary compare)
3. Map code offset to source line via listing
4. Identify bug pattern (branch relaxation, .DUMMY equate, macro, etc.)
5. If root cause found → implement fix → rebuild xasm++ → rebuild A2osX → verify

## Execution Strategy
Sequential (single file, discovery-driven).

## Key patterns to check
- Branch relaxation: grep listing for "JMP" where source has BRA/BCC/BNE/BEQ/BVC/BVS
- DUMMY equates: grep source for ".EQ *" inside .DUMMY blocks
- Macro commas: grep for ">MACRO label   ...text, more text" patterns

## Risks
- 132 bytes = up to 44 separate 3-byte JMP expansions; may be systemic
- May require multiple fix iterations before size converges
