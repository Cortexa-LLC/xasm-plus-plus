* Test .HS directive (hex string)
        .OP 65C02
        .OR $2000
        
HEXDATA .HS 48656C6C6F    ; "Hello" in hex
        
START   NOP
        RTS
