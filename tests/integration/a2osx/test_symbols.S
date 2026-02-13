* Test symbols - BUG-001 regression test
        .OP 65C02
        
ZPTEMP  .EQ $80
ZPPTR   .EQ $82
        
        .OR $2000
        
MESSAGE .AS "Hello"

START   LDA #$00        ; Immediate mode - 2 bytes: A9 00
        STA ZPTEMP      ; Zero Page mode - 2 bytes: 85 80 (BUG-001 FIX)
        
        LDA #<MESSAGE   ; Immediate mode - 2 bytes: A9 00
        STA ZPPTR       ; Zero Page mode - 2 bytes: 85 82 (BUG-001 FIX)
        LDA #>MESSAGE   ; Immediate mode - 2 bytes: A9 20
        STA ZPPTR+1     ; Zero Page mode - 2 bytes: 85 83 (expression+BUG-001 FIX)
        
        RTS             ; 1 byte: 60
