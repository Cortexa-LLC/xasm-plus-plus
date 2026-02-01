; Basic 6502 operations test
; Tests basic addressing modes and common instructions

        ORG $1000

START   LDA #$42        ; Immediate
        STA $80         ; Zero page
        LDA $80         ; Zero page load
        STA $2000       ; Absolute
        LDA $2000       ; Absolute load
        
        LDX #$10        ; Load X immediate
        STX $81         ; Store X zero page
        
        LDY #$20        ; Load Y immediate
        STY $82         ; Store Y zero page
        
        CLC             ; Clear carry
        ADC #$01        ; Add with carry
        SEC             ; Set carry
        SBC #$01        ; Subtract with carry
        
        NOP             ; No operation
        RTS             ; Return from subroutine
