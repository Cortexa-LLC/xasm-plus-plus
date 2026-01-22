; Test 2: Labels and jumps
; Tests: JMP absolute addressing, labels, forward/backward references
; Phase 2.1: Symbol resolution with multi-pass convergence
    .org $8000
start:
    JMP forward     ; Jump forward (forward reference)
backward:
    NOP
    RTS
forward:
    JMP backward    ; Jump backward (backward reference)
