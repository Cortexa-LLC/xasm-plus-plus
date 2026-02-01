* Test: 65C02 Specific Instructions
* Syntax: SCMASM
* CPU: 65C02
*
        .OR $8000
*
* BRA - Branch Always
        BRA LABEL
        NOP
LABEL   BRA LABEL
*
* PHX/PLX - Push/Pull X
        PHX
        PLX
*
* PHY/PLY - Push/Pull Y
        PHY
        PLY
*
* STZ - Store Zero
        STZ $42
        STZ $1234
        STZ $42,X
        STZ $1234,X
*
* TRB/TSB - Test and Reset/Set Bits
        TRB $42
        TSB $42
        TRB $1234
        TSB $1234
*
* BIT immediate
        BIT #$FF
*
        RTS
