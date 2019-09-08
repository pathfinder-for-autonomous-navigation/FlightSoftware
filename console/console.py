#!/usr/local/bin/python3
import sys
import time
import datetime
import serial
import readline
import threading
import struct
import os
import json
from cmd import Cmd
from argparse import ArgumentParser


class StateCmderAndLogger(object):
    '''
    This class is used by the simulation software to read and write to a flight computer's state.
    '''

    def __init__(self, console_port, baud_rate, data_dir, device_name):
        '''
        Initializes state cmd prompt.

        Args:
        console_port: Serial port of connected Teensy
        data_dir: Directory in which to store the results of the run.
        device_name: Name of device being connected to
        '''
        self.device_name = device_name
        self.console_port = console_port
        self.baud_rate = baud_rate
        self.start_time = datetime.datetime.now()

        self.logfile_name = "{}/{}.log".format(data_dir, device_name)
        self.dbfile_name = "{}/{}.db".format(data_dir, device_name)

        self.logfile = open(self.logfile_name, "w+")
        self.dbfile = open(self.dbfile_name, "w+")
        self.field_values = {}

    def connect(self):
        '''Starts serial connection to the flight computer.'''

        try:
            self.running_console_msg_check = True
            self.check_msg_thread = threading.Thread(
                target=self.check_console_msgs)
            self.console = serial.Serial(self.console_port, self.baud_rate)
            self.check_msg_thread.start()

            return True
        except serial.SerialException:
            print("Error: unable to open serial port for {}.".format(self.device_name))
            return False

    def check_console_msgs(self):
        '''
        Read FC output for debug messages and state variable updates. Record debug messages
        to the logging file, and update the console's record of the state.
        '''

        while self.running_console_msg_check:
            if not self.console.in_waiting:
                continue

            logline = ""
            try:
                # Read line coming from device and parse it
                line = self.console.readline().rstrip()
                data = json.loads(line)
                print(line)

                data["time"] = self.start_time + datetime.timedelta(
                    milliseconds=data["t"])

                if "msg" in data:
                    # Print debug data to the logfile
                    logline = "[{}] ({}) {}".format(data["time"], data["svrty"],
                                                    data["msg"])
                elif "err" in data:
                    logline = "[{}] (ERROR) Tried to {} state value named '{}' but encountered an error: {}".format(data["time"], data["mode"], data["field"], data["err"])
                    self.awaiting_value = False
                    self.awaited_value = None
                else:
                    if data['field'] not in self.field_values:
                        self.field_values[data['field']] = {"timeseries": []}

                    self.field_values[data['field']]["now"] = data['val']
                    self.field_values[data['field']]["timeseries"].append(
                        (data['time'], data['val']))

                    # If the "read state" command is awaiting the current field's value,
                    # report it!
                    self.awaiting_value = False
                    self.awaited_value = data['val']

            except json.JSONDecodeError:
                logline = "[RAW] {}".format(line)
            except serial.SerialException:
                print("Error: unable to read serial port for {}. Exiting.".format(self.device_name))
                self.quit()
            except:
                print("Unspecified error. Exiting.")
                self.quit()
            finally:
                self.logfile.write(logline + "\n")

            time.sleep(0.02)

    def rs(self, field_name):
        '''
        Read state.
        
        Read the value of the state field associated with the given field name on the flight controller.
        '''

        print(field_name)
        json_cmd = {"mode": ord('r'), "field": str(field_name)}
        self.awaiting_value = field_name
        self.console.write(json.dumps(json_cmd).encode())

        # Wait for awaited value
        while self.awaiting_value:
            pass

        return self.awaited_value

    def ws(self, field_name, val):
        '''
        Write state.

        Overwrite the value of the state field with the given state field name on the flight controller.
        
        This function doesn't check for the value of the state actually getting set. That can be handled by
        wsfb().
        '''

        json_cmd = {"mode": ord('w'), "field": str(field_name), "val": str(val)}
        self.console.write(json.dumps(json_cmd).encode())

    def wsfb(self, field_name, val):
        '''
        Write state and check write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight controller, and
        then verify (via a read request) that the state was actually set.
        '''

        print("wrote state")
        self.ws(field_name, val)
        print("reading state")
        return val == self.rs(field_name)

    def quit(self):
        '''Quits the program and stores message log and field telemetry to file.'''

        print("Terminating console connection to {}.".format(self.device_name))
        if self.check_msg_thread.is_alive():
            self.running_console_msg_check = False
            self.check_msg_thread.join()
            self.console.close()

        print("Saving log and telemetry for {}.".format(self.device_name))
        self.logfile.close()
        json.dump(self.field_values, self.dbfile, default=str)
        self.dbfile.close()

