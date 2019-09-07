#!/usr/local/bin/python3
import sys
import datetime
import serial
import threading
import struct
import json
from cmd import Cmd
from argparse import ArgumentParser


class StateCmd(object):
    """
    This class is used by the simulation software to read and write to the device state.
    """

    def __init__(self, console_port, data_dir, device_name):
        """
        Initializes state cmd prompt.

        Args:
        console_port: Serial port of connected Teensy
        log_dir: Directory in which to store the results of the run.
        device_name: 
        """

        if device_name not in ["fc", "adcs"]:
            return

        self.start_time = datetime.datetime.now()

        self.logfile_name = "{}/{}/{}.log".format(data_dir,
                                                  self.start_time,
                                                  device_name)
        self.dbfile_name = "{}/{}/{}.db".format(data_dir,
                                                self.start_time,
                                                device_name)

        self.logfile = open(self.logfile_name, "w")
        self.dbfile = open(self.dbfile_name, "w")
        self.field_values = {}

        try:
            self.console = serial.Serial(console_port, 1152000)
        except serial.SerialException:
            print("Error: unable to open serial port. Exiting.")
            self.quit()
        except KeyboardInterrupt:
            print("Exiting due to keyboard interrupt.")
            self.quit()

    def check_console_msgs(self):
        """
        Read FC output for debug messages and state variable updates. Record debug messages
        to the logging file, and update the console's record of the state.
        """

        try:
            # Read line coming from device and parse it
            line = self.console.readline().rstrip()
            data = json.loads(line)

            if "msg" in data:
                # Print debug data to the logfile
                logline = "[{}] ({}) {}".format(data["time"], data["svrty"],
                                                data["msg"])
            elif 'err' in data:
                logline = "[{}] (ERROR) Tried to {} state value named '{}' but encountered an error: {}".format(data["time"], data["mode"], data["field"], data["err"])
            else:
                if data['field'] not in self.field_values:
                    self.field_values[data['field']] = {"timeseries": []}

                self.field_values[data['field']]["now"] = data['val']
                self.field_values[data['field']]["timeseries"].append(
                    (data['time', data['val']]))

            data["time"] = self.start_time + datetime.timedelta(
                milliseconds=data["t"])

        except json.JSONDecodeError:
            logline = "[RAW] {}".format(line)
        except serial.SerialException:
            print("Error: unable to read serial port. Exiting.")
            self.quit()
        except:
            print("Unspecified error. Exiting.")
            self.quit()

        self.logfile.write(logline + "\n")

    def rs(self, field_name):
        """Read the value of the state field associated with the given field name on the flight controller."""

        json_cmd = {"mode": 'r', "field": field_name}
        self.console.write(json.dumps(json_cmd))

    def ws(self, field_name, val):
        """
        Overwrite the value of the state field with the given state field name on the flight controller.
        
        This function doesn't check for the value of the state actually getting set. That can be handled by
        ws_fb().
        """

        json_cmd = {"mode": 'w', "field": field_name, "val": val}
        self.console.write(json.dumps(json_cmd))

    def ws_fb(self, args):
        # TODO
        pass

    def quit(self):
        """Quits the program and stores message log and field telemetry to file."""

        print("Saving log, telemetry, and quitting.")
        self.logfile.close()
        json.dump(self.field_values, self.dbfile)
        self.dbfile.close()
        self.console.close()

class StateCmdPrompt(Cmd, StateCmd):
    """
    This command prompt is used by the user to read and write to device state.
    """

    def __init__(self, console_port, data_dir, device_name):
        super(StateCmdPrompt, self).__init__()

    def preloop(self):
        self.check_console_msgs()

    def do_rs(self, args):
        self.rs(args[0])

    def do_ws(self, args):
        self.ws(args[0], args[1])

    def do_quit(self, args):
        self.quit()
        raise SystemExit

if __name__ == '__main__':
    parser = ArgumentParser(description="""
    Interactive console allows sending state commands to PAN flight computers, and parses console output 
    from flight computers into human-readable, storable logging information.""")

    parser.add_argument("-p", "--port", action="store", help="Serial port to open.", required=True)
    parser.add_argument("-d", "--data-dir", action="store",
        help="""Directory to store the logs and run data in, relative to the location of the console script. 
              Default is logs/.""", default="logs")
    args = parser.parse_args()

    with open("pan_logo.txt", "r") as pan_logo:
        pan_logo_str = pan_logo.read()

        cmd_prompt = StateCmdPrompt(args.port, args.data_dir, "fc")
        cmd_prompt.prompt = pan_logo_str
        cmd_prompt.cmdloop(pan_logo_str)
