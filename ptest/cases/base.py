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

    def __init__(self):
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

    @property
    def sim_duration(self):
        return 0

    @property
    def sim_initial_state(self):
        return 'startup'

    @property
    def single_sat_compatible(self):
        raise NotImplementedError

    def setup_case(self, simulation):
        self.sim = simulation
        self._setup_case()

    def _setup_case(self):
        raise NotImplementedError

    def run_case(self):
        raise NotImplementedError

    def str_to_bool(self, str):
        """
        Helper function to convert "True" or "False" to its boolean value.
        """

        if str == "true":
            return True
        else:
            return False

class SingleSatOnlyCase(Case):
    """
    Base testcase for writing testcases that only work with a single-satellite mission.
    """

    @property
    def single_sat_compatible(self):
        return True

    def setup_case(self, simulation):
        self.sim = simulation
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

    def read_state(self, string_state):
        return self.sim.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        self.sim.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

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

    def read_state_leader(self, string_state):
        return self.sim.flight_controller_leader.read_state(string_state)

    def write_state_leader(self, string_state, state_value):
        self.sim.flight_controller_leader.write_state(string_state, state_value)
        return self.read_state(string_state)

    def read_state_follower(self, string_state):
        return self.sim.flight_controller_follower.read_state(string_state)

    def write_state_follower(self, string_state, state_value):
        self.sim.flight_controller_follower.write_state(string_state, state_value)
        return self.read_state(string_state)
