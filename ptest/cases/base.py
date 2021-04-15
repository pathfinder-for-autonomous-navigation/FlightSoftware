from ..data_consumers import Logger
import time
import math
import threading
import traceback
from .utils import BootUtil, Enums, TestCaseFailure
import psim # the actual python psim repo
import lin
import datetime

class PTestCase(object):
    """
    Base class for all HITL/HOOTL testcases.
    """

    def __init__(self, is_interactive, random_seed, data_dir, device_config):
        self._finished = False
        self.is_interactive = is_interactive
        self.random_seed = random_seed
        self.data_dir = data_dir
        self.logger = Logger("testcase", data_dir, print=True)

        self.device_config = device_config

        self.errored = False
        self.finished = False
        self.devices = None

    @property
    def sim_configs(self):
        '''
        The parameter files from psim
        '''
        return []

    @property
    def sim_model(self):
        '''
        The psim simulation model
        '''
        return None

    @property
    def sim_mapping(self):
        '''
        Json file name that contains the mappings desired
        '''
        return None

    @property
    def scrape_uplinks(self):
        '''
        Look for sent uplinks by scraping the sent mail box in the pan email account
        and send packets to the Flight Computer
        '''
        return False

    @property
    def debug_to_console(self):
        '''
        Default false, set to true if you want debug output from flight computer piped to console
        '''
        return False

    @property 
    def suppress_faults(self):
        '''
        If true, faults will be suppressed on boot. Default True.
        '''
        return True

    @property
    def sim_duration(self):
        """
        Returns the duration of the simulation to run. If set to zero, the
        simulation will not start. sim_duration is measured in seconds.

        Usual values of this field are either 0 or float("inf").
        """
        return 0

    @property
    def sim_initial_state(self):
        """
        Initial state that is fed into the MATLAB simulation.
        """
        return 'startup'

    @property
    def sim_ic_map(self):
        """
        A dictionary of strings representing sim key names to
        values that should be overriding the sim initial conditions
        
        Defaults to empty dict (nothing is mutated)
        """
        return {}

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

    
    def setup_case(self, devices, radios):
        '''
        Entry point for simulation creation
        '''
        self.populate_devices(devices, radios)

        for _,device in devices.items():
            device.case_interaction_setup(self.debug_to_console)

        if self.sim_duration > 0:
            from ..psim import CppSimulation # Lazy import
            self.sim = CppSimulation(self.is_interactive, devices, 
            self.random_seed, self, self.sim_duration, self.sim_initial_state, 
            isinstance(self, SingleSatOnlyCase), self.sim_configs, self.sim_model, 
            self.sim_mapping, self.device_config)
        self.logger.start()
        self.logger.put("[TESTCASE] Starting testcase.")
        self._setup_case()

    def start(self):
        if hasattr(self, "sim"):
            self.sim.start()
        elif self.is_interactive:
            self.testcase_thread = threading.Thread(name="Testcase execution",
                                        target=self.run)
            self.testcase_thread.start()
        else:
            self.run()

    def run(self):
        while not self.finished:
            self.run_case()

    def populate_devices(self, devices, radios):
        """
        Read the list of PTest-connected devices and
        pull in the ones that we care about.
        """
        raise NotImplementedError

    def _setup_case(self):
        """
        Must be implemented by subclasses.
        """
        raise NotImplementedError

    def run_case(self):
        try:
            self._run_case()
        except TestCaseFailure:
            tb = traceback.format_exc()
            self.logger.put(tb)
            self.finish(error=True)
            return

    def _run_case(self):
        """
        Must be implemented by subclasses.
        """
        raise NotImplementedError

    def finish(self, error = False):
        """
        When called, this function indicates to PTest that
        the testcase has finished its execution.
        """

        self.errored = error

        if not self.finished:
            self.logger.put("[TESTCASE] Finished testcase.")
            self.finished = True
            if hasattr(self, "sim"):
                self.sim.stop(self.data_dir)
            self.logger.stop()
            time.sleep(1) # Allow time for logger to stop

    def rs_psim(self, name):
        '''
        Read a psim state field with <name>, log to datastore, and return the python value
        '''
        ret = self.sim.mysim.get(name)
        if(ret is None):
            raise NameError(f"ptest read failed: psim state field {name} does not exist!")
        
        stripped = ret
        if type(ret) in {lin.Vector2, lin.Vector3, lin.Vector4}:
            ret = list(ret)
            stripped = str(ret).strip("[]").replace(" ","")+","
        
        packet = {}
        
        packet["t"] = int(self.sim.mysim["truth.t.ns"]/1e9/1e3) # t: number of ms since sim start
        packet["field"] = name
        packet["val"] = str(stripped)
        packet["time"] = str(datetime.datetime.now())

        # log to datastore
        for d in self.devices:
            d.datastore.put(packet)

        return ret

    def print_rs_psim(self, name):
        '''
        Read a psim state field with <name>, log to datastore, print to console and return the python value
        '''
        ret = self.rs_psim(name)
        self.logger.put(f"{name} is {ret}")

