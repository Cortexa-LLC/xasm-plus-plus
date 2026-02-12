; ==============================================================================
; Z80 Data Tables and Lookup Example
; ==============================================================================
; Demonstrates various data table techniques:
; - Jump tables for command dispatch
; - Lookup tables for mathematical functions
; - String tables
; - Mixed data structures
;
; Assemble with:
;   xasm++ --cpu z80 --syntax edtasm_m80_plusplus data_tables.asm -o tables.bin
;
; ==============================================================================

        TITLE 'Z80 Data Tables Example'

; ==============================================================================
; Constants
; ==============================================================================

; Commands
CMD_HELP  EQU   0
CMD_LIST  EQU   1
CMD_RUN   EQU   2
CMD_QUIT  EQU   3
CMD_MAX   EQU   3                ; Highest command number

; ==============================================================================
; Main Program
; ==============================================================================

        ORG   $8000

START:  LD    SP,$FFFF           ; Initialize stack

        ; Test command dispatcher
        LD    A,CMD_HELP
        CALL  EXECUTE_COMMAND
        
        LD    A,CMD_LIST
        CALL  EXECUTE_COMMAND
        
        LD    A,CMD_RUN
        CALL  EXECUTE_COMMAND
        
        ; Test sine lookup
        LD    A,45               ; 45 degrees
        CALL  GET_SINE
        ; Result in A (scaled 0-255)
        
        ; Test string lookup
        LD    A,1                ; Get string 1
        CALL  GET_STRING
        ; Result: HL points to string
        
        RET                      ; Done

; ==============================================================================
; Command Dispatcher (Jump Table)
; ==============================================================================

EXECUTE_COMMAND:
        ; Input: A = command number (0-CMD_MAX)
        ; Validates command and jumps to handler
        
        ; Validate command number
        CP    CMD_MAX+1
        JR    NC,.invalid        ; Invalid if >= CMD_MAX+1
        
        ; Multiply by 2 (word size) and index into jump table
        ADD   A,A                ; A = A * 2
        LD    HL,JUMP_TABLE
        ADD   A,L                ; Add to low byte
        LD    L,A
        JR    NC,.no_carry
        INC   H                  ; Handle carry
.no_carry:
        
        ; Load handler address and jump
        LD    E,(HL)
        INC   HL
        LD    D,(HL)
        EX    DE,HL
        JP    (HL)               ; Jump to handler
        
.invalid:
        ; Invalid command
        RET

; Jump table (array of function pointers)
JUMP_TABLE:
        DW    CMD_HELP_HANDLER
        DW    CMD_LIST_HANDLER
        DW    CMD_RUN_HANDLER
        DW    CMD_QUIT_HANDLER

; ==============================================================================
; Command Handlers
; ==============================================================================

CMD_HELP_HANDLER:
        ; Display help
        LD    HL,MSG_HELP
        CALL  PRINT_STRING
        RET

CMD_LIST_HANDLER:
        ; List something
        LD    HL,MSG_LIST
        CALL  PRINT_STRING
        RET

CMD_RUN_HANDLER:
        ; Run something
        LD    HL,MSG_RUN
        CALL  PRINT_STRING
        RET

CMD_QUIT_HANDLER:
        ; Quit program
        LD    HL,MSG_QUIT
        CALL  PRINT_STRING
        ; Would halt or return to OS here
        RET

; ==============================================================================
; Sine Lookup Table (0-90 degrees)
; ==============================================================================

GET_SINE:
        ; Input: A = angle in degrees (0-90)
        ; Output: A = sine value (0-255 scale, where 255 = 1.0)
        
        ; Validate input
        CP    91
        JR    NC,.invalid
        
        ; Look up in table
        LD    HL,SINE_TABLE
        ADD   A,L
        LD    L,A
        JR    NC,.no_carry
        INC   H
.no_carry:
        LD    A,(HL)
        RET
        
.invalid:
        LD    A,0                ; Return 0 for invalid
        RET

; Sine table: sin(x) * 255, for x = 0 to 90 degrees
SINE_TABLE:
        DB    0,4,9,13,18,22,27,31,36,40       ; 0-9 degrees
        DB    44,49,53,57,62,66,70,75,79,83    ; 10-19
        DB    87,91,96,100,104,108,112,116,120,124 ; 20-29
        DB    128,132,136,139,143,147,150,154,158,161 ; 30-39
        DB    165,168,171,175,178,181,184,187,190,193 ; 40-49
        DB    196,198,201,204,206,209,211,213,216,218 ; 50-59
        DB    220,222,224,226,228,230,232,234,235,237 ; 60-69
        DB    238,240,241,243,244,245,246,247,248,249 ; 70-79
        DB    250,251,252,252,253,254,254,255,255,255,255 ; 80-90

; ==============================================================================
; String Table
; ==============================================================================

GET_STRING:
        ; Input: A = string index (0-n)
        ; Output: HL = pointer to string (null-terminated)
        
        ; Validate index
        CP    STRING_COUNT
        JR    NC,.invalid
        
        ; Multiply by 2 and index into pointer table
        ADD   A,A
        LD    HL,STRING_TABLE
        ADD   A,L
        LD    L,A
        JR    NC,.no_carry
        INC   H
.no_carry:
        
        ; Load string pointer
        LD    E,(HL)
        INC   HL
        LD    D,(HL)
        EX    DE,HL
        RET
        
.invalid:
        LD    HL,0               ; Return NULL for invalid
        RET

; String pointer table
STRING_TABLE:
        DW    STR_0
        DW    STR_1
        DW    STR_2
        DW    STR_3
STRING_COUNT EQU 4

; Actual strings (null-terminated)
STR_0:  DB    "First string",0
STR_1:  DB    "Second string",0
STR_2:  DB    "Third string",0
STR_3:  DB    "Fourth string",0

; ==============================================================================
; Mixed Data Structure Example
; ==============================================================================

; Structure: name (16 bytes), age (1 byte), score (2 bytes word)
RECORD_SIZE EQU 19

RECORDS:
        ; Record 1
        DB    "Alice",0,0,0,0,0,0,0,0,0,0,0  ; 16 bytes (padded)
        DB    25                              ; Age
        DW    1500                            ; Score (little-endian)
        
        ; Record 2
        DB    "Bob",0,0,0,0,0,0,0,0,0,0,0,0,0 ; 16 bytes (padded)
        DB    30                              ; Age
        DW    2100                            ; Score
        
        ; Record 3
        DB    "Charlie",0,0,0,0,0,0,0,0,0     ; 16 bytes (padded)
        DB    28                              ; Age
        DW    1800                            ; Score

RECORD_COUNT EQU 3

; ==============================================================================
; Helper Functions
; ==============================================================================

PRINT_STRING:
        ; Input: HL = pointer to null-terminated string
        ; Prints string (implementation depends on system)
        ; This is a stub - would call OS or output routine
        RET

; ==============================================================================
; Messages
; ==============================================================================

MSG_HELP:
        DB    "HELP command executed",13,10,0

MSG_LIST:
        DB    "LIST command executed",13,10,0

MSG_RUN:
        DB    "RUN command executed",13,10,0

MSG_QUIT:
        DB    "QUIT command executed",13,10,0

        END   START
