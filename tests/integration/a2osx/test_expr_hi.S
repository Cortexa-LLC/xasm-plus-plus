* Test high byte expression #>LABEL
* Currently BLOCKED by bug: Expression not supported
        .OP 65C02
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #>MESSAGE    ; Load high byte of MESSAGE address
        RTS
