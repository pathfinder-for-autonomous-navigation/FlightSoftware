#!/usr/local/bin/python3
import sys
import datetime
import serial
import json
from argparse import ArgumentParser

def close_logfile(args):
    if args.store_log:
        print("Saving log.")
        logfile.close()

if __name__ == '__main__':
    parser = ArgumentParser(description="Parses console output from Flight Controller into human-readable, storable input.")
    parser.add_argument("-p", "--port", action="store", help="Serial port to open.", required=True)
    parser.add_argument("-s", "--store_log", action="store_true", help="If option selected, saves log to logfile.")
    args = parser.parse_args()

    try:
        start_time = datetime.datetime.now()
        console = serial.Serial(args.port, 9600)
    except serial.SerialException:
        console.close()
        sys.exit("Error: unable to open serial port. Exiting.")

    if args.store_log:
        logfile = open("logs/{}.log".format(str(start_time)), "w")

    while True:
        try:
            # Read line coming from device and parse it
            try:
                line = console.readline().rstrip()
            except serial.SerialException:
                print("Error: unable to open serial port. Exiting.")
                console.close()
                close_logfile(args)
                sys.exit()

            try:
                data = json.loads(line)
            except:
                print(line)
                print("Error: data from device is malformed. Exiting.\n")
                console.close()
                close_logfile(args)
                sys.exit()

            data["time"] = start_time + datetime.timedelta(
                milliseconds=data["t"])

            # Print it to console and (optionally) to logfile
            logline = "[{}] ({}) {}".format(data["time"], data["thd"], data["msg"])
            print(logline)
            if args.store_log:
                logfile.write(logline + "\n")
        except KeyboardInterrupt:
            print("Received keyboard interrupt. Exiting.")
            # Gracefully close serial port and logfile before exiting
            console.close()
            close_logfile(args)
            sys.exit()