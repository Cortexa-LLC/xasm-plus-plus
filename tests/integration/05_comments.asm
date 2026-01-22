; Test 5: Comments
; Tests: Full-line comments, inline comments
; This is a full-line comment
    .org $8000
    LDA #$42    ; Inline comment
; Another full-line comment
    NOP         ; Another inline comment
