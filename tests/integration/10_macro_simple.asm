; FLEX ASM Integration Test: Simple Macro (No Parameters)
; Tests: MACRO/ENDM directives with basic macro expansion

        NAM     MacroSimple
        TTL     Simple Macro Test

; Define a simple macro that clears A and B registers
CLEAR   MACRO
        CLRA
        CLRB
        ENDM

; Main program
        ORG     $1000

START   LDX     #$2000          ; Load pointer
        CLEAR                   ; Invoke macro (should expand to CLRA + CLRB)
        STX     ,Y              ; Store result
        
        CLEAR                   ; Second invocation (tests multiple expansions)
        
        END     START
