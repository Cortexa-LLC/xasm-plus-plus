; Test 2: Labels and jumps
; Tests: JMP absolute addressing, labels
; Note: Symbol resolution not yet implemented, using hex addresses
    .org $8000
start:
    JMP $8007       ; Jump forward (to forward label address)
backward:
    NOP
    RTS
forward:
    JMP $8003       ; Jump backward (to backward label address)
