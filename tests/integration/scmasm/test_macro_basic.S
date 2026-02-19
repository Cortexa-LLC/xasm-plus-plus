* Test: Basic Macro Parameters
* Syntax: SCMASM
* CPU: 6502
*
* Test ]1-]9 parameter substitution
*
        .OR $8000
*
* Define simple macro
        .MA ADDVAL
        CLC
        ADC #]1
        .EM
*
* Define macro with 2 parameters
        .MA MOVE
        LDA ]1
        STA ]2
        .EM
*
* Test single parameter
START   LDA #$10
        >ADDVAL $05
*
* Test two parameters  
        >MOVE $80,$90
*
        RTS
