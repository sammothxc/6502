PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

value = $0200           ; 2 bytes (16 bits)
mod10 = $0202           ; 2 bytes (16 bits)
message = $0204         ; 6 bytes
counter = $020a         ; 2 bytes

E = %10000000
RW = %01000000
RS = %00100000

    .org $8000          ; Program start address

reset:
    ldx #$ff
    txs
    cli

    lda #%11111111      ; Set all pins of PORTB to output
    sta DDRB
    lda #%11100000      ; Set 3 pins of PORTA to output
    sta DDRA

    lda #%00111000      ; Set 8-bit mode, 2-line display, 5x8 font
    jsr lcd_instruction
    lda #%00001110      ; Display on, cursor on, no blink
    jsr lcd_instruction
    lda #%00000110      ; Increment mode, shift cursor, don't shift display
    jsr lcd_instruction
    lda #%00000001      ; Clear display
    jsr lcd_instruction

    lda #0
    sta counter
    sta counter + 1

main_loop:
    lda #%00000010      ; Home cursor
    jsr lcd_instruction
    lda #0
    sta message

div_init:               ; Initialize value to be the number to convert
    sei
    lda counter
    sta value
    lda counter + 1
    sta value + 1
    cli

divide:                 ; Initialize the remainder to zero
    lda #0
    sta mod10
    sta mod10 + 1
    clc

    ldx #16
div_loop:               ; Rotate quotient and remainder
    rol value
    rol value + 1
    rol mod10
    rol mod10 + 1

    sec
    lda mod10
    sbc #10
    tay                 ; save low byte in Y
    lda mod10 + 1       ; a,y = dividende - divisor
    sbc #0
    bcc ignore_result   ; branch if dividend < divisor
    sty mod10
    sta mod10 + 1

ignore_result:
    dex
    bne div_loop
    rol value           ; shift in the last bit of the quotient
    rol value + 1

    lda mod10
    clc
    adc #"0"
    jsr push_char

    lda value
    ora value + 1
    bne divide          ; if value != 0, then keep dividing

    ldx #0
print:
    lda message,x
    beq main_loop
    jsr print_char
    inx
    jmp print

push_char:              ; add chars in A reg to start of string 'message'
    pha                 ; push ne first char onto stack
    ldy #0

char_loop:
    lda message,y       ; get char on string and put into x
    tax
    pla
    sta message,y       ; pull char off stack and add it to string
    iny
    txa
    pha                 ; push char from string onto stack
    bne char_loop

    pla
    sta message,y       ; pull null off stack and add to end of string

lcd_wait:
    pha
    lda #%00000000      ; Set PORTB as input
    sta DDRB
    
lcd_busy:
    lda #RW
    sta PORTA
    lda #(RW | E)
    sta PORTA
    lda PORTB
    and #%10000000      ; Check LCD busy flag
    bne lcd_busy

    lda #RW
    sta PORTA
    lda #%11111111      ; Set PORTB as output
    sta DDRB
    pla
    rts

lcd_instruction:
    jsr lcd_wait
    sta PORTB
    lda #0              ; Clear RS/RW/E bits
    sta PORTA
    lda #E              ; Set E bit
    sta PORTA
    lda #0              ; Clear RS/RW/E bits
    sta PORTA
    rts

print_char:
    jsr lcd_wait
    sta PORTB
    lda #RS             ; Clear RS, Clear RW/E bits
    sta PORTA
    lda #(RS | E)       ; Set E bit
    sta PORTA
    lda #RS             ; Clear E bits
    sta PORTA
    rts

nmi:
irq:
    inc counter
    bne exit_i
    inc counter + 1

exit_i:
    rti

    .org $fffa
    .word nmi
    .word reset
    .word irq