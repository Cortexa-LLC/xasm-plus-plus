* Test: Macro Parameter Count with conditional logic
* Syntax: SCMASM
* CPU: 6502
*
        .OR $8000
*
* Macro that reports parameter count using different code paths
        .MA PARAMTEST
        .DO ]#=0
        NOP              ; Zero parameters
        .FIN
        .DO ]#=1
        LDA #]1          ; One parameter
        .FIN
        .DO ]#=2
        LDA ]1           ; Two parameters
        STA ]2
        .FIN
        .DO ]#=3
        LDX ]1           ; Three parameters
        LDY ]2
        STX ]3
        .FIN
        .EM
*
* Test all cases
START   >PARAMTEST
        >PARAMTEST $42
        >PARAMTEST $80,$90
        >PARAMTEST $70,$71,$72
        RTS
