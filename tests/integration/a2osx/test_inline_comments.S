* Test inline comments after directives
* This test verifies that inline comments work correctly

        .OR $0800

* Test Case 1: .EQ with inline comment
pRWReg  .EQ 0    x.printf.s

* Test Case 2: .EQ with hex value and comment
IOADDR  .EQ $C000  IO Address Range

* Test Case 3: Multiple spaces before comment
VALUE   .EQ 42       This is a comment

* Test Case 4: Instruction with inline comment
        LDA #$00  Load zero into accumulator
        STA pRWReg  Store to pRWReg variable

* Test Case 5: .DA with inline comment
TABLE   .DA $01,$02,$03  Three data values

* Test Case 6: Comment with special characters
TEST    .EQ 123    x.y.z/a/b/c

* Test Case 7: .OR with inline comment
        .OR $1000    Start of new section

* Success marker
        NOP
