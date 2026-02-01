; Basic 6502 opcodes test
; Tests common instructions across all CPU variants

        ORG $1000

; Load/Store operations
        LDA #$42        ; Immediate
        LDX #$10
        LDY #$20
        STA $80         ; Zero page
        STX $81
        STY $82

; Arithmetic
        ADC #$01
        SBC #$02
        INC $80
        DEC $80
        INX
        DEX
        INY
        DEY

; Logic
        AND #$0F
        ORA #$F0
        EOR #$FF

; Shifts
        ASL A
        LSR A
        ROL A
        ROR A

; Branches
loop    BCC loop
        BCS loop
        BEQ loop
        BNE loop
        BMI loop
        BPL loop
        BVC loop
        BVS loop

; Jumps
        JMP end
        JSR sub
end     RTS

sub     NOP
        RTS
