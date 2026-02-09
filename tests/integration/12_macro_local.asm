; FLEX ASM Integration Test: Macro with Local Labels
; Tests: Local label uniquification (.LABEL -> .LABEL_001, .LABEL_002)

        NAM     MacroLocal
        TTL     Macro with Local Labels Test

; Define a macro with a local label
; Each invocation should get a unique .LOOP label
COPY    MACRO   COUNT
        LDX     #COUNT
.LOOP   LDA     ,X+             ; Local label
        STA     ,Y+
        DECB
        BNE     .LOOP           ; Branch to local label
        ENDM

; Define a macro with multiple local labels
DELAY   MACRO   OUTER,INNER
        LDX     #OUTER
.OUTER  LDY     #INNER
.INNER  LEAY    -1,Y
        BNE     .INNER          ; Inner loop
        LEAX    -1,X
        BNE     .OUTER          ; Outer loop
        ENDM

; Main program
        ORG     $1000

START   LDB     #10
        LDY     #$2000
        COPY    10              ; First invocation: .LOOP becomes .LOOP_001
        
        LDB     #20
        COPY    20              ; Second invocation: .LOOP becomes .LOOP_002
        
        DELAY   5,100           ; Multiple local labels: .OUTER_003, .INNER_003
        
        END     START
