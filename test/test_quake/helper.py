import serial
import time
from email_helper import downloadFiles, sendFile
from argparse import ArgumentParser

if __name__ == '__main__':
    parser = ArgumentParser(
        description=
        '''Handles Iridium verification for Quake radio hardware checkouts.''')

    parser.add_argument(
        "-p",
        "--port",
        action="store",
        help=
        "Serial port to open to connect to the Teensy running the checkout test.",
        required=False)
    args = parser.parse_args()

    console = serial.Serial(args.port, 9600)
    console.write("Ready")

    while True:
        line = console.readline().rstrip()
        if "sending" in line:
            time.sleep(20)
            found_packets = downloadFiles()
            try:
                if found_packets[0] == "hello from PAN!":
                    console.write("received")
                    print("received")
                else:
                    raise ValueError
            except:
                console.write("not found")
                print("not received")

        if "waiting" in line:
            with open("uplink_data.txt", "w") as f:
                f.write("Hello from ground!")

            found_packets = sendFile("uplink_data.txt")
            while True:
                line = console.readline.rstrip()
                if "received" in line:
                    console.write("received")
                    print("received")
                else:
                    console.write("not received")
                    print("received")

        if "exiting" in line:
            break
