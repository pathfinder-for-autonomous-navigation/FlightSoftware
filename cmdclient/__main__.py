from argparse import ArgumentParser
import json
from cmd import Cmd
import sys
import requests
from cerberus import Validator

class CmdClient(Cmd):
    def __init__(self, config):
        self.config = config

        self.intro = "Beginning command client.\nType \"help\" for a list of commands.\n" \
                    "Remember to choose a device to command before you start commanding."
        self.prompt = '> '

        self.cmded_device = list(config.keys())[0]


        self.url = "http://localhost:"+str(config[self.cmded_device]["port"]) #creates an http url to access the device

        Cmd.__init__(self)

    def do_lc(self, args):
        """
        List available devices to connect to.
        """
        print("Available devices: " + str([key for key in self.config]))

    def do_cc(self, args):
        """
        Check the device we are currently connected to.
        """
        print("Currently connected device: " + self.cmded_device)

    def do_sc(self, args):
        """
        Switches which device is currently being commanded.
        """
        args = args.split()
        if len(args) < 1:
            print("Need to specify a computer to switch to.")
            return

        comp = args[0]
        if comp not in self.config:
            print(f"Failed: a connection for {comp} is not configured.")
        else:
            self.cmded_device = comp
            print(f"Switched to commanding {comp}")

    def do_list(self, args):
        """
        Lists the elements in the queued uplink for the current device, or, returns None if
        no uplink is currently queued or can be queued.
        """
        raise NotImplementedError

    def do_add(self, args):
        """
        Adds a set of state fields to the currently queued uplink. The command may fail if
        the packet size after adding the state fields would exceed the maximum allowable uplink
        size (currently 70 bytes.)
        """
        endpoint = self.url+"/send-telem"
        r = requests.get(url = endpoint)
        data = r.json
        print(data)

    def do_remove(self, args):
        """
        Removes a set of state fields from the currently queued uplink.
        """
        raise NotImplementedError

    def do_pause(self, args):
        """
        Attempts to pause the uplink queue timer, if one is running.
        """
        endpoint = self.url+"/time"
        r = requests.get(url = endpoint)
        data = r.json
        print(data)

    def do_resume(self, args):
        """
        Attempts to resume the uplink queue timer, if one was running.
        """
        endpoint = self.url+"/resume"
        r = requests.get(url = endpoint)
        data = r.json
        print(data)


    def do_exit(self, args):
        """Exits the client."""
        sys.exit(0)

    def do_quit(self, args):
        """Exits the client."""
        sys.exit(0)

def main():
    with open("cmdclient/config.json") as config_file:
        config = json.load(config_file)

    config_schema = {
        "server" : {"type" : "string", "required" : True},
        "port" : {"type" : "integer", "required" : True},
        "type" : {"type" : "string", "allowed" : ["StateSession", "RadioSession"], "required" : True},
    }

    v = Validator(config_schema)
    for key in config:
        config_item = config[key]
        if not v.validate(config_item, config_schema):
            print("Malformed config file. The following errors were found. Exiting.")
            print(v.errors)
            sys.exit(1)

    cmd_client = CmdClient(config)

    try:
        while True:
            cmd_client.cmdloop()
    except KeyboardInterrupt:
        print("Exiting due to keyboard interrupt.")
        sys.exit(0)

if __name__ == "__main__":
    main()
