; Test 1: Basic instructions
; Tests: LDA, STA, NOP, RTS
    .org $8000
start:
    LDA #$48    ; Load 'H'
    STA $0200   ; Store to memory
    NOP         ; No operation
    RTS         ; Return
