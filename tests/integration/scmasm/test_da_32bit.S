* Test .DA directive with 24-bit and 32-bit values
* Verifies correct byte ordering for multi-byte data
        .OP 65C02
        .OR $2000
        
* Test 8-bit (low byte with #)
BYTE1   .DA #$12345678          ; Should emit: 78

* Test 8-bit (second byte with /)
BYTE2   .DA /$12345678          ; Should emit: 56

* Test 16-bit (default)
WORD1   .DA $12345678           ; Should emit: 78 56

* Test 24-bit (< prefix)
TRI1    .DA <$12345678          ; Should emit: 78 56 34

* Test 32-bit (> prefix)
DWORD1  .DA >$12345678          ; Should emit: 78 56 34 12

* Test with forward reference (32-bit)
        .DA >TARGET

TARGET  NOP
        RTS
