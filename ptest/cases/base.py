from ..data_consumers import Logger
import time
import math

# Base classes for writing testcases.
class TestCaseFailure(Exception):
    """Raise in case of test case failure."""

class FSWEnum(object):
    """
    Class that encodes flight software enums into Python objects in
    a way that makes them easy to access during testing.
    """

    def __init__(self, arr):
        self.arr = arr
        self._indexed_by_name = {}
        self._indexed_by_num = {}
        for i in range(len(arr)):
            self._indexed_by_name[arr[i]] = i
            self._indexed_by_num[i] = arr[i]

    def get_by_name(self, name):
        return self._indexed_by_name[name]

    def get_by_num(self, num):
        return self._indexed_by_num[num]


class Case(object):
    """
    Base class for all HITL/HOOTL testcases.
    """

    def __init__(self, data_dir):
        self._finished = False

        self.mission_states = FSWEnum([
            "startup",
            "detumble",
            "initialization_hold",
            "standby",
            "follower",
            "leader",
            "follower_close_approach",
            "leader_close_approach",
            "docking",
            "docked",
            "safehold",
            "manual"
        ])

        self.prop_states = FSWEnum([
            "disabled",
            "idle",
            "await_pressurizing",
            "pressurizing",
            "venting",
            "firing",
            "await_firing",
            "handling_fault",
            "manual"
        ])

        self.adcs_states = FSWEnum([
            "startup",
            "limited",
            "zero_torque",
            "zero_L",
            "detumble",
            "point_manual",
            "point_standby",
            "point_docking"
        ])

        self.radio_states = FSWEnum([
            "disabled",
            "wait",
            "transceive",
            "read",
            "write",
            "config"
        ])

        self.sat_designations = FSWEnum([
            "undecided",
            "leader",
            "follower"
        ])

        self.piksi_modes = FSWEnum([
            "spp",
            "fixed_rtk",
            "float_rtk",
            "no_fix",
            "sync_error",
            "nsat_error",
            "crc_error",
            "time_limit_error",
            "data_error",
            "no_data_error",
            "dead"
        ])
        
        self.rwa_modes = FSWEnum([
            "RWA_DISABLED",
            "RWA_SPEED_CTRL",
            "RWA_ACCEL_CTRL"
        ])

        self.imu_modes = FSWEnum([
            "IMU_MAG_NORMAL",
            "IMU_MAG_CALIBRATE"
        ])

        self.mtr_modes = FSWEnum([
            "MTR_ENABLED",
            "MTR_DISABLED"
        ])
        self.havt_length = 18
        
        # copied from havt_devices.hpp
        self.havt_devices = FSWEnum([
        "IMU_GYR",
        "IMU_MAG1",
        "IMU_MAG2",
        "MTR1",
        "MTR2",
        "MTR3",
        "RWA_POT",
        "RWA_WHEEL1",
        "RWA_WHEEL2",
        "RWA_WHEEL3",
        "RWA_ADC1",
        "RWA_ADC2",
        "RWA_ADC3",
        "SSA_ADC1",
        "SSA_ADC2",
        "SSA_ADC3",
        "SSA_ADC4",
        "SSA_ADC5"])

        self.logger = Logger("testcase", data_dir, print=True)

    def mag_of(self, vals):
        '''
        Returns the magnitude of a list of vals 
        by taking the square root of the sum of the square of the components.
        '''
        assert(type(vals) is list)
        return math.sqrt(sum([x*x for x in vals]))

    def sum_of_differentials(self, lists_of_vals):
        '''
        Given a list of list of vals, return the sum of all the differentials from one list to the next.

        Returns a val.

        Ex: sum_of_differentials([[1,1,1],[1,2,3],[1,2,2]]) evaluates to 4
        '''
        total_diff = [0 for x in lists_of_vals[0]]
        for i in range(len(lists_of_vals) - 1):
            diff = [abs(lists_of_vals[i][j] - lists_of_vals[i+1][j]) for j in range(len(lists_of_vals[i]))]
            total_diff = [diff[x] + total_diff[x] for x in range(len(total_diff))]

        return sum(total_diff)

    @property
    def sim_duration(self):
        return 0

    @property
    def sim_initial_state(self):
        return 'startup'

    @property
    def single_sat_compatible(self):
        raise NotImplementedError

    @property
    def finished(self):
        return self._finished
    
    @finished.setter
    def finished(self, finished):
        assert type(finished) is bool
        self._finished = finished

    def setup_case(self, simulation):
        self.sim = simulation
        self.logger.start()
        self.logger.put("[TESTCASE] Starting testcase.")
        self._setup_case()

    def _setup_case(self):
        raise NotImplementedError

    def run_case(self):
        raise NotImplementedError
            
    def finish(self):
        if not self.finished:
            if not self.sim.is_interactive:
                self.sim.running = False
            self.logger.put("[TESTCASE] Finished testcase.")
            self.finished = True
            time.sleep(1)
            self.logger.stop()

