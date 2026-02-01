; 65816-specific opcodes test
; Tests instructions added in 65816

        ORG $3000

; Processor mode control
        CLC
        XCE             ; Exchange carry and emulation
        REP #$30        ; Reset processor status bits
        SEP #$30        ; Set processor status bits

; Block moves
        MVN $01,$02     ; Move negative
        MVP $03,$04     ; Move positive

; Long addressing (24-bit)
        LDA $123456     ; Long absolute
        STA $FEDCBA

; Stack operations
        PEA $1234       ; Push effective address
        PEI ($80)       ; Push effective indirect
        PER label       ; Push effective relative

label   NOP

; Other 65816 instructions
        TCD             ; Transfer C to D
        TCS             ; Transfer C to S
        TDC             ; Transfer D to C
        TSC             ; Transfer S to C
        TXY             ; Transfer X to Y
        TYX             ; Transfer Y to X

        RTL             ; Return long
