* Test: 65816 Specific Instructions
* Syntax: SCMASM
* CPU: 65816
*
        .OR $8000
*
* REP/SEP - Reset/Set Processor Status
        REP #$30
        SEP #$30
*
* XBA - Exchange B and A
        XBA
*
* PHA/PLA - Push/Pull 16-bit accumulator
        PHA
        PLA
*
* MVN/MVP - Block Move
        MVN $01,$02
        MVP $01,$02
*
* PEA - Push Effective Absolute
        PEA $1234
*
* PER - Push Effective PC Relative
        PER $8020
*
* PHB/PLB - Push/Pull Data Bank
        PHB
        PLB
*
* PHD/PLD - Push/Pull Direct Page
        PHD
        PLD
*
* PHK - Push Program Bank
        PHK
*
* TCD/TDC - Transfer C to D / D to C
        TCD
        TDC
*
* TCS/TSC - Transfer C to S / S to C
        TCS
        TSC
*
* TXY/TYX - Transfer X to Y / Y to X
        TXY
        TYX
*
* WDM - Reserved for future expansion
        WDM #$42
*
        RTS
