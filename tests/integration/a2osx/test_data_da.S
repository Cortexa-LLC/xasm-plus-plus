* Test .DA directive (data bytes)
        .OP 65C02
        .OR $2000
        
BYTES   .DA #$01,#$02,#$03,#$04
        
START   NOP
        RTS
