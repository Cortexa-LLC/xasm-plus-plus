* Test symbols
        .OP 65C02
        
ZPTEMP  .EQ $80
ZPPTR   .EQ $82
        
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #$00
        STA #ZPTEMP
        
        LDA #<MESSAGE
        STA #ZPPTR
        LDA #>MESSAGE
        STA #ZPPTR
        
        RTS
