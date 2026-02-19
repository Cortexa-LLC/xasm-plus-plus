* Test ]# parameter count feature
* ]# should expand to the number of parameters passed

        .OR $2000

* Macro that stores param count in zero page
STORE_COUNT .MA
        lda #]#         * Load parameter count
        sta $80         * Store to zero page
        .EM

* Test with 0 parameters
        >STORE_COUNT

* Test with 1 parameter  
        >STORE_COUNT $10

* Test with 2 parameters
        >STORE_COUNT $20,$30

* Test with 3 parameters
        >STORE_COUNT $40,$50,$60
