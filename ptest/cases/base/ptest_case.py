from ...data_consumers import Logger
from ..utils import Enums, TestCaseFailure, suppress_faults

import threading
import time
import traceback


class FancyFlightController(object):
    """Wrapper around a flight controller specific to HOOTL and HITL testcases.
    """
    def __init__(self, flight_controller, logger):
        super(FancyFlightController, self).__init__()

        self.__flight_controller = flight_controller
        self.__logger = logger

    def rs(self, name: str, print: bool=False):
        """Reads a statefield and prints it to the logger if requested.
        """
        value = self.__flight_controller.smart_read(name)
        if print:
            self.__logger.put("{} is {}".format(name, value))

        return value
    
    def print_rs(self, name: str):
        """Reads a statefield, returns it, and prints it."""
        return self.rs(name, print=True)

    def ws(self, name: str, value, print: bool=False):
        """Writes a statefield and prints it to the logger if requested.
        """
        self.__flight_controller.write_state(name, value)
        if print:
            self.__logger.put("{} set to {}".format(name, value))

    def print_ws(self, name: str, value):
        """Writes a statefield, and prints it."""
        self.ws(name, value, print=True)

    @property
    def mission_state(self) -> str:
        """Returns the current mission state as a string.
        """
        return Enums.mission_states[self.rs("pan.state")]

    @mission_state.setter
    def mission_state(self, state: str):
        """Sets the mission state.
        """
        self.ws("pan.state", int(Enums.mission_states[state]))


class PTestCase(object):
    """Base class for all HOOTL and HITL testcases.

    Attributes:
        debug_to_console  Setting this to true will pipe debug output from the
                          flight computer to the console. Defaults to false.
    """
    def __init__(self, is_interactive, random_seed, data_dir, device_config):
        self.is_interactive = is_interactive
        self.random_seed = random_seed
        self.data_dir = data_dir
        self.device_config = device_config

        self.logger = Logger("testcase", data_dir, print=True)

        self.errored = False
        self.finished = False
        self.devices = None

        self.debug_to_console = False

    def setup(self, devices, radios):
        """Initial entrypoint for running a testcase.

        Here, we simply populate the testcase devices and setup any interaction.
        From there, the PSim simulation would be setup next or control would
        be handed off to the next class down the inheritance tree.
        """
        self.populate_devices(devices, radios)

        for _,device in devices.items():
            device.case_interaction_setup(self.debug_to_console)

        self.logger.start()
        self.logger.put("[TESTCASE] Starting testcase.")

    def populate_devices(self, devices, radios):
        """Read the list of PTest-connected devices and pull in the ones that we
        care about.
        """
        raise NotImplementedError

    def start(self):
        """Called after setup to actually run the testcase.
        """
        def _run():
            while not self.finished:
                try:
                    self.run()
                except TestCaseFailure:
                    tb = traceback.format_exc()
                    self.logger.put(tb)
                    self.finish(True)
                    return

        if self.is_interactive:
            self.testcase_thread = threading.Thread(name="Testcase execution", target=_run, daemon=True)
            self.testcase_thread.start()
        else:
            _run()

    def run(self):
        """Implemented by the subclass testcase providing the actual testcase
        behavior.

        This function is called in a loop repeatedly until testcase termination.
        If you wish to exit the testcase from within the run function itself,
        simply call the finish function.
        """
        raise NotImplementedError

    def finish(self, error=False):
        """
        When called, this function indicates to PTest that
        the testcase has finished its execution.
        """
        self.errored = error

        if not self.finished:
            self.logger.put("[TESTCASE] Finished testcase.")
            self.finished = True
            self.logger.stop()
            time.sleep(1) # Allow time for logger to stop

    def cycle(self):
        """Steps the testcase forward.

        When working with a PSim simulation testcase, this will also step the
        simulation forward in time and handling transactions between flight
        software and PSim.
        """
        pass

    @property
    def havt_read(self):
        '''
        Returns the ADCS HAVT table as a list of booleans
        '''
        read_list = [False for x in range(Enums.havt_length)]
        for x in range(Enums.havt_length):
            read_list[x] = self.rs("adcs_monitor.havt_device"+str(x))
        return read_list

    def print_havt_read(self):
        '''
        Prints the ADCS HAVT list in reverse (normal) order
        '''
        binary_list = [1 if x else 0 for x in self.havt_read]

        string_of_binary_list = [str(x) for x in binary_list]
        
        # Reverse the list so it prints as it does in ADCSSoftware
        string_of_binary_list.reverse()

        list_of_list = [string_of_binary_list[4*i:(4*i)+4] for i in range((int)(Enums.havt_length/4)+1)]
        final = [x + [" "] for x in list_of_list]

        final_string = ''.join([''.join(x) for x in final])
        self.logger.put("HAVT Read: "+str(final_string))

    def print_non_functional_adcs_havt(self):
        '''
        Prints all non functional devices
        '''
        for x in range(Enums.havt_length):
            if not self.havt_read[x]:
                self.logger.put(f"Device #{x}, {Enums.havt_devices[x]} is not functional")

    def print_header(self, title):
        self.logger.put("\n"+title+"\n")

    def soft_assert(self, condition, *args):
        """
        Soft assert prints a fail message if the condition is False
        
        If specificied with a fail message, then a pass message, 
        it will also print a pass message if condition is True.
        """
        if condition: 
            if len(args) == 1:
                pass
            else:
                self.logger.put(args[1])
        else: 
            self.logger.put(f"\n$ SOFT ASSERTION ERROR: {args[0]}\n")

    def special_function(self):
        '''A special function to be overriden by a case that can be called from the console'''
        pass