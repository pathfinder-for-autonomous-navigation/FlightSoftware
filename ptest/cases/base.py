from ..data_consumers import Logger
import time
import math
import threading
import traceback
from .utils import Enums, TestCaseFailure, suppress_faults
import psim # the actual python psim repo
import lin
import datetime

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

class SingleSatCase(PTestCase):
    """Base class for all HOOTL and HITL testcases involving a single satellite.

    Attributes:
        initial_state  Set this to the string name of a desired flight software
                       state. The testcase will cycle in setup until this state
                       is reached. Defaults to 'startup'.

        initial_state_timeout  Timeout period in cycles in which flight software
                               has to get into the desired initial state. If
                               this timeout is violated, the testcase will fail.

        skip_deployment_wait  Set this to true to skip the deployment wait. This
                              sets the deployment elapsed counter high enough in
                              setup so the satellite skips deployment wait on
                              its very next cycle. Defaults to false.

        suppress_faults  Set this to true to suppress faults in setup. Defaults
                         to true.
    """
    def __init__(self, *args, **kwargs):
        super(SingleSatCase, self).__init__(*args, **kwargs)

        self.initial_state = "startup"
        self.initial_state_timeout = 25
        self.skip_deployment_wait = False
        self.suppress_faults = True

    def populate_devices(self, devices, radios):
        self.flight_controller = devices["FlightController"]
        self.devices = [self.flight_controller]

    def setup(self, devices, radios):
        super(SingleSatCase, self).setup(devices, radios)

        if self.suppress_faults:
            self.logger.put("[TESTCASE] Suppressing Faults!")
            suppress_faults(self.flight_controller, self.logger)

        if self.skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping deployment wait!")
            self.flight_controller.write_state("pan.deployment.elapsed", "15000")

        self.pre_boot()

        cycles = 0
        initial_state = Enums.mission_states[self.initial_state]
        state = self.flight_controller.smart_read("pan.state")
        while cycles < self.initial_state_timeout and state != initial_state:
            if cycles > self.initial_state_timeout:
                raise TestCaseFailure(f"Failed to reach desired state of {initial_state}")

            super(SingleSatCase, self).cycle()

            cycles = cycles + 1
            state = self.flight_controller.smart_read("pan.state")

        self.post_boot()

    def pre_boot(self):
        """Allows a testcase to perform actions just after supressing faults and
        skipping deployment wait but before attempting to boot the spacecraft.
        See setup for more information.
        """
        pass

    def post_boot(self):
        """Allows a testcase to perform actions just after booting the
        spacecraft. See setup for more information.
        """
        pass

    def read_state(self, string_state):
        """
        Wrapper function around flight controller's read_state.
        """
        return self.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        """
        Wrapper function around flight controller's write_state.
        """
        self.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    @property
    def mission_state(self):
        """
        Returns mission state as a string: "standby", "startup", etc.
        """
        return Enums.mission_states[int(self.flight_controller.read_state("pan.state"))]

    @mission_state.setter
    def mission_state(self, state):
        self.flight_controller.write_state("pan.state", int(Enums.mission_states[state]))

    def cycle(self):
        """
        Steps the FC forward by one CC

        Asserts the FC did indeed step forward by one CC
        """
        super(SingleSatCase, self).cycle()
        
        self.flight_controller.write_state('cycle.start', 'true')

    def rs(self, name):
        """
        Reads a state field (with type inference from smart_read()).

        Checks that the name is indeed a string.
        """
        assert(type(name) is str), "State field name was not a string."
        ret = self.flight_controller.smart_read(name)
        return ret

    def print_rs(self, name):
        """
        Reads a statefield, and also prints it.
        """
        ret = self.rs(name)
        self.logger.put(f"{name} is {ret}")
        return ret

    def ws(self, name, val):
        """
        Writes a state
        """
        self.flight_controller.write_state(name, val)

    def print_ws(self, name, val):
        """
        Writes the state and prints the written value.
        """
        self.logger.put(f"{name} set to: {val}")
        self.ws(name, val)

