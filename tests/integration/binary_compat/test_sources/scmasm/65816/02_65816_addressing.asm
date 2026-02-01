* Test: 65816 24-bit Addressing Modes
* Syntax: SCMASM
* CPU: 65816
*
        .OR $8000
*
* Stack Relative
        ADC $03,S
        AND $03,S
        CMP $03,S
        EOR $03,S
        LDA $03,S
        ORA $03,S
        SBC $03,S
        STA $03,S
*
* Stack Relative Indirect Indexed with Y
        ADC ($03,S),Y
        AND ($03,S),Y
        CMP ($03,S),Y
        EOR ($03,S),Y
        LDA ($03,S),Y
        ORA ($03,S),Y
        SBC ($03,S),Y
        STA ($03,S),Y
*
* Absolute Long (24-bit)
        ADC $123456
        AND $123456
        CMP $123456
        EOR $123456
        LDA $123456
        ORA $123456
        SBC $123456
        STA $123456
        JMP $123456
        JML $123456
*
* Absolute Long Indexed with X
        ADC $123456,X
        AND $123456,X
        CMP $123456,X
        EOR $123456,X
        LDA $123456,X
        ORA $123456,X
        SBC $123456,X
        STA $123456,X
*
* Direct Page Indirect Long [dp]
        ADC [$42]
        AND [$42]
        CMP [$42]
        EOR [$42]
        LDA [$42]
        ORA [$42]
        SBC [$42]
        STA [$42]
*
* Direct Page Indirect Long Indexed with Y [dp],Y
        ADC [$42],Y
        AND [$42],Y
        CMP [$42],Y
        EOR [$42],Y
        LDA [$42],Y
        ORA [$42],Y
        SBC [$42],Y
        STA [$42],Y
*
        RTS
