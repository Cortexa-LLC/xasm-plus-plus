* Test: Data Directives
* Syntax: SCMASM
* CPU: 6502
*
        .OR $8000
*
* Single bytes
BYTE1   .DA $42
BYTE2   .DA $FF
*
* Multiple bytes
BYTES   .DA $01,$02,$03,$04,$05
*
* Words (16-bit values, little-endian, use .HS)
WORD1   .HS 3412
WORD2   .HS CDAB
*
* ASCII string
TEXT    .AS "HELLO"
*
* Hex bytes
HEXDATA .HS 0A0B0C0D0E
