* Test: Basic 6502 Operations
* Syntax: SCMASM
* CPU: 6502
*
        .OR $8000
*
* Data transfer instructions
START   LDA #$42
        STA $80
        LDX #$10
        STX $81
        LDY #$20
        STY $82
*
* Register transfer
        TAX
        TXA
        TAY
        TYA
*
* Stack operations
        PHA
        PHP
        PLA
        PLP
*
* Simple control flow
        RTS
