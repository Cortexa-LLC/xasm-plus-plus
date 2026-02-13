* Test .BS directive - BUG-002 regression test
        .OP 65C02
        .OR $3000
        
* Test 1: Simple decimal count
BUFFER1 .BS 10          ; Reserve 10 bytes

* Test 2: Hex count
BUFFER2 .BS $100        ; Reserve 256 bytes

* Test 3: Expression with symbol
SIZE    .EQ 8
BUFFER3 .BS SIZE*4      ; Reserve 32 bytes (8*4)

END     NOP
