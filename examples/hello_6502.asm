; Simple 6502 program - Phase 1 demonstration
; Loads a value and stores it in memory

  .org $8000

start:
  lda #$42        ; Load accumulator with $42
  sta $0200       ; Store at address $0200
  ldx #$00        ; Initialize X register
  ldy #$10        ; Initialize Y register

loop:
  inx             ; Increment X
  iny             ; Increment Y
  cpx #$10        ; Compare X with $10
  bne loop        ; Branch if not equal
  
  jmp start       ; Jump back to start
  nop             ; No operation
  rts             ; Return from subroutine
