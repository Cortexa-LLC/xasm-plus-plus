* Test .EQ directive
        .OP 65C02
        
VALUE   .EQ $42
        
        .OR $2000
        
START   LDA #VALUE
        RTS
