from ..data_consumers import Logger
import time
import math
from .utils import BootUtil, TestCaseFailure, Enums

# Base classes for writing testcases.

class Case(object):
    """
    Base class for all HITL/HOOTL testcases.
    """

    def __init__(self, is_interactive, data_dir):
        self._finished = False
        self.is_interactive = is_interactive
        self.logger = Logger("testcase", data_dir, print=True)

    def mag_of(self, vals):
        """
        Returns the magnitude of a list of vals 
        by taking the square root of the sum of the square of the components.
        """

        assert(type(vals) is list)
        return math.sqrt(sum([x*x for x in vals]))

    def sum_of_differentials(self, lists_of_vals):
        """
        Given a list of list of vals, return the sum of all the differentials from one list to the next.

        Returns a val.

        Ex: sum_of_differentials([[1,1,1],[1,2,3],[1,2,2]]) evaluates to 4
        """

        total_diff = [0 for x in lists_of_vals[0]]
        for i in range(len(lists_of_vals) - 1):
            diff = [abs(lists_of_vals[i][j] - lists_of_vals[i+1][j]) for j in range(len(lists_of_vals[i]))]
            total_diff = [diff[x] + total_diff[x] for x in range(len(total_diff))]

        return sum(total_diff)

    @property
    def sim_duration(self):
        """
        Returns the duration that the MATLAB simulation to run. If set to zero, the MATLAB
        simulation will not start.

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
    def finished(self):
        """
        Should be set to true if the testcase has completed.
        """
        return self._finished

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

    @finished.setter
    def finished(self, finished):
        assert type(finished) is bool
        self._finished = finished

    def setup_case(self, devices):
        self.populate_devices(devices)
        self.logger.start()
        self.logger.put("[TESTCASE] Starting testcase.")
        self._setup_case()

    def populate_devices(self, devices):
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
        """
        Must be implemented by subclasses.
        """
        raise NotImplementedError
            
    def finish(self):
        """
        When called, this function indicates to PTest that
        the testcase has finished its execution.
        """

        if not self.finished:
            self.logger.put("[TESTCASE] Finished testcase.")
            self.finished = True
            time.sleep(1)
            self.logger.stop()

class SingleSatOnlyCase(Case):
    """
    Base testcase for writing testcases that only work with a single-satellite mission.
    """

    @property
    def initial_state(self):
        """
        Sets the initial state for the boot utility.
        """
        return "manual"

    def populate_devices(self, devices):
        self.flight_controller = devices["FlightController"]

    @property
    def fast_boot(self):
        """
        If true, the boot utility will immediately jump to the initial_state
        rather than stepping through the state machine.
        """
        return True

    def _setup_case(self):
        self.setup_pre_bootsetup()

        # Prevent faults from mucking up the state machine.
        self.flight_controller.write_state("gomspace.low_batt.suppress", "true")
        self.flight_controller.write_state("fault_handler.enabled", "false")
        self.one_day_ccno = self.flight_controller.smart_read("pan.one_day_ccno")

        self.boot_util = BootUtil(self.flight_controller, self.logger, self.initial_state, self.fast_boot, self.one_day_ccno)
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

    def run_case(self):
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

class MissionCase(Case):
    """
    Base testcase for writing testcases that only work with a full mission simulation
    with both satellites.

    This function contains many functions that have exactly the same purpose as their
    counterparts in SingleSatOnlyCase. Be sure to read the class documentation for that
    case.
    """

    def populate_devices(self, devices):
        self.flight_controller_leader = devices["FlightControllerLeader"]
        self.flight_controller_follower = devices["FlightControllerFollower"]

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
        self.setup_pre_bootsetup_leader()
        self.setup_pre_bootsetup_follower()
        self.boot_util_leader = BootUtil(self.flight_controller_leader, self.logger, self.initial_state_leader, self.fast_boot_leader)
        self.boot_util_follower = BootUtil(self.flight_controller_follower, self.logger, self.initial_state_follower, self.fast_boot_follower)
        self.boot_util_leader.setup_boot()
        self.boot_util_follower.setup_boot()
        self.setup_post_bootsetup_leader()
        self.setup_post_bootsetup_follower()

    def setup_pre_bootsetup_leader(self): pass
    def setup_pre_bootsetup_follower(self): pass
    def setup_post_bootsetup_leader(self): pass
    def setup_post_bootsetup_follower(self): pass

    def run_case(self):
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
        return self.read_state(string_state)

    def read_state_follower(self, string_state):
        return self.flight_controller_follower.read_state(string_state)

    def write_state_follower(self, string_state, state_value):
        self.flight_controller_follower.write_state(string_state, state_value)
        return self.read_state(string_state)