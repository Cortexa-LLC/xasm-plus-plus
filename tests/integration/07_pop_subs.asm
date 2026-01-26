* Prince of Persia style subroutines test
DemoDisk = 0
EditorDisk = 0
CheckTimer = 0
org = $e000
 tr on
 lst off
*-------------------------------
*
*   S  U  B  S
*
*-------------------------------
 org org

 jmp ADDTORCHES
 jmp DOFLASHON
 jmp PAGEFLIP
 jmp DEMO
 jmp SHOWTIME

 jmp DOFLASHOFF
 jmp LRCLSE
 jmp potioneffect
 jmp checkalert
 jmp reflection

*-------------------------------
* Stub routines
*-------------------------------

ADDTORCHES rts
DOFLASHON rts
PAGEFLIP rts
DEMO rts
SHOWTIME rts
DOFLASHOFF rts
LRCLSE rts
potioneffect rts
checkalert rts
reflection rts
