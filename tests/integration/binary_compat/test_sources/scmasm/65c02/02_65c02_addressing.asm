* Test: 65C02 New Addressing Modes
* Syntax: SCMASM
* CPU: 65C02
*
        .OR $8000
*
* Zero Page Indirect (zp)
        ADC ($42)
        AND ($42)
        CMP ($42)
        EOR ($42)
        LDA ($42)
        ORA ($42)
        SBC ($42)
        STA ($42)
*
* JMP Indirect with X-indexing
        JMP ($1234,X)
*
        RTS
