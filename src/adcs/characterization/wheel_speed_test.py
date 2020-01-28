"""
28 JAN 2020
src/adcs/characterization/wheel_speed_test.py

Kyle Krol         (kpk63@cornell.edu)
Nathan Zimmerberg (nhz2@cornell.edu)

Some utility functions to test the magnetometers.
"""

import serial
import sys


def main():

    # Search for a teensy on serial
    teensy = None
    for port in serial.tools.list_ports.comports():
        if port.manufacturer == 'Teensyduino':
            teensy = port.device
            break

    # Ensure a teensy was found
    if not teensy:
        print('Could not find a teensy to connect to!', file=sys.stderr)
        sys.exit(-1)

    # Ensure an output file was specified
    if len(sys.argv) < 2:
        print('Need to specify an output file like so: python <script> <file>', file=sys.stderr)
        sys.exit(-1)

    # Open the output file and serial port
    with serial.Serial(teensy, 4000000, timeout=20) as istream:
      with open(sys.argv[1], 'w') as ostream:

        # Read until complete is reached
        line = istream.readline()
        while not line.startswith('#COMPLETE'):
            print(line, file=sys.stdout)
            print(line, file=ostream)
            line = istream.readline()

    # Exit on success
    sys.exit(0)


if __name__ == '__main__':
    main()
