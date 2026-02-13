* File: bug003_offset_repro.S
* Test case for BUG-003: Addition operator (+)
        .OP 65C02
        
ZPPTR   .EQ $80
        
        .OR $2000
        
START   LDA #$12
        STA ZPPTR+1      ; Store to ZPPTR+1 ($81)
        RTS
