import argparse
import serial
import time


def main():
    parser = argparse.ArgumentParser(description="Parallel EEPROM Programmer")

    parser.add_argument("-d", "--device", action="store", type=str, required=True)
    parser.add_argument("-r", "--read", action="store_true")
    parser.add_argument("-w", "--write", action="store_true")
    parser.add_argument("-f", "--file", action="store", type=str)
    parser.add_argument("-l", "--limit", action="store", type=int, default=None)
    parser.add_argument("-o", "--offset", action="store", type=int, default=0)
    parser.add_argument("-c", "--clear", action="store_true")

    args = parser.parse_args()

    # Open serial port
    ser = serial.Serial(args.device, 115200)

    time.sleep(1)

    if not ser.is_open:
        print("Failed to open " + ser.name)
        exit(1)

    ser.flushInput()

    print("Connected to " + ser.name + " at " + str(ser.baudrate))

    addr = 0

    if (args.offset):
        addr = args.offset

    if args.read:
        print("Reading EEPROM")

        for x in range(args.limit):
            command = "RD" + hex(addr)[2:].zfill(4).upper() + '\n'
            b = command.encode()
            ser.write(b)

            # Wait for response
            response = ser.readline().decode().strip()
            print(hex(addr)[2:].zfill(4).upper() + " : " + response.zfill(2))
            addr += 1

    elif args.write:
        print("Writing file " + args.file + " to EEPROM")

        # Open binary file
        with open(args.file, mode='rb') as file:
            contents = file.read()

            print("Input file size: " + str(len(contents)))

            # Set limit to file size if no limit specified
            limit = args.limit if args.limit is not None else len(contents)

            print("Limiting to first " + str(limit) + " bytes")

            if args.write:
                for b in contents:
                    command = "WR" + \
                        hex(addr)[2:].zfill(4).upper() + \
                        hex(b)[2:].zfill(2).upper() + '\n'
                    b = command.encode()
                    ser.write(b)
                    print("write")
                    addr += 1

                    # Wait for response
                    response = ser.readline().decode().strip()

                    if response != "DONE":
                        print(response)
                        ser.close()
                        print("Closed " + ser.name)
                        exit(1)
                    else:
                        print(
                            str(addr - args.offset) + " / " + str(len(contents)))

                    if args.limit is not None and addr >= args.limit + args.offset:
                        break

    elif args.clear:
        print("Wiping EEPROM")
        for x in range(args.limit):
            command = "WR" + \
                hex(addr)[2:].zfill(4).upper() + \
                hex(255)[2:].zfill(2).upper() + '\n'
            b = command.encode()
            ser.write(b)
            addr += 1

            # Wait for response
            response = ser.readline().decode().strip()

            if response != "DONE":
                print(response)
                ser.close()
                print("Closed " + ser.name)
                exit(1)
            else:
                print(str(addr - args.offset) + " / " + str(args.limit))

    ser.close()
    print("Closed " + ser.name)


main()