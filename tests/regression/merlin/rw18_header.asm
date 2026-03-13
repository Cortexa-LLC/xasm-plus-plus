* Test: --rw18 USR Header Generation
* Syntax: merlin
* CPU: 6502
* Flags: --rw18
*
* Feature: --rw18 flag prepends 12-byte RW18 header to binary output
*          for Merlin/Prince of Persia compatibility.
*
* Header format:
*   4 bytes: Magic "USR\x1a"
*   8 bytes: Four uint16_t arguments from USR directive (little-endian)
*
* If USR directive is not present, header uses zeros for arguments.
* This test validates the header structure when assembled with --rw18.
*
* Note: This test requires manual verification of binary output
*       or comparison with expected golden binary.

         ORG   $8000

* Define USR directive arguments
* These will populate the RW18 header when --rw18 flag is used
         USR   $0300,$8000,$0800,$0000

* Simple test program
START    LDA   #$42
         STA   $0300
         
         LDX   #$00
LOOP     INX
         CPX   #$10
         BNE   LOOP
         
         RTS

* Data section
DATA     HEX   0102030405

         END
