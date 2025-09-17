PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

E = %10000000
RW = %01000000
RS = %00100000

    .org $8000          ; Program start address

reset:
    ldx #$ff
    txs
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

    ldx #0
print:
    lda message,x
    beq nop_loop
    jsr print_char
    inx
    jmp print

nop_loop:
    jmp nop_loop

message: .asciiz "Hello, world!"

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

    .org $fffc          ; Boot location
    .word reset
    .word $0000