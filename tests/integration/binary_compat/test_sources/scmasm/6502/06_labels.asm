1000 * Test: Label Handling
1010 * Syntax: SCMASM
1020 * CPU: 6502
1030 *
1040         .OR $8000
1050 *
1060 * Test various label formats
1070 START   LDA #$00
1080 LOOP    INX
1090         CPX #$10
1100         BNE LOOP
1110 *
1120 * Subroutines with labels
1130 SUB1    LDA #$01
1140 SUB1LP  DEX
1150         BNE SUB1LP
1160         RTS
1170 *
1180 SUB2    LDA #$02
1190 SUB2LP  DEY
1200         BNE SUB2LP
1210         RTS
