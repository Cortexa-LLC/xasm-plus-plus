; Test 3: Data directives
; Tests: .db (data byte), .dw (data word, little-endian)
    .org $8000
message:
    .db $48, $45, $4C, $4C, $4F  ; "HELLO"
address:
    .dw $1234                     ; Little-endian: 34 12
