1000 * Test: Conditional Assembly
1010 * Syntax: SCMASM
1020 * CPU: 6502
1030 *
1040         .OR $8000
1050 *
1060 * Test conditional assembly
1070 DEBUG   .EQ 1
1080 *
1090 START   LDA #$00
1100         .DO DEBUG
1110         NOP
1120         NOP
1130         .FI
1140 *
1150         .DO DEBUG=0
1160         BRK
1170         .FI
1180 *
1190         RTS
