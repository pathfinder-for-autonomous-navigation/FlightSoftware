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
    def __init__(self, console_port, dbfile_name, logfile_name, store_log):
        self.logfile = open(logfile_name, "w")
        self.dbfile = open(dbfile_name, "w")
        self.field_values = {}

        try:
            self.console = serial.Serial(console_port, 1152000)
        except serial.SerialException:
            print("Error: unable to open serial port. Exiting.")
            self.do_quit(None)
        except KeyboardInterrupt:
            print("Exiting due to keyboard interrupt.")
            self.do_quit(None)

        self.store_log = store_log
        self.start_time = datetime.datetime.now()

    def preloop(self):
        # Read FC output for debug messages and state variable updates

        try:
            # Read line coming from device and parse it
            line = self.console.readline().rstrip()
            data = json.loads(line)

            # Print it to the logfile
            if "msg" in data:
                logline = "[{}] ({}) {}".format(data["time"], data["svrty"],
                                                data["msg"])
            else:
                if data['field'] not in self.field_values:
                    self.field_values[data['field']] = []
                if not 'err' in data:
                    self.field_values[data['field']].append((data['time',data['val']]))

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
        # Getting a confirmation of the actual value being set is handled by the logger thread.

    def do_quit(self, args):
        """Quits the program and stores message log and field telemetry to file."""
        if self.store_log:
            print("Saving log, telemetry, and quitting.")
            self.logfile.close()

        json.dump(self.field_values, self.dbfile)
        self.dbfile.close()
        self.console.close()
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
    dbfile_name = "{}/{}.db".format(args.log_dir, str(datetime.datetime.now()))

    with open("pan_logo.txt", "r") as pan_logo:
        pan_logo_str = pan_logo.read()

        cmd_prompt = FCStateCmdPrompt(args.port, dbfile_name, logfile_name, args.store_log)
        cmd_prompt.prompt = "> "
        cmd_prompt.cmdloop(pan_logo_str)
