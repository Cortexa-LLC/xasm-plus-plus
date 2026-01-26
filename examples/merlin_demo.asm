* Merlin syntax demonstration
* Prince of Persia style assembly

         org   $8000

SCREEN   equ   $0400
COUNTER  =     $10

Start    lda   #$00
         sta   COUNTER
         ldx   #$FF
         
Loop     inx
         lda   SCREEN,x
         beq   Done
         jmp   Loop
         
Done     rts
