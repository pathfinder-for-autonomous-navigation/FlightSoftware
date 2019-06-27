#!/usr/local/bin/python3
import sys
import datetime
import serial
import struct
import json
from argparse import ArgumentParser

def close_logfile(args):
    if args.store_log:
        print("Saving log.")
        logfile.close()

def graceful_exit(args):
    close_logfile(args)
    console.close()
    sys.exit()

if __name__ == '__main__':
    parser = ArgumentParser(description="Parses console output from Flight Controller into human-readable, storable logging information.")
    parser.add_argument("-p", "--port", action="store", help="Serial port to open.", required=True)
    parser.add_argument("-s", "--store-log", action="store_true", help="If option selected, saves log to logfile.")
    parser.add_argument("-d", "--log-dir", action="store", 
        help="""Directory to store the logs in, relative to the location of the console script. 
              Default is logs/.""", default="logs")
    args = parser.parse_args()

    if args.store_log:
        logfile = open("{}/{}.log".format(args.log_dir, str(datetime.datetime.now())), "w")

    with open("pan_logo.txt", "r") as pan_logo:
        print(pan_logo.read())

    try:
        start_time = datetime.datetime.now()
        console = serial.Serial(args.port, 9600)
    except serial.SerialException:
        print("Error: unable to open serial port. Exiting.")
        graceful_exit(args)
    except KeyboardInterrupt:
        print("Exiting due to keyboard interrupt.")
        graceful_exit(args)

    while True:
        try:
            # Read line coming from device and parse it
            line = console.readline().rstrip()
            data = json.loads(line)

            data["time"] = start_time + datetime.timedelta(milliseconds=data["t"])

            # Print it to console and (optionally) to logfile
            logline = "[{}] ({}:{}) {}".format(data["time"], data["thd"], data["svrty"], data["msg"])
            print(logline)
            if args.store_log:
                logfile.write(logline + "\n")
        except json.JSONDecodeError:
            print("Error: data from device is malformed. Exiting.\n")
            graceful_exit(args)
        except serial.SerialException:
            print("Error: unable to read serial port. Exiting.")
            graceful_exit(args)
        except KeyboardInterrupt:
            print("Received keyboard interrupt. Exiting.")
            graceful_exit(args)
        except:
            print("Unspecified error. Exiting.")
            graceful_exit(args)