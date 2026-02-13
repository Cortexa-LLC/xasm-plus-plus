* Test offset expression LABEL+N
* Currently BLOCKED by bug: Expression not supported
        .OP 65C02
        
ZPPTR   .EQ $80
        
        .OR $2000
        
START   LDA #$12
        STA ZPPTR+1      ; Store to ZPPTR+1
        RTS
