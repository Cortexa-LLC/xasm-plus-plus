* Test: 6502 Addressing Modes
* Syntax: Merlin
* CPU: 6502
*
* Tests all addressing modes supported by 6502:
* - Immediate
* - Zero Page
* - Zero Page,X
* - Zero Page,Y
* - Absolute
* - Absolute,X
* - Absolute,Y
* - (Indirect,X)
* - (Indirect),Y
* - Indirect (JMP only)

         ORG   $8000

* Immediate addressing
START    LDA   #$42          ; Load immediate value
         LDX   #$10
         LDY   #$20
         CMP   #$42
         CPX   #$10
         CPY   #$20

* Zero page addressing
         LDA   $80           ; Load from zero page
         STA   $81           ; Store to zero page
         LDX   $82
         STX   $83
         LDY   $84
         STY   $85

* Zero page,X addressing
         LDA   $80,X         ; Load with X index
         STA   $81,X
         LDY   $82,X
         STY   $83,X

* Zero page,Y addressing
         LDX   $80,Y         ; Load with Y index (LDX/STX only)
         STX   $81,Y

* Absolute addressing
         LDA   $2000         ; Load from absolute address
         STA   $2001
         LDX   $2002
         STX   $2003
         LDY   $2004
         STY   $2005

* Absolute,X addressing
         LDA   $2000,X       ; Load with X index
         STA   $2001,X
         LDY   $2002,X       ; LDY supports abs,X (not STY)

* Absolute,Y addressing
         LDA   $2000,Y       ; Load with Y index
         STA   $2001,Y
         LDX   $2002,Y

* (Indirect,X) - indexed indirect
         LDA   ($80,X)       ; Pre-indexed indirect
         STA   ($82,X)

* (Indirect),Y - indirect indexed
         LDA   ($80),Y       ; Post-indexed indirect
         STA   ($82),Y

* Indirect (JMP only)
         JMP   ($2000)       ; Indirect jump

         END
