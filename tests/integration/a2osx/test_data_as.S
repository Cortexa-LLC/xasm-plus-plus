* Test .AS directive (ASCII string)
        .OP 65C02
        .OR $2000
        
MESSAGE .AS "Hello A2oSX"
        
START   NOP
        RTS
