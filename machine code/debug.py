rom = bytearray([
    0xA9, 0x11,  # LDA #$11
    0xA9, 0x22,  # LDA #$22
    0xA9, 0x33,  # LDA #$33
    0x4C, 0x00, 0x80  # JMP $8000
])

rom += bytearray([0xEA] * (32768 - len(rom)))  # Fill with NOPs

rom[0x7ffc] = 0x00  # Set the reset vector to point to the start of the code
rom[0x7ffd] = 0x80

with open("rom.bin", "wb") as out_file:
    out_file.write(rom)
print("ROM created")