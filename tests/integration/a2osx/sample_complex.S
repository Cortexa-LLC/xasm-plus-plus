* A2oSX Sample - Complex
* Tests conditional assembly, listing control, advanced features

        .OP 65C02           ; Target 65C02 CPU
        
*--------------------------------------
* Configuration
*--------------------------------------
DEBUG   .EQ 1               ; Debug mode enabled
RELEASE .EQ 0               ; Release mode disabled

*--------------------------------------
* Zero Page Structure (DUMMY section)
*--------------------------------------
        .LIST OFF           ; Disable listing for structures
        .DUMMY
        .OR $80
ZP.BASE
ZP.SRC  .BS 2
ZP.DST  .BS 2
ZP.LEN  .BS 2
ZP.FLAGS .BS 1
        .EQ FLAG_DEBUG=$01
        .EQ FLAG_TRACE=$02
ZP.STRUCTSIZE .EQ *-ZP.BASE
        .ED
        .LIST ON

*--------------------------------------
* Macro Library
*--------------------------------------
        .MA PRINT           ; Print null-terminated string
        .DO DEBUG
        LDA #<]1
        STA ZP.SRC
        LDA #>]1
        STA ZP.SRC+1
        JSR PRINTSTR
        .FIN
        .EM

        .MA MEMCPY          ; Memory copy macro
        LDA #<]1            ; Source
        STA ZP.SRC
        LDA #>]1
        STA ZP.SRC+1
        LDA #<]2            ; Dest
        STA ZP.DST
        LDA #>]2
        STA ZP.DST+1
        LDA #<]3            ; Length
        STA ZP.LEN
        LDA #>]3
        STA ZP.LEN+1
        JSR COPYMEM
        .EM

*--------------------------------------
* Main Program
*--------------------------------------
        .OR $2000

START   CLD
        SEI                 ; Disable interrupts
        
        .DO DEBUG
        LDA #FLAG_DEBUG
        STA ZP.FLAGS
        PRINT DBG_MSG       ; Conditional debug print
        .FIN
        
        .LIST OFF           ; Don't list data copy
        MEMCPY INITDATA,BUFFER,$100
        .LIST ON
        
        CLI                 ; Enable interrupts
        
        .DO RELEASE
        JMP OPTIMIZED_PATH
        .ELSE
        JMP DEBUG_PATH
        .FIN

*--------------------------------------
* Debug Support (conditional)
*--------------------------------------
        .DO DEBUG

DEBUG_PATH
        LDA ZP.FLAGS
        AND #FLAG_DEBUG
        BEQ .1
        
        PRINT DBG_RUNNING
        
.1      JSR MAIN_LOGIC
        BRK                 ; Break for debugger

DBG_MSG     .PS "Debug Mode"
DBG_RUNNING .PS "Running..."

PRINTSTR
        LDY #$00
.LP     LDA (ZP.SRC),Y
        BEQ .EX
        JSR $FDED           ; Monitor COUT
        INY
        BNE .LP
.EX     RTS

        .FIN

*--------------------------------------
* Release Path (conditional)
*--------------------------------------
        .DO RELEASE
OPTIMIZED_PATH
        JSR MAIN_LOGIC
        RTS
        .FIN

*--------------------------------------
* Common Logic
*--------------------------------------
MAIN_LOGIC
        LDX #$00
.LP     LDA INITDATA,X
        STA BUFFER,X
        INX
        CPX #$20
        BNE .LP
        RTS

COPYMEM
        LDY #$00
.LP     LDA (ZP.SRC),Y
        STA (ZP.DST),Y
        INC ZP.SRC
        BNE .1
        INC ZP.SRC+1
.1      INC ZP.DST
        BNE .2
        INC ZP.DST+1
.2      LDA ZP.LEN
        BNE .3
        DEC ZP.LEN+1
.3      DEC ZP.LEN
        LDA ZP.LEN
        ORA ZP.LEN+1
        BNE .LP
        RTS

*--------------------------------------
* Data Section
*--------------------------------------
INITDATA
        .PS "A2oSX Operating System"
        .HS 0D0A00          ; CR, LF, NULL
        
PATHNAME .AZ "/A2OSX/BIN/SHELL"

LOOKUP_TABLE
        .DA #$00,#$01,#$02,#$03,#$04,#$05,#$06,#$07
        .DA #$08,#$09,#$0A,#$0B,#$0C,#$0D,#$0E,#$0F

*--------------------------------------
* Buffer Space
*--------------------------------------
        .OR $3000
BUFFER  .BS 256
