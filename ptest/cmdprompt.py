try:
    import readline
except ImportError:
    # We're on Windows, so readline doesn't exist
    pass
from cmd import Cmd
import timeit
from .cases.utils import Enums
from .plotter import PlotterClient
from .usb_session import USBSession
from .cases.utils import str_to_val
import csv

def USBSessionOnly(fn):
    """
    Ensures that the function is only called for a State Session device.
    """

    def inner(self, args):
        if isinstance(self.cmded_device, USBSession):
            fn(self, args)
        else:
            print("Cannot use this function since currently commanded device is not a USB session.")

    return inner

class StateCmdPrompt(Cmd):
    '''
    This command prompt is used by the user to read and write to write to the state of
    Teensies and simulation devices.
    '''

    def __init__(self, devices, radios, exit_fn, testcase):
        self.devices = {**devices, **radios}
        self.exit_fn = exit_fn
        self.testcase = testcase
        if not self.devices:
            # There's no flight controller to connect with.
            self.do_exit(None)

        # By default, if it's available, set the prompt to be commanding the Flight Controller.
        try:
            self.cmded_device = self.devices['FlightController']
        except KeyError:
            try:
                self.cmded_device = self.devices['FlightControllerFollower']
            except KeyError:
                self.cmded_device = list(self.devices.values())[0]

        self.intro = "Beginning console.\nType \"help\" for a list of commands.\n" \
                    "NOTE: You are currently connected to the {}.".format(self.cmded_device.device_name)
        self.prompt = '> '

        Cmd.__init__(self)

    def emptyline(self):
        # Don't do anything with an empty line input
        pass

    def do_checkcomp(self, args):
        '''
        Lists the Teensy currently being interacted with by the user.
        '''
        print(f"Currently interacting with {self.cmded_device.device_name}")

    def do_listcomp(self, args):
        '''
        Lists all available Teensies.
        '''
        print("Available devices:")
        for device_name in self.devices.keys():
            print(device_name)

    def do_switchcomp(self, args):
        '''
        
        Switches the Teensy that the user is controlling by the command line.

        If no args are presented, it attempts to automatically switch 
        to the other sat

        Otherwise, the full name must be specified.
        '''
        args = args.split()
        if len(self.devices) == 2:
            if len(args) < 1:
                list_of_names = [x for x,y in self.devices.items()]
                if self.cmded_device.device_name == self.devices[list_of_names[0]].device_name:
                    self.cmded_device = self.devices[list_of_names[1]]
                else:
                    self.cmded_device = self.devices[list_of_names[0]]

                print(f"Switched to {self.cmded_device.device_name}")                
                
            else:
                try:
                    self.cmded_device = self.devices[args[0]]
                    print(f"Switched to {self.cmded_device.device_name}")                

                except KeyError:
                    print('Invalid device specified')
            
        else:
            print("Can't switchcomp in SingleSatCase")

    def do_rs(self, args):
        '''
        Read state. See usb_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print('Need to specify a state field to read')
            return

        start_time = timeit.default_timer()
        try:
            read_result = self.cmded_device.smart_read(args[0])
        except NameError:
            read_result = None
            print('Field with that name does not exist!')
            return 
            
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        try:
            human_readable_result = Enums()[args[0]][read_result]
            print(f"{read_result} ({human_readable_result}) \t\t\t\t\t\t(Completed in {elapsed_time} us)")
        except KeyError:
            # args[0] is not an enum field.
            print(f"{read_result} \t\t\t\t\t\t(Completed in {elapsed_time} us)")
            
    def do_rs_psim(self, args):
        '''
        Read state. See usb_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print('Need to specify a state field to read')
            return

        start_time = timeit.default_timer()
        try:
            read_result = self.testcase.rs_psim(args[0])
        except NameError:
            read_result = None
            print('Field with that name does not exist!')
            return 
            
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        try:
            human_readable_result = Enums()[args[0]][read_result]
            print(f"{read_result} ({human_readable_result}) \t\t\t\t\t\t(Completed in {elapsed_time} us)")
        except KeyError:
            # args[0] is not an enum field.
            print(f"{read_result} \t\t\t\t\t\t(Completed in {elapsed_time} us)")            
            
    def do_ws_psim(self, args):
        '''
        Write state. See usb_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print('Need to specify a state field to set')
            return
        elif len(args) < 2:
            print('Need to specify the value to set')
            return
        
        val = str_to_val(args[1])

        start_time = timeit.default_timer()
        write_succeeded = self.testcase.ws_psim(args[0], val)
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        write_succeeded = "Succeeded" if write_succeeded else "Failed"
        print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    def do_ws(self, args):
        '''
        Write state. See usb_session.py for documentation.
        '''
        args = args.split()

        if len(args) < 1:
            print('Need to specify a state field to set')
            return
        elif len(args) < 2:
            print('Need to specify the value to set')
            return

        start_time = timeit.default_timer()
        write_succeeded = self.cmded_device.write_state(args[0], args[1])
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        write_succeeded = "Succeeded" if write_succeeded else "Failed"
        print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    @USBSessionOnly
    def do_cycle(self, args):
        '''
        Start a control cycle.
        '''
        self.testcase.cycle()

    @USBSessionOnly
    def do_cyclecount(self, args):
        '''
        Get the number of control cycles that have executed.
        '''
        self.do_rs("pan.cycle_no")

    @USBSessionOnly
    def do_telem(self, args):
        '''
        Dump telemetry.
        '''
        start_time = timeit.default_timer()
        write_succeeded = self.cmded_device.write_state("telem.dump", "true")
        write_succeeded &= self.cmded_device.write_state("cycle.start", "true")
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        write_succeeded = "Succeeded" if write_succeeded else "Failed"
        print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")
    
    @USBSessionOnly
    def do_parsetelem(self, args):
        '''
        Parse a telelmetry file using DowlinkParser.
        '''
        args = args.split()
        if len(args) != 0:
            print('parsetelem takes no args')
            return
        else:
            start_time = timeit.default_timer()
            print(self.cmded_device.parsetelem())
            elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

            print(f"\t\t\t\t\t\t(Completed in {elapsed_time} us)")

    @USBSessionOnly
    def do_dbtelem(self, args):
        '''
        Store Telemetry in Database
        '''
        args = args.split()
        if len(args) != 0:
            print('dbtelem takes no args')
            return
        else:
            start_time = timeit.default_timer()
            successful_upload = self.cmded_device.dbtelem()
            elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

            write_succeeded = "Succeeded" if successful_upload else "Failed"
            print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    @USBSessionOnly
    def do_scrapeuplinks(self, args):
        '''
        Scrape all telemtry sent to the PAN email 
        and send it to the Flight Computer
        '''
        args = args.split()
        if len(args) != 0:
            print('scrapeuplinks takes no args')
            return
        else:
            start_time = timeit.default_timer()
            success = self.cmded_device.scrape_uplink()
            elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

            write_succeeded = "Succeeded" if success else "Failed"
            print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    def do_wms(self, args):
        '''
        Write multiple states. See usb_session.py for documentation.
        '''
        args = args.split()

        if len(args) == 0:
            print('Need to specify a state field to set')
            return
        elif len(args) % 2 != 0:
            print("Need to specify a value for every state field to set")
            return

        fields = [args[x] for x in range(0, len(args), 2)]
        vals = [args[x] for x in range(1, len(args), 2)]

        start_time = timeit.default_timer()
        write_succeeded = self.cmded_device.write_multiple_states(fields, vals)
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        write_succeeded = "Succeeded" if write_succeeded else "Failed"
        print(f"{write_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    def do_plot(self, args):
        '''
        Plot the given state fields. See usb_session.py for documentation.
        '''
        plotter = PlotterClient(self.cmded_device.datastore.dataList)
        plotter.do_plot(args)
    
    def do_csv(self, filepath):
        '''
            add measurements from a run (key,value pairs) to rows of a csv file 
            and open file at given filepath
            @param filepath: the location of the csv file to open
        '''
        
        if len(filepath) == 0:
            print("Need to specify a file path to put csv file (from FlightSoftware), Format: PATH/TO/FILE/")
            return
        
        with open(str(filepath) + 'mtr_logs.csv', 'a', newline='') as csvfile:
            mtrwriter = csv.writer(csvfile)
            for data in self.cmded_device.datastore.dataList:
                for key, value in data.items():
                    mtrwriter.writerow([key, value])
        print("csv file \'mtr_logs\'written to " + str(filepath))
            

    @USBSessionOnly
    def do_uplink(self, args):
        '''
        Uplink fields
        '''
        args = args.split()

        if len(args) == 0:
            print('Need to specify a state field to set')
            return
        elif len(args) % 2 != 0:
            print("Need to specify a value for every state field to set")
            return

        fields = [args[x] for x in range(0, len(args), 2)]
        vals = [str_to_val(args[x]) for x in range(1, len(args), 2)]

        start_time = timeit.default_timer()
        uplink_succeeded = self.cmded_device.uplink(fields, vals)
        uplink_succeeded &= self.cmded_device.write_state("cycle.start", "true")
        elapsed_time = int((timeit.default_timer() - start_time) * 1E6)

        uplink_succeeded = "Succeeded" if uplink_succeeded else "Failed"
        print(f"{uplink_succeeded} \t\t\t\t\t\t(Completed in {elapsed_time} us)")

    def do_quit(self, args):
        '''
        Exits the command line and terminates connections with the flight computer(s).
        '''
        self.exit_fn('Exiting command line.', is_error=False)
        return True

    def do_exit(self, args):
        '''
        Exits the command line and terminates connections with the flight computer(s).
        '''
        self.do_quit(None)
        return True
    
    def do_special(self, args):
        '''Call the special function'''
        self.testcase.special_function()