class SingleSatOnlyCase(Case):
    """
    Base testcase for writing testcases that only work with a single-satellite mission.
    """

    @property
    def single_sat_compatible(self):
        return True

    def _setup_case(self):
        if self.sim.is_single_sat_sim:
            self.setup_case_singlesat()
        else:
            raise NotImplementedError

    def run_case(self):
        if not self.sim.is_single_sat_sim:
            raise Exception(f"Testcase {__class__.__name__} only works for a single-satellite simulation.")
        self.run_case_singlesat()

    def run_case_singlesat(self):
        raise NotImplementedError

    def cycle(self):
        ''' 
        Steps the FC forward by one CC

        Asserts the FC did indeed step forward by one CC
        '''
        init = self.rs("pan.cycle_no")
        self.sim.flight_controller.write_state('cycle.start', 'true')
        if self.rs("pan.cycle_no") != init + 1:
            raise TestCaseFailure(f"FC did not step forward by one cycle")

    def rs(self, name):
        '''
        Reads a state field (with type inference from smart_read()).

        Checks that the name is indeed a string.
        '''
        assert(type(name) is str), "State field name was not a string."
        ret = self.sim.flight_controller.smart_read(name)
        return ret

    def print_rs(self, name):
        '''
        Reads a statefield, and also prints it.
        '''
        self.logger.put(f"{name} is {self.rs(name)}")
    
    def ws(self, name, val):
        '''
        Writes a state, and also confirms that the read command matches the applied state.
        '''
        self.sim.flight_controller.write_state(name, val)
        read_val = self.rs(name)
        assert(read_val == val), f"Write state not applied, expected: {val}, got {read_val} instead"

    def print_header(self, title):
        self.logger.put("\n"+title+"\n")

    def soft_assert(self, condition, *args):
        '''
        Soft assert prints a fail message if the condition is False
        
        If specificied with a fail message, then a pass message, 
        it will also print a pass message if condition is True.
        '''
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
    """

    @property
    def single_sat_compatible(self):
        return False

    def run_case(self):
        if self.sim.is_single_sat_sim:
            raise Exception(f"Testcase {__class__.__name__} only works for a full-mission simulation.")
        self.run_case_fullmission()

    def run_case_fullmission(self):
        raise NotImplementedError


class FlexibleCase(Case):
    """
    Base class for cases that should be able to work with either 1 or 2
    satellites.
    """

    @property
    def single_sat_compatible(self):
        return True

    def _setup_case(self):
        if self.sim.is_single_sat_sim:
            self.setup_case_singlesat()
        else:
            self.setup_case_fullmission()

    def setup_case_singlesat(self):
        raise NotImplementedError

    def setup_case_fullmission(self):
        raise NotImplementedError

    def run_case(self):
        if self.sim.is_single_sat_sim:
            self.run_case_singlesat()
        else:
            self.run_case_fullmission()

    def run_case_singlesat(self):
        raise NotImplementedError

    def run_case_fullmission(self):
        raise NotImplementedError