class SingleSatOnlyCase(PTestCase):
    """
    Base testcase for writing testcases that only work with a single-satellite mission.
    """

    @property
    def initial_state(self):
        """
        Sets the initial state for the boot utility.
        """
        return "manual"

    def populate_devices(self, devices, radios):
        self.flight_controller = devices["FlightController"]
        self.devices = [self.flight_controller]

    @property
    def fast_boot(self):
        """
        If true, the boot utility will immediately jump to the initial_state
        rather than stepping through the state machine.
        """
        return True

    def _setup_case(self):
        self.setup_pre_bootsetup()

        self.one_day_ccno = self.flight_controller.smart_read("pan.one_day_ccno")

        self.boot_util = BootUtil(self.flight_controller, self.logger, self.initial_state, 
            self.fast_boot, self.one_day_ccno, self.suppress_faults)

        self.boot_util.setup_boot()
        self.setup_post_bootsetup()

    def setup_pre_bootsetup(self):
        """
        Setup that should run prior to the boot utility setup.

        The boot utility sets up fault suppressions and timeouts so that it steps through
        the state machine in the correct way. Therefore, all fault-suppression related
        setup that overlaps with the boot utility should happen in the setup_post_bootsetup
        function, not here.
        """
        pass

    def setup_post_bootsetup(self):
        """
        Setup that should run after the boot utility has finished its setup. See
        documentation for setup_pre_bootsetup for more details.
        """
        pass

    def _run_case(self):
        if not self.boot_util.finished_boot(): return
        self.run_case_singlesat()

    def run_case_singlesat(self):
        """
        Interface method that will contain the body of the test, which must be implemented by testcases.
        
        This function is analogous to Arduino's loop() method. It will run repeatedly, in step with the
        MATLAB simulation (if it is turned on.)
        """
        raise NotImplementedError

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

        init = self.rs("pan.cycle_no")
        self.flight_controller.write_state('cycle.start', 'true')
        if self.rs("pan.cycle_no") != init + 1:
            raise TestCaseFailure(f"FC did not step forward by one cycle")

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

class MissionCase(PTestCase):
    """
    Base testcase for writing testcases that only work with a full mission simulation
    with both satellites.

    This function contains many functions that have exactly the same purpose as their
    counterparts in SingleSatOnlyCase. Be sure to read the class documentation for that
    case.
    """

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
        

    @property
    def initial_state_leader(self):
        raise NotImplementedError
    @property
    def initial_state_follower(self):
        raise NotImplementedError

    @property
    def fast_boot_leader(self):
        raise NotImplementedError
    @property
    def fast_boot_follower(self):
        raise NotImplementedError

    def _setup_case(self):
        if self.devices != None:
            self.setup_pre_bootsetup_leader()
            self.setup_pre_bootsetup_follower()
            self.one_day_ccno_leader = self.flight_controller_leader.smart_read("pan.one_day_ccno")
            self.one_day_ccno_follower = self.flight_controller_follower.smart_read("pan.one_day_ccno")

            self.boot_util_leader = BootUtil(
                self.flight_controller_leader, self.logger, self.initial_state_leader, 
                self.fast_boot_leader, self.one_day_ccno_leader, self.suppress_faults)
            self.boot_util_follower = BootUtil(
                self.flight_controller_follower, self.logger, self.initial_state_follower, 
                self.fast_boot_follower, self.one_day_ccno_follower, self.suppress_faults)
            self.boot_util_leader.setup_boot()
            self.boot_util_follower.setup_boot()
            self.setup_post_bootsetup_leader()
            self.setup_post_bootsetup_follower()

    def setup_pre_bootsetup_leader(self): pass
    def setup_pre_bootsetup_follower(self): pass
    def setup_post_bootsetup_leader(self): pass
    def setup_post_bootsetup_follower(self): pass

    def _run_case(self):
        if self.devices != None:
            if not self.boot_util_follower.finished_boot(): return
            if not self.boot_util_leader.finished_boot(): return
        self.run_case_fullmission()

    def run_case_fullmission(self):
        raise NotImplementedError

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
