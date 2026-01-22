; Test 4: Origin directive
; Tests: .org changes address, multiple sections
    .org $8000
    LDA #$01
    .org $9000
    LDA #$02
