* Integration test for XC directive
* Tests CPU mode switching between 6502 and 65C02
*
* Expected behavior:
* - XC enables 65C02 mode (TSB, TRB, PHY, etc. available)
* - XC OFF disables 65C02 mode (back to 6502)
* - Mode can be toggled multiple times
* - Instructions only work in appropriate mode

 org $2000

* Test 1: Enable 65C02 mode and use 65C02 instructions
 xc
 tsb $42     ; Zero Page TSB (65C02)
 trb $43     ; Zero Page TRB (65C02)
 phy         ; Push Y (65C02)
 ply         ; Pull Y (65C02)
 phx         ; Push X (65C02)
 plx         ; Pull X (65C02)
 stz $44     ; Store Zero (65C02)

* Test 2: Disable 65C02 mode and use 6502 instructions
 xc off
 lda #$00    ; 6502 instruction
 sta $45     ; 6502 instruction
 ldx #$FF    ; 6502 instruction
 stx $46     ; 6502 instruction

* Test 3: Re-enable 65C02 mode
 xc
 tsb $1234   ; Absolute TSB (65C02)
 trb $1235   ; Absolute TRB (65C02)
 bra skip    ; BRA (65C02) - unconditional branch
 nop         ; This should be skipped
 nop
 nop
skip
 stz $47     ; Store Zero (65C02)
