* Test: Addressing Modes
* Syntax: SCMASM
* CPU: 6502
*
        .OR $8000
*
* Zero page
ZP      LDA $42
        STA $43
*
* Absolute
ABS     LDA $1234
        STA $5678
*
* Immediate
IMM     LDA #$AA
        LDX #$BB
        LDY #$CC
*
* Indexed
IDX     LDA $1000,X
        STA $2000,Y
        LDA $40,X
        LDY $50,X
*
* Indirect
IND     LDA ($40,X)
        STA ($50),Y
*
        RTS
