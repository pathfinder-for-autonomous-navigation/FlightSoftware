import readline
from cmd import Cmd

class StateCmdPrompt(Cmd):
    '''
    This command prompt is used by the user to read and write to write to the state of
    flight computer(s). It can conveniently command both the ADCS Computer and the Flight Computer.
    '''

    def __init__(self, fc_cmd, adcs_cmd):
        self.adcs_cmd = adcs_cmd
        self.fc_cmd = fc_cmd

        # By default, if it's available, set the prompt to be commanding the Flight Computer.
        if self.fc_cmd:
            self.current_cmd = self.fc_cmd
        elif self.adcs_cmd:
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
        Read state. See state_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to read")
            return

        print(self.current_cmd.read_state(args[0]))

    def do_ws(self, args):
        '''
        Write state. See state_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to set")
            return
        elif len(args) < 2:
            print("Need to specify the value to set")
            return

        self.current_cmd.write_state(args[0], args[1])

    def do_wsfb(self, args):
        '''
        Write state and check write operation with feedback. See state_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print("Need to specify a state field to set")
            return
        elif len(args) < 2:
            print("Need to specify the value to set")
            return

        self.current_cmd.write_state_fb(args[0], args[1])

    def do_os(self, args):
        '''
        Override simulation state. See state_session.py for documentation.
        '''
        args = args.split()
        self.current_cmd.override_state(args[0], args[1])

    def do_ro(self, args):
        '''
        Release override of simulation state. See state_session.py for documentation.
        '''
        args = args.split()
        self.current_cmd.release_override(args[0])

    def do_osfb(self, args):
        '''
        Override state and check write operation with feedback. See state_session.py for documentation.
        '''
        args = args.split()
        self.current_cmd.override_state_fb(args[0], args[1])

    def do_quit(self, args):
        '''
        Exits the command line and terminates connections with the flight computer(s).
        '''
        if self.adcs_cmd:
            self.adcs_cmd.disconnect()
        if self.fc_cmd:
            self.fc_cmd.disconnect()
        print("Exiting.")
        raise SystemExit
