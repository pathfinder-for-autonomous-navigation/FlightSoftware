#!/usr/local/bin/python3
import readline
from cmd import Cmd
from argparse import ArgumentParser
from state_session import StateSession

class StateCmdPrompt(Cmd):
    '''
    This command prompt is used by the user to read and write to write to the state of
    flight computer(s). It can conveniently command both the ADCS Computer and the Flight Computer.
    '''

    def __init__(self, data_dir, console_port_fc="", console_port_adcs=""):
        self.adcs_cmd = None
        self.fc_cmd = None
        if console_port_adcs:
            self.adcs_cmd = StateSession(console_port_adcs, 1152000, data_dir,
                                         "ADCSComputer")
            if not self.adcs_cmd.connect():
                self.do_quit(None)
        if console_port_fc:
            self.fc_cmd = StateSession(console_port_fc, 1152000, data_dir,
                                       "FlightComputer")
            if not self.fc_cmd.connect():
                self.do_quit(None)

        # By default, if it's available, set the default flight commander to be for the
        # Flight Computer and not the ADCS computer.
        if console_port_fc:
            self.current_cmd = self.fc_cmd
        elif console_port_adcs:
            self.current_cmd = self.adcs_cmd
        else:
            self.current_cmd = None

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
        cmd_prompt = StateCmdPrompt(args.data_dir, args.port_fc, args.port_adcs)
        cmd_prompt.prompt = "> "
        try:
            cmd_prompt.cmdloop(pan_logo.read())
        except KeyboardInterrupt:
            cmd_prompt.do_quit(None)
