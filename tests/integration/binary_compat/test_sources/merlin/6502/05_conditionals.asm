* Test: Conditional Assembly
* Syntax: Merlin
* CPU: 6502
*
* Tests conditional assembly directives

DEBUG    EQU   1                  ; Enable debug mode
RELEASE  EQU   0                  ; Disable release mode

         ORG   $8000

* Conditional block 1: DEBUG mode
         DO    DEBUG
         LDA   #$DE               ; Debug marker
         STA   $80
         FIN

* Conditional block 2: RELEASE mode
         DO    RELEASE
         LDA   #$RE               ; Release marker
         STA   $81
         FIN

* Common code (always assembled)
         RTS

         END
