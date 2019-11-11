import serial
import traceback
import time
from email_helper import downloadFiles, sendFile
from argparse import ArgumentParser

if __name__ == '__main__':
    parser = ArgumentParser(
        description='''Handles Iridium verification for Quake radio hardware checkouts.''')

    parser.add_argument(
        "-p",
        "--port",
        action="store",
        help="Serial port to open to connect to the Teensy running the checkout test.",
        required=False)
    args = parser.parse_args()

    try:
        console = serial.Serial(args.port, 9600)
        console.write(str.encode("Ready"))
    except serial.SerialException:
        print("Failed to connect to serial port. Exiting.")
        raise SystemExit

    while True:
        line = console.readline().rstrip().decode()
        print("email: " + line)
        if "sending" in line:
            time.sleep(20)
            found_packets = downloadFiles()
            try:
                for m in found_packets:
                    print("m: " + m)
                    if m == "hello from PAN!":
                        console.write(str.encode("received"))
                        print("received")
                # For some reason, when this is uncommented, exeption is raised
                # else:
                #     raise ValueError
            except Exception as e:
                traceback.print_exc(e)
                console.write(str.encode("not found"))
                print("not received")

        if "waiting" in line:
            print("waiting...")
            # with open("uplinkdata.sbd", "w") as f:
            #     f.write("Hello from ground!")

            # sendFile("uplinkdata.sbd")
            # time.sleep(40)
            console.write(str.encode("message sent"))
            print("message sent")
            # need to wait?
            if "received" in line:
                print("received")
                # else:
                #     console.write(str.encode("not received"))
                #     print("not received")

        if "exiting" in line:
            break
