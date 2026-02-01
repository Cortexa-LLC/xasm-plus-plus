* Test: Macros
* Syntax: Merlin
* CPU: 6502
*
* Tests macro definition and expansion

         ORG   $8000

* Simple macro - no parameters
         MAC   INIT
         LDA   #$00
         TAX
         TAY
         <<<

* Macro with parameters
         MAC   STORE
         LDA   #]1
         STA   ]2
         <<<

* Macro with multiple parameters
         MAC   COPY
         LDA   ]1
         STA   ]2
         <<<

* Use the macros
START    INIT              ; Expand INIT macro

         STORE $42,$80    ; Store $42 to $80
         STORE $99,$81    ; Store $99 to $81

         COPY  $80,$2000  ; Copy $80 to $2000
         COPY  $81,$2001  ; Copy $81 to $2001

         RTS

         END
