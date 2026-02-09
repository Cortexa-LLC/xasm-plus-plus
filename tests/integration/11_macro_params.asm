; FLEX ASM Integration Test: Macro with Parameters
; Tests: Parameter substitution in macro expansion

        NAM     MacroParams
        TTL     Macro with Parameters Test

; Define a macro that moves byte from SRC to DEST
MOVB    MACRO   SRC,DEST
        LDA     SRC
        STA     DEST
        ENDM

; Define a macro that adds two numbers
ADDM    MACRO   VAL1,VAL2,RESULT
        LDA     VAL1
        ADDA    VAL2
        STA     RESULT
        ENDM

; Main program
        ORG     $1000

; Test MOVB macro with different arguments
START   MOVB    $80,$90         ; Should expand to: LDA $80 / STA $90
        MOVB    $A0,$B0         ; Should expand to: LDA $A0 / STA $B0
        
; Test ADDM macro with three parameters
        ADDM    $C0,$C1,$C2     ; Should expand to: LDA $C0 / ADDA $C1 / STA $C2
        
        END     START
