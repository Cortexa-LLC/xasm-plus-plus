* Test: Branch Relaxation - Start Short Algorithm
* Syntax: merlin
* CPU: 6502
* Flags: --relax-branches
*
* Bug: Branch relaxation incorrectly used "start long, shrink" algorithm
*      which inflated addresses in pass 1, causing branches that could
*      fit SHORT to appear out of range and get unnecessarily expanded.
*
* Fix: Changed to "start short, expand only when necessary" algorithm
*      which produces minimal expansions.
*
* Classic counterexample: 3 branches to same forward target
*   - Start-long inflates all → all appear long → final = 9 bytes (wrong)
*   - Start-short tries all short → all fit → final = 6 bytes (correct)
*
* This test validates the minimal expansion behavior.

         ORG   $8000

* Test 1: Three sequential short branches to same forward target
* All three should stay SHORT (2 bytes each) = 6 bytes total
* Start-long incorrectly expands to 9 bytes

START    BCC   TARGET        ; Should be SHORT (2 bytes)
         BCS   TARGET        ; Should be SHORT (2 bytes)
         BEQ   TARGET        ; Should be SHORT (2 bytes)
         NOP                 ; Padding
         NOP
         NOP
TARGET   RTS

* Test 2: Branch just within range (+127 bytes)
* Should stay SHORT

NEAR     BCC   FAR
         DS    124           ; Exactly at limit
FAR      RTS

* Test 3: Branch just out of range (+128 bytes)
* Should expand to B!cc+3/JMP sequence (5 bytes)

EDGE     BCC   DISTANT
         DS    125           ; Forces expansion
DISTANT  RTS

* Test 4: Multiple branches with mixed distances
* Only the out-of-range branch should expand

MIX1     BCS   MIX_NEAR      ; SHORT
         BEQ   MIX_FAR       ; LONG (expanded)
         BCC   MIX_NEAR      ; SHORT
         DS    130
MIX_FAR  NOP
MIX_NEAR RTS

* Test 5: Backward branches (negative offsets)
* These should also use start-short

BACK     NOP
         NOP
         NOP
         BCC   BACK          ; SHORT backward
         BCS   BACK          ; SHORT backward
         RTS

         END