class StateCmdPrompt(Cmd):
    '''
    This command prompt is used by the user to read and write to write to the state of
    flight computer(s). It can conveniently command both the ADCS Computer and the Flight Computer.
    '''

    def __init__(self, data_dir, console_port_adcs="", console_port_fc=""):
        self.adcs_cmd = None
        self.fc_cmd = None
        if console_port_adcs:
            self.adcs_cmd = StateCmderAndLogger(console_port_adcs, 1152000, data_dir, "ADCSComputer")
            if not self.adcs_cmd.connect():
                self.do_quit(None)
        if console_port_fc:
            self.fc_cmd = StateCmderAndLogger(console_port_fc, 1152000, data_dir, "FlightComputer")
            if not self.fc_cmd.connect():
                self.do_quit(None)

        # By default, use only the Flight Computer commander if it's available.
        if console_port_fc:
            self.current_cmd = self.fc_cmd
        else:
            self.current_cmd = self.adcs_cmd

        Cmd.__init__(self)

    def emptyline(self):
        # Don't do anything with an empty line input
        pass

    def do_listcomp(self, args):
        '''
        Lists the flight computer currently being interacted with by the user.
        '''
        print("Currently interacting with {}".format(self.current_cmd.device_name))

    def do_switchcomp(self, args):
        '''
        Switches the flight computers that the user is controlling by the command line.
        Only performs a switch if both flight computers are actually connected.
        '''
        if self.current_cmd == self.adcs_cmd and self.fc_cmd:
            self.current_cmd = self.fc_cmd
        elif self.adcs_cmd:
            self.current_cmd = self.adcs_cmd

        print("Switched to {}".format(self.current_cmd.device_name))

    def do_rs(self, args):
        '''
        Read state.

        Read the value of the state field associated with the given field name on the flight controller.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to read")
            return

        print(self.current_cmd.rs(args[0]))

    def do_ws(self, args):
        '''
        Write state.

        Overwrite the value of the state field with the given state field name on the flight controller.

        This function doesn't check for the value of the state actually getting set. That can be handled by
        wsfb.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to set")
            return
        elif len(args) < 2:
            print("Need to specify the value to set")
            return

        self.current_cmd.ws(args[0], args[1])

    def do_wsfb(self, args):
        '''
        Write state and check write operation with feedback.

        Overwrite the value of the state field with the given state field name on the flight controller, and
        then verify (via a read request) that the state was actually set.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to set")
            return
        elif len(args) < 2:
            print("Need to specify the value to set")
            return

        self.current_cmd.wsfb(args[0], args[1])

    def do_quit(self, args):
        '''
        Exits the command line and terminates connections with the flight computer(s).
        '''
        if self.adcs_cmd:
            self.adcs_cmd.quit()
        if self.fc_cmd:
            self.fc_cmd.quit()
        print("Exiting.")
        raise SystemExit

if __name__ == '__main__':
    parser = ArgumentParser(description='''
    Interactive console allows sending state commands to PAN flight computers, and parses console output 
    from flight computers into human-readable, storable logging information.''')

    parser.add_argument("-pa", "--port-adcs", action="store", help="Serial port to open to connect to the ADCS Computer.",   required=False)
    parser.add_argument("-pf", "--port-fc",   action="store", help="Serial port to open to connect to the Flight Computer.", required=False)
    parser.add_argument("-d", "--data-dir", action="store",
        help='''Directory to store the logs and run data in, relative to the location of the console script. 
              Default is logs/.''', default="logs")
    args = parser.parse_args()

    with open("pan_logo.txt", "r") as pan_logo:
        cmd_prompt = StateCmdPrompt(args.data_dir, args.port_adcs, args.port_fc)
        cmd_prompt.prompt = "> "
        try:
            cmd_prompt.cmdloop(pan_logo.read())
        except KeyboardInterrupt:
            cmd_prompt.do_quit(None)
