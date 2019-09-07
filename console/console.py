#!/usr/local/bin/python3
import sys
import datetime
import serial
import threading
import struct
import json
from cmd import Cmd
from argparse import ArgumentParser

class FCStateCmdPrompt(Cmd):
    def __init__(self, console_port, logfile_name, store_log):
        self.logfile = open(logfile_name)

        try:
            self.console = serial.Serial(console_port, 9600)
        except serial.SerialException:
            print("Error: unable to open serial port. Exiting.")
            self.do_quit(None)
        except KeyboardInterrupt:
            print("Exiting due to keyboard interrupt.")
            self.do_quit(None)

        self.store_log = store_log
        self.start_time = datetime.datetime.now()

    def preloop(self):
        try:
            # Read line coming from device and parse it
            line = self.console.readline().rstrip()
            data = json.loads(line)

            # Print it to the logfile
            if "msg" in data:
                logline = "[{}] ({}) {}".format(data["time"], data["svrty"],
                                                data["msg"])
            else:
                # Write state variable to 
                logline = "[{}] (STATEMSG) {} = {}".format(
                    data["time"], data["field"], data["val"])

            data["time"] = self.start_time + datetime.timedelta(
                milliseconds=data["t"])
        except json.JSONDecodeError:
            logline = "[RAW] {}".format(line)
        except serial.SerialException:
            print("Error: unable to read serial port. Exiting.")
            self.do_quit(None)
        except:
            print("Unspecified error. Exiting.")
            self.do_quit(None)

        if args.store_log:
            self.logfile.write(logline + "\n")

    def do_rs(self, args):
        """Read the value of the state field associated with the given field name on the flight controller."""
        field_name = args[0]
        json_cmd = {
            "r/w" : 'r',
            "field" : field_name
        }
        self.console.write(json.dumps(json_cmd))
        # Getting the actual value is handled by the logger thread.

    def do_ws(self, args):
        """Overwrite the value of the state field with the given state field name on the flight controller."""
        field_name = args[0]
        val = args[1]
        json_cmd = {
            "r/w" : 'w',
            "field" : field_name,
            "val" : val
        }
        self.console.write(json.dumps(json_cmd))
        # Getting a confirmation of the actual value being set is
        # handled by the logger thread.

    def do_quit(self, args):
        """Quits the program."""
        if self.store_log:
            print("Saving log.")
            self.logfile.close()
        self.console.close()
        print("Quitting.")
        raise SystemExit

if __name__ == '__main__':
    parser = ArgumentParser(description="Parses console output from Flight Controller into human-readable, storable logging information.")
    parser.add_argument("-p", "--port", action="store", help="Serial port to open.", required=True)
    parser.add_argument("-s", "--store-log", action="store_true", help="If option selected, saves log to logfile.")
    parser.add_argument("-d", "--log-dir", action="store",
        help="""Directory to store the logs in, relative to the location of the console script. 
              Default is logs/.""", default="logs")
    args = parser.parse_args()

    logfile_name = "{}/{}.log".format(args.log_dir, str(datetime.datetime.now()))

    with open("pan_logo.txt", "r") as pan_logo:
        pan_logo_str = pan_logo.read()

        cmd_prompt = FCStateCmdPrompt(args.port, logfile_name, args.store_log)
        cmd_prompt.prompt = "> "
        cmd_prompt.cmdloop(pan_logo_str)
