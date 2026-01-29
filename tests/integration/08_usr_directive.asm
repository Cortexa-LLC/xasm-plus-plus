* Test USR directive (external subroutine call)
* USR should translate to JSR instruction

         ORG $8000

* USR with hex address
START    USR $C000

* USR with decimal address  
         USR 49152

* USR with label
MYLOOP   NOP
         USR MYLOOP

* Label with USR
CALLSUB  USR $D000
