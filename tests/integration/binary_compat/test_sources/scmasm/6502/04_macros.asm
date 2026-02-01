1000 * Test: Macro Support
1010 * Syntax: SCMASM
1020 * CPU: 6502
1030 *
1040         .OR $8000
1050 *
1060 * Define a simple macro
1070         .MA ADDVAL
1080         CLC
1090         ADC #]1
1100         .EM
1110 *
1120 * Use the macro
1130 START   LDA #$10
1140         >ADDVAL $05
1150         >ADDVAL $0A
1160         RTS
