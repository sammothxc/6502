code = bytearray([
    0xa9, 0x42,  # LDA #$42
    0x8d, 0x00, 0x60,  # STA $6000
])

rom = bytearray([0xea] * 32768 - len(code))

rom[0x7ffc] = 0x00  # Set the reset vector to point to the start of the code
rom[0x7ffd] = 0x80

with open("rom2.bin", "wb") as out_file:
    out_file.write(rom)
print("ROM created")