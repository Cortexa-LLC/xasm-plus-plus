* Test: Macro Parameter Count (]#)
* Syntax: SCMASM
* CPU: 6502
*
* Test ]# parameter count substitution
*
        .OR $8000
*
* Define macro that uses ]# to check parameter count
        .MA FLEX
        .DO ]#=2
        LDA ]1,]2      ; Two parameters: indexed mode
        .ELSE
        LDA ]1         ; One parameter: direct mode
        .FIN
        .EM
*
* Test with 1 parameter
START   >FLEX $80
*
* Test with 2 parameters
        >FLEX $80,X
*
        RTS
