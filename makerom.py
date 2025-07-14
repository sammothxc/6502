rom = bytearray([0xea] * 32768)  # 32KB ROM filled with NOPs

with open("rom.bin", "wb") as out_file:
    out_file.write(rom)
print("ROM created with 32KB of NOPs.")