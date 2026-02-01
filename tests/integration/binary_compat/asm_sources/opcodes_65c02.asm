; 65C02-specific opcodes test
; Tests instructions added in 65C02

        ORG $2000

; 65C02 new instructions
        BRA forward     ; Branch always
forward NOP

; Stack operations
        PHX             ; Push X
        PHY             ; Push Y
        PLX             ; Pull X
        PLY             ; Pull Y

; Zero page indirect
        STA ($80)       ; Zero page indirect
        LDA ($82)

; Bit operations
        TRB $80         ; Test and reset bits
        TSB $81         ; Test and set bits

; Misc
        STZ $90         ; Store zero
        STZ $1000       ; Store zero absolute

        RTS
