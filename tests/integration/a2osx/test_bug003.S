* BUG-003 Regression Test: Expression operators (<, >, +, -)
* Tests that expression operators work correctly
        .OP 65C02
        
ZPPTR   .EQ $80
        
        .OR $C0DE
MESSAGE .AS "Hello"

        .OR $3000
        
* Test low byte operator: <
START   LDA #<MESSAGE   ; Low byte of $C0DE = $DE
        CMP #$DE        ; Verify
        
* Test high byte operator: >
        LDA #>MESSAGE   ; High byte of $C0DE = $C0
        CMP #$C0        ; Verify
        
* Test addition: +
        LDA #$00
        STA ZPPTR+0     ; Store to $80
        STA ZPPTR+1     ; Store to $81
        STA ZPPTR+2     ; Store to $82
        
* Test subtraction: -
        LDA MESSAGE-1   ; Load from $C0DD
        
* Test complex expression: <(SYMBOL+offset)
        LDA #<(MESSAGE+$10)  ; Low byte of $C0EE = $EE
        
* Test complex expression: >(SYMBOL-offset)
        LDA #>(MESSAGE-$DE)  ; High byte of $C000 = $C0
        
        RTS
