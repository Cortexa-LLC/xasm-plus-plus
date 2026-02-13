* A2oSX Sample - Simple
* Tests basic SCMASM directives used in A2oSX
* No includes, no complex features

        .OP 65C02           ; Target 65C02 CPU

* Zero Page Variables
ZP.TEMP     .EQ $80
ZP.PTR      .EQ $82

* Entry Point
        .OR $2000

START   LDA #$00
        STA ZP.TEMP
        
        LDA #<MESSAGE
        STA ZP.PTR
        LDA #>MESSAGE
        STA ZP.PTR+1

        RTS

* Data Section
MESSAGE .PS "Hello A2oSX"    ; Pascal string (length-prefixed)
BYTES   .DA #$01,#$02,#$03  ; Data bytes
HEXDATA .HS 48656C6C6F      ; Hex string "Hello"
TEXT    .AS "World"         ; ASCII string
ZTEXT   .AZ "Zero Term"     ; Zero-terminated

* Buffer space would use .BS but that directive has a bug
* .OR $3000
* BUFFER  .BS 256            ; Block storage (reserve 256 bytes)
