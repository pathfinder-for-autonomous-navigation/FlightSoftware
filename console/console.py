#!/usr/local/bin/python3

from argparse import ArgumentParser
from state_session import StateSession
from cmdprompt import StateCmdPrompt

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
        if args.port_fc:
            fc_cmd = StateSession(args.data_dir, "FlightComputer")
            if not fc_cmd.connect(args.port_fc, 1152000):
                fc_cmd = None
        if args.port_adcs:
            adcs_cmd = StateSession(args.data_dir, "ADCSComputer")
            if not adcs_cmd.connect(args.port_adcs, 1152000):
                adcs_cmd = None

        cmd_prompt = StateCmdPrompt(fc_cmd, adcs_cmd)
        cmd_prompt.prompt = "> "
        try:
            cmd_prompt.cmdloop(pan_logo.read())
        except KeyboardInterrupt:
            # Gracefully exit debugging session
            cmd_prompt.do_quit(None)
