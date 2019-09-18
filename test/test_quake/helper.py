import serial
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
            # TODO collect data that is being sent, download email, verify that data that was sent is the same
            # as data that was received over Iridium
            pass

        if "waiting" in line:
            # TODO Send uplink via email to Iridium, send Teensy the data that was sent so that it can verify the data
            pass

        if "exiting" in line:
            break
