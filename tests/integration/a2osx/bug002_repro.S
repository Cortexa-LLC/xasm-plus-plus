* File: bug002_repro.S
* Test case for BUG-002: .BS directive should reserve bytes, not parse as binary
        .OP 65C02
        .OR $3000
        
BUFFER  .BS 256    ; Reserve 256 bytes (should work now!)
STACK   .BS $100   ; Reserve 256 bytes using hex
SMALL   .BS 10     ; Reserve 10 bytes
