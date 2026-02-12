; ==============================================================================
; CP/M Hello World
; ==============================================================================
; Simple CP/M program that prints "Hello, World!" and returns to CP/M.
;
; Assemble with:
;   xasm++ --cpu z80 --syntax edtasm_m80_plusplus hello_cpm.asm -o hello.com
;
; Run under CP/M:
;   A>hello
;
; ==============================================================================

        TITLE 'CP/M Hello World Example'

; CP/M System Equates
BDOS    EQU   5                  ; BDOS entry point
WRMSG   EQU   9                  ; Write message function ($ terminated)

; Program starts at $100 (CP/M TPA)
        ORG   $100

START:  LD    DE,MESSAGE         ; Point to message
        LD    C,WRMSG            ; Function 9: Write string
        CALL  BDOS               ; Call BDOS
        RET                      ; Return to CP/M (exit)

MESSAGE:
        DB    'Hello, World!',13,10,'$'

        END   START
