from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc


class PiksiFaultHandler(SingleSatOnlyCase):
    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return SingleAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"

    @property
    def sim_duration(self):
        return 0

    @property
    def initial_state(self):
        return "leader"       

    @property
    def fast_boot(self):
        return True

    @property
    def debug_to_console(self):
        '''
        Default false, set to true if you want debug output from flight computer piped to console
        '''
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.no_cdpgs_max_wait = self.rs("piksi_fh.no_cdpgs_max_wait")
        self.cdpgs_delay_max_wait = self.rs("piksi_fh.cdpgs_delay_max_wait")

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")

    def cycle_no(self):
        return self.rs("pan.cycle_no")

    def check_is_leader(self):
        if self.mission_state != "leader":
            raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")

    def check_is_close_appr(self):
        if self.mission_state != "leader_close_approach":
            raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'.")

    def check_is_standby(self):
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby' after no rtk signal during wait period in 'leader_close_approach'.")

    def set_fixed_rtk(self):
        self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])

    def set_no_fix(self):
        self.ws("piksi.state", Enums.piksi_modes["no_fix"])

    def cycle_til_bound(self, n, mode=None):
        while self.cycle_tracker <= n:
            if mode == "check_close_appr":
                self.check_is_close_appr()

            self.cycle()
            self.collect_diagnostic_data()
            self.cycle_tracker += 1

            if mode == "set_rtk":
                self.set_fixed_rtk()

    def run_case_singlesat(self):
        ######################## TEST 1 ########################
        # Setup initial states
        self.collect_diagnostic_data()
        self.set_fixed_rtk()
        self.check_is_leader()

        # Test 1: Set piksi mode to 'dead'
        self.logger.put("Running first test: Piksi is Dead")
        self.ws("piksi.state",Enums.piksi_modes["dead"])
        self.cycle()

        # Mission state should be set to safehold
        if self.mission_state != "safehold":
            raise TestCaseFailure("Failed to set mission state to 'safehold' when piksi mode was dead.")
        

        ######################## TEST 2 ########################
        # Reset mission state to 'leader'
        self.ws("pan.state", Enums.mission_states["leader"])

        # Test 2: Last rtkfix before close approach
        self.logger.put("Running second test: Last rtkfix before close approach")
    
        # Change piksi mode so this is registered as the last rtkfix cycle
        self.set_no_fix()

        # Cycle for an arbitrary number of times (>0)
        self.cycle_tracker = 0
        self.cycle_til_bound(30)
   
        # Set mission state to leader_close_approach
        self.logger.put("Setting mission state to 'leader_close_approach'") 
        # Set distance between sats to pass 'close approach' threshold
        self.ws("orbit.baseline_pos", [10,10,10])        
        self.cycle()
        self.collect_diagnostic_data()
        self.check_is_close_appr()

        # Wait for no_cdpgs_max_wait cycles
        self.cycle_tracker = 0
        self.cycle_til_bound(self.no_cdpgs_max_wait, mode="check_close_appr")

        # Mission state should be set to standby
        self.check_is_standby()
        
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for 'no_cdpgs_max_wait' cycles.'")

        ######################## TEST 3 ########################
        # Reset mission state to 'leader'
        self.ws("pan.state", Enums.mission_states["leader"])

        # Test 3: Last rtkfix after close approach
        self.logger.put("Running third test: Last rtkfix after close approach")

        # Set distance between sats to not pass 'close approach' threshold
        self.ws("orbit.baseline_pos", [100,100,100])
        self.cycle()
        self.collect_diagnostic_data()
        self.check_is_leader()

        # Set mission state to leader_close_approach
        self.logger.put("Setting mission state to 'leader_close_approach'") 
        # Set distance between sats to pass 'close approach' threshold
        self.ws("orbit.baseline_pos", [10,10,10])        
        self.cycle()
        self.collect_diagnostic_data()
        self.set_fixed_rtk()
        self.check_is_close_appr()

        # Cycle for an arbitrary number of times, ensuring piksi mode is "fixed_rtk"
        self.cycle_tracker = 0
        self.cycle_til_bound(10, mode="set_rtk")

        # Change piksi mode so this is registered as the last rtkfix cycle
        self.ws("piksi.state", Enums.piksi_modes["no_fix"])

        # Wait for cdpgs_delay_max_wait cycles
        self.cycle_tracker = 0
        self.cycle_til_bound(self.cdpgs_delay_max_wait, mode="check_close_appr")

        # Mission state should be set to standby
        self.check_is_standby()
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for 'cdpgs_delay_max_wait' cycles.'")

        self.finish()