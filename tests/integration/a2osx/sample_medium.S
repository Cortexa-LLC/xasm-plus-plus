* A2oSX Sample - Medium Complexity
* Tests DUMMY sections (structure definitions)
* Tests macros and more directives

        .OP 65C02           ; Target 65C02 CPU

*--------------------------------------
* Zero Page Structure Definition
*--------------------------------------
        .DUMMY              ; Start dummy section (no code emitted)
        .OR $80
ZP.BASE
ZP.PTR1 .BS 2               ; Pointer 1 (2 bytes)
ZP.PTR2 .BS 2               ; Pointer 2 (2 bytes)
ZP.TEMP .BS 1               ; Temporary byte
ZP.COUNT .BS 1              ; Counter
ZP.SIZE .EQ *-ZP.BASE       ; Size of structure
        .ED                 ; End dummy section

*--------------------------------------
* Macro Definitions
*--------------------------------------
        .MA SETPTR          ; Macro: Set pointer
        LDA #<]1
        STA ]2
        LDA #>]1
        STA ]2+1
        .EM

        .MA INC16           ; Macro: Increment 16-bit value
        INC ]1
        BNE *+4
        INC ]1+1
        .EM

*--------------------------------------
* Main Code
*--------------------------------------
        .OR $2000

INIT    CLD                 ; Clear decimal mode
        
        SETPTR BUFFER,ZP.PTR1    ; Use macro to set pointer
        
        LDY #$00
.1      LDA MESSAGE,Y       ; Copy message
        STA (ZP.PTR1),Y
        INY
        CPY #MESSAGE.LEN
        BNE .1

        INC16 ZP.PTR1       ; Use macro to increment pointer

        RTS

*--------------------------------------
* Data Section
*--------------------------------------
MESSAGE .PS "A2oSX OS"
MESSAGE.LEN .EQ *-MESSAGE-1

FILENAME .AS "KERNEL.SYS"
PATH     .AZ "/A2OSX/SYS"

BINTABLE .DA #$00,#$10,#$20,#$30
         .DA #$40,#$50,#$60,#$70

        .OR $3000
BUFFER  .BS 512             ; 512-byte buffer
