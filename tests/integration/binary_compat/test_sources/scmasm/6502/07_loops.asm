1000 * Test: Loop Directives
1010 * Syntax: SCMASM
1020 * CPU: 6502
1030 *
1040         .OR $8000
1050 *
1060 * Test loop directive (.DO/.FI with counter)
1070 COUNT   .EQ 5
1080 *
1090 START   LDX #$00
1100         .DO COUNT
1110         NOP
1120         .FI
1130         RTS
