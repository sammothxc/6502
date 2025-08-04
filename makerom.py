code = bytearray([
    0xa9, 0xff,  # LDA #$FF
    0x8d, 0x02, 0x60,  # STA $6002

    0xa9, 0x55,  # LDA #$55
    0x8d, 0x02, 0x60,  # STA $6002

    0xa9, 0xaa,  # LDA #$AA
    0x8d, 0x02, 0x60,  # STA $6002

    0x4c, 0x05, 0x80   # JMP $8005
])

rom = bytearray([0xea] * 32768 - (len(code))) # Fill with NOPs

rom[0x7ffc] = 0x00  # Set the reset vector to point to the start of the code
rom[0x7ffd] = 0x80

with open("rom2.bin", "wb") as out_file:
    out_file.write(rom)
print("ROM created")