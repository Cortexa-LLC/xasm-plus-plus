* File: bug003_lo_repro.S
* Test case for BUG-003: Low byte operator (<)
        .OP 65C02
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #<MESSAGE    ; Get low byte of MESSAGE address
        RTS