class DualSatCase(PTestCase):
    """Base class for all HOOTL and HITL testcases involving two satellites.
    See the SingleSatCase for descriptions of the attributes. Many of the other
    functions used here mirror elements of SingleSatCase as well so be sure to
    checkout the documentation there too.
    """
    def __init__(self, *args, **kwargs):
        super(DualSatCase, self).__init__(*args, **kwargs)

        self.leader_initial_state = "startup"
        self.follower_initial_state = "startup"
        self.initial_state_timeout = 25
        self.leader_skip_deployment_wait = False
        self.follower_skip_deployment_wait = False
        self.leader_suppress_faults = True
        self.follower_suppress_faults = True

    def populate_devices(self, devices, radios):
        if devices:
            self.flight_controller_leader = devices["FlightControllerLeader"]
            self.flight_controller_follower = devices["FlightControllerFollower"]
            self.devices = [self.flight_controller_leader, self.flight_controller_follower]
        if "FlightControllerLeaderRadio" in radios:
            self.radio_leader = radios["FlightControllerLeaderRadio"]
            self.radio_follower = radios["FlightControllerFollowerRadio"]
        else:
            self.radio_leader = None
            self.radio_follower = None

    def setup(self, devices, radios):
        super(DualSatCase, self).setup(devices, radios)

        if self.leader_suppress_faults:
            self.logger.put("[TESTCASE] Suppressing leader faults!")
            suppress_faults(self.flight_controller_leader, self.logger)

        if self.follower_suppress_faults:
            self.logger.put("[TESTCASE] Suppressing follower faults!")
            suppress_faults(self.flight_controller_follower, self.logger)

        if self.leader_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping leader deployment wait!")
            self.flight_controller_leader.write_state("pan.deployment.elapsed", "15000")

        if self.follower_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping follower deployment wait!")
            self.flight_controller_follower.write_state("pan.deployment.elapsed", "15000")

        self.pre_boot()

        cycles = 0
        leader_initial_state = Enums.mission_states[self.leader_initial_state]
        leader_state = self.flight_controller_leader.smart_read("pan.state")
        follower_initial_state = Enums.mission_states[self.follower_initial_state]
        follower_state = self.flight_controller_follower.smart_read("pan.state")
        while leader_state != leader_initial_state and follower_state != follower_initial_state:
            if cycles > self.initial_state_timeout:
                raise TestCaseFailure(f"Failed to reach desired states of {leader_initial_state} and {follower_initial_state}")

            super(DualSatCase, self).cycle()

            cycles = cycles + 1
            leader_state = self.flight_controller_leader.smart_read("pan.state")
            follower_state = self.flight_controller_follower.smart_read("pan.state")

        self.post_boot()

    def pre_boot(self):
        """Allows a testcase to perform actions just after supressing faults and
        skipping deployment wait but before attempting to boot the spacecraft.
        See setup for more information.
        """
        pass

    def post_boot(self):
        """Allows a testcase to perform actions just after booting the
        spacecraft. See setup for more information.
        """
        pass

    def cycle(self):
        """Step the flight controllers forward by one CC
        """
        super(DualSatCase, self).cycle()

        self.flight_controller_leader.write_state('cycle.start', 'true')
        self.flight_controller_follower.write_state('cycle.start', 'true')

    @property
    def mission_state_leader(self):
        return Enums.mission_states[int(self.flight_controller_leader.read_state("pan.state"))]

    @property
    def mission_state_follower(self):
        return Enums.mission_states[int(self.flight_controller_follower.read_state("pan.state"))]

    @mission_state_leader.setter
    def mission_state_leader(self, state):
        self.flight_controller_leader.write_state("pan.state", int(Enums.mission_states[state]))

    @mission_state_follower.setter
    def mission_state_follower(self, state):
        self.flight_controller_follower.write_state("pan.state", int(Enums.mission_states[state]))

    def read_state_leader(self, string_state):
        return self.flight_controller_leader.read_state(string_state)

    def write_state_leader(self, string_state, state_value):
        self.flight_controller_leader.write_state(string_state, state_value)
        return self.flight_controller_leader.read_state(string_state)

    def read_state_follower(self, string_state):
        return self.flight_controller_follower.read_state(string_state)

    def write_state_follower(self, string_state, state_value):
        self.flight_controller_follower.write_state(string_state, state_value)
        return self.flight_controller_follower.read_state(string_state)
