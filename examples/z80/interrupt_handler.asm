; ==============================================================================
; Z80 Interrupt Handler Example
; ==============================================================================
; Demonstrates interrupt mode 1 (IM 1) with timer-based interrupt handling.
; Suitable for TRS-80, ZX Spectrum, or other Z80 systems.
;
; In IM 1, interrupts jump to $0038 (RST 38H).
;
; Assemble with:
;   xasm++ --cpu z80 --syntax edtasm_m80_plusplus interrupt_handler.asm -o int.bin
;
; ==============================================================================

        TITLE 'Z80 Interrupt Handler Example'

; System equates
ROM_START EQU $0000
RAM_START EQU $8000
STACK_TOP EQU $FFFF

; ==============================================================================
; Interrupt Vector at $0038 (IM 1)
; ==============================================================================

        ORG   $38
        JP    ISR                ; Jump to interrupt service routine

; ==============================================================================
; Main Program
; ==============================================================================

        ORG   RAM_START

START:  LD    SP,STACK_TOP       ; Initialize stack pointer
        
        ; Initialize interrupt counter
        XOR   A
        LD    (TICK_COUNT),A
        LD    (TICK_COUNT+1),A
        
        ; Enable interrupts
        IM    1                  ; Interrupt mode 1
        EI                       ; Enable interrupts
        
MAIN_LOOP:
        ; Main program loop
        ; Check if tick count reached threshold
        LD    HL,(TICK_COUNT)
        LD    DE,100             ; Check for 100 ticks
        OR    A                  ; Clear carry
        SBC   HL,DE
        JR    C,MAIN_LOOP        ; Loop if less than 100
        
        ; Reset counter
        XOR   A
        LD    (TICK_COUNT),A
        LD    (TICK_COUNT+1),A
        
        ; Do something every 100 ticks
        CALL  DO_PERIODIC_TASK
        
        JR    MAIN_LOOP          ; Continue forever

; ==============================================================================
; Interrupt Service Routine
; ==============================================================================

ISR:    PUSH  AF                 ; Save all registers
        PUSH  BC
        PUSH  DE
        PUSH  HL
        
        ; Increment 16-bit tick counter
        LD    HL,(TICK_COUNT)
        INC   HL
        LD    (TICK_COUNT),HL
        
        ; Check for overflow (wrap at 65535)
        LD    A,H
        OR    L
        JR    NZ,.no_overflow
        
        ; Overflow - could set flag or call handler here
.no_overflow:
        
        POP   HL                 ; Restore registers
        POP   DE
        POP   BC
        POP   AF
        
        EI                       ; Re-enable interrupts
        RETI                     ; Return from interrupt

; ==============================================================================
; Periodic Task (called every 100 ticks)
; ==============================================================================

DO_PERIODIC_TASK:
        ; Put periodic task code here
        ; For example: blink LED, update display, etc.
        RET

; ==============================================================================
; Data
; ==============================================================================

TICK_COUNT:
        DW    0                  ; 16-bit tick counter

        END   START
