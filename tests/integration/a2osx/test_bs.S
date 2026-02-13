* Test .BS directive (block storage)
* Currently BLOCKED by bug: .BS interprets as binary, not byte count
        .OP 65C02
        .OR $2000
        
START   NOP
        RTS

        .OR $3000
BUFFER  .BS 256    ; Should reserve 256 bytes
