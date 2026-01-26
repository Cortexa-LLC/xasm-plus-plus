 tr on
 lst off
* Prince of Persia style equates test
*-------------------------------
*
*  Equates
*
*-------------------------------
*  Main memory locations

rw18 = $d000
peelbuf1 = $d000
peelbuf2 = $d800
hrtables = $e000
unpack = $ea00
hires = $ee00
master = $f880

*  Aux memory

grafix = $400
tables = $e00
frameadv = $1290
redbufs = $5e00

*-------------------------------
*
*  Jump table structures
*
*-------------------------------
 dum master

_firstboot ds 3
_loadlevel ds 3
_reload ds 3
_loadstage2 ds 3
 ds 3

_attractmode ds 3
_cutprincess ds 3
_savegame ds 3
_loadgame ds 3

 dend
