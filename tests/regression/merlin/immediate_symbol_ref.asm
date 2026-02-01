* Test: Immediate Mode with Symbol References
* Syntax: Merlin
* CPU: 6502
*
* Bug: Expression evaluator fails with "stoul: no conversion"
*      when immediate mode references symbols
*
* This pattern appears frequently in Prince of Persia source:
*   POPside1 = $a9
*   lda #POPside1

         ORG   $8000

* Define constants using EQU syntax
CONST1   =     $42
CONST2   =     $FF
OFFSET   =     $10

* Test immediate mode with symbol references
START    LDA   #CONST1       ; Should load $42
         LDX   #CONST2       ; Should load $FF
         LDY   #OFFSET       ; Should load $10
         
* Test with hex literals still work
         LDA   #$AA
         LDX   #$BB
         LDY   #$CC

* Test arithmetic expressions with symbols
         LDA   #CONST1+1     ; Should load $43
         LDX   #CONST2-1     ; Should load $FE
         
         RTS

         END
