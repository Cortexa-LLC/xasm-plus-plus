* Test: Data Directives
* Syntax: Merlin
* CPU: 6502
*
* Tests data definition directives:
* - DFB (Define Byte)
* - DA (Define Address)
* - ASC (ASCII String)
* - DCI (DCI String)
* - INV (Inverse ASCII)
* - FLS (Flash ASCII)

         ORG   $8000

* Single byte definitions
BYTE1    DFB   $42           ; Single byte
BYTE2    DFB   $00,$01,$02   ; Multiple bytes
BYTE3    DFB   $FF

* Word definitions (little-endian)
WORD1    DA    $1234         ; Word (34 12)
WORD2    DA    $ABCD,$5678   ; Multiple words

* ASCII strings
STR1     ASC   'HELLO'       ; Plain ASCII
STR2     ASC   'A'           ; Single character
STR3     ASC   'TEST123'     ; Mixed alphanumeric

* DCI strings (last char with high bit set)
DCI1     DCI   'APPLE'       ; Last char | $80
DCI2     DCI   'II'          ; Two chars, last inverted

* Inverse ASCII (all chars with high bit set)
INV1     INV   'INVERSE'     ; All chars | $80

* Flash ASCII (all chars with high bit clear)
FLS1     FLS   'FLASH'       ; All chars & $7F

* Mixed data
MIXED    DFB   $00
         ASC   'HI'
         DFB   $FF
         DA    $1234

         END
