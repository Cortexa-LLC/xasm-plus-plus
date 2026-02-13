* BUG-001 Regression Test: Symbol resolution in addressing modes
* Tests that symbols can be used in all 6502 addressing modes
        .OP 65C02
        
* Define zero page symbols
ZPTEMP  .EQ $80
ZPPTR   .EQ $82

* Define absolute address symbols
        .OR $2000
BUFFER  .BS 256
TABLE   .BS 16

        .OR $3000
        
* Test zero page addressing with symbols
START   LDA ZPTEMP      ; Zero page: AD 80
        STA ZPTEMP      ; Zero page: 85 80
        
* Test zero page indexed
        LDX #$00
        LDA ZPTEMP,X    ; Zero page,X: B5 80
        STA ZPTEMP,X    ; Zero page,X: 95 80
        
* Test absolute addressing with symbols
        LDA BUFFER      ; Absolute: AD 00 20
        STA BUFFER      ; Absolute: 8D 00 20
        
* Test absolute indexed
        LDA BUFFER,X    ; Absolute,X: BD 00 20
        STA BUFFER,X    ; Absolute,X: 9D 00 20
        
        LDA BUFFER,Y    ; Absolute,Y: B9 00 20
        STA BUFFER,Y    ; Absolute,Y: 99 00 20
        
* Test indirect indexed
        LDA (ZPPTR),Y   ; Indirect,Y: B1 82
        STA (ZPPTR),Y   ; Indirect,Y: 91 82
        
* Test indexed indirect
        LDA (ZPPTR,X)   ; Indexed indirect: A1 82
        STA (ZPPTR,X)   ; Indexed indirect: 81 82
        
        RTS
