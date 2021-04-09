from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class PiksiFaultHandler_1(SingleSatOnlyCase):
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
        return float("inf")

    @property
    def initial_state(self):
        return "leader"       

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")

    def run_case_singlesat(self):
        # Setup initial states
        self.collect_diagnostic_data()
        self.cycle_tracker = 0
        self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
        if self.mission_state != "leader":
            print(self.mission_state)
            raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")

        # Test 1: Set piksi mode to 'dead'
        self.logger.put("Running first test: dead piksi")
        self.ws("piksi.state",Enums.piksi_modes["dead"])
        self.cycle()

        # Mission state should be set to standby
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby' when piksi mode was dead.")
        
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' when piksi was dead.'")
        self.finish()




class PiksiFaultHandler_2(SingleSatOnlyCase):
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
        return float("inf")

    @property
    def initial_state(self):
        return "leader"       

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.no_cdpgs_max_wait = self.rs("piksi_fh.no_cdpgs_max_wait")

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")

    def cycle_no(self):
        return self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        # Setup initial states
        self.collect_diagnostic_data()
        self.cycle_tracker = 0
        self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
        if self.mission_state != "leader":
            raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")
        
        # Test 2: Last rtkfix before close approach
        self.logger.put("Running second test: Last rtkfix before close approach")

        # Change piksi mode so this is registered as the last rtkfix cycle
        self.ws("piksi.state", Enums.piksi_modes["no_fix"])

        # This should work and I'm not sure why it doesn't
        # Cycle for an arbitrary number of times (>0)
        # while self.cycle_tracker < 10:
        #     # Debugging
        #     # if self.cycle_tracker <= self.cdpgs_delay_max_wait:
        #     #     print(self.mission_state)
        #     if self.rs("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
        #         print("AHHH IT WENT BACK TO FIXED RTK")
        #     self.cycle()
        #     self.cycle_tracker += 1
        #     # Make sure piksi is still in no_fix state
        #     self.ws("piksi.state", Enums.piksi_modes["no_fix"])
        #     if self.rs("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
        #         print("AHHH IT WENT BACK TO FIXED RTK")
        
        # self.cycle_tracker = 0
   
        # Set mission state to leader_close_approach
        self.logger.put("Setting mission state to 'leader_close_approach'")     
        self.ws("pan.state", Enums.mission_states["leader_close_approach"])
      
        # Wait for cdpgs_delay_max_wait cycles
        while self.cycle_tracker <= self.no_cdpgs_max_wait:
           # Debugging
            if self.cycle_tracker <= self.no_cdpgs_max_wait:
                print(self.mission_state)

            if self.mission_state != "leader_close_approach":
                raise TestCaseFailure("Mission state was changed from 'leader_close_approach' before wait period was over.")

            self.cycle()
            self.cycle_tracker += 1

        # Mission state should be set to standby
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby' after one day of no rtk signal in 'leader_close_approach'.")
        
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for max time.'")
        self.finish()



class PiksiFaultHandler_3(SingleSatOnlyCase):
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
        return float("inf")

    @property
    def initial_state(self):
        return "leader"       

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)
        self.cdpgs_delay_max_wait = self.rs("piksi_fh.cdpgs_delay_max_wait")

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")

    def cycle_no(self):
        return self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        # Setup initial states
        self.collect_diagnostic_data()
        self.cycle_tracker = 0
        self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
        if self.mission_state != "leader":
            raise TestCaseFailure("PTest does not intialize mission state to 'leader'.")
        
        # Set mission state to leader_close_approach
        self.logger.put("Setting mission state to 'leader_close_approach'")     
        self.ws("pan.state", Enums.mission_states["leader_close_approach"])
        self.cycle()
        if self.mission_state != "leader_close_approach":
            raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'.")

        # Cycle for an arbitrary number of times (>0)
        while self.cycle_tracker < 10:
            # Debugging
            # if self.cycle_tracker <= self.cdpgs_delay_max_wait:
            #     print(self.mission_state)
            self.cycle()
            self.cycle_tracker += 1
            # Make sure piksi is still in fixed_rtk state
            self.ws("piksi.state", Enums.piksi_modes["fixed_rtk"])
        
        self.cycle_tracker = 0

        # Change piksi mode so this is registered as the last rtkfix cycle
        self.ws("piksi.state", Enums.piksi_modes["no_fix"])

        # Wait for cdpgs_delay_max_wait cycles
        while self.cycle_tracker <= self.cdpgs_delay_max_wait:
            # Debugging
            # if self.cycle_tracker <= self.cdpgs_delay_max_wait:
            #     print(self.mission_state)

            if self.mission_state != "leader_close_approach":
                raise TestCaseFailure("Mission state was changed from 'leader_close_approach' before wait period was over.")

            self.cycle()
            self.cycle_tracker += 1

        # Mission state should be set to standby
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby' after one day of no rtk signal in 'leader_close_approach'.")
        
        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for max time.'")
        self.finish()
