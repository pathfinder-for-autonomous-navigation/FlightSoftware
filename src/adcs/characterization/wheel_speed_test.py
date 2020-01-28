"""
28 JAN 2020
src/adcs/characterization/wheel_speed_test.py

Kyle Krol         (kpk63@cornell.edu)
Nathan Zimmerberg (nhz2@cornell.edu)

A utility function to record data from wheel speed sensor characterization
tests. Note, this is written in python 2.7 to conform with platfomio.
"""

from __future__ import print_function
from serial.tools import list_ports
import serial
import sys


def main():

    # Ensure an output file was specified
    if len(sys.argv) < 3:
        print('Need to specify an output file like so: python <script> <file>', file=sys.stderr)
        sys.exit(-1)

    # Open the output file and serial port
    with serial.Serial(sys.argv[1], 4000000, timeout=20) as istream:
      with open(sys.argv[2], 'w') as ostream:

        # Read until complete is reached
        line = istream.readline().rstrip()
        while not line.startswith('#COMPLETE'):
            print(line, file=sys.stdout)
            print(line, file=ostream)
            line = istream.readline().rstrip()

    # Exit on success
    sys.exit(0)


if __name__ == '__main__':
    main()
