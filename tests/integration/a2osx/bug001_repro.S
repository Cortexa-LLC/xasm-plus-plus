* File: bug001_repro.S
* Test case for BUG-001: Symbol resolution in addressing modes
        .OP 65C02
        
ZPTEMP  .EQ $80
        
        .OR $2000
        
START   LDA #ZPTEMP      ; ✅ Works - assembles as A9 80
        STA ZPTEMP       ; ❌ Fails - error: Unsupported instruction: ZPTEMP
        RTS
