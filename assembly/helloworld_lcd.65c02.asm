PORTB = $6000
PORTA = $6001
DDRB = $6002
DDRA = $6003

E = %10000000
RW = %01000000
RS = %00100000

    .org $8000      ; Program start address

reset:
    ldx #$ff
    txs
    lda #%11111111  ; Set all pins of PORTB to output
    sta DDRB
    lda #%11100000  ; Set 3 pins of PORTA to output
    sta DDRA

    lda #%00111000  ; Set 8-bit mode, 2-line display, 5x8 font
    jsr lcd_instruction
    lda #%00001110  ; Display on, cursor on, no blink
    jsr lcd_instruction
    lda #%00000110  ; Increment mode, shift cursor, don't shift display
    jsr lcd_instruction
    lda #%00000001  ; Clear display
    jsr lcd_instruction

    lda #"H"        ; Send H
    jsr print_char

    lda #"e"        ; Send e
    jsr print_char

    lda #"l"        ; Send l
    jsr print_char

    lda #"l"        ; Send l
    jsr print_char

    lda #"o"        ; Send o
    jsr print_char

    lda #","        ; Send ","
    jsr print_char

    lda #" "        ; Send " "
    jsr print_char

    lda #"W"        ; Send W
    jsr print_char

    lda #"o"        ; Send o
    jsr print_char

    lda #"r"        ; Send r
    jsr print_char

    lda #"l"        ; Send l
    jsr print_char

    lda #"d"        ; Send d
    jsr print_char

    lda #"!"        ; Send "!"
    jsr print_char

inf_loop:
    jmp inf_loop

lcd_instruction:
    sta PORTB
    lda #0          ; Clear RS/RW/E bits
    sta PORTA
    lda #E          ; Set E bit
    sta PORTA
    lda #0          ; Clear RS/RW/E bits
    sta PORTA
    rts

print_char:
    sta PORTB
    lda #RS         ; Clear RS, Clear RW/E bits
    sta PORTA
    lda #(RS | E)   ; Set E bit
    sta PORTA
    lda #RS         ; Clear E bits
    sta PORTA
    rts

    .org $fffc
    .word reset
    .word $0000